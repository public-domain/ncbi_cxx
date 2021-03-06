#ifndef CONN___NETSCHEDULE_API_CONST__HPP
#define CONN___NETSCHEDULE_API_CONST__HPP

/*  $Id: netschedule_api_const.hpp 120679 2008-02-26 20:02:31Z kazimird $
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
 * Authors:  Anatoliy Kuznetsov, Maxim Didenko
 *
 * File Description:
 *   NetSchedule client API.
 *
 */


/// @file netschedule_client.hpp
/// NetSchedule client specs.
///

#include <corelib/ncbistl.hpp>


BEGIN_NCBI_SCOPE



/// @internal
const unsigned int kNetScheduleMaxDataSize = 512;
/// @internal
const unsigned int kNetScheduleMaxDBDataSize = kNetScheduleMaxDataSize * 4;

/// @internal
const unsigned int kNetScheduleMaxErrSize = 1024;
/// @internal
const unsigned int kNetScheduleMaxDBErrSize = kNetScheduleMaxErrSize * 4;



/* @} */


END_NCBI_SCOPE


#endif  /* CONN___NETSCHEDULE_API_CONST__HPP */
