#ifndef NETSCHEDULE_BDB_QUEUE__HPP
#define NETSCHEDULE_BDB_QUEUE__HPP


/*  $Id: bdb_queue.hpp 148064 2008-12-18 20:09:11Z joukovv $
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
 * File Description:
 *   NetSchedule job status database.
 *
 */


/// @file bdb_queue.hpp
/// NetSchedule job status database.
///
/// @internal


#include <corelib/ncbimtx.hpp>
#include <corelib/ncbicntr.hpp>

#include <connect/services/netschedule_api.hpp>
#include "ns_util.hpp"
#include "job_status.hpp"
#include "queue_clean_thread.hpp"
#include "notif_thread.hpp"
#include "squeue.hpp"
#include "queue_vc.hpp"
#include "background_host.hpp"
#include "worker_node.hpp"

BEGIN_NCBI_SCOPE


struct SQueueDescription
{
    const char*           host;
    unsigned              port;
    const char*           queue;
    map<unsigned, string> host_name_cache;
};


struct SFieldsDescription
{
    enum EFieldType {
        eFT_Job,
        eFT_Tag,
        eFT_Run,
        eFT_RunNum
    };
    typedef string (*FFormatter)(const string&, SQueueDescription*);

    vector<int>    field_types;
    vector<int>    field_nums;
    vector<FFormatter> formatters;
    bool           has_tags;
    vector<string> pos_to_tag;
    bool           has_affinity;
    bool           has_run;

    void Init() {
        field_types.clear();
        field_nums.clear();
        formatters.clear();
        pos_to_tag.clear();
        has_tags = false;
        has_affinity = false;
        has_run = false;
    }
};


class CQueueDataBase;

/// Main queue entry point
///
/// @internal
///
class CQueue
{
public:
    /// @param client_host_addr - 0 means internal use
    CQueue(CQueueDataBase&    db,
           CRef<SLockedQueue> queue,
           unsigned           client_host_addr);

    /// Is the queue empty long enough to be deleted?
    bool IsExpired(void);

    unsigned Submit(CJob& job);

    /// Submit job batch
    /// @return
    ///    ID of the first job, second is first_id+1 etc.
    unsigned SubmitBatch(vector<CJob>& batch);

    void Cancel(unsigned job_id);

    /// Move job to pending ignoring its current status
    void ForceReschedule(unsigned job_id);

    // Worker node-specific methods
    void PutResult(SWorkerNodeInfo& node_info,
                   unsigned         job_id,
                   int              ret_code,
                   const string*    output);

    void GetJob(SWorkerNodeInfo&    node_info,
                CRequestContextFactory* rec_ctx_f,
                const list<string>* aff_list,
                CJob*               new_job);

    void PutResultGetJob(SWorkerNodeInfo& node_info,
                         // PutResult parameters
                         unsigned         done_job_id,
                         int              ret_code,
                         const string*    output,
                         // GetJob parameters
                         CRequestContextFactory* rec_ctx_f,
                         const list<string>* aff_list,
                         CJob*            new_job);

    bool PutProgressMessage(unsigned      job_id,
                            const string& msg);

    void FailJob(const SWorkerNodeInfo& node_info,
                 unsigned               job_id,
                 const string&          err_msg,
                 const string&          output,
                 int                    ret_code);

    void ReturnJob(const SWorkerNodeInfo& node_info, unsigned job_id);

    /// @param expected_status
    ///    If current status is different from expected try to
    ///    double read the database (to avoid races between nodes
    ///    and submitters)
    bool GetJobDescr(unsigned   job_id,
                     int*       ret_code,
                     string*    input,
                     string*    output,
                     string*    err_msg,
                     string*    progress_msg,
                     TJobStatus expected_status
                         = CNetScheduleAPI::eJobNotFound);

    TJobStatus GetStatus(unsigned job_id) const;

