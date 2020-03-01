/* $Id: Mod.hpp 141754 2008-09-29 20:29:10Z slottad $
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

/// @file Mod.hpp
/// User-defined methods of the data storage class.
///
/// This file was originally generated by application DATATOOL
/// using the following specifications:
/// 'unimod.xsd'.
///
/// New methods or data members can be added to it if needed.
/// See also: Mod_.hpp


#ifndef ALGO_MS_FORMATS_UNIMOD_MOD_HPP
#define ALGO_MS_FORMATS_UNIMOD_MOD_HPP


// generated includes
#include <algo/ms/formats/unimod/Mod_.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

BEGIN_unimod_SCOPE // namespace ncbi::objects::unimod::

/////////////////////////////////////////////////////////////////////////////
class NCBI_UNIMOD_EXPORT CMod : public CMod_Base
{
    typedef CMod_Base Tparent;
public:
    // constructor
    CMod(void);
    // destructor
    ~CMod(void);
    bool isResidueSubset(const string residues);
    size_t name_distance(const string name);

private:
    // Prohibit copy constructor and assignment operator
    CMod(const CMod& value);
    CMod& operator=(const CMod& value);

};

/////////////////// CMod inline methods

// constructor
inline
CMod::CMod(void)
{
}


/////////////////// end of CMod inline methods


END_unimod_SCOPE // namespace ncbi::objects::unimod::

END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE


#endif // ALGO_MS_FORMATS_UNIMOD_MOD_HPP
/* Original file checksum: lines: 90, chars: 2417, CRC32: d9348d85 */