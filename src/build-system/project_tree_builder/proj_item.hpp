#ifndef PROJECT_TREE_BUILDER__PROJ_ITEM__HPP
#define PROJECT_TREE_BUILDER__PROJ_ITEM__HPP

/* $Id: proj_item.hpp 152243 2009-02-12 18:13:00Z gouriano $
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
 * Author:  Viatcheslav Gorelenkov
 *
 */


#include "proj_datatool_generated_src.hpp"
#include "file_contents.hpp"

#include <corelib/ncbienv.hpp>
#include <set>


BEGIN_NCBI_SCOPE


/////////////////////////////////////////////////////////////////////////////
///
/// CProjKey --
///
/// Project key  abstraction.
///
/// Project key (type + project_id).

class CProjKey
{
public:
    typedef enum {
        eNoProj,
        eLib,
        eApp,
        eDll,
        eMsvc,
        eLast 
    } TProjType;

    CProjKey(void);
    CProjKey(TProjType type, const string& project_id);
    CProjKey(const CProjKey& key);
    CProjKey& operator= (const CProjKey& key);
    ~CProjKey(void);

    bool operator<  (const CProjKey& key) const;
    bool operator== (const CProjKey& key) const;
    bool operator!= (const CProjKey& key) const;

    TProjType     Type(void) const;
    const string& Id  (void) const;

private:
    TProjType m_Type;
    string    m_Id;

};

/////////////////////////////////////////////////////////////////////////////
///
/// CProjItem --
///
/// Project abstraction.
///
/// Representation of one project from the build tree.

class CProjItem
{
public:
    typedef CProjKey::TProjType TProjType;


    CProjItem(void);
    CProjItem(const CProjItem& item);
    CProjItem& operator= (const CProjItem& item);

    CProjItem(TProjType type,
              const string&         name,
              const string&         id,
              const string&         sources_base,
              const list<string>&   sources, 
              const list<CProjKey>& depends,
              const list<string>&   requires,
              const list<string>&   libs_3_party,
              const list<string>&   include_dirs,
              const list<string>&   defines,
              EMakeFileType maketype,
              const string& guid);
    
    ~CProjItem(void);

    string GetPath(void) const;

    /// Name of atomic project.
    string       m_Name;

    /// ID of atomic project.
    string       m_ID;

    /// Type of the project.
    TProjType    m_ProjType;

    /// Base directory of source files (....c++/src/a/ )
    string       m_SourcesBaseDir;

    /// List of source files without extension ( *.cpp or *.c ) -
    /// with relative pathes from m_SourcesBaseDir.
    list<string> m_Sources;
    
    /// What projects this project is depend upon (IDs).
    list<CProjKey> m_Depends;
    set< CProjKey> m_UnconditionalDepends;

    /// What this project requires to have (in user site).
    list<string> m_Requires;

    /// Resolved contents of LIBS flag (Third-party libs)
    list<string> m_Libs3Party;

    /// Resolved contents of CPPFLAG ( -I$(include)<m_IncludeDir> -I$(srcdir)/..)
    /// Absolute pathes
    list<string>  m_IncludeDirs;

    /// Source files *.asn , *.dtd to be processed by datatool app
    list<CDataToolGeneratedSrc> m_DatatoolSources;

    /// Defines like USE_MS_DBLIB
    list<string>  m_Defines;

    /// Libraries from NCBI C Toolkit to link with
    list<string>  m_NcbiCLibs;
    
    /// Type of the project
    EMakeFileType m_MakeType;
    
    /// project GUID
    mutable string m_GUID;

    string  m_DllHost;
    list<string> m_HostedLibs;
    
    string m_ExportHeadersDest;
    list<string> m_ExportHeaders;

    list<string> m_CheckInfo;
    set<string> m_CheckConfigs;

    list<string> m_Includes;
    list<string> m_Inlines;
    
    mutable bool m_IsBundle;
private:
    void Clear(void);
    void SetFrom(const CProjItem& item);
};


END_NCBI_SCOPE

#endif //PROJECT_TREE_BUILDER__PROJ_ITEM__HPP
