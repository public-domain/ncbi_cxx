/*  $Id: objistr.cpp 147457 2008-12-10 18:21:48Z ivanovp $
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
* Author: Eugene Vasilchenko
*
* File Description:
*   !!! PUT YOUR DESCRIPTION HERE !!!
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbi_safe_static.hpp>
#include <corelib/ncbiutil.hpp>
#include <corelib/ncbimtx.hpp>
#include <corelib/ncbithr.hpp>
#include <corelib/ncbi_param.hpp>

#include <exception>

#include <util/bytesrc.hpp>

#include <serial/objistr.hpp>
#include <serial/impl/typeref.hpp>
#include <serial/impl/member.hpp>
#include <serial/impl/variant.hpp>
#include <serial/impl/classinfo.hpp>
#include <serial/impl/choice.hpp>
#include <serial/impl/aliasinfo.hpp>
#include <serial/impl/continfo.hpp>
#include <serial/enumvalues.hpp>
#include <serial/impl/memberlist.hpp>
#include <serial/delaybuf.hpp>
#include <serial/impl/objistrimpl.hpp>
#include <serial/objectinfo.hpp>
#include <serial/objectiter.hpp>
#include <serial/impl/objlist.hpp>
#include <serial/impl/choiceptr.hpp>
#include <serial/serialimpl.hpp>
#include <serial/pack_string.hpp>
#include <serial/error_codes.hpp>

#include <limits.h>
#if HAVE_WINDOWS_H
// In MSVC limits.h doesn't define FLT_MIN & FLT_MAX
# include <float.h>
#endif

#undef _TRACE
#define _TRACE(arg) ((void)0)

#define NCBI_USE_ERRCODE_X   Serial_IStream

BEGIN_NCBI_SCOPE

CRef<CByteSource> CObjectIStream::GetSource(ESerialDataFormat format,
                                            const string& fileName,
                                            TSerialOpenFlags openFlags)
{
    if ( (openFlags & eSerial_StdWhenEmpty) && fileName.empty() ||
         (openFlags & eSerial_StdWhenDash) && fileName == "-" ||
         (openFlags & eSerial_StdWhenStd) && fileName == "stdin" ) {
        return CRef<CByteSource>(new CStreamByteSource(NcbiCin));
    }
    else {
        bool binary;
        switch ( format ) {
        case eSerial_AsnText:
        case eSerial_Xml:
            binary = false;
            break;
        case eSerial_AsnBinary:
            binary = true;
            break;
        default:
            NCBI_THROW(CSerialException,eNotImplemented,
                       "CObjectIStream::Open: unsupported format");
        }
        
        if ( (openFlags & eSerial_UseFileForReread) )  {
            // use file as permanent file
            return CRef<CByteSource>(new CFileByteSource(fileName, binary));
        }
        else {
            // open file as stream
            return CRef<CByteSource>(new CFStreamByteSource(fileName, binary));
        }
    }
}

CRef<CByteSource> CObjectIStream::GetSource(CNcbiIstream& inStream,
                                            bool deleteInStream)
{
    if ( deleteInStream ) {
        return CRef<CByteSource>(new CFStreamByteSource(inStream));
    }
    else {
        return CRef<CByteSource>(new CStreamByteSource(inStream));
    }
}

CObjectIStream* CObjectIStream::Create(ESerialDataFormat format)
{
    switch ( format ) {
    case eSerial_AsnText:
        return CreateObjectIStreamAsn();
    case eSerial_AsnBinary:
        return CreateObjectIStreamAsnBinary();
    case eSerial_Xml:
        return CreateObjectIStreamXml();
    default:
        break;
    }
    NCBI_THROW(CSerialException,eNotImplemented,
               "CObjectIStream::Open: unsupported format");
}

CObjectIStream* CObjectIStream::Create(ESerialDataFormat format,
                                       CByteSource& source)
{
    AutoPtr<CObjectIStream> stream(Create(format));
    stream->Open(source);
    return stream.release();
}

CObjectIStream* CObjectIStream::Create(ESerialDataFormat format,
                                       CByteSourceReader& reader)
{
    AutoPtr<CObjectIStream> stream(Create(format));
    stream->Open(reader);
    return stream.release();
}

CObjectIStream* CObjectIStream::CreateFromBuffer(ESerialDataFormat format,
                                                 const char* buffer,
                                                 size_t size)
{
    AutoPtr<CObjectIStream> stream(Create(format));
    stream->OpenFromBuffer(buffer, size);
    return stream.release();
}

CObjectIStream* CObjectIStream::Open(ESerialDataFormat format,
                                     CNcbiIstream& inStream,
                                     bool deleteInStream)
{
    CRef<CByteSource> src = GetSource(inStream, deleteInStream);
    return Create(format, *src);
}

CObjectIStream* CObjectIStream::Open(ESerialDataFormat format,
                                     const string& fileName,
                                     TSerialOpenFlags openFlags)
{
    CRef<CByteSource> src = GetSource(format, fileName, openFlags);
    return Create(format, *src);
}

/////////////////////////////////////////////////////////////////////////////
// data verification setup

ESerialVerifyData CObjectIStream::ms_VerifyDataDefault = eSerialVerifyData_Default;
static CStaticTls<int> s_VerifyTLS;


void CObjectIStream::SetVerifyDataThread(ESerialVerifyData verify)
{
    x_GetVerifyDataDefault();
    ESerialVerifyData tls_verify = ESerialVerifyData(intptr_t(s_VerifyTLS.GetValue()));
    if (tls_verify != eSerialVerifyData_Never &&
        tls_verify != eSerialVerifyData_Always &&
        tls_verify != eSerialVerifyData_DefValueAlways) {
        s_VerifyTLS.SetValue(reinterpret_cast<int*>(verify));
    }
}

void CObjectIStream::SetVerifyDataGlobal(ESerialVerifyData verify)
{
    x_GetVerifyDataDefault();
    if (ms_VerifyDataDefault != eSerialVerifyData_Never &&
        ms_VerifyDataDefault != eSerialVerifyData_Always &&
        ms_VerifyDataDefault != eSerialVerifyData_DefValueAlways) {
        ms_VerifyDataDefault = verify;
    }
}

ESerialVerifyData CObjectIStream::x_GetVerifyDataDefault(void)
{
    ESerialVerifyData verify;
    if (ms_VerifyDataDefault == eSerialVerifyData_Never ||
        ms_VerifyDataDefault == eSerialVerifyData_Always ||
        ms_VerifyDataDefault == eSerialVerifyData_DefValueAlways) {
        verify = ms_VerifyDataDefault;
    } else {
        verify = ESerialVerifyData(intptr_t(s_VerifyTLS.GetValue()));
        if (verify == eSerialVerifyData_Default) {
            if (ms_VerifyDataDefault == eSerialVerifyData_Default) {

                // change the default here, if you wish
                ms_VerifyDataDefault = eSerialVerifyData_Yes;
                //ms_VerifyDataDefault = eSerialVerifyData_No;

                const char* str = getenv(SERIAL_VERIFY_DATA_READ);
                if (str) {
                    if (NStr::CompareNocase(str,"YES") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_Yes;
                    } else if (NStr::CompareNocase(str,"NO") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_No;
                    } else if (NStr::CompareNocase(str,"NEVER") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_Never;
                    } else  if (NStr::CompareNocase(str,"ALWAYS") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_Always;
                    } else  if (NStr::CompareNocase(str,"DEFVALUE") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_DefValue;
                    } else  if (NStr::CompareNocase(str,"DEFVALUE_ALWAYS") == 0) {
                        ms_VerifyDataDefault = eSerialVerifyData_DefValueAlways;
                    }
                }
            }
            verify = ms_VerifyDataDefault;
        }
    }
    return verify;
}

/////////////////////////////////////////////////////////////////////////////
// skip unknown members setup

ESerialSkipUnknown CObjectIStream::ms_SkipUnknownDefault = eSerialSkipUnknown_Default;
static CStaticTls<int> s_SkipTLS;


void CObjectIStream::SetSkipUnknownThread(ESerialSkipUnknown skip)
{
    x_GetSkipUnknownDefault();
    ESerialSkipUnknown tls_skip = ESerialSkipUnknown(intptr_t(s_SkipTLS.GetValue()));
    if (tls_skip != eSerialSkipUnknown_Never &&
        tls_skip != eSerialSkipUnknown_Always) {
        s_SkipTLS.SetValue(reinterpret_cast<int*>(skip));
    }
}

void CObjectIStream::SetSkipUnknownGlobal(ESerialSkipUnknown skip)
{
    x_GetSkipUnknownDefault();
    if (ms_SkipUnknownDefault != eSerialSkipUnknown_Never &&
        ms_SkipUnknownDefault != eSerialSkipUnknown_Always) {
        ms_SkipUnknownDefault = skip;
    }
}

ESerialSkipUnknown CObjectIStream::x_GetSkipUnknownDefault(void)
{
    ESerialSkipUnknown skip;
    if (ms_SkipUnknownDefault == eSerialSkipUnknown_Never ||
        ms_SkipUnknownDefault == eSerialSkipUnknown_Always) {
        skip = ms_SkipUnknownDefault;
    } else {
        skip = ESerialSkipUnknown(intptr_t(s_SkipTLS.GetValue()));
        if (skip == eSerialSkipUnknown_Default) {
            if (ms_SkipUnknownDefault == eSerialSkipUnknown_Default) {

                // change the default here, if you wish
                ms_SkipUnknownDefault = eSerialSkipUnknown_No;
                //ms_SkipUnknownDefault = eSerialSkipUnknown_Yes;

                const char* str = getenv(SERIAL_SKIP_UNKNOWN_MEMBERS);
                if (str) {
                    if (NStr::CompareNocase(str,"YES") == 0) {
                        ms_SkipUnknownDefault = eSerialSkipUnknown_Yes;
                    } else if (NStr::CompareNocase(str,"NO") == 0) {
                        ms_SkipUnknownDefault = eSerialSkipUnknown_No;
                    } else if (NStr::CompareNocase(str,"NEVER") == 0) {
                        ms_SkipUnknownDefault = eSerialSkipUnknown_Never;
                    } else  if (NStr::CompareNocase(str,"ALWAYS") == 0) {
                        ms_SkipUnknownDefault = eSerialSkipUnknown_Always;
                    }
                }
            }
            skip = ms_SkipUnknownDefault;
        }
    }
    return skip;
}


NCBI_PARAM_ENUM_ARRAY(ESerialSkipUnknown, SERIAL, SKIP_UNKNOWN_VARIANTS)
{
    {"NO",     eSerialSkipUnknown_No},
    {"NEVER",  eSerialSkipUnknown_Never},
    {"YES",    eSerialSkipUnknown_Yes},
    {"ALWAYS", eSerialSkipUnknown_Always}
};
NCBI_PARAM_ENUM_DECL(ESerialSkipUnknown, SERIAL, SKIP_UNKNOWN_VARIANTS);
NCBI_PARAM_ENUM_DEF(ESerialSkipUnknown, SERIAL, SKIP_UNKNOWN_VARIANTS, eSerialSkipUnknown_No);
static NCBI_PARAM_TYPE(SERIAL, SKIP_UNKNOWN_VARIANTS) ms_SkipUnknownVariantsDefault;

void CObjectIStream::SetSkipUnknownVariantsThread(ESerialSkipUnknown skip)
{
    ESerialSkipUnknown now = ms_SkipUnknownVariantsDefault.GetThreadDefault();
    if (now != eSerialSkipUnknown_Never &&
        now != eSerialSkipUnknown_Always) {
        if (skip == eSerialSkipUnknown_Default) {
            ms_SkipUnknownVariantsDefault.ResetThreadDefault();
        } else {
            ms_SkipUnknownVariantsDefault.SetThreadDefault(skip);
        }
    }
}

void CObjectIStream::SetSkipUnknownVariantsGlobal(ESerialSkipUnknown skip)
{
    if (skip == eSerialSkipUnknown_Default) {
        return;
    }
    ESerialSkipUnknown now = ms_SkipUnknownVariantsDefault.GetDefault();
    if (now != eSerialSkipUnknown_Never &&
        now != eSerialSkipUnknown_Always) {
        if (skip == eSerialSkipUnknown_Default) {
            ms_SkipUnknownVariantsDefault.ResetDefault();
        } else {
            ms_SkipUnknownVariantsDefault.SetDefault(skip);
        }
    }
}

ESerialSkipUnknown CObjectIStream::x_GetSkipUnknownVariantsDefault(void)
{
    return ms_SkipUnknownVariantsDefault.GetThreadDefault();
}


ESerialSkipUnknown CObjectIStream::UpdateSkipUnknownMembers(void)
{
    ESerialSkipUnknown skip = m_SkipUnknown;
    if ( skip == eSerialSkipUnknown_Default ) {
        skip = x_GetSkipUnknownDefault();
        if ( skip == eSerialSkipUnknown_Default ) {
            skip = eSerialSkipUnknown_No;
        }
        m_SkipUnknown = skip;
    }
    return skip;
}


ESerialSkipUnknown CObjectIStream::UpdateSkipUnknownVariants(void)
{
    ESerialSkipUnknown skip = m_SkipUnknownVariants;
    if ( skip == eSerialSkipUnknown_Default ) {
        skip = x_GetSkipUnknownVariantsDefault();
        if ( skip == eSerialSkipUnknown_Default ) {
            skip = eSerialSkipUnknown_No;
        }
        m_SkipUnknownVariants = skip;
    }
    return skip;
}


/////////////////////////////////////////////////////////////////////////////

CObjectIStream::CObjectIStream(ESerialDataFormat format)
    : m_DiscardCurrObject(false),
      m_DataFormat(format),
      m_VerifyData(x_GetVerifyDataDefault()),
      m_SkipUnknown(eSerialSkipUnknown_Default),
      m_SkipUnknownVariants(eSerialSkipUnknown_Default),
      m_Fail(fNotOpen),
      m_Flags(fFlagNone)
{
}

CObjectIStream::~CObjectIStream(void)
{
    try {
        Close();
        ResetLocalHooks();
    }
    catch (...) {
        ERR_POST_X(1, "Cannot close input stream");
    }
}

void CObjectIStream::Open(CByteSourceReader& reader)
{
    Close();
    _ASSERT(m_Fail == fNotOpen);
    m_Input.Open(reader);
    m_Fail = 0;
}

void CObjectIStream::OpenFromBuffer(const char* buffer, size_t size)
{
    Close();
    _ASSERT(m_Fail == fNotOpen);
    m_Input.Open(buffer, size);
    m_Fail = 0;
}

void CObjectIStream::Open(CByteSource& source)
{
    CRef<CByteSourceReader> reader = source.Open();
    Open(*reader);
}

void CObjectIStream::Open(CNcbiIstream& inStream, bool deleteInStream)
{
    CRef<CByteSource> src = GetSource(inStream, deleteInStream);
    Open(*src);
}

void CObjectIStream::ResetLocalHooks(void)
{
    CMutexGuard guard(GetTypeInfoMutex());
    m_ObjectHookKey.Clear();
    m_ClassMemberHookKey.Clear();
    m_ChoiceVariantHookKey.Clear();
    m_ObjectSkipHookKey.Clear();
    m_ClassMemberSkipHookKey.Clear();
    m_ChoiceVariantSkipHookKey.Clear();
}

void CObjectIStream::Close(void)
{
    if (m_Fail != fNotOpen) {
        m_Input.Close();
        if ( m_Objects )
            m_Objects->Clear();
        ClearStack();
        m_Fail = fNotOpen;
    }
}

CObjectIStream::TFailFlags
CObjectIStream::SetFailFlags(TFailFlags flags,
                             const char* /* message */)
{
    TFailFlags old = m_Fail;
    if (flags == fNoError) {
        m_Fail = flags;
    } else {
        m_Fail |= flags;
        if ( !old && flags ) {
            // first fail
// redundant
//            ERR_POST_X(2, Error << "CObjectIStream: error at "<<
//                       GetPosition()<<": "<<GetStackTrace() << ": " << message);
        }
    }
    return old;
}

bool CObjectIStream::InGoodState(void)
{
    if ( fail() ) {
        // fail flag already set
        return false;
    }
    else if ( m_Input.fail() ) {
        // IO exception thrown without setting fail flag
        SetFailFlags(fReadError, m_Input.GetError());
        m_Input.ResetFail();
        return false;
    }
    else {
        // ok
        return true;
    }
}

bool CObjectIStream::EndOfData(void)
{
    const TFailFlags failure =
        fEOF | fReadError | fFormatError | fOverflow | fInvalidData |
        fIllegalCall | fFail | fNotOpen | fNotImplemented;
    if (GetFailFlags() & failure || m_Input.EndOfData()) {
        return true;
    }
    try {
        m_Input.PeekChar();
    } catch (...) {
        return true;
    }
    return false;
}

void CObjectIStream::HandleEOF(CEofException& expt)
{
    string msg(TopFrame().GetFrameInfo());
    PopFrame();
    if (GetStackDepth() < 2) {
        NCBI_RETHROW_SAME(expt,msg);
    } else {
        ThrowError(fEOF, msg);
    }
}

void CObjectIStream::Unended(const string& msg)
{
    if ( InGoodState() )
        ThrowError(fFail, msg);
}

void CObjectIStream::UnendedFrame(void)
{
    Unended("internal error: unended object stack frame");
}

void CObjectIStream::x_SetPathHooks(bool set)
{
    if (!m_PathReadObjectHooks.IsEmpty()) {
        CReadObjectHook* hook = m_PathReadObjectHooks.GetHook(*this);
        if (hook) {
            CTypeInfo* item = m_PathReadObjectHooks.FindType(*this);
            if (item) {
                if (set) {
                    item->SetLocalReadHook(*this, hook);
                } else {
                    item->ResetLocalReadHook(*this);
                }
            }
        }
    }
    if (!m_PathSkipObjectHooks.IsEmpty()) {
        CSkipObjectHook* hook = m_PathSkipObjectHooks.GetHook(*this);
        if (hook) {
            CTypeInfo* item = m_PathSkipObjectHooks.FindType(*this);
            if (item) {
                if (set) {
                    item->SetLocalSkipHook(*this, hook);
                } else {
                    item->ResetLocalSkipHook(*this);
                }
            }
        }
    }
    if (!m_PathReadMemberHooks.IsEmpty()) {
        CReadClassMemberHook* hook = m_PathReadMemberHooks.GetHook(*this);
        if (hook) {
            CMemberInfo* item = m_PathReadMemberHooks.FindItem(*this);
            if (item) {
                if (set) {
                    item->SetLocalReadHook(*this, hook);
                } else {
                    item->ResetLocalReadHook(*this);
                }
            }
        }
    }
    if (!m_PathSkipMemberHooks.IsEmpty()) {
        CSkipClassMemberHook* hook = m_PathSkipMemberHooks.GetHook(*this);
        if (hook) {
            CMemberInfo* item = m_PathSkipMemberHooks.FindItem(*this);
            if (item) {
                if (set) {
                    item->SetLocalSkipHook(*this, hook);
                } else {
                    item->ResetLocalSkipHook(*this);
                }
            }
        }
    }
    if (!m_PathReadVariantHooks.IsEmpty()) {
        CReadChoiceVariantHook* hook = m_PathReadVariantHooks.GetHook(*this);
        if (hook) {
            CVariantInfo* item = m_PathReadVariantHooks.FindItem(*this);
            if (item) {
                if (set) {
                    item->SetLocalReadHook(*this, hook);
                } else {
                    item->ResetLocalReadHook(*this);
                }
            }
        }
    }
    if (!m_PathSkipVariantHooks.IsEmpty()) {
        CSkipChoiceVariantHook* hook = m_PathSkipVariantHooks.GetHook(*this);
        if (hook) {
            CVariantInfo* item = m_PathSkipVariantHooks.FindItem(*this);
            if (item) {
                if (set) {
                    item->SetLocalSkipHook(*this, hook);
                } else {
                    item->ResetLocalSkipHook(*this);
                }
            }
        }
    }
}

void CObjectIStream::SetPathReadObjectHook(const string& path,
                                           CReadObjectHook* hook)
{
    m_PathReadObjectHooks.SetHook(path,hook);
    WatchPathHooks();
}
void CObjectIStream::SetPathSkipObjectHook(const string& path,
                                           CSkipObjectHook* hook)
{
    m_PathSkipObjectHooks.SetHook(path,hook);
    WatchPathHooks();
}
void CObjectIStream::SetPathReadMemberHook(const string& path,
                                            CReadClassMemberHook* hook)
{
    m_PathReadMemberHooks.SetHook(path,hook);
    WatchPathHooks();
}
void CObjectIStream::SetPathSkipMemberHook(const string& path,
                                            CSkipClassMemberHook* hook)
{
    m_PathSkipMemberHooks.SetHook(path,hook);
    WatchPathHooks();
}
void CObjectIStream::SetPathReadVariantHook(const string& path,
                                            CReadChoiceVariantHook* hook)
{
    m_PathReadVariantHooks.SetHook(path,hook);
    WatchPathHooks();
}
void CObjectIStream::SetPathSkipVariantHook(const string& path,
                                            CSkipChoiceVariantHook* hook)
{
    m_PathSkipVariantHooks.SetHook(path,hook);
    WatchPathHooks();
}

void CObjectIStream::UseMemoryPool(void)
{
    SetMemoryPool(new CObjectMemoryPool);
}

string CObjectIStream::GetStackTrace(void) const
{
    return GetStackTraceASN();
}

CNcbiStreampos CObjectIStream::GetStreamOffset(void) const
{
    return m_Input.GetStreamPos();
}

CNcbiStreampos CObjectIStream::GetStreamPos(void) const
{
    return m_Input.GetStreamPos();
}

void CObjectIStream::SetStreamOffset(CNcbiStreampos pos)
{
    m_Input.SetStreamPos(pos);
}

void CObjectIStream::SetStreamPos(CNcbiStreampos pos)
{
    m_Input.SetStreamPos(pos);
}

string CObjectIStream::GetPosition(void) const
{
    return "byte "+NStr::Int8ToString(NcbiStreamposToInt8(GetStreamPos()));
}

void CObjectIStream::ThrowError1(const CDiagCompileInfo& diag_info, 
                                 TFailFlags fail, const char* message)
{
    ThrowError1(diag_info,fail,string(message));
}

void CObjectIStream::ThrowError1(const CDiagCompileInfo& diag_info, 
                                 TFailFlags fail, const string& message)
{
    CSerialException::EErrCode err;
    SetFailFlags(fail, message.c_str());
    switch(fail)
    {
    case fNoError:
        CNcbiDiag(diag_info, eDiag_Trace) << ErrCode(NCBI_ERRCODE_X, 6)
                                          << message;
        return;
    case fEOF:            err = CSerialException::eEOF;            break;
    default:
    case fReadError:      err = CSerialException::eIoError;        break;
    case fFormatError:    err = CSerialException::eFormatError;    break;
    case fOverflow:       err = CSerialException::eOverflow;       break;
    case fInvalidData:    err = CSerialException::eInvalidData;    break;
    case fIllegalCall:    err = CSerialException::eIllegalCall;    break;
    case fFail:           err = CSerialException::eFail;           break;
    case fNotOpen:        err = CSerialException::eNotOpen;        break;
    case fMissingValue:   err = CSerialException::eMissingValue;   break;
    case fNotImplemented: err = CSerialException::eNotImplemented; break;
    }
    throw CSerialException(diag_info,0,err,GetPosition()+": "+message);
}

static inline
TTypeInfo MapType(const string& name)
{
    return CClassTypeInfoBase::GetClassInfoByName(name);
}

void CObjectIStream::RegisterObject(TTypeInfo typeInfo)
{
    if ( m_Objects )
        m_Objects->RegisterObject(typeInfo);
}

void CObjectIStream::RegisterObject(TObjectPtr objectPtr, TTypeInfo typeInfo)
{
    if ( m_Objects )
        m_Objects->RegisterObject(objectPtr, typeInfo);
}

const CReadObjectInfo&
CObjectIStream::GetRegisteredObject(CReadObjectInfo::TObjectIndex index)
{
    if ( !m_Objects ) {
        ThrowError(fFormatError,"invalid object index: NO_COLLECT defined");
    }
    return m_Objects->GetRegisteredObject(index);
}

// root reader
void CObjectIStream::SkipFileHeader(TTypeInfo typeInfo)
{
    BEGIN_OBJECT_FRAME2(eFrameNamed, typeInfo);
    
    string name = ReadFileHeader();
    const string& tname = typeInfo->GetName();
    if ( !name.empty() && !tname.empty() && name != tname ) {
        ThrowError(fFormatError,
                   "incompatible type "+name+"<>"+typeInfo->GetName());
    }

    END_OBJECT_FRAME();
}

void CObjectIStream::EndOfRead(void)
{
    if ( m_Objects )
        m_Objects->Clear();
}

void CObjectIStream::Read(const CObjectInfo& object, ENoFileHeader)
{
    // root object
    BEGIN_OBJECT_FRAME2(eFrameNamed, object.GetTypeInfo());
    
    ReadObject(object);

    EndOfRead();
    
    END_OBJECT_FRAME();
}

void CObjectIStream::Read(const CObjectInfo& object)
{
    // root object
    SkipFileHeader(object.GetTypeInfo());
    Read(object, eNoFileHeader);
}

void CObjectIStream::Read(TObjectPtr object, TTypeInfo typeInfo, ENoFileHeader)
{
    // root object
    BEGIN_OBJECT_FRAME2(eFrameNamed, typeInfo);

    ReadObject(object, typeInfo);
    
    EndOfRead();

    END_OBJECT_FRAME();
}

void CObjectIStream::Read(TObjectPtr object, TTypeInfo typeInfo)
{
    // root object
    SkipFileHeader(typeInfo);
    Read(object, typeInfo, eNoFileHeader);
}

CObjectInfo CObjectIStream::Read(TTypeInfo typeInfo)
{
    // root object
    SkipFileHeader(typeInfo);
    CObjectInfo info(typeInfo->Create(), typeInfo);
    Read(info, eNoFileHeader);
    return info;
}

CObjectInfo CObjectIStream::Read(const CObjectTypeInfo& type)
{
    return Read(type.GetTypeInfo());
}

void CObjectIStream::Skip(TTypeInfo typeInfo, ENoFileHeader)
{
    BEGIN_OBJECT_FRAME2(eFrameNamed, typeInfo);

    SkipObject(typeInfo);
    
    EndOfRead();

    END_OBJECT_FRAME();
}

void CObjectIStream::Skip(TTypeInfo typeInfo)
{
    SkipFileHeader(typeInfo);
    Skip(typeInfo, eNoFileHeader);
}

void CObjectIStream::Skip(const CObjectTypeInfo& type)
{
    Skip(type.GetTypeInfo());
}

void CObjectIStream::StartDelayBuffer(void)
{
    m_Input.StartSubSource();
}

CRef<CByteSource> CObjectIStream::EndDelayBuffer(void)
{
    return m_Input.EndSubSource();
}

void CObjectIStream::EndDelayBuffer(CDelayBuffer& buffer,
                                    const CItemInfo* itemInfo,
                                    TObjectPtr objectPtr)
{
    CRef<CByteSource> src = EndDelayBuffer();
    buffer.SetData(itemInfo, objectPtr, GetDataFormat(), *src);
}

bool CObjectIStream::ExpectedMember(const CMemberInfo* memberInfo)
{
    const CItemInfo* info = CItemsInfo::FindNextMandatory(memberInfo);
    if (info) {
        if (GetVerifyData() == eSerialVerifyData_Yes) {
            ThrowError(fFormatError,
                    "member "+info->GetId().ToString()+" expected");
        } else {
            SetFailFlags(fMissingValue);
            ERR_POST_X(3, "member "+info->GetId().ToString()+" is missing");
        }
    }
    return (info != 0);
}

void CObjectIStream::DuplicatedMember(const CMemberInfo* memberInfo)
{
    ThrowError(fFormatError,
               "duplicate member: "+memberInfo->GetId().ToString());
}

void CObjectIStream::ReadSeparateObject(const CObjectInfo& object)
{
    if ( m_Objects ) {
        size_t firstObject = m_Objects->GetObjectCount();
        ReadObject(object);
        size_t lastObject = m_Objects->GetObjectCount();
        m_Objects->ForgetObjects(firstObject, lastObject);
    }
    else {
        ReadObject(object);
    }
}

void CObjectIStream::ReadExternalObject(TObjectPtr objectPtr,
                                        TTypeInfo typeInfo)
{
    _TRACE("CObjectIStream::Read("<<NStr::PtrToString(objectPtr)<<", "<<
           typeInfo->GetName()<<")");
    RegisterObject(objectPtr, typeInfo);
    ReadObject(objectPtr, typeInfo);
}

CObjectInfo CObjectIStream::ReadObject(void)
{
    TTypeInfo typeInfo = MapType(ReadFileHeader());
    TObjectPtr objectPtr = 0;
    BEGIN_OBJECT_FRAME2(eFrameNamed, typeInfo);

    CRef<CObject> ref;
    if ( typeInfo->IsCObject() ) {
        objectPtr = typeInfo->Create(GetMemoryPool());
        ref.Reset(static_cast<CObject*>(objectPtr));
    }
    else {
        objectPtr = typeInfo->Create();
    }
    RegisterObject(objectPtr, typeInfo);
    ReadObject(objectPtr, typeInfo);
    if ( typeInfo->IsCObject() )
        ref.Release();
    END_OBJECT_FRAME();
    return make_pair(objectPtr, typeInfo);
}

void CObjectIStream::ReadObject(const CObjectInfo& object)
{
    ReadObject(object.GetObjectPtr(), object.GetTypeInfo());
}

void CObjectIStream::SkipObject(const CObjectTypeInfo& objectType)
{
    SkipObject(objectType.GetTypeInfo());
}

void CObjectIStream::ReadClassMember(const CObjectInfo::CMemberIterator& member)
{
    const CMemberInfo* memberInfo = member.GetMemberInfo();
    TObjectPtr classPtr = member.GetClassObject().GetObjectPtr();
    ReadObject(memberInfo->GetMemberPtr(classPtr),
               memberInfo->GetTypeInfo());
}

void CObjectIStream::ReadChoiceVariant(const CObjectInfoCV& object)
{
    const CVariantInfo* variantInfo = object.GetVariantInfo();
    TObjectPtr choicePtr = object.GetChoiceObject().GetObjectPtr();
    variantInfo->DefaultReadVariant(*this, choicePtr);
}

string CObjectIStream::ReadFileHeader(void)
{
    // this is to check if the file is empty or not
    m_Input.PeekChar();
    return NcbiEmptyString;
}

string CObjectIStream::PeekNextTypeName(void)
{
    return NcbiEmptyString;
}