    /// count status snapshot for affinity token
    /// returns false if affinity token not found
    bool CountStatus(CJobStatusTracker::TStatusSummaryMap* status_map,
                     const string&                         affinity_token);

    /// Prolong job expiration timeout
    /// @param tm
    ///    Time worker node needs to execute the job (in seconds)
    void JobDelayExpiration(SWorkerNodeInfo& node_info,
                            unsigned         job_id,
                            time_t           tm);

    /// Check jobs for expiry and if expired, delete up to batch_size jobs
    /// @return
    ///    Number of deleted jobs
    unsigned CheckJobsExpiry(unsigned batch_size, TJobStatus status);

    /// Delete batch_size jobs using jobs-to-delete vector
    /// @return
    ///    Number of deleted jobs
    unsigned DoDeleteBatch(unsigned batch_size);

    /// Delete all job ids already deleted (physically) from the queue
    void ClearAffinityIdx(void);

    /// Remove all jobs
    void Truncate(void);

    /// Remove job from the queue
    void DropJob(unsigned job_id);

    // Read-Confirm stage
    /// Request done jobs for reading with timeout
    void ReadJobs(unsigned peer_addr,
                  unsigned count, unsigned timeout,
                  unsigned& read_id, TNSBitVector& jobs);
    /// Confirm reading of these jobs
    void ConfirmJobs(unsigned read_id, TNSBitVector& jobs);
    /// Fail (negative acknoledge) reading of these jobs
    void FailReadingJobs(unsigned read_id, TNSBitVector& jobs);
    /// Get affinity preference list
    string GetAffinityList(SWorkerNodeInfo& node_info);

    // Make new worker node record with id, host and port
    // Also check that old nodes with same (host, port) are cleared
    void InitWorkerNode(const SWorkerNodeInfo& node_info);
    /// Clear all jobs, still running for node.
    /// Fails all such jobs, called by external node watcher, can safely
    /// clean out node's record
    void ClearWorkerNode(const string& node_id);

    /// @param host_addr
    ///    host address in network BO
    ///
    void RegisterNotificationListener(const SWorkerNodeInfo& node_info,
                                      unsigned short         port,
                                      unsigned               timeout);
    void UnRegisterNotificationListener(const SWorkerNodeInfo& node_info);
    void RegisterWorkerNodeVisit(SWorkerNodeInfo& node_info);

    /// Remove affinity association for a specified host
    ///
    /// @note Affinity is based on worker node id
    void ClearAffinity(const string& node_id);

    /// Pass socket for monitor
    void SetMonitorSocket(CSocket& socket);
    /// Are we monitoring?
    bool IsMonitoring();
    /// Send string to monitor
    void MonitorPost(const string& msg);

    /// UDP notification to all listeners
    void NotifyListeners(bool unconditional=false,
                         unsigned aff_id=0);

    /// Check execution timeout. Now checks reading timeout as well.
    /// All jobs failed to execute, go back to pending
    void CheckExecutionTimeout(void);

    unsigned CountStatus(TJobStatus) const;

    void StatusStatistics(TJobStatus status,
                          TNSBitVector::statistics* st) const;

    /// Count database records
    unsigned CountRecs() const;

    unsigned GetMaxInputSize() const;
    unsigned GetMaxOutputSize() const;

    unsigned GetNumParams() const;
    string GetParamName(unsigned n) const;
    string GetParamValue(unsigned n) const;

    void PrintStat(CNcbiOstream& out);
    void PrintWorkerNodeStat(CNcbiOstream& out) const;
    void PrintSubmHosts(CNcbiOstream& out) const;
    void PrintWNodeHosts(CNcbiOstream& out) const;
    void PrintQueue(CNcbiOstream& out,
                    TJobStatus    job_status,
                    const string& host,
                    unsigned      port);

