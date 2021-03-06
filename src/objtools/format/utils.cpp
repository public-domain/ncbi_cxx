/*  $Id: utils.cpp 142995 2008-10-14 14:47:17Z ludwigf $
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
* Author:  Mati Shomrat, NCBI
*
* File Description:
*   shared utility functions
*
*/
#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>

#include <objects/general/Date.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/Date.hpp>
#include <objects/seq/Bioseq.hpp>
#include <objects/seq/Seq_inst.hpp>
#include <objects/seq/Seq_ext.hpp>
#include <objects/seq/Delta_ext.hpp>
#include <objects/seq/Delta_seq.hpp>
#include <objects/seq/Seq_literal.hpp>
#include <objects/seq/MolInfo.hpp>
#include <objects/seq/seqport_util.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objmgr/scope.hpp>
#include <objmgr/bioseq_handle.hpp>
#include <objmgr/seqdesc_ci.hpp>
#include <objmgr/util/sequence.hpp>
#include <algorithm>
#include "utils.hpp"


BEGIN_NCBI_SCOPE
BEGIN_SCOPE(objects)

bool IsPartOfUrl(
    const string& sentence,
    size_t pos )
{
    const string separators( "( \t\r\n" );
    const string legal_path_chars(
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-." );
    
    //
    //  Weed out silly input:
    //
    if ( sentence == "" || pos > sentence.length() - 1 ) {
        return false;
    }
    if ( string::npos != separators.find( sentence[ pos ] ) ) {
        return false;
    }
    
    //
    //  Find the start of the "word" that surrounds the given position:
    //
    string::size_type left_edge = sentence.find_last_of( separators, pos );
    if ( left_edge == string::npos ) {
        left_edge = 0;
    }
    else {
        ++left_edge;
    }
    
    //
    //  If it's a URL, it better start with a protocol specifier we approve of:
    //
    if ( sentence.substr( left_edge, 5 ) != "http:" ) {
        return false;
    }
    
    //
    //  In addition to that, we require the tilde to show up in a pattern like
    //  "/~[0..9A..Za..z_-.]+". This is inherited from the C toolkit flat file
    //  generator:
    //
    if ( sentence[ pos-1 ] != '/' ) {
        return false;
    }
    ++pos;
    if ( string::npos == legal_path_chars.find( sentence[ pos ] ) ) {
        return false;
    }
    
    for ( ++pos; sentence[ pos ] != 0; ++pos ) {
        if ( string::npos == legal_path_chars.find( sentence[ pos ] ) ) {
            return ( sentence[ pos ] == '/' );
        }
    }
    
    return false; /* never found the terminating '/' */
};     

void ExpandTildes(string& s, ETildeStyle style)
{
    if ( style == eTilde_tilde ) {
        return;
    }

    SIZE_TYPE start = 0, tilde, length = s.length();

    tilde = s.find('~', start);
    if (tilde == NPOS) {  // no tilde
        return;
    }

    string result;

    while ( (start < length)  &&  (tilde = s.find('~', start)) != NPOS ) {
        result.append(s, start, tilde - start);
        char next = (tilde + 1) < length ? s[tilde + 1] : 0;
        switch ( style ) {
        case eTilde_space:
            if ( (tilde + 1 < length  &&  isdigit((unsigned char) next) )  ||
                 (tilde + 2 < length  &&  (next == ' '  ||  next == '(')  &&
                  isdigit((unsigned char) s[tilde + 2]))) {
                result += '~';
            } else {
                result += ' ';
            }
            start = tilde + 1;
            break;
            
        case eTilde_newline:
            if ( tilde + 1 < length  &&  s[tilde + 1] == '~' ) {
                result += '~';
                start = tilde + 2;
            } else {
                result += ";\n";
                start = tilde + 1;
            }
            break;

        case eTilde_comment:
            if (tilde > 0  &&  s[tilde - 1] == '`') {
                result.replace(result.length() - 1, 1, 1,'~');
            }
            else if ( IsPartOfUrl( s, tilde ) ) {
                result += '~';
            } 
            else {
                result += '\n';
            }
            start = tilde + 1;
            break;

        default: // just keep it, for lack of better ideas
            result += '~';
            start = tilde + 1;
            break;
        }
    }
    if (start < length) {
        result.append(s, start, NPOS);
    }
    s.swap(result);
}


void ConvertQuotes(string& str)
{
    replace(str.begin(), str.end(), '\"', '\'');
}


string ConvertQuotes(const string& str)
{
    string retval = str;
    ConvertQuotes(retval);
    return retval;
}


void StripSpaces(string& str)
{
    if (str.empty()) {
        return;
    }

    string::iterator end = str.end();
    string::iterator it = str.begin();
    string::iterator new_str = it;
    while (it != end) {
        *new_str++ = *it;
        if ( (*it == ' ')  ||  (*it == '\t')  ||  (*it == '(') ) {
            for (++it; *it == ' ' || *it == '\t'; ++it) continue;
            if (*it == ')' || *it == ',') {
                --new_str;
            }
        } else {
            ++it;
        }
    }
    str.erase(new_str, str.end());
}


bool RemovePeriodFromEnd(string& str, bool keep_ellipsis)
{
    static const string kEllipsis = "...";

    if ( NStr::EndsWith(str, '.') ) {
        if ( !keep_ellipsis  ||  !NStr::EndsWith(str, kEllipsis) ) {
            str.erase(str.length() - 1);
            return true;
        }
    }

    return false;
}


void AddPeriod(string& str)
{
    static const string kChars = " \t~.\n";

    size_t pos = str.find_last_not_of(kChars);
    str.erase(pos + 1);
    str += '.';
}


void TrimSpaces(string& str, int indent)
{
    if (str.empty()  ||  str.length()  <= (size_t)indent) {
        return;
    }
    if (indent < 0) {
        indent = 0;
    }

    int end = str.length() - 1;
    while (end >= indent  &&  isspace((unsigned char) str[end])) {
        end--;
    }
    if (end < indent) {
        str.erase(indent);
    } else {
        str.erase(end + 1);
    }
}


void TrimSpacesAndJunkFromEnds(string& str, bool allow_ellipsis)
{
    if (str.empty()) {
        return;
    }

    size_t strlen = str.length();
    size_t begin = 0;
    while (begin != strlen) {
        unsigned char ch = str[begin];
        if (ch > ' ') {
            break;
        } else {
            ++begin;
        }
    }
    if (begin == strlen) {
        str.erase();
        return;
    }

    size_t end = strlen - 1;
    bool has_period = false;
    while (end > begin) {
        unsigned char ch = str[end];
        if (ch <= ' '  ||  ch == '.'  ||  ch ==  ','  ||  ch == '~'  ||  ch == ';') {
            has_period = (has_period  ||  ch == '.');
            --end;
        } else {
            break;
        }
    }

    str = str.substr( begin, end + 1 );
    if ( allow_ellipsis && (NPOS != NStr::Find(str, "...", end)) ) {
        str += "...";

    } else if (has_period) {
        str += '.';
    }
}


void MakeLegalFlatFileString( string& str )
{
	// NB: The fixups below should never be necessary if the input string has
	//	ever seen BasicCleanup. In case it hasn't, we'll do the minimum nece-
	//	ssary to produce "legal" output.

	// A flatfile string must not contain double quotes. Instead, we will turn 
	//  " into a '.

	size_t doubleQuote = NStr::Find( str, "\"" );
	if ( NPOS == doubleQuote ) {
		return;
	}
	while ( doubleQuote != NPOS ) {
		str[ doubleQuote ] = '\'';
		doubleQuote = NStr::Find( str, "\"", doubleQuote+1 );
	}
}


static bool s_IsWholeWord(const string& str, size_t pos)
{
    // NB: To preserve the behavior of the C toolkit we only test on the left.
    // This was an old bug in the C toolkit that was never fixed and by now
    // has become the expected behavior.
    return (pos > 0) ?
        isspace((unsigned char) str[pos - 1])  ||  ispunct((unsigned char) str[pos - 1]) : true;
}


void JoinString(string& to, const string& prefix, const string& str, bool noRedundancy)
{
    if ( str.empty() ) {
        return;
    }

    if ( to.empty() ) {
        to += str;
        return;
    }
    
    size_t pos = NPOS;
    if (noRedundancy) {
        for ( pos = NStr::Find(to, str); pos != NPOS; pos += str.length()) {
            if (s_IsWholeWord(to, pos)) {
                return;
            }
        }        
    }

    to += prefix;
    to += str;
}


string JoinString(const list<string>& l, const string& delim, bool noRedundancy)
{
    if ( l.empty() ) {
        return kEmptyStr;
    }

    string result = l.front();
    list<string>::const_iterator it = l.begin();
    while ( ++it != l.end() ) {
        JoinString(result, delim, *it, noRedundancy);
    }

    return result;
}


// Validate the correct format of an accession string.
static bool s_IsValidAccession(const string& acc)
{
    static const size_t kMaxAccLength = 16;

    if ( acc.empty() ) {
        return false;
    }

    if ( acc.length() >= kMaxAccLength ) {
        return false;
    }

    // first character must be uppercase letter
    if ( !(isalpha((unsigned char) acc[0])  &&  isupper((unsigned char) acc[0])) ) {
        return false;
    }

    size_t num_alpha   = 0,
           num_undersc = 0,
           num_digits  = 0;

    const char* ptr = acc.c_str();
    if ( NStr::StartsWith(acc, "NZ_") ) {
        ptr += 3;
    }
    for ( ; isalpha((unsigned char)(*ptr)); ++ptr, ++num_alpha );
    for ( ; *ptr == '_'; ++ptr, ++num_undersc );
    for ( ; isdigit((unsigned char)(*ptr)); ++ptr, ++num_digits );

    if ( (*ptr != '\0')  &&  (*ptr != ' ')  &&  (*ptr != '.') ) {
        return false;
    }

    switch ( num_undersc ) {
    case 0:
        {{
            if ( (num_alpha == 1  &&  num_digits == 5)  ||
                 (num_alpha == 2  &&  num_digits == 6)  ||
                 (num_alpha == 3  &&  num_digits == 5)  || 
                 (num_alpha == 4  &&  num_digits == 8) ) {
                return true;
            }
        }}
        break;

    case 1:
        {{
            // RefSeq accession
            if ( (num_alpha != 2)  ||
                 (num_digits != 6  &&  num_digits != 8  &&  num_digits != 9) ) {
                return false;
            }
            
            char first_letter = acc[0];
            char second_letter = acc[1];

            if ( first_letter == 'N' ) {
                if ( second_letter == 'C'  ||  second_letter == 'G'  ||
                     second_letter == 'M'  ||  second_letter == 'R'  ||
                     second_letter == 'P'  ||  second_letter == 'W'  ||
                     second_letter == 'T' ) {
                    return true;
                }
            } else if ( first_letter == 'X' ) {
                if ( second_letter == 'M'  ||  second_letter == 'R'  ||
                     second_letter == 'P' ) {
                    return true;
                }
            } else if ( first_letter == 'Z'  ||  first_letter == 'A'  ||
                        first_letter == 'Y' ) {
                return (second_letter == 'P');
            }
        }}
        break;

    default:
        return false;
    }

    return false;
}


static bool s_IsValidDotVersion(const string& accn)
{
    size_t pos = accn.find('.');
    if (pos == NPOS) {
        return false;
    }
    size_t num_digis = 0;
    for (++pos; pos < accn.size(); ++pos) {
        if (isdigit((unsigned char) accn[pos])) {
            ++num_digis;
        } else {
            return false;
        }
    }

    return (num_digis >= 1);
}


bool IsValidAccession(const string& accn, EAccValFlag flag)
{
    bool valid = s_IsValidAccession(accn);
    if (valid  &&  flag == eValidateAccDotVer) {
        valid = s_IsValidDotVersion(accn);
    }
    return valid;
}


void DateToString(const CDate& date, string& str,  bool is_cit_sub)
{
    static const string regular_format  = "%{%2D%|01%}-%{%3N%|JUN%}-%Y";
    static const string cit_sub_format = "%{%2D%|??%}-%{%3N%|???%}-%{%4Y%|/???%}";

    const string& format = is_cit_sub ? cit_sub_format : regular_format;

    string date_str;
    date.GetDate(&date_str, format);
    NStr::ToUpper(date_str);

    str.append(date_str);
}


void GetDeltaSeqSummary(const CBioseq_Handle& seq, SDeltaSeqSummary& summary)
{
    if ( !seq.IsSetInst()                                ||
         !seq.IsSetInst_Repr()                           ||
         !(seq.GetInst_Repr() == CSeq_inst::eRepr_delta) ||
         !seq.IsSetInst_Ext()                            ||
         !seq.GetInst_Ext().IsDelta() ) {
        return;
    }

    SDeltaSeqSummary temp;
    CScope& scope = seq.GetScope();

    const CDelta_ext::Tdata& segs = seq.GetInst_Ext().GetDelta().Get();
    temp.num_segs = segs.size();
    
    size_t len = 0;

    CNcbiOstrstream text;

    CDelta_ext::Tdata::const_iterator curr = segs.begin();
    CDelta_ext::Tdata::const_iterator end = segs.end();
    CDelta_ext::Tdata::const_iterator next;
    for ( ; curr != end; curr = next ) {
        {{
            // set next to one after curr
            next = curr; ++next;
        }}
        size_t from = len + 1;
        switch ( (*curr)->Which() ) {
        case CDelta_seq::e_Loc:
            {{
                const CDelta_seq::TLoc& loc = (*curr)->GetLoc();
                if ( loc.IsNull() ) {  // gap
                    ++temp.num_gaps;
                    text << "* " << from << ' ' << len 
                         << " gap of unknown length~";
                } else {  // count length
                    size_t tlen = sequence::GetLength(loc, &scope);
                    len += tlen;
                    temp.residues += tlen;
                    text << "* " << setw(8) << from << ' ' << setw(8) << len 
                         << ": contig of " << tlen << " bp in length~";
                }
            }}  
            break;
        case CDelta_seq::e_Literal:
            {{
                const CDelta_seq::TLiteral& lit = (*curr)->GetLiteral();
                size_t lit_len = lit.CanGetLength() ? lit.GetLength() : 0;
                len += lit_len;
                if ( lit.CanGetSeq_data() ) {
                    temp.residues += lit_len;
                    while ( next != end  &&  (*next)->IsLiteral()  &&
                        (*next)->GetLiteral().CanGetSeq_data() ) {
                        const CDelta_seq::TLiteral& next_lit = (*next)->GetLiteral();
                        size_t next_len = next_lit.CanGetLength() ?
                            next_lit.GetLength() : 0;
                        lit_len += next_len;
                        len += next_len;
                        temp.residues += next_len;
                        ++next;
                    }
                    text << "* " << setw(8) << from << ' ' << setw(8) << len 
                         << ": contig of " << lit_len << " bp in length~";
                } else {
                    bool unk = false;
                    ++temp.num_gaps;
                    if ( lit.CanGetFuzz() ) {
                        const CSeq_literal::TFuzz& fuzz = lit.GetFuzz();
                        if ( fuzz.IsLim()  &&  
                             fuzz.GetLim() == CInt_fuzz::eLim_unk ) {
                            unk = true;
                            ++temp.num_faked_gaps;
                            if ( from > len ) {
                                text << "*                    gap of unknown length~";
                            } else {
                                text << "* " << setw(8) << from << ' ' << setw(8) << len 
                                     << ": gap of unknown length~";
                            }
                        }
                    }
                    if ( !unk ) {
                        text << "* " << setw(8) << from << " " << setw(8) << len
                             << ": gap of " << lit_len << " bp~";
                    }
                }
            }}
            break;

        default:
            break;
        }
    }
    summary = temp;
    summary.text = CNcbiOstrstreamToString(text);
}


const string& GetTechString(int tech)
{
    static const string concept_trans_str = "conceptual translation";
    static const string seq_pept_str = "direct peptide sequencing";
    static const string both_str = "conceptual translation with partial peptide sequencing";
    static const string seq_pept_overlap_str = "sequenced peptide, ordered by overlap";
    static const string seq_pept_homol_str = "sequenced peptide, ordered by homology";
    static const string concept_trans_a_str = "conceptual translation supplied by author";
    
    switch ( tech ) {
    case CMolInfo::eTech_concept_trans:
        return concept_trans_str;

    case CMolInfo::eTech_seq_pept :
        return seq_pept_str;

    case CMolInfo::eTech_both:
        return both_str;

    case CMolInfo::eTech_seq_pept_overlap:
        return seq_pept_overlap_str;

    case CMolInfo::eTech_seq_pept_homol:
        return seq_pept_homol_str;

    case CMolInfo::eTech_concept_trans_a:
        return concept_trans_a_str;

    default:
        return kEmptyStr;
    }

    return kEmptyStr;
}


bool s_IsModelEvidanceUop(const CUser_object& uo)
{
    return (uo.CanGetType()  &&  uo.GetType().IsStr()  &&
        uo.GetType().GetStr() == "ModelEvidence");
}


const CUser_object* s_FindModelEvidanceUop(const CUser_object& uo)
{
    if ( s_IsModelEvidanceUop(uo) ) {
        return &uo;
    }

    const CUser_object* temp = 0;
    ITERATE (CUser_object::TData, ufi, uo.GetData()) {
        const CUser_field& uf = **ufi;
        if ( !uf.CanGetData() ) {
            continue;
        }
        const CUser_field::TData& data = uf.GetData();

        switch ( data.Which() ) {
        case CUser_field::TData::e_Object:
            temp = s_FindModelEvidanceUop(data.GetObject());
            break;

        case CUser_field::TData::e_Objects:
            ITERATE (CUser_field::TData::TObjects, obj, data.GetObjects()) {
                temp = s_FindModelEvidanceUop(**obj);
                if ( temp != 0 ) {
                    break;
                }
            }
            break;

        default:
            break;
        }
        if ( temp != 0 ) {
            break;
        }
    }

    return temp;
}


bool s_GetModelEvidance(const CBioseq_Handle& bsh, SModelEvidance& me)
{
    CConstRef<CUser_object> moduop;
    bool result = false;

    for (CSeqdesc_CI it(bsh, CSeqdesc::e_User);  it;  ++it) {
        moduop.Reset(s_FindModelEvidanceUop(it->GetUser()));
        if (moduop.NotEmpty()) {
            result = true;
            CConstRef<CUser_field> ufp;
            if ( moduop->HasField("Contig Name") ) {
                ufp.Reset(&(moduop->GetField("Contig Name")));
                if ( ufp.NotEmpty()  &&  ufp->IsSetData()  &&  ufp->GetData().IsStr() ) {
                    me.name = ufp->GetData().GetStr();
                }
            }
            if ( moduop->HasField("Method") ) {
                ufp = &(moduop->GetField("Method"));
                if ( ufp.NotEmpty()  &&  ufp->IsSetData()  &&  ufp->GetData().IsStr() ) {
                    me.method = ufp->GetData().GetStr();
                }
            }
            if ( moduop->HasField("mRNA") ) {
                me.mrnaEv = true;
            }
            if ( moduop->HasField("EST") ) {
                me.estEv = true;
            }
        }
    }

    return result;
}


bool GetModelEvidance(const CBioseq_Handle& bsh, SModelEvidance& me)
{
    if ( s_GetModelEvidance(bsh, me) ) {
        return true;
    }

    if ( CSeq_inst::IsAa(bsh.GetInst_Mol()) ) {
        CBioseq_Handle nuc = sequence::GetNucleotideParent(bsh);
        if ( nuc  ) {
            return s_GetModelEvidance(nuc, me);
        }
    }

    return false;
}


// in Ncbistdaa order
static const char* kAANames[] = {
    "---", "Ala", "Asx", "Cys", "Asp", "Glu", "Phe", "Gly", "His", "Ile",
    "Lys", "Leu", "Met", "Asn", "Pro", "Gln", "Arg", "Ser", "Thr", "Val",
    "Trp", "OTHER", "Tyr", "Glx", "Sec", "TERM"
};


const char* GetAAName(unsigned char aa, bool is_ascii)
{
    if (is_ascii) {
        aa = CSeqportUtil::GetMapToIndex
            (CSeq_data::e_Ncbieaa, CSeq_data::e_Ncbistdaa, aa);
    }
    return (aa < sizeof(kAANames)/sizeof(*kAANames)) ? kAANames[aa] : "OTHER";
}


END_SCOPE(objects)
END_NCBI_SCOPE
