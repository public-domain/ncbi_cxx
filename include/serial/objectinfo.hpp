#ifndef OBJECTINFO__HPP
#define OBJECTINFO__HPP

/*  $Id: objectinfo.hpp 143121 2008-10-15 16:18:21Z vasilche $
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
*   Object information classes
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <serial/serialdef.hpp>
#include <serial/impl/continfo.hpp>
#include <serial/impl/ptrinfo.hpp>
#include <serial/impl/stdtypes.hpp>
#include <serial/impl/classinfo.hpp>
#include <serial/impl/choice.hpp>
#include <serial/impl/enumerated.hpp>
#include <vector>
#include <memory>


/** @addtogroup ObjStreamSupport
 *
 * @{
 */


BEGIN_NCBI_SCOPE

class CObjectTypeInfo;
class CConstObjectInfo;
class CObjectInfo;

class CPrimitiveTypeInfo;
class CClassTypeInfoBase;
class CClassTypeInfo;
class CChoiceTypeInfo;
class CContainerTypeInfo;
class CPointerTypeInfo;

class CMemberId;
class CItemInfo;
class CMemberInfo;
class CVariantInfo;

class CReadContainerElementHook;

class CObjectTypeInfoMI;
class CObjectTypeInfoVI;
class CObjectTypeInfoCV;
class CConstObjectInfoMI;
class CConstObjectInfoCV;
class CConstObjectInfoEI;
class CObjectInfoMI;
class CObjectInfoCV;
class CObjectInfoEI;

/// Facilitate access to the data type information.
/// No concrete object is referenced.
class NCBI_XSERIAL_EXPORT CObjectTypeInfo
{
public:
    typedef CObjectTypeInfoMI CMemberIterator;
    typedef CObjectTypeInfoVI CVariantIterator;
    typedef CObjectTypeInfoCV CChoiceVariant;

    CObjectTypeInfo(TTypeInfo typeinfo = 0);

    /// Get type name
    const string& GetName(void) const
        {
            return GetTypeInfo()->GetName();
        }

    /// Get data type family
    ETypeFamily GetTypeFamily(void) const;

    bool Valid(void) const
        {
            return m_TypeInfo != 0;
        }
    DECLARE_OPERATOR_BOOL_PTR(m_TypeInfo);
    
    bool operator==(const CObjectTypeInfo& type) const;
    bool operator!=(const CObjectTypeInfo& type) const;

    // primitive type interface
    // only when GetTypeFamily() == eTypeFamilyPrimitive
    EPrimitiveValueType GetPrimitiveValueType(void) const;
    bool IsPrimitiveValueSigned(void) const;
    // only when GetPrimitiveValueType() == ePrimitiveValueEnum
    const CEnumeratedTypeValues& GetEnumeratedTypeValues(void) const;

    // container interface
    // only when GetTypeFamily() == eTypeFamilyContainer
    CObjectTypeInfo GetElementType(void) const;

    // class interface
    // only when GetTypeFamily() == eTypeFamilyClass
    CMemberIterator BeginMembers(void) const;
    CMemberIterator FindMember(const string& memberName) const;
    CMemberIterator FindMemberByTag(int memberTag) const;

    // choice interface
    // only when GetTypeFamily() == eTypeFamilyChoice
    CVariantIterator BeginVariants(void) const;
    CVariantIterator FindVariant(const string& memberName) const;
    CVariantIterator FindVariantByTag(int memberTag) const;

    // pointer interface
    // only when GetTypeFamily() == eTypeFamilyPointer
    CObjectTypeInfo GetPointedType(void) const;

    /// Set local (for the specified stream) read hook
    /// @param stream
    ///   Input data stream reader
    /// @param hook
    ///   Pointer to hook object
    void SetLocalReadHook(CObjectIStream& stream,
                          CReadObjectHook* hook) const;

    /// Set global (for all streams) read hook
    /// @param hook
    ///   Pointer to hook object
    void SetGlobalReadHook(CReadObjectHook* hook) const;

    /// Reset local read hook
    /// @param stream
    ///   Input data stream reader
    void ResetLocalReadHook(CObjectIStream& stream) const;

    /// Reset global read hooks
    void ResetGlobalReadHook(void) const;

    /// Set local context-specific read hook
    /// @param stream
    ///   Input data stream reader
    /// @param path
    ///   Context (stack path)
    /// @param hook
    ///   Pointer to hook object
    void SetPathReadHook(CObjectIStream* stream, const string& path,
                         CReadObjectHook* hook) const;


