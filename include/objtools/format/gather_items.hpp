#ifndef OBJTOOLS_FORMAT___GATHER_ITEMS__HPP
#define OBJTOOLS_FORMAT___GATHER_ITEMS__HPP

/*  $Id: gather_items.hpp 103491 2007-05-04 17:18:18Z kazimird $
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
* Author:  Mati Shomrat
*
* File Description:
*
*/
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>

#include <objtools/format/context.hpp>
#include <objtools/format/flat_file_config.hpp>
#include <objtools/format/items/comment_item.hpp>
#include <objtools/format/items/feature_item.hpp>

#include <deque>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)


class CBioseq_Handle;
class CSeq_feat;
class CFlatItemOStream;


class NCBI_FORMAT_EXPORT CFlatGatherer : public CObject
{
public:
    
    // virtual constructor
    static CFlatGatherer* New(CFlatFileConfig::TFormat format);

    virtual void Gather(CFlatFileContext& ctx, CFlatItemOStream& os) const;

    virtual ~CFlatGatherer(void);

protected:
    typedef CRange<TSeqPos> TRange;

    CFlatGatherer(void) {}

    CFlatItemOStream& ItemOS     (void) const { return *m_ItemOS;  }
    CBioseqContext& Context      (void) const { return *m_Current; }
    const CFlatFileConfig& Config(void) const { return m_Context->GetConfig(); }

    virtual void x_GatherSeqEntry(const CSeq_entry_Handle& entry) const;
    virtual void x_GatherBioseq(const CBioseq_Handle& seq) const;
    virtual void x_DoMultipleSections(const CBioseq_Handle& seq) const;
    virtual void x_DoSingleSection(CBioseqContext& ctx) const = 0;

    // references
    typedef CBioseqContext::TReferences TReferences;
    void x_GatherReferences(void) const;
    void x_GatherReferences(const CSeq_loc& loc, TReferences& refs) const;
    void x_GatherCDSReferences(TReferences& refs) const;

    // features
    void x_GatherFeatures  (void) const;
    void x_GetFeatsOnCdsProduct(const CSeq_feat& feat, const CSeq_loc& mapped_loc, CBioseqContext& ctx) const;
    void x_CopyCDSFromCDNA(const CSeq_feat& feat, CBioseqContext& ctx) const;
    bool x_SkipFeature(const CSeq_feat& feat, const CBioseqContext& ctx) const;
    void x_GatherFeaturesOnLocation(const CSeq_loc& loc, SAnnotSelector& sel,
        CBioseqContext& ctx) const;

    // source features
    typedef CRef<CSourceFeatureItem>    TSFItem;
    typedef deque<TSFItem>              TSourceFeatSet;
    void x_GatherSourceFeatures(void) const;
    void x_CollectBioSources(TSourceFeatSet& srcs) const;
    void x_CollectBioSourcesOnBioseq(const CBioseq_Handle& bh,
        const TRange& range, CBioseqContext& ctx, TSourceFeatSet& srcs) const;
    void x_CollectSourceDescriptors(const CBioseq_Handle& bh, CBioseqContext& ctx,
        TSourceFeatSet& srcs) const;
    void x_CollectSourceFeatures(const CBioseq_Handle& bh,
        const TRange& range, CBioseqContext& ctx,
        TSourceFeatSet& srcs) const;
    void x_MergeEqualBioSources(TSourceFeatSet& srcs) const;
    void x_SubtractFromFocus(TSourceFeatSet& srcs) const;

    // alignments
    void x_GatherAlignments(void) const;

    // comments
    void x_GatherComments  (void) const;
    void x_AddComment(CCommentItem* comment) const;
    void x_AddGSDBComment(const CDbtag& dbtag, CBioseqContext& ctx) const;
    void x_FlushComments(void) const;
    void x_IdComments(CBioseqContext& ctx) const;
    void x_RefSeqComments(CBioseqContext& ctx) const;
    void x_HistoryComments(CBioseqContext& ctx) const;
    void x_WGSComment(CBioseqContext& ctx) const;
    void x_GBBSourceComment(CBioseqContext& ctx) const;
    void x_BarcodeComment(CBioseqContext& ctx) const;
    void x_DescComments(CBioseqContext& ctx) const;
    void x_MaplocComments(CBioseqContext& ctx) const;
    void x_RegionComments(CBioseqContext& ctx) const;
    void x_HTGSComments(CBioseqContext& ctx) const;
    void x_FeatComments(CBioseqContext& ctx) const;
    void x_NameComments(CBioseqContext& ctx) const;

    // sequence 
    void x_GatherSequence  (void) const;
    

private:
    CFlatGatherer(const CFlatGatherer&);
    CFlatGatherer& operator=(const CFlatGatherer&);

    // types
    typedef vector< CRef<CCommentItem> > TCommentVec;

    // data
    mutable CRef<CFlatItemOStream>   m_ItemOS;
    mutable CRef<CFlatFileContext>   m_Context;
    mutable CRef<CBioseqContext>     m_Current;
    mutable TCommentVec              m_Comments;
};


END_SCOPE(objects)
END_NCBI_SCOPE

#endif  /* OBJTOOLS_FORMAT___GATHER_INFO__HPP */
