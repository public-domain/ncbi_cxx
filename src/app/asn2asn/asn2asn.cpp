/*  $Id: asn2asn.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
*   asn2asn test program
*/

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbienv.hpp>
#include <corelib/ncbithr.hpp>
#include <corelib/ncbimempool.hpp>
#include <objects/seqset/Seq_entry.hpp>
#include <objects/seqset/Bioseq_set.hpp>
#include <objects/seq/Seq_descr.hpp>
#include <objects/seq/Seqdesc.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/general/Dbtag.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <memory>

BEGIN_NCBI_SCOPE

// CSEQ_ENTRY_REF_CHOICE macro to switch implementation of CSeq_entry choice
// as choice class or virtual base class.
// 0 -> generated choice class
// 1 -> virtual base class
#define CSEQ_ENTRY_REF_CHOICE 0

#if CSEQ_ENTRY_REF_CHOICE

template<typename T> const CTypeInfo* (*GetTypeRef(const T* object))(void);
template<typename T> pair<void*, const CTypeInfo*> ObjectInfo(T& object);
template<typename T> pair<const void*, const CTypeInfo*> ConstObjectInfo(const T& object);

EMPTY_TEMPLATE
inline
const CTypeInfo* (*GetTypeRef< CRef<NCBI_NS_NCBI::objects::CSeq_entry> >(const CRef<NCBI_NS_NCBI::objects::CSeq_entry>* object))(void)
{
    return &NCBI_NS_NCBI::objects::CSeq_entry::GetRefChoiceTypeInfo;
}
EMPTY_TEMPLATE
inline
pair<void*, const CTypeInfo*> ObjectInfo< CRef<NCBI_NS_NCBI::objects::CSeq_entry> >(CRef<NCBI_NS_NCBI::objects::CSeq_entry>& object)
{
    return make_pair((void*)&object, GetTypeRef(&object)());
}
EMPTY_TEMPLATE
inline
pair<const void*, const CTypeInfo*> ConstObjectInfo< CRef<NCBI_NS_NCBI::objects::CSeq_entry> >(const CRef<NCBI_NS_NCBI::objects::CSeq_entry>& object)
{
    return make_pair((const void*)&object, GetTypeRef(&object)());
}

#endif

END_NCBI_SCOPE

#include "asn2asn.hpp"
#include <corelib/ncbiutil.hpp>
#include <corelib/ncbiargs.hpp>
#include <objects/seq/Bioseq.hpp>
#include <serial/objectio.hpp>
#include <serial/objistr.hpp>
#include <serial/objostr.hpp>
#include <serial/objcopy.hpp>
#include <serial/serial.hpp>
#include <serial/objhook.hpp>
#include <serial/iterator.hpp>

USING_NCBI_SCOPE;

using namespace NCBI_NS_NCBI::objects;

#if CSEQ_ENTRY_REF_CHOICE
typedef CRef<CSeq_entry> TSeqEntry;
#else
typedef CSeq_entry TSeqEntry;
#endif

int main(int argc, char** argv)
{
    return CAsn2Asn().AppMain(argc, argv, 0, eDS_Default, 0, "asn2asn");
}

static
void SeqEntryProcess(CSeq_entry& entry);  /* dummy function */

#if CSEQ_ENTRY_REF_CHOICE
static
void SeqEntryProcess(CRef<CSeq_entry>& entry)
{
    SeqEntryProcess(*entry);
}
#endif

class CCounter
{
public:
    CCounter(void)
        : m_Counter(0)
        {
        }
    ~CCounter(void)
        {
            _ASSERT(m_Counter == 0);
        }

    operator int(void) const
        {
            return m_Counter;
        }
private:
    friend class CInc;
    int m_Counter;
};

class CInc
{
public:
    CInc(CCounter& counter)
        : m_Counter(counter)
        {
            ++counter.m_Counter;
        }
    ~CInc(void)
        {
            --m_Counter.m_Counter;
        }
private:
    CCounter& m_Counter;
};

class CReadSeqSetHook : public CReadClassMemberHook
{
public:
    void ReadClassMember(CObjectIStream& in,
                         const CObjectInfo::CMemberIterator& member);

