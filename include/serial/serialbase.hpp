#ifndef SERIALBASE__HPP
#define SERIALBASE__HPP

/*  $Id: serialbase.hpp 129054 2008-05-29 15:24:36Z lavr $
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
*   File to be included in all headers generated by datatool
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbiobj.hpp>
#include <serial/exception.hpp>
#include <serial/serialdef.hpp>
#include <serial/error_codes.hpp>
#include <typeinfo>

#define BITSTRING_AS_VECTOR    0

#if !BITSTRING_AS_VECTOR
#include <util/bitset/ncbi_bitset.hpp>
#include <util/bitset/bmserial.h>
#endif


/** @addtogroup GenClassSupport
 *
 * @{
 */


BEGIN_NCBI_SCOPE

// CBitString
#if BITSTRING_AS_VECTOR
typedef std::vector< bool > CBitString;
#else
typedef bm::bvector< > CBitString;
#endif

class CTypeInfo;
class CClassTypeInfo;
class CChoiceTypeInfo;
class CEnumeratedTypeValues;

// enum for choice classes generated by datatool
enum EResetVariant {
    eDoResetVariant,
    eDoNotResetVariant
};

typedef void (*TPreReadFunction)(const CTypeInfo* info, void* object);
typedef void (*TPostReadFunction)(const CTypeInfo* info, void* object);
typedef void (*TPreWriteFunction)(const CTypeInfo* info, const void* object);
typedef void (*TPostWriteFunction)(const CTypeInfo* info, const void* object);

NCBI_XSERIAL_EXPORT
void SetPreRead(CClassTypeInfo*  info, TPreReadFunction function);

NCBI_XSERIAL_EXPORT
void SetPostRead(CClassTypeInfo*  info, TPostReadFunction function);

NCBI_XSERIAL_EXPORT
void SetPreRead(CChoiceTypeInfo* info, TPreReadFunction function);

NCBI_XSERIAL_EXPORT
void SetPostRead(CChoiceTypeInfo* info, TPostReadFunction function);

NCBI_XSERIAL_EXPORT
void SetPreWrite(CClassTypeInfo*  info, TPreWriteFunction function);

NCBI_XSERIAL_EXPORT
void SetPostWrite(CClassTypeInfo*  info, TPostWriteFunction function);

NCBI_XSERIAL_EXPORT
void SetPreWrite(CChoiceTypeInfo* info, TPreWriteFunction function);

NCBI_XSERIAL_EXPORT
void SetPostWrite(CChoiceTypeInfo* info, TPostWriteFunction function);

template<class Class>
class CClassPrePostReadWrite
{
public:
    static void PreRead(const CTypeInfo* /*info*/, void* object)
        {
            static_cast<Class*>(object)->PreRead();
        }
    static void PostRead(const CTypeInfo* /*info*/, void* object)
        {
            static_cast<Class*>(object)->PostRead();
        }
    static void PreWrite(const CTypeInfo* /*info*/, const void* object)
        {
            static_cast<const Class*>(object)->PreWrite();
        }
    static void PostWrite(const CTypeInfo* /*info*/, const void* object)
        {
            static_cast<const Class*>(object)->PostWrite();
        }
};

/// Base class for all serializable objects
class NCBI_XSERIAL_EXPORT CSerialObject : public CObject
{
public:
    CSerialObject(void);
    virtual ~CSerialObject(void);
    virtual const CTypeInfo* GetThisTypeInfo(void) const = 0;
    /// Set object to copy of another one
    virtual void Assign(const CSerialObject& source,
                        ESerialRecursionMode how = eRecursive);
    /// Check if both objects contain the same values
    virtual bool Equals(const CSerialObject& object,
                        ESerialRecursionMode how = eRecursive) const;
    virtual void DebugDump(CDebugDumpContext ddc, unsigned int depth) const;

    void ThrowUnassigned(TMemberIndex index) const;
    // for all GetX() methods called in the current thread
    static  void SetVerifyDataThread(ESerialVerifyData verify);
    // for all GetX() methods called in the current process
    static  void SetVerifyDataGlobal(ESerialVerifyData verify);

    static const char* ms_UnassignedStr;
    static const char  ms_UnassignedByte;

    /// Check if object data type has namespace name
    bool HasNamespaceName(void) const;
    /// Get namespace name
    const string& GetNamespaceName(void) const;
    /// Set namespace name
    void SetNamespaceName(const string& ns_name);

    /// Check if data type has namespace prefix
    bool HasNamespacePrefix(void) const;
    /// Get namespace prefix
    const string& GetNamespacePrefix(void) const;
    /// Set namespace prefix
    void SetNamespacePrefix(const string& ns_prefix);

private:
    static ESerialVerifyData x_GetVerifyData(void);
    static ESerialVerifyData ms_VerifyDataDefault;
};

