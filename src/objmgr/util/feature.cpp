/*  $Id: feature.cpp 132642 2008-07-01 17:37:47Z vasilche $
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
* Author:  Clifford Clausen
*
* File Description:
*   Sequence utilities
*/

#include <ncbi_pch.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>
#include <serial/iterator.hpp>
#include <serial/enumvalues.hpp>


#include <objmgr/object_manager.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/seq_vector.hpp>
#include <objmgr/feat_ci.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/SeqFeatXref.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Prot_ref.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/Rsite_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Feat_id.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seq/IUPACaa.hpp>
#include <objects/seq/NCBIstdaa.hpp>
#include <objects/seq/NCBIeaa.hpp>
#include <objects/seq/NCBI8aa.hpp>
#include <objects/seq/Pubdesc.hpp>
#include <objects/seq/Heterogen.hpp>

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_loc_mix.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>

#include <objects/pub/Pub_equiv.hpp>
#include <objects/pub/Pub.hpp>
#include <objects/pub/Pub_set.hpp>

#include <objmgr/util/feature.hpp>
#include <objmgr/util/sequence.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(feature)
USING_SCOPE(sequence);


// Appends a label onto "label" based on the type of feature       
void s_GetTypeLabel(const CSeq_feat& feat, string* label)
{    
    string tlabel;
    
    // Determine typelabel
    CSeqFeatData::ESubtype idx = feat.GetData().GetSubtype();
    if ( idx != CSeqFeatData::eSubtype_bad ) {
        tlabel = feat.GetData().GetKey();
        if (feat.GetData().Which() == CSeqFeatData::e_Imp  && 
            tlabel != "CDS") {
            tlabel = "[" + tlabel + "]";
        } else if (feat.GetData().Which() == CSeqFeatData::e_Region  && 
            feat.GetData().GetRegion() == "Domain"  &&  feat.IsSetComment() ) {
            tlabel = "Domain";
        }
    } else if (feat.GetData().Which() == CSeqFeatData::e_Imp) {
        tlabel = "[" + feat.GetData().GetImp().GetKey() + "]";
    } else {
        tlabel = "Unknown=0";
    }
    *label += tlabel;  
}


// Appends a label onto tlabel for a CSeqFeatData::e_Cdregion
inline
static void s_GetCdregionLabel
(const CSeq_feat& feat, 
 string*          tlabel,
 CScope*          scope)
{
    // Check that tlabel exists and that the feature data is Cdregion
    if (!tlabel  ||  !feat.GetData().IsCdregion()) {
        return;
    }
    
    const CGene_ref* gref = 0;
    const CProt_ref* pref = 0;
    
    // Look for CProt_ref object to create a label from
    if (feat.IsSetXref()) {
        ITERATE ( CSeq_feat::TXref, it, feat.GetXref()) {
            const CSeqFeatXref& xref = **it;
            if ( !xref.IsSetData() ) {
                continue;
            }

            switch (xref.GetData().Which()) {
            case CSeqFeatData::e_Prot:
                pref = &xref.GetData().GetProt();
                break;
            case CSeqFeatData::e_Gene:
                gref = &xref.GetData().GetGene();
                break;
            default:
                break;
            }
        }
    }
    
    // Try and create a label from a CProt_ref in CSeqFeatXref in feature
    if (pref) {
        pref->GetLabel(tlabel);
        return;
    }
    
    // Try and create a label from a CProt_ref in the feat product and
    // return if found 
    if (feat.IsSetProduct()  &&  scope) {
        try {
            const CSeq_id& id = GetId(feat.GetProduct(), scope);            
            CBioseq_Handle hnd = scope->GetBioseqHandle(id);
            if (hnd) {
                const CBioseq& seq = *hnd.GetCompleteBioseq();
            
                // Now look for a CProt_ref feature in seq and
                // if found call GetLabel() on the CProt_ref
                CTypeConstIterator<CSeqFeatData> it = ConstBegin(seq);
                for (;it; ++it) {
                    if (it->Which() == CSeqFeatData::e_Prot) {
                        it->GetProt().GetLabel(tlabel);
                        return;
                    }
                }
            }
        } catch (CObjmgrUtilException&) {}
    }
    
    // Try and create a label from a CGene_ref in CSeqFeatXref in feature
    if (gref) {
        gref->GetLabel(tlabel);
    }

    // check to see if the CDregion is just an open reading frame
    if (feat.GetData().GetCdregion().IsSetOrf()  &&
        feat.GetData().GetCdregion().GetOrf()) {
        string str("open reading frame: ");
        switch (feat.GetData().GetCdregion().GetFrame()) {
        case CCdregion::eFrame_not_set:
            str += "frame not set; ";
            break;
        case CCdregion::eFrame_one:
            str += "frame 1; ";
            break;
        case CCdregion::eFrame_two:
            str += "frame 2; ";
            break;
        case CCdregion::eFrame_three:
            str += "frame 3; ";
            break;
        }

        switch (sequence::GetStrand(feat.GetLocation(), scope)) {
        case eNa_strand_plus:
            str += "positive strand";
            break;
        case eNa_strand_minus:
            str += "negative strand";
            break;
        case eNa_strand_both:
            str += "both strands";
            break;
        case eNa_strand_both_rev:
            str += "both strands (reverse)";
            break;
        default:
            str += "strand unknown";
            break;
        }

        *tlabel += str;
    }


}


