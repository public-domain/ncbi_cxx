/*  $Id: ncfetch.cpp 109711 2007-08-30 15:12:05Z didenko $
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
 * Authors:  Mike Dicuccio, Anatoliy Kuznetsov
 *
 * File Description:
 *     BLOB (image) fetch from NetCache.
 *     Takes two CGI arguments:
 *       key=NC_KEY
 *       fmt=FORMAT  (default: "image/png")
 *
 */

#include <ncbi_pch.hpp>
#include <cgi/cgiapp.hpp>
#include <cgi/cgictx.hpp>
#include <connect/services/netcache_api.hpp>
#include <corelib/reader_writer.hpp>
#include <corelib/rwstream.hpp>

USING_NCBI_SCOPE;


/// NetCache BLOB/image fetch application
///
class CNetCacheBlobFetchApp : public CCgiApplication
{
public:
    CNetCacheBlobFetchApp() {}
    virtual int  ProcessRequest(CCgiContext& ctx);
};

int CNetCacheBlobFetchApp::ProcessRequest(CCgiContext& ctx)
{
    const CCgiRequest& request = ctx.GetRequest();
    CCgiResponse&      reply   = ctx.GetResponse();

    bool is_found;
    string key = request.GetEntry("key", &is_found);
    if (key.empty() || !is_found) {
        return 0;
    }
    string fmt = request.GetEntry("fmt", &is_found);
    if (fmt.empty() || !is_found) {
        fmt = "image/png";
    }

    reply.SetContentType(fmt);
    reply.WriteHeader();

    CNetCacheAPI cli("ncfetch");
    size_t blob_size = 0;
    auto_ptr<IReader> reader(cli.GetData(key, &blob_size));
    if (!reader.get()) {
        return 0;
    }
    LOG_POST(Info << "retrieved data: " << blob_size << " bytes");
    
    CRStream strm(reader.get());
    reply.out() << strm.rdbuf();
    

    return 0;
}

int main(int argc, const char* argv[])
{
    SetSplitLogFile(true);
    GetDiagContext().SetOldPostFormat(false);

    return CNetCacheBlobFetchApp().AppMain(argc, argv, 0, eDS_Default, 0);
}


