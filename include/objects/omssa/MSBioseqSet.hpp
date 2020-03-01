/* $Id: MSBioseqSet.hpp 103491 2007-05-04 17:18:18Z kazimird $
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

/// @file MSBioseqSet.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'omssa.asn'.
///
/// New methods or data members can be added to it if needed.
/// See also: MSBioseqSet_.hpp


#ifndef OBJECTS_OMSSA_MSBIOSEQSET_HPP
#define OBJECTS_OMSSA_MSBIOSEQSET_HPP


// generated includes
#include <objects/omssa/MSBioseqSet_.hpp>

#include <objects/seq/Bioseq.hpp>
// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

/////////////////////////////////////////////////////////////////////////////
class NCBI_OMSSA_EXPORT CMSBioseqSet : public CMSBioseqSet_Base
{
    typedef CMSBioseqSet_Base Tparent;
public:
    // constructor
    CMSBioseqSet(void);
    // destructor
    ~CMSBioseqSet(void);

    /**
     * Retrieve a bioseq based on oid
     * 
     * @param oid
     * @return bioseq or null if not found
     */
    CConstRef <CMSBioseq::TSeq> GetBioseqByOid(const int Oid) const;

private:
    // Prohibit copy constructor and assignment operator
    CMSBioseqSet(const CMSBioseqSet& value);
    CMSBioseqSet& operator=(const CMSBioseqSet& value);

};

/////////////////// CMSBioseqSet inline methods

// constructor
inline
CMSBioseqSet::CMSBioseqSet(void)
{
}


/////////////////// end of CMSBioseqSet inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_OMSSA_MSBIOSEQSET_HPP
/* Original file checksum: lines: 94, chars: 2609, CRC32: ece3846c */
