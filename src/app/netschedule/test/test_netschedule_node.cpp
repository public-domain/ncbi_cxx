/*  $Id: test_netschedule_node.cpp 157175 2009-04-13 17:20:31Z kazimird $
 * ===========================================================================
 *
 *                            PUBLIC DOMAIN NOTICE
 *               National Center for Biotechnology Information
 *
 *  This software/database is a "United States Government Work" under the
 *  terms of the United States Copyright Act.  It was written as part of
 *  the author's official duties as a United States Government employee and
 *  thus cannot be copyrighted.  This software/database is freely available
 *  to the public for use. The National Library of Medicine and the U.S.
 *  Government have not placed any restriction on its use or reproduction.
 *
 *  Although all reasonable efforts have been taken to ensure the accuracy
 *  and reliability of the software and data, the NLM and the U.S.
 *  Government do not and cannot warrant the performance or results that
 *  may be obtained by using this software or data. The NLM and the U.S.
 *  Government disclaim all warranties, express or implied, including
 *  warranties of performance, merchantability or fitness for any particular
 *  purpose.
 *
 *  Please cite the author in any work or product based on this material.
 *
 * ===========================================================================
 *
 * Authors:  Anatoliy Kuznetsov, Dmitry Kazimirov
 *
 * File Description:  NetSchedule client test
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbireg.hpp>
#include <corelib/ncbi_system.hpp>
#include <corelib/ncbimisc.hpp>

#include <connect/services/netschedule_api.hpp>
#include <connect/ncbi_socket.hpp>
#include <connect/ncbi_types.h>

#include <util/random_gen.hpp>

#include "test_netschedule_data.hpp"


BEGIN_NCBI_SCOPE

NCBI_PARAM_DECL(string, output, size_distr);
typedef NCBI_PARAM_TYPE(output, size_distr) TParam_SizeDistr;

NCBI_PARAM_DECL(string, output, time_distr);
typedef NCBI_PARAM_TYPE(output, time_distr) TParam_TimeDistr;

NCBI_PARAM_DECL(double, output, failure_rate);
typedef NCBI_PARAM_TYPE(output, failure_rate) TParam_FailureRate;

/// @internal
class CInvalidParamException : public CException
{
public:
    enum EErrCode {
        eUndefined = 1,
        eNotANumber,
        eInvalidCharacter
    };

    /// Translate from an error code value to its string representation.
    virtual const char* GetErrCodeString(void) const;

    // Standard exception boilerplate code.
    NCBI_EXCEPTION_DEFAULT(CInvalidParamException, CException);
};

const char* CInvalidParamException::GetErrCodeString(void) const
{
    switch (GetErrCode()) {
    case eUndefined:
        return "eUndefined";

    case eNotANumber:
        return "eNotANumber";

    case eInvalidCharacter:
        return "eInvalidCharacter";

    default:
        return CException::GetErrCodeString();
    }
}

static CRandom s_Random;

///////////////////////////////////////////////////////////////////////
// This class generates a random integer from a series of ranges
// defined like this: 5, 6 - 9, 10 - 50
class CDistribution
{
public:
    void LoadFromParameter(const char* parameter_name,
        const char* parameter_value);

    unsigned GetNextValue() const;

private:
    typedef std::pair<unsigned, unsigned> TRange;
    typedef std::vector<TRange> TRangeVector;

    const char* SkipSpaces(const char* input_string);

    TRangeVector m_RangeVector;
};

void CDistribution::LoadFromParameter(const char* parameter_name,
    const char* parameter_value)
{
    if (*parameter_value == '\0') {
        NCBI_THROW(CInvalidParamException, eUndefined,
            string("Configuration parameter '") + parameter_name +
                "' was not defined.");
    }

    m_RangeVector.clear();

    const char* pos = parameter_value;

    TRange new_range;

    unsigned* current_bound_ptr = &new_range.first;
    new_range.second = 0;

    for (;;) {
        pos = SkipSpaces(pos);

        unsigned bound = (unsigned) (*pos - '0');

        if (bound > 9) {
            NCBI_THROW(CInvalidParamException, eNotANumber,
                string("In configuration parameter '") + parameter_name +
                    "': not a number at position " +
                        NStr::UIntToString(pos - parameter_value + 1) + ".");
        }

        unsigned digit;

        while ((digit = (unsigned) (*++pos - '0')) <= 9)
            bound = bound * 10 + digit;

        *current_bound_ptr = bound;

        pos = SkipSpaces(pos);

        switch (*pos) {
        case '\0':
            m_RangeVector.push_back(new_range);
            return;

        case ',':
            m_RangeVector.push_back(new_range);
            ++pos;
            current_bound_ptr = &new_range.first;
            new_range.second = 0;
            break;

        case '-':
            ++pos;
            current_bound_ptr = &new_range.second;
            break;

        default:
            NCBI_THROW(CInvalidParamException, eInvalidCharacter,
                string("In configuration parameter '") + parameter_name +
                    "': invalid character at position " +
                        NStr::UIntToString(pos - parameter_value + 1) + ".");
        }
    }
}

unsigned CDistribution::GetNextValue() const
{
    CRandom::TValue random_number = s_Random.GetRand();

    TRangeVector::const_iterator random_range =
        m_RangeVector.begin() + (random_number % m_RangeVector.size());

    return random_range->second == 0 ? random_range->first :
        random_range->first +
            (random_number % (random_range->second - random_range->first + 1));
}

const char* CDistribution::SkipSpaces(const char* input_string)
{
    while (*input_string == ' ' || *input_string == '\t')
        ++input_string;

    return input_string;
}


/// Test application
///
/// @internal
///
class CTestNetScheduleNode : public CNcbiApplication
{
public:
    void Init(void);
    int Run(void);

private:
    CDistribution m_SizeDistr;
    CDistribution m_TimeDistr;
};


NCBI_PARAM_DEF_EX(string, output, size_distr, kEmptyStr,
    eParam_NoThread, NS_OUTPUT_SIZE_DISTR);
NCBI_PARAM_DEF_EX(string, output, time_distr, kEmptyStr,
    eParam_NoThread, NS_OUTPUT_TIME_DISTR);
NCBI_PARAM_DEF_EX(double, output, failure_rate, 0.0,
    eParam_NoThread, NS_OUTPUT_FAILURE_RATE);

void CTestNetScheduleNode::Init(void)
{
    InitOutputBuffer();

    SetDiagPostFlag(eDPF_Trace);
    SetDiagPostLevel(eDiag_Info);

    // Setup command line arguments and parameters

    // Create command-line argument descriptions class
    auto_ptr<CArgDescriptions> arg_desc(new CArgDescriptions);

    // Specify USAGE context
    arg_desc->SetUsageContext(GetArguments().GetProgramBasename(),
        "NetSchedule node");

    arg_desc->AddPositional("service", "NetSchedule service name.",
        CArgDescriptions::eString);

    arg_desc->AddPositional("queue", "NetSchedule queue name (like: noname).",
        CArgDescriptions::eString);

    arg_desc->AddOptionalKey("udp_port", "udp_port",
        "Incoming UDP port", CArgDescriptions::eInteger);

    arg_desc->AddFlag("verbose", "Log progress.");

    // Setup arg.descriptions for this application
    SetupArgDescriptions(arg_desc.release());
}

int CTestNetScheduleNode::Run(void)
{
    m_SizeDistr.LoadFromParameter("size_distr",
        TParam_SizeDistr::GetDefault().c_str());
    m_TimeDistr.LoadFromParameter("time_distr",
        TParam_TimeDistr::GetDefault().c_str());

    CArgs args = GetArgs();

    string service = args["service"].AsString();
    string queue_name = args["queue"].AsString();

    unsigned short udp_port = 9111;

    if (args["udp_port"])
        udp_port = args["udp_port"].AsInteger();

    bool verbose = args["verbose"];

    string program_name = GetProgramDisplayName();

    LOG_POST(Info << program_name << " started");
    LOG_POST(Info << "UDP port: " << udp_port);
    LOG_POST(Info << "Failure rate: " << TParam_FailureRate::GetDefault());

    CNetScheduleAPI ns_api(service, program_name, queue_name);

    CNetScheduleExecuter ns_exec = ns_api.GetExecuter(udp_port);

    string job_key;
    string input;

    set<string> jobs_processed;

    // The main job processing loop, polls the
    // queue for available jobs
    //
    // There is no payload algorithm here, node just
    // sleeps and reports the processing result back to server.
    // Practical application should use NetCache as result storage
    //
    // Well behaved node should not constantly poll the queue for
    // jobs (GetJob()).
    // When NetSchedule Server reports there are no more jobs: worker node
    // should take a brake and do not poll for a while...
    // Or use WaitJob(), it receives notification messages from the
    // server using stateless UDP protocol.
    // It is strongly suggested that there is just one program using
    // specified UDP port on the machine.

    CNetScheduleJob job;

    string expected_input = "Hello " + queue_name;

    bool done = false;

    while (!done) {
        if (ns_exec.WaitJob(job, 180, udp_port)) {
            if (job.input == "DIE") {
                LOG_POST(Info << "Got poison pill, exiting.");
                done = true;
            } else if (job.input != expected_input) {
                LOG_POST(Error << "Unexpected input: " + input);
            }

            // do no job here, just delay for a little while
            unsigned delay = m_TimeDistr.GetNextValue();
            if (delay > 0)
                SleepMilliSec(delay);

            unsigned output_size = m_SizeDistr.GetNextValue();
            if (output_size > 0) {
                if (output_size > MAX_OUTPUT_SIZE)
                    output_size = MAX_OUTPUT_SIZE;

                job.output.assign(output_buffer, output_size);
                job.output[output_size - 1] = '\n';
            } else
                job.output.erase();

            job.ret_code = 0;

            if (s_Random.GetRand() <
                TParam_FailureRate::GetDefault() * s_Random.GetMax())
                ns_exec.PutFailure(job);
            else
                ns_exec.PutResult(job);

            if (jobs_processed.find(job.job_id) == jobs_processed.end()) {
                jobs_processed.insert(job.job_id);
                if (verbose) {
                    LOG_POST(Info << "Job " << job.job_id << " is done.");
                }
            } else {
                LOG_POST(Error << "Job: " << job.job_id <<
                    " has already been processed.");
            }
        }
    }

    return 0;
}

END_NCBI_SCOPE


USING_NCBI_SCOPE;

int main(int argc, const char* argv[])
{
    return CTestNetScheduleNode().AppMain(argc, argv, 0, eDS_Default);
}