/// XML attribute information item
class NCBI_XSERIAL_EXPORT CSerialAttribInfoItem
{
public:
    CSerialAttribInfoItem(const string& name,
                          const string& ns_name, const string& value);
    CSerialAttribInfoItem(const CSerialAttribInfoItem& other);
    virtual ~CSerialAttribInfoItem(void);

    /// Get local name of the information item
    const string& GetName(void) const;
    /// Get namespace name of the information item
    const string& GetNamespaceName(void) const;
    /// Get normalized value of the information item
    const string& GetValue(void) const;
private:
    string m_Name;
    string m_NsName;
    string m_Value;
};

/// Serializable object that stores any combination of parsable data
///
/// In DTD - elements with category ANY
/// In XML schema - element of an unspecified type ('any')
class NCBI_XSERIAL_EXPORT CAnyContentObject : public CSerialObject
{
public:
    CAnyContentObject(void);
    CAnyContentObject(const CAnyContentObject& other);
    virtual ~CAnyContentObject(void);

    virtual const CTypeInfo* GetThisTypeInfo(void) const
    { return GetTypeInfo(); }
    static const CTypeInfo* GetTypeInfo(void);

    void Reset(void);
    CAnyContentObject& operator= (const CAnyContentObject& other);
    bool operator== (const CAnyContentObject& other) const;

    /// Set local name
    void SetName(const string& name);
    /// Get local name
    const string& GetName(void) const;
    /// Set normalized value
    void SetValue(const string& value);
    /// Get normalized value
    const string& GetValue(void) const;

    /// Set namespace name
    void SetNamespaceName(const string& ns_name);
    /// Get namespace name
    const string& GetNamespaceName(void) const;
    /// Set namespace prefix
    void SetNamespacePrefix(const string& ns_prefix);
    /// Get namespace prefix
    const string& GetNamespacePrefix(void) const;

    /// Add attribute
    void AddAttribute(const string& name,
                      const string& ns_name, const string& value);
    /// Get object attributes
    const vector<CSerialAttribInfoItem>& GetAttributes(void) const;

private:
    void x_Copy(const CAnyContentObject& other);
    void x_Decode(const string& value);
    string m_Name;
    string m_Value;
    string m_NsName;
    string m_NsPrefix;
    vector<CSerialAttribInfoItem> m_Attlist;
};

/// Base class for user-defined serializable classes
/// to allow for objects assignment and comparison.
///
/// EXAMPLE:
///   class CSeq_entry : public CSeq_entry_Base, CSerialUserOp
class NCBI_XSERIAL_EXPORT CSerialUserOp
{
    friend class CClassTypeInfo;
    friend class CChoiceTypeInfo;
public:
    virtual ~CSerialUserOp() { }
protected:
    /// Will be called after copying the datatool-generated members
    virtual void UserOp_Assign(const CSerialUserOp& source) = 0;
    /// Will be called after comparing the datatool-generated members
    virtual bool UserOp_Equals(const CSerialUserOp& object) const = 0;
};


/////////////////////////////////////////////////////////////////////
///
/// Alias wrapper templates
///

template <class TPrim>
class NCBI_XSERIAL_EXPORT CAliasBase
{
public:
    typedef CAliasBase<TPrim> TThis;

    CAliasBase(void) {}
    explicit CAliasBase(const TPrim& value)
        : m_Data(value) {}

    const TPrim& Get(void) const
        {
            return m_Data;
        }
    TPrim& Set(void)
        {
            return m_Data;
        }
    void Set(const TPrim& value)
        {
            m_Data = value;
        }
    operator TPrim(void) const
        {
            return m_Data;
        }

    TThis& operator*(void)
        {
            return *this;
        }
    TThis* operator->(void)
        {
            return this;
        }

    bool operator<(const TPrim& value) const
        {
            return m_Data < value;
        }
    bool operator>(const TPrim& value) const
        {
            return m_Data > value;
        }
    bool operator==(const TPrim& value) const
        {
            return m_Data == value;
        }
    bool operator!=(const TPrim& value) const
        {
            return m_Data != value;
        }

    static TConstObjectPtr GetDataPtr(const TThis* alias)
        {
            return &alias->m_Data;
        }

protected:
    TPrim m_Data;
};


template <class TStd>
class NCBI_XSERIAL_EXPORT CStdAliasBase : public CAliasBase<TStd>
{
    typedef CAliasBase<TStd> TParent;
    typedef CStdAliasBase<TStd> TThis;
public:
    CStdAliasBase(void)
        : TParent((TStd)0) {}
    explicit CStdAliasBase(const TStd& value)
        : TParent(value) {}
};


