#ifndef OBJOSTRJSON__HPP
#define OBJOSTRJSON__HPP

/*  $Id: objostrjson.hpp 155542 2009-03-24 16:30:48Z gouriano $
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
* Author: Andrei Gourianov
*
* File Description:
*   Encode data object using JSON format
*/

#include <corelib/ncbistd.hpp>
#include <serial/objostr.hpp>
#include <stack>


/** @addtogroup ObjStreamSupport
 *
 * @{
 */


BEGIN_NCBI_SCOPE

/////////////////////////////////////////////////////////////////////////////
///
/// CObjectOStreamJson --
///
/// Encode serial data object using JSON format
class NCBI_XSERIAL_EXPORT CObjectOStreamJson : public CObjectOStream
{
public:

    /// Constructor.
    ///
    /// @param out
    ///   Output stream    
    /// @param deleteOut
    ///   when TRUE, the output stream will be deleted automatically
    ///   when the writer is deleted
    CObjectOStreamJson(CNcbiOstream& out, bool deleteOut);

    /// Destructor.
    virtual ~CObjectOStreamJson(void);

    /// Set default encoding of 'string' objects
    ///
    /// @param enc
    ///   Encoding
    void SetDefaultStringEncoding(EEncoding enc);

    /// Get default encoding of 'string' objects
    ///
    /// @return
    ///   Encoding
    EEncoding GetDefaultStringEncoding(void) const;

    /// Get current stream position as string.
    /// Useful for diagnostic and information messages.
    ///
    /// @return
    ///   string
    virtual string GetPosition(void) const;

    virtual void WriteFileHeader(TTypeInfo type);
    virtual void EndOfWrite(void);

protected:
    virtual void WriteBool(bool data);
    virtual void WriteChar(char data);
    virtual void WriteInt4(Int4 data);
    virtual void WriteUint4(Uint4 data);
    virtual void WriteInt8(Int8 data);
    virtual void WriteUint8(Uint8 data);
    virtual void WriteFloat(float data);
    virtual void WriteDouble(double data);
    virtual void WriteCString(const char* str);
    virtual void WriteString(const string& s,
                             EStringType type = eStringTypeVisible);
    virtual void WriteStringStore(const string& s);
    virtual void CopyString(CObjectIStream& in);
    virtual void CopyStringStore(CObjectIStream& in);

    virtual void WriteNullPointer(void);
    virtual void WriteObjectReference(TObjectIndex index);
    virtual void WriteOtherBegin(TTypeInfo typeInfo);
    virtual void WriteOtherEnd(TTypeInfo typeInfo);
    virtual void WriteOther(TConstObjectPtr object, TTypeInfo typeInfo);

    virtual void WriteNull(void);
    virtual void WriteAnyContentObject(const CAnyContentObject& obj);
    virtual void CopyAnyContentObject(CObjectIStream& in);

    virtual void WriteBitString(const CBitString& obj);
    virtual void CopyBitString(CObjectIStream& in);

    virtual void WriteEnum(const CEnumeratedTypeValues& values,
                           TEnumValueType value);
    virtual void CopyEnum(const CEnumeratedTypeValues& values,
                          CObjectIStream& in);

#ifdef VIRTUAL_MID_LEVEL_IO
    virtual void WriteClassMember(const CMemberId& memberId,
                                  TTypeInfo memberType,
                                  TConstObjectPtr memberPtr);
    virtual bool WriteClassMember(const CMemberId& memberId,
                                  const CDelayBuffer& buffer);
#endif

    // low level I/O
    virtual void BeginNamedType(TTypeInfo namedTypeInfo);
    virtual void EndNamedType(void);

    virtual void BeginContainer(const CContainerTypeInfo* containerType);
    virtual void EndContainer(void);
    virtual void BeginContainerElement(TTypeInfo elementType);
    virtual void EndContainerElement(void);

    virtual void BeginClass(const CClassTypeInfo* classInfo);
    virtual void EndClass(void);
    virtual void BeginClassMember(const CMemberId& id);

    virtual void EndClassMember(void);

    virtual void BeginChoice(const CChoiceTypeInfo* choiceType);
    virtual void EndChoice(void);
    virtual void BeginChoiceVariant(const CChoiceTypeInfo* choiceType,
                                    const CMemberId& id);
    virtual void EndChoiceVariant(void);

    virtual void WriteBytes(const ByteBlock& block,
                            const char* bytes, size_t length);
    virtual void WriteChars(const CharBlock& block,
                            const char* chars, size_t length);

    // Write current separator to the stream
    virtual void WriteSeparator(void);

private:
    void WriteMemberId(const CMemberId& id);
    void WriteSkippedMember(void);
    void WriteEscapedChar(char c, EEncoding enc_in);
    void WriteEncodedChar(const char*& src, EStringType type);
    void x_WriteString(const string& value,
                       EStringType type = eStringTypeVisible);
    void WriteKey(const string& key);
    void WriteValue(const string& value,
                    EStringType type = eStringTypeVisible);
    void WriteKeywordValue(const string& value);
    void StartBlock(void);
    void EndBlock(void);
    void NextElement(void);
    void BeginArray(void);
    void EndArray(void);
    void NameSeparator(void);

    bool m_BlockStart;
    bool m_ExpectValue;
    string m_SkippedMemberId;
    EEncoding m_StringEncoding;
};


/* @} */

END_NCBI_SCOPE

#endif