pair<TObjectPtr, TTypeInfo> CObjectIStream::ReadPointer(TTypeInfo declaredType)
{
    _TRACE("CObjectIStream::ReadPointer("<<declaredType->GetName()<<")");
    TObjectPtr objectPtr = 0;
    TTypeInfo objectType = 0;
    switch ( ReadPointerType() ) {
    case eNullPointer:
        _TRACE("CObjectIStream::ReadPointer: null");
        return pair<TObjectPtr, TTypeInfo>(0, declaredType);
    case eObjectPointer:
        {
            _TRACE("CObjectIStream::ReadPointer: @...");
            TObjectIndex index = ReadObjectPointer();
            _TRACE("CObjectIStream::ReadPointer: @" << index);
            const CReadObjectInfo& info = GetRegisteredObject(index);
            objectType = info.GetTypeInfo();
            objectPtr = info.GetObjectPtr();
            if ( !objectPtr ) {
                ThrowError(fFormatError,
                    "invalid reference to skipped object: object ptr is NULL");
            }
            break;
        }
    case eThisPointer:
        {
            _TRACE("CObjectIStream::ReadPointer: new");
            CRef<CObject> ref;
            if ( declaredType->IsCObject() ) {
                objectPtr = declaredType->Create(GetMemoryPool());
                ref.Reset(static_cast<CObject*>(objectPtr));
            }
            else {
                objectPtr = declaredType->Create();
            }
            RegisterObject(objectPtr, declaredType);
            ReadObject(objectPtr, declaredType);
            if ( declaredType->IsCObject() )
                ref.Release();
            return make_pair(objectPtr, declaredType);
        }
    case eOtherPointer:
        {
            _TRACE("CObjectIStream::ReadPointer: new...");
            string className = ReadOtherPointer();
            _TRACE("CObjectIStream::ReadPointer: new " << className);
            objectType = MapType(className);

            BEGIN_OBJECT_FRAME2(eFrameNamed, objectType);
                
            CRef<CObject> ref;
            if ( objectType->IsCObject() ) {
                objectPtr = objectType->Create(GetMemoryPool());
                ref.Reset(static_cast<CObject*>(objectPtr));
            }
            else {
                objectPtr = objectType->Create();
            }
            RegisterObject(objectPtr, objectType);
            ReadObject(objectPtr, objectType);
            if ( objectType->IsCObject() )
                ref.Release();
                
            END_OBJECT_FRAME();

            ReadOtherPointerEnd();
            break;
        }
    default:
        ThrowError(fFormatError,"illegal pointer type");
        objectPtr = 0;
        objectType = 0;
        break;
    }
    while ( objectType != declaredType ) {
        // try to check parent class pointer
        if ( objectType->GetTypeFamily() != eTypeFamilyClass ) {
            ThrowError(fFormatError,"incompatible member type");
        }
        const CClassTypeInfo* parentClass =
            CTypeConverter<CClassTypeInfo>::SafeCast(objectType)->GetParentClassInfo();
        if ( parentClass ) {
            objectType = parentClass;
        }
        else {
            ThrowError(fFormatError,"incompatible member type");
        }
    }
    return make_pair(objectPtr, objectType);
}

void CObjectIStream::ReadOtherPointerEnd(void)
{
}

void CObjectIStream::SkipExternalObject(TTypeInfo typeInfo)
{
    _TRACE("CObjectIStream::SkipExternalObject("<<typeInfo->GetName()<<")");
    RegisterObject(typeInfo);
    SkipObject(typeInfo);
}

void CObjectIStream::SkipPointer(TTypeInfo declaredType)
{
    _TRACE("CObjectIStream::SkipPointer("<<declaredType->GetName()<<")");
    switch ( ReadPointerType() ) {
    case eNullPointer:
        _TRACE("CObjectIStream::SkipPointer: null");
        return;
    case eObjectPointer:
        {
            _TRACE("CObjectIStream::SkipPointer: @...");
            TObjectIndex index = ReadObjectPointer();
            _TRACE("CObjectIStream::SkipPointer: @" << index);
            GetRegisteredObject(index);
            break;
        }
    case eThisPointer:
        {
            _TRACE("CObjectIStream::ReadPointer: new");
            RegisterObject(declaredType);
            SkipObject(declaredType);
            break;
        }
    case eOtherPointer:
        {
            _TRACE("CObjectIStream::ReadPointer: new...");
            string className = ReadOtherPointer();
            _TRACE("CObjectIStream::ReadPointer: new " << className);
            TTypeInfo typeInfo = MapType(className);
            BEGIN_OBJECT_FRAME2(eFrameNamed, typeInfo);
                
            RegisterObject(typeInfo);
            SkipObject(typeInfo);

            END_OBJECT_FRAME();
            ReadOtherPointerEnd();
            break;
        }
    default:
        ThrowError(fFormatError,"illegal pointer type");
    }
}

void CObjectIStream::BeginNamedType(TTypeInfo /*namedTypeInfo*/)
{
}

void CObjectIStream::EndNamedType(void)
{
}

void CObjectIStream::ReadNamedType(TTypeInfo
#ifndef VIRTUAL_MID_LEVEL_IO
                                   namedTypeInfo
#endif
                                   ,
                                   TTypeInfo typeInfo, TObjectPtr object)
{
#ifndef VIRTUAL_MID_LEVEL_IO
    BEGIN_OBJECT_FRAME2(eFrameNamed, namedTypeInfo);
    BeginNamedType(namedTypeInfo);
#endif
    ReadObject(object, typeInfo);
#ifndef VIRTUAL_MID_LEVEL_IO
    EndNamedType();
    END_OBJECT_FRAME();
#endif
}

void CObjectIStream::SkipNamedType(TTypeInfo namedTypeInfo,
                                   TTypeInfo typeInfo)
{
    BEGIN_OBJECT_FRAME2(eFrameNamed, namedTypeInfo);
    BeginNamedType(namedTypeInfo);

    SkipObject(typeInfo);

    EndNamedType();
    END_OBJECT_FRAME();
}

void CObjectIStream::EndContainerElement(void)
{
}

void CObjectIStream::ReadContainer(const CContainerTypeInfo* containerType,
                                   TObjectPtr containerPtr)
{
    BEGIN_OBJECT_FRAME2(eFrameArray, containerType);
    BeginContainer(containerType);

    TTypeInfo elementType = containerType->GetElementType();
    BEGIN_OBJECT_FRAME2(eFrameArrayElement, elementType);

    CContainerTypeInfo::CIterator iter;
    bool old_element = containerType->InitIterator(iter, containerPtr);
    while ( BeginContainerElement(elementType) ) {
        if ( old_element ) {
            elementType->ReadData(*this, containerType->GetElementPtr(iter));
            old_element = containerType->NextElement(iter);
        }
        else {
            containerType->AddElement(containerPtr, *this);
        }
        EndContainerElement();
    }
    if ( old_element ) {
        containerType->EraseAllElements(iter);
    }

    END_OBJECT_FRAME();

    EndContainer();
    END_OBJECT_FRAME();
}