    TNSBitVector* ExecSelect(const string& query, list<string>& fields);
    typedef vector<string>  TRecord;
    typedef vector<TRecord> TRecordSet;
    void PrepareFields(SFieldsDescription& field_descr,
                       const list<string>& fields);
    void ExecProject(TRecordSet&               record_set,
                     const TNSBitVector&       ids,
                     const SFieldsDescription& field_descr);
    bool x_FillRecord(vector<string>& record,
                      const SFieldsDescription& field_descr,
                      const CJob& job,
                      map<string, string>& tags,
                      const CJobRun* run,
                      int run_num);
    /// Queue dump
    void PrintJobDbStat(unsigned      job_id,
                        CNcbiOstream& out,
                        TJobStatus    status
                            = CNetScheduleAPI::eJobNotFound);
    /// Dump all job records
    void PrintAllJobDbStat(CNcbiOstream & out);

    void PrintJobStatusMatrix(CNcbiOstream & out);

    // Access control
    bool IsDenyAccessViolations() const;
    bool IsLogAccessViolations() const;
    bool IsVersionControl() const;
    bool IsMatchingClient(const CQueueClientInfo& cinfo) const;
    /// Check if client is a configured submitter
    bool IsSubmitAllowed() const;
    /// Check if client is a configured worker node
    bool IsWorkerAllowed() const;


    typedef SLockedQueue::TStatEvent TStatEvent;
    double GetAverage(TStatEvent);

    // Service for CQueueDataBase
    /// Free unsued memory (status storage)
    void OptimizeMem(void)
        { x_GetLQueue()->OptimizeMem(); }

    // BerkeleyDB-specific statistics
    void PrintMutexStat(CNcbiOstream& out);
    void PrintLockStat(CNcbiOstream& out);
    void PrintMemStat(CNcbiOstream& out);


private:
    // Support of CQueue iterators
    friend class CQueueIterator;
    CQueue(const CQueueDataBase& db) :
        m_Db(const_cast<CQueueDataBase&>(db)), // HACK! - legitimate here,
                                               // because iterator access is
                                               // const, and this constructor
                                               // is for CQueueIterator only
        m_ClientHostAddr(0) {}

    // Helper method for CQueueIterator so that it can return the same
    // CQueue object reference every iteration, only modifying underlying
    // SLockedQueue.
    void x_Assume(CRef<SLockedQueue> slq) { m_LQueue = slq; }

    //
    time_t x_ComputeExpirationTime(time_t time_run,
                                   time_t run_timeout) const;

    void x_PrintJobStat(const CJob&   job,
                        unsigned      queue_run_timeout,
                        CNcbiOstream& out,
                        const char*   fsp = "\n",
                        bool          fflag = true);
    void x_PrintShortJobStat(const CJob&   job,
                             const string& host,
                             unsigned      port,
                             CNcbiOstream& out,
                             const char*   fsp = "\t");

    /// @return TRUE if job record has been found and updated
    bool x_UpdateDB_PutResultNoLock(unsigned             job_id,
                                    time_t               curr,
                                    bool                 delete_done,
                                    int                  ret_code,
                                    const string&        output,
                                    CJob&                job);

    enum EGetJobUpdateStatus
    {
        eGetJobUpdate_Ok,
        eGetJobUpdate_NotFound,
        eGetJobUpdate_JobStopped,
        eGetJobUpdate_JobFailed
    };
    EGetJobUpdateStatus x_UpdateDB_GetJobNoLock(
                                const SWorkerNodeInfo& node_info,
                                time_t                 curr,
                                unsigned               job_id,
                                CJob&                  job);

    CRef<SLockedQueue> x_GetLQueue(void);
    const CRef<SLockedQueue> x_GetLQueue(void) const;

