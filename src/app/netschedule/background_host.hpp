#ifndef NETSCHEDULE_BACKGROUND_HOST__HPP
#define NETSCHEDULE_BACKGROUND_HOST__HPP

/*  $Id: background_host.hpp 146013 2008-11-19 16:38:44Z kazimird $
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
 * Authors:  Victor Joukov
 *
 * File Description: Host interface for background thread. Allows thread to
 *                   report errors and to learn is it needed to execute.
 *
 *
 */

BEGIN_NCBI_SCOPE

class CBackgroundHost
{
public:
    enum ESeverity {
        eWarning = 0,
        eError,
        eFatal
    };
    virtual ~CBackgroundHost() {}
    virtual void ReportError(ESeverity severity, const string& what) = 0;
    virtual bool ShouldRun() = 0;
};


END_NCBI_SCOPE

#endif /* NETSCHEDULE_BACKGROUND_HOST__HPP */