inline
static void s_GetRnaRefLabelFromComment
(const CSeq_feat& feat, 
 string*          label,
 ELabelType       label_type,
 const string*    type_label)
{
    if (feat.IsSetComment()  &&  !feat.GetComment().empty()) {
        if (label_type == eContent  &&  type_label != 0
            &&  feat.GetComment().find(*type_label) == string::npos) {
            *label += *type_label + "-" + feat.GetComment();
        } else {
            *label += feat.GetComment();
        }
    } else if (type_label) {
        *label += *type_label;
    }
}


// Appends a label onto "label" for a CRNA_ref
inline
static void s_GetRnaRefLabel
(const CSeq_feat& feat, 
 string*          label,
 ELabelType       label_type,
 const string*    type_label)
{
    // Check that label exists and that feature data is type RNA-ref
    if (!label  ||  !feat.GetData().IsRna()) {
        return;
    }
    
    const CRNA_ref& rna = feat.GetData().GetRna();
    
    // Append the feature comment, the type label, or both  and return 
    // if Ext is not set
    if (!rna.IsSetExt()) {
        s_GetRnaRefLabelFromComment(feat, label, label_type, type_label);
        return;
    }
    
    // Append a label based on the type of the type of the ext of the
    // CRna_ref
    string tmp_label;
    switch (rna.GetExt().Which()) {
    case CRNA_ref::C_Ext::e_not_set:
        s_GetRnaRefLabelFromComment(feat, label, label_type, type_label);
        break;
    case CRNA_ref::C_Ext::e_Name:
        tmp_label = rna.GetExt().GetName();
        if (label_type == eContent  &&  type_label != 0  &&
                tmp_label.find(*type_label) == string::npos) {
            *label += *type_label + "-" + tmp_label;
        } else if (!tmp_label.empty()) {
            *label += tmp_label;
        } else if (type_label) {
            *label += *type_label;
        }
        break;
    case CRNA_ref::C_Ext::e_TRNA:
    {
        if ( !rna.GetExt().GetTRNA().IsSetAa() ) {
            s_GetRnaRefLabelFromComment(feat, label, label_type, type_label);
            break;                
        }
        try {
            CTrna_ext::C_Aa::E_Choice aa_code_type = 
                rna.GetExt().GetTRNA().GetAa().Which();
            int aa_code;
            CSeq_data in_seq, out_seq;
            string str_aa_code;
            switch (aa_code_type) {
            case CTrna_ext::C_Aa::e_Iupacaa:        
                // Convert an e_Iupacaa code to an Iupacaa3 code for the label
                aa_code = rna.GetExt().GetTRNA().GetAa().GetIupacaa();
                str_aa_code = CSeqportUtil::GetCode(CSeq_data::e_Iupacaa,
                                                    aa_code); 
                in_seq.SetIupacaa().Set() = str_aa_code;
                CSeqportUtil::Convert(in_seq, &out_seq,
                                      CSeq_data::e_Ncbistdaa);
                if (out_seq.GetNcbistdaa().Get().size()) {
                    aa_code = out_seq.GetNcbistdaa().Get()[0];
                    tmp_label = CSeqportUtil::GetIupacaa3(aa_code);
                } else {
                    s_GetRnaRefLabelFromComment(feat, label, label_type,
                                                type_label);
                }
                break;
            case CTrna_ext::C_Aa::e_Ncbieaa:
                // Convert an e_Ncbieaa code to an Iupacaa3 code for the label
                aa_code = rna.GetExt().GetTRNA().GetAa().GetNcbieaa();
                str_aa_code = CSeqportUtil::GetCode(CSeq_data::e_Ncbieaa,
                                                    aa_code);
                in_seq.SetNcbieaa().Set() = str_aa_code;
                CSeqportUtil::Convert(in_seq, &out_seq,
                                      CSeq_data::e_Ncbistdaa);
                if (out_seq.GetNcbistdaa().Get().size()) {
                    aa_code = out_seq.GetNcbistdaa().Get()[0];
                    tmp_label = CSeqportUtil::GetIupacaa3(aa_code);
                } else {
                    s_GetRnaRefLabelFromComment(feat, label, label_type,
                                                type_label);
                }
                break;
            case CTrna_ext::C_Aa::e_Ncbi8aa:
                // Convert an e_Ncbi8aa code to an Iupacaa3 code for the label
                aa_code = rna.GetExt().GetTRNA().GetAa().GetNcbi8aa();
                tmp_label = CSeqportUtil::GetIupacaa3(aa_code);
                break;
            case CTrna_ext::C_Aa::e_Ncbistdaa:
                // Convert an e_Ncbistdaa code to an Iupacaa3 code for the label
                aa_code = rna.GetExt().GetTRNA().GetAa().GetNcbistdaa();
                tmp_label = CSeqportUtil::GetIupacaa3(aa_code);
                break;
            default:
                break;
            }
        
            // Append to label, depending on ELabelType
            if (label_type == eContent  &&  type_label != 0) {
                *label += *type_label + "-" + tmp_label;
            } else if (!tmp_label.empty()) {
                *label += tmp_label;
            } else if (type_label) {
                *label += *type_label;
            }
        } catch (CSeqportUtil::CBadIndex&) {
            // fall back to comment (if any)
            s_GetRnaRefLabelFromComment(feat, label, label_type, type_label);
        }
        
        break;
    }}       
}


