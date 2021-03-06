/*  $Id: ncbi_process.cpp 145442 2008-11-12 17:55:08Z lavr $
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
 * Authors:  Aaron Ucko, Vladimir Ivanov
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/error_codes.hpp>
#include <corelib/ncbidiag.hpp>
#include <corelib/ncbifile.hpp>
#include <corelib/ncbithr.hpp>
#include <corelib/ncbi_process.hpp>
#include <corelib/ncbi_safe_static.hpp>
#include <corelib/ncbi_system.hpp>

#if defined(NCBI_OS_UNIX)
#  include <errno.h>
#  include <fcntl.h>
#  include <signal.h>
#  include <stdio.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#elif defined(NCBI_OS_MSWIN)
#  include <corelib/ncbitime.hpp>  // for CStopWatch
#  include <process.h>
#  include <tlhelp32.h>
#endif

#if defined(NCBI_OS_MSWIN)
#  pragma warning (disable : 4191)
#endif


#define NCBI_USE_ERRCODE_X   Corelib_Process


BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
//
// CProcess::CExitInfo
//

// CExitInfo process state
enum EExitInfoState {
    eExitInfo_Unknown,
    eExitInfo_Alive,
    eExitInfo_Terminated
};


#define EXIT_INFO_CHECK \
    if ( !IsPresent() ) { \
        NCBI_THROW(CCoreException, eCore, \
                  "CProcess::CExitInfo state is unknown. " \
                  "Please check IsPresent() first."); \
    }


CProcess::CExitInfo::CExitInfo(void)
{
    state  = eExitInfo_Unknown;
    status = 0;
}


bool CProcess::CExitInfo::IsPresent(void)
{
    return state != eExitInfo_Unknown;
}


bool CProcess::CExitInfo::IsAlive(void)
{
    EXIT_INFO_CHECK;
    return state == eExitInfo_Alive;
}


bool CProcess::CExitInfo::IsExited(void)
{
    EXIT_INFO_CHECK;
    if (state != eExitInfo_Terminated) {
        return false;
    }
#if defined(NCBI_OS_UNIX)
    return WIFEXITED(status) != 0;
#elif defined(NCBI_OS_MSWIN)
    // The process always terminates with exit code
    return true;
#endif
}


bool CProcess::CExitInfo::IsSignaled(void)
{
    EXIT_INFO_CHECK;
    if (state != eExitInfo_Terminated) {
        return false;
    }
#if defined(NCBI_OS_UNIX)
    return WIFSIGNALED(status) != 0;
#elif defined(NCBI_OS_MSWIN)
    // The process always terminates with exit code
    return false;
#endif
}


int CProcess::CExitInfo::GetExitCode(void)
{
    if ( !IsExited() ) {
        return -1;
    }
#if defined(NCBI_OS_UNIX)
    return WEXITSTATUS(status);
#elif defined(NCBI_OS_MSWIN)
    return status;
#endif
}


int CProcess::CExitInfo::GetSignal(void)
{
    if ( !IsSignaled() ) {
        return -1;
    }
#if defined(NCBI_OS_UNIX)
    return WTERMSIG(status);
#elif defined(NCBI_OS_MSWIN)
    return -1;
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
// CProcess 
//

// Predefined timeouts (in milliseconds)
const unsigned long           kWaitPrecision        = 100;
const unsigned long CProcess::kDefaultKillTimeout   = 1000;


CProcess::CProcess(TPid process, EProcessType type)
    : m_Process(process), m_Type(type)
{
    return;
}

#if defined(NCBI_OS_MSWIN)
// The helper constructor for MS Windows to avoid cast from
// TProcessHandle to TPid
CProcess::CProcess(TProcessHandle process, EProcessType type)
    : m_Process((intptr_t)process), m_Type(type)
{
    return;
}

#endif


#if defined NCBI_THREAD_PID_WORKAROUND
TPid CProcess::sx_GetPid(EGetPidFlag flag)
{
    if ( flag == ePID_GetThread ) {
        // Return real PID, do not cache it.
        return getpid();
    }

    DEFINE_STATIC_FAST_MUTEX(s_GetPidMutex);
    static TPid s_CurrentPid = 0;
    static TPid s_ParentPid = 0;

    if (CThread::GetSelf() == 0) {
        // For main thread always force caching of PIDs
        CFastMutexGuard guard(s_GetPidMutex);
        s_CurrentPid = getpid();
        s_ParentPid = getppid();
    }
    else {
        // For child threads update cached PIDs only if there was a fork
        // First call is always from the main thread (explicit or through
        // CThread::Run()), s_CurrentPid must be != 0 in any child thread.
        _ASSERT(s_CurrentPid);
        TPid pid = getpid();
        TPid thr_pid = CThread::sx_GetThreadPid();
        if (thr_pid  &&  thr_pid != pid) {
            // Thread's PID has changed - fork detected.
            // Use current PID and PPID as globals.
            CThread::sx_SetThreadPid(pid);
            CFastMutexGuard guard(s_GetPidMutex);
            s_CurrentPid = pid;
            s_ParentPid = getppid();
        }
    }
    return flag == ePID_GetCurrent ? s_CurrentPid : s_ParentPid;
}
#endif


TPid CProcess::GetCurrentPid(void)
{
#if   defined NCBI_THREAD_PID_WORKAROUND
    return sx_GetPid(ePID_GetCurrent);
#elif defined(NCBI_OS_UNIX)
    return getpid();
#elif defined(NCBI_OS_MSWIN)
    return GetCurrentProcessId();
#endif
}


TPid CProcess::GetParentPid(void)
{
#if   defined NCBI_THREAD_PID_WORKAROUND
    return sx_GetPid(ePID_GetParent);
#elif defined(NCBI_OS_UNIX)
    return getppid();
#elif defined(NCBI_OS_MSWIN)
    TPid ppid = (TPid)(-1);
    // Open snapshot handle
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE) {

        PROCESSENTRY32 pe;
        DWORD pid = GetCurrentProcessId();
        pe.dwSize = sizeof(PROCESSENTRY32);

        BOOL retval = Process32First(hSnapshot, &pe);
        while (retval) {
            if (pe.th32ProcessID == pid) {
                ppid = pe.th32ParentProcessID;
                break;
            }
            pe.dwSize = sizeof(PROCESSENTRY32);
            retval = Process32Next(hSnapshot, &pe);
        }

        // close snapshot handle
        CloseHandle(hSnapshot);
    }
    return ppid;
#endif
}


TPid CProcess::Fork(void)
{
#ifdef NCBI_OS_UNIX
    TPid pid = ::fork();
    CDiagContext::UpdatePID();
    return pid;
#else
    NCBI_THROW(CCoreException, eCore,
               "Fork() not implemented on this platform");
#endif
}


bool CProcess::Daemonize(const char* logfile, CProcess::TDaemonFlags flags)
{
#ifdef NCBI_OS_UNIX
    int  fdin  = ::dup(STDIN_FILENO);
    int  fdout = ::dup(STDOUT_FILENO);
    int  fderr = ::dup(STDERR_FILENO);

    try {
        if (flags & fKeepStdin) {
            int nullr = ::open("/dev/null", O_RDONLY);
            if (nullr < 0)
                throw "Error opening /dev/null for reading";
            if (nullr != STDIN_FILENO) {
                int error = ::dup2(nullr, STDIN_FILENO);
                int x_errno = errno;
                ::close(nullr);
                if (error < 0) {
                    errno = x_errno;
                    throw "Error redirecting stdin";
                }
            }
        }
        if (flags & fKeepStdout) {
            int nullw = ::open("/dev/null", O_WRONLY);
            if (nullw < 0)
                throw "Error opening /dev/null for writing";
            ::fflush(stdout);
            if (nullw != STDOUT_FILENO) {
                int error = ::dup2(nullw, STDOUT_FILENO);
                int x_errno = errno;
                ::close(nullw);
                if (error < 0) {
                    ::dup2(fdin, STDIN_FILENO);
                    errno = x_errno;
                    throw "Error redirecting stdout";
                }
            }
        }
        if (logfile) {
            int fd = (!*logfile ? ::open("/dev/null", O_WRONLY | O_APPEND) :
                      ::open(logfile, O_WRONLY | O_APPEND | O_CREAT, 0666));
            if (fd < 0)
                throw "Unable to open logfile for stderr";
            ::fflush(stderr);
            if (fd != STDERR_FILENO) {
                int error = ::dup2(fd, STDERR_FILENO);
                int x_errno = errno;
                ::close(fd);
                if (error < 0) {
                    ::dup2(fdin,  STDIN_FILENO);
                    ::dup2(fdout, STDOUT_FILENO);
                    errno = x_errno;
                    throw "Error redirecting stderr";
                }
            }
        }
        TPid pid = Fork();
        if (pid == (pid_t)(-1)) {
            int x_errno = errno;
            ::dup2(fdin,  STDIN_FILENO);
            ::dup2(fdout, STDOUT_FILENO);
            ::dup2(fderr, STDERR_FILENO);
            errno = x_errno;
            throw "Cannot fork";
        }
        if (pid)
            ::_exit(0);
        if (!(flags & fDontChroot))
            (void) ::chdir("/");        // NB: "/" always exists
        if (!(flags & fKeepStdin))
            ::fclose(stdin);
        ::close(fdin);
        if (!(flags & fKeepStdout))
            ::fclose(stdout);
        ::close(fdout);
        if (!logfile)
            ::fclose(stderr);
        ::close(fderr);
        ::setsid();
        if (flags & fImmuneTTY) {
            pid = Fork();
            if (pid == (pid_t)(-1)) {
                const char* error = strerror(errno);
                ERR_POST_X(2,
                           string("[Daemonize]  Second fork() failed to"
                                  " immune from TTY accruals (") + error +
                           string("), continue anyways"));
            } else if (pid) {
                ::_exit(0);
            }
        }
        return true/*success*/;
    }
    catch (const char* what) {
        int x_errno = errno;
        const char* error = x_errno ? strerror(x_errno) : 0;
        ERR_POST_X(1, string("[Daemonize]  ") + what +
                   (error  &&  *error ? string(": ") + error : kEmptyStr));
        ::close(fdin);
        ::close(fdout);
        ::close(fderr);
        errno = x_errno;
    }
