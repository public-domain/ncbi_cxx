/*  $Id: notif_thread.cpp 129154 2008-05-30 01:26:33Z joukovv $
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
 * Authors:  Anatoliy Kuznetsov
 *
 * File Description: Notification thread.
 *
 *
 */

#include <ncbi_pch.hpp>

#include "notif_thread.hpp"
#include "bdb_queue.hpp"

BEGIN_NCBI_SCOPE

class CQueueDataBase;

void CJobNotificationThread::DoJob(void)
{
    try {
        m_QueueDB.NotifyListeners();
    }
    catch(exception& ex)
    {
        RequestStop();
        ERR_POST(Error << "Error during notification: "
                        << ex.what()
                        << " notification thread has been stopped.");
    }
}

CJobNotificationThread::~CJobNotificationThread()
{
}


END_NCBI_SCOPE