    CCounter m_Level;
};

class CWriteSeqSetHook : public CWriteClassMemberHook
{
public:
    void WriteClassMember(CObjectOStream& out,
                          const CConstObjectInfo::CMemberIterator& member);

    CCounter m_Level;
};

class CWriteSeqEntryHook : public CWriteObjectHook
{
public:
    void WriteObject(CObjectOStream& out, const CConstObjectInfo& object);

    CCounter m_Level;
};


template<typename Member>
class CReadInSkipClassMemberHook : public CSkipClassMemberHook
{
public:
    typedef Member TObject;
    CReadInSkipClassMemberHook() { object = TObject(); }
    
    void SkipClassMember(CObjectIStream& in, const CObjectTypeInfoMI& member)
        {
            _ASSERT((*member).GetTypeInfo()->GetSize() == sizeof(object));
            CObjectInfo info(&object, (*member).GetTypeInfo());
            in.ReadObject(info);
            NcbiCout << "Skipped class member: " <<
                member.GetClassType().GetTypeInfo()->GetName() << '.' <<
                member.GetMemberInfo()->GetId().GetName() << ": " <<
                MSerial_AsnText << info << NcbiEndl;
        }
    
private:
    TObject object;
};

template<typename Object>
class CReadInSkipObjectHook : public CSkipObjectHook
{
public:
    typedef Object TObject;

    CReadInSkipObjectHook() : object() {}

    void SkipObject(CObjectIStream& in, const CObjectTypeInfo& type)
        {
            _ASSERT(type.GetTypeInfo()->GetSize() == sizeof(object));
            CObjectInfo info(&object, type.GetTypeInfo());
            in.ReadObject(info);

            const CSeq_descr& descr = object;
            string title;
            string genome;
            int subtype;
            string taxname;
            string taxid;
            int biomol;
            int complete;
            int tech;

            NcbiCout << "Skipped object: " <<
                type.GetTypeInfo()->GetName() << ": " <<
                MSerial_AsnText << info << NcbiEndl;
            ITERATE(list< CRef< CSeqdesc > >,it,descr.Get()) {
                CConstRef<CSeqdesc> desc = *it;
                if (desc->IsTitle()) {
                    title = desc->GetTitle();
                    string lt = NStr::ToLower(title); 
                }
                else if (desc->IsSource()) { 
                    const CBioSource & source = desc->GetSource();
                    if (source.IsSetGenome())
                        genome = source.GetGenome();					
                    if (source.IsSetSubtype()) {
                        ITERATE(list< CRef< CSubSource > >,it,source.GetSubtype()) {
                            string name = (*it)->GetName();
                            int st = (*it)->GetSubtype();
                            if (st == 19) subtype = st;
                        }
                    }
                    if (source.IsSetOrg()) {
                        const COrg_ref & orgref= source.GetOrg();
                        taxname = orgref.GetTaxname();
                        ITERATE(vector< CRef< CDbtag > > ,it,orgref.GetDb()) {
                            if (CDbtag::eDbtagType_taxon == (*it)->GetType()) {
                                taxid = (*it)->GetTag().GetId();
                                break;
                            }
                        }
                    }
                }
                else if (desc->IsMolinfo()) {
                    const CMolInfo & molinfo = desc->GetMolinfo();
                    if  (molinfo.IsSetBiomol())
                        biomol = molinfo.GetBiomol();
                    if (molinfo.IsSetCompleteness())
                        complete = molinfo.GetCompleteness();
                    if (molinfo.IsSetTech())
                        tech = molinfo.GetTech();
                }
            }
        }

private:
    TObject object;
};


