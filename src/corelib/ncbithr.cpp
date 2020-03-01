/*  $Id: ncbithr.cpp 147541 2008-12-11 16:23:32Z ivanovp $
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
 * Author:  Denis Vakatov, Aleksey Grichenko
 *
 * File Description:
 *   Multi-threading -- classes, functions, and features.
 *
 *    TLS:
 *      CTlsBase         -- TLS implementation (base class for CTls<>)
 *
 *    THREAD:
 *      CThread          -- thread wrapper class
 *
 *    RW-LOCK:
 *      CInternalRWLock  -- platform-dependent RW-lock structure (fwd-decl)
 *      CRWLock          -- Read/Write lock related  data and methods
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbithr.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbi_safe_static.hpp>
#include <corelib/ncbi_limits.h>
#include <corelib/ncbi_system.hpp>
#include <corelib/error_codes.hpp>
#include <assert.h>
#ifdef NCBI_POSIX_THREADS
#  include <sys/time.h> // for gettimeofday()
#endif

#include "ncbidbg_p.hpp"


#define NCBI_USE_ERRCODE_X   Corelib_Threads

BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//  CTlsBase::
//


DEFINE_STATIC_MUTEX(s_TlsCleanupMutex);


CUsedTlsBases::CUsedTlsBases(void)
{
}


CUsedTlsBases::~CUsedTlsBases(void)
{
    ClearAll();
}


void CUsedTlsBases::ClearAll(void)
{
    CMutexGuard tls_cleanup_guard(s_TlsCleanupMutex);
    NON_CONST_ITERATE(TTlsSet, it, m_UsedTls) {
        CTlsBase* tls = *it;
        tls->x_DeleteTlsData();
        if (tls->m_AutoDestroy) {
            tls->RemoveReference();
        }
    }
    m_UsedTls.clear();
}


void CUsedTlsBases::Register(CTlsBase* tls)
{
    CMutexGuard tls_cleanup_guard(s_TlsCleanupMutex);
    if (tls->m_AutoDestroy) {
        tls->AddReference();
    }
    m_UsedTls.insert(tls);
}


void CUsedTlsBases::Deregister(CTlsBase* tls)
{
    CMutexGuard tls_cleanup_guard(s_TlsCleanupMutex);
    m_UsedTls.erase(tls);
    if (tls->m_AutoDestroy) {
        tls->RemoveReference();
    }
}


static CSafeStaticPtr<CUsedTlsBases> s_MainUsedTls;


void CTlsBase::x_Init(void)
{
    // Create platform-dependent TLS key (index)
#if defined(NCBI_WIN32_THREADS)
    xncbi_Verify((m_Key = TlsAlloc()) != DWORD(-1));
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Verify(pthread_key_create(&m_Key, 0) == 0);
    // pthread_key_create does not reset the value to 0 if the key has been
    // used and deleted.
    xncbi_Verify(pthread_setspecific(m_Key, 0) == 0);
#else
    m_Key = 0;
#endif

    m_Initialized = true;
}


void CTlsBase::x_Destroy(void)
{
    x_Reset();
    m_Initialized = false;

    // Destroy system TLS key
#if defined(NCBI_WIN32_THREADS)
    if ( TlsFree(m_Key) ) {
        m_Key = 0;
        return;
    }
    assert(0);
#elif defined(NCBI_POSIX_THREADS)
    if (pthread_key_delete(m_Key) == 0) {
        m_Key = 0;
        return;
    }
    assert(0);
#else
    m_Key = 0;
    return;
#endif
}


// Platform-specific TLS data storing
inline
void s_TlsSetValue(TTlsKey& key, void* data, const char* err_message)
{
#if defined(NCBI_WIN32_THREADS)
    xncbi_Validate(TlsSetValue(key, data), err_message);
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Validate(pthread_setspecific(key, data) == 0, err_message);
#else
    key = data;
    assert(err_message);  // to get rid of the "unused variable" warning
#endif
}


void CTlsBase::x_SetValue(void*        value,
                          FCleanupBase cleanup,
                          void*        cleanup_data)
{
    if ( !m_Initialized ) {
        return;
    }

    // Get previously stored data
    STlsData* tls_data = static_cast<STlsData*> (x_GetTlsData());

    // Create and initialize TLS structure, if it was not present
    if ( !tls_data ) {
        tls_data = new STlsData;
        xncbi_Validate(tls_data != 0,
                       "CTlsBase::x_SetValue() -- cannot allocate "
                       "memory for TLS data");
        tls_data->m_Value       = 0;
        tls_data->m_CleanupFunc = 0;
        tls_data->m_CleanupData = 0;
    }

    // Cleanup
    if (tls_data->m_CleanupFunc  &&  tls_data->m_Value != value) {
        tls_data->m_CleanupFunc(tls_data->m_Value, tls_data->m_CleanupData);
    }

    // Store the values
    tls_data->m_Value       = value;
    tls_data->m_CleanupFunc = cleanup;
    tls_data->m_CleanupData = cleanup_data;

    // Store the structure in the TLS
    s_TlsSetValue(m_Key, tls_data,
                  "CTlsBase::x_SetValue() -- error setting value");

    // Add to the used TLS list to cleanup data in the thread Exit()
    CThread::GetUsedTlsBases().Register(this);
}


bool CTlsBase::x_DeleteTlsData(void)
{
    if ( !m_Initialized ) {
        return false;
    }

    // Get previously stored data
    STlsData* tls_data = static_cast<STlsData*> (x_GetTlsData());
    if ( !tls_data ) {
        return false;
    }

    // Cleanup & destroy
    if ( tls_data->m_CleanupFunc ) {
        tls_data->m_CleanupFunc(tls_data->m_Value, tls_data->m_CleanupData);
    }
    delete tls_data;

    // Store NULL in the TLS
    s_TlsSetValue(m_Key, 0,
                  "CTlsBase::x_Reset() -- error cleaning-up TLS");

    return true;
}



void CTlsBase::x_Reset(void)
{
    if ( x_DeleteTlsData() ) {
        // Deregister this TLS from the current thread
        CThread::GetUsedTlsBases().Deregister(this);
    }
}



/////////////////////////////////////////////////////////////////////////////
//  CExitThreadException::
//
//    Exception used to terminate threads safely, cleaning up
//    all the resources allocated.
//


class CExitThreadException
{
public:
    // Create new exception object, initialize counter.
    CExitThreadException(void);

    // Create a copy of exception object, increase counter.
    CExitThreadException(const CExitThreadException& prev);

    // Destroy the object, decrease counter. If the counter is
    // zero outside of CThread::Wrapper(), rethrow exception.
    ~CExitThreadException(void);

    // Inform the object it has reached CThread::Wrapper().
    void EnterWrapper(void)
    {
        *m_InWrapper = true;
    }
private:
    int* m_RefCount;
    bool* m_InWrapper;
};


CExitThreadException::CExitThreadException(void)
    : m_RefCount(new int),
      m_InWrapper(new bool)
{
    *m_RefCount = 1;
    *m_InWrapper = false;
}


CExitThreadException::CExitThreadException(const CExitThreadException& prev)
    : m_RefCount(prev.m_RefCount),
      m_InWrapper(prev.m_InWrapper)
{
    (*m_RefCount)++;
}


CExitThreadException::~CExitThreadException(void)
{
    if (--(*m_RefCount) > 0) {
        // Not the last object - continue to handle exceptions
        return;
    }

    bool tmp_in_wrapper = *m_InWrapper; // save the flag
    delete m_RefCount;
    delete m_InWrapper;

    if ( !tmp_in_wrapper ) {
        // Something is wrong - terminate the thread
        assert(((void)("CThread::Exit() -- cannot exit thread"), 0));
#if defined(NCBI_WIN32_THREADS)
        ExitThread(0);
#elif defined(NCBI_POSIX_THREADS)
        pthread_exit(0);
#endif
    }

}



/////////////////////////////////////////////////////////////////////////////
//  CThread::
//

// Mutex to protect CThread members and to make sure that Wrapper() function
// will not proceed until after the appropriate Run() is finished.
DEFINE_STATIC_FAST_MUTEX(s_ThreadMutex);

unsigned int CThread::sm_ThreadsCount = 0;


// Internal storage for thread objects and related variables/functions
CStaticTls<CThread>* CThread::sm_ThreadsTls;


void s_CleanupThreadsTls(void* /* ptr */)
{
    CThread::sm_ThreadsTls = 0;  // Indicate that the TLS is destroyed
}


