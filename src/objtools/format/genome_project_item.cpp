/*  $Id: genome_project_item.cpp 99676 2007-03-05 20:41:55Z kazimird $
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
* Author:  Frank Ludwig, NCBI
*
* File Description:
*   flat-file generator -- genome project item implementation
*
*/
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/seq/Bioseq.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_field.hpp>
#include <objmgr/seqdesc_ci.hpp>

#include <objtools/format/formatter.hpp>
#include <objtools/format/text_ostream.hpp>
#include <objtools/format/items/genome_project_item.hpp>
#include <objtools/format/context.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)


CGenomeProjectItem::CGenomeProjectItem(CBioseqContext& ctx) :
    CFlatItem(&ctx),
    m_ProjectNumber(0)
{
    x_GatherInfo(ctx);
}


void CGenomeProjectItem::Format
(IFormatter& formatter,
 IFlatTextOStream& text_os) const

{
    formatter.FormatGenomeProject(*this, text_os);
}

int CGenomeProjectItem::GetProjectNumber() const {
    return m_ProjectNumber;
}
/***************************************************************************/
/*                                  PRIVATE                                */
/***************************************************************************/


void CGenomeProjectItem::x_GatherInfo(CBioseqContext& ctx)
{
    for (CSeqdesc_CI desc(ctx.GetHandle(), CSeqdesc::e_User);  desc;  ++desc) {
        const CUser_object& uo = desc->GetUser();

        if ( !uo.GetType().IsStr() ) {
            continue;
        }
        string strHeader = uo.GetType().GetStr();
        if ( 0 == NStr::CompareNocase(uo.GetType().GetStr(), "GenomeProjectsDB")) {
            ITERATE (CUser_object::TData, uf_it, uo.GetData()) {
                const CUser_field& field = **uf_it;
                if ( field.IsSetLabel()  &&  field.GetLabel().IsStr() ) {
                    const string& label = field.GetLabel().GetStr();
                    if ( 0 == NStr::CompareNocase(label, "ProjectID")) {
                        m_ProjectNumber = field.GetData().GetInt();
                    }
                }
            }
        }
    }
}


END_SCOPE(objects)
END_NCBI_SCOPE