template <class TString>
class NCBI_XSERIAL_EXPORT CStringAliasBase : public CAliasBase<TString>
{
    typedef CAliasBase<TString> TParent;
    typedef CStringAliasBase<TString> TThis;
public:
    CStringAliasBase(void)
        : TParent() {}
    explicit CStringAliasBase(const TString& value)
        : TParent(value) {}
};


template<typename T>
class CUnionBuffer
{   // char buffer support, used in choices
public:
    typedef T    TObject;                  // object type
    typedef char TBuffer[sizeof(TObject)]; // char buffer type

    // cast to object type
    TObject& operator*(void)
        {
            return *reinterpret_cast<TObject*>(m_Buffer);
        }
    const TObject& operator*(void) const
        {
            return *reinterpret_cast<const TObject*>(m_Buffer);
        }

    // construct/destruct object
    void Construct(void)
        {
            ::new(static_cast<void*>(m_Buffer)) TObject();
        }
    void Destruct(void)
        {
            (**this).~TObject();
        }
    
private:
    TBuffer m_Buffer;
};


/////////////////////////////////////////////////////////////////////
//
//  Assignment and comparison for serializable objects
//

/// Set object to copy of another one
template <class C>
C& SerialAssign(C& dest, const C& src, ESerialRecursionMode how = eRecursive)
{
    if ( typeid(src) != typeid(dest) ) {
        string msg("Assignment of incompatible types: ");
        msg += typeid(dest).name();
        msg += " = ";
        msg += typeid(src).name();
        NCBI_THROW(CSerialException,eIllegalCall, msg);
    }
    C::GetTypeInfo()->Assign(&dest, &src, how);
    return dest;
}

/// Compare serial objects
template <class C>
bool SerialEquals(const C& object1, const C& object2,
                  ESerialRecursionMode how = eRecursive)
{
    if ( typeid(object1) != typeid(object2) ) {
        string msg("Cannot compare types: ");
        msg += typeid(object1).name();
        msg += " == ";
        msg += typeid(object2).name();
        NCBI_THROW(CSerialException,eIllegalCall, msg);
    }
    return C::GetTypeInfo()->Equals(&object1, &object2, how);
}

/// Create on heap a clone of the source object
template <typename C>
C* SerialClone(const C& src)
{
    typename C::TTypeInfo type = C::GetTypeInfo();
    TObjectPtr obj = type->Create();
    type->Assign(obj, &src);
    return static_cast<C*>(obj);
}

/////////////////////////////////////////////////////////////////////////////
//
//  I/O stream manipulators and helpers for serializable objects
//

// Helper base class
class NCBI_XSERIAL_EXPORT MSerial_Flags
{
protected:
    MSerial_Flags(unsigned long all, unsigned long flags);
private:
    MSerial_Flags(void) {}
    MSerial_Flags(const MSerial_Flags&) {}
    MSerial_Flags& operator= (const MSerial_Flags&) {return *this;}

    void SetFlags(CNcbiIos& io) const;
    unsigned long m_All;
    unsigned long m_Flags;

    friend CNcbiOstream& operator<< (CNcbiOstream& io, const MSerial_Flags& obj);
    friend CNcbiIstream& operator>> (CNcbiIstream& io, const MSerial_Flags& obj);
};

inline
CNcbiOstream& operator<< (CNcbiOstream& io, const MSerial_Flags& obj)
{
    obj.SetFlags(io);
    return io;
}
inline
CNcbiIstream& operator>> (CNcbiIstream& io, const MSerial_Flags& obj)
{
    obj.SetFlags(io);
    return io;
}

// Formatting
class NCBI_XSERIAL_EXPORT MSerial_Format : public MSerial_Flags
{
public:
    explicit MSerial_Format(ESerialDataFormat fmt);
};

// Class member assignment verification
class NCBI_XSERIAL_EXPORT MSerial_VerifyData : public MSerial_Flags
{
public:
    explicit MSerial_VerifyData(ESerialVerifyData fmt);
};

// Default string encoding in XML streams
class NCBI_XSERIAL_EXPORT MSerialXml_DefaultStringEncoding : public MSerial_Flags
{
public:
    explicit MSerialXml_DefaultStringEncoding(EEncoding fmt);
};