// Appends a label to tlabel for a CImp_feat. A return value of true indicates 
// that the label was created for a CImp_feat key = "Site-ref" 
inline
static bool s_GetImpLabel
(const CSeq_feat&      feat, 
 string*               tlabel,
 ELabelType            label_type,
 const string*         type_label)
{
    // Return if tlablel does not exist or feature data is not Imp-feat
    if (!tlabel  ||  !feat.GetData().IsImp()) {
        return false;
    }
    
    const string& key = feat.GetData().GetImp().GetKey();
    bool empty = true;
    
    // If the key is Site-ref
    if (NStr::EqualNocase(key, "Site-ref")) {
        if (feat.IsSetCit()) {
            // Create label based on Pub-set
            feat.GetCit().GetLabel(tlabel);
            return true;
        }
    // else if the key is not Site-ref
    } else if (label_type == eContent) {
        // If the key is CDS
        if (NStr::EqualNocase(key, "CDS")) {
            *tlabel += "[CDS]";
        // else if the key is repeat_unit or repeat_region
        } else if (NStr::EqualNocase(key, "repeat_unit")  ||
                   NStr::EqualNocase(key, "repeat_region")) {
            if (feat.IsSetQual()) {
                // Loop thru the feature qualifiers
                ITERATE( CSeq_feat::TQual, it, feat.GetQual()) {
                    // If qualifier qual is rpt_family append qualifier val
                    if (NStr::EqualNocase((*it)->GetQual(),"rpt_family")) { 
                        *tlabel += (*it)->GetVal();
                        empty = false;
                        break;
                    }
                }
            }
            
            // If nothing has been appended yet
            if (empty) {
                *tlabel += type_label ? *type_label : string("");
            }
        // else if the key is STS
        } else if (NStr::EqualNocase(key, "STS")) {
            if (feat.IsSetQual()) {
                ITERATE( CSeq_feat::TQual, it, feat.GetQual()) {
                    if (NStr::EqualNocase((*it)->GetQual(),"standard_name"))
                    { 
                           *tlabel = (*it)->GetVal();
                           empty = false;
                           break;
                    }
                }
            }
            
            // If nothing has been appended yet
            if (empty) {
                if (feat.IsSetComment()) {
                    size_t pos = feat.GetComment().find(";");
                    if (pos == string::npos) {
                        *tlabel += feat.GetComment();
                    } else {
                        *tlabel += feat.GetComment().substr(0, pos);
                    } 
                } else {
                    *tlabel += type_label ? *type_label : string("");
                }
            }
        // else if the key is misc_feature
        } else if (NStr::EqualNocase(key, "misc_feature")) {
            if (feat.IsSetQual()) {
                // Look for a single qualifier qual in order of preference 
                // "standard_name", "function", "number", any and
                // append to tlabel and return if found
                ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                    if (NStr::EqualNocase((*it)->GetQual(),"standard_name")) {
                        *tlabel += (*it)->GetVal();
                        return false;
                    }
                }
                ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                    if (NStr::EqualNocase((*it)->GetQual(), "function")) {
                        *tlabel += (*it)->GetVal();
                        return false;
                    }
                }
                ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                    if (NStr::EqualNocase((*it)->GetQual(), "number")) {
                        *tlabel += (*it)->GetVal();
                        return false;
                    }
                }
                ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
                    *tlabel += (*it)->GetVal();
                    return false;
                }
                // Append type_label if there is one
                if (empty) {
                    *tlabel += type_label ? *type_label : string("");
                    return false;
                }
            }
        }
    } 
    return false;                
}

 
// Return a label based on the content of the feature
void s_GetContentLabel
(const CSeq_feat&      feat,
 string*               label,
 const string*         type_label,
 ELabelType            label_type, 
 CScope*               scope)
{
    string tlabel;
    
    // Get a content label dependent on the type of the feature data
    switch (feat.GetData().Which()) {
    case CSeqFeatData::e_Gene:
        feat.GetData().GetGene().GetLabel(&tlabel);
        break;
    case CSeqFeatData::e_Org:
        feat.GetData().GetOrg().GetLabel(&tlabel);
        break;
    case CSeqFeatData::e_Cdregion:
        s_GetCdregionLabel(feat, &tlabel, scope);
        break;
    case CSeqFeatData::e_Prot:
        feat.GetData().GetProt().GetLabel(&tlabel);
        break;
    case CSeqFeatData::e_Rna:
        s_GetRnaRefLabel(feat, &tlabel, label_type, type_label);
        break;  
    case CSeqFeatData::e_Pub:
        feat.GetData().GetPub().GetPub().GetLabel(&tlabel); 
        break;
    case CSeqFeatData::e_Seq:
        break;
    case CSeqFeatData::e_Imp:
        if (s_GetImpLabel(feat, &tlabel, label_type, type_label)) {
            *label += tlabel;
            return;
        }
        break;
    case CSeqFeatData::e_Region:
        if (feat.GetData().GetRegion().find("Domain") != string::npos  && 
                feat.IsSetComment()) {
            tlabel += feat.GetComment();
        } else {
            tlabel += feat.GetData().GetRegion();
        }
        break;
    case CSeqFeatData::e_Comment:
        tlabel += feat.IsSetComment() ? feat.GetComment() : string("");
        break;
    case CSeqFeatData::e_Bond:
        // Get the ASN string name for the enumerated EBond type
        tlabel += CSeqFeatData::GetTypeInfo_enum_EBond()
            ->FindName(feat.GetData().GetBond(), true);
        break;
    case CSeqFeatData::e_Site:
        // Get the ASN string name for the enumerated ESite type
        tlabel += CSeqFeatData::GetTypeInfo_enum_ESite()
            ->FindName(feat.GetData().GetSite(), true);
        break;
    case CSeqFeatData::e_Rsite:
        switch (feat.GetData().GetRsite().Which()) {
        case CRsite_ref::e_Str:
            tlabel += feat.GetData().GetRsite().GetStr();
            break;
        case CRsite_ref::e_Db:
            tlabel += feat.GetData().GetRsite().GetDb().GetTag().IsStr() ?
                feat.GetData().GetRsite().GetDb().GetTag().GetStr() : 
                string("?");
            break;
        default:
            break;
        }
        break;
    case CSeqFeatData::e_User:
        if (feat.GetData().GetUser().IsSetClass()) {
            tlabel += feat.GetData().GetUser().GetClass();
        } else if (feat.GetData().GetUser().GetType().IsStr()) {
            tlabel += feat.GetData().GetUser().GetType().GetStr();
        }
    case CSeqFeatData::e_Txinit:
        break;
    case CSeqFeatData::e_Num:
        break;
    case CSeqFeatData::e_Psec_str:
        tlabel += CSeqFeatData::GetTypeInfo_enum_EPsec_str()
            ->FindName(feat.GetData().GetPsec_str(), true);
        break;    
    case CSeqFeatData::e_Non_std_residue:
        tlabel += feat.GetData().GetNon_std_residue();
        break;
    case CSeqFeatData::e_Het:
        tlabel += feat.GetData().GetHet().Get();
        break;        
    case CSeqFeatData::e_Biosrc:
        {{
            const CBioSource& biosrc = feat.GetData().GetBiosrc();
            string str;
            if (biosrc.IsSetSubtype()) {
                ITERATE (CBioSource::TSubtype, iter, biosrc.GetSubtype()) {
                    if ( !str.empty() ) {
                        str += "; ";
                    }
                    (*iter)->GetLabel(&str);
                }
            }
            if (str.empty()) {
                feat.GetData().GetBiosrc().GetOrg().GetLabel(&str);
            } else {
                str += " (";
                feat.GetData().GetBiosrc().GetOrg().GetLabel(&str);
                str += ")";
            }
            tlabel += str;
        }}
        break;        
    default:
        break;
    }
    
    // Return if a label has been calculated above
    if (!tlabel.empty()) {
        *label += tlabel;
        return;
    }
    
    // Put Seq-feat qual into label
    if (feat.IsSetQual()) {
        string prefix("/");
        ITERATE(CSeq_feat::TQual, it, feat.GetQual()) {
            tlabel += prefix + (**it).GetQual();
            prefix = " ";
            if (!(**it).GetVal().empty()) {
                tlabel += "=" + (**it).GetVal();
            }
        }
    }
    
    // Put Seq-feat comment into label
    if (feat.IsSetComment()) {
        if (tlabel.empty()) {
            tlabel = feat.GetComment();
        } else {
            tlabel += "; " + feat.GetComment();
        }
    }
    
    *label += tlabel;
}


