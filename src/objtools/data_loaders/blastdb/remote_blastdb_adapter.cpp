/*  $Id: remote_blastdb_adapter.cpp 148871 2009-01-05 16:51:12Z camacho $
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
 *  Author: Christiam Camacho
 *
 * ===========================================================================
 */

/** @file remote_blastdb_adapter.cpp
 * Defines the CRemoteBlastDbAdapter class
 */
#ifndef SKIP_DOXYGEN_PROCESSING
static char const rcsid[] = "$Id: remote_blastdb_adapter.cpp 148871 2009-01-05 16:51:12Z camacho $";
#endif /* SKIP_DOXYGEN_PROCESSING */

#include <ncbi_pch.hpp>
#include "remote_blastdb_adapter.hpp"
#include <algo/blast/api/remote_blast.hpp>
#include <algo/blast/api/remote_services.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Seq_literal.hpp>

BEGIN_NCBI_SCOPE
USING_SCOPE(blast);
BEGIN_SCOPE(objects)

CRemoteBlastDbAdapter::CRemoteBlastDbAdapter(const string& db_name,
                                             CSeqDB::ESeqType db_type)
: m_DbName(db_name), m_DbType(db_type), m_NextLocalId(1) 
{
    CRemoteServices rmt_svc;
    const bool kIsProtein = (db_type == CSeqDB::eProtein) ? true : false;
    if ( !rmt_svc.IsValidBlastDb(db_name, kIsProtein) ) {
        ostrstream out;
        out << (kIsProtein ? "Protein" : "Nucleotide") <<  " BLAST database "
            << "'" << db_name << "' does not exist in the NCBI servers";
        NCBI_THROW(CSeqDBException, eArgErr, CNcbiOstrstreamToString(out));
    }
}

int 
CRemoteBlastDbAdapter::GetSeqLength(int oid)
{
    _ASSERT(m_Cache[oid].IsValid());
    return m_Cache[oid].GetLength();
}

IBlastDbAdapter::TSeqIdList
CRemoteBlastDbAdapter::GetSeqIDs(int oid)
{
    _ASSERT(m_Cache[oid].IsValid());
    return m_Cache[oid].GetIdList();
}

CRef<CBioseq> 
CRemoteBlastDbAdapter::GetBioseqNoData(int oid, int /* target_gi = 0 */)
{
    /// @todo FIXME we should do something with the target_gi
    _ASSERT(m_Cache[oid].IsValid());
    // N.B.: the assignment to a newly created bioseq is deliberate to avoid an
    // exception when loading data in the object manager
    CRef<CBioseq> retval(new CBioseq);
    retval->Assign(*m_Cache[oid].GetBioseq());
    return retval;
}

/// Returns false always. Logs an error message with severity warning for all
/// errors but sequence not found.
/// @param errors errors reported by server [in]
/// @param warnings warnings reported by server [in]
static bool 
RemoteBlastDbLoader_ErrorHandler(const string& errors, const string& warnings)
{
    const bool retval = false;
    /// FIXME ideally this would come from some error code rather than string
    /// parsing
    if (NStr::Find(errors, "Failed to fetch sequence: [") != NPOS) {
        return retval;
    }

    string msg;
    if ( !errors.empty() ) {
        msg = errors;
    }
    if ( !warnings.empty() ) {
        msg += (msg.empty() ? warnings : " " + warnings);
    }
    if (msg.empty()) {
        msg = "Failed to retrieve sequence data via remote BLAST database ";
        msg += "data loader";
    }
    ERR_POST(Warning << msg);
    return retval;
}

void CRemoteBlastDbAdapter::x_FetchData(int oid, int begin, int end)
{
    CCachedSeqDataForRemote& cached_seqdata = m_Cache[oid];
    _ASSERT( !cached_seqdata.HasSequenceData(begin, end) );
    const char seqtype = (GetSequenceType() == CSeqDB::eProtein) ? 'p' : 'n';
    
    CRef<CSeq_interval> seq_int
        (new CSeq_interval(*cached_seqdata.GetIdList().front(), begin, end));
    CRemoteBlast::TSeqIntervalVector seqids(1, seq_int);
    CRemoteBlast::TSeqIdVector ids;
    CRemoteBlast::TSeqDataVector seq_data;
    string errors, warnings;
    const bool kVerbose = (getenv("VERBOSE") ? true : false);

    CRemoteBlast::GetSequenceParts(seqids, m_DbName, seqtype, ids, seq_data,
                                   errors, warnings, kVerbose);
    if (seq_data.empty() || !errors.empty() || !warnings.empty() || 
        ids.empty() ) {
        RemoteBlastDbLoader_ErrorHandler(errors, warnings);
    }
    _ASSERT(ids.size() == seq_data.size());
    cached_seqdata.GetSeqDataChunk(begin, end) = seq_data.front();
    _ASSERT(cached_seqdata.HasSequenceData(begin, end));
}


CRef<CSeq_data> 
CRemoteBlastDbAdapter::GetSequence(int oid, 
                                   int begin /* = 0 */, 
                                   int end /* = 0*/)
{
    CCachedSeqDataForRemote& cached_seqdata = m_Cache[oid]; 
    _ASSERT(cached_seqdata.IsValid());
    if ( !cached_seqdata.HasSequenceData(begin, end) ) {
        x_FetchData(oid, begin, end);
    }
    return cached_seqdata.GetSeqDataChunk(begin, end);
}

// N.B.: this method should be called when the BLAST database data loader
// hasn't been able to find the id in its cache
bool 
CRemoteBlastDbAdapter::SeqidToOid(const CSeq_id & id, int & oid)
{
    // N.B.: This method doesn't get any sequence data from the server side.
    const char seqtype = (GetSequenceType() == CSeqDB::eProtein) ? 'p' : 'n';

    oid = m_NextLocalId++;
    
    // Return types
    CRemoteBlast::TBioseqVector bioseqs;
    CRemoteBlast::TSeqIdVector seqids;
    const bool kVerbose = (getenv("VERBOSE") ? true : false);
    string errors, warnings;
    seqids.push_back(CRef<CSeq_id>(const_cast<CSeq_id*>(&id)));
    
    CRemoteBlast::GetSequencesInfo(seqids, m_DbName, seqtype, bioseqs, errors,
                                   warnings, kVerbose);
    if ( !errors.empty() || !warnings.empty() || bioseqs.empty() ) {
        return RemoteBlastDbLoader_ErrorHandler(errors, warnings);
    }
    _ASSERT(bioseqs.size() == seqids.size());
    
    // cache the retrieved information
    CCachedSeqDataForRemote& cached_seqdata = m_Cache[oid];
    cached_seqdata.SetLength(bioseqs.front()->GetLength());
    cached_seqdata.SetIdList(bioseqs.front()->SetId());
    cached_seqdata.SetBioseq(bioseqs.front());
    return cached_seqdata.IsValid();
}

END_SCOPE(objects)
END_NCBI_SCOPE

