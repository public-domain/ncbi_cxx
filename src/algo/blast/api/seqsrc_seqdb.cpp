/*  $Id: seqsrc_seqdb.cpp 151315 2009-02-03 18:13:26Z camacho $
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
* Author:  Ilya Dondoshansky
*
*/

/// @file seqsrc_seqdb.cpp
/// Implementation of the BlastSeqSrc interface for a C++ BLAST databases API

#include <ncbi_pch.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <algo/blast/api/seqsrc_seqdb.hpp>
#include <algo/blast/core/blast_util.h>
#include <algo/blast/core/blast_seqsrc_impl.h>
#include <objtools/blast/seqdb_reader/seqdbexpert.hpp>
#include "blast_setup.hpp"

/** @addtogroup AlgoBlast
 *
 * @{
 */

#ifndef SKIP_DOXYGEN_PROCESSING
USING_NCBI_SCOPE;
USING_SCOPE(objects);
USING_SCOPE(blast);
#endif /* SKIP_DOXYGEN_PROCESSING */

/// Simple container to support SeqSrc-local data.
struct SSeqDB_SeqSrc_Data {
    /// Constructor.
    SSeqDB_SeqSrc_Data()
    {
    }
    
    /// Constructor.
    SSeqDB_SeqSrc_Data(CSeqDB * ptr, const vector<int> & algo_ids)
        : seqdb((CSeqDBExpert*) ptr), algorithm_ids(algo_ids)
    {
    }
    
    /// Make a copy of this object, sharing the same SeqDB object.
    SSeqDB_SeqSrc_Data * clone()
    {
        return new SSeqDB_SeqSrc_Data(&* seqdb, algorithm_ids);
    }
    
    /// Convenience to allow datap->method to use SeqDB methods.
    CSeqDBExpert * operator->()
    {
        _ASSERT(! seqdb.Empty());
        return &*seqdb;
    }
    
    /// Convenience to allow datap->method to use SeqDB methods.
    CSeqDBExpert & operator*()
    {
        _ASSERT(! seqdb.Empty());
        return *seqdb;
    }
    
    /// SeqDB object.
    CRef<CSeqDBExpert> seqdb;
    
    /// Algorithm IDs for mask data fetching.
    vector<int> algorithm_ids;
    
#if ((!defined(NCBI_COMPILER_WORKSHOP) || (NCBI_COMPILER_VERSION  > 550)) && \
     (!defined(NCBI_COMPILER_MIPSPRO)) )
    /// Ranges of the sequence to include (for masking).
    CSeqDB::TSequenceRanges seq_ranges;
#endif
};

typedef SSeqDB_SeqSrc_Data TSeqDBData;

extern "C" {

#ifdef KAPPA_PRINT_DIAGNOSTICS

static Blast_GiList*
s_SeqDbGetGiList(void* seqdb_handle, void* args)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    
    Int4* oid = (Int4*) args;
    
    if (!datap || !oid)
       return NULL;

    vector<int> gis;
    datap->GetGis(*oid, gis);

    Blast_GiList* retval = Blast_GiListNewEx(gis.size());
    copy(gis.begin(), gis.end(), retval->data);
    retval->num_used = gis.size();

    return retval;
}

#endif /* KAPPA_PRINT_DIAGNOSTICS */

/// Retrieves the length of the longest sequence in the BlastSeqSrc.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static Int4 
s_SeqDbGetMaxLength(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetMaxLength();
}

/// Retrieves the number of sequences in the BlastSeqSrc.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static Int4 
s_SeqDbGetNumSeqs(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetNumSeqs();
}

/// Retrieves the number of sequences from alias file to be used for
//  search-space calculations.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static Int4 
s_SeqDbGetNumSeqsStats(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetNumSeqsStats();
}

/// Retrieves the total length of all sequences in the BlastSeqSrc.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static Int8 
s_SeqDbGetTotLen(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetTotalLength();
}