void CThread::CreateThreadsTls(void)
{
    static CStaticTls<CThread> s_ThreadsTls(s_CleanupThreadsTls);

    sm_ThreadsTls = &s_ThreadsTls;
}


TWrapperRes CThread::Wrapper(TWrapperArg arg)
{
    // Get thread object and self ID
    CThread* thread_obj = static_cast<CThread*>(arg);

    // Set Toolkit thread ID. Otherwise no mutexes will work!
    {{
        CFastMutexGuard guard(s_ThreadMutex);

        static int s_ThreadCount = 0;
        s_ThreadCount++;

        thread_obj->m_ID = s_ThreadCount;
        xncbi_Validate(thread_obj->m_ID != 0,
                       "CThread::Wrapper() -- error assigning thread ID");
        GetThreadsTls().SetValue(thread_obj);
    }}

#if defined NCBI_THREAD_PID_WORKAROUND
    // Store this thread's PID. Changed PID means forking of the thread.
    thread_obj->m_ThreadPID =
        CProcess::sx_GetPid(CProcess::ePID_GetThread);
#endif

    // Run user-provided thread main function here
    try {
        thread_obj->m_ExitData = thread_obj->Main();
    }
    catch (CExitThreadException& e) {
        e.EnterWrapper();
    }
#if defined(NCBI_COMPILER_MSVC)  &&  defined(_DEBUG)
    // Microsoft promotes many common application errors to exceptions.
    // This includes occurrences such as dereference of a NULL pointer and
    // walking off of a dangling pointer.  The catch-all is lifted only in
    // debug mode to permit easy inspection of such error conditions, while
    // maintaining safety of production, release-mode applications.
    NCBI_CATCH_X(1, "CThread::Wrapper: CThread::Main() failed");
#else
    NCBI_CATCH_ALL_X(2, "CThread::Wrapper: CThread::Main() failed");
#endif

    // Call user-provided OnExit()
    try {
        thread_obj->OnExit();
    }
#if defined(NCBI_COMPILER_MSVC)  &&  defined(_DEBUG)
    // Microsoft promotes many common application errors to exceptions.
    // This includes occurrences such as dereference of a NULL pointer and
    // walking off of a dangling pointer.  The catch-all is lifted only in
    // debug mode to permit easy inspection of such error conditions, while
    // maintaining safety of production, release-mode applications.
    NCBI_CATCH_X(3, "CThread::Wrapper: CThread::OnExit() failed");
#else
    NCBI_CATCH_ALL_X(4, "CThread::Wrapper: CThread::OnExit() failed");
#endif

    // Cleanup local storages used by this thread
    thread_obj->m_UsedTls.ClearAll();

    {{
        CFastMutexGuard state_guard(s_ThreadMutex);

        // Thread is terminated - decrement counter under mutex
        --sm_ThreadsCount;

        // Indicate the thread is terminated
        thread_obj->m_IsTerminated = true;

        // Schedule the thread object for destruction, if detached
        if ( thread_obj->m_IsDetached ) {
            thread_obj->m_SelfRef.Reset();
        }
    }}

    return 0;
}


