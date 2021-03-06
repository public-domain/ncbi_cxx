/* $Id: validator.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 * Author:  Jonathan Kans, Clifford Clausen, Aaron Ucko.......
 *
 * File Description:
 *   Validates CSeq_entries and CSeq_submits
 *
 */
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <serial/serialbase.hpp>
#include <objects/submit/Seq_submit.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/util/sequence.hpp>
#include <objtools/validator/validator.hpp>
#include <util/static_map.hpp>

#include "validatorp.hpp"


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)
BEGIN_SCOPE(validator)
USING_SCOPE(sequence);


// *********************** CValidator implementation **********************


CValidator::CValidator(CObjectManager& objmgr) :
    m_ObjMgr(&objmgr),
    m_PrgCallback(0),
    m_UserData(0)
{
}


CValidator::~CValidator(void)
{
}


CConstRef<CValidError> CValidator::Validate
(const CSeq_entry& se,
 CScope* scope,
 Uint4 options)
{
    CRef<CValidError> errors(new CValidError(&se));
    CValidError_imp imp(*m_ObjMgr, &(*errors), options);
    imp.SetProgressCallback(m_PrgCallback, m_UserData);
    if ( !imp.Validate(se, 0, scope) ) {
        errors.Reset();
    }
    return errors;
}

CConstRef<CValidError> CValidator::Validate
(const CSeq_entry_Handle& seh,
 Uint4 options)
{
    CRef<CValidError> errors(new CValidError(&*seh.GetCompleteSeq_entry()));
    CValidError_imp imp(*m_ObjMgr, &(*errors), options);
    imp.SetProgressCallback(m_PrgCallback, m_UserData);
    if ( !imp.Validate(seh, 0) ) {
        errors.Reset();
    }
    return errors;
}


CConstRef<CValidError> CValidator::Validate
(const CSeq_submit& ss,
 CScope* scope,
 Uint4 options)
{
    CRef<CValidError> errors(new CValidError(&ss));
    CValidError_imp imp(*m_ObjMgr, &(*errors), options);
    imp.Validate(ss, scope);
    return errors;
}


CConstRef<CValidError> CValidator::Validate
(const CSeq_annot_Handle& sah,
 Uint4 options)
{
    CConstRef<CSeq_annot> sar = sah.GetCompleteSeq_annot();
    CRef<CValidError> errors(new CValidError(&*sar));
    CValidError_imp imp(*m_ObjMgr, &(*errors), options);
    imp.Validate(sah);
    return errors;
}


void CValidator::SetProgressCallback(TProgressCallback callback, void* user_data)
{
    m_PrgCallback = callback;
    m_UserData = user_data;
}


END_SCOPE(validator)
END_SCOPE(objects)
END_NCBI_SCOPE
