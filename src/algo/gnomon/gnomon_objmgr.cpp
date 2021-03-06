/*  $Id: gnomon_objmgr.cpp 142779 2008-10-09 18:25:17Z chetvern $
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
 * Authors:  Mike DiCuccio
 *
 * File Description: gnomon library parts requiring object manager support
 * to allow apps not using these to be linked w/o object manager libs
 *
 */

#include <ncbi_pch.hpp>
#include <algo/gnomon/gnomon.hpp>
#include <algo/gnomon/gnomon_exception.hpp>
#include "hmm.hpp"
#include "hmm_inlines.hpp"

#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Packed_seqint.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objmgr/util/sequence.hpp>
#include <objects/seqalign/seqalign__.hpp>
#include <objects/general/general__.hpp>

#include <stdio.h>

#include <functional>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(gnomon)
USING_SCOPE(ncbi::objects);

namespace {
int GetProdPosInBases(const CProduct_pos& product_pos)
{
    if (product_pos.IsNucpos())
        return product_pos.GetNucpos();

    const CProt_pos&  prot_pos = product_pos.GetProtpos();
    return prot_pos.GetAmin()*3+ prot_pos.GetFrame()-1;
}

int GetCompartmentNum(const CSeq_align& sa)
{
    if (sa.CanGetExt()) {
        ITERATE(CSeq_align::TExt, e, sa.GetExt()) {
            const CUser_object& uo = **e;
            if (uo.CanGetData()) {
                ITERATE(CUser_object::TData, field, uo.GetData()) {
                    if ((*field)->CanGetLabel() && (*field)->GetLabel().IsStr() && (*field)->GetLabel().GetStr()=="CompartmentId") {
                        return (*field)->GetData().GetInt();
                    }
                }
            }
        }
    }
    return 0;
}
}

void debug() {
}