#else
    NCBI_THROW(CCoreException, eCore,
               "Daemonize() not implemented on this platform");
#endif
    return false/*failure*/;
}


bool CProcess::IsAlive(void) const
{
#if defined(NCBI_OS_UNIX)
    return kill((TPid)m_Process, 0) == 0  ||  errno == EPERM;

#elif defined(NCBI_OS_MSWIN)
    HANDLE hProcess = 0;
    if (m_Type == ePid) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,
                               FALSE, (TPid)m_Process);
        if (!hProcess) {
            return GetLastError() == ERROR_ACCESS_DENIED;
        }
    } else {
        hProcess = (TProcessHandle)m_Process;
    }
    DWORD status = 0;
    _ASSERT(STILL_ACTIVE != 0);
    GetExitCodeProcess(hProcess, &status);
    if (m_Type == ePid) {
        CloseHandle(hProcess);
    }
    return status == STILL_ACTIVE;
#endif
}


bool CProcess::Kill(unsigned long timeout) const
{
    // Safe process termination
    bool safe = (timeout > 0);

#if defined(NCBI_OS_UNIX)

    TPid pid = (TPid)m_Process;

    // Try to kill the process with SIGTERM first
    if (kill(pid, SIGTERM) == -1  &&  errno == EPERM) {
        return false;
    }

    // Check process termination within the timeout
    for (;;) {
        waitpid(pid, 0, WNOHANG);
        if (kill(pid, 0) < 0) {
            return true;
        }
        unsigned long x_sleep = kWaitPrecision;
        if (x_sleep > timeout) {
            x_sleep = timeout;
        }
        if ( !x_sleep ) {
             break;
        }
        SleepMilliSec(x_sleep);
        timeout -= x_sleep;
    }

    // Try harder to kill the stubborn process -- SIGKILL may not be caught!
    int res = kill(pid, SIGKILL);
    if ( !safe ) {
        return res == 0;
    }
    SleepMilliSec(kWaitPrecision);
    // Reap the zombie (if child) up from the system
    waitpid(pid, 0, WNOHANG);
    // Check whether the process cannot be killed (most likely due
    // to a kernel problem)
    return kill(pid, 0) != 0;

#elif defined(NCBI_OS_MSWIN)

    // Try to kill current process?
    if ( m_Type == ePid  &&  (TPid)m_Process == GetCurrentPid() ) {
        ExitProcess(-1);
        // should not reached
        return false;
    }

    HANDLE hProcess    = NULL;
    HANDLE hThread     = NULL;
    bool   enable_sync = true;

    // Get process handle
    if (m_Type == eHandle) {
        hProcess = (TProcessHandle)m_Process;

    } else {  // m_Type == ePid
        hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_TERMINATE |
                               SYNCHRONIZE, FALSE, (TPid)m_Process);
        if ( !hProcess ) {
            // Try to open with minimal access right needed
            // to terminate process.
            enable_sync = false;
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (TPid)m_Process);
            if (!hProcess) {
                if (GetLastError() != ERROR_ACCESS_DENIED) {
                    return false;
                }
                // If we have an administrative rights, that we can try
                // to terminate the process using SE_DEBUG_NAME privilege,
                // which system administrators normally have, but it might
                // be disabled by default. When this privilege is enabled,
                // the calling thread can open processes with any access
                // rights regardless of the security descriptor assigned
                // to the process.

                // Determine OS version
                OSVERSIONINFO vi;
                vi.dwOSVersionInfoSize = sizeof(vi);
                GetVersionEx(&vi);
                if (vi.dwPlatformId != VER_PLATFORM_WIN32_NT) {
                    return false;
                }

                // Get current thread token 
                HANDLE hToken;
                if (!OpenThreadToken(GetCurrentThread(), 
                                     TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
                                     FALSE, &hToken)) {
                    if (GetLastError() != ERROR_NO_TOKEN) {
                        return false;
                    }
                    // Rrevert to the process token, if not impersonating
                    if (!OpenProcessToken(GetCurrentProcess(),
                                          TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES,
                                          &hToken)) {
                        return false;
                    }
                }

                // Try to enable the SE_DEBUG_NAME privilege

                TOKEN_PRIVILEGES tp, tp_prev;
                DWORD            tp_prev_size = sizeof(tp_prev);

                tp.PrivilegeCount = 1;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);

                if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp),
                                           &tp_prev, &tp_prev_size)) {
                    CloseHandle(hToken);
                    return false;
                }
                if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
                    // The AdjustTokenPrivileges function cannot add new
                    // privileges to the access token. It can only enable or
                    // disable the token's existing privileges.
                    CloseHandle(hToken);
                    return false;
                }

                // Try to open process handle again
                hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (TPid)m_Process);
                
                // Restore original privilege state
                AdjustTokenPrivileges(hToken, FALSE, &tp_prev, sizeof(tp_prev),
                                      NULL, NULL);
                CloseHandle(hToken);
            }
        }
    }

    // Check process handle
    if ( !hProcess  ||  hProcess == INVALID_HANDLE_VALUE ) {
        return true;
    }
    // Terminate process
    bool terminated = false;

    CStopWatch timer;
    if ( safe ) {
        timer.Start();
    }
    // Safe process termination
    if ( safe  &&  enable_sync ) {
        // (kernel32.dll loaded at same address in each process)
        FARPROC exitproc = GetProcAddress(GetModuleHandle("KERNEL32.DLL"),
                                        "ExitProcess");
        if ( exitproc ) {
            hThread = CreateRemoteThread(hProcess, NULL, 0,
                                        (LPTHREAD_START_ROUTINE)exitproc,
                                        0, 0, 0);
            // Wait until process terminated, or timeout expired
            if (hThread   &&
                (WaitForSingleObject(hProcess, timeout) == WAIT_OBJECT_0)){
                terminated = true;
            }
        }
    }
    // Try harder to kill stubborn process
    if ( !terminated ) {
        if ( TerminateProcess(hProcess, -1) != 0  ||
            GetLastError() == ERROR_INVALID_HANDLE ) {
            // If process "terminated" succesfuly or error occur but
            // process handle became invalid -- process has terminated
            terminated = true;
        }
    }
    if (safe  &&  terminated) {
        // The process terminating now.
        // Reset flag, and wait for real process termination.

        terminated = false;
        double elapsed = timer.Elapsed() * kMilliSecondsPerSecond;
        unsigned long linger_timeout = (elapsed < timeout) ? 
            (unsigned long)((double)timeout - elapsed) : 0;

        for (;;) {
            if ( !IsAlive() ) {
                terminated = true;
                break;
            }
            unsigned long x_sleep = kWaitPrecision;
            if (x_sleep > linger_timeout) {
                x_sleep = linger_timeout;
            }
            if ( !x_sleep ) {
                break;
            }
            SleepMilliSec(x_sleep);
            linger_timeout -= x_sleep;
        }
    }
    // Close temporary process handle
    if ( hThread ) {
        CloseHandle(hThread);
    }
    if (m_Type == ePid) {
        CloseHandle(hProcess);
    }
    return terminated;