/*****************************************************************************
*
*   Main program loop to read, process, write SeqEntrys
*
*****************************************************************************/
void CAsn2Asn::Init(void)
{
    auto_ptr<CArgDescriptions> d(new CArgDescriptions);

    d->SetUsageContext("asn2asn", "convert Seq-entry or Bioseq-set data");

    d->AddKey("i", "inputFile",
              "input data file",
              CArgDescriptions::eInputFile);
    d->AddOptionalKey("o", "outputFile",
                      "output data file",
                      CArgDescriptions::eOutputFile);
    d->AddFlag("e",
               "treat data as Seq-entry");
    d->AddFlag("b",
               "binary ASN.1 input format");
    d->AddFlag("X",
               "XML input format");
    d->AddFlag("s",
               "binary ASN.1 output format");
    d->AddFlag("x",
               "XML output format");
    d->AddFlag("C",
               "Convert data without reading in memory");
    d->AddFlag("S",
               "Skip data without reading in memory");
    d->AddFlag("P",
               "Use memory pool for deserialization");
    d->AddOptionalKey("l", "logFile",
                      "log errors to <logFile>",
                      CArgDescriptions::eOutputFile);
    d->AddDefaultKey("c", "count",
                      "perform command <count> times",
                      CArgDescriptions::eInteger, "1");
    d->AddDefaultKey("tc", "threadCount",
                      "perform command in <threadCount> thread",
                      CArgDescriptions::eInteger, "1");
    d->AddFlag("m",
               "Input file contains multiple objects");
    
    d->AddFlag("ih",
               "Use read hooks");
    d->AddFlag("oh",
               "Use write hooks");

    d->AddFlag("q",
               "Quiet execution");

    SetupArgDescriptions(d.release());
}

class CAsn2AsnThread : public CThread
{
public:
    CAsn2AsnThread(int index, CAsn2Asn* asn2asn)
        : m_Index(index), m_Asn2Asn(asn2asn), m_DoneOk(false)
    {
    }

    void* Main()
    {
        string suffix = '.'+NStr::IntToString(m_Index);
        try {
            m_Asn2Asn->RunAsn2Asn(suffix);
        }
        catch (exception& e) {
            CNcbiDiag() << Error << "[asn2asn thread " << m_Index << "]" <<
                "Exception: " << e.what();
            return 0;
        }
        m_DoneOk = true;
        return 0;
    }

    bool DoneOk() const
    {
        return m_DoneOk;
    }

private:
    int m_Index;
    CAsn2Asn* m_Asn2Asn;
    bool m_DoneOk;
};

int CAsn2Asn::Run(void)
{
	SetDiagPostLevel(eDiag_Error);

    const CArgs& args = GetArgs();

    if ( const CArgValue& l = args["l"] )
        SetDiagStream(&l.AsOutputFile());


    size_t threadCount = args["tc"].AsInteger();
    vector< CRef<CAsn2AsnThread> > threads(threadCount);
    for ( size_t i = 1; i < threadCount; ++i ) {
        threads[i] = new CAsn2AsnThread(i, this);
        threads[i]->Run();
    }
    
    try {
      RunAsn2Asn("");
    }
    catch (exception& e) {
        CNcbiDiag() << Error << "[asn2asn]" << "Exception: " << e.what();
        return 1;
    }

    for ( size_t i = 1; i < threadCount; ++i ) {
        threads[i]->Join();
        if ( !threads[i]->DoneOk() ) {
            NcbiCerr << "Error in thread: " << i << NcbiEndl;
            return 1;
        }
    }

    return 0;
}

DEFINE_STATIC_FAST_MUTEX(s_ArgsMutex);

