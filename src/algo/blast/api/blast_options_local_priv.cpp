#ifndef SKIP_DOXYGEN_PROCESSING
static char const rcsid[] =
    "$Id: blast_options_local_priv.cpp 119800 2008-02-14 15:33:39Z camacho $";
#endif /* SKIP_DOXYGEN_PROCESSING */

/* ===========================================================================
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
 * ===========================================================================
 */

/// @file blast_options_local_priv.cpp
/// Definition of local representation of BLAST options.

#include <ncbi_pch.hpp>
#include <algo/blast/api/blast_exception.hpp>
#include "blast_options_local_priv.hpp"

/** @addtogroup AlgoBlast
 *
 * @{
 */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
BEGIN_SCOPE(blast)

#ifndef SKIP_DOXYGEN_PROCESSING

CBlastOptionsLocal::CBlastOptionsLocal()
{
    QuerySetUpOptions* query_setup = NULL;
    BlastQuerySetUpOptionsNew(&query_setup);
    m_QueryOpts.Reset(query_setup);
    m_InitWordOpts.Reset((BlastInitialWordOptions*)calloc(1, sizeof(BlastInitialWordOptions)));
    m_LutOpts.Reset((LookupTableOptions*)calloc(1, sizeof(LookupTableOptions)));
    m_ExtnOpts.Reset((BlastExtensionOptions*)calloc(1, sizeof(BlastExtensionOptions)));
    m_HitSaveOpts.Reset((BlastHitSavingOptions*)calloc(1, sizeof(BlastHitSavingOptions)));
    m_ScoringOpts.Reset((BlastScoringOptions*)calloc(1, sizeof(BlastScoringOptions)));

    BlastEffectiveLengthsOptionsNew(&m_EffLenOpts);
    BlastDatabaseOptionsNew(&m_DbOpts);
    PSIBlastOptionsNew(&m_PSIBlastOpts);
    m_Program = eBlastNotSet;
    m_UseMBIndex = false;
    m_MBIndexLoaded = false;
}

CBlastOptionsLocal::~CBlastOptionsLocal()
{
}

void 
CBlastOptionsLocal::SetDbGeneticCode(int gc)
{
    m_DbOpts->genetic_code = gc;
}

EBlastProgramType 
CBlastOptionsLocal::GetProgramType() const
{
    return EProgramToEBlastProgramType(m_Program);
}

static void 
s_BlastMessageToException(Blast_Message** blmsg_ptr, const string& default_msg)
{
    if (!blmsg_ptr || *blmsg_ptr == NULL)
        return;

    Blast_Message* blmsg = *blmsg_ptr;
    string msg = blmsg ? blmsg->message : default_msg;

    *blmsg_ptr = Blast_MessageFree(blmsg);

    if (msg != NcbiEmptyString)
        NCBI_THROW(CBlastException, eInvalidOptions, msg);
}

bool
CBlastOptionsLocal::Validate() const
{
    Blast_Message* blmsg = NULL;

    if (BLAST_ValidateOptions(GetProgramType(), m_ExtnOpts, 
                                       m_ScoringOpts, 
                                       m_LutOpts,
                                       m_InitWordOpts,
                                       m_HitSaveOpts,
                                       &blmsg)) {
        s_BlastMessageToException(&blmsg, "Options validation failed");
        return false;
    }
    else
        // Index validation.
        if( m_UseMBIndex && 
                (m_Program != eMegablast && m_Program != eBlastn) ) {
            NCBI_THROW(CBlastException, eInvalidOptions, 
                    "Database index can be used only with contiguous megablast." );
        }

        return true;
}

void
CBlastOptionsLocal::DebugDump(CDebugDumpContext ddc, unsigned int depth) const
{
    ddc.SetFrame("CBlastOptionsLocal");
    DebugDumpValue(ddc,"m_Program", m_Program);
    m_QueryOpts.DebugDump(ddc, depth);
    m_LutOpts.DebugDump(ddc, depth);
    m_InitWordOpts.DebugDump(ddc, depth);
    m_ExtnOpts.DebugDump(ddc, depth);
    m_HitSaveOpts.DebugDump(ddc, depth);
    m_PSIBlastOpts.DebugDump(ddc, depth);
    m_DbOpts.DebugDump(ddc, depth);
    m_ScoringOpts.DebugDump(ddc, depth);
    m_EffLenOpts.DebugDump(ddc, depth);
}

inline int
x_safe_strcmp(const char* a, const char* b)
{
    if (a != b) {
        if (a != NULL && b != NULL) {
            return strcmp(a,b);
        } else {
            return 1;
        }
    }
    return 0;
}

inline int
x_safe_memcmp(const void* a, const void* b, size_t size)
{
    if (a != b) {
        if (a != NULL && b != NULL) {
            return memcmp(a, b, size);
        } else {
            return 1;
        }
    }
    return 0;
}