#endif
}


#if defined(NCBI_OS_MSWIN)

// MS Windows:
// A helper function for terminating all processes
// in the tree within specified timeout.

// If 'timer' is specified we use safe process termination.
static bool s_KillGroup(DWORD pid,
                        CStopWatch *timer, unsigned long &timeout)
{
    // Open snapshot handle.
    // We cannot use one shapshot for recursive calls, 
    // because it is not reentrant.
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    // Terminate all children first
    if (!Process32First(hSnapshot, &pe)) {
        return false;
    }
    do {
        if (pe.th32ParentProcessID == pid) {
            // Safe termination -- update timeout
            if ( timer ) {
                double elapsed = timer->Elapsed() * kMilliSecondsPerSecond;
                timeout = (elapsed < timeout) ?
                    (unsigned long)((double)timeout - elapsed) : 0;
                if ( !timeout ) {
                    CloseHandle(hSnapshot);
                    return false;
                }
            }
            bool res = s_KillGroup(pe.th32ProcessID, timer, timeout);
            if ( !res ) {
                CloseHandle(hSnapshot);
                return false;
            }
        }
    }
    while (Process32Next(hSnapshot, &pe)); 

    // Terminate the specified process

    // Safe termination -- update timeout
    if ( timer ) {
        double elapsed = timer->Elapsed() * kMilliSecondsPerSecond;
        timeout = (elapsed < timeout) ?
            (unsigned long)((double)timeout - elapsed) : 0;
        if ( !timeout ) {
            CloseHandle(hSnapshot);
            return false;
        }
    }
    bool res = CProcess(pid, CProcess::ePid).Kill(timeout);

    // Close snapshot handle
    CloseHandle(hSnapshot);
    return res;
}

