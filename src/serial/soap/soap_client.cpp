/*  $Id: soap_client.cpp 131269 2008-06-18 13:22:40Z gouriano $
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
 * Author:  Andrei Gourianov
 *
 * File Description:
 *   SOAP http client
 *
 */

#include <ncbi_pch.hpp>
#include <serial/soap/soap_client.hpp>
#include <serial/objistrxml.hpp>
#include <serial/objostrxml.hpp>
#include <connect/ncbi_conn_stream.hpp>

#include <algorithm>


BEGIN_NCBI_SCOPE

CSoapHttpClient::CSoapHttpClient(const string& server_url,
                                 const string& namespace_name)
    : m_ServerUrl(server_url), m_DefNamespace(namespace_name),
      m_OmitScopePrefixes(false)
{
}

CSoapHttpClient::~CSoapHttpClient(void)
{
}

void CSoapHttpClient::SetServerUrl(const string& server_url)
{
    m_ServerUrl = server_url;
}
const string& CSoapHttpClient::GetServerUrl(void) const
{
    return m_ServerUrl;
}
void CSoapHttpClient::SetDefaultNamespaceName(const string& namespace_name)
{
    m_DefNamespace = namespace_name;
}
const string& CSoapHttpClient::GetDefaultNamespaceName(void) const
{
    return m_DefNamespace;
}

void CSoapHttpClient::RegisterObjectType(TTypeInfoGetter type_getter)
{
    if (find(m_Types.begin(), m_Types.end(), type_getter) == m_Types.end()) {
        m_Types.push_back(type_getter);
    }
}

void CSoapHttpClient::Invoke(CSoapMessage& response,
                             const CSoapMessage& request,
                             CConstRef<CSoapFault>* fault /*=0*/,
                             const string& soap_action /*= kEmptyStr*/)
{
    response.Reset();
    vector< TTypeInfoGetter >::iterator types_in;
    for (types_in = m_Types.begin(); types_in != m_Types.end(); ++types_in) {
        response.RegisterObjectType(*types_in);
    }

    char content_type[MAX_CONTENT_TYPE_LEN + 1];

    SConnNetInfo* net_info = ConnNetInfo_Create(0);
    net_info->debug_printout = eDebugPrintout_Data;
    ConnNetInfo_ParseURL(net_info, m_ServerUrl.c_str());
    {
        string s("SOAPAction: \"");
        s += soap_action;
        s += "\"\r\n";
        ConnNetInfo_SetUserHeader(net_info,s.c_str());
    }

    CConn_HttpStream http(net_info,
        MIME_ComposeContentTypeEx(eMIME_T_Text, eMIME_Xml, eENCOD_None,
            content_type, sizeof(content_type) - 1));

    auto_ptr<CObjectOStream> os(CObjectOStream::Open(eSerial_Xml, http));
    auto_ptr<CObjectIStream> is(CObjectIStream::Open(eSerial_Xml, http));

// allow unknown data
    is->SetSkipUnknownMembers(eSerialSkipUnknown_Yes);
// allow missing mandatory data
    is->SetVerifyData(eSerialVerifyData_No);

    if (m_OmitScopePrefixes) {
        dynamic_cast<CObjectOStreamXml*>(os.get())->SetEnforcedStdXml(true);
        dynamic_cast<CObjectIStreamXml*>(is.get())->SetEnforcedStdXml(true);
    }

    *os << request;
    *is >> response;
    if (fault) {
        *fault = SOAP_GetKnownObject<CSoapFault>(response);
    }
}

void CSoapHttpClient::SetOmitScopePrefixes(bool bOmit)
{
    m_OmitScopePrefixes = bOmit;
}

END_NCBI_SCOPE
