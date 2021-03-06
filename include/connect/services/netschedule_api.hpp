#ifndef CONN___NETSCHEDULE_API__HPP
#define CONN___NETSCHEDULE_API__HPP

/*  $Id: netschedule_api.hpp 157173 2009-04-13 17:19:28Z kazimird $
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
 * Authors:  Anatoliy Kuznetsov, Maxim Didenko, Victor Joukov, Dmitry Kazimirov
 *
 * File Description:
 *   NetSchedule client API.
 *
 */


/// @file netschedule_api.hpp
/// NetSchedule client specs.
///

#include <connect/services/netschedule_key.hpp>
#include <connect/services/netschedule_api_expt.hpp>
#include <connect/services/netschedule_api_const.hpp>
#include <connect/services/netservice_api.hpp>

#include <corelib/plugin_manager.hpp>

BEGIN_NCBI_SCOPE


/** @addtogroup NetScheduleClient
 *
 * @{
 */

class CNetScheduleSubmitter;
class CNetScheduleExecuter;
class CNetScheduleAdmin;

struct CNetScheduleJob;

struct SNetScheduleAPIImpl;

/// Client API for NCBI NetSchedule server.
///
/// This API is logically divided into two sections:
/// Job Submitter API and Worker Node API.
///
/// As objects of this class are only smart pointers to the real
/// implementation, it is advisable that these objects are
/// allocated on the stack rather than the heap.
///
/// @sa CNetServiceException, CNetScheduleException
///

class NCBI_XCONNECT_EXPORT CNetScheduleAPI
{
    NET_COMPONENT(NetScheduleAPI);

    /// Construct the client without linking it to any particular
    /// server. Actual server (host and port) will be extracted from the
    /// job key
    ///
    /// @param service_name
    ///    Name of the service to connect to (format:
    ///    LB service name or host:port)
    /// @param client_name
    ///    Name of the client program (project)
    /// @param queue_name
    ///    Name of the job queue
    ///
    CNetScheduleAPI(const std::string& service_name,
                    const std::string& client_name,
                    const std::string& queue_name,
                    const std::string& lbsm_affinity_name = kEmptyStr);

    /// Set program version (like: MyProgram v. 1.2.3)
    ///
    /// Program version is passed to NetSchedule queue so queue
    /// controls versions and does not allow obsolete clients
    /// to connect and submit or execute jobs
    ///
    void SetProgramVersion(const string& pv);

    /// Get program version string
    const string& GetProgramVersion() const;

    /// Return Queue name
    const string& GetQueueName() const;
    // -----------------------------------------------------------------

    /// Job status codes
    /// NB: eReturned, eTimeout and eReadTimeout states are visible only
    /// in job history, and ePending cannot be there.
    enum EJobStatus
    {
        eJobNotFound = -1, ///< No such job
        ePending     = 0,  ///< Waiting for execution
        eRunning,          ///< Running on a worker node
        eReturned,         ///< Returned back to the queue (to be rescheduled)
        eCanceled,         ///< Explicitly canceled
        eFailed,           ///< Failed to run (execution timeout)
        eDone,             ///< Job is ready (computed successfully)
        eReading,          ///< Job has its output been reading
        eConfirmed,        ///< Final state - read confirmed
        eReadFailed,       ///< Final state - read failed
        eTimeout,          ///< Execution canceled due to timeout
        eReadTimeout,      ///< Reading timed out

        eLastStatus        ///< Fake status (do not use)
    };

    /// Printable status type
    static
    string StatusToString(EJobStatus status);

    /// Parse status string into enumerator value
    ///
    /// Acceptable string values:
    ///   Pending, Running, Returned, Canceled, Failed, Done, Reading,
    ///   Confirmed, ReadFailed, Timeout, ReadTimeout
    /// Abbreviated
    ///   Pend, Run, Return, Cancel, Fail
    ///
    /// @return eJobNotFound if string cannot be parsed
    static
    EJobStatus StringToStatus(const string& status_str);

    /// Job masks
    ///
    enum EJobMask {
        eEmptyMask    = 0,
        /// Exclusive job - the node executes only this job, even if
        /// there are processor resources
        eExclusiveJob = (1 << 0),
        // Not implemented yet ---v
        /// This jobs comes to the node before every regular jobs
        eOutOfOrder   = (1 << 1),
        /// This job will be scheduled to every active node
        eForEachNode  = (1 << 2),
        /// This job should be interpreted by client library, not client itself
        /// Can contain control information, e.g. instruction for node to die
        eSystemJob    = (1 << 3),
        //                     ---^
        eUserMask     = (1 << 16) ///< User's masks start from here
    };
    typedef unsigned TJobMask;