    /// Set local (for the specified stream) write hook
    /// @param stream
    ///   Output data stream writer
    /// @param hook
    ///   Pointer to hook object
    void SetLocalWriteHook(CObjectOStream& stream,
                          CWriteObjectHook* hook) const;

    /// Set global (for all streams) write hook
    /// @param hook
    ///   Pointer to hook object
    void SetGlobalWriteHook(CWriteObjectHook* hook) const;

    /// Reset local write hook
    /// @param stream
    ///   Output data stream writer
    void ResetLocalWriteHook(CObjectOStream& stream) const;

    /// Reset global write hooks
    void ResetGlobalWriteHook(void) const;

    /// Set local context-specific write hook
    /// @param stream
    ///   Output data stream writer
    /// @param path
    ///   Context (stack path)
    /// @param hook
    ///   Pointer to hook object
    void SetPathWriteHook(CObjectOStream* stream, const string& path,
                          CWriteObjectHook* hook) const;


    /// Set local (for the specified stream) skip hook
    /// @param stream
    ///   Input data stream reader
    /// @param hook
    ///   Pointer to hook object
    void SetLocalSkipHook(CObjectIStream& stream,
                          CSkipObjectHook* hook) const;

    /// Set global (for all streams) skip hook
    /// @param hook
    ///   Pointer to hook object
    void SetGlobalSkipHook(CSkipObjectHook* hook) const;

    /// Reset local skip hook
    /// @param stream
    ///   Input data stream reader
    void ResetLocalSkipHook(CObjectIStream& stream) const;

    /// Reset global skip hooks
    void ResetGlobalSkipHook(void) const;

    /// Set local context-specific skip hook
    /// @param stream
    ///   Input data stream reader
    /// @param path
    ///   Context (stack path)
    /// @param hook
    ///   Pointer to hook object
    void SetPathSkipHook(CObjectIStream* stream, const string& path,
                         CSkipObjectHook* hook) const;


    /// Set local (for the specified stream) copy hook
    /// @param stream
    ///   Data copier
    /// @param hook
    ///   Pointer to hook object
    void SetLocalCopyHook(CObjectStreamCopier& stream,
                          CCopyObjectHook* hook) const;

    /// Set global (for all streams) copy hook
    /// @param hook
    ///   Pointer to hook object
    void SetGlobalCopyHook(CCopyObjectHook* hook) const;

    /// Reset local copy hook
    /// @param stream
    ///   Data copier
    void ResetLocalCopyHook(CObjectStreamCopier& stream) const;

    /// Reset global read hooks
    void ResetGlobalCopyHook(void) const;

    /// Set local context-specific copy hook
    /// @param stream
    ///   Data copier
    /// @param path
    ///   Context (stack path)
    /// @param hook
    ///   Pointer to hook object
    void SetPathCopyHook(CObjectStreamCopier* stream, const string& path,
                         CCopyObjectHook* hook) const;

public: // mostly for internal use
    TTypeInfo GetTypeInfo(void) const;
    const CPrimitiveTypeInfo* GetPrimitiveTypeInfo(void) const;
    const CEnumeratedTypeInfo* GetEnumeratedTypeInfo(void) const;
    const CClassTypeInfo* GetClassTypeInfo(void) const;
    const CChoiceTypeInfo* GetChoiceTypeInfo(void) const;
    const CContainerTypeInfo* GetContainerTypeInfo(void) const;
    const CPointerTypeInfo* GetPointerTypeInfo(void) const;

    TMemberIndex FindMemberIndex(const string& name) const;
    TMemberIndex FindMemberIndex(int tag) const;
    TMemberIndex FindVariantIndex(const string& name) const;
    TMemberIndex FindVariantIndex(int tag) const;

    CMemberIterator GetMemberIterator(TMemberIndex index) const;
    CVariantIterator GetVariantIterator(TMemberIndex index) const;

protected:
    void ResetTypeInfo(void);
    void SetTypeInfo(TTypeInfo typeinfo);

    void CheckTypeFamily(ETypeFamily family) const;
    void WrongTypeFamily(ETypeFamily needFamily) const;

private:
    TTypeInfo m_TypeInfo;

private:
    CTypeInfo* GetNCTypeInfo(void) const;
};


