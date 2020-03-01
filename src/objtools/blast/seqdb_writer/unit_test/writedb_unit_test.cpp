/*  $Id: writedb_unit_test.cpp 146314 2008-11-24 17:48:41Z camacho $
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
 * Authors:  Kevin Bealer
 *
 * File Description:
 *   CWriteDB unit test.
 *
 */

#include <ncbi_pch.hpp>

#include <objtools/blast/seqdb_reader/seqdbexpert.hpp>
#include <objtools/blast/seqdb_writer/writedb.hpp>
#include <objmgr/object_manager.hpp>
#include <objmgr/seq_vector.hpp>
#include <objtools/readers/fasta.hpp>
#include <serial/objistr.hpp>
#include <serial/serial.hpp>
#include "../mask_info_registry.hpp"
#include <sstream>

// Keep Boost's inclusion of <limits> from breaking under old WorkShop versions.
#if defined(numeric_limits)  &&  defined(NCBI_NUMERIC_LIMITS)
#  undef numeric_limits
#endif

#define BOOST_AUTO_TEST_MAIN
#include <boost/test/auto_unit_test.hpp>
#ifndef BOOST_PARAM_TEST_CASE
#  include <boost/test/parameterized_test.hpp>
#endif
#include <boost/current_function.hpp>
#ifndef BOOST_AUTO_TEST_CASE
#  define BOOST_AUTO_TEST_CASE BOOST_AUTO_UNIT_TEST
#endif

#include <common/test_assert.h>  /* This header must go last */

#ifndef SKIP_DOXYGEN_PROCESSING

USING_NCBI_SCOPE;
USING_SCOPE(objects);
using boost::unit_test::test_suite;

// Use macros rather than inline functions to get accurate line number reports

