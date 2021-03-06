/*  $Id: formatguess_unit_test.cpp 151444 2009-02-04 16:35:34Z grichenk $
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
* Author:  Pavel Ivanov, Aleksey Grichenko, NCBI
*
* File Description:
*   Unit test for CFormatGuess class
*
* ===========================================================================
*/

#include <ncbi_pch.hpp>

#include <corelib/ncbi_system.hpp>
#include <util/format_guess.hpp>

// This header must be included before all Boost.Test headers if there are any
#include <corelib/test_boost.hpp>


USING_NCBI_SCOPE;


NCBITEST_AUTO_INIT()
{
}

NCBITEST_AUTO_FINI()
{
}


// Real data does not matter, just need some non-printable chars.
static const char* kData_AsnBin = "\1\2\3\4\5\6\7\0";
static const char* kData_Rmo =
    " 1320 15.6  6.2  0.0  HSU08988  6563 6781 (22462) C  MER7A   "
    "DNA/MER2_type    (0)  337  104  20\n"
    "12279 10.5  2.1  1.7  HSU08988  6782 7718 (21525) C  Tigger1 "
    "DNA/MER2_type    (0) 2418 1486  19\n"
    " 1769 12.9  6.6  1.9  HSU08988  7719 8022 (21221) C  AluSx   "
    "SINE/Alu         (0)  317    1  17\n"
    "12279 10.5  2.1  1.7  HSU08988  8023 8694 (20549) C  Tigger1 "
    "DNA/MER2_type  (932) 1486  818  19\n";
static const char* kHeader_Rmo =
    "SW perc query position matching\n"
    "score div. del. ins. sequence\n";
static const char* kData_Gtf =
    "381 Twinscan  CDS          380   401   .   +   0  "
    "gene_id \"001\"; transcript_id \"001.1\";\n"
    "381 Twinscan  CDS          501   650   .   +   2  "
    "gene_id \"001\"; transcript_id \"001.1\";\n"
    "381 Twinscan  CDS          700   707   .   +   2  "
    "gene_id \"001\"; transcript_id \"001.1\";\n"
    "381 Twinscan  start_codon  380   382   .   +   0  "
    "gene_id \"001\"; transcript_id \"001.1\";\n";
static const char* kData_Glimmer3 =
    ">gms:3447|cmr:632 chromosome 1 {Mycobacterium smegmatis MC2}\n"
    "orf00001 499 1692 +1 13.14\n"
    "orf00004 1721 2614 +2 14.20\n"
    "orf00006 2624 3778 +2 10.35\n"
    "orf00009 3775 4359 +1 9.34\n";
static const char* kData_Agp =
    //"# ORGANISM: Homo sapiens\n"
    //"# TAX_ID: 9606\n"
    //"# ASSEMBLY NAME: EG1\n"
    //"# ASSEMBLY DATE: 06-September-2006\n"
    //"# GENOME CENTER: NCBI\n"
    //"# DESCRIPTION: Example AGP specifying the assembly of scaffolds from WGS contigs\n"
    "EG1_scaffold1\t1\t3043\t1\tW\tAADB02037551.1\t1\t3043\t+\n"
    "EG1_scaffold2\t1\t40448\t1\tW\tAADB02037552.1\t1\t40448\t+\n"
    "EG1_scaffold2\t40449\t40548\t2\tN\t100	fragment\tyes\t\n"
    "EG1_scaffold2\t40549\t117529\t3\tW\tAADB02037553.1\t1\t76981\t+\n"
    "EG1_scaffold2\t117530\t117629\t4\tN\t100\tfragment\tyes\t\n"
    "EG1_scaffold2\t117630\t145298\t5\tW\tAADB02037554.1\t1\t27669\t+\n"
    "EG1_scaffold2\t145299\t145398\t6\tN\t100\tfragment\tyes\t\n"
    "EG1_scaffold2\t145399\t148350\t7\tW\tAADB02037555.1\t1\t2952\t+\n"
    "EG1_scaffold2\t148351\t148450\t8\tN\t100\tfragment\tyes\t\n"
    "EG1_scaffold2\t148451\t152599\t9\tW\tAADB02037556.1\t1\t4149\t-\n"
    "EG1_scaffold2\t152600\t152699\t10\tN\t100\tfragment\tyes\t\n";
static const char* kData_Xml =
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE Seq-entry PUBLIC \"-//NCBI//NCBI Seqset/EN\" "
    "\"http://www.ncbi.nlm.nih.gov/dtd/NCBI_Seqset.dtd\">\n"
    "<Seq-entry>\n";
static const char* kData_Wiggle =
    "browser position chr19:59302001-59311000\n"
    "browser hide all\n"
    "browser pack refGene encodeRegions\n"
    "browser full altGraph\n"
    "#	300 base wide bar graph, autoScale is on by default == graphing\n"
    "#	limits will dynamically change to always show full range of data\n"
    "#	in viewing window, priority = 20 positions this as the second graph\n"
    "#	Note, zero-relative, half-open coordinate system in use for bed format\n"
    "track type=wiggle_0 name=\"Bed Format\" description=\"BED format\" \\\n"
    "    visibility=full color=200,100,0 altColor=0,100,200 priority=20\n"
    "chr19 59302000 59302300 -1.0\n"
    "chr19 59302300 59302600 -0.75\n"
    "chr19 59302600 59302900 -0.50\n"
    "chr19 59302900 59303200 -0.25\n"
    "chr19 59303200 59303500 0.0\n"
    "chr19 59303500 59303800 0.25\n"
    "chr19 59303800 59304100 0.50\n"
    "chr19 59304100 59304400 0.75\n"
    "chr19 59304400 59304700 1.00\n";
static const char* kData_Bed =
    "track name=pairedReads description=\"Clone Paired Reads\" useScore=1\n"
    "chr22 1000 5000 cloneA 960 + 1000 5000 0 2 567,488, 0,3512\n"
    "chr22 2000 6000 cloneB 900 - 2000 6000 0 2 433,399, 0,3601\n";
static const char* kData_Bed15 =
    "#chrom\tchromStart\tchromEnd\tname\tscore\tstrand\tthickStart\tthickEnd\t"
    "reserved\tblockCount\tblockSizes\tchromStarts\texpCount\texpIds\texpScores\n"
    "chr1\t159639972\t159640031\t2440848\t500\t-\t159639972\t159640031\t"
    "0\t1\t59,\t0,\t33\t0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,"
    "20,21,22,23,24,25,26,27,28,29,30,31,32,\t0.593000,1.196000,-0.190000,"
    "-1.088000,0.093000,-0.731000,0.130000,-0.008000,-1.087000,0.609000,"
    "-1.061000,-1.092000,0.807000,0.499000,-0.322000,-0.985000,0.309000,"
    "0.000000,0.812000,-0.457000,-0.560000,0.096000,0.186000,-1.092000,"
    "0.045000,0.573000,1.170000,1.336000,1.251000,1.919000,-0.056000,-0.189000,"
    "0.028000,\n"
    "chr1\t159640161\t159640190\t2440849\t500\t-\t159640161\t159640190\t"
    "0\t1\t29,\t0,\t33\t0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,"
    "19,20,21,22,23,24,25,26,27,28,29,30,31,32,	-0.906000,-1.247000,0.111000,"
    "-0.515000,-0.057000,-0.892000,0.167000,1.278000,0.051000,-0.596000,"
    "-0.251000,-0.826000,0.487000,0.714000,0.674000,1.046000,0.694000,0.236000,"
    "-0.718000,-1.196000,-1.274000,-1.278000,-1.055000,0.838000,-0.494000,"
    "1.137000,0.000000,0.690000,0.166000,-0.232000,0.174000,-1.253000,1.363000,\n"
    "chr1\t159640215\t159640242\t2440850\t500\t-\t159640215\t159640242\t0\t1\t"
    "27,\t0,\t33\t0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,"
    "23,24,25,26,27,28,29,30,31,32,	-0.465000,0.127000,1.215000,-0.073000,"
    "-0.465000,-0.141000,0.507000,-0.462000,-0.464000,0.570000,1.356000,"
    "0.559000,-0.459000,-0.464000,-0.458000,0.000000,0.322000,-0.454000,"
    "0.887000,-0.464000,1.196000,-0.463000,0.376000,-0.461000,0.547000,"
    "0.032000,-0.464000,0.066000,0.762000,-0.465000,-0.456000,0.919000,"
    "-0.464000,\n";
static const char* kData_Newick =
    "(Bovine:0.69395,(Hylobates:0.36079,(Pongo:0.33636,(G._Gorilla:0.17147, "
    "(P._paniscus:0.19268,H._sapiens:0.11927):0.08386):0.06124):0.15057):"
    "0.54939, Rodent:1.21460);";
static const char* kData_Alignment1 =
    "#NEXUS\n"
    "[TITLE: NoName]\n\n"
    "begin data;\n"
    "dimensions ntax=3 nchar=384;\n"
    "format interleave datatype=protein   gap=- symbols=\"FSTNKEYVQMCLAWPHDRIG\";\n\n"
    "matrix\n"
    "CYS1_DICDI          -----MKVIL LFVLAVFTVF VSS------- --------RG IPPEEQ---- \n"
    "ALEU_HORVU          MAHARVLLLA LAVLATAAVA VASSSSFADS NPIRPVTDRA ASTLESAVLG \n"
    "CATH_HUMAN          ------MWAT LPLLCAGAWL LGV------- -PVCGAAELS VNSLEK---- ";
static const char* kData_Alignment2 =
    "CLUSTAL W (1.83) multiple sequence alignment\n\n"
    "aboA            -NLFV-ALYDFVASGDNTLSITKGEKLRV-------LGYNHNG-------EWCEA--QTK 42\n"
    "ycsB            KGVIY-ALWDYEPQNDDELPMKEGDCMTI-------IHREDEDEI-----EWWWA--RLN 45\n"
    "pht             -GYQYRALYDYKKEREEDIDLHLGDILTVNKGSLVALGFSDGQEARPEEIGWLNGYNETT 59\n";
static const char* kData_DistanceMatrix =
    "   14\n"
    "Mouse     \n"
    "Bovine      1.7043\n"
    "Lemur       2.0235  1.1901\n"
    "Tarsier     2.1378  1.3287  1.2905\n";
    //"Squir Monk  1.5232  1.2423  1.3199  1.7878\n"
    //"Jpn Macaq   1.8261  1.2508  1.3887  1.3137  1.0642\n";
static const char* kData_FlatFileSequence =
    "        1 ccagaatggt tactatggac atccgccaac catacaagct atggtgaaat gctttatcta\n"
    "       61 tctcattttt agtttcaaag cttttgttat aacacatgca aatccatatc cgtaaccaat\n"
    "      121 atccaatcgc ttgacatagt ctgatgaagt ttttggtagt taagataaag ctcgagactg\n"
    "      181 atatttcata tactggatga tttagggaaa cttgcattct attcatgaac gaatgagtca\n"
    "      241 atacgagaca caaccaagca tgcaaggagc tgtgagttga tgttctatgc tatttaagta\n";
static const char* kData_FiveColFeatureTable =
    ">Feature Sc_16\n"
    "1	7000	REFERENCE\n"
    "			PubMed		8849441\n"
    "<1	1050	gene\n"
    "			gene		ATH1\n"
    "			locus_tag	YPR026W\n";
static const char* kData_SnpMarkers =
    "rs10509971\t10\t114.981618\tA\n"
    "rs7580303\t2\t2.065249\tC\n"
    "rs7527281\t1\t213.591486\tC\n"
    "rs1358064\t7\t86.58632\tG\n"
    "rs4237768\t11\t5.963848\tG\n"
    "rs11771665\t7\t86.510866\tA\n"
    "rs6542185\t2\t114.423281\tT\n";
static const char* kData_Fasta =
    ">gi|13990994|dbj|BAA33523.2| hedgehog [Homo sapiens]\n"
    "MSPARLRPRLHFCLVLLLLLVVPAAWGCGPGRVVGSRRRPPRKLVPLAYKQFSPNVPEKTLGASGRYEGK\n"
    "IARSSERFKELTPNYNPDIIFKDEENTGADRLMTQRCKDRLNSLAISVMNQWPGVKLRVTEGWDEDGHHS\n"
    "EESLHYEGRAVDITTSDRDRNKYGLLARLAVEAGFDWVYYESKAHVHCSVKSEHSAAAKTGGCFPAGAQV";
static const char* kData_TextASN =
    "Seq-entry ::= set {\n"
    "  level 1 ,\n"
    "  class nuc-prot ,\n"
    "  descr {\n"
    "    pub {\n";
static const char* kData_PhrapAceOld =
    "DNA Contig35\n"
    "GATAAGataATAAtGGAAAATaGAAaccGGAaAaATaATAAaATaaTTTc\n"
    "aGATcGcTGaAGAaGAaGaGAAGaGAATAGcAGccCaATGTGAGAAGCTC\n"
    "GGcAAAAAAGGACTCGAAGaaGcGGGAAAGAGTCTgGAAGcTGCCATTCT";
static const char* kData_PhrapAceNew =
    "AS 1 6\n\n"
    "CO Contig1 1222 6 0 U\n"
    "AGTTTTAGTTTTCCTCTGAAGCAAGCACACCTTCCCTTTCCCGTCTGTCTATCCATCCCT\n"
    "GACCCTGTTGTCTGTCTATCCCTGACCCCGTAGTCTCCTAAGTCGCCCCAGATTTTGTGA\n"
    "ACACCCTCTGGAACTAGAATCTAGTGGGCGGATGGACCATTTACTAGACGGAGGTAGAGG\n";
static const char* kData_Table =
    " a         b         c         d         f         g         h\n"
    "-0.465000 -0.141000  0.507000 -0.462000 -0.464000  0.570000  1.356000\n"
    " 0.559000 -0.459000 -0.464000 -0.458000  0.000000  0.322000 -0.454000\n"
    " 0.887000 -0.464000  1.196000 -0.463000  0.376000 -0.461000  0.547000\n"
    " 0.032000 -0.464000  0.066000  0.762000 -0.465000 -0.456000  0.919000\n";
static const char* kData_TableHint_Preferred =
    "DNA        a         b         c         d         f         g\n"
    "-0.465000 -0.141000  0.507000 -0.462000 -0.464000  0.570000  1.356000\n"
    " 0.559000 -0.459000 -0.464000 -0.458000  0.000000  0.322000 -0.454000\n";


BOOST_AUTO_TEST_CASE(TestBinaryAsn)
{
    CNcbiIstrstream str(kData_AsnBin);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eBinaryASN);
}

BOOST_AUTO_TEST_CASE(TestRepeatMasker)
{
    {{
        // Without header
        CNcbiIstrstream str(kData_Rmo);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eRmo);
    }}
    {{
        // With header
        string with_header = string(kHeader_Rmo) + string(kData_Rmo);
        CNcbiIstrstream str(with_header.c_str());
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eRmo);
    }}
}

BOOST_AUTO_TEST_CASE(TestGtf)
{
    CNcbiIstrstream str(kData_Gtf);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eGtf);
}

BOOST_AUTO_TEST_CASE(TestGlimmer3)
{
    CNcbiIstrstream str(kData_Glimmer3);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eGlimmer3);
}

