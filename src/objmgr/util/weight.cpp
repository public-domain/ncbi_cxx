/*  $Id: weight.cpp 107994 2007-07-31 20:01:14Z ucko $
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
* Author:  Aaron Ucko
*
* File Description:
*   Weights for protein sequences
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <objmgr/object_manager.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/feat_ci.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/seq_vector_ci.hpp>
#include <objmgr/objmgr_exception.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_inst.hpp>

#include <objects/seqfeat/Prot_ref.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <objects/seqloc/Seq_loc.hpp>

#include <objmgr/util/weight.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

// By NCBIstdaa:
// A B C D E F G H  I  K  L M N P Q  R S T V  W X Y Z U *  O  J
static const int kNumC[] =
{0,3,4,3,4,5,9,2,6, 6, 6, 6,5,4,5,5, 6,3,4,5,11,0,9,5,3,0,12, 6};
static const int kNumH[] =
{0,5,5,5,5,7,9,3,7,11,12,11,9,6,7,8,12,5,7,9,10,0,9,7,5,0,21,11};
static const int kNumN[] =
{0,1,1,1,1,1,1,1,3, 1, 2, 1,1,2,1,2, 4,1,1,1, 2,0,1,1,1,0, 3, 1};
static const int kNumO[] =
{0,1,3,1,3,3,1,1,1, 1, 1, 1,1,2,1,2, 1,2,2,1, 1,0,2,3,1,0, 3, 1};
static const int kNumS[] =
{0,0,0,1,0,0,0,0,0, 0, 0, 0,1,0,0,0, 0,0,0,0, 0,0,0,0,0,0, 0, 0};
static const int kNumSe[] =
{0,0,0,0,0,0,0,0,0, 0, 0, 0,0,0,0,0, 0,0,0,0, 0,0,0,0,1,0, 0, 0};
static const size_t kMaxRes = sizeof(kNumC) / sizeof(*kNumC) - 1;


double GetProteinWeight(const CBioseq_Handle& handle, const CSeq_loc* location)
{
    CSeqVector v = (location
                    ? CSeqVector(*location, handle.GetScope())
                    : handle.GetSeqVector());
    v.SetCoding(CSeq_data::e_Ncbistdaa);

    TSeqPos size = v.size();

    // Start with water (H2O)
    TSeqPos c = 0, h = 2, n = 0, o = 1, s = 0, se = 0;

    for (CSeqVector_CI vit(v);  vit.GetPos() < size;  ++vit) {
        CSeqVector::TResidue res = *vit;
        if ( res >= kMaxRes  ||  !kNumC[res] ) {
            NCBI_THROW(CObjmgrUtilException, eBadResidue,
                "GetProteinWeight: bad residue");
        }
        c  += kNumC [res];
        h  += kNumH [res];
        n  += kNumN [res];
        o  += kNumO [res];
        s  += kNumS [res];
        se += kNumSe[res];
    }
    return 12.01115 * c + 1.0079 * h + 14.0067 * n + 15.9994 * o + 32.064 * s
        + 78.96 * se;
}


void GetProteinWeights(const CBioseq_Handle& handle, TWeights& weights)
{
    if (handle.GetBioseqMolType() != CSeq_inst::eMol_aa) {
        NCBI_THROW(CObjmgrUtilException, eBadSequenceType,
            "GetMolecularWeights requires a protein!");
    }
    weights.clear();

    set<CConstRef<CSeq_loc> > locations;
    CConstRef<CSeq_loc> signal;

    // Look for explicit markers: ideally cleavage products (mature
    // peptides), but possibly just signal peptides
    SAnnotSelector sel;
    sel.SetOverlapIntervals().SetResolveTSE()
        .IncludeFeatSubtype(CSeqFeatData::eSubtype_mat_peptide_aa) // mature
        .IncludeFeatSubtype(CSeqFeatData::eSubtype_sig_peptide_aa) // signal
        .IncludeFeatType(CSeqFeatData::e_Region)
        .IncludeFeatType(CSeqFeatData::e_Site);
    for (CFeat_CI feat(handle, sel); feat;  ++feat) {
        bool is_mature = false, is_signal = false;
        const CSeqFeatData& data = feat->GetData();
        switch (data.Which()) {
        case CSeqFeatData::e_Prot:
            switch (data.GetProt().GetProcessed()) {
            case CProt_ref::eProcessed_mature:         is_mature = true; break;
            case CProt_ref::eProcessed_signal_peptide: is_signal = true; break;
            default: break;
            }
            break;

        case CSeqFeatData::e_Region:
            if (!NStr::CompareNocase(data.GetRegion(), "mature chain")
                ||  !NStr::CompareNocase(data.GetRegion(),
                                         "processed active peptide")) {
                is_mature = true;
            } else if (!NStr::CompareNocase(data.GetRegion(), "signal")) {
                is_signal = true;
            }
            break;

        case CSeqFeatData::e_Site:
            if (data.GetSite() == CSeqFeatData::eSite_signal_peptide) {
                is_signal = true;
            }
            break;

        default:
            break;
        }

        if (is_mature) {
            locations.insert(CConstRef<CSeq_loc>(&feat->GetLocation()));
        } else if (is_signal  &&  signal.Empty()
                   &&  !feat->GetLocation().IsWhole() ) {
            signal = &feat->GetLocation();
        }
    }

    if (locations.empty()) {
        CSeqVector v = handle.GetSeqVector(CBioseq_Handle::eCoding_Iupac);
        CRef<CSeq_loc> whole(new CSeq_loc);
        if ( signal.NotEmpty() ) {
            // Expects to see at beginning; is this assumption safe?
            CSeq_interval& interval = whole->SetInt();
            interval.SetFrom(signal->GetTotalRange().GetTo() + 1);
            interval.SetTo(v.size() - 1);
            interval.SetId(const_cast<CSeq_id&>(*handle.GetSeqId()));
        } else if (v[0] == 'M') { // Treat initial methionine as start codon
            CSeq_interval& interval = whole->SetInt();
            interval.SetFrom(1);
            interval.SetTo(v.size() - 1);
            interval.SetId(const_cast<CSeq_id&>(*handle.GetSeqId()));
        }
        else {
            whole->SetWhole(const_cast<CSeq_id&>(*handle.GetSeqId()));
        }
        locations.insert(CConstRef<CSeq_loc>(whole));
    }

    ITERATE(set<CConstRef<CSeq_loc> >, it, locations) {
        try {
            // Split up to ensure that we call [] only if
            // GetProteinWeight succeeds.
            double weight = GetProteinWeight(handle, *it);
            weights[*it] = weight;
        } catch (CObjmgrUtilException) {
            // Silently elide
        }
    }
}


END_SCOPE(objects)
END_NCBI_SCOPE
