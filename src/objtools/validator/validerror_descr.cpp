/*  $Id: validerror_descr.cpp 147690 2008-12-15 15:33:51Z kans $
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
 * Author:  Jonathan Kans, Clifford Clausen, Aaron Ucko......
 *
 * File Description:
 *   validation of seq_descr
 *   .......
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include "validatorp.hpp"

#include <serial/iterator.hpp>

#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/Seq_descr.hpp>


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(validator)


CValidError_descr::CValidError_descr(CValidError_imp& imp) :
    CValidError_base(imp)
{
}


CValidError_descr::~CValidError_descr(void)
{
}


void CValidError_descr::ValidateSeqDescr(const CSeq_descr& descr)
{
    size_t  num_sources = 0,
            num_titles = 0;
    CConstRef<CSeqdesc> last_source,
                        last_title;

    FOR_EACH_DESCRIPTOR_ON_DESCR (dt, descr) {
        const CSeqdesc& desc = **dt;

        switch ( desc.Which() ) {
        case CSeqdesc::e_Mol_type:
        case CSeqdesc::e_Modif:
        case CSeqdesc::e_Method:
            // obsolete
            break;

        case CSeqdesc::e_Name:
            break;
        case CSeqdesc::e_Title:
            num_titles++;
            last_title = &desc;
            break;
        

        case CSeqdesc::e_Comment:
            break;

        case CSeqdesc::e_Num:
            break;
        case CSeqdesc::e_Maploc:
            break;

        case CSeqdesc::e_Pir:
            break;

        case CSeqdesc::e_Genbank:
            break;

        case CSeqdesc::e_Pub:
            break;

        case CSeqdesc::e_Region:
            break;
        case CSeqdesc::e_User:
            break;

        case CSeqdesc::e_Sp:
            break;

        case CSeqdesc::e_Dbxref:
            break;

        case CSeqdesc::e_Embl:
            break;

        case CSeqdesc::e_Create_date:
            break;
        case CSeqdesc::e_Update_date:
            break;

        case CSeqdesc::e_Prf:
            break;

        case CSeqdesc::e_Pdb:
            break;

        case CSeqdesc::e_Het:
            break;
        case CSeqdesc::e_Source:
            num_sources++;
            last_source = &desc;
            break;
        case CSeqdesc::e_Org:
            break;
        case CSeqdesc::e_Molinfo:
            break;

        default:
            break;
        }
    }

    if ( num_sources > 1 ) {
        PostErr(eDiag_Error, eErr_SEQ_DESCR_MultipleBioSources,
            "Undesired multiple source descriptors (reporting last one)", *last_source);
    }
    if ( num_titles > 1 ) {
        PostErr(eDiag_Error, eErr_SEQ_DESCR_MultipleTitles,
            "Multiple Title blocks (reporting last one)", *last_title);
    }
}


END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE
