/*  $Id: writer_cache.cpp 141052 2008-09-23 18:45:55Z vasilche $
 * ===========================================================================
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
 *  Author:  Eugene Vasilchenko, Anatoliy Kuznetsov
 *
 *  File Description: Cached writer for GenBank data loader
 *
 */
#include <ncbi_pch.hpp>
#include <objtools/data_loaders/genbank/cache/writer_cache.hpp>
#include <objtools/data_loaders/genbank/cache/writer_cache_entry.hpp>
#include <objtools/data_loaders/genbank/cache/reader_cache_params.h>
#include <objtools/data_loaders/genbank/readers.hpp> // for entry point
#include <objtools/data_loaders/genbank/request_result.hpp>
#include <objtools/data_loaders/genbank/dispatcher.hpp>

#include <corelib/rwstream.hpp>
#include <corelib/plugin_manager_store.hpp>

#include <serial/objostrasnb.hpp>
#include <serial/serial.hpp>

#include <objmgr/objmgr_exception.hpp>

#include <util/cache/icache.hpp>

#include <memory>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

CCacheWriter::CCacheWriter(void)
{
}


void CCacheWriter::InitializeCache(CReaderCacheManager& cache_manager,
                                   const TPluginManagerParamTree* params)
{
    const TPluginManagerParamTree* writer_params = params ?
        params->FindNode(NCBI_GBLOADER_WRITER_CACHE_DRIVER_NAME) : 0;
    ICache* id_cache = 0;
    ICache* blob_cache = 0;
    auto_ptr<TParams> id_params
        (GetCacheParams(writer_params, eCacheWriter, eIdCache));
    auto_ptr<TParams> blob_params
        (GetCacheParams(writer_params, eCacheWriter, eBlobCache));
    _ASSERT(id_params.get());
    _ASSERT(blob_params.get());
    const TParams* share_id_param =
        id_params->FindNode(NCBI_GBLOADER_WRITER_CACHE_PARAM_SHARE);
    bool share_id = !share_id_param  ||
        NStr::StringToBool(share_id_param->GetValue().value);
    const TParams* share_blob_param =
        blob_params->FindNode(NCBI_GBLOADER_WRITER_CACHE_PARAM_SHARE);
    bool share_blob = !share_blob_param  ||
        NStr::StringToBool(share_blob_param->GetValue().value);
    if (share_id  ||  share_blob) {
        if ( share_id ) {
            ICache* cache = cache_manager.
                FindCache(CReaderCacheManager::fCache_Id,
                          id_params.get());
            if ( cache ) {
                _ASSERT(!id_cache);
                id_cache = cache;
            }
        }
        if ( share_blob ) {
            ICache* cache = cache_manager.
                FindCache(CReaderCacheManager::fCache_Blob,
                          blob_params.get());
            if ( cache ) {
                _ASSERT(!blob_cache);
                blob_cache = cache;
            }
        }
    }
    if ( !id_cache ) {
        id_cache = CreateCache(writer_params, eCacheWriter, eIdCache);
        if ( id_cache ) {
            cache_manager.RegisterCache(*id_cache,
                CReaderCacheManager::fCache_Id);
        }
    }
    if ( !blob_cache ) {
        blob_cache = CreateCache(writer_params, eCacheWriter, eBlobCache);
        if ( blob_cache ) {
            cache_manager.RegisterCache(*blob_cache,
                CReaderCacheManager::fCache_Blob);
        }
    }
    SetIdCache(id_cache);
    SetBlobCache(blob_cache);
}


void CCacheWriter::ResetCache(void)
{
    SetIdCache(0);
    SetBlobCache(0);
}