    /// Key-value pair, value can be empty
    typedef pair<string,string> TJobTag;
    typedef vector<TJobTag>     TJobTags;


    CNetScheduleSubmitter GetSubmitter();

    /// Create an instance of CNetScheduleExecuter.
    ///
    /// @param port Control port that the worker node will be
    ///             listening to.  If omitted, the INIT command
    ///             will not be sent to NetSchedule.
    CNetScheduleExecuter GetExecuter(unsigned short control_port = 0);

    CNetScheduleAdmin     GetAdmin();

    CNetService GetService();

    struct SServerParams {
        size_t max_input_size;
        size_t max_output_size;
        bool   fast_status;
    };

    const SServerParams& GetServerParams();

    /// Get job's details
    /// @param job
    ///
    EJobStatus GetJobDetails(CNetScheduleJob& job);

    void GetProgressMsg(CNetScheduleJob& job);

    void SetCommunicationTimeout(const STimeout& to)
        {GetService().SetCommunicationTimeout(to);}

    /// Connection management options
    enum EConnectionMode {
        /// Close connection after each call.
        /// This mode frees server side resources, but reconnection can be
        /// costly because of the network overhead
        eCloseConnection,

        /// Keep connection open (default).
        /// This mode occupies server side resources(session thread),
        /// use this mode very carefully
        eKeepConnection
    };

    /// Set connection mode
    /// @sa GetConnMode
    void SetConnMode(EConnectionMode conn_mode)
    {
        GetService().SetPermanentConnection(
            conn_mode == eCloseConnection ? eOff : eOn);
    }
};


/////////////////////////////////////////////////////////////////////////////////////
////
/// Job description
///
struct CNetScheduleJob
{
    CNetScheduleJob(const string& _input = kEmptyStr, const string& _affinity = kEmptyStr,
                    CNetScheduleAPI::TJobMask _mask = CNetScheduleAPI::eEmptyMask)
        : input(_input), affinity(_affinity), mask(_mask), ret_code(0) {}

    void Reset()
    {
        input.erase();
        affinity.erase();
        mask = CNetScheduleAPI::eEmptyMask;
        tags.clear();
        job_id.erase();
        ret_code = 0;
        output.erase();
        error_msg.erase();
        progress_msg.erase();
    }
    // input parameters

    ///    Input data. Arbitrary string (cannot exceed 1K). This string
    ///    encodes input data for the job. It is suggested to use NetCache
    ///    to keep the actual data and pass NetCache key as job input.
    string    input;

    string    affinity;

    std::string client_ip;
    std::string session_id;

    CNetScheduleAPI::TJobMask  mask;
    CNetScheduleAPI::TJobTags  tags;

    // output and error

    ///    Job key
    string    job_id;
    ///    Job return code.
    int       ret_code;
    ///    Job result data.  Arbitrary string (cannot exceed 1K).
    string    output;
    string    error_msg;
    string    progress_msg;
};

struct SNetScheduleSubmitterImpl;

/// Smart pointer to the job submission part of the NetSchedule API.
/// Objects of this class are returned by
/// CNetScheduleAPI::GetSubmitter().  It is possible to have several
/// submitters per one CNetScheduleAPI object.
///
/// @sa CNetScheduleAPI, CNetScheduleExecuter
class NCBI_XCONNECT_EXPORT CNetScheduleSubmitter
{
    NET_COMPONENT(NetScheduleSubmitter);

    /// Submit job.
    /// @note on success job.job_id will be set.
    string SubmitJob(CNetScheduleJob& job) const;

    /// Submit job batch.
    /// Method automatically splits the submission into reasonable sized
    /// transactions, so there is no limit on the input batch size.
    ///
    /// Every job in the job list receives job id
    ///
    void SubmitJobBatch(vector<CNetScheduleJob>& jobs) const;

    /// Initiate bulk retrieval of job results.
    /// The command retrieves ids of jobs marked as done.
    ///
    /// @param batch_id
    ///    A string reference to store the identifier of the batch,
    ///    to be passed to ReadConfirm() or ReadRollback().
    /// @param job_ids
    ///    Array for storing job ids.
    /// @param max_jobs
    ///    Maximum number of jobs returned.
    /// @param timeout
    ///    Number of seconds to wait before the status of jobs
    ///    is automatically reverted to 'done'.  If zero,
    ///    status will not be reverted automatically.
    /// @return
    ///    True if a batch was successfully retrieved from a NetSchedule
    ///    server.  False if no servers reported jobs marked as done.
    bool Read(std::string& batch_id,
        std::vector<std::string>& job_ids,
        unsigned max_jobs,
        unsigned timeout = 0);

