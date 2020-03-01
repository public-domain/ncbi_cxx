/* $Id: data4xmlformat.hpp 151315 2009-02-03 18:13:26Z camacho $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's offical duties as a United States Government employee and
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
* Authors: Jason Papadopoulos, Christiam Camacho
* 
*/

/** @file data4xmlformat.hpp
 * Implementation of interface class to produce data required for generating
 * BLAST XML output
 */

#ifndef APP___DATA4XMLFORMAT__HPP
#define APP___DATA4XMLFORMAT__HPP

#include <algo/blast/api/blast_aux.hpp>
#include <algo/blast/api/sseqloc.hpp>
#include <algo/blast/api/setup_factory.hpp>
#include <algo/blast/api/uniform_search.hpp>
#include <objtools/blast_format/blastfmtutil.hpp>
#include <objtools/blast_format/blastxml_format.hpp>

BEGIN_NCBI_SCOPE

/// Strategy class to gather the data for generating BLAST XML output
class CCmdLineBlastXMLReportData : public IBlastXMLReportData
{
public:
    /// Constructor
    /// @param queries Query sequences [in]
    /// @param results results set containing one query per element or one
    /// iteration per element in the case of PSI-BLAST [in]
    /// @param opts Blast options container [in]
    /// @param dbname Name of database to search ("" if none) [in]
    /// @param db_is_aa true if database contains protein sequences [in]
    /// @param qgencode Genetic code used to translate query sequences
    ///                 (if applicable) [in]
    /// @param dbgencode Genetic code used to translate database sequences
    ///                 (if applicable) [in]
    ///
    CCmdLineBlastXMLReportData(CRef<blast::CBlastQueryVector> queries,
               const blast::CSearchResultSet& results,
               const blast::CBlastOptions& opts, 
               const string& dbname, bool db_is_aa,
               int qgencode = BLAST_GENETIC_CODE,
               int dbgencode = BLAST_GENETIC_CODE,
               bool is_remote = false,
               const vector<int>& dbfilt_algorithms = vector<int>());

    /// Destructor
    ~CCmdLineBlastXMLReportData();

    //------------ callbacks needed by IBlastXMLReportData ---------

    /// @inheritDoc
    string GetBlastProgramName(void) const {
        return blast::Blast_ProgramNameFromType(m_Options.GetProgramType());
    }

    /// @inheritDoc
    blast::EProgram GetBlastTask(void) const {
        return m_Options.GetProgram();
    }

    /// @inheritDoc
    string GetDatabaseName(void) const { return m_DbName; }

    /// @inheritDoc
    double GetEvalueThreshold(void) const { 
        return m_Options.GetEvalueThreshold();
    }

    /// @inheritDoc
    int GetGapOpeningCost(void) const {
        return m_Options.GetGapOpeningCost();
    }

    /// @inheritDoc
    int GetGapExtensionCost(void) const {
        return m_Options.GetGapExtensionCost();
    }

    /// @inheritDoc
    int GetMatchReward(void) const {
        return m_Options.GetMatchReward();
    }

    /// @inheritDoc
    int GetMismatchPenalty(void) const {
        return m_Options.GetMismatchPenalty();
    }

    /// @inheritDoc
    string GetPHIPattern(void) const {
        const char *tmp = m_Options.GetPHIPattern();
        return tmp == NULL ? string() : string(tmp);
    }

    /// @inheritDoc
    string GetFilterString(void) const {
        blast::TAutoCharPtr tmp = 
            m_Options.GetFilterString(); /* NCBI_FAKE_WARNING */
        return tmp.get() == NULL ? NcbiEmptyString : string(tmp.get());
    }

    /// @inheritDoc
    string GetMatrixName(void) const {
        const char *tmp = m_Options.GetMatrixName();
        return tmp == NULL ? string() : string(tmp);
    }

