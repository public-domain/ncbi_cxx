/* $Id: soap_body.hpp 103491 2007-05-04 17:18:18Z kazimird $
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
 */

/// @file soap_body.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'soap_11.xsd'.
///
/// New methods or data members can be added to it if needed.
/// See also: soap_body_.hpp


#ifndef SOAP_BODY_HPP
#define SOAP_BODY_HPP


// generated includes
#include <serial/soap/soap_body_.hpp>

BEGIN_NCBI_SCOPE

// generated classes

/////////////////////////////////////////////////////////////////////////////
class CSoapBody : public CSoapBody_Base
{
    typedef CSoapBody_Base Tparent;
public:
    // constructor
    CSoapBody(void);
    // destructor
    ~CSoapBody(void);

private:
    // Prohibit copy constructor and assignment operator
    CSoapBody(const CSoapBody& value);
    CSoapBody& operator=(const CSoapBody& value);

};

/////////////////// CSoapBody inline methods

// constructor
inline
CSoapBody::CSoapBody(void)
{
}


/////////////////// end of CSoapBody inline methods

END_NCBI_SCOPE

#endif // SOAP_BODY_HPP