CThread::CThread(void)
    : m_IsRun(false),
      m_IsDetached(false),
      m_IsJoined(false),
      m_IsTerminated(false),
      m_ExitData(0)
#if defined NCBI_THREAD_PID_WORKAROUND
      , m_ThreadPID(0)
#endif
{
    DoDeleteThisObject();
#if defined(HAVE_PTHREAD_SETCONCURRENCY)  &&  defined(NCBI_POSIX_THREADS)
    // Adjust concurrency for Solaris etc.
    if (pthread_getconcurrency() == 0) {
        xncbi_Validate(pthread_setconcurrency(GetCpuCount()) == 0,
                       "CThread::CThread() -- pthread_setconcurrency(2) "
                       "failed");
    }
#endif
}


CThread::~CThread(void)
{
#if defined(NCBI_WIN32_THREADS)
    // close handle if it's not yet closed
    CFastMutexGuard state_guard(s_ThreadMutex);
    if ( m_IsRun && m_Handle != NULL ) {
        CloseHandle(m_Handle);
        m_Handle = NULL;
    }
#endif
}



inline TWrapperRes ThreadWrapperCaller(TWrapperArg arg) {
    return CThread::Wrapper(arg);
}

#if defined(NCBI_POSIX_THREADS)
extern "C" {
    typedef TWrapperRes (*FSystemWrapper)(TWrapperArg);

    static TWrapperRes ThreadWrapperCallerImpl(TWrapperArg arg) {
        return ThreadWrapperCaller(arg);
    }
}
#elif defined(NCBI_WIN32_THREADS)
extern "C" {
    typedef TWrapperRes (WINAPI *FSystemWrapper)(TWrapperArg);

    static TWrapperRes WINAPI ThreadWrapperCallerImpl(TWrapperArg arg) {
        return ThreadWrapperCaller(arg);
    }
}
#endif


