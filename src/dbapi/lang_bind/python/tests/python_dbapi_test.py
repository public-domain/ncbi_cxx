#! /usr/bin/env python
 
# $Id: python_dbapi_test.py 149362 2009-01-09 18:03:03Z ivanovp $
# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================
#
# Author:  Pavel Ivanov
#
#

from python_ncbi_dbapi import *


def check(cond, msg):
    if not cond:
        raise Exception(msg)

def checkEqual(var1, var2):
    check(var1 == var2, "Values not equal: '" + str(var1) + "' !='" + str(var2) + "'")


conn = connect('ftds', 'MSSQL', 'MSDEV1', 'DBAPI_Sample', 'anyone', 'allowed')
cursor = conn.cursor()

cursor.execute('select qq = 57 + 33')
result = cursor.fetchone()
checkEqual(result[0], 90)


cursor.execute('select name, type from sysobjects')
checkEqual(len(cursor.fetchone()), 2)
checkEqual(len(cursor.fetchmany(1)), 1)
checkEqual(len(cursor.fetchmany(2)), 2)
checkEqual(len(cursor.fetchmany(3)), 3)
cursor.fetchall()


cursor.execute('select name, type from sysobjects where type = @type_par', {'@type_par':'S'})
cursor.fetchall()


cursor.execute("""
        CREATE TABLE #sale_stat (
                year INT NOT NULL,
                month VARCHAR(255) NOT NULL,
                stat INT NOT NULL
        )
""")

month_list = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December']
sql = "insert into #sale_stat(year, month, stat) values (@year, @month, @stat)"

# Check that the temporary table was successfully created and we can get data from it
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 0)
# Start transaction
cursor.execute('BEGIN TRANSACTION')
# Insert records
cursor.executemany(sql, [{'@year':year, '@month':month, '@stat':stat} for stat in range(1, 3) for year in range(2004, 2006) for month in month_list])
# Check how many records we have inserted
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 48)
# "Standard interface" rollback
conn.rollback();
# Check how many records left after "standard" ROLLBACK
# "Standard interface" rollback command is not supposed to affect current transaction.
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 48)
# Rollback transaction
cursor.execute('ROLLBACK TRANSACTION')
# Start transaction
cursor.execute('BEGIN TRANSACTION')
# Check how many records left after ROLLBACK
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 0)
# Insert records again
cursor.executemany(sql, [{'@year':year, '@month':month, '@stat':stat} for stat in range(1, 3) for year in range(2004, 2006) for month in month_list])
# Check how many records we have inserted
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 48)
# Commit transaction
cursor.execute('COMMIT TRANSACTION')
# Check how many records left after COMMIT
cursor.execute("select * from #sale_stat")
checkEqual(len( cursor.fetchall() ), 48)


cursor.callproc('sp_databases')
cursor.fetchall()
# Retrieve return status
checkEqual(cursor.get_proc_return_status(), 0)
# Call a stored procedure with a parameter.
cursor.callproc('sp_server_info', {'@attribute_id':1} )
cursor.fetchall()
# Retrieve return status
checkEqual(cursor.get_proc_return_status(), 0)
# Call Stored Procedure using an "execute" method.
cursor.execute('sp_databases')
cursor.fetchall()
# Retrieve return status
checkEqual(cursor.get_proc_return_status(), 0)
# Call a stored procedure with a parameter.
cursor.execute('execute sp_server_info 1')
cursor.fetchall()
# Retrieve return status
checkEqual(cursor.get_proc_return_status(), 0)


print 'All tests completed successfully'