void CObjectIStream::SkipContainer(const CContainerTypeInfo* containerType)
{
    BEGIN_OBJECT_FRAME2(eFrameArray, containerType);
    BeginContainer(containerType);

    TTypeInfo elementType = containerType->GetElementType();
    BEGIN_OBJECT_FRAME2(eFrameArrayElement, elementType);

    while ( BeginContainerElement(elementType) ) {
        SkipObject(elementType);
        EndContainerElement();
    }

    END_OBJECT_FRAME();

    EndContainer();
    END_OBJECT_FRAME();
}

void CObjectIStream::EndClass(void)
{
}

void CObjectIStream::EndClassMember(void)
{
}

void CObjectIStream::ReadClassRandom(const CClassTypeInfo* classType,
                                     TObjectPtr classPtr)
{
    BEGIN_OBJECT_FRAME2(eFrameClass, classType);
    BeginClass(classType);

    ReadClassRandomContentsBegin(classType);

    TMemberIndex index;
    while ( (index = BeginClassMember(classType)) != kInvalidMember ) {

        ReadClassRandomContentsMember(classPtr);
        
        EndClassMember();
    }

    ReadClassRandomContentsEnd();
    
    EndClass();
    END_OBJECT_FRAME();
}

void CObjectIStream::ReadClassSequential(const CClassTypeInfo* classType,
                                         TObjectPtr classPtr)
{
    TMemberIndex prevIndex = kInvalidMember;
    BEGIN_OBJECT_FRAME2(eFrameClass, classType);
    BeginClass(classType);
    
    ReadClassSequentialContentsBegin(classType);

    TMemberIndex index;
    while ( (index = BeginClassMember(classType, *pos)) != kInvalidMember ) {

        if ((prevIndex != kInvalidMember) && (prevIndex >= index)) {
            const CMemberInfo *mem_info = classType->GetMemberInfo(index);
            if (mem_info->GetId().HaveNoPrefix()) {
                UndoClassMember();
                break;
            }
        }
        prevIndex = index;

        ReadClassSequentialContentsMember(classPtr);

        EndClassMember();
    }

    ReadClassSequentialContentsEnd(classPtr);
    
    EndClass();
    END_OBJECT_FRAME();
}

void CObjectIStream::SkipClassRandom(const CClassTypeInfo* classType)
{
    BEGIN_OBJECT_FRAME2(eFrameClass, classType);
    BeginClass(classType);
    
    SkipClassRandomContentsBegin(classType);

    TMemberIndex index;
    while ( (index = BeginClassMember(classType)) != kInvalidMember ) {

        SkipClassRandomContentsMember();

        EndClassMember();
    }

    SkipClassRandomContentsEnd();
    
    EndClass();
    END_OBJECT_FRAME();
}

void CObjectIStream::SkipClassSequential(const CClassTypeInfo* classType)
{
    BEGIN_OBJECT_FRAME2(eFrameClass, classType);
    BeginClass(classType);
    
    SkipClassSequentialContentsBegin(classType);

    TMemberIndex index;
    while ( (index = BeginClassMember(classType, *pos)) != kInvalidMember ) {

        SkipClassSequentialContentsMember();

        EndClassMember();
    }

    SkipClassSequentialContentsEnd();
    
    EndClass();
    END_OBJECT_FRAME();
}

void CObjectIStream::BeginChoice(const CChoiceTypeInfo* /*choiceType*/)
{
}
void CObjectIStream::EndChoice(void)
{
}
void CObjectIStream::EndChoiceVariant(void)
{
}

void CObjectIStream::ReadChoice(const CChoiceTypeInfo* choiceType,
                                TObjectPtr choicePtr)
{
    BEGIN_OBJECT_FRAME2(eFrameChoice, choiceType);
    BeginChoice(choiceType);
    BEGIN_OBJECT_FRAME(eFrameChoiceVariant);
    TMemberIndex index = BeginChoiceVariant(choiceType);
    _ASSERT(index != kInvalidMember);

    const CVariantInfo* variantInfo = choiceType->GetVariantInfo(index);
    SetTopMemberId(variantInfo->GetId());

    variantInfo->ReadVariant(*this, choicePtr);

    EndChoiceVariant();
    END_OBJECT_FRAME();
    EndChoice();
    END_OBJECT_FRAME();
}

void CObjectIStream::SkipChoice(const CChoiceTypeInfo* choiceType)
{
    BEGIN_OBJECT_FRAME2(eFrameChoice, choiceType);
    BeginChoice(choiceType);
    BEGIN_OBJECT_FRAME(eFrameChoiceVariant);
    TMemberIndex index = BeginChoiceVariant(choiceType);
    if ( index == kInvalidMember )
        ThrowError(fFormatError,"choice variant id expected");

    const CVariantInfo* variantInfo = choiceType->GetVariantInfo(index);
    SetTopMemberId(variantInfo->GetId());

    variantInfo->SkipVariant(*this);

    EndChoiceVariant();
    END_OBJECT_FRAME();
    EndChoice();
    END_OBJECT_FRAME();
}

void CObjectIStream::ReadAlias(const CAliasTypeInfo* aliasType,
                               TObjectPtr aliasPtr)
{
    ReadNamedType(aliasType, aliasType->GetPointedType(),
        aliasType->GetDataPtr(aliasPtr));
}

void CObjectIStream::SkipAlias(const CAliasTypeInfo* aliasType)
{
    SkipNamedType(aliasType, aliasType->GetPointedType());
}

///////////////////////////////////////////////////////////////////////
//
// CObjectIStream::ByteBlock
//

CObjectIStream::ByteBlock::ByteBlock(CObjectIStream& in)
    : m_Stream(in), m_KnownLength(false), m_Ended(false), m_Length(1)
{
    in.BeginBytes(*this);
}

CObjectIStream::ByteBlock::~ByteBlock(void)
{
    if ( !m_Ended ) {
        try {
            GetStream().Unended("byte block not fully read");
        }
        catch (...) {
            ERR_POST_X(4, "unended byte block");
        }
    }
}

void CObjectIStream::ByteBlock::End(void)
{
    _ASSERT(!m_Ended);
    if ( m_Length == 0 ) {
        GetStream().EndBytes(*this);
        m_Ended = true;
    }
}

size_t CObjectIStream::ByteBlock::Read(void* dst, size_t needLength,
                                       bool forceLength)
{
    size_t length;
    if ( KnownLength() ) {
        if ( m_Length < needLength )
            length = m_Length;
        else
            length = needLength;
    }
    else {
        if ( m_Length == 0 )
            length = 0;
        else
            length = needLength;
    }
    
    if ( length == 0 ) {
        if ( forceLength && needLength != 0 )
            GetStream().ThrowError(fReadError, "read fault");
        return 0;
    }

    length = GetStream().ReadBytes(*this, static_cast<char*>(dst), length);
    if ( KnownLength() )
        m_Length -= length;
    if ( forceLength && needLength != length )
        GetStream().ThrowError(fReadError, "read fault");
    return length;
}