/// Retrieves the total length of all sequences from alias file
// to be used for search space calculations.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static Int8 
s_SeqDbGetTotLenStats(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetTotalLengthStats();  
}

/// Retrieves the average length of sequences in the BlastSeqSrc.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
/// @param ignoreme Unused by this implementation [in]
static Int4 
s_SeqDbGetAvgLength(void* seqdb_handle, void* ignoreme)
{
   Int8 total_length = s_SeqDbGetTotLen(seqdb_handle, ignoreme);
   Int4 num_seqs = MAX(1, s_SeqDbGetNumSeqs(seqdb_handle, ignoreme));

   return (Int4) (total_length/num_seqs);
}

/// Retrieves the name of the BLAST database.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
static const char* 
s_SeqDbGetName(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetDBNameList().c_str();
}

/// Checks whether database is protein or nucleotide.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
/// @return TRUE if database is protein, FALSE if nucleotide.
static Boolean 
s_SeqDbGetIsProt(void* seqdb_handle, void*)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;

    return (seqdb.GetSequenceType() == CSeqDB::eProtein);
}

/// Retrieves the sequence meeting the criteria defined by its second argument.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
/// @param args Pointer to BlastSeqSrcGetSeqArg structure [in]
/// @return return codes defined in blast_seqsrc.h
static Int2 
s_SeqDbGetSequence(void* seqdb_handle, BlastSeqSrcGetSeqArg* args)
{
    Int4 oid = -1, len = 0;
    Boolean has_sentinel_byte;
    Boolean buffer_allocated;
    
    if (!seqdb_handle || !args)
        return BLAST_SEQSRC_ERROR;
    
    TSeqDBData * datap = (TSeqDBData *) seqdb_handle;
    
    const vector<int> & filtering_algorithms = datap->algorithm_ids;
    CSeqDBExpert & seqdb = **datap;
    
    oid = args->oid;
    
    // If we are asked to check for OID exclusion, and if the database
    // has a GI list, then we check whether all the seqids have been
    // removed by filtering.  If so we return an error.  The traceback
    // code will exclude this HSP list.
    
    if (args->check_oid_exclusion) {
        if (! seqdb.GetIdSet().Blank()) {
            list< CRef<CSeq_id> > seqids = seqdb.GetSeqIDs(oid);
            
            if (seqids.empty()) {
                return BLAST_SEQSRC_ERROR;
            }
        }
    }
    
    EBlastEncoding encoding = args->encoding;
    has_sentinel_byte = (encoding == eBlastEncodingNucleotide);
    
    buffer_allocated = 
       (encoding == eBlastEncodingNucleotide || encoding == eBlastEncodingNcbi4na);
    
    /* free buffers if necessary */
    if (args->seq)
        BlastSequenceBlkClean(args->seq);
    
    /* If nucleotide and ranges are disabled, remove any range list
       stored in the SeqDB object.  Protein databases ignore ranges in
       any case. */
    
    if (! args->enable_ranges) {
        if (seqdb.GetSequenceType() == CSeqDB::eNucleotide) {
            CSeqDBExpert::TRangeList none;
            seqdb.SetOffsetRanges(oid, none, false, false);
        }
    }
    
    const char *buf;
    if (!buffer_allocated) {
        len = seqdb.GetSequence(oid, &buf);
    } else {
        len = seqdb.GetAmbigSeq(oid, &buf, has_sentinel_byte);
    }
    
    if (len <= 0)
        return BLAST_SEQSRC_ERROR;
    
    BlastSetUp_SeqBlkNew((Uint1*)buf, len, &args->seq, 
                         buffer_allocated);
    
    /* If there is no sentinel byte, and buffer is allocated, i.e. this is
       the traceback stage of a translated search, set "sequence" to the same 
       position as "sequence_start". */
    if (buffer_allocated && !has_sentinel_byte)
       args->seq->sequence = args->seq->sequence_start;
    
    /* For preliminary stage, even though sequence buffer points to a memory
       mapped location, we still need to call ReleaseSequence. This can only be
       guaranteed by making the engine believe tat sequence is allocated.
    */
    if (!buffer_allocated)
        args->seq->sequence_allocated = TRUE;
    
    args->seq->oid = oid;

    // Test should be whether the list of algorithm ids is null or not
    
#if ((!defined(NCBI_COMPILER_WORKSHOP) || (NCBI_COMPILER_VERSION  > 550)) && \
     (!defined(NCBI_COMPILER_MIPSPRO)) )
    if ( !filtering_algorithms.empty() ) {
        static const Boolean kCopySequenceRanges = false;
        CSeqDB::TSequenceRanges & ranges = datap->seq_ranges;
        seqdb.GetMaskData(oid, filtering_algorithms, true, ranges);
        _ASSERT( !ranges.empty() );
        if (BlastSeqBlkSetSeqRanges(args->seq, 
                                    (SSeqRange*)& ranges[0],
                                    ranges.size(), kCopySequenceRanges) != 0) {
            return BLAST_SEQSRC_ERROR;
        }
    }