/// Facilitate read access to a particular instance of an object
/// of the specified type.
class NCBI_XSERIAL_EXPORT CConstObjectInfo : public CObjectTypeInfo
{
public:
    typedef TConstObjectPtr TObjectPtrType;
    typedef CConstObjectInfoEI CElementIterator;
    typedef CConstObjectInfoMI CMemberIterator;
    typedef CConstObjectInfoCV CChoiceVariant;
    
    enum ENonCObject {
        eNonCObject
    };

    /// Create empty CObjectInfo
    CConstObjectInfo(void);
    /// Initialize CObjectInfo
    CConstObjectInfo(TConstObjectPtr objectPtr, TTypeInfo typeInfo);
    CConstObjectInfo(pair<TConstObjectPtr, TTypeInfo> object);
    CConstObjectInfo(pair<TObjectPtr, TTypeInfo> object);
    /// Initialize CObjectInfo when we are sure that object 
    /// is not inherited from CObject (for efficiency)
    CConstObjectInfo(TConstObjectPtr objectPtr, TTypeInfo typeInfo,
                     ENonCObject nonCObject);

    /// Reset CObjectInfo to empty state
    void Reset(void);
    /// Set CObjectInfo
    CConstObjectInfo& operator=(pair<TConstObjectPtr, TTypeInfo> object);
    CConstObjectInfo& operator=(pair<TObjectPtr, TTypeInfo> object);

    /// Check if CObjectInfo initialized with valid object
    bool Valid(void) const
        {
            return m_ObjectPtr != 0;
        }
    DECLARE_OPERATOR_BOOL_PTR(m_ObjectPtr);

    bool operator==(const CConstObjectInfo& obj) const
    {
        return m_ObjectPtr == obj.m_ObjectPtr;
    }
    bool operator!=(const CConstObjectInfo& obj) const
    {
        return m_ObjectPtr != obj.m_ObjectPtr;
    }

    void ResetObjectPtr(void);

    /// Get pointer to object
    TConstObjectPtr GetObjectPtr(void) const;
    pair<TConstObjectPtr, TTypeInfo> GetPair(void) const;

    // primitive type interface
    bool GetPrimitiveValueBool(void) const;
    char GetPrimitiveValueChar(void) const;

    Int4 GetPrimitiveValueInt4(void) const;
    Uint4 GetPrimitiveValueUint4(void) const;
    Int8 GetPrimitiveValueInt8(void) const;
    Uint8 GetPrimitiveValueUint8(void) const;
    int GetPrimitiveValueInt(void) const;
    unsigned GetPrimitiveValueUInt(void) const;
    long GetPrimitiveValueLong(void) const;
    unsigned long GetPrimitiveValueULong(void) const;

    double GetPrimitiveValueDouble(void) const;

    void GetPrimitiveValueString(string& value) const;
    string GetPrimitiveValueString(void) const;

    void GetPrimitiveValueOctetString(vector<char>& value) const;

    void GetPrimitiveValueBitString(CBitString& value) const;

    void GetPrimitiveValueAnyContent(CAnyContentObject& value) const;

    // class interface
    CMemberIterator GetMember(CObjectTypeInfo::CMemberIterator m) const;
    CMemberIterator BeginMembers(void) const;
    CMemberIterator GetClassMemberIterator(TMemberIndex index) const;
    CMemberIterator FindClassMember(const string& memberName) const;
    CMemberIterator FindClassMemberByTag(int memberTag) const;

    // choice interface
    TMemberIndex GetCurrentChoiceVariantIndex(void) const;
    CChoiceVariant GetCurrentChoiceVariant(void) const;

    // pointer interface
    CConstObjectInfo GetPointedObject(void) const;
    
    // container interface
    CElementIterator BeginElements(void) const;

protected:
    void Set(TConstObjectPtr objectPtr, TTypeInfo typeInfo);
    
private:
    TConstObjectPtr m_ObjectPtr; // object pointer
    CConstRef<CObject> m_Ref; // hold reference to CObject for correct removal
};

/// Facilitate read/write access to a particular instance of an object
/// of the specified type.
class NCBI_XSERIAL_EXPORT CObjectInfo : public CConstObjectInfo
{
    typedef CConstObjectInfo CParent;
public:
    typedef TObjectPtr TObjectPtrType;
    typedef CObjectInfoEI CElementIterator;
    typedef CObjectInfoMI CMemberIterator;
    typedef CObjectInfoCV CChoiceVariant;

