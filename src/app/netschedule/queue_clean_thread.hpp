#ifndef NETSCHEDULE_QUEUE_CLEAN_THREAD__HPP
#define NETSCHEDULE_QUEUE_CLEAN_THREAD__HPP

/*  $Id: queue_clean_thread.hpp 129154 2008-05-30 01:26:33Z joukovv $
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
 * Authors:  Anatoliy Kuznetsov, Victor Joukov
 *
 * File Description: Queue cleaning thread.
 *
 *
 */

#include "background_host.hpp"
#include <util/thread_nonstop.hpp>

BEGIN_NCBI_SCOPE

class CQueueDataBase;

/// Thread class, removes obsolete job records
///
/// @internal
class CJobQueueCleanerThread : public CThreadNonStop
{
public:
    CJobQueueCleanerThread(CBackgroundHost& host,
                           CQueueDataBase& qdb,
                           unsigned run_delay)
    : CThreadNonStop(run_delay),
      m_Host(host),
      m_QueueDB(qdb)
#ifdef _DEBUG
      , m_DbgTriggerDBRecover(false)
#endif
    {}

    virtual void DoJob(void);
private:
    CJobQueueCleanerThread(const CJobQueueCleanerThread&);
    CJobQueueCleanerThread& operator=(const CJobQueueCleanerThread&);
private:
    CBackgroundHost& m_Host;
    CQueueDataBase&  m_QueueDB;
#ifdef _DEBUG
    bool m_DbgTriggerDBRecover;
#endif
};


/// Thread class, watches job execution, reschedules forgotten jobs
///
/// @internal
class CJobQueueExecutionWatcherThread : public CThreadNonStop
{
public:
    CJobQueueExecutionWatcherThread(CBackgroundHost& host,
                                    CQueueDataBase& qdb,
                                    unsigned run_delay)
    : CThreadNonStop(run_delay),
      m_Host(host),
      m_QueueDB(qdb)
    {}

    virtual void DoJob(void);
private:
    CJobQueueExecutionWatcherThread(const CJobQueueExecutionWatcherThread&);
    CJobQueueExecutionWatcherThread&
        operator=(const CJobQueueExecutionWatcherThread&);
private:
    CBackgroundHost& m_Host;
    CQueueDataBase&  m_QueueDB;
};


END_NCBI_SCOPE

#endif /* NETSCHEDULE_QUEUE_CLEAN_THREAD__HPP */
