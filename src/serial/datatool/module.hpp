#ifndef MODULE_HPP
#define MODULE_HPP

/*  $Id: module.hpp 122761 2008-03-25 16:45:09Z gouriano $
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
*   Type definitions module: equivalent of ASN.1 module
*
*/

#include <corelib/ncbistd.hpp>
#include <corelib/ncbistre.hpp>
#include <corelib/ncbiutil.hpp>
#include <list>
#include <map>
#include <set>
#include "mcontainer.hpp"
#include "comments.hpp"

BEGIN_NCBI_SCOPE

class CDataType;

class CDataTypeModule : public CModuleContainer {
public:
    CDataTypeModule(const string& name);
    virtual ~CDataTypeModule();

    void SetSourceLine(int line);
    int GetSourceLine(void) const
    {
        return m_SourceLine;
    }

    class Import {
    public:
        string moduleName;
        list<string> types;
    };
    typedef list< AutoPtr<Import> > TImports;
    typedef list< string > TExports;
    typedef list< pair< string, AutoPtr<CDataType> > > TDefinitions;

    bool Errors(void) const
        {
            return m_Errors;
        }

    const string GetVar(const string& section, const string& value) const;
    string GetFileNamePrefix(void) const;
    
    void AddDefinition(const string& name, const AutoPtr<CDataType>& type);
    void AddExports(const TExports& exports);
    void AddImports(const TImports& imports);
    void AddImports(const string& module, const list<string>& types);

    virtual void PrintASN(CNcbiOstream& out) const;
    virtual void PrintSpecDump(CNcbiOstream& out) const;
    virtual void PrintXMLSchema(CNcbiOstream& out) const;
    virtual void PrintDTD(CNcbiOstream& out) const;

    void PrintDTDModular(CNcbiOstream& out) const;
    void PrintXMLSchemaModular(CNcbiOstream& out) const;
    string GetDTDPublicName(void) const;
    string GetDTDFileNameBase(void) const;

    bool Check();
    bool CheckNames();

    const string& GetName(void) const
        {
            return m_Name;
        }
    const TDefinitions& GetDefinitions(void) const
        {
            return m_Definitions;
        }

    // return type visible from inside, or throw CTypeNotFound if none
    CDataType* Resolve(const string& name) const;
    // return type visible from outside, or throw CTypeNotFound if none
    CDataType* ExternalResolve(const string& name,
                               bool allowInternal = false) const;

    CComments& Comments(void)
        {
            return m_Comments;
        }
    CComments& LastComments(void)
        {
            return m_LastComments;
        }
    const TImports& GetImports(void) const
        {
            return m_Imports;
        }
    bool AddImportRef(const string& imp);
    
    static void SetModuleFileSuffix(const string& suffix)
    {
        s_ModuleFileSuffix = suffix;
    }
    static string GetModuleFileSuffix(void)
    {
        return s_ModuleFileSuffix;
    }
    static string ToAsnName(const string& name);
    static string ToAsnId(const string& name);

private:
    int m_SourceLine;
    bool m_Errors;
    string m_Name;
    CComments m_Comments;
    CComments m_LastComments;
    mutable string m_PrefixFromName;

    TExports m_Exports;
    TImports m_Imports;
    TDefinitions m_Definitions;

    typedef map<string, CDataType*> TTypesByName;
    typedef map<string, string> TImportsByName;

    TTypesByName m_LocalTypes;
    TTypesByName m_ExportedTypes;
    TImportsByName m_ImportedTypes;
    set<string> m_ImportRef;
    static string s_ModuleFileSuffix;
};

END_NCBI_SCOPE

#endif