bool
x_QuerySetupOptions_cmp(const QuerySetUpOptions* a, const QuerySetUpOptions* b)
{
    if (x_safe_strcmp(a->filter_string, b->filter_string) != 0) {
        return false;
    }
    if (a->strand_option != b->strand_option) return false;
    if (a->genetic_code != b->genetic_code) return false;
    return true;
}

bool
x_LookupTableOptions_cmp(const LookupTableOptions* a, 
                         const LookupTableOptions* b)
{
    if (a->threshold != b->threshold) return false;
    if (a->lut_type != b->lut_type) return false;
    if (a->word_size != b->word_size) return false;
    if (a->mb_template_length != b->mb_template_length) return false;
    if (a->mb_template_type != b->mb_template_type) return false;
    if (x_safe_strcmp(a->phi_pattern, b->phi_pattern) != 0) return false;
    return true;
}

bool
x_BlastDatabaseOptions_cmp(const BlastDatabaseOptions* a,
                           const BlastDatabaseOptions* b)
{
    if (a->genetic_code != b->genetic_code) return false;
    return true;
}

bool
x_BlastScoringOptions_cmp(const BlastScoringOptions* a,
                          const BlastScoringOptions* b)
{
    if (x_safe_strcmp(a->matrix, b->matrix) != 0) return false;
    if (x_safe_strcmp(a->matrix_path, b->matrix_path) != 0) return false;
    if (a->reward != b->reward) return false;
    if (a->penalty != b->penalty) return false;
    if (a->gapped_calculation != b->gapped_calculation) return false;
    if (a->gap_open != b->gap_open) return false;
    if (a->gap_extend != b->gap_extend) return false;
    if (a->is_ooframe != b->is_ooframe) return false;
    if (a->shift_pen != b->shift_pen) return false;
    return true;
}

bool
x_BlastEffectiveLengthsOptions_cmp(const BlastEffectiveLengthsOptions* a,
                                   const BlastEffectiveLengthsOptions* b)
{
    if (a->db_length != b->db_length) return false;
    if (a->dbseq_num != b->dbseq_num) return false;
    if (a->num_searchspaces != b->num_searchspaces) return false;
    if (x_safe_memcmp((void*)a->searchsp_eff, 
                      (void*)b->searchsp_eff, 
                      min(a->num_searchspaces, b->num_searchspaces)) != 0) {
        return false;
    }
    return true;
}

bool
CBlastOptionsLocal::operator==(const CBlastOptionsLocal& rhs) const
{
    if (this == &rhs)
        return true;

    if (m_Program != rhs.m_Program)
        return false;

    if ( !x_QuerySetupOptions_cmp(m_QueryOpts, rhs.m_QueryOpts) )
        return false;

    if ( !x_LookupTableOptions_cmp(m_LutOpts, rhs.m_LutOpts) )
        return false;

    void *a, *b;

    a = static_cast<void*>( (BlastInitialWordOptions*) m_InitWordOpts);
    b = static_cast<void*>( (BlastInitialWordOptions*) rhs.m_InitWordOpts);
    if ( x_safe_memcmp(a, b, sizeof(BlastInitialWordOptions)) != 0 )
         return false;

    a = static_cast<void*>( (BlastExtensionOptions*) m_ExtnOpts);
    b = static_cast<void*>( (BlastExtensionOptions*) rhs.m_ExtnOpts);
    if ( x_safe_memcmp(a, b, sizeof(BlastExtensionOptions)) != 0 )
         return false;

    a = static_cast<void*>( (BlastHitSavingOptions*) m_HitSaveOpts);
    b = static_cast<void*>( (BlastHitSavingOptions*) rhs.m_HitSaveOpts);
    if ( x_safe_memcmp(a, b, sizeof(BlastHitSavingOptions)) != 0 )
         return false;

    a = static_cast<void*>( (PSIBlastOptions*) m_PSIBlastOpts);
    b = static_cast<void*>( (PSIBlastOptions*) rhs.m_PSIBlastOpts);
    if ( x_safe_memcmp(a, b, sizeof(PSIBlastOptions)) != 0 )
         return false;

    if ( !x_BlastDatabaseOptions_cmp(m_DbOpts, rhs.m_DbOpts) )
        return false;

    if ( !x_BlastScoringOptions_cmp(m_ScoringOpts, rhs.m_ScoringOpts) )
        return false;
    
    if ( !x_BlastEffectiveLengthsOptions_cmp(m_EffLenOpts, rhs.m_EffLenOpts) )
        return false;
    
    return true;
}

bool
CBlastOptionsLocal::operator!=(const CBlastOptionsLocal& rhs) const
{
    return !(*this== rhs);
}

#endif /* SKIP_DOXYGEN_PROCESSING */

END_SCOPE(blast)
END_NCBI_SCOPE

/* @} */