    /// @inheritDoc
    CBlastFormattingMatrix* GetMatrix(void) const {
        return new CBlastFormattingMatrix((int **)m_Matrix,
                                           kMatrixCols, kMatrixCols); 
    }

    /// @inheritDoc
    unsigned int GetNumQueries(void) const { return m_Queries->Size(); }

    /// @inheritDoc
    const blast::TMaskedQueryRegions* 
        GetMaskLocations(int query_index) const {
        if (m_NoHitsFound || query_index >= (int)m_Masks.size()) {
            return NULL;
        }
        return &m_Masks[query_index];
    }

    /// @inheritDoc
    int GetDbNumSeqs(void) const {
        return m_DbInfo.front().number_seqs;
    }

    /// @inheritDoc
    Int8 GetDbLength(void) const {
        return m_DbInfo.front().total_length;
    }

    /// @inheritDoc
    int GetLengthAdjustment(int /*query_index*/) const {
        /// @todo FIXME no way to retrieve this
        /// given the data available
        return 0;
    }

    /// @inheritDoc
    Int8 GetEffectiveSearchSpace(int query_index) const {
        if (m_NoHitsFound || query_index >= (int)m_AncillaryData.size()) {
            return 0;
        }
        return m_AncillaryData[query_index]->GetSearchSpace();
    }

    /// @inheritDoc
    double GetLambda(int query_index) const;

    /// @inheritDoc
    double GetKappa(int query_index) const;

    /// @inheritDoc
    double GetEntropy(int query_index) const;

    /// @inheritDoc
    const objects::CSeq_loc* GetQuery(int query_index) const {
        return m_Queries->GetQuerySeqLoc(query_index);
    }

    /// @inheritDoc
    objects::CScope* GetScope(int query_index) const {
        return m_Queries->GetScope(query_index);
    }

    /// @inheritDoc
    const CSeq_align_set* GetAlignment(int query_index) const {
        if (m_NoHitsFound || query_index >= (int)m_Alignments.size()) {
            return NULL;
        }
        return m_Alignments[query_index].GetPointer();
    }

    /// @inheritDoc
    bool GetGappedMode(void) const {
        return m_Options.GetGappedMode();
    }

    /// @inheritDoc
    int GetMasterGeneticCode() const { return m_QueryGeneticCode; }

    /// @inheritDoc
    int GetSlaveGeneticCode() const { return m_DbGeneticCode; }

    /// @inheritDoc
    vector<string> GetMessages() const { return m_Errors; }

private:
    /// Query sequences
    CRef<blast::CBlastQueryVector> m_Queries;
    /// BLAST algorithm options
    const blast::CBlastOptions& m_Options;
    string m_DbName;            ///< name of blast database
    /// genetic code for the query
    int m_QueryGeneticCode;
    /// genetic code for the database
    int m_DbGeneticCode;

    /// ancillary results data
    vector<CRef<blast::CBlastAncillaryData> > m_AncillaryData;
    /// the alignments
    vector<CConstRef<CSeq_align_set> > m_Alignments;
    /// masks for the queries
    blast::TSeqLocInfoVector m_Masks;
    /// True if results did not find any hits
    bool m_NoHitsFound;
    /// Error messages (one element per query)
    vector<string> m_Errors;

    /// Number of columns used in score matrices
    static const unsigned int kMatrixCols = 28;

    /// Score matrix used to determine neighboring protein residues
    int *m_Matrix[kMatrixCols];

    /// internal representation of database information
    vector<CBlastFormatUtil::SDbInfo> m_DbInfo;

    /// Initialize the score matrix to be used for formatting
    /// (if applicable)
    /// @param matrix_name Name of score matrix. NULL defaults to
    ///                    BLOSUM62 [in]
    ///
    void x_FillScoreMatrix(const char *matrix_name = BLAST_DEFAULT_MATRIX);
};

END_NCBI_SCOPE

#endif /* !APP___DATA4XMLFORMAT__HPP */

