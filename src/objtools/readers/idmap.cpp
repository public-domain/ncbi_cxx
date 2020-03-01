/*  $Id: idmap.cpp 151845 2009-02-09 14:50:16Z ludwigf $
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
 * Author:  Frank Ludwig
 *
 * File Description:
 *   WIGGLE transient data structures
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiapp.hpp>

// Objects includes
#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>

#include <objtools/readers/ucscid.hpp>

#include "idmap.hpp"

BEGIN_NCBI_SCOPE
BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ============================================================================
CIdMap::CIdMap()
//  ============================================================================
{
};

//  ============================================================================
CIdMap::~CIdMap()
//  ============================================================================
{
};

//  ============================================================================
bool
CIdMap::AddMapping(
    const string& strkey,
    CRef<CSeq_id> value,
    unsigned int uLength )
//  ============================================================================
{
    m_IdMap[ strkey ] = CIdMapValue( 
        CSeq_id_Handle::GetHandle( *value ), uLength );
    return true;
};

//  ============================================================================
CSeq_id_Handle
CIdMap::GetMapping(
    const string& strkey,
    unsigned int& uLength )
//  ============================================================================
{
    IdCiter it = m_IdMap.find( strkey );
    if ( it == m_IdMap.end() ) {
        return CSeq_id_Handle();
    }
    uLength = it->second.m_uLength;
    return it->second.m_Id;
};

//  ============================================================================
void
CIdMap::ClearAll()
//  ============================================================================
{
    m_IdMap.clear();
};

//  ============================================================================
void
CIdMap::Dump(
    CNcbiOstream& out,
    const string& strPrefix )
//  ============================================================================
{
    out << strPrefix << "[CIdMap:" << endl;
    for ( IdCiter it = m_IdMap.begin(); it != m_IdMap.end(); ++it ) {
        out << strPrefix << strPrefix << it->first 
            << " ===> " << it->second.m_Id.GetSeqId()->AsFastaString() << endl;
    }
    out << strPrefix << "]" << endl;
};

//  ============================================================================
CSeq_id_Handle
CIdMap::Handle(
    const CSeq_id& key )
//  ============================================================================
{
    return CSeq_id_Handle::GetHandle( key );
};
    
END_objects_SCOPE
END_NCBI_SCOPE