#endif


bool CProcess::KillGroup(unsigned long timeout) const
{
#if defined(NCBI_OS_UNIX)
    TPid pgid = getpgid((TPid)m_Process);
    if (pgid == -1) {
        // TRUE if PID does not match any process
        return errno == ESRCH;
    }
    return KillGroupById(pgid, timeout);
    
#elif defined(NCBI_OS_MSWIN)

    // Convert the process handle to process ID if needed
    TPid pid = 0;
    if (m_Type == eHandle) {
        // Some OS like Windows 2000 and WindowsXP (w/o SP1) don't
        // have GetProcessId() function. Try to load it directy from 
        // KERNEL32.DLL
        static bool  s_TryGetProcessId = true;
        typedef DWORD (STDMETHODCALLTYPE FAR* LPFN_GETPROCESSID)(HANDLE process);
        static LPFN_GETPROCESSID s_GetProcessId = NULL;

        if ( s_TryGetProcessId  &&  !s_GetProcessId ) {
            s_GetProcessId  = (LPFN_GETPROCESSID)GetProcAddress(
                                    GetModuleHandle("KERNEL32.DLL"),
                                    "GetProcessId");
            s_TryGetProcessId = false;
        }
        if ( !s_GetProcessId ) {
            // GetProcessId() is not available on this platform
            return false;
        }
        pid = s_GetProcessId((TProcessHandle)m_Process);

    } else {  // m_Type == ePid
        pid = (TPid)m_Process;
    }
    if (!pid) {
        return false;
    }
    // Use safe process termination if timeout > 0
    unsigned long x_timeout = timeout;
    CStopWatch timer;
    if ( timeout ) {
        timer.Start();
    }
    // Kill process tree
    bool result = s_KillGroup(pid, (timeout > 0) ? &timer : 0, x_timeout);
    return result;
#endif
}


