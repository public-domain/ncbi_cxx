/*  $Id: python_ncbi_dbapi_test.hpp 139088 2008-09-02 16:18:36Z ssikorsk $
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
 * Author: Sergey Sikorskiy
 *
 * File Description: Python DBAPI unit-test
 *
 * ===========================================================================
 */

#ifndef PYTHON_NCBI_DBAPI_TEST_H
#define PYTHON_NCBI_DBAPI_TEST_H

#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbienv.hpp>

#include <boost/test/unit_test.hpp>

#include "../pythonpp/pythonpp_emb.hpp"

using boost::unit_test_framework::test_suite;

BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////
class CTestArguments
{
public:
    CTestArguments(int argc, char * argv[]);

public:
    typedef map<string, string> TDatabaseParameters;

    enum EServerType {
        eUnknown,   //< Server type is not known
        eSybase,    //< Sybase server
        eMsSql,     //< Microsoft SQL server
        eMySql,     //< MySql server
        eSqlite,    //< Sqlite server
        eOracle     //< ORACLE server
    };

    string GetDriverName(void) const
    {
        return m_DriverName;
    }

    string GetServerName(void) const
    {
        return m_ServerName;
    }

    string GetUserName(void) const
    {
        return m_UserName;
    }

    string GetUserPassword(void) const
    {
        return m_UserPassword;
    }

    const TDatabaseParameters& GetDBParameters(void) const
    {
        return m_DatabaseParameters;
    }

    string GetDatabaseName(void) const
    {
        return m_DatabaseName;
    }

    string GetServerTypeStr(void) const;
    EServerType GetServerType(void) const;

private:
    void SetDatabaseParameters(void);

private:

    string m_DriverName;
    string m_ServerName;
    string m_UserName;
    string m_UserPassword;
    string m_DatabaseName;
    TDatabaseParameters m_DatabaseParameters;
};


class CPythonDBAPITest
{
public:
    CPythonDBAPITest(const CTestArguments& args);
    ~CPythonDBAPITest(void);

public:
    // Test IStatement interface.

    // Test particular methods.
    void MakeTestPreparation(void);
    void TestBasic(void);
    void TestConnection(void);
    void TestExecute(void);
    void TestFetch(void);
    void TestParameters(void);
    void TestExecuteMany(void);
    void Test_callproc(void);
    void TestExecuteStoredProc(void);
    void Test_SelectStmt(void);
    void Test_LOB(void);
    void Test_RaiseError(void);
    void Test_Exception(void);

    // Test scenarios.
    void TestTransaction(void);
    void TestScenario_1(void);
    void TestScenario_2(void);

    // Run a Python script from a file
    void TestFromFile(void);

private:
    static void ExecuteStr(const char* cmd);
    static void ExecuteSQL(const string& sql);

private:
    pythonpp::CEngine* m_Engine;
    const CTestArguments m_args;
};

///////////////////////////////////////////////////////////////////////////
struct CPythonDBAPITestSuite : public test_suite
{
    CPythonDBAPITestSuite(const CTestArguments& args);
    ~CPythonDBAPITestSuite(void);
};

END_NCBI_SCOPE

#endif  // PYTHON_NCBI_DBAPI_TEST_H