    void x_AddToTimeLine(unsigned job_id, time_t curr);
    void x_RemoveFromTimeLine(unsigned job_id);
    void x_TimeLineExchange(unsigned remove_job_id,
                            unsigned add_job_id,
                            time_t   curr);

private:
    CQueue(const CQueue&);
    CQueue& operator=(const CQueue&);

private:
    // Per queue data
    CQueueDataBase&    m_Db;      ///< Parent structure reference
    CWeakRefOrig<SLockedQueue> m_LQueue;
    unsigned           m_ClientHostAddr;
}; // CQueue


/// Queue database manager
///
/// @internal
///
class CQueueIterator;
class CQueueCollection
{
public:
    typedef map<string, CRef<SLockedQueue> > TQueueMap;
    typedef CQueueIterator iterator;
    typedef CQueueIterator const_iterator; // HACK

public:
    CQueueCollection(const CQueueDataBase& db);
    ~CQueueCollection();

    void Close();

    CRef<SLockedQueue> GetQueue(const string& qname) const;
    bool QueueExists(const string& qname) const;

    /// Collection takes ownership of queue
    SLockedQueue& AddQueue(const string& name, SLockedQueue* queue);
    // Remove queue from collection, notifying every interested party
    // through week reference mechanism.
    bool RemoveQueue(const string& name);

    CQueueIterator begin() const;
    CQueueIterator end() const;

    size_t GetSize() const { return m_QMap.size(); }

private:
    CQueueCollection(const CQueueCollection&);
    CQueueCollection& operator=(const CQueueCollection&);
private:
    friend class CQueueIterator;
    const CQueueDataBase&  m_QueueDataBase;
    TQueueMap              m_QMap;
    mutable CRWLock        m_Lock;
};



class CQueueIterator
{
public:
    CQueueIterator(const CQueueDataBase& db,
        CQueueCollection::TQueueMap::const_iterator iter, CRWLock* lock);
    // MSVC8 (Studio 2005) can not (or does not want to) perform
    // copy constructor optimization on return, thus it needs
    // explicit copy constructor because CQueue does not have it.
    CQueueIterator(const CQueueIterator& rhs);
    ~CQueueIterator();
    CQueue& operator*();
    const string GetName();
    void operator++();
    bool operator==(const CQueueIterator& rhs) {
        return m_Iter == rhs.m_Iter;
    }
    bool operator!=(const CQueueIterator& rhs) {
        return m_Iter != rhs.m_Iter;
    }
private:
    const CQueueDataBase&                       m_QueueDataBase;
    CQueueCollection::TQueueMap::const_iterator m_Iter;
    CQueue                                      m_Queue;
    mutable CRWLock*                            m_Lock;
};


struct SNSDBEnvironmentParams
{
    string    db_path;
    string    db_log_path;
    unsigned  max_queues;          ///< Number of pre-allocated queues
    unsigned  cache_ram_size;      ///< Size of database cache
    unsigned  mutex_max;           ///< Number of mutexes
    unsigned  max_locks;           ///< Number of locks
    unsigned  max_lockers;         ///< Number of lockers
    unsigned  max_lockobjects;     ///< Number of lock objects
    ///    Size of in-memory LOG (when 0 log is put to disk)
    ///    In memory LOG is not durable, put it to memory
    ///    only if you need better performance
    unsigned  log_mem_size;
    unsigned  max_trans;           ///< Maximum number of active transactions
    unsigned  checkpoint_kb;
    unsigned  checkpoint_min;
    bool      sync_transactions;
    bool      direct_db;
    bool      direct_log;
    bool      private_env;

    bool Read(const IRegistry& reg, const string& sname);

    unsigned GetNumParams() const;
    string GetParamName(unsigned n) const;
    string GetParamValue(unsigned n) const;
};

/// Top level queue database.
/// (Thread-Safe, synchronized.)
///
/// @internal
///
class CQueueDataBase
{
public:
    CQueueDataBase(CBackgroundHost& host);
    ~CQueueDataBase();

    /// @param params
    ///    Parameters of DB environment
    /// @param reinit
    ///    Whether to clean out database
    void Open(const SNSDBEnvironmentParams& params, bool reinit);