CAlignModel::CAlignModel(const CSeq_align& seq_align) :
    CGeneModel(seq_align.GetSegs().GetSpliced().GetGenomic_strand()==eNa_strand_minus?eMinus:ePlus,
               GetCompartmentNum(seq_align),
               seq_align.GetSegs().GetSpliced().GetProduct_type()==CSpliced_seg::eProduct_type_protein? eProt:emRNA) {
   
    debug();

    const CSpliced_seg& sps = seq_align.GetSegs().GetSpliced();
    if(sps.CanGetProduct_strand() && sps.GetProduct_strand()==eNa_strand_minus) 
        Status() |= CGeneModel::eReversed;

    CRef<CSeq_id> product_idref(new CSeq_id);
    product_idref->Assign( sps.GetProduct_id() );
    Support().insert(CSupportInfo(product_idref,false));
    bool is_product_reversed = sps.CanGetProduct_strand() && sps.GetProduct_strand()==eNa_strand_minus;
    int product_len = sps.CanGetProduct_length()?sps.GetProduct_length():0;
    bool is_protein = sps.GetProduct_type()==CSpliced_seg::eProduct_type_protein;
    if (is_protein)
        product_len *=3;
    int prod_prev = -1;
    bool prev_3_prime_splice = false;

    int target_len = product_len;

    vector<TSignedSeqRange> transcript_exons;
    TInDels indels;

    ITERATE(CSpliced_seg::TExons, e_it, sps.GetExons()) {
        const CSpliced_exon& exon = **e_it;
        int prod_cur_start = GetProdPosInBases(exon.GetProduct_start());
        int prod_cur_end = GetProdPosInBases(exon.GetProduct_end());
        if (is_product_reversed) {
            int tmp = prod_cur_start;
            prod_cur_start = product_len - prod_cur_end -1;
            prod_cur_end = product_len - tmp -1;
        }
        int nuc_cur_start = exon.GetGenomic_start();
        int nuc_cur_end = exon.GetGenomic_end();

        bool cur_5_prime_splice = exon.CanGetAcceptor_before_exon() && exon.GetAcceptor_before_exon().CanGetBases() && exon.GetAcceptor_before_exon().GetBases().size()==2;
        if (prod_prev+1 != prod_cur_start || !prev_3_prime_splice || !cur_5_prime_splice)
            AddHole();
        prev_3_prime_splice = exon.CanGetDonor_after_exon() && exon.GetDonor_after_exon().CanGetBases() && exon.GetDonor_after_exon().GetBases().size()==2;

        AddExon(TSignedSeqRange(nuc_cur_start,nuc_cur_end));
        transcript_exons.push_back(TSignedSeqRange(GetProdPosInBases(exon.GetProduct_start()),GetProdPosInBases(exon.GetProduct_end())));

        int pos = 0;
        int prod_pos = prod_cur_start;
        ITERATE(CSpliced_exon::TParts, p_it, exon.GetParts()) {
            const CSpliced_exon_chunk& chunk = **p_it;
            if (chunk.IsProduct_ins()) {
                CInDelInfo fs(Strand()==ePlus?nuc_cur_start+pos:nuc_cur_end-pos+1,chunk.GetProduct_ins(),false);
                if (Strand()==ePlus)
                    indels.push_back(fs);
                else
                    indels.insert(indels.begin(), fs);
                prod_pos += chunk.GetProduct_ins();
            } else if (chunk.IsGenomic_ins()) {
                const int genomic_ins = chunk.GetGenomic_ins();
                if (Strand()==ePlus)
                    indels.push_back(CInDelInfo(nuc_cur_start+pos,genomic_ins));
                else
                    indels.insert(indels.begin(), CInDelInfo(nuc_cur_end-pos-genomic_ins+1,genomic_ins));
                pos += genomic_ins;
            } else if (chunk.IsMatch()) {
                pos += chunk.GetMatch();
                prod_pos += chunk.GetMatch();
            } else if (chunk.IsMismatch()) {
                pos += chunk.GetMismatch();
                prod_pos += chunk.GetMismatch();
            } else { // if (chunk.IsDiag())
                pos += chunk.GetDiag();
                prod_pos += chunk.GetDiag();
            }
        }

        prod_prev = prod_cur_end;
    }

    sort(transcript_exons.begin(),transcript_exons.end());
    bool minusstrand = Strand() == eMinus;
    EStrand orientation = (is_product_reversed == minusstrand) ? ePlus : eMinus;
    if(orientation == eMinus)
       reverse(transcript_exons.begin(),transcript_exons.end());

    if (is_protein) {
        _ASSERT(orientation == Strand());
        if (sps.CanGetModifiers()) {
            ITERATE(CSpliced_seg::TModifiers, m, sps.GetModifiers()) {
                if ((*m)->IsStop_codon_found()) {
                    target_len += 3;
                    if (Strand() == ePlus) {
                        ExtendRight( 3 );
                        _ASSERT((transcript_exons.back().GetTo()+1)%3 == 0);
                        transcript_exons.back().SetTo(transcript_exons.back().GetTo()+3);
                    } else {
                        ExtendLeft( 3 );
                        _ASSERT((transcript_exons.front().GetTo()+1)%3 == 0);
                        transcript_exons.front().SetTo(transcript_exons.front().GetTo()+3);
                    }
                }
            }
        }
    }

    m_alignmap = CAlignMap(Exons(), transcript_exons, indels, orientation, target_len );
    FrameShifts() = m_alignmap.GetInDels(true);
    
    for (CGeneModel::TExons::const_iterator piece_begin = Exons().begin(); piece_begin != Exons().end(); ++piece_begin) {
        _ASSERT( !piece_begin->m_fsplice );
        
        CGeneModel::TExons::const_iterator piece_end;
        for (piece_end = piece_begin; piece_end != Exons().end() && piece_end->m_ssplice; ++piece_end) ;
        _ASSERT( piece_end != Exons().end() );
        
        TSignedSeqRange piece_range(piece_begin->GetFrom(),piece_end->GetTo());
            
        piece_range = m_alignmap.ShrinkToRealPoints(piece_range, is_protein); // finds first projectable interval (on codon boundaries  for proteins)   

        TSignedSeqRange pr;
        while(pr != piece_range) {
            pr = piece_range;
            ITERATE(TInDels, i, indels) { // here we check that no indels touch our interval from outside   
                if((i->IsDeletion() && i->Loc() == pr.GetFrom()) || (i->IsInsertion() && i->Loc()+i->Len() == pr.GetFrom()))
                    pr.SetFrom(pr.GetFrom()+1);                
                else if(i->Loc() == pr.GetTo()+1)
                    pr.SetTo(pr.GetTo()-1);
            }
            if(pr != piece_range)
                piece_range = m_alignmap.ShrinkToRealPoints(pr, is_protein);
        }

        _ASSERT(piece_range.NotEmpty());
        _ASSERT(piece_range.IntersectingWith(piece_begin->Limits()) && piece_range.IntersectingWith(piece_end->Limits()));
        Clip(piece_range, CGeneModel::eDontRemoveExons); 
            
        piece_begin = piece_end;
    }



    if (is_protein) {
        TSignedSeqRange reading_frame = Limits();
        TSignedSeqRange start, stop;
        if (sps.CanGetModifiers()) {
            ITERATE(CSpliced_seg::TModifiers, m, sps.GetModifiers()) {
                TSignedSeqRange rf = m_alignmap.MapRangeOrigToEdited(reading_frame, true);
                if ((*m)->IsStart_codon_found()) {
                    start = m_alignmap.MapRangeEditedToOrig(TSignedSeqRange(rf.GetFrom(),rf.GetFrom()+2),false);
                    reading_frame = m_alignmap.MapRangeEditedToOrig(TSignedSeqRange(rf.GetFrom()+3,rf.GetTo()),true);
                } else if ((*m)->IsStop_codon_found()) {
                    stop = m_alignmap.MapRangeEditedToOrig(TSignedSeqRange(rf.GetTo()-2,rf.GetTo()),false);
                    reading_frame = m_alignmap.MapRangeEditedToOrig(TSignedSeqRange(rf.GetFrom(),rf.GetTo()-3),true);
                }
            }
        }

        CCDSInfo cds_info;
        cds_info.SetReadingFrame( reading_frame, true);
        if (start.NotEmpty()) {
            cds_info.SetStart(start, GetProdPosInBases(sps.GetExons().front()->GetProduct_start()) == 0 && sps.GetExons().front()->GetParts().front()->IsMatch());
        }
        if (stop.NotEmpty()) {
            cds_info.SetStop(stop, GetProdPosInBases(sps.GetExons().back()->GetProduct_end()) == product_len-1 );
        }
        SetCdsInfo(cds_info);
    }
}