#endif
    
    return BLAST_SEQSRC_SUCCESS;
}

/// Returns the memory allocated for the sequence buffer to the CSeqDB 
/// interface.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
/// @param args Pointer to the BlastSeqSrcGetSeqArgs structure, 
/// containing sequence block with the buffer that needs to be deallocated. [in]
static void
s_SeqDbReleaseSequence(void* seqdb_handle, BlastSeqSrcGetSeqArg* args)
{
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;

    _ASSERT(seqdb_handle);
    _ASSERT(args);
    _ASSERT(args->seq);

    if (args->seq->sequence_start_allocated) {
        seqdb.RetSequence((const char**)&args->seq->sequence_start);
        args->seq->sequence_start_allocated = FALSE;
        args->seq->sequence_start = NULL;
    }
    if (args->seq->sequence_allocated) {
        seqdb.RetSequence((const char**)&args->seq->sequence);
        args->seq->sequence_allocated = FALSE;
        args->seq->sequence = NULL;
    }
}

/// Retrieve length of a given database sequence.
/// @param seqdb_handle Pointer to initialized CSeqDB object [in]
/// @param args Pointer to integer indicating ordinal id [in]
/// @return Length of the database sequence or BLAST_SEQSRC_ERROR.
static Int4 
s_SeqDbGetSeqLen(void* seqdb_handle, void* args)
{
    Int4* oid = (Int4*) args;

    if (!seqdb_handle || !oid)
       return BLAST_SEQSRC_ERROR;

    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    return seqdb.GetSeqLength(*oid);
}

/// Assigns next chunk of the database to the sequence source iterator.
/// @param seqdb_handle Reference to the database object, cast to void* to 
///                     satisfy the signature requirement. [in]
/// @param itr Iterator over the database sequence source. [in|out]
static Int2 
s_SeqDbGetNextChunk(void* seqdb_handle, BlastSeqSrcIterator* itr)
{
    if (!seqdb_handle || !itr)
        return BLAST_SEQSRC_ERROR;
    
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    
    vector<int> oid_list(itr->chunk_sz);

    CSeqDB::EOidListType chunk_type = 
        seqdb.GetNextOIDChunk(itr->oid_range[0], itr->oid_range[1], 
                                  oid_list);
    
    if (chunk_type == CSeqDB::eOidRange) {
        if (itr->oid_range[1] <= itr->oid_range[0])
            return BLAST_SEQSRC_EOF;
        itr->itr_type = eOidRange;
        itr->current_pos = itr->oid_range[0];
    } else if (chunk_type == CSeqDB::eOidList) {
        itr->itr_type = eOidList;
        itr->chunk_sz = (Uint4) oid_list.size();
        if (itr->chunk_sz == 0)
            return BLAST_SEQSRC_EOF;
        itr->current_pos = 0;
        Uint4 index;
        for (index = 0; index < itr->chunk_sz; ++index)
            itr->oid_list[index] = oid_list[index];
    }

    return BLAST_SEQSRC_SUCCESS;
}