    /// Mark the specified jobs as successfully processed.
    /// The jobs will change their status to 'confirmed' after
    /// this operation.
    ///
    /// @param batch_id
    ///    Batch number returned by Read().
    /// @param job_ids
    ///    Array of job ids, which results were successfully
    ///    retrieved and processed.
    void ReadConfirm(const std::string& batch_id,
        const std::vector<std::string>& job_ids);

    /// Refuse from processing the results of the specified jobs.
    /// The jobs will change their status back to 'done' after
    /// this operation.
    ///
    /// @param batch_id
    ///    Batch number returned by Read().
    /// @param job_ids
    ///    Array of job ids, for which status needs to be reverted
    ///    back to 'done'.
    /// @param error_message
    ///    This message can be used to describe the cause why the
    ///    job results are returned.
    void ReadRollback(const std::string& batch_id,
        const std::vector<std::string>& job_ids,
        const std::string& error_message = kEmptyStr);

    /// Submit job to server and wait for the result.
    /// This function should be used if we expect that job execution
    /// infrastructure is capable of finishing job in the specified
    /// time frame. This method can save a lot of round trips with the
    /// NetSchedule server (comparing to series of GetStatus calls).
    ///
    /// @param job
    ///    NetSchedule job description structure
    /// @param wait_time
    ///    Time in seconds function waits for the job to finish.
    ///    If job does not finish in the output parameter will hold the empty
    ///    string.
    /// @param udp_port
    ///    UDP port to listen for queue notifications. Try to avoid many
    ///    client programs (or threads) listening on the same port. Message
    ///    is going to be delivered to just only one listener.
    ///
    /// @return job status
    ///
    /// @note the result fields of the job description structure will be set
    ///    if the job is finished during specified time.
    ///
    CNetScheduleAPI:: EJobStatus SubmitJobAndWait(CNetScheduleJob& job,
                                                  unsigned       wait_time,
                                                  unsigned short udp_port);

    /// Cancel job
    ///
    /// @param job_key
    ///    Job key
    void CancelJob(const string& job_key) const;

    /// Get progress message
    ///
    /// @param job
    ///    NetSchedule job description structure. The message is taken from
    ///    progress_msg filed
    ///
    /// @sa PutProgressMsg
    ///
    void GetProgressMsg(CNetScheduleJob& job);

    /// Request of current job status
    /// eJobNotFound is returned if job status cannot be found
    /// (job record timed out)
    ///
    CNetScheduleAPI::EJobStatus GetJobStatus(const string& job_key);

    CNetScheduleAPI::EJobStatus GetJobDetails(CNetScheduleJob& job);

    const CNetScheduleAPI::SServerParams& GetServerParams();
};

/////////////////////////////////////////////////////////////////////////////////////
////
struct SNetScheduleExecuterImpl;

/// Smart pointer to a part of the NetSchedule API that does job
/// retrieval and processing on the worker node side.
/// Objects of this class are returned by
/// CNetScheduleAPI::GetExecuter().
///
/// @sa CNetScheduleAPI, CNetScheduleSubmitter
class NCBI_XCONNECT_EXPORT CNetScheduleExecuter
{
    NET_COMPONENT(NetScheduleExecuter);

    /// Get a pending job.
    /// When function returns TRUE and job_key job receives running status,
    /// client(worker node) becomes responsible for execution or returning
    /// the job. If there are no jobs in the queue function returns FALSE
    /// immediately and you have to repeat the call (after a delay).
    /// Consider WaitJob method as an alternative.
    ///
    /// @param job
    ///     NetSchedule job description structure
    /// @param udp_port
    ///     Used to instruct server that specified client does NOT
    ///     listen to notifications (opposed to WaitJob)
    ///
    /// @return
    ///     TRUE if job has been returned from the queue and its input
    ///     fields are set.
    ///     FALSE means queue is empty or for some reason scheduler
    ///     decided not to grant the job (node is overloaded).
    ///     In this case worker node should pause and come again later
    ///     for a new job.
    ///
    /// @sa WaitJob
    ///
    bool GetJob(CNetScheduleJob& job,
                unsigned short udp_port = 0) const;

    /// Notification wait mode
    enum EWaitMode {
        eWaitNotification,   ///< Wait for notification
        eNoWaitNotification  ///< Register for notification but do not wait
    };

