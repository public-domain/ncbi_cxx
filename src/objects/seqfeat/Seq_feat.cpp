/* $Id: Seq_feat.cpp 155403 2009-03-23 18:50:25Z vasilche $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the ASN data definition file
 *   'seqfeat.asn'.
 */

// generated includes
#include <ncbi_pch.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/seq/seqport_util.hpp>
#include <vector>


BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::


// destructor
CSeq_feat::~CSeq_feat(void)
{
}


static int s_TypeOrder[] = {
    3, // e_not_set = 0,
    0, // e_Gene,
    3, // e_Org,
    2, // e_Cdregion,
    3, // e_Prot,
    1, // e_Rna,
    3  // e_Pub, and the rest
};

int CSeq_feat::GetTypeSortingOrder(CSeqFeatData::E_Choice type)
{
    return s_TypeOrder[min(size_t(type),
                           sizeof(s_TypeOrder)/sizeof(s_TypeOrder[0])-1)];
}


// Corresponds to SortFeatItemListByPos from the C toolkit
int CSeq_feat::CompareNonLocation(const CSeq_feat& f2,
                                  const CSeq_loc& loc1,
                                  const CSeq_loc& loc2) const
{
    const CSeqFeatData& data1 = GetData();
    const CSeqFeatData& data2 = f2.GetData();
    CSeqFeatData::E_Choice type1 = data1.Which();
    CSeqFeatData::E_Choice type2 = data2.Which();

    if ( type1 != type2 ) {
        // order by feature type
        int order1 = GetTypeSortingOrder(type1);
        int order2 = GetTypeSortingOrder(type2);
        int diff = order1 - order2;
        if ( diff != 0 )
            return diff;
    }

    // compare internal intervals
    if ( loc1.IsMix() ) {
        if ( loc2.IsMix() ) {
            const CSeq_loc_mix::Tdata& l1 = loc1.GetMix().Get();
            const CSeq_loc_mix::Tdata& l2 = loc2.GetMix().Get();
            for ( CSeq_loc_mix::Tdata::const_iterator
                      it1 = l1.begin(), it2 = l2.begin(); ;  it1++, it2++) {
                if ( it1 == l1.end() ) {
                    if ( it2 == l2.end() ) {
                        break;
                    }
                    else {
                        // f1 loc is shorter
                        return -1;
                    }
                }
                if ( it2 == l2.end() ) {
                    // f2 loc is shorter
                    return 1;
                }
                int diff = (*it1)->Compare(**it2);
                if ( diff != 0 )
                    return diff;
            }
        }
        else {
            // non-mix loc2 first
            return 1;
        }
    }
    else {
        if ( loc2.IsMix() ) {
            // non-mix loc1 first
            return -1;
        }
    }

    {{ // compare subtypes
        CSeqFeatData::ESubtype subtype1 = data1.GetSubtype();
        CSeqFeatData::ESubtype subtype2 = data2.GetSubtype();
        int diff = subtype1 - subtype2;
        if ( diff != 0 )
            return diff;
    }}

    // subtypes are equal, types must be equal too
    _ASSERT(type1 == type2);

    // type dependent comparison
    if ( type1 == CSeqFeatData::e_Cdregion ) {
        // compare frames of identical CDS ranges
        CCdregion::EFrame frame1 = data1.GetCdregion().GetFrame();
        CCdregion::EFrame frame2 = data2.GetCdregion().GetFrame();
        if (frame1 > CCdregion::eFrame_one
            ||  frame2 > CCdregion::eFrame_one) {
            int diff = frame1 - frame2;
            if ( diff != 0 )
                return diff;
        }
    }
    else if ( type1 == CSeqFeatData::e_Imp ) {
        // compare labels of imp features
        int diff = NStr::CompareNocase(data1.GetImp().GetKey(),
                                       data2.GetImp().GetKey());
        if ( diff != 0 )
            return diff;
    }

    // XXX - should compare parent seq-annots
    // XXX 1. parent Seq-annot idx.itemID
    // XXX 2. features itemID

    return 0; // unknown
}