#if defined NCBI_THREAD_PID_WORKAROUND
TPid CThread::sx_GetThreadPid(void)
{
    CThread* thread_ptr = GetCurrentThread();
    return thread_ptr ? thread_ptr->m_ThreadPID : 0;
}


void CThread::sx_SetThreadPid(TPid pid)
{
    CThread* thread_ptr = GetCurrentThread();
    if ( thread_ptr ) {
        thread_ptr->m_ThreadPID = pid;
    }
}
#endif


bool CThread::Run(TRunMode flags)
{
    // Do not allow the new thread to run until m_Handle is set
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Check
    xncbi_Validate(!m_IsRun,
                   "CThread::Run() -- called for already started thread");

    m_IsDetached = (flags & fRunDetached) != 0;

#if defined NCBI_THREAD_PID_WORKAROUND
    CProcess::sx_GetPid(CProcess::ePID_GetCurrent);
#endif

    // Thread will run - increment counter under mutex
    ++sm_ThreadsCount;
    try {

#if defined(NCBI_WIN32_THREADS)
        // We need this parameter in WinNT - can not use NULL instead!
        DWORD thread_id;
        // Suspend thread to adjust its priority
        DWORD creation_flags = (flags & fRunNice) == 0 ? 0 : CREATE_SUSPENDED;
        m_Handle = CreateThread(NULL, 0, ThreadWrapperCallerImpl,
                                this, creation_flags, &thread_id);
        xncbi_Validate(m_Handle != NULL,
                       "CThread::Run() -- error creating thread");
        if (flags & fRunNice) {
            // Adjust priority and resume the thread
            SetThreadPriority(m_Handle, THREAD_PRIORITY_BELOW_NORMAL);
            ResumeThread(m_Handle);
        }
        if ( m_IsDetached ) {
            CloseHandle(m_Handle);
            m_Handle = NULL;
        }
        else {
            // duplicate handle to adjust security attributes
            HANDLE oldHandle = m_Handle;
            xncbi_Validate(DuplicateHandle(GetCurrentProcess(), oldHandle,
                                           GetCurrentProcess(), &m_Handle,
                                           0, FALSE, DUPLICATE_SAME_ACCESS),
                           "CThread::Run() -- error getting thread handle");
            xncbi_Validate(CloseHandle(oldHandle),
                           "CThread::Run() -- error closing thread handle");
        }
#elif defined(NCBI_POSIX_THREADS)
        pthread_attr_t attr;
        xncbi_Validate(pthread_attr_init (&attr) == 0,
                       "CThread::Run() - error initializing thread attributes");
        if ( ! (flags & fRunUnbound) ) {
#if defined(NCBI_OS_BSD)  ||  defined(NCBI_OS_CYGWIN)  ||  defined(NCBI_OS_IRIX)
            xncbi_Validate(pthread_attr_setscope(&attr,
                                                 PTHREAD_SCOPE_PROCESS) == 0,
                           "CThread::Run() - error setting thread scope");
#else
            xncbi_Validate(pthread_attr_setscope(&attr,
                                                 PTHREAD_SCOPE_SYSTEM) == 0,
                           "CThread::Run() - error setting thread scope");
#endif
        }
        if ( m_IsDetached ) {
            xncbi_Validate(pthread_attr_setdetachstate(&attr,
                                                       PTHREAD_CREATE_DETACHED) == 0,
                           "CThread::Run() - error setting thread detach state");
        }
        xncbi_Validate(pthread_create(&m_Handle, &attr,
                                      ThreadWrapperCallerImpl, this) == 0,
                       "CThread::Run() -- error creating thread");

        xncbi_Validate(pthread_attr_destroy(&attr) == 0,
                       "CThread::Run() - error destroying thread attributes");

#else
        if (flags & fRunAllowST) {
            Wrapper(this);
        }
        else {
            xncbi_Validate(0,
                           "CThread::Run() -- system does not support threads");
        }
#endif

        // prevent deletion of CThread until thread is finished
        m_SelfRef.Reset(this);

    }
    catch (...) {
        // In case of any error we need to decrement threads count
        --sm_ThreadsCount;
        throw;
    }

    // Indicate that the thread is run
    m_IsRun = true;
    return true;
}