void CAsn2Asn::RunAsn2Asn(const string& outFileSuffix)
{
    CFastMutexGuard GUARD(s_ArgsMutex);

    const CArgs& args = GetArgs();

    string inFile = args["i"].AsString();
    ESerialDataFormat inFormat = eSerial_AsnText;
    if ( args["b"] )
        inFormat = eSerial_AsnBinary;
    else if ( args["X"] )
        inFormat = eSerial_Xml;

    const CArgValue& o = args["o"];
    bool haveOutput = o;
    string outFile;
    ESerialDataFormat outFormat = eSerial_AsnText;
    if ( haveOutput ) {
        outFile = o.AsString();
        if ( args["s"] )
            outFormat = eSerial_AsnBinary;
        else if ( args["x"] )
            outFormat = eSerial_Xml;
    }
    outFile += outFileSuffix;

    bool inSeqEntry = args["e"];
    bool skip = args["S"];
    bool convert = args["C"];
    bool readHook = args["ih"];
    bool writeHook = args["oh"];
    bool usePool = args["P"];

    bool quiet = args["q"];
    bool multi = args["m"];

    size_t count = args["c"].AsInteger();

    GUARD.Release();
    
    for ( size_t i = 1; i <= count; ++i ) {
        bool displayMessages = count != 1 && !quiet;
        if ( displayMessages )
            NcbiCerr << "Step " << i << ':' << NcbiEndl;
        auto_ptr<CObjectIStream> in(CObjectIStream::Open(inFormat, inFile,
                                                         eSerial_StdWhenAny));
        if ( usePool ) {
            in->UseMemoryPool();
        }
        auto_ptr<CObjectOStream> out(!haveOutput? 0:
                                     CObjectOStream::Open(outFormat, outFile,
                                                          eSerial_StdWhenAny));

        for ( ;; ) {
            if ( inSeqEntry ) { /* read one Seq-entry */
                if ( skip ) {
                    if ( displayMessages )
                        NcbiCerr << "Skipping Seq-entry..." << NcbiEndl;
                    if ( readHook ) {
                        {{
                            CObjectTypeInfo type = CType<CSeq_descr>();
                            type.SetLocalSkipHook
                                (*in, new CReadInSkipObjectHook<CSeq_descr>);
                        }}
                        {{
                            CObjectTypeInfo type = CType<CBioseq_set>();
                            type.FindMember("class").SetLocalSkipHook
                                (*in, new CReadInSkipClassMemberHook<CBioseq_set::TClass>);
                        }}
                        {{
                            CObjectTypeInfo type = CType<CSeq_inst>();
                            type.FindMember("topology").SetLocalSkipHook
                                (*in, new CReadInSkipClassMemberHook<CSeq_inst::TTopology>);
                        }}
                        in->Skip(CType<CSeq_entry>());
                    }
                    else {
                        in->Skip(CType<CSeq_entry>());
                    }
                }
                else if ( convert && haveOutput ) {
                    if ( displayMessages )
                        NcbiCerr << "Copying Seq-entry..." << NcbiEndl;
                    CObjectStreamCopier copier(*in, *out);
                    copier.Copy(CType<CSeq_entry>());
                }
                else {
                    TSeqEntry entry;
                    //entry.DoNotDeleteThisObject();
                    if ( displayMessages )
                        NcbiCerr << "Reading Seq-entry..." << NcbiEndl;
                    *in >> entry;
                    SeqEntryProcess(entry);     /* do any processing */
                    if ( haveOutput ) {
                        if ( displayMessages )
                            NcbiCerr << "Writing Seq-entry..." << NcbiEndl;
                        *out << entry;
                    }
                }
            }
            else {              /* read Seq-entry's from a Bioseq-set */
                if ( skip ) {
                    if ( displayMessages )
                        NcbiCerr << "Skipping Bioseq-set..." << NcbiEndl;
                    in->Skip(CType<CBioseq_set>());
                }
                else if ( convert && haveOutput ) {
                    if ( displayMessages )
                        NcbiCerr << "Copying Bioseq-set..." << NcbiEndl;
                    CObjectStreamCopier copier(*in, *out);
                    copier.Copy(CType<CBioseq_set>());
                }
                else {
                    CBioseq_set entries;
                    //entries.DoNotDeleteThisObject();
                    if ( displayMessages )
                        NcbiCerr << "Reading Bioseq-set..." << NcbiEndl;
                    if ( readHook ) {
                        CObjectTypeInfo bioseqSetType = CType<CBioseq_set>();
                        bioseqSetType.FindMember("seq-set")
                            .SetLocalReadHook(*in, new CReadSeqSetHook);
                        *in >> entries;
                    }
                    else {
                        *in >> entries;
                        NON_CONST_ITERATE ( CBioseq_set::TSeq_set, seqi,
                                            entries.SetSeq_set() ) {
                            SeqEntryProcess(**seqi);     /* do any processing */
                        }
                    }
                    if ( haveOutput ) {
                        if ( displayMessages )
                            NcbiCerr << "Writing Bioseq-set..." << NcbiEndl;
                        if ( writeHook ) {
#if 0
                            CObjectTypeInfo bioseqSetType = CType<CBioseq_set>();
                            bioseqSetType.FindMember("seq-set")
                                .SetLocalWriteHook(*out, new CWriteSeqSetHook);
#else
                            CObjectTypeInfo seqEntryType = CType<CSeq_entry>();
                            seqEntryType
                                .SetLocalWriteHook(*out, new CWriteSeqEntryHook);
#endif
                            *out << entries;
                        }
                        else {
                            *out << entries;
                        }
                    }
                }
            }
            if ( !multi || in->EndOfData() )
                break;
        }
    }
}


