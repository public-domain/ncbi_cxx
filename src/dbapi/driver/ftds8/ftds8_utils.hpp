#ifndef DBAPI_DRIVER_FTDS8___DBAPI_DRIVER_FTDS8_UTILS__HPP
#define DBAPI_DRIVER_FTDS8___DBAPI_DRIVER_FTDS8_UTILS__HPP

/* $Id: ftds8_utils.hpp 142999 2008-10-14 15:22:48Z ivanovp $
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
 * Author:  Sergey Sikorskiy
 *
 * File Description:  Small utility classes common to the ftds8 driver.
 *
 */

#include <dbapi/driver/impl/dbapi_driver_utils.hpp>
#include "../dbapi_driver_exception_storage.hpp"


BEGIN_NCBI_SCOPE

// Uncomment a line below if you want to simulate a previous ftds driver logic.
// #define FTDS_LOGIC

extern "C"
{

NCBI_DBAPIDRIVER_DBLIB_EXPORT
void
NCBI_EntryPoint_xdbapi_ftds8(
    CPluginManager<I_DriverContext>::TDriverInfoList&   info_list,
    CPluginManager<I_DriverContext>::EEntryPointRequest method);

} // extern C


/////////////////////////////////////////////////////////////////////////////
// Singleton

impl::CDBExceptionStorage& GetFTDS8ExceptionStorage(void);


END_NCBI_SCOPE


#endif // DBAPI_DRIVER_FTDS8___DBAPI_DRIVER_FTDS8_UTILS__HPP