    /// Wait for a job to come.
    /// Variant of GetJob method. The difference is that if there no
    /// pending jobs, method waits for a notification from the server.
    ///
    /// NetSchedule server sends UDP packets with queue notification
    /// information. This is unreliable protocol, some notification may be
    /// lost. WaitJob internally makes an attempt to connect the server using
    /// reliable stateful TCP/IP, so even if some UDP notifications are
    /// lost jobs will be still delivered (with a delay).
    ///
    /// When new job arrives to the queue server may not send the notification
    /// to all clients immediately, it depends on specific queue notification
    /// timeout
    ///
    /// @param job
    ///     NetSchedule job description structure
    /// @param wait_time
    ///    Time in seconds function waits for new jobs to come.
    ///    If there are no jobs in the period of time,
    ///    the function returns FALSE.
    ///    Do not specify too long waiting time because it
    ///    increases chances of UDP notification loss
    ///    (60-320 seconds is a reasonable value).
    /// @param udp_port
    ///    UDP port to listen for queue notifications. Try to avoid many
    ///    client programs (or threads) listening on the same port. Message
    ///    is going to be delivered to just only one listener.
    ///
    /// @param wait_mode
    ///    Notification wait mode. Function either waits for the message in
    ///    this call, or returns control (eNoWaitNotification).
    ///    In the second case caller should call WaitNotification to listen
    ///    for server signals.
    ///
    /// @sa GetJob, WaitNotification
    ///
    bool WaitJob(CNetScheduleJob& job,
                 unsigned       wait_time,
                 unsigned short udp_port,
                 EWaitMode      wait_mode = eWaitNotification) const;


    /// Put job result (job should be received by GetJob() or WaitJob())
    ///
    /// @param job
    ///     NetSchedule job description structure. its ret_code
    ///     and output fields should be set
    ///
    void PutResult(const CNetScheduleJob& job) const;

    /// Put job result, get new job from the queue
    /// If this is the first call and there is no previous job
    /// (done_job.job_id is empty) this is equivalent to GetJob
    ///
    /// @sa PutResult, GetJob
    bool PutResultGetJob(const CNetScheduleJob& done_job,
        CNetScheduleJob& new_job) const;

    /// Put job interim (progress) message
    ///
    /// @param job
    ///     NetSchedule job description structure. its progerss_msg
    ///     field should be set
    ///
    /// @sa GetProgressMsg
    ///
    void PutProgressMsg(const CNetScheduleJob& job) const;

    /// Get progress message
    ///
    /// @param job
    ///    NetSchedule job description structure. The message is taken from
    ///    progress_msg filed
    ///
    /// @sa PutProgressMsg
    ///
    void GetProgressMsg(CNetScheduleJob& job);


    /// Submit job failure diagnostics. This method indicates that
    /// job failed because of some fatal, unrecoverable error.
    ///
    /// @param job
    ///     NetSchedule job description structure. its error_msg
    ///     and optionally ret_code and output fields should be set
    ///
    void PutFailure(const CNetScheduleJob& job) const;

    /// Request of current job status
    /// eJobNotFound is returned if job status cannot be found
    /// (job record timed out)
    ///
    CNetScheduleAPI::EJobStatus GetJobStatus(const string& job_key);

    /// Transfer job to the "Returned" status. It will be
    /// re-executed after a while.
    ///
    /// Node may decide to return the job if it cannot process it right
    /// now (does not have resources, being asked to shutdown, etc.)
    ///
    void ReturnJob(const string& job_key) const;

    /// Set job execution timeout
    ///
    /// When node picks up the job for execution it may evaluate what time it
    /// takes for computation and report it to the queue. If job does not
    /// finish in the specified time frame (because of a failure)
    /// it is going to be rescheduled
    ///
    /// Default value for the run timeout specified in the queue settings on
    /// the server side.
    ///
    /// @param time_to_run
    ///    Time in seconds to finish the job. 0 means "queue default value".
    ///
    void SetRunTimeout(const string& job_key, unsigned time_to_run) const;

    /// Increment job execution timeout
    ///
    /// When node picks up the job for execution it may periodically
    /// communicate to the server that job is still alive and
    /// prolong job execution timeout, so job server does not try to
    /// reschedule.
    ///
    /// @param runtime_inc
    ///    Estimated time in seconds(from the current moment) to
    ///    finish the job.
    ///
    /// @sa SetRunTimeout
    void JobDelayExpiration(const string& job_key, unsigned runtime_inc) const;


    /// Register client-listener
    void RegisterClient(unsigned short udp_port) const;

    const CNetScheduleAPI::SServerParams& GetServerParams() const;


    /// Unregister client-listener. After this call, the
    /// server will not try to send any notification messages or
    /// maintain job affinity for the client.
    void UnRegisterClient(unsigned short udp_port) const;

