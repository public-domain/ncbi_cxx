/* $Id: ValidError.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 *   using the following specifications:
 *   'valerr.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <objects/seq/Seqdesc.hpp>

// generated includes
#include <objects/valerr/ValidError.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// *********************** CValidError implementation **********************


CValidError::CValidError(const CSerialObject* obj) :
    m_Validated(obj)
{
}

void CValidError::AddValidErrItem
(EDiagSev             sev,
 unsigned int         ec,
 const string&        msg,
 const string&        desc,
 const CSerialObject& obj,
 const string&        acc)
{
    CRef<CValidErrItem> item(new CValidErrItem(sev, ec, msg, desc, obj, acc));
    SetErrs().push_back(item);
    m_Stats[item->GetSeverity()]++;
}


void CValidError::AddValidErrItem
(EDiagSev             sev,
 unsigned int         ec,
 const string&        msg,
 const string&        desc,
 const CSeqdesc&      seqdesc,
 const CSeq_entry&    ctx,
const string&         acc)
{
    CRef<CValidErrItem> item(new CValidErrItem(sev, ec, msg, desc, seqdesc, ctx, acc));
    SetErrs().push_back(item);
    m_Stats[item->GetSeverity()]++;
}


CValidError::~CValidError()
{
}


// ************************ CValidError_CI implementation **************

CValidError_CI::CValidError_CI(void) :
    m_Validator(0),
    m_ErrCodeFilter(kEmptyStr), // eErr_UNKNOWN
    m_MinSeverity(eDiagSevMin),
    m_MaxSeverity(eDiagSevMax)
{
}


CValidError_CI::CValidError_CI
(const CValidError& ve,
 const string& errcode,
 EDiagSev           minsev,
 EDiagSev           maxsev) :
    m_Validator(&ve),
    m_Current(ve.GetErrs().begin()),
    m_ErrCodeFilter(errcode),
    m_MinSeverity(minsev),
    m_MaxSeverity(maxsev)
{
    if ( IsValid()  &&  !Filter(**m_Current) ) {
        Next();
    }
}


CValidError_CI::CValidError_CI(const CValidError_CI& other)
{
    if ( this != &other ) {
        *this = other;
    }
}


CValidError_CI::~CValidError_CI(void)
{
}


CValidError_CI& CValidError_CI::operator=(const CValidError_CI& iter)
{
    if (this == &iter) {
        return *this;
    }

    m_Validator = iter.m_Validator;
    m_Current = iter.m_Current;
    m_ErrCodeFilter = iter.m_ErrCodeFilter;
    m_MinSeverity = iter.m_MinSeverity;
    m_MaxSeverity = iter.m_MaxSeverity;
    return *this;
}


CValidError_CI& CValidError_CI::operator++(void)
{
    Next();
    return *this;
}


bool CValidError_CI::IsValid(void) const
{
    return m_Current != m_Validator->GetErrs().end();
}


const CValidErrItem& CValidError_CI::operator*(void) const
{
    return **m_Current;
}


const CValidErrItem* CValidError_CI::operator->(void) const
{
    return &(**m_Current);
}


bool CValidError_CI::Filter(const CValidErrItem& item) const
{
    EDiagSev item_sev = (*m_Current)->GetSeverity();
    if ( (m_ErrCodeFilter.empty()  ||  
          NStr::StartsWith(item.GetErrCode(), m_ErrCodeFilter))  &&
         ((item_sev >= m_MinSeverity)  &&  (item_sev <= m_MaxSeverity)) ) {
        return true;;
    }
    return false;
}


void CValidError_CI::Next(void)
{
    if ( AtEnd() ) {
        return;
    }

    do {
        ++m_Current;
    } while ( !AtEnd()  &&  !Filter(**m_Current) );
}


bool CValidError_CI::AtEnd(void) const
{
    return m_Current == m_Validator->GetErrs().end();
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 64, chars: 1869, CRC32: b677bbc2 */