    /// Create empty CObjectInfo
    CObjectInfo(void);
    /// Initialize CObjectInfo
    CObjectInfo(TObjectPtr objectPtr, TTypeInfo typeInfo);
    CObjectInfo(pair<TObjectPtr, TTypeInfo> object);
    /// Initialize CObjectInfo when we are sure that object 
    /// is not inherited from CObject (for efficiency)
    CObjectInfo(TObjectPtr objectPtr, TTypeInfo typeInfo,
                ENonCObject nonCObject);
    /// Create CObjectInfo with new object
    explicit CObjectInfo(TTypeInfo typeInfo);
    explicit CObjectInfo(const CObjectTypeInfo& type);

    /// Set CObjectInfo to point to another object
    CObjectInfo& operator=(pair<TObjectPtr, TTypeInfo> object);
    
    /// Get pointer to object
    TObjectPtr GetObjectPtr(void) const;
    pair<TObjectPtr, TTypeInfo> GetPair(void) const;

    // primitive type interface
    void SetPrimitiveValueBool(bool value);
    void SetPrimitiveValueChar(char value);

    void SetPrimitiveValueInt4(Int4 value);
    void SetPrimitiveValueUint4(Uint4 value);
    void SetPrimitiveValueInt8(Int8 value);
    void SetPrimitiveValueUint8(Uint8 value);
    void SetPrimitiveValueInt(int value);
    void SetPrimitiveValueUInt(unsigned value);
    void SetPrimitiveValueLong(long value);
    void SetPrimitiveValueULong(unsigned long value);

    void SetPrimitiveValueDouble(double value);

    void SetPrimitiveValueString(const string& value);

    void SetPrimitiveValueOctetString(const vector<char>& value);

    void SetPrimitiveValueBitString(const CBitString& value);

    void SetPrimitiveValueAnyContent(const CAnyContentObject& value);

    // class interface
    CMemberIterator GetMember(CObjectTypeInfo::CMemberIterator m) const;
    CMemberIterator BeginMembers(void) const;
    CMemberIterator GetClassMemberIterator(TMemberIndex index) const;
    CMemberIterator FindClassMember(const string& memberName) const;
    CMemberIterator FindClassMemberByTag(int memberTag) const;
    // create if necessary and return member object
    CObjectInfo SetClassMember(TMemberIndex index) const;

    // choice interface
    CChoiceVariant GetCurrentChoiceVariant(void) const;
    // select if necessary and return variant object
    CObjectInfo SetChoiceVariant(TMemberIndex index) const;

    // pointer interface
    CObjectInfo GetPointedObject(void) const;
    // create if necessary and return pointed object
    CObjectInfo SetPointedObject(void) const;

    // container interface
    CElementIterator BeginElements(void) const;
    void ReadContainer(CObjectIStream& in, CReadContainerElementHook& hook);
    // add and return new element object
    CObjectInfo AddNewElement(void) const;
    // add new pointer element, create new pointed object and return it
    CObjectInfo AddNewPointedElement(void) const;
};

// get starting point of object hierarchy
template<class C>
inline
TTypeInfo ObjectType(const C& /*obj*/)
{
    return C::GetTypeInfo();
}

template<class C>
inline
pair<TObjectPtr, TTypeInfo> ObjectInfo(C& obj)
{
    return pair<TObjectPtr, TTypeInfo>(&obj, C::GetTypeInfo());
}

// get starting point of non-modifiable object hierarchy
template<class C>
inline
pair<TConstObjectPtr, TTypeInfo> ConstObjectInfo(const C& obj)
{
    return pair<TConstObjectPtr, TTypeInfo>(&obj, C::GetTypeInfo());
}

template<class C>
inline
pair<TConstObjectPtr, TTypeInfo> ObjectInfo(const C& obj)
{
    return pair<TConstObjectPtr, TTypeInfo>(&obj, C::GetTypeInfo());
}

template<class C>
inline
pair<TObjectPtr, TTypeInfo> RefChoiceInfo(CRef<C>& obj)
{
    return pair<TObjectPtr, TTypeInfo>(&obj, C::GetRefChoiceTypeInfo());
}

template<class C>
inline
pair<TConstObjectPtr, TTypeInfo> ConstRefChoiceInfo(const CRef<C>& obj)
{
    return pair<TConstObjectPtr, TTypeInfo>(&obj, C::GetRefChoiceTypeInfo());
}


/* @} */


#include <serial/impl/objectinfo.inl>

END_NCBI_SCOPE

#endif  /* OBJECTINFO__HPP */
