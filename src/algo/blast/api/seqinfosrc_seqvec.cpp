#ifndef SKIP_DOXYGEN_PROCESSING
static char const rcsid[] =
    "$Id: seqinfosrc_seqvec.cpp 134303 2008-07-17 17:42:49Z camacho $";
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
 * Author:  Ilya Dondoshansky
 *
 */

/** @file seqinfosrc_seqvec.cpp
 * Implementation of the concrete strategy for an IBlastSeqInfoSrc interface to
 * retrieve sequence identifiers and lengths from a vector of Seq-locs.
 */

#include <ncbi_pch.hpp>
#include <objmgr/util/sequence.hpp>
#include <algo/blast/api/seqinfosrc_seqvec.hpp>
#include <algo/blast/api/blast_exception.hpp>

/** @addtogroup AlgoBlast
 *
 * @{
 */

BEGIN_NCBI_SCOPE
USING_SCOPE(objects);
BEGIN_SCOPE(blast)

CSeqVecSeqInfoSrc::CSeqVecSeqInfoSrc(const TSeqLocVector& seqv)
    : m_SeqVec(seqv)
{
    if (seqv.size() == 0) {
        NCBI_THROW(CBlastException, eInvalidArgument, 
                   "Empty sequence vector for id and length retrieval");
    }
}

CSeqVecSeqInfoSrc::~CSeqVecSeqInfoSrc()
{
}

list< CRef<CSeq_id> > CSeqVecSeqInfoSrc::GetId(Uint4 index) const
{
    if (index >= m_SeqVec.size()) {
        NCBI_THROW(CBlastException, eInvalidArgument, 
                   "Index out of range for id retrieval");
    }

    list < CRef<CSeq_id> > seqid_list;
    CRef<CSeq_id> seqid_ref;

    seqid_ref.Reset(const_cast<CSeq_id*>(
                        &sequence::GetId(*m_SeqVec[index].seqloc, 
                                         m_SeqVec[index].scope)));

    seqid_list.push_back(seqid_ref);
    return seqid_list;
}

CConstRef<CSeq_loc> CSeqVecSeqInfoSrc::GetSeqLoc(Uint4 index) const
{
    if (index >= m_SeqVec.size()) {
        NCBI_THROW(CBlastException, eInvalidArgument, 
                   "Index out of range for Seq-loc retrieval");
    }
    return m_SeqVec[index].seqloc;
}

Uint4 CSeqVecSeqInfoSrc::GetLength(Uint4 index) const
{
    if (index >= m_SeqVec.size()) {
        NCBI_THROW(CBlastException, eInvalidArgument, 
                   "Index out of range for length retrieval");
    }

    return sequence::GetLength(*m_SeqVec[index].seqloc, 
                               m_SeqVec[index].scope);
}

size_t CSeqVecSeqInfoSrc::Size() const
{
    return m_SeqVec.size();
}

bool CSeqVecSeqInfoSrc::HasGiList() const
{
    return false;
}

static void 
s_SeqIntervalToSeqLocInfo(CRef<CSeq_interval> interval,
                          const TSeqRange& target_range,
                          const CSeqLocInfo::ETranslationFrame frame,
                          TMaskedSubjRegions& retval)
{
    TSeqRange loc(interval->GetFrom(), 0);
    loc.SetToOpen(interval->GetTo());

    if (loc.IntersectingWith(target_range)) {
        CRef<CSeqLocInfo> sli(new CSeqLocInfo(interval, frame));
        retval.push_back(sli);
    }
}

bool CSeqVecSeqInfoSrc::GetMasks(Uint4 index,
                                 const TSeqRange& target_range,
                                 TMaskedSubjRegions& retval) const
{
    const CSeqLocInfo::ETranslationFrame kFrame = CSeqLocInfo::eFrameNotSet;

    CRef<CSeq_loc> mask = m_SeqVec[index].mask;
    if (mask.Empty() || target_range == TSeqRange::GetEmpty()) {
        return false;
    }

    if (mask->IsInt()) {
        s_SeqIntervalToSeqLocInfo(CRef<CSeq_interval>(&mask->SetInt()),
                                  target_range, kFrame, retval);
    } else if (mask->IsPacked_int()) {
        ITERATE(CPacked_seqint::Tdata, itr, mask->GetPacked_int().Get()) {
            s_SeqIntervalToSeqLocInfo(*itr, target_range, kFrame, retval);
        }
    } else {
        NCBI_THROW(CBlastException, eInvalidArgument, 
                   "Type of mask not supported");
    }

    return (retval.empty() ? false : true);
}

END_SCOPE(blast)
END_NCBI_SCOPE

/* @} */