    static bool WaitNotification(const string&  queue_name,
                                 unsigned       wait_time,
                                 unsigned short udp_port);

    /// Return Queue name
    const string& GetQueueName() const;
    const string& GetClientName() const;
    const string& GetServiceName() const;
};

/////////////////////////////////////////////////////////////////////////////////////
////

struct SNetScheduleAdminImpl;

class NCBI_XCONNECT_EXPORT CNetScheduleAdmin
{
    NET_COMPONENT(NetScheduleAdmin);

    /// Status map, shows number of jobs in each status
    typedef map<CNetScheduleAPI::EJobStatus, unsigned> TStatusMap;

    /// Returns statuses for a given affinity token
    /// @param status_map
    ///    Status map (status to job count)
    /// @param affinity_token
    ///    Affinity token (optional)
    void StatusSnapshot(TStatusMap& status_map,
        const string& affinity_token) const;


    /// Delete job
    void DropJob(const string& job_key);

    enum ECreateQueueFlags {
        eIgnoreDuplicateName = 0,
        eErrorOnDublicateName
    };

    /// Create queue of given queue class
    /// @param qname
    ///    Name of the queue to create
    /// @param qclass
    ///    Parameter set described in config file in qclass_* section
    void CreateQueue(
        const string& qname,
        const string& qclass,
        const string& comment = kEmptyStr,
        ECreateQueueFlags flags = eIgnoreDuplicateName) const;

    /// Delete queue
    /// Applicable only to queues, created through CreateQueue method
    /// @param qname
    ///    Name of the queue to delete.
    void DeleteQueue(const string& qname) const;


    /// Shutdown level
    ///
    enum EShutdownLevel {
        eNoShutdown = 0,    ///< No Shutdown was requested
        eNormalShutdown,    ///< Normal shutdown was requested
        eShutdownImmediate, ///< Urgent shutdown was requested
        eDie                ///< A serious error occurred, the server shuts down
    };

    /// Shutdown the server daemon.
    ///
    void ShutdownServer(EShutdownLevel level = eNormalShutdown) const;

    /// Kill all jobs in the queue.
    ///
    void DropQueue() const;

    void DumpJob(CNcbiOstream& out, const string& job_key) const;

    /// Reschedule a job
    ///
    void ForceReschedule(const string& job_key) const;

    /// Turn server-side logging on(off)
    ///
    void Logging(bool on_off) const;

    void ReloadServerConfig() const;

    //////////////////////////////////////////////////////
    /// Print version string
    void PrintServerVersion(CNcbiOstream& output_stream) const;

    struct SWorkerNodeInfo {
        string name;
        string prog;
        string host;
        unsigned short port;
        CTime last_access;
    };

    void GetWorkerNodes(std::list<SWorkerNodeInfo>& worker_nodes) const;

    enum EStatisticsOptions
    {
        eStatisticsAll,
        eStatisticsBrief
    };

    void PrintServerStatistics(CNcbiOstream& output_stream,
        EStatisticsOptions opt = eStatisticsBrief) const;

    void DumpQueue(CNcbiOstream& output_stream) const;

    void PrintQueue(CNcbiOstream& output_stream,
        CNetScheduleAPI::EJobStatus status) const;

    unsigned CountActiveJobs();

    void Monitor(CNcbiOstream& out) const;

    struct SServerQueueList {
        CNetServer server;
        std::list<std::string> queues;

        SServerQueueList(SNetServerImpl* server_impl) : server(server_impl) {}
    };

    typedef std::list<SServerQueueList> TQueueList;

    void GetQueueList(TQueueList& result) const;

    /// Query by tags
    unsigned long Count(const string& query) const;
    void Query(const string& query, const vector<string>& fields, CNcbiOstream& os) const;
    void Select(const string& select_stmt, CNcbiOstream& os) const;

    void RetrieveKeys(const string& query, CNetScheduleKeys& ids) const;
};


NCBI_DECLARE_INTERFACE_VERSION(SNetScheduleAPIImpl, "xnetschedule_api", 1,0, 0);


/// @internal
extern NCBI_XCONNECT_EXPORT const char* kNetScheduleAPIDriverName;


void NCBI_XCONNECT_EXPORT NCBI_EntryPoint_xnetscheduleapi(
     CPluginManager<SNetScheduleAPIImpl>::TDriverInfoList&   info_list,
     CPluginManager<SNetScheduleAPIImpl>::EEntryPointRequest method);


/* @} */


END_NCBI_SCOPE


#endif  /* CONN___NETSCHEDULE_API__HPP */