bool CProcess::KillGroupById(TPid pgid, unsigned long timeout)
{
#if defined(NCBI_OS_UNIX)

    // Try to kill the process group with SIGTERM first
    if (kill(-pgid, SIGTERM) == -1  &&  errno == EPERM) {
        return false;
    }
    // Check process termination within the timeout 
    bool safe = (timeout > 0);
    for (;;) {
        // Rip the zombie (if group leader is a child) up from the system
        waitpid(pgid, 0, WNOHANG);
        if (kill(-pgid, 0) < 0) {
            return true;
        }
        unsigned long x_sleep = kWaitPrecision;
        if (x_sleep > timeout) {
            x_sleep = timeout;
        }
        if ( !x_sleep ) {
             break;
        }
        SleepMilliSec(x_sleep);
        timeout -= x_sleep;
    }
    // Try harder to kill the stubborn processes -- SIGKILL may not be caught!
    int res = kill(-pgid, SIGKILL);
    if ( !safe ) {
        return res == 0;
    }
    SleepMilliSec(kWaitPrecision);
    // Rip the zombie (if group leader is a child) up from the system
    waitpid(pgid, 0, WNOHANG);
    // Check whether the process cannot be killed
    // (most likely due to a kernel problem)
    return kill(-pgid, 0) != 0;

#elif defined(NCBI_OS_MSWIN)
    // Cannot be implemented, use non-static version of KillGroup()
    // for specified process.
    return false;
#endif
}


