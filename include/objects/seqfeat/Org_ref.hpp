/* $Id: Org_ref.hpp 133846 2008-07-14 20:21:46Z kans $
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
 *   'seqfeat.asn'.
 */

#ifndef OBJECTS_SEQFEAT_ORG_REF_HPP
#define OBJECTS_SEQFEAT_ORG_REF_HPP


// generated includes
#include <objects/seqfeat/Org_ref_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class NCBI_SEQFEAT_EXPORT COrg_ref : public COrg_ref_Base
{
    typedef COrg_ref_Base Tparent;
public:
    // constructor
    COrg_ref(void);
    // destructor
    ~COrg_ref(void);
    
    // Appends a label to "label" based on content
    void GetLabel(string* label) const;

    // Returns NCBI Taxonomy database id (AKA tax id)
    // if the latter is found in Org_ref; otherwise returns 0
    int GetTaxId() const;
    // Sets tax id into Org_ref contents.
    // Returns old value of tax id or 0 if it was not found
    int SetTaxId( int tax_id );

    // shortcut access to selected OrgName methods
    bool IsSetLineage(void) const;
    const string& GetLineage(void) const;
    
    bool IsSetGcode(void) const;
    int GetGcode(void) const;
    
    bool IsSetMgcode(void) const;
    int GetMgcode(void) const;
    
    bool IsSetDivision(void) const;
    const string& GetDivision(void) const;
    
    bool IsSetOrgMod(void) const;
    
private:
    // Prohibit copy constructor and assignment operator
    COrg_ref(const COrg_ref& value);
    COrg_ref& operator=(const COrg_ref& value);
};



/////////////////// COrg_ref inline methods

// constructor
inline
COrg_ref::COrg_ref(void)
{
}


/////////////////// end of COrg_ref inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // OBJECTS_SEQFEAT_ORG_REF_HPP
/* Original file checksum: lines: 90, chars: 2371, CRC32: 49735a91 */