void CThread::Detach(void)
{
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Check the thread state: it must be run, but not detached yet
    xncbi_Validate(m_IsRun,
                   "CThread::Detach() -- called for not yet started thread");
    xncbi_Validate(!m_IsDetached,
                   "CThread::Detach() -- called for already detached thread");

    // Detach the thread
#if defined(NCBI_WIN32_THREADS)
    xncbi_Validate(CloseHandle(m_Handle),
                   "CThread::Detach() -- error closing thread handle");
    m_Handle = NULL;
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Validate(pthread_detach(m_Handle) == 0,
                   "CThread::Detach() -- error detaching thread");
#endif

    // Indicate the thread is detached
    m_IsDetached = true;

    // Schedule the thread object for destruction, if already terminated
    if ( m_IsTerminated ) {
        m_SelfRef.Reset();
    }
}


void CThread::Join(void** exit_data)
{
    // Check the thread state: it must be run, but not detached yet
    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        xncbi_Validate(m_IsRun,
                       "CThread::Join() -- called for not yet started thread");
        xncbi_Validate(!m_IsDetached,
                       "CThread::Join() -- called for detached thread");
        xncbi_Validate(!m_IsJoined,
                       "CThread::Join() -- called for already joined thread");
        m_IsJoined = true;
    }}

    // Join (wait for) and destroy
#if defined(NCBI_WIN32_THREADS)
    xncbi_Validate(WaitForSingleObject(m_Handle, INFINITE) == WAIT_OBJECT_0,
                   "CThread::Join() -- can not join thread");
    DWORD status;
    xncbi_Validate(GetExitCodeThread(m_Handle, &status) &&
                   status != DWORD(STILL_ACTIVE),
                   "CThread::Join() -- thread is still running after join");
    xncbi_Validate(CloseHandle(m_Handle),
                   "CThread::Join() -- can not close thread handle");
    m_Handle = NULL;
#elif defined(NCBI_POSIX_THREADS)
    xncbi_Validate(pthread_join(m_Handle, 0) == 0,
                   "CThread::Join() -- can not join thread");
#endif

    // Set exit_data value
    if ( exit_data ) {
        *exit_data = m_ExitData;
    }

    // Schedule the thread object for destruction
    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        m_SelfRef.Reset();
    }}
}


void CThread::Exit(void* exit_data)
{
    // Don't exit from the main thread
    CThread* x_this = GetCurrentThread();
    xncbi_Validate(x_this != 0,
                   "CThread::Exit() -- attempt to call for the main thread");

    {{
        CFastMutexGuard state_guard(s_ThreadMutex);
        x_this->m_ExitData = exit_data;
    }}

    // Throw the exception to be caught by Wrapper()
    throw CExitThreadException();
}


bool CThread::Discard(void)
{
    CFastMutexGuard state_guard(s_ThreadMutex);

    // Do not discard after Run()
    if ( m_IsRun ) {
        return false;
    }

    // Schedule for destruction (or, destroy it right now if there is no
    // other CRef<>-based references to this object left).
    m_SelfRef.Reset(this);
    m_SelfRef.Reset();
    return true;
}


void CThread::OnExit(void)
{
    return;
}


CUsedTlsBases& CThread::GetUsedTlsBases(void)
{
    CThread* thread = CThread::GetCurrentThread();
    if ( thread ) {
        return thread->m_UsedTls;
    }
    else {
        return *s_MainUsedTls;
    }
}


void CThread::GetSystemID(TThreadSystemID* id)
{
#if defined(NCBI_WIN32_THREADS)
    // Can not use GetCurrentThread() since it also requires
    // DuplicateHandle() and CloseHandle() to be called for the result.
    *id = GetCurrentThreadId();
#elif defined(NCBI_POSIX_THREADS)
    *id = pthread_self();
#else
    *id = 0;
#endif
    return;
}


END_NCBI_SCOPE