string CGeneModel::GetProtein (const CResidueVec& contig_sequence) const
{
    string prot_seq;
    if(ReadingFrame().Empty())
        return prot_seq;

    CAlignMap cdsmap(Exons(), FrameShifts(), Strand(), RealCdsLimits());
    CResidueVec cds;
    cdsmap.EditedSequence(contig_sequence, cds);
    TSignedSeqRange mappedcds = cdsmap.MapRangeOrigToEdited(GetCdsInfo().Cds(), false);
    int ashift = mappedcds.GetFrom()%3;
    int bshift = ((int)cds.size()-ashift)%3;

    string cds_seq((char*)&cds[ashift],cds.size()-ashift-bshift);
    objects::CSeqTranslator::Translate(cds_seq,prot_seq);
    return prot_seq;
}

//
// helper function - convert a vector<TSignedSeqRange> into a compact CSeq_loc
//
namespace {

CRef<CSeq_loc> s_ExonDataToLoc(const vector<TSignedSeqRange>& vec,
                               ENa_strand strand, const CSeq_id& id)
{
    CRef<CSeq_loc> loc(new CSeq_loc());

    CPacked_seqint::Tdata data;
    ITERATE (vector<TSignedSeqRange>, iter, vec) {
        CRef<CSeq_interval> ival(new CSeq_interval);
        ival->SetFrom(iter->GetFrom());
        ival->SetTo(iter->GetTo());
        ival->SetStrand(strand);
        ival->SetId().Assign(id);

        data.push_back(ival);
    }

    if (data.size() == 1) {
        loc->SetInt(*data.front());
    } else {
        loc->SetPacked_int().Set().swap(data);
    }

    return loc;
}

} //end unnamed namespace