void CCacheWriter::SaveStringSeq_ids(CReaderRequestResult& result,
                                     const string& seq_id)
{
    if ( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    WriteSeq_ids(seq_id, ids);
}


namespace {
    class CStoreBuffer {
    public:
        CStoreBuffer(void)
            : m_Ptr(m_Buffer)
            {
            }
        
        const char* data(void) const
            {
                return m_Buffer;
            }
        size_t size(void) const
            {
                return m_Ptr - m_Buffer;
            }
        void CheckStore(size_t size) const;
        void StoreUint4(Uint4 v)
            {
                CheckStore(4);
                x_StoreUint4(v);
            }
        void StoreInt4(Int4 v)
            {
                StoreUint4(v);
            }
        void StoreString(const string& s)
            {
                size_t size = s.size();
                CheckStore(4+size);
                x_StoreUint4(size);
                memcpy(m_Ptr, s.data(), size);
                m_Ptr += size;
            }

    protected:
        void x_StoreUint4(Uint4 v)
            {
                m_Ptr[0] = v>>24;
                m_Ptr[1] = v>>16;
                m_Ptr[2] = v>>8;
                m_Ptr[3] = v;
                m_Ptr += 4;
            }

    private:
        CStoreBuffer(const CStoreBuffer&);
        void operator=(const CStoreBuffer&);

        char m_Buffer[4096];
        char* m_Ptr;
    };

    
    void CStoreBuffer::CheckStore(size_t size) const
    {
        if ( m_Ptr + size > m_Buffer + sizeof(m_Buffer) ) {
            NCBI_THROW(CLoaderException, eLoaderFailed,
                       "store buffer overflow");
        }
    }
}

void CCacheWriter::SaveStringGi(CReaderRequestResult& result,
                                const string& seq_id)
{
    if( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    if ( ids->IsLoadedGi() ) {
        CStoreBuffer str;
        str.StoreInt4(ids->GetGi());
        m_IdCache->Store(seq_id, 0, GetGiSubkey(),
                         str.data(), str.size());
    }
}


void CCacheWriter::SaveSeq_idSeq_ids(CReaderRequestResult& result,
                                     const CSeq_id_Handle& seq_id)
{
    if( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    WriteSeq_ids(GetIdKey(seq_id), ids);
}


void CCacheWriter::SaveSeq_idGi(CReaderRequestResult& result,
                                const CSeq_id_Handle& seq_id)
{
    if( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    if ( ids->IsLoadedGi() ) {
        CStoreBuffer str;
        str.StoreInt4(ids->GetGi());
        m_IdCache->Store(GetIdKey(seq_id), 0, GetGiSubkey(),
                         str.data(), str.size());
    }
}


void CCacheWriter::SaveSeq_idAccVer(CReaderRequestResult& result,
                                    const CSeq_id_Handle& seq_id)
{
    if( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    if ( ids->IsLoadedAccVer() ) {
        CSeq_id_Handle acc = ids->GetAccVer();
        const string& str = acc.AsString();
        m_IdCache->Store(GetIdKey(seq_id), 0, GetAccVerSubkey(),
                         str.data(), str.size());
    }
}


void CCacheWriter::SaveSeq_idLabel(CReaderRequestResult& result,
                                   const CSeq_id_Handle& seq_id)
{
    if( !m_IdCache) {
        return;
    }

    CLoadLockSeq_ids ids(result, seq_id);
    if ( ids->IsLoadedLabel() ) {
        const string& str = ids->GetLabel();
        m_IdCache->Store(GetIdKey(seq_id), 0, GetLabelSubkey(),
                         str.data(), str.size());
    }
}


namespace {
    class CCacheDataEraser {
        CCacheDataEraser(ICache* cache,
                         const string& key, int version, const string& subkey)
            : m_Cache(cache),
              m_Key(key), m_Version(version), m_Subkey(subkey)
            {
            }
        ~CCacheDataEraser(void) {
            if ( m_Cache ) {
                try {
                    m_Cache->Remove(m_Key, m_Version, m_Subkey);
                }
                catch (...) { // ignored
                }
                m_Cache = 0;
            }
        }

        void Done(void) {
            m_Cache = 0;
        }
        
    private:
        ICache* m_Cache;
        string m_Key;
        int m_Version;
        string m_Subkey;
    };
}


void CCacheWriter::WriteSeq_ids(const string& key,
                                const CLoadLockSeq_ids& ids)
{
    if( !m_IdCache) {
        return;
    }

    if ( !ids.IsLoaded() ) {
        return;
    }

    try {
        auto_ptr<IWriter> writer
            (m_IdCache->GetWriteStream(key, 0, GetSeq_idsSubkey()));
        if ( !writer.get() ) {
            return;
        }

        {{
            CWStream w_stream(writer.get());
            CObjectOStreamAsnBinary obj_stream(w_stream);
            ITERATE ( CLoadInfoSeq_ids, it, *ids ) {
                obj_stream << *it->GetSeqId();
            }
        }}

        writer.reset();
    }
    catch ( ... ) {
        // In case of an error we need to remove incomplete data
        // from the cache.
        try {
            m_BlobCache->Remove(key, 0, GetSeq_idsSubkey());
        }
        catch ( exception& /*exc*/ ) {
            // ignored
        }
        // ignore cache write error - it doesn't affect application
    }
}


void CCacheWriter::SaveSeq_idBlob_ids(CReaderRequestResult& result,
                                      const CSeq_id_Handle& seq_id,
                                      const SAnnotSelector* sel)
{
    if ( !m_IdCache) {
        return;
    }

    CLoadLockBlob_ids ids(result, seq_id, sel);
    if ( !ids.IsLoaded() ) {
        return;
    }

    CStoreBuffer str;
    str.StoreInt4(IDS_MAGIC);
    str.StoreUint4(ids->GetState());
    str.StoreUint4(ids->size());
    ITERATE ( CLoadInfoBlob_ids, it, *ids ) {
        const CBlob_id& id = *it->first;
        str.StoreUint4(id.GetSat());
        str.StoreUint4(id.GetSubSat());
        str.StoreUint4(id.GetSatKey());
        const CBlob_Info& info = it->second;
        str.StoreUint4(info.GetContentsMask());
        str.StoreUint4(info.GetNamedAnnotNames().size());
        ITERATE(CBlob_Info::TNamedAnnotNames, it2, info.GetNamedAnnotNames()) {
            str.StoreString(*it2);
        }
    }
    m_IdCache->Store(GetIdKey(seq_id), 0, GetBlob_idsSubkey(sel),
                     str.data(), str.size());
}


void CCacheWriter::SaveBlobVersion(CReaderRequestResult& /*result*/,
                                   const TBlobId& blob_id,
                                   TBlobVersion version)
{
    if( !m_IdCache ) {
        return;
    }

    CStoreBuffer str;
    str.StoreInt4(version);
    m_IdCache->Store(GetBlobKey(blob_id), 0, GetBlobVersionSubkey(),
                     str.data(), str.size());
}


class CCacheBlobStream : public CWriter::CBlobStream
{
public:
    typedef int TVersion;

    CCacheBlobStream(ICache* cache, const string& key,
                     TVersion version, const string& subkey)
        : m_Cache(cache), m_Key(key), m_Version(version), m_Subkey(subkey),
          m_Writer(cache->GetWriteStream(key, version, subkey))
        {
            if ( m_Writer.get() ) {
                m_Stream.reset(new CWStream(m_Writer.get()));
            }
        }
    ~CCacheBlobStream(void)
        {
            if ( m_Stream.get() ) {
                Abort();
            }
        }

    bool CanWrite(void) const
        {
            return m_Stream.get() != 0;
        }

    CNcbiOstream& operator*(void)
        {
            _ASSERT(m_Stream.get());
            return *m_Stream;
        }

    void Close(void)
        {
            *m_Stream << flush;
            if ( !*m_Stream ) {
                Abort();
                NCBI_THROW(CLoaderException, eLoaderFailed,
                           "cannot write into cache");
            }
            m_Stream.reset();
            m_Writer.reset();
        }

    void Abort(void)
        {
            m_Stream.reset();
            m_Writer.reset();
            Remove();
        }

    void Remove(void)
        {
            m_Cache->Remove(m_Key, m_Version, m_Subkey);
        }

private:
    ICache*             m_Cache;
    string              m_Key;
    TVersion            m_Version;
    string              m_Subkey;
    auto_ptr<IWriter>   m_Writer;
    auto_ptr<CWStream>  m_Stream;
};


CRef<CWriter::CBlobStream>
CCacheWriter::OpenBlobStream(CReaderRequestResult& result,
                             const TBlobId& blob_id,
                             TChunkId chunk_id,
                             const CProcessor& processor)
{
    if( !m_BlobCache ) {
        return null;
    }

    try {
        CLoadLockBlob blob(result, blob_id);
        CRef<CBlobStream> stream
            (new CCacheBlobStream(m_BlobCache, GetBlobKey(blob_id),
                                  blob.GetBlobVersion(),
                                  GetBlobSubkey(chunk_id)));
        if ( !stream->CanWrite() ) {
            return null;
        }
        
        WriteProcessorTag(**stream, processor);
        return stream;
    }
    catch ( ... ) {
        return null;
    }
}


bool CCacheWriter::CanWrite(EType type) const
{
    return (type == eIdWriter ? m_IdCache : m_BlobCache) != 0;
}


END_SCOPE(objects)


using namespace objects;


/// Class factory for Cache writer
///
/// @internal
///
class CCacheWriterCF :
    public CSimpleClassFactoryImpl<CWriter, CCacheWriter>
{
private:
    typedef CSimpleClassFactoryImpl<CWriter, CCacheWriter> TParent;
public:
    CCacheWriterCF()
        : TParent(NCBI_GBLOADER_WRITER_CACHE_DRIVER_NAME, 0) {}
    ~CCacheWriterCF() {}


    CWriter*
    CreateInstance(const string& driver  = kEmptyStr,
                   CVersionInfo version = NCBI_INTERFACE_VERSION(CWriter),
                   const TPluginManagerParamTree* params = 0) const
    {
        if ( !driver.empty()  &&  driver != m_DriverName )
            return 0;

        if ( !version.Match(NCBI_INTERFACE_VERSION(CWriter)) ) {
            return 0;
        }
        return new CCacheWriter();
    }
};


void NCBI_EntryPoint_CacheWriter(
     CPluginManager<CWriter>::TDriverInfoList&   info_list,
     CPluginManager<CWriter>::EEntryPointRequest method)
{
    CHostEntryPointImpl<CCacheWriterCF>::NCBI_EntryPointImpl(info_list,
                                                             method);
}


void NCBI_EntryPoint_xwriter_cache(
     CPluginManager<CWriter>::TDriverInfoList&   info_list,
     CPluginManager<CWriter>::EEntryPointRequest method)
{
    NCBI_EntryPoint_CacheWriter(info_list, method);
}


void GenBankWriters_Register_Cache(void)
{
    RegisterEntryPoint<CWriter>(NCBI_EntryPoint_CacheWriter);
}


END_NCBI_SCOPE