/// Finds the next not searched ordinal id in the iteration over BLAST database.
/// @param seqdb_handle Reference to the database object, cast to void* to 
///                     satisfy the signature requirement. [in]
/// @param itr Iterator of the BlastSeqSrc pointed by ptr. [in]
/// @return Next ordinal id.
static Int4 
s_SeqDbIteratorNext(void* seqdb_handle, BlastSeqSrcIterator* itr)
{
    Int4 retval = BLAST_SEQSRC_EOF;
    Int4 status = BLAST_SEQSRC_SUCCESS;

    _ASSERT(seqdb_handle);
    _ASSERT(itr);

    /* If internal iterator is uninitialized/invalid, retrieve the next chunk 
       from the BlastSeqSrc */
    if (itr->current_pos == UINT4_MAX) {
        status = s_SeqDbGetNextChunk(seqdb_handle, itr);
        if (status == BLAST_SEQSRC_ERROR || status == BLAST_SEQSRC_EOF) {
            return status;
        }
    }

    Uint4 last_pos = 0;

    if (itr->itr_type == eOidRange) {
        retval = itr->current_pos;
        last_pos = itr->oid_range[1];
    } else if (itr->itr_type == eOidList) {
        retval = itr->oid_list[itr->current_pos];
        last_pos = itr->chunk_sz;
    } else {
        /* Unsupported/invalid iterator type! */
        fprintf(stderr, "Invalid iterator type: %d\n", itr->itr_type);
        abort();
    }

    ++itr->current_pos;
    if (itr->current_pos >= last_pos) {
        itr->current_pos = UINT4_MAX;  /* invalidate internal iteration */
    }

    return retval;
}

/// Resets CSeqDB's internal chunk bookmark
/// @param seqdb_handle Reference to the database object, cast to void* to 
///                     satisfy the signature requirement. [in]
static void
s_SeqDbResetChunkIterator(void* seqdb_handle)
{
    _ASSERT(seqdb_handle);
    CSeqDB & seqdb = **(TSeqDBData *) seqdb_handle;
    seqdb.ResetInternalChunkBookmark();
}

}

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
BEGIN_SCOPE(blast)

/// Encapsulates the arguments needed to initialize CSeqDB.
class CSeqDbSrcNewArgs {
public:
    /// Constructor
    CSeqDbSrcNewArgs(const string& db, bool is_prot,
                     Uint4 first_oid = 0, Uint4 final_oid = 0,
                     const vector<int>& filtering_algorithms = vector<int>())
        : m_DbName(db), m_IsProtein(is_prot), 
          m_FirstDbSeq(first_oid), m_FinalDbSeq(final_oid),
          m_FilteringAlgorithms(filtering_algorithms)
    {}

    /// Getter functions for the private fields
    const string GetDbName() const { return m_DbName; }
    /// Returns database type: protein or nucleotide
    char GetDbType() const { return m_IsProtein ? 'p' : 'n'; }
    /// Returns first database ordinal id covered by this BlastSeqSrc
    Uint4 GetFirstOid() const { return m_FirstDbSeq; }
    /// Returns last database ordinal id covered by this BlastSeqSrc
    Uint4 GetFinalOid() const { return m_FinalDbSeq; }
    /// Returns the default filtering algorithms to use with sequence data
    /// extracted from this BlastSeqSrc
    vector<int> GetFilteringAlgorithms() const { return m_FilteringAlgorithms; }

private:
    string m_DbName;        ///< Database name
    bool m_IsProtein;       ///< Is this database protein?
    Uint4 m_FirstDbSeq;     ///< Ordinal id of the first sequence to search
    Uint4 m_FinalDbSeq;     ///< Ordinal id of the last sequence to search
    /// List of filtering algorithms to use when retrieving sequence data
    vector<int> m_FilteringAlgorithms;
};