///////////////////////////////////////////////////////////////////////
//
// CObjectIStream::CharBlock
//

CObjectIStream::CharBlock::CharBlock(CObjectIStream& in)
    : m_Stream(in), m_KnownLength(false), m_Ended(false), m_Length(1)
{
    in.BeginChars(*this);
}

CObjectIStream::CharBlock::~CharBlock(void)
{
    if ( !m_Ended ) {
        try {
            GetStream().Unended("char block not fully read");
        }
        catch (...) {
            ERR_POST_X(5, "unended char block");
        }
    }
}

void CObjectIStream::CharBlock::End(void)
{
    _ASSERT(!m_Ended);
    if ( m_Length == 0 ) {
        GetStream().EndChars(*this);
        m_Ended = true;
    }
}

size_t CObjectIStream::CharBlock::Read(char* dst, size_t needLength,
                                       bool forceLength)
{
    size_t length;
    if ( KnownLength() ) {
        if ( m_Length < needLength )
            length = m_Length;
        else
            length = needLength;
    }
    else {
        if ( m_Length == 0 )
            length = 0;
        else
            length = needLength;
    }
    
    if ( length == 0 ) {
        if ( forceLength && needLength != 0 )
            GetStream().ThrowError(fReadError, "read fault");
        return 0;
    }

    length = GetStream().ReadChars(*this, dst, length);
    if ( KnownLength() )
        m_Length -= length;
    if ( forceLength && needLength != length )
        GetStream().ThrowError(fReadError, "read fault");
    return length;
}


void CObjectIStream::EndBytes(const ByteBlock& /*b*/)
{
}

void CObjectIStream::EndChars(const CharBlock& /*b*/)
{
}

Int1 CObjectIStream::ReadInt1(void)
{
    Int4 data = ReadInt4();
    Int1 ret = Int1(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

Uint1 CObjectIStream::ReadUint1(void)
{
    Uint4 data = ReadUint4();
    Uint1 ret = Uint1(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

Int2 CObjectIStream::ReadInt2(void)
{
    Int4 data = ReadInt4();
    Int2 ret = Int2(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

Uint2 CObjectIStream::ReadUint2(void)
{
    Uint4 data = ReadUint4();
    Uint2 ret = Uint2(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

Int4 CObjectIStream::ReadInt4(void)
{
    Int8 data = ReadInt8();
    Int4 ret = Int4(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

Uint4 CObjectIStream::ReadUint4(void)
{
    Uint8 data = ReadUint8();
    Uint4 ret = Uint4(data);
    if ( ret != data )
        ThrowError(fOverflow, "integer overflow");
    return ret;
}

float CObjectIStream::ReadFloat(void)
{
    double data = ReadDouble();
#if defined(FLT_MIN) && defined(FLT_MAX)
    if ( data < FLT_MIN || data > FLT_MAX )
        ThrowError(fOverflow, "float overflow");
#endif
    return float(data);
}

#if SIZEOF_LONG_DOUBLE != 0
long double CObjectIStream::ReadLDouble(void)
{
    return ReadDouble();
}
#endif

char* CObjectIStream::ReadCString(void)
{
    string s;
    ReadString(s);
    return strdup(s.c_str());
}

void CObjectIStream::ReadStringStore(string& s)
{
    ReadString(s);
}

void CObjectIStream::ReadPackedString(string& s,
                                      CPackString& pack_string,
                                      EStringType type)
{
    ReadString(s, type);
    pack_string.Pack(s);
}

void CObjectIStream::SkipInt1(void)
{
    SkipSNumber();
}

void CObjectIStream::SkipUint1(void)
{
    SkipUNumber();
}

void CObjectIStream::SkipInt2(void)
{
    SkipSNumber();
}

void CObjectIStream::SkipUint2(void)
{
    SkipUNumber();
}

void CObjectIStream::SkipInt4(void)
{
    SkipSNumber();
}

void CObjectIStream::SkipUint4(void)
{
    SkipUNumber();
}

void CObjectIStream::SkipInt8(void)
{
    SkipSNumber();
}

void CObjectIStream::SkipUint8(void)
{
    SkipUNumber();
}

void CObjectIStream::SkipFloat(void)
{
    SkipFNumber();
}

void CObjectIStream::SkipDouble(void)
{
    SkipFNumber();
}

#if SIZEOF_LONG_DOUBLE != 0
void CObjectIStream::SkipLDouble(void)
{
    SkipFNumber();
}
#endif

void CObjectIStream::SkipCString(void)
{
    SkipString();
}

void CObjectIStream::SkipStringStore(void)
{
    SkipString();
}

void CObjectIStream::SkipAnyContentVariant(void)
{
    SkipAnyContentObject();
}

void CObjectIStream::ReadCompressedBitString(CBitString& obj)
{
    ByteBlock bl(*this);
    vector<unsigned char> v;
    unsigned char buf[2048];
    size_t count;
    while ( (count = bl.Read(buf, sizeof(buf))) != 0 ) {
        v.insert(v.end(), buf, buf + count);
    }
    bm::deserialize(obj, reinterpret_cast<const unsigned char*>(&v.front()));
    bl.End();
}

char ReplaceVisibleChar(char c, EFixNonPrint fix_method, size_t at_line)
{
    _ASSERT(fix_method != eFNP_Allow);
    if ( fix_method != eFNP_Replace ) {
        string message = "Bad char in VisibleString" +
            ((at_line > 0) ?
             " starting at line " + NStr::UIntToString(at_line) :
             string("")) + ": " + NStr::IntToString(int(c) & 0xff);
        switch (fix_method) {
        case eFNP_ReplaceAndWarn:
            CNcbiDiag(eDiag_Error, eDPF_Default)
                << ErrCode(NCBI_ERRCODE_X, 7) << message << Endm;
            break;
        case eFNP_Throw:
            NCBI_THROW(CSerialException,eFormatError,message);
        case eFNP_Abort:
            CNcbiDiag(eDiag_Fatal, eDPF_Default)
                << ErrCode(NCBI_ERRCODE_X, 8) << message << Endm;
            break;
        default:
            break;
        }
    }
    return '#';
}

END_NCBI_SCOPE