CRef<CSeq_annot> CGnomonEngine::GetAnnot(const CSeq_id& id)
{
    list<CGeneModel> genes = GetGenes();

    CRef<objects::CSeq_annot> annot(new CSeq_annot());

    annot->AddName("GNOMON gene scan output");

    CSeq_annot::C_Data::TFtable& ftable = annot->SetData().SetFtable();

    unsigned int counter = 0;
    string locus_tag_base("GNOMON_");
    ITERATE (list<CGeneModel>, it, genes) {
        const CGeneModel& igene = *it;
        int strand = igene.Strand();
        TSignedSeqRange cds_limits = igene.RealCdsLimits();

        vector<TSignedSeqRange> mrna_vec;
        copy(igene.Exons().begin(), igene.Exons().end(), back_inserter(mrna_vec));
        vector<TSignedSeqRange> cds_vec;

        for (size_t j = 0;  j < mrna_vec.size();  ++j) {
            TSignedSeqRange intersect(mrna_vec[j] & cds_limits);
            if (!intersect.Empty()) {
                cds_vec.push_back(intersect);
            }
        }

        // stop-codon removal from cds
        if (igene.HasStop()) {
            if (strand == ePlus) {
                _ASSERT(cds_vec.back().GetLength()>=3);
                cds_vec.back().SetTo(cds_vec.back().GetTo() - 3);
            } else {
                _ASSERT(cds_vec.front().GetLength()>=3);
                cds_vec.front().SetFrom(cds_vec.front().GetFrom() + 3);
            }
        }

        //
        // create our mRNA
        CRef<CSeq_feat> feat_mrna;
        if (mrna_vec.size()) {
            feat_mrna = new CSeq_feat();
            feat_mrna->SetData().SetRna().SetType(CRNA_ref::eType_mRNA);
            feat_mrna->SetLocation
                (*s_ExonDataToLoc(mrna_vec,
                 (strand == ePlus ? eNa_strand_plus : eNa_strand_minus), id));
        } else {
            continue;
        }

        //
        // create the accompanying CDS
        CRef<CSeq_feat> feat_cds;
        if (!cds_vec.empty()) {
            feat_cds = new CSeq_feat();
            feat_cds->SetData().SetCdregion().SetFrame(CCdregion::TFrame(1+(strand == ePlus?(igene.ReadingFrame().GetFrom()-cds_limits.GetFrom())%3:(cds_limits.GetTo()-igene.ReadingFrame().GetTo())%3)));

            feat_cds->SetLocation
                (*s_ExonDataToLoc(cds_vec,
                 (strand == ePlus ? eNa_strand_plus : eNa_strand_minus), id));
        }

        //
        // create a dummy gene feature as well
        CRef<CSeq_feat> feat_gene(new CSeq_feat());

        char buf[32];
        sprintf(buf, "%04u", ++counter);
        string name(locus_tag_base);
        name += buf;
        feat_gene->SetData().SetGene().SetLocus_tag(name);
        feat_gene->SetLocation().SetInt()
            .SetFrom(feat_mrna->GetLocation().GetTotalRange().GetFrom());
        feat_gene->SetLocation().SetInt()
            .SetTo(feat_mrna->GetLocation().GetTotalRange().GetTo());
        feat_gene->SetLocation().SetInt().SetStrand
            (strand == ePlus ? eNa_strand_plus : eNa_strand_minus);

        feat_gene->SetLocation().SetId
            (sequence::GetId(feat_mrna->GetLocation(), 0));

        ftable.push_back(feat_gene);
        ftable.push_back(feat_mrna);
        if (feat_cds) {
            ftable.push_back(feat_cds);
        }
    }
    return annot;
}