int CProcess::Wait(unsigned long timeout, CExitInfo* info) const
{
    // Reset extended information
    if (info) {
        info->state  = eExitInfo_Unknown;
        info->status = 0;
    }

#if defined(NCBI_OS_UNIX)
    TPid pid     = (TPid)m_Process;
    int  options = (timeout == kInfiniteTimeoutMs) ? 0 : WNOHANG;
    int  status;

    // Check process termination within timeout (or infinite)
    for (;;) {
        TPid ws = waitpid(pid, &status, options);
        if (ws > 0) {
            // Process has terminated.
            _ASSERT(ws == pid);
            if (info) {
                info->state  = eExitInfo_Terminated;
                info->status = status;
            }
            return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        } else if (ws == 0) {
            // Process is still running
            _ASSERT(timeout != kInfiniteTimeoutMs);
            unsigned long x_sleep = kWaitPrecision;
            if (x_sleep > timeout) {
                x_sleep = timeout;
            }
            if ( !x_sleep ) {
                if (info) {
                    info->state = eExitInfo_Alive;
                }
                break;
            }
            SleepMilliSec(x_sleep);
            timeout -= x_sleep;
        } else if (errno != EINTR ) {
            // Some error
            break;
        }
    }
    return -1;

#elif defined(NCBI_OS_MSWIN)
    HANDLE hProcess;
    bool   enable_sync = true;
    // Get process handle
    if (m_Type == ePid) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                               FALSE, (TPid)m_Process);
        if ( !hProcess ) {
            enable_sync = false;
            hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, (TPid)m_Process);
            if (!hProcess   &&  GetLastError() == ERROR_ACCESS_DENIED) {
                return -1;
            }
        }
    } else {
        hProcess = (TProcessHandle)m_Process;
    }
    int status = -1;
    DWORD x_status = STILL_ACTIVE;
    try {
        // Is process still running?
        if (!hProcess  ||  hProcess == INVALID_HANDLE_VALUE) {
            throw -1;
        }
        if (GetExitCodeProcess(hProcess, &x_status)  &&
            x_status != STILL_ACTIVE) {
            throw (int)x_status;
        }
        if (info  &&  x_status == STILL_ACTIVE) {
            info->state = eExitInfo_Alive;
        }
        // Wait for process termination, or timeout expired
        if (enable_sync  &&  timeout) {
            DWORD tv = (timeout == kInfiniteTimeoutMs) ? INFINITE : 
                                                         (DWORD)timeout;
            DWORD ws = WaitForSingleObject(hProcess, tv);
            switch(ws) {
                case WAIT_OBJECT_0:
                    // Get exit code below
                    break;
                case WAIT_TIMEOUT:
                    throw (int)STILL_ACTIVE;
                default:
                    throw -1;
            }
            // Get process exit code
            if (GetExitCodeProcess(hProcess, &x_status)) {
                throw (int)x_status;
            }
            throw -1;
        }
        status = (int)x_status;
    }
    catch (int e) {
        status = e;
        if (info) {
            info->status = status;
            if (status < 0) {
                info->state = eExitInfo_Unknown;
            } else if (status == STILL_ACTIVE) {
                info->state = eExitInfo_Alive;
            } else {
                info->state = eExitInfo_Terminated;
            }
        }
        if (status == STILL_ACTIVE) {
            status = -1;
        }
    }
    if (m_Type == ePid ) {
        CloseHandle(hProcess);
    }
    return status;
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
// CPIDGuard
//

