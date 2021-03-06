/*  $Id: test_strsearch.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 * Author: Anatoliy Kuznetsov
 *
 * File Description: Test application for string search
 *
 */


#include <ncbi_pch.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbiargs.hpp>
#include <corelib/ncbitime.hpp>
#include <stdio.h>

#include <util/strsearch.hpp>

#include <common/test_assert.h>  /* This header must go last */

USING_NCBI_SCOPE;



////////////////////////////////
// Test functions, classes, etc.
//



static
void s_TEST_BoyerMooreMatcher(void)
{
    LOG_POST("String search test (Boyer-Moore).");

    const char* str = "123 567 BB";
    size_t len = strlen(str);
    {{    
        CBoyerMooreMatcher matcher("BB");
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 8);
    }}
    {{    
        CBoyerMooreMatcher matcher("BB", NStr::eNocase, 
                                   CBoyerMooreMatcher::eWholeWordMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 8);
    }}
    {{
        CBoyerMooreMatcher matcher("123", NStr::eNocase, 
                                   CBoyerMooreMatcher::eWholeWordMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 0);
    }}
    {{
        CBoyerMooreMatcher matcher("1234", NStr::eNocase, 
                                   CBoyerMooreMatcher::eWholeWordMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert((int)pos == -1l);
    }}
    {{
        CBoyerMooreMatcher matcher("bb", NStr::eCase, 
                                   CBoyerMooreMatcher::eWholeWordMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert((int)pos == -1l);
    }}
    {{    
        CBoyerMooreMatcher matcher("67", NStr::eNocase, 
                                   CBoyerMooreMatcher::eWholeWordMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert((int)pos == -1l);
    }}
    {{
        CBoyerMooreMatcher matcher("67", NStr::eNocase, 
                                   CBoyerMooreMatcher::eSubstrMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 5);
    }}
    {{
        CBoyerMooreMatcher matcher("67", NStr::eNocase, 
                                   CBoyerMooreMatcher::eSuffixMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 5);
    }}
    {{
        CBoyerMooreMatcher matcher("56", NStr::eNocase, 
                                   CBoyerMooreMatcher::ePrefixMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 4);
    }}
    {{
        CBoyerMooreMatcher matcher("123", NStr::eNocase, 
                                   CBoyerMooreMatcher::ePrefixMatch);
        size_t pos = matcher.Search(str, 0, len);
        assert(pos == 0);
    }}
    {{
        CBoyerMooreMatcher matcher("drosophila", NStr::eNocase, 
                                   CBoyerMooreMatcher::ePrefixMatch);
        matcher.InitCommonDelimiters();
        const char* str1 = 
            "eukaryotic initiation factor 4E-I [Drosophila melanogaster]";

        size_t len1 = strlen(str1);
        size_t pos  = matcher.Search(str1, 0, len1);
        assert(pos != (SIZE_TYPE)-1);
    }}

    LOG_POST("String search test (Boyer-Moore) ok.");
}



////////////////////////////////
// Test application
//

class CStrSearchTest : public CNcbiApplication
{
public:
    void Init(void);
    int Run(void);
};


void CStrSearchTest::Init(void)
{
    SetDiagPostLevel(eDiag_Warning);
    SetDiagPostFlag(eDPF_File);
    SetDiagPostFlag(eDPF_Line);
    auto_ptr<CArgDescriptions> d(new CArgDescriptions);
    d->SetUsageContext("test_bdb", "test BDB library");
    SetupArgDescriptions(d.release());
}


int CStrSearchTest::Run(void)
{
    s_TEST_BoyerMooreMatcher();

    LOG_POST("\nTEST execution completed successfully!");

    return 0;
}


///////////////////////////////////
// APPLICATION OBJECT  and  MAIN
//

int main(int argc, const char* argv[])
{
    // Execute main application function
    return CStrSearchTest().AppMain(argc, argv, 0, eDS_Default, 0);
}