/*****************************************************************************
*
*   void SeqEntryProcess (sep)
*      just a dummy routine that does nothing
*
*****************************************************************************/
static
void SeqEntryProcess(CSeq_entry& /* seqEntry */)
{
}

void CReadSeqSetHook::ReadClassMember(CObjectIStream& in,
                                      const CObjectInfo::CMemberIterator& member)
{
    CInc inc(m_Level);
    if ( m_Level == 1 ) {
        // (do not have to read member open/close tag, it's done by this time)

        // Read each element separately to a local TSeqEntry,
        // process it somehow, and... not store it in the container.
        for ( CIStreamContainerIterator i(in, member); i; ++i ) {
            TSeqEntry entry;
            //entry.DoNotDeleteThisObject();
            i >> entry;
            SeqEntryProcess(entry);
        }

        // MemberIterators are DANGEROUS!  -- do not use the following
        // unless...

        // The same trick can be done with CIStreamClassMember -- to traverse
        // through the class members rather than container elements.
        // CObjectInfo object = member;
        // for ( CIStreamClassMemberIterator i(in, object); i; ++i ) {
        //    // CObjectTypeInfo::CMemberIterator nextMember = *i;
        //    in.ReadObject(object.GetMember(*i));
        // }
    }
    else {
        // standard read
        in.ReadClassMember(member);
    }
}

void CWriteSeqEntryHook::WriteObject(CObjectOStream& out,
                                     const CConstObjectInfo& object)
{
    CInc inc(m_Level);
    if ( m_Level == 1 ) {
        NcbiCerr << "entry" << NcbiEndl;
        // const CSeq_entry& entry = *CType<CSeq_entry>::Get(object);
        object.GetTypeInfo()->DefaultWriteData(out, object.GetObjectPtr());
    }
    else {
        // const CSeq_entry& entry = *CType<CSeq_entry>::Get(object);
        object.GetTypeInfo()->DefaultWriteData(out, object.GetObjectPtr());
    }
}

void CWriteSeqSetHook::WriteClassMember(CObjectOStream& out,
                                        const CConstObjectInfo::CMemberIterator& member)
{
    // keep track of the level of write recursion
    CInc inc(m_Level);

    // just for fun -- do the hook only on the first level of write recursion,
    if ( m_Level == 1 ) {
        // provide opening and closing(automagic, in destr) tags for the member
        COStreamClassMember m(out, member);

        // out.WriteObject(*member);  or, with just the same effect:

        // provide opening and closing(automagic) tags for the container
        COStreamContainer o(out, member);

        typedef CBioseq_set::TSeq_set TSeq_set;
        // const TSeq_set& cnt = *CType<TSeq_set>::Get(*member);
        // but as soon as we know for sure that it *is* TSeq_set, so:
        const TSeq_set& cnt = *CType<TSeq_set>::GetUnchecked(*member);

        // write elem-by-elem
        for ( TSeq_set::const_iterator i = cnt.begin(); i != cnt.end(); ++i ) {
            const TSeqEntry& entry = **i;
            // COStreamContainer is smart enough to automagically put
            // open/close tags for each element written
            o << entry;

            // here, we could e.g. write each elem twice:
            // o << entry;  o << entry;
            // we cannot change the element content as everything is const in
            // the write hooks.
        }
    }
    else {
        // on all other levels -- use standard write func for this member
        out.WriteClassMember(member);
    }
}
