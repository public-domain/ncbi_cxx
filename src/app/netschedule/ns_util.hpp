#ifndef NETSCHEDULE_NS_UTIL__HPP
#define NETSCHEDULE_NS_UTIL__HPP

/*  $Id: ns_util.hpp 143684 2008-10-21 18:50:06Z joukovv $
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
 * File Description: Utility functions for NetSchedule
 *
 */

#include <corelib/ncbistl.hpp>
#include "ns_types.hpp"

BEGIN_NCBI_SCOPE

string NS_EncodeBitVector(TNSBitVector& bv);
TNSBitVector NS_DecodeBitVector(const string& s);
void NS_FormatIPAddress(unsigned int ipaddr, string& str_addr);
string NS_FormatIPAddress(unsigned int ipaddr);

class CRequestContext;
class CRequestContextFactory
{
public:
    virtual ~CRequestContextFactory() {}
    virtual CRequestContext* Get() = 0;
    virtual void Return(CRequestContext*) = 0;
};

END_NCBI_SCOPE

#endif /* NETSCHEDULE_NS_UTIL__HPP */