    // Read queue information from registry and configure queues
    // accordingly.
    // returns minimum run timeout, necessary for watcher thread
    unsigned Configure(const IRegistry& reg);

    CQueue* OpenQueue(const string& name, unsigned peer_addr);

    typedef SLockedQueue::TQueueKind TQueueKind;
    void MountQueue(const string& qname,
                    const string& qclass,
                    TQueueKind    kind,
                    const SQueueParameters& params,
                    SQueueDbBlock* queue_db_block);

    void CreateQueue(const string& qname, const string& qclass,
                     const string& comment = "");
    void DeleteQueue(const string& qname);
    void QueueInfo(const string& qname, int& kind,
                   string* qclass, string* comment);
    string GetQueueNames(const string& sep) const;

    void UpdateQueueParameters(const string& qname,
                               const SQueueParameters& params);

    void Close(void);
    bool QueueExists(const string& qname) const
                { return m_QueueCollection.QueueExists(qname); }

    /// Remove old jobs
    void Purge(void);
    void StopPurge(void);
    void RunPurgeThread(void);
    void StopPurgeThread(void);

    /// Notify all listeners
    void NotifyListeners(void);
    void RunNotifThread(void);
    void StopNotifThread(void);

    void CheckExecutionTimeout(void);
    void RunExecutionWatcherThread(unsigned run_delay);
    void StopExecutionWatcherThread(void);


    void SetUdpPort(unsigned short port) { m_UdpPort = port; }
    unsigned short GetUdpPort(void) const { return m_UdpPort; }

    /// Force transaction checkpoint
    void TransactionCheckPoint(bool clean_log=false);

    // BerkeleyDB-specific statistics
    void PrintMutexStat(CNcbiOstream& out) {
        m_Env->PrintMutexStat(out);
    }
    void PrintLockStat(CNcbiOstream& out) {
        m_Env->PrintLockStat(out);
    }
    void PrintMemStat(CNcbiOstream& out) {
        m_Env->PrintMemStat(out);
    }


private:
    // No copy
    CQueueDataBase(const CQueueDataBase&);
    CQueueDataBase& operator=(const CQueueDataBase&);

protected:
    /// get next job id (counter increment)
    unsigned GetNextId();

    /// Returns first id for the batch
    unsigned GetNextIdBatch(unsigned count);

private:
    int x_AllocateQueue(const string& qname, const string& qclass,
                        int kind, const string& comment);

    unsigned x_PurgeUnconditional(unsigned batch_size);
    void x_OptimizeStatusMatrix(void);
    bool x_CheckStopPurge(void);
    void x_CleanParamMap(void);

    CBackgroundHost&                m_Host;
    CBDB_Env*                       m_Env;
    string                          m_Path;
    string                          m_Name;

    mutable CFastMutex              m_ConfigureLock;
    typedef map<string, SQueueParameters*> TQueueParamMap;
    TQueueParamMap                  m_QueueParamMap;
    SQueueDescriptionDB             m_QueueDescriptionDB;
    CQueueCollection                m_QueueCollection;
    
    // Pre-allocated Berkeley DB blocks
    CQueueDbBlockArray              m_QueueDbBlockArray;

    CRef<CJobQueueCleanerThread>    m_PurgeThread;

    bool                 m_StopPurge;         ///< Purge stop flag
    CFastMutex           m_PurgeLock;
    unsigned             m_FreeStatusMemCnt;  ///< Free memory counter
    time_t               m_LastFreeMem;       ///< time of the last memory opt
    unsigned short       m_UdpPort;           ///< UDP notification port

    CRef<CJobNotificationThread>             m_NotifThread;
    CRef<CJobQueueExecutionWatcherThread>    m_ExeWatchThread;

}; // CQueueDataBase


END_NCBI_SCOPE

#endif /* NETSCHEDULE_BDB_QUEUE__HPP */
