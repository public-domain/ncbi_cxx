/* $Id: Id_pat.hpp 149902 2009-01-15 23:55:28Z ucko $
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
 *   'biblio.asn'.
 */

#ifndef OBJECTS_BIBLIO_ID_PAT_HPP
#define OBJECTS_BIBLIO_ID_PAT_HPP


// generated includes
#include <objects/biblio/Id_pat_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class NCBI_BIBLIO_EXPORT CId_pat : public CId_pat_Base
{
    typedef CId_pat_Base Tparent;
public:
    // constructor
    CId_pat(void);
    // destructor
    ~CId_pat(void);

    // comparison function
    bool Match(const CId_pat& idp2) const;
    static bool Id_Match(const C_Id& id1, const C_Id& id2);

    // Appends a label onto "label" based on content
    void GetLabel(string* label) const;

    // may return either actual number or application number
    const string& GetSomeNumber(void) const;

private:
    // Prohibit copy constructor & assignment operator
    CId_pat(const CId_pat&);
    CId_pat& operator= (const CId_pat&);
};



/////////////////// CId_pat inline methods

// constructor
inline
CId_pat::CId_pat(void)
{
}


/////////////////// end of CId_pat inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_BIBLIO_ID_PAT_HPP
/* Original file checksum: lines: 85, chars: 2206, CRC32: 9ef520cb */
