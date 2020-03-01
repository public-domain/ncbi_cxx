/* $Id: Genetic_code_table.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 * Author:  .......
 *
 * File Description:
 *   .......
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the ASN data definition file
 *   'seqfeat.asn'.
 */

// standard includes
#include <ncbi_pch.hpp>
#include <corelib/ncbimtx.hpp>
#include <serial/serial.hpp>
#include <serial/objistr.hpp>

// generated includes
#include <objects/seqfeat/Genetic_code_table.hpp>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CGenetic_code_table::~CGenetic_code_table(void)
{
}

// genetic code translation tables

// destructor
CTrans_table::~CTrans_table(void)
{
}

// translation finite state machine base codes - ncbi4na
enum EBaseCode {
    eBase_gap = 0,
    eBase_A,      /* A    */
    eBase_C,      /* C    */
    eBase_M,      /* AC   */
    eBase_G,      /* G    */
    eBase_R,      /* AG   */
    eBase_S,      /* CG   */
    eBase_V,      /* ACG  */
    eBase_T,      /* T    */
    eBase_W,      /* AT   */
    eBase_Y,      /* CT   */
    eBase_H,      /* ACT  */
    eBase_K,      /* GT   */
    eBase_D,      /* AGT  */
    eBase_B,      /* CGT  */
    eBase_N       /* ACGT */
};

// static instances of single copy translation tables common to all genetic codes
int   CTrans_table::sm_NextState  [4097];
int   CTrans_table::sm_RvCmpState [4097];
int   CTrans_table::sm_BaseToIdx  [256];

// initialize base conversion, next state, and reverse complement state tables
void CTrans_table::x_InitFsaTable (void)
{
    char         ch;
    int          i, j, k, p, q, r, nx, st;
    static char  charToBase [17] = "-ACMGRSVTWYHKDBN";
    static char  baseToComp [17] = "-TGKCYSBAWRDMHVN";

    // illegal characters map to 0
    for (i = 0; i < 256; i++) {
        sm_BaseToIdx [i] = 0;
    }

    // map iupacna alphabet to EBaseCode
    for (i = eBase_gap; i <= eBase_N; i++) {
        ch = charToBase [i];
        sm_BaseToIdx [(int) ch] = i;
        ch = tolower ((unsigned char) ch);
        sm_BaseToIdx [(int) ch] = i;
    }
    sm_BaseToIdx [(int) 'U'] = eBase_T;
    sm_BaseToIdx [(int) 'u'] = eBase_T;
    sm_BaseToIdx [(int) 'X'] = eBase_N;
    sm_BaseToIdx [(int) 'x'] = eBase_N;

    // also map ncbi4na alphabet to EBaseCode
    for (i = eBase_gap; i <= eBase_N; i++) {
        sm_BaseToIdx [(int) i] = i;
    }

    // treat state 0 as already having seen NN,
    // avoiding single and double letter states
    sm_NextState [0] = 4081;
    sm_RvCmpState [0] = 4096;

    // states 1 through 4096 are triple letter states (---, --A, ..., NNT, NNN)
    for (i = eBase_gap, st = 1; i <= eBase_N; i++) {
        for (j = eBase_gap, nx = 1; j <= eBase_N; j++) {
            for (k = eBase_gap; k <= eBase_N; k++, st++, nx += 16) {
                sm_NextState [st] = nx;
                p = sm_BaseToIdx [(int) (Uint1) baseToComp [k]];
                q = sm_BaseToIdx [(int) (Uint1) baseToComp [j]];
                r = sm_BaseToIdx [(int) (Uint1) baseToComp [i]];
                sm_RvCmpState [st] = 256 * p + 16 * q + r + 1;
            }
        }
    }
}

