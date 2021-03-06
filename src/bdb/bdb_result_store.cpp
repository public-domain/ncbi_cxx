/*  $Id: bdb_result_store.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 * Author: Anatoliy Kuznetsov
 *
 * File Description:  Multi-volume result set implementation
 *
 */

#include <ncbi_pch.hpp>
#include <bdb/bdb_result_store.hpp>

BEGIN_NCBI_SCOPE

CBDB_ResultStoreBase::CBDB_ResultStoreBase()
  : CBDB_BLobFile()
{
    BindKey("subset_id", &subset_id);
    BindKey("volume_id", &volume_id);
    BindKey("format",    &format);
}

END_NCBI_SCOPE