void GetLabel
(const CSeq_feat&    feat,
 string*             label,
 ELabelType          label_type,
 CScope*             scope)
{
 
    // Ensure that label exists
    if (!label) {
        return;
    }
    
    // Get the type label
    string type_label;
    s_GetTypeLabel(feat, &type_label);
    
    // Append the type label and return if content label not required
    if (label_type == eBoth) {
        *label += type_label + ": ";
    } else if (label_type == eType) {
        *label += type_label;
        return;
    }
    
    // Append the content label
    size_t label_len = label->size();
    s_GetContentLabel(feat, label, &type_label, label_type, scope);
    
    // If there is no content label, append the type label
    if (label->size() == label_len  &&  label_type == eContent) {
        *label += type_label;
    }
}


void CFeatIdRemapper::Reset(void)
{
    m_IdMap.clear();
}


size_t CFeatIdRemapper::GetFeatIdsCount(void) const
{
    return m_IdMap.size();
}


int CFeatIdRemapper::RemapId(int old_id, const CTSE_Handle& tse)
{
    TFullId key(old_id, tse);
    int& new_id = m_IdMap[key];
    if ( !new_id ) {
        new_id = m_IdMap.size();
    }
    return new_id;
}


bool CFeatIdRemapper::RemapId(CFeat_id& id, const CTSE_Handle& tse)
{
    bool mapped = false;
    if ( id.IsLocal() ) {
        CObject_id& local = id.SetLocal();
        if ( local.IsId() ) {
            int old_id = local.GetId();
            int new_id = RemapId(old_id, tse);
            if ( new_id != old_id ) {
                mapped = true;
                local.SetId(new_id);
            }
        }
    }
    return mapped;
}