extern "C" {

/// SeqDb sequence source destructor: frees its internal data structure
/// @param seq_src BlastSeqSrc structure to free [in]
/// @return NULL
static BlastSeqSrc* 
s_SeqDbSrcFree(BlastSeqSrc* seq_src)
{
    if (!seq_src) 
        return NULL;
    
    TSeqDBData * datap = static_cast<TSeqDBData*>
        (_BlastSeqSrcImpl_GetDataStructure(seq_src));
    
    delete datap;
    return NULL;
}

/// SeqDb sequence source copier: creates a new reference to the CSeqDB object
/// and copies the rest of the BlastSeqSrc structure.
/// @param seq_src BlastSeqSrc structure to copy [in]
/// @return Pointer to the new BlastSeqSrc.
static BlastSeqSrc* 
s_SeqDbSrcCopy(BlastSeqSrc* seq_src)
{
    if (!seq_src) 
        return NULL;
    
    TSeqDBData * datap = static_cast<TSeqDBData*>
        (_BlastSeqSrcImpl_GetDataStructure(seq_src));
    
    _BlastSeqSrcImpl_SetDataStructure(seq_src, (void*) datap->clone());
    
    return seq_src;
}

/// Initializes the data structure and function pointers in a SeqDb based 
/// BlastSeqSrc.
/// @param retval Structure to populate [in] [out]
/// @param seqdb Reference to a CSeqDB object [in]
static void 
s_InitNewSeqDbSrc(BlastSeqSrc* retval, TSeqDBData * datap)
{
    _ASSERT(retval);
    _ASSERT(datap);
    
    /* Initialize the BlastSeqSrc structure fields with user-defined function
     * pointers and seqdb */
    _BlastSeqSrcImpl_SetDeleteFnPtr   (retval, & s_SeqDbSrcFree);
    _BlastSeqSrcImpl_SetCopyFnPtr     (retval, & s_SeqDbSrcCopy);
    _BlastSeqSrcImpl_SetDataStructure (retval, (void*) datap);
    _BlastSeqSrcImpl_SetGetNumSeqs    (retval, & s_SeqDbGetNumSeqs);
    _BlastSeqSrcImpl_SetGetNumSeqsStats(retval, & s_SeqDbGetNumSeqsStats);
    _BlastSeqSrcImpl_SetGetMaxSeqLen  (retval, & s_SeqDbGetMaxLength);
    _BlastSeqSrcImpl_SetGetAvgSeqLen  (retval, & s_SeqDbGetAvgLength);
    _BlastSeqSrcImpl_SetGetTotLen     (retval, & s_SeqDbGetTotLen);
    _BlastSeqSrcImpl_SetGetTotLenStats(retval, & s_SeqDbGetTotLenStats);
    _BlastSeqSrcImpl_SetGetName       (retval, & s_SeqDbGetName);
    _BlastSeqSrcImpl_SetGetIsProt     (retval, & s_SeqDbGetIsProt);
    _BlastSeqSrcImpl_SetGetSequence   (retval, & s_SeqDbGetSequence);
    _BlastSeqSrcImpl_SetGetSeqLen     (retval, & s_SeqDbGetSeqLen);
    _BlastSeqSrcImpl_SetIterNext      (retval, & s_SeqDbIteratorNext);
    _BlastSeqSrcImpl_SetResetChunkIterator(retval, & s_SeqDbResetChunkIterator);
    _BlastSeqSrcImpl_SetReleaseSequence   (retval, & s_SeqDbReleaseSequence);
#ifdef KAPPA_PRINT_DIAGNOSTICS
    _BlastSeqSrcImpl_SetGetGis        (retval, & s_SeqDbGetGiList);
#endif /* KAPPA_PRINT_DIAGNOSTICS */
}

/// Populates a BlastSeqSrc, creating a new reference to the already existing 
/// SeqDb object.
/// @param retval Original BlastSeqSrc [in]
/// @param args Pointer to a reference to CSeqDB object [in]
/// @return retval
static BlastSeqSrc* 
s_SeqDbSrcSharedNew(BlastSeqSrc* retval, void* args)
{
    _ASSERT(retval);
    _ASSERT(args);
    
    TSeqDBData * datap = (TSeqDBData *) args;
    
    s_InitNewSeqDbSrc(retval, datap->clone());
    
    return retval;
}

/// SeqDb sequence source constructor 
/// @param retval BlastSeqSrc structure (already allocated) to populate [in]
/// @param args Pointer to internal CSeqDbSrcNewArgs structure (@sa
/// CSeqDbSrcNewArgs) [in]
/// @return Updated seq_src structure (with all function pointers initialized
static BlastSeqSrc* 
s_SeqDbSrcNew(BlastSeqSrc* retval, void* args)
{
    _ASSERT(retval);
    _ASSERT(args);
    
    CSeqDbSrcNewArgs* seqdb_args = (CSeqDbSrcNewArgs*) args;
    _ASSERT(seqdb_args);
    
    TSeqDBData * datap = new TSeqDBData;
    
    try {
        bool is_protein = (seqdb_args->GetDbType() == 'p');
        
        datap->seqdb.Reset(new CSeqDBExpert(seqdb_args->GetDbName(),
                                            (is_protein
                                             ? CSeqDB::eProtein
                                             : CSeqDB::eNucleotide)));
        
        datap->seqdb->SetIterationRange(seqdb_args->GetFirstOid(),
                                        seqdb_args->GetFinalOid());
        
        datap->algorithm_ids = seqdb_args->GetFilteringAlgorithms();
    } catch (const ncbi::CException& e) {
        _BlastSeqSrcImpl_SetInitErrorStr(retval, 
                        strdup(e.ReportThis(eDPF_ErrCodeExplanation).c_str()));
    } catch (const std::exception& e) {
        _BlastSeqSrcImpl_SetInitErrorStr(retval, strdup(e.what()));
    } catch (...) {
        _BlastSeqSrcImpl_SetInitErrorStr(retval, 
             strdup("Caught unknown exception from CSeqDB constructor"));
    }
    
    /* Initialize the BlastSeqSrc structure fields with user-defined function
     * pointers and seqdb */
    
    s_InitNewSeqDbSrc(retval, datap);
    
    return retval;
}

}