//
//
// This function deduces the frame from 5p coordinate of the CDS which should be on the 5p codon boundary
//
// 
double CCodingPropensity::GetScore(CConstRef<CHMMParameters> hmm_params, const CSeq_loc& cds, CScope& scope, int *const gccontent, double *const startscore)
{
    *gccontent = 0;
    const CSeq_id* seq_id = cds.GetId();
    if (seq_id == NULL)
	NCBI_THROW(CGnomonException, eGenericError, "CCodingPropensity: CDS has multiple ids or no id at all");
    
    // Need to know GC content in order to load correct models.
    // Compute on the whole transcript, not just CDS.
    CBioseq_Handle xcript_hand = scope.GetBioseqHandle(*seq_id);
    CSeqVector xcript_vec = xcript_hand.GetSeqVector();
    xcript_vec.SetIupacCoding();
    unsigned int gc_count = 0;
    CSeqVector_CI xcript_iter(xcript_vec);
    for( ;  xcript_iter;  ++xcript_iter) {
        if (*xcript_iter == 'G' || *xcript_iter == 'C') {
            ++gc_count;
        }
    }
    *gccontent = static_cast<unsigned int>(100.0 * gc_count / xcript_vec.size() + 0.5);
	
    const CMC3_CodingRegion<5>&   cdr = dynamic_cast<const CMC3_CodingRegion<5>&>(hmm_params->GetParameter(CMC3_CodingRegion<5>::class_id(), *gccontent));
    const CMC_NonCodingRegion<5>& ncdr = dynamic_cast<const CMC_NonCodingRegion<5>&>(hmm_params->GetParameter(CMC_NonCodingRegion<5>::class_id(), *gccontent));

    // Represent coding sequence as enum Nucleotides
    CSeqVector vec(cds, scope);
    vec.SetIupacCoding();
    CEResidueVec seq;
    seq.reserve(vec.size());
    CSeqVector_CI iter(vec);
    for( ;  iter;  ++iter) {
	seq.push_back(fromACGT(*iter));
    }

    // Sum coding and non-coding scores across coding sequence.
    // Don't include stop codon!
    double score = 0;
    for (unsigned int i = 5;  i < seq.size() - 3;  ++i)
        score += cdr.Score(seq, i, i % 3) - ncdr.Score(seq, i);

    if(startscore) {
        //
        // start evalustion needs 17 bases total (11 before ATG and 3 after)
        // if there is not enough sequence it will be substituted by Ns which will degrade the score
        //

        const CWMM_Start& start = dynamic_cast<const CWMM_Start&>(hmm_params->GetParameter(CWMM_Start::class_id(), *gccontent));

        int totallen = xcript_vec.size();
        int left, right;
        int extraNs5p = 0;
        int extrabases = start.Left()+2;            // (11) extrabases left of ATG needed for start (6) and noncoding (5) evaluation
        if(cds.GetStrand() == eNa_strand_plus) {
            int startposition = cds.GetTotalRange().GetFrom();
            if(startposition < extrabases) {
                left = 0;
                extraNs5p = extrabases-startposition;
            } else {
                left = startposition-extrabases;
            }
            right = min(startposition+2+start.Right(),totallen-1);
        } else {
            int startposition = cds.GetTotalRange().GetTo();
            if(startposition+extrabases >= totallen) {
                right = totallen-1;
                extraNs5p = startposition+extrabases-totallen+1;
            } else {
                right = startposition+extrabases;
            }
            left = max(0,startposition-2-start.Right());
        }


        CRef<CSeq_loc> startarea(new CSeq_loc());
        CSeq_interval& d = startarea->SetInt();
        d.SetStrand(cds.GetStrand());
        CRef<CSeq_id> id(new CSeq_id());
        id->Assign(*seq_id);
        d.SetId(*id);
        d.SetFrom(left);
        d.SetTo(right);
        
        CSeqVector sttvec(*startarea, scope);
        sttvec.SetIupacCoding();
        CEResidueVec sttseq;
        sttseq.resize(extraNs5p, enN);                         // fill with Ns if necessery
        for(unsigned int i = 0; i < sttvec.size(); ++i) {
            sttseq.push_back(fromACGT(sttvec[i]));
        }
        sttseq.resize(5+start.Left()+start.Right(), enN);      // add Ns if necessery
        
        *startscore = start.Score(sttseq, extrabases+2);
        if(*startscore != BadScore()) {
            for(unsigned int i = 5; i < sttseq.size(); ++i) {
                *startscore -= ncdr.Score(sttseq, i);
            }
        }
    }

    return score;
}

END_SCOPE(gnomon)
END_NCBI_SCOPE
