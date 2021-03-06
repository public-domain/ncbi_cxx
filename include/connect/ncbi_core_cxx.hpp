#ifndef CONNECT___NCBI_CORE_CXX__H
#define CONNECT___NCBI_CORE_CXX__H

/* $Id: ncbi_core_cxx.hpp 143260 2008-10-16 18:10:47Z lavr $
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
 * Author:  Anton Lavrentiev
 *
 * File description:
 *   C++->C conversion functions for basic CORE connect stuff:
 *     Registry
 *     Logging
 *     Locking
 *
 */

#include <connect/ncbi_core.h>
#include <corelib/ncbireg.hpp>


/** @addtogroup UtilityFunc
 *
 * @{
 */


BEGIN_NCBI_SCOPE


/// NB: now that registries are CObjects, any we "own" will be deleted
/// if and only if nothing else still holds a reference to them.
extern NCBI_XCONNECT_EXPORT REG     REG_cxx2c
(IRWRegistry* reg,
 bool         pass_ownership = false
 );


extern NCBI_XCONNECT_EXPORT LOG     LOG_cxx2c(void);


extern NCBI_XCONNECT_EXPORT MT_LOCK MT_LOCK_cxx2c
(CRWLock*     lock = 0,
 bool         pass_ownership = false
 );


typedef enum {
    eConnectInit_OwnNothing  = 0,
    eConnectInit_OwnRegistry = 0x01,
    eConnectInit_OwnLock     = 0x02
} EConnectInitFlags;

typedef unsigned int FConnectInitFlags;  // bitwise OR of EConnectInitFlags


extern NCBI_XCONNECT_EXPORT void CONNECT_Init
(IRWRegistry*      reg  = 0,
 CRWLock*          lock = 0,
 FConnectInitFlags flag = eConnectInit_OwnNothing);


/////////////////////////////////////////////////////////////////////////////
///
/// Helper hook-up class that installs default logging/registry/locking
/// (but only if they have not yet been installed explicitly by user).
///

class NCBI_XCONNECT_EXPORT CConnIniter
{
protected:
    CConnIniter();
};


/////////////////////////////////////////////////////////////////////////////
///
/// C++ version of STimeout
///

class NCBI_XCONNECT_EXPORT CTimeout
{
public:
    /// Create default timeout
    CTimeout(void) : m_Ptr(kDefaultTimeout) {}
    /// Create timeout from STimeout*
    CTimeout(const STimeout* tmo) { Set(tmo); }
    /// Initialize timeout in seconds and microseconds
    CTimeout(unsigned int sec, unsigned int usec) { Set(sec, usec); }
    ~CTimeout(void) {}

    /// Get STimeout*
    const STimeout* Get(void) const { return m_Ptr; }
    /// Convert to const STimeout*
    operator const STimeout*(void) const { return Get(); }

    /// Set timeout
    void Set(const STimeout* tmo);
    /// Set timeout in seconds and microseconds
    void Set(unsigned int sec, unsigned int usec);
    /// Copy timeout from STimeout*
    const CTimeout& operator=(const STimeout* tmo) { Set(tmo); return *this; }

private:
    const STimeout* m_Ptr;
    STimeout        m_Timeout;
};


END_NCBI_SCOPE


/* @} */

#endif  // CONNECT___NCBI_CORE_CXX__HPP