BOOST_AUTO_TEST_CASE(TestAgp)
{
    CNcbiIstrstream str(kData_Agp);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eAgp);
}

BOOST_AUTO_TEST_CASE(TestXml)
{
    CNcbiIstrstream str(kData_Xml);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eXml);
}

BOOST_AUTO_TEST_CASE(TestWiggle)
{
    CNcbiIstrstream str(kData_Wiggle);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eWiggle);
}

BOOST_AUTO_TEST_CASE(TestBed)
{
    {{
        CNcbiIstrstream str(kData_Bed);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eBed);
    }}
    {{
        CNcbiIstrstream str(kData_Bed15);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eBed15);
    }}
}

BOOST_AUTO_TEST_CASE(TestNewick)
{
    CNcbiIstrstream str(kData_Newick);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eNewick);
}

BOOST_AUTO_TEST_CASE(TestAlignment)
{
    {{
        CNcbiIstrstream str(kData_Alignment1);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eAlignment);
    }}
    {{
        CNcbiIstrstream str(kData_Alignment2);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eAlignment);
    }}
}

BOOST_AUTO_TEST_CASE(TestDistanceMatrix)
{
    CNcbiIstrstream str(kData_DistanceMatrix);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eDistanceMatrix);
}

BOOST_AUTO_TEST_CASE(TestFlatFileSequence)
{
    CNcbiIstrstream str(kData_FlatFileSequence);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eFlatFileSequence);
}

BOOST_AUTO_TEST_CASE(TestFiveColFeatureTable)
{
    CNcbiIstrstream str(kData_FiveColFeatureTable);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eFiveColFeatureTable);
}

BOOST_AUTO_TEST_CASE(TestSnpMarkers)
{
    CNcbiIstrstream str(kData_SnpMarkers);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eSnpMarkers);
}

BOOST_AUTO_TEST_CASE(TestFasta)
{
    CNcbiIstrstream str(kData_Fasta);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eFasta);
}

BOOST_AUTO_TEST_CASE(TestTextASN)
{
    CNcbiIstrstream str(kData_TextASN);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eTextASN);
}

/*
static const char* kData_Taxplot =
    "Not implemented";

    BOOST_AUTO_TEST_CASE(TestTaxplot)
{
    // Taxplot format check is not implemented yet
    CNcbiIstrstream str(kData_Taxplot);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eTaxplot);
}
*/

BOOST_AUTO_TEST_CASE(TestPhrapAce)
{
    {{
        CNcbiIstrstream str(kData_PhrapAceOld);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::ePhrapAce);
    }}
    {{
        CNcbiIstrstream str(kData_PhrapAceNew);
        CFormatGuess guess(str);
        BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::ePhrapAce);
    }}
}

BOOST_AUTO_TEST_CASE(TestTable)
{
    CNcbiIstrstream str(kData_Table);
    CFormatGuess guess(str);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eTable);
}

BOOST_AUTO_TEST_CASE(TestTableHint_Preferred)
{
    CNcbiIstrstream str(kData_TableHint_Preferred);
    CFormatGuess guess(str);
    // Without hints it should be detected as phrap (false positive!)
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::ePhrapAce);
    // With hint it becomes table
    guess.GetFormatHints().AddPreferredFormat(CFormatGuess::eTable);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eTable);
}

BOOST_AUTO_TEST_CASE(TestTableHint_Disabled)
{
    // Use flatfile data
    CNcbiIstrstream str(kData_FlatFileSequence);
    CFormatGuess guess(str);
    // Without hints it's a flatfile
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eFlatFileSequence);
    // With hint it becomes table
    guess.GetFormatHints().AddDisabledFormat(CFormatGuess::eFlatFileSequence);
    BOOST_CHECK_EQUAL(guess.GuessFormat(), CFormatGuess::eTable);
}
