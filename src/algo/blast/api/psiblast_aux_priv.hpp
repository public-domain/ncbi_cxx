/*  $Id: psiblast_aux_priv.hpp 124526 2008-04-15 15:27:44Z camacho $
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
 * Author:  Christiam Camacho
 *
 */

/** @file psiblast_aux_priv.hpp
 * Declarations of auxiliary functions/classes for PSI-BLAST
 */

#ifndef ALGO_BLAST_API___PSIBLAST_AUX_PRIV__HPP
#define ALGO_BLAST_API___PSIBLAST_AUX_PRIV__HPP

#include <corelib/ncbiobj.hpp>
#include <objects/seqalign/Dense_seg.hpp>

/** @addtogroup AlgoBlast
 *
 * @{
 */

struct BlastScoreBlk;
struct PSIBlastOptions;

BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
// Forward declarations

template <class T> class CNcbiMatrix;

BEGIN_SCOPE(objects)
    class CSeq_id;
    class CBioseq;
    class CSeq_align_set;
    class CPssmWithParameters;
END_SCOPE(objects)

BEGIN_SCOPE(blast)

class CBlastOptions;
class CBlastOptionsHandle;
class IQueryFactory;
class TSearchMessages;

/////////////////////////////////////////////////////////////////////////////
// Function prototypes/Class definitions

/** Setup CORE BLAST score block structure with data from the scoremat PSSM.
 * @note this function might modify the composition based statistics settings.
 * @param score_blk BlastScoreBlk structure to set up [in|out]
 * @param pssm scoremat PSSM [in]
 * @param messages Error/warning messages [in|out]
 * @param options PSI-BLAST options [in|out]
 */
void PsiBlastSetupScoreBlock(BlastScoreBlk* score_blk,
                             CConstRef<objects::CPssmWithParameters> pssm,
                             TSearchMessages& messages,
                             CConstRef<CBlastOptions> options);

/** Given a PSSM with frequency ratios and options, invoke the PSSM engine to
 * compute the scores.
 * @param pssm object containing the PSSM's frequency ratios [in|out]
 * @param opts PSSM engine options [in]
 */
void PsiBlastComputePssmScores(CRef<objects::CPssmWithParameters> pssm,
                               const CBlastOptions& opts);

/// Returns the lowest score from the list of scores in CDense_seg::TScores
/// @param scores list of scores [in]
/// @param bit_score If not NULL, returns the bit score corresponding to the
/// lowest evalue found [in|out]
double GetLowestEvalue(const objects::CDense_seg::TScores& scores,
                       double* bit_score = NULL);

/** Auxiliary class to retrieve sequence identifiers its position in the
 * alignment which are below the inclusion evalue threshold.
 */
class CPsiBlastAlignmentProcessor {
public:
    /// Container of Seq-ids for the subject sequences (hits) aligned with the
    /// query
    typedef vector< CRef<objects::CSeq_id> > THitIdentifiers;

    /// Extract all the THitId which have evalues below the inclusion threshold
    /// @param alignments 
    ///     alignments corresponding to one query sequence [in]
    /// @param evalue_inclusion_threshold
    ///     All hits in the above alignment which have evalues below this
    ///     parameter will be included in the return value [in]
    /// @param output
    ///     Return value of this method [out]
    void operator()(const objects::CSeq_align_set& alignments,
                    double evalue_inclusion_threshold,
                    THitIdentifiers& output);
};

/// Auxialiry class containing static methods to validate PSI-BLAST search
/// components
class CPsiBlastValidate {
public:

    /** Perform validation on the PSSM
     * @param pssm PSSM as specified in scoremat.asn [in]
     * @param require_scores Set to true if scores MUST be present (otherwise,
     * either scores or frequency ratios are acceptable) [in]
     * @throws CBlastException on failure when validating data
     */
    static void
    Pssm(const objects::CPssmWithParameters& pssm,
         bool require_scores = false);

    /// Enumeration to specify the different uses of the query factory
    enum EQueryFactoryType { eQFT_Query, eQFT_Subject };

    /// Function to perform sanity checks on the query factory
    static void
    QueryFactory(CRef<IQueryFactory> query_factory, 
                 const CBlastOptionsHandle& opts_handle, 
                 EQueryFactoryType query_factory_type = eQFT_Query);
};

/** Even though the query sequence and the matrix gap costs are not a
 * product of the PSSM engine, set them as they are required for the
 * PSI-BLAST (query sequence) and RPS-BLAST/formatrpsdb (gap costs)
 * @param pssm PSSM to modify [in|out]
 * @param query Query sequence which corresponds to the PSSM [in]
 * @param gap_open Gap opening cost associated with the matrix used to build
 * the PSSM [in]
 * @param gap_extend Gap extension cost associated with the matrix used to 
 * build the PSSM [in]
 */
void
PsiBlastAddAncillaryPssmData(objects::CPssmWithParameters& pssm, 
                             const objects::CBioseq& query, 
                             int gap_open, 
                             int gap_extend);

END_SCOPE(blast)
END_NCBI_SCOPE

/* @} */

#endif  /* ALGO_BLAST_API___PSIBLAST_AUX_PRIV__HPP */