BlastSeqSrc* 
SeqDbBlastSeqSrcInit(const string& dbname, bool is_prot, 
                 Uint4 first_seq, Uint4 last_seq,
                 const vector<int>& filtering_algorithms /* = vector<int>() */)
{
    BlastSeqSrcNewInfo bssn_info;
    BlastSeqSrc* seq_src = NULL;
    CSeqDbSrcNewArgs seqdb_args(dbname, is_prot, first_seq, last_seq,
                                filtering_algorithms);

    bssn_info.constructor = &s_SeqDbSrcNew;
    bssn_info.ctor_argument = (void*) &seqdb_args;
    seq_src = BlastSeqSrcNew(&bssn_info);
    return seq_src;
}

BlastSeqSrc* 
SeqDbBlastSeqSrcInit(CSeqDB * seqdb,
                 const vector<int>& filtering_algorithms /* = vector<int>() */)
{
    BlastSeqSrcNewInfo bssn_info;
    BlastSeqSrc * seq_src = NULL;

    TSeqDBData data(seqdb, filtering_algorithms);

    bssn_info.constructor = & s_SeqDbSrcSharedNew;
    bssn_info.ctor_argument = (void*) & data;
    seq_src = BlastSeqSrcNew(& bssn_info);
    return seq_src;
}


END_SCOPE(blast)
END_NCBI_SCOPE

/* @} */
