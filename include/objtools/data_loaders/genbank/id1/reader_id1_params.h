#ifndef GBLOADER_ID1_PARAMS__HPP_INCLUDED
#define GBLOADER_ID1_PARAMS__HPP_INCLUDED

/*  $Id: reader_id1_params.h 150711 2009-01-28 19:45:58Z vasilche $
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
*  ===========================================================================
*
*  Author: Eugene Vasilchenko
*
*  File Description:
*    GenBank reader id1 configuration parameters
*
* ===========================================================================
*/

/* Name of ID1 reader driver */
#define NCBI_GBLOADER_READER_ID1_DRIVER_NAME "id1"

/* Maximum number of simultaneous connection to id1 server */
#define NCBI_GBLOADER_READER_ID1_PARAM_NUM_CONN "no_conn"
/* Whether to open first connection immediately or not (default: true) */
#define NCBI_GBLOADER_READER_ID1_PARAM_PREOPEN  "preopen"
/* NCBI service name to connect to */
#define NCBI_GBLOADER_READER_ID1_PARAM_SERVICE_NAME "service"
/* Timeout of network connections in seconds */
#define NCBI_GBLOADER_READER_ID1_PARAM_TIMEOUT "timeout"
/* Number of retries on errors */
#define NCBI_GBLOADER_READER_ID1_PARAM_RETRY_COUNT "retry"

#endif