// Formatting
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_AsnText(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_AsnBinary(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_Xml(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_Json(CNcbiIos& io);

// Class member assignment verification
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_VerifyDefault(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_VerifyNo(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_VerifyYes(CNcbiIos& io);
NCBI_XSERIAL_EXPORT CNcbiIos& MSerial_VerifyDefValue(CNcbiIos& io);

class CConstObjectInfo;
class CObjectInfo;
// Input/output
NCBI_XSERIAL_EXPORT CNcbiOstream& operator<< (CNcbiOstream& str, const CSerialObject& obj);
NCBI_XSERIAL_EXPORT CNcbiIstream& operator>> (CNcbiIstream& str, CSerialObject& obj);
NCBI_XSERIAL_EXPORT CNcbiOstream& operator<< (CNcbiOstream& str, const CConstObjectInfo& obj);
NCBI_XSERIAL_EXPORT CNcbiIstream& operator>> (CNcbiIstream& str, const CObjectInfo& obj);


END_NCBI_SCOPE

// these methods must be defined in root namespace so they have prefix NCBISER

// default functions do nothing
template<class CInfo>
inline
void NCBISERSetPreRead(const void* /*object*/, CInfo* /*info*/)
{
}

template<class CInfo>
inline
void NCBISERSetPostRead(const void* /*object*/, CInfo* /*info*/)
{
}

template<class CInfo>
inline
void NCBISERSetPreWrite(const void* /*object*/, CInfo* /*info*/)
{
}

template<class CInfo>
inline
void NCBISERSetPostWrite(const void* /*object*/, CInfo* /*info*/)
{
}

// define for declaring specific function
#define NCBISER_HAVE_PRE_READ(Class) \
template<class CInfo> \
inline \
void NCBISERSetPreRead(const Class* /*object*/, CInfo* info) \
{ \
    NCBI_NS_NCBI::SetPreRead \
        (info, &NCBI_NS_NCBI::CClassPrePostReadWrite<Class>::PreRead);\
}

#define NCBISER_HAVE_POST_READ(Class) \
template<class CInfo> \
inline \
void NCBISERSetPostRead(const Class* /*object*/, CInfo* info) \
{ \
    NCBI_NS_NCBI::SetPostRead \
        (info, &NCBI_NS_NCBI::CClassPrePostReadWrite<Class>::PostRead);\
}

#define NCBISER_HAVE_PRE_WRITE(Class) \
template<class CInfo> \
inline \
void NCBISERSetPreWrite(const Class* /*object*/, CInfo* info) \
{ \
    NCBI_NS_NCBI::SetPreWrite \
        (info, &NCBI_NS_NCBI::CClassPrePostReadWrite<Class>::PreWrite);\
}

#define NCBISER_HAVE_POST_WRITE(Class) \
template<class CInfo> \
inline \
void NCBISERSetPostWrite(const Class* /*object*/, CInfo* info) \
{ \
    NCBI_NS_NCBI::SetPostWrite \
        (info, &NCBI_NS_NCBI::CClassPrePostReadWrite<Class>::PostWrite);\
}

#define DECLARE_INTERNAL_TYPE_INFO() \
    typedef const NCBI_NS_NCBI::CTypeInfo* TTypeInfo; \
    virtual TTypeInfo GetThisTypeInfo(void) const { return GetTypeInfo(); } \
    static  TTypeInfo GetTypeInfo(void)

#define ENUM_METHOD_NAME(EnumName) \
    NCBI_NAME2(GetTypeInfo_enum_,EnumName)
#define DECLARE_ENUM_INFO(EnumName) \
    const NCBI_NS_NCBI::CEnumeratedTypeValues* ENUM_METHOD_NAME(EnumName)(void)
#define DECLARE_INTERNAL_ENUM_INFO(EnumName) \
    static DECLARE_ENUM_INFO(EnumName)

#define DECLARE_STD_ALIAS_TYPE_INFO() \
    static const NCBI_NS_NCBI::CTypeInfo* GetTypeInfo(void)

#if HAVE_NCBI_C

#define ASN_STRUCT_NAME(AsnStructName) NCBI_NAME2(struct_, AsnStructName)
#define ASN_STRUCT_METHOD_NAME(AsnStructName) \
    NCBI_NAME2(GetTypeInfo_struct_,AsnStructName)

#define DECLARE_ASN_TYPE_INFO(AsnStructName) \
    const NCBI_NS_NCBI::CTypeInfo* ASN_STRUCT_METHOD_NAME(AsnStructName)(void)
#define DECLARE_ASN_STRUCT_INFO(AsnStructName) \
    struct ASN_STRUCT_NAME(AsnStructName); \
    DECLARE_ASN_TYPE_INFO(AsnStructName); \
    inline \
    const NCBI_NS_NCBI::CTypeInfo* \
    GetAsnStructTypeInfo(const ASN_STRUCT_NAME(AsnStructName)* ) \
    { \
        return ASN_STRUCT_METHOD_NAME(AsnStructName)(); \
    } \
    struct ASN_STRUCT_NAME(AsnStructName)

#define DECLARE_ASN_CHOICE_INFO(AsnChoiceName) \
    DECLARE_ASN_TYPE_INFO(AsnChoiceName)

#endif

/* @} */

#endif  /* SERIALBASE__HPP */
