/* $Id: bcp.cpp 118386 2008-01-28 20:30:19Z ssikorsk $
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
 * Author:  Sergey Sikorskiy
 *
 * File Description:
 *    Driver for SQLite v3 embedded database server
 *
 */

#include <ncbi_pch.hpp>

#include <dbapi/driver/sqlite3/interfaces.hpp>

#include "sqlite3_utils.hpp"


BEGIN_NCBI_SCOPE

///////////////////////////////////////////////////////////////////////////////
CSL3_BCPInCmd::CSL3_BCPInCmd(CSL3_Connection& conn,
                             const string&    table_name) 
: CSL3_LangCmd(conn, MakeInsertStmt(conn, table_name))
{
    ExecuteSQL("BEGIN TRANSACTION");
}


CSL3_BCPInCmd::~CSL3_BCPInCmd(void)
{
}


bool CSL3_BCPInCmd::CommitBCPTrans(void)
{
    try {
        ExecuteSQL("COMMIT TRANSACTION");
    }
    catch(NCBI_NS_NCBI::CException)
    {
        return false;
    }

    return true;
}


bool CSL3_BCPInCmd::EndBCP(void)
{
    try {
        ExecuteSQL("COMMIT TRANSACTION");
    }
    catch(NCBI_NS_NCBI::CException)
    {
        return false;
    }

    return true;
}


void CSL3_BCPInCmd::ExecuteSQL(const string& sql)
{
    sqlite3_stmt* stmt = NULL;
    const char* sql_tail = NULL;

    Check(sqlite3_prepare(GetConnection().GetSQLite3(),
                          sql.c_str(),
                          sql.size(),
                          &stmt,
                          &sql_tail
                          ));

    int rc = sqlite3_step(stmt);
    switch (rc) {
    case SQLITE_DONE:
    case SQLITE_ROW:
        break;
    case SQLITE_BUSY:
    case SQLITE_ERROR:
    case SQLITE_MISUSE:
        Check(sqlite3_finalize(stmt));
        CHECK_DRIVER_ERROR(rc != SQLITE_OK,
                           "Failed to execute a statement." + GetDbgInfo(),
                           100000);
        break;
    default:
        Check(sqlite3_finalize(stmt));
        DATABASE_DRIVER_ERROR("Invalid return code." + GetDbgInfo(), 100000);
    }

    Check(sqlite3_finalize(stmt));
}

string 
CSL3_BCPInCmd::MakePlaceholders(size_t num)
{
    string result;

    for (size_t i = 0; i < num; ++i) {
        if (i > 0) {
            result += ", ?";
        } else {
            result += "?";
        }
    }

    return result;
}

size_t 
CSL3_BCPInCmd::GetTableColumnNum(
        CSL3_Connection& conn, 
        const string& table_name)
{
    size_t result = 0;
    const string sql("SELECT * FROM " + table_name);
    sqlite3_stmt* stmt = MakeSQLiteStmt(conn, sql);
    
    int rc = sqlite3_step(stmt);
    switch (rc) {
    case SQLITE_DONE:
        break;
    case SQLITE_ROW:
        break;
    default:
        DATABASE_DRIVER_ERROR("Invalid return code in SQL statement '" + sql + "'", 100000);
    }
    result = sqlite3_column_count(stmt);

    sqlite3_finalize(stmt);

    return result;
}

string CSL3_BCPInCmd::MakeInsertStmt(
        CSL3_Connection& conn, 
        const string& table_name)
{
    string sql("INSERT INTO " + table_name + " VALUES(");
    
    sql += MakePlaceholders(GetTableColumnNum(conn, table_name));
    sql += ")";

    return sql;
}


END_NCBI_SCOPE