const CGene_ref* CSeq_feat::GetGeneXref(void) const

{
    ITERATE(CSeq_feat::TXref, it, GetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsGene ()) {
            return &((*it)->GetData ().GetGene ());
        }
    }
    return 0;
}

void CSeq_feat::SetGeneXref(CGene_ref& value)

{
    NON_CONST_ITERATE(CSeq_feat::TXref, it, SetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsGene ()) {
            (*it)->SetData ().SetGene ().Assign(value);
            return;
        }
    }
    TXref& xref = SetXref ();
    CRef<CSeqFeatXref> gref (new CSeqFeatXref);
    xref.push_back (gref);
    gref->SetData ().SetGene ().Assign (value);
}

CGene_ref& CSeq_feat::SetGeneXref(void)
{
    NON_CONST_ITERATE(CSeq_feat::TXref, it, SetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsGene ()) {
            return ((*it)->SetData ().SetGene ());
        }
    }
    TXref& xref = SetXref ();
    CRef<CSeqFeatXref> gref (new CSeqFeatXref);
    xref.push_back (gref);
    return gref->SetData ().SetGene ();
}

const CProt_ref* CSeq_feat::GetProtXref(void) const

{
    ITERATE(CSeq_feat::TXref, it, GetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsProt ()) {
            return &((*it)->GetData ().GetProt ());
        }
    }
    return 0;
}

void CSeq_feat::SetProtXref(CProt_ref& value)

{
    NON_CONST_ITERATE(CSeq_feat::TXref, it, SetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsProt ()) {
            (*it)->SetData ().SetProt ().Assign(value);
            return;
        }
    }
    TXref& xref = SetXref ();
    CRef<CSeqFeatXref> pref (new CSeqFeatXref);
    xref.push_back (pref);
    pref->SetData ().SetProt ().Assign (value);
}

CProt_ref& CSeq_feat::SetProtXref(void)

{
    NON_CONST_ITERATE(CSeq_feat::TXref, it, SetXref ()) {
        if ((*it)->IsSetData () && (*it)->GetData ().IsProt ()) {
            return ((*it)->SetData ().SetProt ());
        }
    }
    TXref& xref = SetXref ();
    CRef<CSeqFeatXref> pref (new CSeqFeatXref);
    xref.push_back (pref);
    return pref->SetData ().SetProt ();
}


void CSeq_feat::AddQualifier(const string& qual_name, const string& qual_val)
{
    CRef<CGb_qual> qual(new CGb_qual());
    qual->SetQual(qual_name);
    qual->SetVal(qual_val);
    SetQual().push_back(qual);
}


void CSeq_feat::AddDbxref(const string& db, const string& tag)
{
    CRef<CDbtag> dbtag(new CDbtag());
    dbtag->SetDb(db);
    dbtag->SetTag().SetStr(tag);
    SetDbxref().push_back(dbtag);
}


void CSeq_feat::AddDbxref(const string& db, int tag)
{
    CRef<CDbtag> dbtag(new CDbtag());
    dbtag->SetDb(db);
    dbtag->SetTag().SetId(tag);
    SetDbxref().push_back(dbtag);
}


CConstRef<CDbtag> CSeq_feat::GetNamedDbxref(const string& db) const
{
    if (IsSetDbxref()) {
        ITERATE (TDbxref, iter, GetDbxref()) {
            if ( (*iter)->GetDb() == db) {
                return *iter;
            }
        }
    }

    return CConstRef<CDbtag>();
}


const string& CSeq_feat::GetNamedQual(const string& qual_name) const
{
    if (IsSetQual()) {
        ITERATE (TQual, iter, GetQual()) {
            if ( (*iter)->GetQual() == qual_name  &&  (*iter)->IsSetVal()) {
                return (*iter)->GetVal();
            }
        }
    }

    return kEmptyStr;
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 61, chars: 1885, CRC32: 417ca6e0 */