#define CHECK_NO_THROW(statement)                                       \
    try {                                                               \
        statement;                                                      \
        BOOST_CHECK_MESSAGE(true, "no exceptions were thrown by "#statement); \
    } catch (std::exception& e) {                                       \
        BOOST_ERROR("an exception was thrown by "#statement": " << e.what()); \
    } catch (...) {                                                     \
        BOOST_ERROR("a nonstandard exception was thrown by "#statement); \
    }

#define CHECK(expr)       CHECK_NO_THROW(BOOST_CHECK(expr))
#define CHECK_EQUAL(x, y) CHECK_NO_THROW(BOOST_CHECK_EQUAL(x, y))
#define CHECK_THROW(s, x) BOOST_CHECK_THROW(s, x)

// Fetch sequence and nucleotide data for the given oid as a pair of
// strings (in ncbi2na packed format), one for sequence data and one
// for ambiguities.

void
s_FetchRawData(CSeqDBExpert & seqdb,
               int            oid,
               string       & sequence,
               string       & ambig)
{
    const char * buffer (0);
    int          slength(0);
    int          alength(0);
    
    seqdb.GetRawSeqAndAmbig(oid, & buffer, & slength, & alength);
    
    sequence.assign(buffer, slength);
    ambig.assign(buffer + slength, alength);
    
    seqdb.RetSequence(& buffer);
}

// Return a Seq-id built from the given int (gi).

CRef<CSeq_id> s_GiToSeqId(int gi)
{
    CRef<CSeq_id> seqid(new CSeq_id(CSeq_id::e_Gi, gi));
    
    return seqid;
}

// Return a Seq-id built from the given string (accession or FASTA
// format Seq-id).

CRef<CSeq_id> s_AccToSeqId(const char * acc)
{
    CRef<CSeq_id> seqid(new CSeq_id(acc));
    
    return seqid;
}

// HexDump utility functions

string s_HexDumpText(const string      & raw,
                     const vector<int> & layout,
                     int                 base)
{
    CHECK(layout.size());
    
    string visible;
    string tmp;
    
    int layout_i = 0;
    int width = 0;
    
    for(int i = 0; i < (int)raw.size(); i += width) {
        width = layout[layout_i];
        CHECK(width);
        
        Uint8 mask = Uint8(Int8(-1));
        mask >>= (64 - 8*width);
        
        int left = raw.size() - i;
        int width1 = (left < width) ? left : width;
        
        string sub(raw, i, width1);
        
        // Read a standard order value into x.
        
        Uint8 x = 0;
        
        for(int by = 0; by < (int)sub.size(); by++) {
            x = (x << 8) + (sub[by] & 0xFF);
        }
        
        if (visible.size())
            visible += " ";
        
        tmp.resize(0);
        NStr::UInt8ToString(tmp, x & mask, 0, base);
        
        visible += tmp;
        layout_i = (layout_i + 1) % layout.size();
    }
    
    return visible;
}

string s_HexDumpText(const string & raw, int per, int base)
{
    vector<int> layout;
    layout.push_back(per);
    
    return s_HexDumpText(raw, layout, base);
}

// Overlay version

string s_HexDumpFile(const string      & fname,
                     const vector<int> & layout,
                     int                 base)
{
    ifstream f(fname.c_str());
    
    string raw;
    
    while(f && ! f.eof()) {
        char buf[1024];
        f.read(buf, 1024);
        
        int amt = f.gcount();
        
        if (! amt)
            break;
        
        raw.append(buf, amt);
    }
    
    return s_HexDumpText(raw, layout, base);
}

string s_HexDumpFile(const string & fname,
                     int            per,
                     int            base)
{
    vector<int> layout;
    layout.push_back(per);
    
    return s_HexDumpFile(fname, layout, base);
}

// Copy the sequences listed in 'ids' (integers or FASTA Seq-ids) from
// the CSeqDB object to the CWriteDB object, using CBioseqs as the
// intermediate data.

typedef vector< CRef<CSeq_id> > TIdList;

class CNonException : exception {
public:
    
};

#define CHECK_CUTPOINT(X) if (cutpoint == X) throw CNonException()

int g_NuclJ_OidCount = 99;

static void
s_DupIdsBioseq(CWriteDB      & w,
               CSeqDB        & s,
               const TIdList & ids,
               int             cutpoint)
{
    int count1 = 0;
    
    ITERATE(TIdList, iter, ids) {
        CRef<CSeq_id> seqid = *iter;
        
        CHECK(seqid.NotEmpty());
        
        CHECK_CUTPOINT(4);
        
        int oid = -1;
        bool found = s.SeqidToOid(*seqid, oid);
        
        CHECK(found);
        
        CRef<CBioseq> bs;
        
        CHECK_CUTPOINT(5);
        
        if (seqid->IsGi()) {
            bs = s.GetBioseq(oid, seqid->GetGi());
        } else {
            bs = s.GetBioseq(oid);
        }
        
        CHECK_CUTPOINT(6);
        
        CRef<CBlast_def_line_set> bdls = s.GetHdr(oid);
        
        CHECK(bs.NotEmpty());
        CHECK(bdls.NotEmpty());
        
        CHECK_CUTPOINT(7);
        
        w.AddSequence(*bs);
        w.SetDeflines(*bdls);
        
        count1++;
        CHECK_CUTPOINT(8);
        
        if (count1 > 3) {
            CHECK_CUTPOINT(9);
        }
        
        if (count1 > g_NuclJ_OidCount) {
            CHECK_CUTPOINT(10);
        }
    }
}

// Copy the sequences listed in 'ids' (integers or FASTA Seq-ids) from
// the CSeqDB object to the CWriteDB object, using packed ncbi2na
// strings ('raw' data) as the intermediate data.

static void
s_DupIdsRaw(CWriteDB      & w,
            CSeqDBExpert  & seqdb,
            const TIdList & ids)
{
    bool is_nucl = seqdb.GetSequenceType() == CSeqDB::eNucleotide;
    
    ITERATE(TIdList, iter, ids) {
        CRef<CSeq_id> seqid = *iter;
        
        int oid = -1;
        bool found = seqdb.SeqidToOid(*seqid, oid);
        
        CHECK(found);
        
        string seq, ambig;
        
        s_FetchRawData(seqdb, oid, seq, ambig);
        CRef<CBlast_def_line_set> bdls = seqdb.GetHdr(oid);
        
        CHECK(! seq.empty());
        CHECK(ambig.empty() || is_nucl);
        CHECK(bdls.NotEmpty());
        
        w.AddSequence(seq, ambig);
        w.SetDeflines(*bdls);
    }
}

// Serialize the provided ASN.1 object into a string.

template<class ASNOBJ>
void s_Stringify(const ASNOBJ & a, string & s)
{
    CNcbiOstrstream oss;
    oss << MSerial_AsnText << a;
    s = CNcbiOstrstreamToString(oss);
}

// Deserialize the provided string into an ASN.1 object.

template<class ASNOBJ>
void s_Unstringify(const string & s, ASNOBJ & a)
{
    istringstream iss;
    iss.str(s);
    iss >> MSerial_AsnText >> a;
}

// Duplicate the provided ASN.1 object (via {,de}serialization).

template<class ASNOBJ>
CRef<ASNOBJ> s_Duplicate(const ASNOBJ & a)
{
    CRef<ASNOBJ> newobj(new ASNOBJ);
    
    string s;
    s_Stringify(a, s);
    s_Unstringify(s, *newobj);
    
    return newobj;
}

// Compare the two CBioseqs by comparing their serialized forms.

void s_CompareBioseqs(CBioseq & src, CBioseq & dst)
{
    string s1, s2;
    s_Stringify(src, s1);
    s_Stringify(dst, s2);
    
    CHECK_EQUAL(s1, s2);
}

// Test the database compared to a reference database, usually the
// database that provided the source data.

void
s_TestDatabase(CSeqDBExpert & src,
               const string & name,
               const string & title)
{
    CSeqDBExpert dst(name, src.GetSequenceType());
    
    for(int oid = 0; dst.CheckOrFindOID(oid); oid++) {
        int gi(0), src_oid(0);
        
        bool rv1 = dst.OidToGi(oid, gi);
        bool rv2 = src.GiToOid(gi, src_oid);
        
        CHECK(rv1);
        CHECK(rv2);
        
        CRef<CBioseq> bss = src.GetBioseq(src_oid);
        CRef<CBioseq> bsd = dst.GetBioseq(oid);
        
        s_CompareBioseqs(*bss, *bsd);
    }
    
    CHECK_EQUAL(dst.GetTitle(), title);
}

// Remove the specified files.

void s_RemoveFile(const string & f)
{
    CDirEntry de(f);
    de.Remove(CDirEntry::eOnlyEmpty);
}

void s_RemoveFiles(const vector<string> & files)
{
    for(unsigned i = 0; i < files.size(); i++) {
        s_RemoveFile(files[i]);
    }
}

// Check if the given file is already sorted.

void s_CheckSorted(const string & fname)
{
    CNcbiIfstream file(fname.c_str());
    
    string s, s2;
    
    while(NcbiGetlineEOL(file, s)) {
        CHECK(s2 <= s);
        s.swap(s2);
    }
}

// Check the files that make up a database volume.
//
// nsd/psd: Check that the file is in sorted order

string s_ExtractLast(const string & data, const string & delim)
{
    size_t pos = data.rfind(delim);
    
    if (pos == string::npos)
        return "";
    
    return string(data,
                  pos+delim.size(),
                  data.size()-(pos + delim.size()));
}

// Check the files that make up a database volume.
//
// nsd/psd: Check that the file is in sorted order

void s_CheckFiles(const vector<string> & files,
                  bool                   need_hash = false)
{
    bool found_hash = false;
    
    for(unsigned i = 0; i < files.size(); i++) {
        string ext = s_ExtractLast(files[i], ".");
        
        if (ext == "nsd" || ext == "psd") {
            s_CheckSorted(files[i]);
        }
        if (ext == "nhd" || ext == "phd") {
            s_CheckSorted(files[i]);
            found_hash = true;
        }
    }
    
    if (need_hash) {
        CHECK(found_hash);
    }
}

// Do sanity checks appropriate for some files, then remove them.

void s_WrapUpFiles(const vector<string> & files)
{
    s_CheckFiles(files);
    s_RemoveFiles(files);
}

// Like s_WrapUpFiles but starting with the DB.

void s_WrapUpDb(CWriteDB & db)
{
    vector<string> files;
    db.ListFiles(files);
    s_WrapUpFiles(files);
}

class CWrapperUpper {
public:
    CWrapperUpper()
    {
    }
    
    ~CWrapperUpper()
    {
        if (m_Db.NotEmpty()) {
            s_WrapUpDb(*m_Db);
        }
    }
    
    void SetDb(CWriteDB & db)
    {
        m_Db.Reset(& db);
    }
    
private:
    CRef<CWriteDB> m_Db;
};

// Copy the specified ids (int -> GI, string -> FASTA Seq-id) from the
// source database (src_name) to a new CWriteDB object, then perform
// checks on the resulting database and remove it.

static void
s_DupSequencesTest(const TIdList & ids,
                   bool            is_protein,
                   bool            raw_data,
                   const string &          src_name,
                   const string &          dst_name,
                   const string &          title,
                   int             cutpoint = 99)
{
    CWrapperUpper wrap;
    
    CHECK_CUTPOINT(1);
    
    CSeqDBExpert src(src_name, (is_protein
                                ? CSeqDB::eProtein
                                : CSeqDB::eNucleotide));
    
    vector<string> files;
    
    CRef<CWriteDB> db;
    
    CHECK_CUTPOINT(2);
    
    db.Reset(new CWriteDB(dst_name,
                          (is_protein
                           ? CWriteDB::eProtein
                           : CWriteDB::eNucleotide),
                          title,
                          CWriteDB::eFullIndex));
    
    wrap.SetDb(*db);
    
    CHECK_CUTPOINT(3);
    
    if (raw_data) {
        s_DupIdsRaw(*db, src, ids);
    } else {
        s_DupIdsBioseq(*db, src, ids, cutpoint);
    }
    
    CHECK_CUTPOINT(10);
    
    db->Close();
    db->ListFiles(files);
    db.Reset();
    
    CHECK_CUTPOINT(11);
    
    s_TestDatabase(src, dst_name, title);
    
    CHECK_CUTPOINT(12);
}

// Get and return a CScope with local copies of test sequences loaded.

static CRef<CScope> s_GetScope()
{
    static CRef<CObjectManager> obj_mgr = CObjectManager::GetInstance();

    CRef<CScope> scope(new CScope(*obj_mgr));

    auto_ptr<CObjectIStream> ois
        (CObjectIStream::Open(eSerial_AsnText, "data/gi129295.asn"));
    CRef<CSeq_entry> entry(new CSeq_entry);

    *ois >> *entry;
    scope->AddTopLevelSeqEntry(*entry);

    ois.reset(CObjectIStream::Open(eSerial_AsnText, "data/gi129296.asn"));
    entry.Reset(new CSeq_entry);

    *ois >> *entry;
    scope->AddTopLevelSeqEntry(*entry);

    return scope;
}

static void s_BuildIds(TIdList & ids, int * gis)
{
    for(int * ptr = gis; *ptr; ptr ++) {
        ids.push_back(s_GiToSeqId(*ptr));
    }
}

static void s_BuildIds(TIdList & ids, const char ** gis)
{
    for(const char ** ptr = gis; *ptr; ptr ++) {
        ids.push_back(s_AccToSeqId(*ptr));
    }
}

CRef<CBioseq> s_FastaStringToBioseq(const string & str, bool protein)
{
    istrstream istr(str.data(), str.size());
    
    CRef<ILineReader> lr(new CStreamLineReader(istr));
    
    typedef CFastaReader::EFlags TFlags;
    
    TFlags flags = (TFlags) (CFastaReader::fAllSeqIds |
                             (protein
                              ? CFastaReader::fAssumeProt
                              : CFastaReader::fAssumeNuc));
    
    CFastaReader fr(*lr, flags);
    
    CHECK(! lr->AtEOF());
    CRef<CSeq_entry> entry = fr.ReadOneSeq();
    
    CHECK(! entry.Empty());
    CHECK(entry->IsSeq());
    
    CRef<CBioseq> bs(& entry->SetSeq());
    
    return bs;
}


// Unit test verbosity support.

static void s_UnitTestVerbosity(string s)
{
    static bool enabled = static_cast<bool>(getenv("VERBOSE_UT") != NULL);
    
    if (enabled) {
        cout << "Running test: " << s << endl;
    }
}

#define UNIT_TEST_FILTER(s) \
{ \
    string nm(s); \
    string filter(getenv("FILTER_UT") ? getenv("FILTER_UT") : ""); \
    \
    if (nm.size() && filter.size() && nm.find(filter) == string::npos) { \
        cout << "Skipping test: " << s << endl; \
        return; \
    } \
}

#define START UNIT_TEST_FILTER(BOOST_CURRENT_FUNCTION); \
              s_UnitTestVerbosity(BOOST_CURRENT_FUNCTION)


//
// Actual test cases.
//

static void s_NuclBioseqDupSwitch(int cutpoint)
{
    START;
    
    int gis[] = {
        78883515, 78883517, /*71143095,*/ 24431485, 19110479, 15054463,
        15054465, 15054467, 15054469, 15054471, 19570808, 18916476,
        1669608,  1669610,  1669612,  1669614,  1669616,  10944307,
        10944309, 10944311, 19909844, 19909846, 19909860, 19911180,
        19911220, 19911222, 19911224, 57472140, 20126670, 20387092,
        57639630, 57639632, 7670507,  2394289,  21280378, 21327938,
        6518520,  20086356, 20086357, 21392391, 20086359, 19110509,
        21623739, 21623761, 38303844, 38197377, 56788779, 57032781,
        57870443, 56789136, 0
    };
    
    TIdList ids;
    s_BuildIds(ids, gis);
    
    CHECK_CUTPOINT(0);
    
    const string srcname("nt");
    const string dstname("w-nucl-bs");
    const string title("bioseq nucleotide dup");

    s_DupSequencesTest(ids,
                       false,
                       false,
                       srcname,
                       dstname,
                       title,
                       cutpoint);
    
    CHECK_CUTPOINT(13);
    
    const string dstname2("w-nucl-raw");
    const string title2("raw nucleotide dup");
    s_DupSequencesTest(ids,
                       false,
                       true,
                       srcname,
                       dstname2,
                       title2,
                       cutpoint);
    
    CHECK_CUTPOINT(14);
}

#if 0
BOOST_AUTO_TEST_CASE(NuclBioseqDupZ)
{
    START;
    
    try {
        s_NuclBioseqDupSwitch(0);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupA)
{
    START;
    
    try {
        s_NuclBioseqDupSwitch(1);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupB)
{
    START;

    try {
        s_NuclBioseqDupSwitch(2);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupC)
{
    START;

    try {
        s_NuclBioseqDupSwitch(3);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupD)
{
    START;

    try {
        s_NuclBioseqDupSwitch(4);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupE)
{
    START;

    try {
        s_NuclBioseqDupSwitch(5);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupF)
{
    START;

    try {
        s_NuclBioseqDupSwitch(6);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupG)
{
    START;
    
    try {
        s_NuclBioseqDupSwitch(7);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupH)
{
    START;

    try {
        s_NuclBioseqDupSwitch(8);
    }
    catch(CNonException &) {
    }
}
#endif

BOOST_AUTO_TEST_CASE(NuclBioseqDupI)
{
    START;

    try {
        s_NuclBioseqDupSwitch(9);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ4)
{
    START;
    
    g_NuclJ_OidCount = 4;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ8)
{
    START;

    g_NuclJ_OidCount = 8;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ12)
{
    START;

    g_NuclJ_OidCount = 12;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ16)
{
    START;
    
    g_NuclJ_OidCount = 16;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ20)
{
    START;
    
    g_NuclJ_OidCount = 20;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ24)
{
    START;
    
    g_NuclJ_OidCount = 24;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ28)
{
    START;
    
    g_NuclJ_OidCount = 28;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ32)
{
    START;
    
    g_NuclJ_OidCount = 32;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ33)
{
    START;
    
    g_NuclJ_OidCount = 33;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ34)
{
    START;
    
    g_NuclJ_OidCount = 34;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ35)
{
    START;
    
    g_NuclJ_OidCount = 35;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ36)
{
    START;
    
    g_NuclJ_OidCount = 36;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ40)
{
    START;
    
    g_NuclJ_OidCount = 40;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ44)
{
    START;
    
    g_NuclJ_OidCount = 44;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ45)
{
    START;
    
    g_NuclJ_OidCount = 45;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ46)
{
    START;
    
    g_NuclJ_OidCount = 46;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ47)
{
    START;
    
    g_NuclJ_OidCount = 47;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ48)
{
    START;
    
    g_NuclJ_OidCount = 48;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ49)
{
    START;
    
    g_NuclJ_OidCount = 49;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ50)
{
    START;
    
    g_NuclJ_OidCount = 50;
    
    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupJ)
{
    START;

    try {
        s_NuclBioseqDupSwitch(10);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupK)
{
    START;

    try {
        s_NuclBioseqDupSwitch(11);
    }
    catch(CNonException &) {
    }
}

#if 0
BOOST_AUTO_TEST_CASE(NuclBioseqDupL)
{
    START;

    try {
        s_NuclBioseqDupSwitch(12);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupM)
{
    START;

    try {
        s_NuclBioseqDupSwitch(13);
    }
    catch(CNonException &) {
    }
}

BOOST_AUTO_TEST_CASE(NuclBioseqDupN)
{
    START;

    try {
        s_NuclBioseqDupSwitch(14);
    }
    catch(CNonException &) {
    }
}
#endif

BOOST_AUTO_TEST_CASE(NuclBioseqDup)
{
    START;
    
    s_NuclBioseqDupSwitch(99);
}

BOOST_AUTO_TEST_CASE(ProtBioseqDup)
{
    START;
    
    int gis[] = {
        1477444,   1669609,   1669611,  1669615, 1669617, 7544146,
        22652804, /*1310870,*/ 3114354, 3891778, 3891779, 81294290,
        81294330,  49089974,  62798905, 3041810, 7684357, 7684359,
        7684361,   7684363,   7544148,  3452560, 3452564, 6681587,
        6681590,   6729087,   7259315,  2326257, 3786310, 3845607,
        13516469,  2575863,   4049591,  3192363, 1871126, 2723484,
        6723181,   11125717,  2815400,  1816433, 3668177, 6552408,
        13365559,  8096667,   3721768,  9857600, 2190043, 3219276,
        10799943,  10799945,  0
    };
    
    TIdList ids;
    s_BuildIds(ids, gis);
    
    s_DupSequencesTest(ids,
                       true,
                       false,
                       "nr",
                       "w-prot-bs",
                       "bioseq protein dup");
    
    s_DupSequencesTest(ids,
                       true,
                       true,
                       "nr",
                       "w-prot-raw",
                       "raw protein dup");
}

BOOST_AUTO_TEST_CASE(EmptyBioseq)
{
    START;
    
    CWriteDB fails("failing-db",
                   CWriteDB::eProtein,
                   "title",
                   CWriteDB::eFullIndex);
    
    CRef<CBioseq> bs(new CBioseq);
    fails.AddSequence(*bs);
    
    CHECK_THROW(fails.Close(), CWriteDBException);
}

BOOST_AUTO_TEST_CASE(BioseqHandle)
{
    START;
    
    CWriteDB db("from-loader",
                CWriteDB::eProtein,
                "title",
                CWriteDB::eFullIndex);
    
    CRef<CScope> scope = s_GetScope();
    
    // Normal bioseq handle.
    
    CRef<CSeq_id> id1(new CSeq_id("gi|129295"));
    CBioseq_Handle bsh1 = scope->GetBioseqHandle(*id1);
    db.AddSequence(bsh1);
    
    // Clean up.
    
    db.Close();
    s_WrapUpDb(db);
}

BOOST_AUTO_TEST_CASE(BioseqHandleAndSeqVectorNonWriteDB)
{
    START;
    
    // This is a modified version of the following test.  The
    // assumption is that some errors occur due to environmental
    // factors.  Hopefully this test will help to determine the
    // library in which these intermittent errors occur.
    
    CRef<CScope> scope = s_GetScope();
    
    CRef<CSeq_id> id2(new CSeq_id("gi|129296"));
    CBioseq_Handle bsh2 = scope->GetBioseqHandle(*id2);
    CConstRef<CBioseq> bs1c = bsh2.GetCompleteBioseq();
    
    CRef<CBioseq> bs1 = s_Duplicate(*bs1c);
    CSeqVector sv(bsh2);
    
    string bytes;
    sv.GetSeqData(0, sv.size(), bytes);
    
    CHECK(bytes.size() == sv.size());
}

BOOST_AUTO_TEST_CASE(BioseqHandleAndSeqVector)
{
    START;
    
    CRef<CScope> scope = s_GetScope();
    
    // Bioseq + CSeqVector.
    
    CRef<CSeq_id> id2(new CSeq_id("gi|129296"));
    CBioseq_Handle bsh2 = scope->GetBioseqHandle(*id2);
    CConstRef<CBioseq> bs1c = bsh2.GetCompleteBioseq();
    
    CRef<CBioseq> bs1 = s_Duplicate(*bs1c);
    CSeqVector sv(bsh2);
    
    string bytes;
    sv.GetSeqData(0, sv.size(), bytes);
}

BOOST_AUTO_TEST_CASE(BioseqHandleAndSeqVectorWriteDB)
{
    START;
    
    CWriteDB db("from-loader",
                CWriteDB::eProtein,
                "title",
                CWriteDB::eFullIndex);
    
    CRef<CScope> scope = s_GetScope();
    
    // Bioseq + CSeqVector.
    
    CRef<CSeq_id> id2(new CSeq_id("gi|129296"));
    CBioseq_Handle bsh2 = scope->GetBioseqHandle(*id2);
    CConstRef<CBioseq> bs1c = bsh2.GetCompleteBioseq();
    
    CRef<CBioseq> bs1 = s_Duplicate(*bs1c);
    CSeqVector sv(bsh2);
    
    // Make sure CSeqVector is exercised by removing the Seq-data.
    
    bs1->SetInst().ResetSeq_data();
    db.AddSequence(*bs1, sv);
    
    // Clean up.
    
    db.Close();
    s_WrapUpDb(db);
}

BOOST_AUTO_TEST_CASE(SetPig)
{
    START;
    
    string nm = "pigs";
    vector<string> files;
    
    {
        CSeqDB nr("nr", CSeqDB::eProtein);
        
        CWriteDB db(nm,
                    CWriteDB::eProtein,
                    "title",
                    CWriteDB::eFullIndex);
        
        db.AddSequence(*nr.GiToBioseq(129295));
        db.SetPig(101);
        
        db.AddSequence(*nr.GiToBioseq(129296));
        db.SetPig(102);
        
        db.AddSequence(*nr.GiToBioseq(129297));
        db.SetPig(103);
        
        db.Close();
        db.ListFiles(files);
    }
    
    CSeqDB db2(nm, CSeqDB::eProtein);
    
    int oid = 0;
    
    for(; db2.CheckOrFindOID(oid); oid++) {
        int pig(0);
        vector<int> gis;
        
        bool rv1 = db2.OidToPig(oid, pig);
        db2.GetGis(oid, gis, false);
        
        bool found_gi = false;
        for(unsigned i = 0; i < gis.size(); i++) {
            if (gis[i] == (129295 + oid)) {
                found_gi = true;
            }
        }
        
        CHECK(rv1);
        CHECK(found_gi);
        CHECK_EQUAL(pig-oid, 101);
    }
    
    CHECK_EQUAL(oid, 3);
    
    s_WrapUpFiles(files);
}

// Test multiple volume construction and maximum letter limit.

BOOST_AUTO_TEST_CASE(MultiVolume)
{
    START;
    
    CSeqDB nr("nr", CSeqDB::eProtein);
    
    CWriteDB db("multivol",
                CWriteDB::eProtein,
                "title",
                CWriteDB::eFullIndex);
    
    db.SetMaxVolumeLetters(500);
    
    int gis[] = { 129295, 129296, 129297, 129299, 0 };
    
    Uint8 letter_count = 0;
    
    for(int i = 0; gis[i]; i++) {
        int oid(0);
        nr.GiToOid(gis[i], oid);
        
        db.AddSequence(*nr.GetBioseq(oid));
        letter_count += nr.GetSeqLength(oid);
    }
    
    db.Close();
    
    vector<string> v;
    vector<string> f;
    db.ListVolumes(v);
    db.ListFiles(f);
    
    CHECK_EQUAL(3, (int) v.size());
    CHECK_EQUAL(v[0], string("multivol.00"));
    CHECK_EQUAL(v[1], string("multivol.01"));
    CHECK_EQUAL(v[2], string("multivol.02"));
    
    CHECK_EQUAL(22, (int) f.size());
    
    // Check resulting db.
    
    CRef<CSeqDB> seqdb(new CSeqDB("multivol", CSeqDB::eProtein));
    
    int oids(0);
    Uint8 letters(0);
    
    seqdb->GetTotals(CSeqDB::eUnfilteredAll, & oids, & letters, false);
    
    CHECK_EQUAL(oids, 4);
    CHECK_EQUAL(letter_count, letters);
    
    seqdb.Reset();
    
    s_WrapUpFiles(f);
}

BOOST_AUTO_TEST_CASE(UsPatId)
{
    START;
    
    CRef<CSeq_id> seqid(new CSeq_id("pat|us|123|456"));
    vector<string> files;
    
    {
        CRef<CWriteDB> writedb
            (new CWriteDB("uspatid",
                          CWriteDB::eProtein,
                          "patent id test",
                          CWriteDB::eFullIndex));
        
        CSeqDB seqdb("nr", CSeqDB::eProtein);
        
        CRef<CBioseq> bs = seqdb.GiToBioseq(129297);
        
        CRef<CBlast_def_line_set> bdls(new CBlast_def_line_set);
        CRef<CBlast_def_line> dl(new CBlast_def_line);
        bdls->Set().push_back(dl);
        
        dl->SetTitle("Some protein sequence");
        dl->SetSeqid().push_back(seqid);
        dl->SetTaxid(12345);
        
        writedb->AddSequence(*bs);
        writedb->SetDeflines(*bdls);
        
        writedb->Close();
        writedb->ListFiles(files);
        CHECK(files.size() != 0);
    }
    
    CSeqDB seqdb("uspatid", CSeqDB::eProtein);
    int oid(-1);
    bool found = seqdb.SeqidToOid(*seqid, oid);
    
    CHECK_EQUAL(found, true);
    CHECK_EQUAL(oid,   0);
    
    s_WrapUpFiles(files);
}

BOOST_AUTO_TEST_CASE(IsamSorting)
{
    START;
    
    // This checks whether the following IDs are fetchable from the
    // given database.  It will fail if either the production blast
    // databases (i.e. found at $BLASTDB) are corrupted or if the
    // newly produced database is corrupted.  It will also fail if any
    // of the IDs are legitimately missing (removed by the curators),
    // in which case the given ID must be removed from the list.
    
    // However, the selection of these specific IDs is not arbitrary;
    // these are several sets of IDs which have a common 6 letter
    // prefix.  The test will not work correctly if these IDs are
    // replaced with IDs that don't have this trait, if too many are
    // removed, or if the IDs are put in sorted order.
    
    // A null terminated array of NUL terminated strings.
    
    const char* accs[] = {
        "AAC76335.1", "AAC77159.1", "AAA58145.1", "AAC76880.1",
        "AAC76230.1", "AAC76373.1", "AAC77137.1", "AAC76637.2",
        "AAA58101.1", "AAC76329.1", "AAC76702.1", "AAC77109.1",
        "AAC76757.1", "AAA58162.1", "AAC76604.1", "AAC76539.1",
        "AAA24224.1", "AAC76351.1", "AAC76926.1", "AAC77047.1",
        "AAC76390.1", "AAC76195.1", "AAA57930.1", "AAC76134.1",
        "AAC76586.2", "AAA58123.1", "AAC76430.1", "AAA58107.1",
        "AAC76765.1", "AAA24272.1", "AAC76396.2", "AAA24183.1",
        "AAC76918.1", "AAC76727.1", "AAC76161.1", "AAA57964.1",
        "AAA24251.1", 0
    };
    
    TIdList ids;
    s_BuildIds(ids, accs);
    
    s_DupSequencesTest(ids,
                       true,
                       false,
                       "nr",
                       "w-isam-sort-bs",
                       "test of string ISAM sortedness");
}

BOOST_AUTO_TEST_CASE(HashToOid)
{
    START;
    
    CSeqDBExpert nr("nr", CSeqDB::eProtein);
    CSeqDBExpert nt("nt", CSeqDB::eNucleotide);
    
    int prot_gis[] = { 129295, 129296, 129297, 0 };
    int nucl_gis[] = { 555, 556, 405832, 0 };
    
    TIdList prot_ids, nucl_ids;
    s_BuildIds(prot_ids, prot_gis);
    s_BuildIds(nucl_ids, nucl_gis);
    
    typedef CWriteDB::EIndexType TType;
    
    TType itype = TType(CWriteDB::eFullWithTrace |
                        CWriteDB::eAddHash);
    
    CRef<CWriteDB> prot(new CWriteDB("w-prot-hash",
                                     CWriteDB::eProtein,
                                     "test of hash ISAMs (P)",
                                     itype));
    
    CRef<CWriteDB> nucl(new CWriteDB("w-nucl-hash",
                                     CWriteDB::eNucleotide,
                                     "test of hash ISAMs (N)",
                                     itype));
    
    s_DupIdsBioseq(*prot, nr, prot_ids, 99);
    s_DupIdsBioseq(*nucl, nt, nucl_ids, 99);
    
    prot->Close();
    nucl->Close();

    s_WrapUpDb(*prot);
    s_WrapUpDb(*nucl);
}

BOOST_AUTO_TEST_CASE(FastaReaderBioseq)
{
    START;
    
    vector<string> files;
    
    string title = "from-fasta-reader";
    
    string
        I1("gi|123"), T1("One two three."),
        I2("gi|124"), T2("One two four.");
    
    {
        CRef<CWriteDB> wr(new CWriteDB(title,
                                       CWriteDB::eProtein,
                                       "title",
                                       CWriteDB::eFullIndex));
        
        // Build a multi-defline bioseq and read it with CFastaReader.
        
        string str =
            ">"    + I1 + " " + T1 +
            "\001" + I2 + " " + T2 + "\n" +
            "ELVISLIVES\n";
        
        CRef<CBioseq> bs = s_FastaStringToBioseq(str, true);
        
        wr->AddSequence(*bs);
        wr->Close();
        
        // Clean up.
        
        wr->ListFiles(files);
    }
    
    {
        CSeqDB rd("from-fasta-reader", CSeqDB::eProtein);
        CHECK(rd.GetNumOIDs() == 1);
        
        CRef<CBlast_def_line_set> bdls =
            rd.GetHdr(0);
        
        CHECK(bdls->Get().size() == 2);
        CHECK(bdls->Get().front()->GetTitle() == T1);
        CHECK(bdls->Get().front()->GetSeqid().size() == 1);
        CHECK(bdls->Get().front()->GetSeqid().front()->AsFastaString() == I1);
        
        CHECK(bdls->Get().back()->GetTitle() == T2);
        CHECK(bdls->Get().back()->GetSeqid().size() == 1);
        CHECK(bdls->Get().back()->GetSeqid().front()->AsFastaString() == I2);
    }
    
    s_WrapUpFiles(files);
}

BOOST_AUTO_TEST_CASE(BinaryListBuilder)
{
    START;
    
    string fn4("test4.til"), fn8("test8.til");
    
    {
        CBinaryListBuilder blb4(CBinaryListBuilder::eTi);
        CBinaryListBuilder blb8(CBinaryListBuilder::eTi);
        
        for(int i = 0; i<10; i++) {
            blb4.AppendId(Int8(1) << (i*2));
            blb8.AppendId(Int8(1) << (i*4));
        }
        
        blb4.Write(fn4);
        blb8.Write(fn8);
    }
    
    string h4 = s_HexDumpFile(fn4, 4, 16);
    string h8 = s_HexDumpFile(fn8, 4, 16);
    
    // The FF...FD symbol indicates a 4 byte TI list; the FF..FC
    // symbol is the eight byte version.
    
    CHECK(h4 ==
          "FFFFFFFD A "
          "1 4 10 40 100 "
          "400 1000 4000 10000 40000");
    
    CHECK(h8 ==
          "FFFFFFFC A "
          "0 1 0 10 0 100 0 1000 0 10000 "
          "0 100000 0 1000000 0 10000000 1 0 10 0");
    
    CFile(fn4).Remove();
    CFile(fn8).Remove();
}

BOOST_AUTO_TEST_CASE(FourAndEightByteTis)
{
    START;
    
    typedef pair<string, string> TPair;
    vector< TPair > ids48;
    
    // Generate gnl|ti# IDs where # is 1234*2^N for db4, and
    // 1234*1000^N for db8.
    
    {
        Int8 a4(1234), b4(2), a8(1234), b8(1000);
        
        string prefix = "gnl|ti|";
        
        for(int i = 0; i < 5; i++) {
            TPair p;
            p.first = prefix + NStr::Int8ToString(a4);
            p.second = prefix + NStr::Int8ToString(a8);
            
            ids48.push_back(p);
            Int8 p4(a4), p8(a8);
            
            a4 *= b4;
            a8 *= b8;
            
            // Check for overflow.
            
            CHECK(a4 > p4);
            CHECK(a8 > p8);
        }
        
        // Make sure we really do have 32 and 64 bit IDs.
        
        CHECK((a4 >> 32) == 0);
        CHECK((a8 >> 32) != 0);
    }
    
    string dbname4 = "test-db-short-tis";
    string dbname8 = "test-db-long-tis";
    
    CWriteDB db4(dbname4,
                 CWriteDB::eNucleotide,
                 dbname4 + " database.",
                 CWriteDB::eFullWithTrace);
    
    CWriteDB db8(dbname8,
                 CWriteDB::eNucleotide,
                 dbname8 + " database.",
                 CWriteDB::eFullWithTrace);
    
    string iupac = "GATTACA";
    
    ITERATE(vector< TPair >, iter, ids48) {
        string f4 = string(">") + iter->first + " test\n" + iupac + "\n";
        string f8 = string(">") + iter->second + " test\n" + iupac + "\n";
        
        db4.AddSequence( *s_FastaStringToBioseq(f4, false) );
        db8.AddSequence( *s_FastaStringToBioseq(f8, false) );
    }
    
    db4.Close();
    db8.Close();
    
    // Use 4 byte dumps for the (mixed field width) index files.
    
    string index4 = s_HexDumpFile(dbname4 + ".nti", 4, 16);
    string index8 = s_HexDumpFile(dbname8 + ".nti", 4, 16);
    
    string
        i4("1 0 28 5 1 100 0 0 0 4D2 0 FFFFFFFF 0"),
        i8("1 5 3C 5 1 100 0 0 0 0 4D2 0 FFFFFFFF FFFFFFFF 0"),
        d4("1234 0 2468 1 4936 2 9872 3 19744 4"),
        d8("1234 0 1234000 1 1234000000 2 1234000000000 3 1234000000000000 4");
    
    CHECK(index4 == i4);
    CHECK(index8 == i8);
    
    vector<int> overlay;
    overlay.push_back(8);
    overlay.push_back(4);
    
    // The 32-bit TI data file is uniformly 4 bytes.  The 8 byte file
    // alternates between 8 and 4 byte fields.
    
    string data4 = s_HexDumpFile(dbname4 + ".ntd", 4, 10);
    string data8 = s_HexDumpFile(dbname8 + ".ntd", overlay, 10);
    
    s_WrapUpDb(db4);
    s_WrapUpDb(db8);
    
    CHECK(data4 == d4);
    CHECK(data8 == d8);
}

#if ((!defined(NCBI_COMPILER_WORKSHOP) || (NCBI_COMPILER_VERSION  > 550)) && \
     (!defined(NCBI_COMPILER_MIPSPRO)) )
void s_WrapUpColumn(CWriteDB_ColumnBuilder & cb)
{
    vector<string> files;
    cb.ListFiles(files);
    s_WrapUpFiles(files);
}

BOOST_AUTO_TEST_CASE(UserDefinedColumns)
{
    START;
    
    // Create and open the DBs and columns.
    
    typedef map<string,string> TMeta;
    TMeta meta_data;
    meta_data["created-by"] = "unit test";
    meta_data["purpose"] = "none";
    meta_data["format"] = "text";
    
    vector<string> column_data;
    column_data.push_back("Groucho Marx");
    column_data.push_back("Charlie Chaplain");
    column_data.push_back("");
    column_data.push_back("Abbott and Costello");
    column_data.push_back("Jackie Gleason");
    column_data.push_back("Jerry Seinfeld");
    column_data.back()[5] = (char) 0;
    
    string fname("user-column");
    string vname("user-column-db");
    string title("comedy");
    
    CSeqDB R("nr", CSeqDB::eProtein);
    CWriteDB W(vname,
               CWriteDB::eProtein,
               "User defined column");
    
    CWriteDB_ColumnBuilder CB(title, fname);
    
    int col_id = W.CreateUserColumn(title);
    
    ITERATE(TMeta, iter, meta_data) {
        CB.AddMetaData(iter->first, iter->second);
        W.AddColumnMetaData(col_id, iter->first, iter->second);
    }
    
    // Build database and column.
    
    int i = 0;
    
    ITERATE(vector<string>, iter, column_data) {
        W.AddSequence(*R.GetBioseq(i++));
        
        CBlastDbBlob & b1 = W.SetBlobData(col_id);
        b1.WriteString(*iter, CBlastDbBlob::eNone);
        
        CBlastDbBlob b2(*iter, false);
        CB.AddBlob(b2);
    }
    
    // Close the DB and the column.
    
    W.Close();
    CB.Close();
    
    // Test the resulting files.
    
    // (Currently, the files created here are not tested.  Instead,
    // the SeqDB test uses copies of these files and tests the data
    // integrity via the SeqDB functionality.)
    
    // Clean up.
    
    s_WrapUpColumn(CB);
    s_WrapUpDb(W);
}

// Register standard masking algorithms with default/sensible options
BOOST_AUTO_TEST_CASE(RegisterMaskingAlgorithms)
{
    CMaskInfoRegistry registry;

    vector<int> algo_ids;
    algo_ids.push_back(registry.Add(eBlast_filter_program_seg));
    algo_ids.push_back(registry.Add(eBlast_filter_program_dust));
    algo_ids.push_back(registry.Add(eBlast_filter_program_windowmasker));
    algo_ids.push_back(registry.Add(eBlast_filter_program_repeat, "9606"));
    algo_ids.push_back(registry.Add(eBlast_filter_program_other, "dummy"));

    ITERATE(vector<int>, id, algo_ids) {
        CHECK_EQUAL(true, registry.IsRegistered(*id));
    }
}

BOOST_AUTO_TEST_CASE(RegisterVariantsOfSameMaskingAlgorithm)
{
    CMaskInfoRegistry registry;

    int id1 = registry.Add(eBlast_filter_program_seg);
    int id2 = registry.Add(eBlast_filter_program_seg, "dummy");
    CHECK_EQUAL(id1+1, id2);
}

void 
RegisterTooManyVariantsOfSameMaskingAlgorithm
    (EBlast_filter_program masking_algo,
     size_t kMaxNumSupportedAlgorithmVariants)
{
    CMaskInfoRegistry registry;

    vector<int> algo_ids;
    for (size_t i = 0; i < kMaxNumSupportedAlgorithmVariants*2; i++) {
        string options;
        // for repeat and other masking algorithms, there must be options,
        // otherwise the actual masking algorithm value becomes the algorithm
        // id when no options are provided
        if (i == 0 && masking_algo < eBlast_filter_program_repeat) {
            options.assign("");
        } else {
            options.assign(NStr::IntToString(i));
        }
    
        int algo_id = -1;
        if (i >= kMaxNumSupportedAlgorithmVariants) {
            CHECK_THROW(algo_id = registry.Add(masking_algo, options), 
                        CWriteDBException);
        } else {
            algo_id = registry.Add(masking_algo, options);
        }
        if (algo_id != -1) {
            //cerr << "Inserted id  " << algo_id << endl;
            algo_ids.push_back(algo_id);
        }
    }

    // Ensure that the IDs were assigned in increasing order
    CHECK_EQUAL(kMaxNumSupportedAlgorithmVariants, algo_ids.size());
    for (size_t i = 0; i < algo_ids.size(); i++) {
        CHECK_EQUAL((int)(masking_algo + i), algo_ids[i]);
    }

    // Ensure that only valid IDs were assigned
    for (size_t i = 0; i < kMaxNumSupportedAlgorithmVariants*2; i++) {
        int algo_id = masking_algo + i;
        if (i >= kMaxNumSupportedAlgorithmVariants) {
            CHECK_EQUAL(false, registry.IsRegistered(algo_id));
        } else {
            CHECK_EQUAL(true, registry.IsRegistered(algo_id));
        }
    }
}

BOOST_AUTO_TEST_CASE(RegisterTooManyVariantsOfDust)
{
    const EBlast_filter_program self = eBlast_filter_program_dust;
    const size_t max_algo_variants = eBlast_filter_program_seg - self;
    RegisterTooManyVariantsOfSameMaskingAlgorithm(self, max_algo_variants);
}

BOOST_AUTO_TEST_CASE(RegisterTooManyVariantsOfSeg)
{
    const EBlast_filter_program self = eBlast_filter_program_seg;
    const size_t max_algo_variants = eBlast_filter_program_windowmasker - self;
    RegisterTooManyVariantsOfSameMaskingAlgorithm(self, max_algo_variants);
}

BOOST_AUTO_TEST_CASE(RegisterTooManyVariantsOfWindowMasker)
{
    const EBlast_filter_program self = eBlast_filter_program_windowmasker;
    const size_t max_algo_variants = eBlast_filter_program_repeat - self;
    RegisterTooManyVariantsOfSameMaskingAlgorithm(self, max_algo_variants);
}

BOOST_AUTO_TEST_CASE(RegisterTooManyVariantsOfRepeats)
{
    const EBlast_filter_program self = eBlast_filter_program_repeat;
    const size_t max_algo_variants = eBlast_filter_program_other - self;
    RegisterTooManyVariantsOfSameMaskingAlgorithm(self, max_algo_variants);
}

BOOST_AUTO_TEST_CASE(RegisterTooManyVariantsOfOther)
{
    const EBlast_filter_program self = eBlast_filter_program_other;
    const size_t max_algo_variants = eBlast_filter_program_max - self;
    RegisterTooManyVariantsOfSameMaskingAlgorithm(self, max_algo_variants);
}

BOOST_AUTO_TEST_CASE(MaskDataColumn)
{
    START;
    
    CSeqDB R("nr", CSeqDB::eProtein);
    CWriteDB W("mask-data-db", CWriteDB::eProtein, "Mask data test");
    const int kNumSeqs = 10;
    
    vector<int> oids;
    int next_oid = 0;
    
    // Get kNumSeqs sequences with length less than 1024
    for(int i = 0; i < kNumSeqs; i++) {
        int L = R.GetSeqLength(next_oid);
        
        while(L < 1024) {
            ++next_oid;
            L = R.GetSeqLength(next_oid);
        }
        
        oids.push_back(next_oid++);
    }
    
    int seg_id = W.RegisterMaskAlgorithm(eBlast_filter_program_seg);
    
    int repeat_id = W.RegisterMaskAlgorithm(eBlast_filter_program_repeat, 
                                            "-species Desmodus_rotundus");
    
    // Populate it.
    
    for(int i = 0; i < kNumSeqs; i++) {
        int oid = oids[i];
        W.AddSequence(*R.GetBioseq(oid));
        
        CMaskedRangesVector ranges;
        
        if (i & 1) {
            ranges.push_back(SBlastDbMaskData());
            ranges.back().algorithm_id = seg_id;
            
            for(int j = 0; j < (i+5); j++) {
                pair<TSeqPos, TSeqPos> rng;
                rng.first = i * 13 + j * 7 + 2;
                rng.second = rng.first + 3 + (i+j) % 11;
                
                ranges.back().offsets.push_back(rng);
            }
        }
        
        if (i & 2) {
            ranges.push_back(SBlastDbMaskData());
            ranges.back().algorithm_id = repeat_id;
            
            for(int j = 0; j < (i+5); j++) {
                pair<TSeqPos, TSeqPos> rng;
                rng.first = i * 10 + j * 5 + 2;
                rng.second = rng.first + 20;
                
                ranges.back().offsets.push_back(rng);
            }
        }
        
        // Set the mask data if either list above was used, or in some
        // cases when neither is.  (Calling SetMaskData() with an
        // empty array should be the same as not calling it at all;
        // this code tests that equivalence.)
        
        if (i & 7) {
            W.SetMaskData(ranges);
        }
    }
    
    // Close the DB.
    
    W.Close();
    
    // Test the resulting files.
    
    // (Currently, the files created here are not tested.  Instead,
    // the SeqDB test uses copies of these files and tests the data
    // integrity via the SeqDB functionality.)
    
    // Clean up.
    
    s_WrapUpDb(W);
}

BOOST_AUTO_TEST_CASE(DuplicateAlgoId)
{
    START;
    
    CWriteDB W("mask-data-db", CWriteDB::eProtein, "Mask data test");
    
    (void)W.RegisterMaskAlgorithm(eBlast_filter_program_seg);
    int seg_repeated_id;
    CHECK_THROW( seg_repeated_id =
                 W.RegisterMaskAlgorithm(eBlast_filter_program_seg),
                 CWriteDBException );
}

BOOST_AUTO_TEST_CASE(TooManyAlgoId)
{
    START;
    
    CWriteDB W("mask-data-db", CWriteDB::eProtein, "Mask data test");
    
    EBlast_filter_program masking_algorithm = eBlast_filter_program_seg;
    vector<int> algo_ids;

    // Ensure that the last one fails
    const size_t kMaxNumSupportedAlgorithmVariants =
        eBlast_filter_program_windowmasker - masking_algorithm;
    for (size_t i = 0; i < kMaxNumSupportedAlgorithmVariants*2; i++) {
        string options( i == 0 ? "" : NStr::IntToString(i));
        int algo_id = -1;
        if (i >= kMaxNumSupportedAlgorithmVariants) {
            CHECK_THROW( 
                algo_id = W.RegisterMaskAlgorithm(masking_algorithm, options),
                CWriteDBException);
        } else {
            algo_id = W.RegisterMaskAlgorithm(masking_algorithm, options);
        }
        if (algo_id != -1) {
            algo_ids.push_back(algo_id);
        }
    }

    // Ensure that the IDs were assigned in increasing order
    CHECK_EQUAL(kMaxNumSupportedAlgorithmVariants, algo_ids.size());
    for (size_t i = 0; i < algo_ids.size(); i++) {
        CHECK_EQUAL((int)(masking_algorithm + i), (int)algo_ids[i]);
    }
}

BOOST_AUTO_TEST_CASE(UndefinedAlgoID)
{
    START;
    
    CSeqDB R("nr", CSeqDB::eProtein);
    CWriteDB W("mask-data-db", CWriteDB::eProtein, "Mask data test");
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_seg);
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_seg, 
                                                 "-species Aotus_vociferans");
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_repeat, 
                                            "-species Desmodus_rotundus");
    
    // Populate it.
    
    int oid = 0;
    
    int L = R.GetSeqLength(oid);
    W.AddSequence(*R.GetBioseq(oid));
    
    CMaskedRangesVector ranges;
    
    ranges.push_back(SBlastDbMaskData());
    ranges.back().algorithm_id = (int)eBlast_filter_program_dust;
    
    pair<TSeqPos, TSeqPos> rng;
    rng.first = L/3;
    rng.second = L;
    
    ranges.back().offsets.push_back(rng);
    
    CHECK_THROW(W.SetMaskData(ranges), CWriteDBException);
    
    W.Close();
    s_WrapUpDb(W);
}

BOOST_AUTO_TEST_CASE(MaskDataBoundsError)
{
    START;
    
    CSeqDB R("nr", CSeqDB::eProtein);
    CWriteDB W("mask-data-db", CWriteDB::eProtein, "Mask data test");
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_seg);
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_seg, 
                                                 "-species Aotus_vociferans");
    
    W.RegisterMaskAlgorithm(eBlast_filter_program_repeat, 
                                            "-species Desmodus_rotundus");
    
    // Populate it.
    
    int oid = 0;
    
    int L = R.GetSeqLength(oid);
    W.AddSequence(*R.GetBioseq(oid));
    
    CMaskedRangesVector ranges;
    
    ranges.push_back(SBlastDbMaskData());
    ranges.back().algorithm_id = (int)eBlast_filter_program_dust;
    
    pair<TSeqPos, TSeqPos> rng;
    rng.first = L/3;
    rng.second = L+1;
    
    ranges.back().offsets.push_back(rng);
    CHECK_THROW(W.SetMaskData(ranges), CWriteDBException);
    
    W.Close();
    s_WrapUpDb(W);
}
#endif

BOOST_AUTO_TEST_CASE(AliasFileGeneration)
{
    CTmpFile tmpfile;
    const string kDbName("nr");
    const string kGiFileName("gifiles.txt");
    const string kTitle("My alias file");
    string kAliasFileName(tmpfile.GetFileName());

    CWriteDB_CreateAliasFile(kAliasFileName, kDbName, CWriteDB::eProtein,
                             kGiFileName, kTitle);
    kAliasFileName += ".pal";
    CFileDeleteAtExit::Add(kAliasFileName);

    BOOST_REQUIRE(CFile(kAliasFileName).Exists());
    ifstream alias_file(kAliasFileName.c_str());

    string line;
    while (getline(alias_file, line)) {
        if (NStr::Find(line, "TITLE") != NPOS) {
            BOOST_REQUIRE(NStr::Find(line, kTitle) != NPOS);
        }
        if (NStr::Find(line, "DBLIST") != NPOS) {
            BOOST_REQUIRE(NStr::Find(line, kDbName) != NPOS);
        }
        if (NStr::Find(line, "GILIST") != NPOS) {
            BOOST_REQUIRE(NStr::Find(line, kGiFileName) != NPOS);
        }
        if (NStr::Find(line, "Alias file created") != NPOS) {
            // this should be enough granularity
            const string kCurrentYear = 
                NStr::IntToString(CTime(CTime::eCurrent).Year());
            BOOST_REQUIRE(NStr::Find(line, kCurrentYear) != NPOS);
        }
    }
}

#endif /* SKIP_DOXYGEN_PROCESSING */
