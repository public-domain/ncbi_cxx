/*  $Id: ucscid.hpp 152006 2009-02-10 19:12:48Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   WIGGLE transient data structures
 *
 */

#ifndef OBJTOOLS_IDMAPPER___UCSCID__HPP
#define OBJTOOLS_IDMAPPER___UCSCID__HPP

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ============================================================================
class NCBI_XOBJREAD_EXPORT UcscID:
//  ============================================================================
    public CSeq_id
{
public:
    static string Label(
        const std::string&,
        const std::string& );
    
public:
    UcscID(
        const std::string& );
};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_IDMAPPER___UCSCID__HPP