// Protective mutex
DEFINE_STATIC_FAST_MUTEX(s_PidGuardMutex);

// NOTE: This method to protect PID file works only within one process.
//       CPIDGuard know nothing about PID file modification or deletion 
//       by other processes. Be aware.


CPIDGuard::CPIDGuard(const string& filename, const string& dir)
    : m_OldPID(0), m_NewPID(0)
{
    string real_dir;
    CDirEntry::SplitPath(filename, &real_dir, 0, 0);
    if (real_dir.empty()) {
        if (dir.empty()) {
            real_dir = CDir::GetTmpDir();
        } else {
            real_dir = dir;
        }
        m_Path = CDirEntry::MakePath(real_dir, filename);
    } else {
        m_Path = filename;
    }
    UpdatePID();
}


CPIDGuard::~CPIDGuard(void)
{
    Release();
}


void CPIDGuard::Release(void)
{
    if ( !m_Path.empty() ) {
        // MT-Safe protect
        CFastMutexGuard LOCK(s_PidGuardMutex);

        // Read info
        TPid pid = 0;
        unsigned int ref = 0;
        CNcbiIfstream in(m_Path.c_str());
        if ( in.good() ) {
            in >> pid >> ref;
            in.close();
            if ( m_NewPID != pid ) {
                // We do not own this file more
                return;
            }
            if ( ref ) {
                ref--;
            }
            // Check reference counter
            if ( ref ) {
                // Write updated reference counter into the file
                CNcbiOfstream out(m_Path.c_str(),
                                  IOS_BASE::out | IOS_BASE::trunc);
                if ( out.good() ) {
                    out << pid << endl << ref << endl;
                }
                if ( !out.good() ) {
                    NCBI_THROW(CPIDGuardException, eWrite,
                               "Unable to write into PID file " + m_Path +": "
                               + strerror(errno));
                }
            } else {
                // Remove the file
                CDirEntry(m_Path).Remove();
            }
        }
        m_Path.erase();
    }
}


void CPIDGuard::Remove(void)
{
    if ( !m_Path.empty() ) {
        // MT-Safe protect
        CFastMutexGuard LOCK(s_PidGuardMutex);
        // Remove the file
        CDirEntry(m_Path).Remove();
        m_Path.erase();
    }
}


void CPIDGuard::UpdatePID(TPid pid)
{
    if (pid == 0) {
        pid = CProcess::GetCurrentPid();
    }

    // MT-Safe protect
    CFastMutexGuard LOCK(s_PidGuardMutex);

    // Read old PID
    unsigned int ref = 1;
    CNcbiIfstream in(m_Path.c_str());
    if ( in.good() ) {
        in >> m_OldPID >> ref;
        if ( m_OldPID == pid ) {
            // Guard the same PID. Just increase the reference counter.
            ref++;
        } else {
            if ( CProcess(m_OldPID,CProcess::ePid).IsAlive() ) {
                NCBI_THROW2(CPIDGuardException, eStillRunning,
                            "Process is still running", m_OldPID);
            }
            ref = 1;
        }
    }
    in.close();

    // Write new PID
    CNcbiOfstream out(m_Path.c_str(), IOS_BASE::out | IOS_BASE::trunc);
    if ( out.good() ) {
        out << pid << endl << ref << endl;
    }
    if ( !out.good() ) {
        NCBI_THROW(CPIDGuardException, eWrite,
                   "Unable to write into PID file " + m_Path + ": "
                   + strerror(errno));
    }
    // Save updated pid
    m_NewPID = pid;
}

const char* CPIDGuardException::GetErrCodeString(void) const
{
    switch (GetErrCode()) {
    case eStillRunning: return "eStillRunning";
    case eWrite:        return "eWrite";
    default:            return CException::GetErrCodeString();
    }
}


END_NCBI_SCOPE