bool CFeatIdRemapper::RemapId(CFeat_id& id, const CFeat_CI& feat_it)
{
    bool mapped = false;
    if ( id.IsLocal() ) {
        CObject_id& local = id.SetLocal();
        if ( local.IsId() ) {
            int old_id = local.GetId();
            int new_id = RemapId(old_id, feat_it.GetAnnot().GetTSE_Handle());
            if ( new_id != old_id ) {
                mapped = true;
                local.SetId(new_id);
            }
        }
    }
    return mapped;
}


bool CFeatIdRemapper::RemapIds(CSeq_feat& feat, const CTSE_Handle& tse)
{
    bool mapped = false;
    if ( feat.IsSetId() ) {
        if ( RemapId(feat.SetId(), tse) ) {
            mapped = true;
        }
    }
    if ( feat.IsSetXref() ) {
        NON_CONST_ITERATE ( CSeq_feat::TXref, it, feat.SetXref() ) {
            CSeqFeatXref& xref = **it;
            if ( xref.IsSetId() && RemapId(xref.SetId(), tse) ) {
                mapped = true;
            }
        }
    }
    return mapped;
}


CRef<CSeq_feat> CFeatIdRemapper::RemapIds(const CFeat_CI& feat_it)
{
    CRef<CSeq_feat> feat(SerialClone(feat_it->GetMappedFeature()));
    if ( feat->IsSetId() ) {
        RemapId(feat->SetId(), feat_it);
    }
    if ( feat->IsSetXref() ) {
        NON_CONST_ITERATE ( CSeq_feat::TXref, it, feat->SetXref() ) {
            CSeqFeatXref& xref = **it;
            if ( xref.IsSetId() ) {
                RemapId(xref.SetId(), feat_it);
            }
        }
    }
    return feat;
}


END_SCOPE(feature)
END_SCOPE(objects)
END_NCBI_SCOPE