// initialize genetic code specific translation tables
void CTrans_table::x_InitFsaTransl (const string *ncbieaa,
                                    const string *sncbieaa) const
{
    char        ch, aa, orf;
    bool        go_on;
    int         i, j, k, p, q, r, x, y, z, st, cd;
    static int  expansions [4] = {eBase_A, eBase_C, eBase_G, eBase_T};
                                // T = 0, C = 1, A = 2, G = 3
    static int  codonIdx [9] = {0, 2, 1, 0, 3, 0, 0, 0, 0};

    // return if unable to find ncbieaa and sncbieaa strings
    if (ncbieaa == 0 || sncbieaa == 0) return;

    // also check length of ncbieaa and sncbieaa strings
    if (ncbieaa->size () != 64 || sncbieaa->size () != 64) return;

    // ambiguous codons map to unknown amino acid or not start
    for (i = 0; i <= 4096; i++) {
        m_AminoAcid [i] = 'X';
        m_OrfStart [i] = '-';
    }

    // lookup amino acid for each codon in genetic code table
    for (i = eBase_gap, st = 1; i <= eBase_N; i++) {
        for (j = eBase_gap; j <= eBase_N; j++) {
            for (k = eBase_gap; k <= eBase_N; k++, st++) {
                aa = '\0';
                orf = '\0';
                go_on = true;

                // expand ambiguous IJK nucleotide symbols into component bases XYZ
                for (p = 0; p < 4 && go_on; p++) {
                    x = expansions [p];
                    if ((x & i) != 0) {
                        for (q = 0; q < 4 && go_on; q++) {
                            y = expansions [q];
                            if ((y & j) != 0) {
                                for (r = 0; r < 4 && go_on; r++) {
                                    z = expansions [r];
                                    if ((z & k) != 0) {

                                        // calculate offset in genetic code string

                                        // the T = 0, C = 1, A = 2, G = 3 order is
                                        // necessary because the genetic code strings
                                        // are presented in TCAG order in printed tables
                                        // and in the genetic code strings
                                        cd = 16 * codonIdx [x] + 4 * codonIdx [y] + codonIdx [z];

                                        // lookup amino acid for codon XYZ
                                        ch = (*ncbieaa) [cd];
                                        if (aa == '\0') {
                                            aa = ch;
                                        } else if (aa != ch) {
                                            // allow Asx (Asp or Asn) and Glx (Glu or Gln)
                                            if ((aa == 'B' || aa == 'D' || aa == 'N') &&
                                                (ch == 'D' || ch == 'N')) {
                                                aa = 'B';
                                            } else if ((aa == 'Z' || aa == 'E' || aa == 'Q') &&
                                                       (ch == 'E' || ch == 'Q')) {
                                                aa = 'Z';
                                            } else {
                                                aa = 'X';
                                            }
                                        }

                                        // lookup translation start flag
                                        ch = (*sncbieaa) [cd];
                                        if (orf == '\0') {
                                            orf = ch;
                                        } else if (orf != ch) {
                                            orf = 'X';
                                        }

                                        // drop out of loop as soon as answer is known
                                        if (aa == 'X' && orf == 'X') {
                                            go_on = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // assign amino acid and orf start
                if (aa != '\0') {
                    m_AminoAcid [st] = aa;
                }
                if (orf != '\0') {
                    m_OrfStart [st] = orf;
                }
            }
        }
    }
}

// implementation class for genetic code table and translation tables

class CGen_code_table_imp : public CObject
{
public:
    // constructors (the default uses sm_GenCodeTblMemStr)
    CGen_code_table_imp(void);
    CGen_code_table_imp(CObjectIStream& ois);
    // destructor
    ~CGen_code_table_imp(void);

    // return initialized translation table given genetic code
    const CTrans_table& GetTransTable (int  gc);
    const CTrans_table& GetTransTable (const CGenetic_code& gc);

    // return single copy loaded genetic code table for iteration
    const CGenetic_code_table& GetCodeTable (void);

    const string& GetNcbieaa(int id) const;
    const string& GetNcbieaa(const CGenetic_code& gc) const;
    
    const string& GetSncbieaa(int id) const;
    const string& GetSncbieaa(const CGenetic_code& gc) const;

private:
    // genetic code table data
    CRef <CGenetic_code_table> m_GcTable;

    // typedefs
    typedef vector< CRef< CTrans_table > > TTransTablesById;

    // translation tables
    TTransTablesById  m_TransTablesById;

    // local copy of genetic code table ASN.1
    static const char * sm_GenCodeTblMemStr [];
};

// single instance of implementation class is initialized before Main
AutoPtr<CGen_code_table_imp> CGen_code_table::sm_Implementation;
DEFINE_STATIC_FAST_MUTEX(s_ImplementationMutex);

void CGen_code_table::x_InitImplementation()
{
    CFastMutexGuard LOCK(s_ImplementationMutex);
    if ( !sm_Implementation.get() ) {
        sm_Implementation.reset(new CGen_code_table_imp());
    }
}


// public access functions

const CTrans_table& CGen_code_table::GetTransTable (int id)
{
    return x_GetImplementation().GetTransTable (id);
}


const CTrans_table& CGen_code_table::GetTransTable(const CGenetic_code& gc)
{
    return x_GetImplementation().GetTransTable(gc);
}


const CGenetic_code_table& CGen_code_table::GetCodeTable(void)
{
    return x_GetImplementation().GetCodeTable();
}


const string& CGen_code_table::GetNcbieaa(int id)
{
    return x_GetImplementation().GetNcbieaa(id);
}


const string& CGen_code_table::GetNcbieaa(const CGenetic_code& gc)
{
    return x_GetImplementation().GetNcbieaa(gc);
}


const string& CGen_code_table::GetSncbieaa(int id) 
{
    return x_GetImplementation().GetSncbieaa(id);
}


const string& CGen_code_table::GetSncbieaa(const CGenetic_code& gc)
{
    return x_GetImplementation().GetSncbieaa(gc);
}


string CGen_code_table::IndexToCodon(int index)
{
    if ( index < 0 || index > 63 ) return CNcbiEmptyString::Get();

    static char na[4] = { 'T', 'C', 'A', 'G' };    
    string codon;
    codon.resize(3);
    int total = index;
    int div = 16;
    for ( int i = 0; i < 3; ++i ) {
        int j = total / div;
        codon[i] = na[j];
        total -= div * j;
        div /= 4;
    }

    return codon;
}


int CGen_code_table::CodonToIndex(char base1, char base2, char base3)
{
    string codon;
    codon.insert(codon.end(), base1);
    codon.insert(codon.end(), base2);
    codon.insert(codon.end(), base3);

    return CodonToIndex(codon);
}


void CGen_code_table::LoadTransTable(CObjectIStream& ois)
{
    CFastMutexGuard LOCK(s_ImplementationMutex);
    sm_Implementation.reset(new CGen_code_table_imp(ois));
}


void CGen_code_table::LoadTransTable(const string& path,
                                     ESerialDataFormat format)
{
    auto_ptr<CObjectIStream> ois(CObjectIStream::Open(path, format));
    LoadTransTable(*ois);
}


static bool s_ValidCodon(const string& codon) 
{
    if ( codon.length() != 3 ) return false;
    
    for ( int i = 0; i < 3; ++i ) {
        char ch = toupper((unsigned char) codon[i]);
        if ( ch != 'A' && ch != 'G' && ch != 'C' && ch != 'T'  && ch != 'U' ) {
            return false;
        }
    }
    return true;
}


int CGen_code_table::CodonToIndex(const string& codon)
{
    if ( !s_ValidCodon(codon) ) return -1;
    
    int weight = 0;
    int index = 0;
    int mul = 16;
    for ( int i = 0; i < 3; ++i ) {
        switch ( toupper((unsigned char) codon[i]) ) {
        case 'A' :
            weight = 2;
            break;
        case 'C' :
            weight = 1;
            break;
        case 'G' :
            weight = 3;
            break;
        case 'T' :
        case 'U' :
            weight = 0;
            break;
        }

        index += mul * weight;
        mul /= 4;
    }

    return index;
}



// constructors
CGen_code_table_imp::CGen_code_table_imp(void)
{
    // initialize common CTrans_table tables
    CTrans_table::x_InitFsaTable ();

    // Compose a long-long string
    string str;
    for (size_t i = 0;  sm_GenCodeTblMemStr [i];  i++) {
        str += sm_GenCodeTblMemStr [i];
    }

    // create an in memory stream on sm_GenCodeTblMemStr
    CNcbiIstrstream is(str.c_str(), str.length());
    auto_ptr<CObjectIStream>
        asn_codes_in(CObjectIStream::Open(eSerial_AsnText, is));

    // read single copy of genetic-code table
    m_GcTable = new CGenetic_code_table;
    *asn_codes_in >> *m_GcTable;
}

CGen_code_table_imp::CGen_code_table_imp(CObjectIStream& ois)
{
    if ( !CTrans_table::sm_NextState[0] ) {
        CTrans_table::x_InitFsaTable ();
    }

    // read single copy of genetic-code table
    m_GcTable = new CGenetic_code_table;
    ois >> *m_GcTable;
    // perform additional sanity checks?
}

// destructor
CGen_code_table_imp::~CGen_code_table_imp(void)
{
}

// constructor
CTrans_table::CTrans_table(const CGenetic_code& gc)
{
    const string * ncbieaa  = 0;
    const string * sncbieaa = 0;

    // find amino acid and orf start strings given genetic code instance
    ITERATE (CGenetic_code::Tdata, gcd, gc.Get ()) {
        switch ((*gcd)->Which ()) {
            case CGenetic_code::C_E::e_Ncbieaa :
                ncbieaa = & (*gcd)->GetNcbieaa ();
                break;
            case CGenetic_code::C_E::e_Sncbieaa :
                sncbieaa = & (*gcd)->GetSncbieaa ();
                break;
            default:
                break;
        }
    }

    // throw exception if unable to find ncbieaa and sncbieaa strings
    if (ncbieaa == 0 || sncbieaa == 0) {
        NCBI_THROW (CException, eUnknown,
                    "Could not find ncbieaa and sncbieaa");
    }

    // initialize translation table for this genetic code instance
    x_InitFsaTransl (ncbieaa, sncbieaa);
}

const CTrans_table& CGen_code_table_imp::GetTransTable (int id)
{
    _ASSERT(id >= 0);

    // look for already created translation table
    if ( size_t(id) < m_TransTablesById.size ()) {
        CRef< CTrans_table> tbl = m_TransTablesById [id];
        if (tbl != 0) {
        	// already in list, already initialized, so return
            return *tbl;
        }
    }

    // this mutex is automatically freed when the function exits
    DEFINE_STATIC_FAST_MUTEX(mtx);
    CFastMutexGuard   LOCK (mtx);

    // test again within mutex lock to see if another thread was just adding it
    if ( size_t(id) < m_TransTablesById.size ()) {
        CRef< CTrans_table> tbl = m_TransTablesById [id];
        if (tbl != 0) {
        	// already in list, already initialized, so return
            return *tbl;
        }
    }

    // now look for the genetic code and initialize the translation table
    ITERATE (CGenetic_code_table::Tdata, gcl, m_GcTable->Get ()) {
        ITERATE (CGenetic_code::Tdata, gcd, (*gcl)->Get ()) {
            if ((*gcd)->IsId ()  &&  (*gcd)->GetId () == id) {

        	    // found proper genetic code, so create new trans table
        	    CRef< CTrans_table> tbl(new CTrans_table (**gcl));

        	    // extend size of translation table list, if necessary
        	    if ( size_t(id) >= m_TransTablesById.size ()) {
        	        m_TransTablesById.resize (id + 1);
        	    }

        	    // add new table to list of translation tables
        	    m_TransTablesById [id] = tbl;

        	    return *tbl;
            }
        }
    }

    // throw exception if failure
    NCBI_THROW (CException, eUnknown,
                "Unable to find genetic code number " +
                NStr::IntToString (id));
}

const CTrans_table& CGen_code_table_imp::GetTransTable (const CGenetic_code& gc)
{
    const string * ncbieaa  = 0;
    const string * sncbieaa = 0;

    ITERATE (CGenetic_code::Tdata, gcd, gc.Get ()) {
        switch ((*gcd)->Which ()) {
            case CGenetic_code::C_E::e_Id :
            {
                // lookup table by ID
                int id = (*gcd)->GetId ();
                return GetTransTable (id);
            }
            case CGenetic_code::C_E::e_Ncbieaa :
                ncbieaa = & (*gcd)->GetNcbieaa ();
                break;
            case CGenetic_code::C_E::e_Sncbieaa :
                sncbieaa = & (*gcd)->GetSncbieaa ();
                break;
            default:
                break;
        }
    }

    if (ncbieaa != 0  &&  sncbieaa != 0) {
      // return * new CTrans_table (gc);

      NCBI_THROW (CException, eUnknown,
                  "GetTransTable without ID not yet supported");
    }

    NCBI_THROW (CException, eUnknown,
                "GetTransTable does not have sufficient information");
}

const CGenetic_code_table & CGen_code_table_imp::GetCodeTable (void)
{
    return *m_GcTable;
}


const string& CGen_code_table_imp::GetNcbieaa(int id) const
{
    ITERATE (CGenetic_code_table::Tdata, gcl, m_GcTable->Get ()) {
        if ( (*gcl)->GetId() == id ) {
            return (*gcl)->GetNcbieaa();
        }
    }
    return CNcbiEmptyString::Get();
}


const string& CGen_code_table_imp::GetNcbieaa(const CGenetic_code& gc) const
{
    return gc.GetNcbieaa();
}


const string& CGen_code_table_imp::GetSncbieaa(int id) const
{
    ITERATE (CGenetic_code_table::Tdata, gcl, m_GcTable->Get ()) {
        if ( (*gcl)->GetId() == id ) {
            return (*gcl)->GetSncbieaa();
        }
    }
    
    return CNcbiEmptyString::Get();
}


const string& CGen_code_table_imp::GetSncbieaa(const CGenetic_code& gc) const
{
    return gc.GetSncbieaa();
}


// standard genetic code
//
// ncbieaa  "FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG"
// sncbieaa "---M---------------M---------------M----------------------------"
//
// -- Base1  TTTTTTTTTTTTTTTTCCCCCCCCCCCCCCCCAAAAAAAAAAAAAAAAGGGGGGGGGGGGGGGG
// -- Base2  TTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGGTTTTCCCCAAAAGGGG
// -- Base3  TCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAGTCAG

/*
                         Second Position
First      T             C             A             G      Third
-----------------------------------------------------------------
  T   TTT Phe [F]   TCT Ser [S]   TAT Tyr [Y]   TGT Cys [C]   T
      TTC Phe [F]   TCC Ser [S]   TAC Tyr [Y]   TGC Cys [C]   C
      TTA Leu [L]   TCA Ser [S]   TAA Ter [*]   TGA Ter [*]   A
      TTG Leu [L]   TCG Ser [S]   TAG Ter [*]   TGG Trp [W]   G
-----------------------------------------------------------------
  C   CTT Leu [L]   CCT Pro [P]   CAT His [H]   CGT Arg [R]   T
      CTC Leu [L]   CCC Pro [P]   CAC His [H]   CGC Arg [R]   C
      CTA Leu [L]   CCA Pro [P]   CAA Gln [Q]   CGA Arg [R]   A
      CTG Leu [L]   CCG Pro [P]   CAG Gln [Q]   CGG Arg [R]   G
-----------------------------------------------------------------
  A   ATT Ile [I]   ACT Thr [T]   AAT Asn [N]   AGT Ser [S]   T
      ATC Ile [I]   ACC Thr [T]   AAC Asn [N]   AGC Ser [S]   C
      ATA Ile [I]   ACA Thr [T]   AAA Lys [K]   AGA Arg [R]   A
      ATG Met [M]   ACG Thr [T]   AAG Lys [K]   AGG Arg [R]   G
-----------------------------------------------------------------
  G   GTT Val [V]   GCT Ala [A]   GAT Asp [D]   GGT Gly [G]   T
      GTC Val [V]   GCC Ala [A]   GAC Asp [D]   GGC Gly [G]   C
      GTA Val [V]   GCA Ala [A]   GAA Glu [E]   GGA Gly [G]   A
      GTG Val [V]   GCG Ala [A]   GAG Glu [E]   GGG Gly [G]   G
-----------------------------------------------------------------
*/

// local copy of gc.prt genetic code table ASN.1
const char * CGen_code_table_imp::sm_GenCodeTblMemStr [] =
{
    "Genetic-code-table ::= {\n",
    "{ name \"Standard\" , name \"SGC0\" , id 1 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"---M---------------M---------------M----------------------------\" } ,\n",
    "{ name \"Vertebrate Mitochondrial\" , name \"SGC1\" , id 2 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSS**VVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"--------------------------------MMMM---------------M------------\" } ,\n",
    "{ name \"Yeast Mitochondrial\" , name \"SGC2\" , id 3 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWTTTTPPPPHHQQRRRRIIMMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"----------------------------------MM----------------------------\" } ,\n",
    "{ name \"Mold Mitochondrial; Protozoan Mitochondrial; Coelenterate\n",
    "Mitochondrial; Mycoplasma; Spiroplasma\" ,\n",
    "name \"SGC3\" , id 4 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"--MM---------------M------------MMMM---------------M------------\" } ,\n",
    "{ name \"Invertebrate Mitochondrial\" , name \"SGC4\" , id 5 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSSSVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"---M----------------------------MMMM---------------M------------\" } ,\n",
    "{ name \"Ciliate Nuclear; Dasycladacean Nuclear; Hexamita Nuclear\" ,\n",
    "name \"SGC5\" , id 6 ,\n",
    "ncbieaa  \"FFLLSSSSYYQQCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Echinoderm Mitochondrial; Flatworm Mitochondrial\" , name \"SGC8\" , id 9 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M---------------M------------\" } ,\n",
    "{ name \"Euplotid Nuclear\" , name \"SGC9\" , id 10 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCCWLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Bacterial and Plant Plastid\" , id 11 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"---M---------------M------------MMMM---------------M------------\" } ,\n",
    "{ name \"Alternative Yeast Nuclear\" , id 12 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CC*WLLLSPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-------------------M---------------M----------------------------\" } ,\n",
    "{ name \"Ascidian Mitochondrial\" , id 13 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNKKSSGGVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"---M------------------------------MM---------------M------------\" } ,\n",
    "{ name \"Alternative Flatworm Mitochondrial\" , id 14 ,\n",
    "ncbieaa  \"FFLLSSSSYYY*CCWWLLLLPPPPHHQQRRRRIIIMTTTTNNNKSSSSVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Blepharisma Macronuclear\" , id 15 ,\n",
    "ncbieaa  \"FFLLSSSSYY*QCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Chlorophycean Mitochondrial\" , id 16 ,\n",
    "ncbieaa  \"FFLLSSSSYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Trematode Mitochondrial\" , id 21 ,\n",
    "ncbieaa  \"FFLLSSSSYY**CCWWLLLLPPPPHHQQRRRRIIMMTTTTNNNKSSSSVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M---------------M------------\" } ,\n",
    "{ name \"Scenedesmus obliquus mitochondrial\" , id 22 ,\n",
    "ncbieaa  \"FFLLSS*SYY*LCC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"-----------------------------------M----------------------------\" } ,\n",
    "{ name \"Thraustochytrium mitochondrial code\" , id 23 ,\n",
    "ncbieaa  \"FF*LSSSSYY**CC*WLLLLPPPPHHQQRRRRIIIMTTTTNNKKSSRRVVVVAAAADDEEGGGG\",\n",
    "sncbieaa \"--------------------------------M--M---------------M------------\" } };\n",
    0  // to indicate that there is no more data
};


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 64, chars: 1914, CRC32: 6d579336 */
