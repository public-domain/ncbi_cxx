/*  $Id: ContextValidator.cpp 157431 2009-04-15 17:03:51Z dicuccio $
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
 * Authors:
 *      Victor Sapojnikov
 *
 * File Description:
 *      AGP context-sensitive validation (uses information from several lines).
 *
 *
 */

#include <ncbi_pch.hpp>

#include "ContextValidator.hpp"
#include <algorithm>
#include <objects/seqloc/Seq_id.hpp>

using namespace ncbi;
using namespace objects;
BEGIN_NCBI_SCOPE

//// class CAgpValidateReader
CAgpValidateReader::CAgpValidateReader(CAgpErrEx& agpErr, CMapCompLen& comp2len) //, bool checkCompNames
  : CAgpReader(&agpErr), m_comp2len(comp2len)
{
  m_CheckCompNames=false; // checkCompNames;

  m_CommentLineCount=m_EolComments=0;
  m_componentsInLastScaffold=m_componentsInLastObject=0;
  m_gapsInLastScaffold=m_gapsInLastObject=0;
  m_prev_orientation_unknown=false;
  m_ObjCount = 0;
  m_ScaffoldCount = 0;
  m_SingleCompScaffolds = 0;
  m_SingleCompObjects = 0;
  m_SingleCompScaffolds_withGaps=0;
  m_SingleCompObjects_withGaps=0;
  m_NoCompScaffolds=0;

  m_CompCount = 0;
  m_GapCount = 0;

  memset(m_CompOri, 0, sizeof(m_CompOri));
  memset(m_GapTypeCnt, 0, sizeof(m_GapTypeCnt));
}

bool CAgpValidateReader::OnError()
{
  if(m_line_skipped) {
    // Avoid printing the wrong AGP line along with "orientation_unknown" error
    m_prev_orientation_unknown=false;

    // For lines with non-syntax errors that are not skipped,
    // these are called from OnGapOrComponent()
    if(m_this_row->pcomment!=NPOS) m_EolComments++; // ??
    agpErr.LineDone(m_line, m_line_num, true);
  }

  return true; // continue checking for errors
}

void CAgpValidateReader::OnGapOrComponent()
{
  if(m_this_row->pcomment!=NPOS) m_EolComments++;
  m_TypeCompCnt.add( m_this_row->GetComponentType() );

  if( m_this_row->IsGap() ) {
    m_GapCount++;
    m_gapsInLastObject++;

    int i = m_this_row->gap_type;
    if(m_this_row->linkage) i+= CAgpRow::eGapCount;
    NCBI_ASSERT( i < (int)(sizeof(m_GapTypeCnt)/sizeof(m_GapTypeCnt[0])),
      "m_GapTypeCnt[] index out of bounds" );
    m_GapTypeCnt[i]++;

    if( !m_this_row->GapEndsScaffold() ) {
      m_gapsInLastScaffold++;
      if(m_prev_orientation_unknown && m_componentsInLastScaffold==1) {
                                   // ^^^can probably ASSERT this^^^
        agpErr.Msg(CAgpErrEx::E_UnknownOrientation, NcbiEmptyString, CAgpErr::fAtPrevLine);
        m_prev_orientation_unknown=false;
      }
    }
  }
  else { // component line
    m_CompCount++;
    m_componentsInLastScaffold++;
    m_componentsInLastObject++;
    switch(m_this_row->orientation) {
      case '+': m_CompOri[CCompVal::ORI_plus ]++; break;
      case '-': m_CompOri[CCompVal::ORI_minus]++; break;
      case '0': m_CompOri[CCompVal::ORI_zero ]++; break;
      case 'n': m_CompOri[CCompVal::ORI_na   ]++; break;
    }

    //// Orientation "0" or "na" only for singletons
    // A saved potential error
    if(m_prev_orientation_unknown) {
      // Make sure that prev_orientation_unknown
      // is not a leftover from the preceding singleton.
      if( m_componentsInLastScaffold==2 ) {
        agpErr.Msg(CAgpErrEx::E_UnknownOrientation,
          NcbiEmptyString, CAgpErr::fAtPrevLine);
      }
      m_prev_orientation_unknown=false;
    }

    if( m_this_row->orientation != '+' && m_this_row->orientation != '-' ) {
      if(m_componentsInLastScaffold==1) {
        // Report an error later if the current scaffold
        // turns out not a singleton. Only singletons can have
        // components with unknown orientation.
        //
        // Note: previous component != previous line
        // if there was a non-breaking gap; thus, we check
        // prev_orientation_unknown for such gaps, too.
        m_prev_orientation_unknown=true;
      }
      else {
        // This error is real, not "potential"; report it now.
        agpErr.Msg(CAgpErrEx::E_UnknownOrientation, NcbiEmptyString);
      }
    }

    //// Check that component spans do not overlap and are in correct order

    // Try to insert to the span as a new entry
    CCompVal comp;
    comp.init(*m_this_row, m_line_num);
    TCompIdSpansPair value_pair( m_this_row->GetComponentId(), CCompSpans(comp) );
    pair<TCompId2Spans::iterator, bool> id_insert_result =
        m_CompId2Spans.insert(value_pair);

    if(id_insert_result.second == false) {
      // Not inserted - the key already exists.
      CCompSpans& spans = (id_insert_result.first)->second;

      // Chose the most specific warning among:
      //   W_SpansOverlap W_SpansOrder W_DuplicateComp
      // The last 2 are omitted for draft seqs.
      CCompSpans::TCheckSpan check_sp = spans.CheckSpan(
        // can replace with m_this_row->component_beg, etc
        comp.beg, comp.end, m_this_row->orientation!='-'
      );
      if( check_sp.second == CAgpErrEx::W_SpansOverlap  ) {
        agpErr.Msg(CAgpErrEx::W_SpansOverlap,
          string(": ")+ check_sp.first->ToString()
        );
      }
      else if( ! m_this_row->IsDraftComponent() ) {
        agpErr.Msg(check_sp.second, // W_SpansOrder or W_DuplicateComp
          string("; preceding span: ")+ check_sp.first->ToString()
        );
      }

      // Add the span to the existing entry
      spans.AddSpan(comp);
    }

    CSeq_id::EAccessionInfo acc_inf = CSeq_id::IdentifyAccession( m_this_row->GetComponentId() );
    int div = acc_inf & CSeq_id::eAcc_division_mask;
    bool unknown_or_local =
      div == CSeq_id::eAcc_unknown ||
      div == CSeq_id::eAcc_local;
    if(m_CheckCompNames) {
      string msg;
      if(  acc_inf & CSeq_id::fAcc_prot ) msg="; looks like a protein accession";
      else if( unknown_or_local         ) msg="; local or misspelled accession";

      if(msg.size()) agpErr.Msg(CAgpErrEx::G_InvalidCompId, msg);
    }

    if(!unknown_or_local && acc_inf & CSeq_id::fAcc_nuc) {
      if( div == CSeq_id::eAcc_wgs ||
          div == CSeq_id::eAcc_wgs_intermed
      ) {
        if(m_this_row->component_type != 'W') agpErr.Msg(CAgpErr::W_CompIsWgsTypeIsNot);
      }
      else if( div == CSeq_id::eAcc_htgs ) {
        if(m_this_row->component_type == 'W') agpErr.Msg(CAgpErr::W_CompIsNotWgsTypeIs);
      }
    }

    if( m_comp2len.size() ) {
      TMapStrInt::iterator it = m_comp2len.find( m_this_row->GetComponentId() );
      if( it==m_comp2len.end() ) {
        // to do: check object id, do not report error if it matches

        agpErr.Msg(CAgpErrEx::G_InvalidCompId, string(": ")+m_this_row->GetComponentId());
      }
      else {
        m_this_row->CheckComponentEnd(it->second);
      }

    }

  }

  CAgpErrEx* errEx = static_cast<CAgpErrEx*>(GetErrorHandler());
  errEx->LineDone(m_line, m_line_num);

  // m_this_row = current gap or component (check with m_this_row->IsGap())
}

void CAgpValidateReader::OnScaffoldEnd()
{
  NCBI_ASSERT(m_componentsInLastScaffold>0 || m_gapsInLastScaffold>0,
    "CAgpValidateReader::OnScaffoldEnd() invoked for a scaffold with no components or gaps");

  m_ScaffoldCount++;
  if(m_componentsInLastScaffold==1) {
    m_SingleCompScaffolds++;
    if(m_gapsInLastScaffold) m_SingleCompScaffolds_withGaps++;
  }
  else if(m_componentsInLastScaffold==0) {
    m_NoCompScaffolds++;
  }
  m_componentsInLastScaffold=0;
  m_gapsInLastScaffold=0;
}

void CAgpValidateReader::OnObjectChange()
{
  if(!m_at_beg) {
    // m_prev_row = the last  line of the old object
    m_ObjCount++;
    if(m_componentsInLastObject==0) agpErr.Msg(
      CAgpErrEx::W_ObjNoComp, string(" ") + m_prev_row->GetObject(),
      CAgpErr::fAtPrevLine
    );
    if(m_componentsInLastObject  ==1) {
      m_SingleCompObjects++;
      if(m_gapsInLastObject) m_SingleCompObjects_withGaps++;
    }

    // if(m_prev_row->IsGap() && m_componentsInLastScaffold==0) m_ScaffoldCount--; (???)
    m_componentsInLastObject=0;
    m_gapsInLastObject=0;
  }

  if(!m_at_end) {
    // m_this_row = the first line of the new object
    TObjSetResult obj_insert_result = m_ObjIdSet.insert(m_this_row->GetObject());
    if (obj_insert_result.second == false) {
      agpErr.Msg(CAgpErrEx::E_DuplicateObj, m_this_row->GetObject(),
        CAgpErr::fAtThisLine);
    }
  }

}

#define ALIGN_W(x) setw(w) << resetiosflags(IOS_BASE::left) << (x)
void CAgpValidateReader::x_PrintTotals() // without comment counts
{
  //// Counts of errors and warnings
  int e_count=agpErr.CountTotals(CAgpErrEx::E_Last);
  // In case -fa or -len was used, add counts for G_InvalidCompId and G_CompEndGtLength.
  e_count+=agpErr.CountTotals(CAgpErrEx::G_Last);
  int w_count=agpErr.CountTotals(CAgpErrEx::W_Last);
  if(e_count || w_count || m_ObjCount) {
    if( m_ObjCount==0 && !agpErr.m_MaxRepeatTopped &&
        e_count==agpErr.CountTotals(CAgpErrEx::E_NoValidLines)
    ) return; // all files are empty, no need to say it again

    cout << "\n";
    agpErr.PrintTotals(cout, e_count, w_count, agpErr.m_msg_skipped);
    if(agpErr.m_MaxRepeatTopped) {
      cout << " (to print all: -limit 0)";
    }
    cout << ".";
    if(agpErr.m_MaxRepeat && (e_count+w_count) ) {
      cout << "\n";
      // Do not print the count of skipped lines
      // so that "(Use -g...)" hint appears right after the last count.
      // (Will not work too well if we add more warnings...)
      agpErr.PrintMessageCounts(cout, CAgpErrEx::CODE_First, CAgpErrEx::CODE_Last, false);
      if(!m_CheckCompNames && (
        agpErr.CountTotals(CAgpErrEx::W_CompIsWgsTypeIsNot) ||
        agpErr.CountTotals(CAgpErrEx::W_CompIsNotWgsTypeIs)
      ) ) {
        cout << "         (Use -g to print lines with WGS component_id/component_type mismatch.)\n";
      }
      // Print only the count of skipped lines
      agpErr.PrintMessageCounts(cout, CAgpErrEx::CODE_Last, CAgpErrEx::CODE_Last-1, true);
    }
  }
  if(m_ObjCount==0) {
    // cout << "No valid AGP lines.\n";
    return;
  }

  cout << "\n";

  //// Prepare component/gap types and counts for later printing
  string s_comp, s_gap;

  CValuesCount::TValPtrVec comp_cnt;
  m_TypeCompCnt.GetSortedValues(comp_cnt);

  for(CValuesCount::TValPtrVec::iterator
    it = comp_cnt.begin();
    it != comp_cnt.end();
    ++it
  ) {
    string *s = CAgpRow::IsGap((*it)->first[0]) ? &s_gap : &s_comp;

    if( s->size() ) *s+= ", ";
    *s+= (*it)->first;
    *s+= ":";
    *s+= NStr::IntToString((*it)->second);
  }

  //// Various counts of AGP elements

  // w: width for right alignment
  int w = NStr::IntToString(m_CompId2Spans.size()).size(); // +1;

  cout << "\n"
    "Objects                : "<<ALIGN_W(m_ObjCount           )<<"\n"
    "- with single component: "<<ALIGN_W(m_SingleCompObjects  )<<"\n";
  if(m_SingleCompObjects_withGaps) {
    cout << "  *** single component + gap(s): " << m_SingleCompObjects_withGaps<< "\n";
  }
  cout << "\n"
    "Scaffolds              : "<<ALIGN_W(m_ScaffoldCount      )<<"\n"
    "- with single component: "<<ALIGN_W(m_SingleCompScaffolds)<<"\n";
  if(m_SingleCompScaffolds_withGaps) {
    cout << "  ***single component + gap(s): " << m_SingleCompScaffolds_withGaps<< "\n";
  }
  if(m_NoCompScaffolds) {
    cout << "***no components, gaps only: " << m_NoCompScaffolds << "\n";
  }
  cout << "\n";


  cout<<
    "Components             : "<< ALIGN_W(m_CompCount);
  if(m_CompCount) {
    if( s_comp.size() ) {
      if( NStr::Find(s_comp, ",")!=NPOS ) {
        // (W: 1234, D: 5678)
        cout << " (" << s_comp << ")";
      }
      else {
        // One type of components: (W) or (invalid type)
        cout << " (" << s_comp.substr( 0, NStr::Find(s_comp, ":") ) << ")";
      }
    }
    cout << "\n"
      "\torientation +  : " << ALIGN_W(m_CompOri[CCompVal::ORI_plus ]) << "\n"
      "\torientation -  : " << ALIGN_W(m_CompOri[CCompVal::ORI_minus]) << "\n"
      "\torientation 0  : " << ALIGN_W(m_CompOri[CCompVal::ORI_zero ]) << "\n"
      "\torientation na : " << ALIGN_W(m_CompOri[CCompVal::ORI_na   ]) << "\n";
  }

  cout << "\n" << "Gaps                   : " << ALIGN_W(m_GapCount);
  if(m_GapCount) {
    // Print (N) if all components are of one type,
    //        or (N: 1234, U: 5678)
    if( s_gap.size() ) {
      if( NStr::Find(s_gap, ",")!=NPOS ) {
        // (N: 1234, U: 5678)
        cout << " (" << s_gap << ")";
      }
      else {
        // One type of gaps: (N)
        cout << " (" << s_gap.substr( 0, NStr::Find(s_gap, ":") ) << ")";
      }
    }

    int linkageYesCnt =
      m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapClone   ]+
      m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapFragment]+
      m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapRepeat  ];
    int linkageNoCnt = m_GapCount - linkageYesCnt;

    int doNotBreakCnt= linkageYesCnt + m_GapTypeCnt[CAgpRow::eGapFragment];
    int breakCnt     = linkageNoCnt  - m_GapTypeCnt[CAgpRow::eGapFragment];

    cout<< "\n- do not break scaffold: "<<ALIGN_W(doNotBreakCnt);
    if(doNotBreakCnt) {
      cout<< "\n  clone   , linkage yes: "<<
            ALIGN_W(m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapClone   ]);
      cout<< "\n  fragment, linkage yes: "<<
            ALIGN_W(m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapFragment]);
      cout<< "\n  fragment, linkage no : "<<
            ALIGN_W(m_GapTypeCnt[                   CAgpRow::eGapFragment]);
      cout<< "\n  repeat  , linkage yes: "<<
            ALIGN_W(m_GapTypeCnt[CAgpRow::eGapCount+CAgpRow::eGapRepeat  ]);
    }

    cout<< "\n- break it, linkage no : "<<ALIGN_W(breakCnt);
    if(breakCnt) {
      for(int i=0; i<CAgpRow::eGapCount; i++) {
        if(i==CAgpRow::eGapFragment) continue;
        cout<< "\n\t"
            << setw(15) << setiosflags(IOS_BASE::left) << CAgpRow::GapTypeToString(i)
            << ": " << ALIGN_W( m_GapTypeCnt[i] );
      }
    }

  }
  cout << "\n";

  if(m_ObjCount) {
    {
      CAccPatternCounter objNamePatterns;
      //objNamePatterns.AddNames(m_ObjIdSet);
      for(TObjSet::const_iterator it = m_ObjIdSet.begin();
          it != m_ObjIdSet.end(); ++it
      ) {
        objNamePatterns.AddName(*it);
      }

      x_PrintPatterns(objNamePatterns, "Object names");
    }
  }

  if(m_CompId2Spans.size()) {
    {
      CAccPatternCounter compNamePatterns;
      for(TCompId2Spans::iterator it = m_CompId2Spans.begin();
        it != m_CompId2Spans.end(); ++it)
      {
        compNamePatterns.AddName(it->first);
      }
      bool hasSuspicious = x_PrintPatterns(compNamePatterns, "Component names");
      if(!m_CheckCompNames && hasSuspicious ) {
        cout<< "Use -g or -a to print lines with suspicious accessions.\n";
      }
    }
  }
}

void CAgpValidateReader::PrintTotals()
{
  x_PrintTotals();

  if(m_CommentLineCount || m_EolComments) cout << "\n";
  if(m_CommentLineCount) {
    cout << "#Comment line count    : " << m_CommentLineCount << "\n";
  }
  if(m_EolComments) {
    cout << "End of line #comments  : " << m_EolComments << "\n";
  }
}

// TRUE:
//   &s at input &pos has one of 3 valid kinds of strings: [123..456]  [123,456]  123
//   output args, strings of digits                      :  sd1  sd2    sd1 sd2   sd1 (sd2 empty)
//   output &pos is moved past the recognized string (which can have any tail)
// FALSE: not a valid number or range format.
static bool ReadNumberOrRange(const string& s, int& pos, string& sd1, string& sd2)
{
  bool openBracket=false;
  if( s[pos]=='[' ) {
    openBracket=true;
    pos++;
  }

  //// count digits -> numDigits
  // DDD [DDD,DDD] [DDD..DDD]
  // ^p1  ^p1 ^p2   ^p1  ^p2
  int p1=pos;
  int len1=0;
  int p2=0;
  while( pos<(int)s.size() ) {
    char ch=s[pos];

    if( isdigit(ch) ) {}
    else if(openBracket) {
      // separators, closing bracket
      if(pos==p1) return false;
      if(ch=='.' || ch==',') {
        if( pos >= (int)s.size()-1 || len1 )
          return false; // nothing after separator || encountered second separator
        len1=pos-p1;
        if(ch=='.') {
          // ..
          pos++;
          if( pos >= (int)s.size() || s[pos] != '.' ) return false;
        }
        p2=pos+1;
      }
      else if(ch==']') {
        if( !p2 || p2==pos ) return false;
        openBracket=false;
        pos++;
        break;
      }
      else return false;
    }
    else break;

    pos++;
  }

  if(openBracket || pos==p1) return false;
  if(!len1) {
    // a plain number, no brackets
    sd1=s.substr(p1, pos-p1);
    sd2=NcbiEmptyString;
  }
  else {
    sd1=s.substr(p1, len1);
    sd2=s.substr(p2, pos-1-p2);
  }
  return true;
}

enum EComponentNameCategory
{
  fNucleotideAccession=0,
  fUnknownFormat      =1,
  fProtein            =2,
  fOneAccManyVer      =4,
  f_category_mask     =7,

  fSome               = 8 // different results for [start..stop]
};
static int GetNameCategory(const string& s)
{
  //// count letters_ -> numLetters
  int numLetters=0;
  for(;;) {
    if( numLetters>=(int)s.size() ) return fUnknownFormat;
    if( !isalpha(s[numLetters]) && s[numLetters]!='_' ) break;
    numLetters++;
  }
  if(numLetters<1 || numLetters>4) return fUnknownFormat;

  int pos=numLetters;
  string sd1, sd2; // strings of digits
  if( !ReadNumberOrRange(s, pos, sd1, sd2) ) return fUnknownFormat;

  //// optional .version or .[range of versions]
  string ver1, ver2; // string of digits
  if(pos<(int)s.size()) {
    if(s[pos]!='.') return fUnknownFormat;
    pos++;

    if( !ReadNumberOrRange(s, pos, ver1, ver2) ) return fUnknownFormat;
    if(pos<(int)s.size()) return fUnknownFormat;

    if(ver1.size()) ver1=string(".")+ver1;
    if(ver2.size()) ver2=string(".")+ver2;

    if(ver1.size()>4 && ver2.size()>4) return fUnknownFormat;
  }

  if(sd2.size()==0) {
    // one accession
    if(ver2.size()!=0) return fOneAccManyVer;
    CSeq_id::EAccessionInfo acc_inf = CSeq_id::IdentifyAccession(s);
    if(  acc_inf & CSeq_id::fAcc_prot ) return fProtein;
    if( (acc_inf & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_unknown ) return fUnknownFormat;
    if( (acc_inf & CSeq_id::eAcc_division_mask) == CSeq_id::eAcc_local   ) return fUnknownFormat;
    return fNucleotideAccession; // the best possible result
  }

  // check both ends of the range in case one has a different number of digits
  string ltr=s.substr(0, numLetters);
  // Note: we do not care if ver1 did not actually came from ltr+sd1, ver2 from ltr+sd2, etc.
  int c1 = GetNameCategory( ltr + sd1 + ver1 );
  int c2 = GetNameCategory( ltr + sd2 + (ver2.size()?ver2:ver1));
  if(c1==c2) {
    if(c1==fNucleotideAccession && (ver1.size()>4 || ver2.size()>4) )
      return fUnknownFormat|fSome;
    return c1;
  }
  return fSome|(c1>c2?c1:c2); // some accessions are suspicious
}

// Sort by accession count, print not more than MaxPatterns or 2*MaxPatterns
bool CAgpValidateReader::x_PrintPatterns(
  CAccPatternCounter& namePatterns, const string& strHeader)
{
  const int MaxPatterns=10;

  // Sorted by count to print most frequent first
  CAccPatternCounter::TMapCountToString cnt_pat; // multimap<int,string>
  namePatterns.GetSortedPatterns(cnt_pat);
  SIZE_TYPE cnt_pat_size=cnt_pat.size();

  cout << "\n";

  // Calculate width of columns 1 (wPattern) and 2 (w, count)
  int w = NStr::IntToString(
      cnt_pat.rbegin()->first // the biggest count
    ).size()+1;

  int wPattern=strHeader.size()-2;
  int totalCount=0;
  int nucCount=0;
  int otherCount=0;
  int patternsPrinted=0;
  bool mixedPattern=false;
  for(
    CAccPatternCounter::TMapCountToString::reverse_iterator
    it = cnt_pat.rbegin(); it != cnt_pat.rend(); it++
  ) {
    if( ++patternsPrinted<=MaxPatterns ||
        cnt_pat_size<=2*MaxPatterns
    ) {
      int i = it->second.size();
      if( i > wPattern ) wPattern = i;
    }
    else {
      if(w+15>wPattern) wPattern = w+15;
    }
    totalCount+=it->first;
    int code=GetNameCategory(it->second);
    ( code==fNucleotideAccession ? nucCount : otherCount ) += it->first;
    if(code & fSome) mixedPattern=true;
  }

  bool mixedCategories=(nucCount && otherCount);
  if(mixedCategories && wPattern<20) wPattern=20;
  // Print the total
  cout<< setw(wPattern+2) << setiosflags(IOS_BASE::left)
      << strHeader << ": " << ALIGN_W(totalCount) << "\n";

  bool printNuc=(nucCount>0);
  // 1 or 2 (if mixedCategories) iterations
  for(;;) {
    if(mixedCategories) {
      // Could be an error - print extra sub-headings to get attention
      cout<< string("------------------------").substr(0, wPattern-20);
      if     (printNuc) cout << " Nucleotide accessions: " << ALIGN_W(nucCount  ) << "\n";
      else              cout << " OTHER identifiers    : " << ALIGN_W(otherCount) << "\n";
    }

    // Print the patterns
    patternsPrinted=0;
    int accessionsSkipped=0;
    int patternsSkipped=0;
    for(
      CAccPatternCounter::TMapCountToString::reverse_iterator
      it = cnt_pat.rbegin(); it != cnt_pat.rend(); it++
    ) {
      int code=GetNameCategory(it->second);
      if(mixedCategories && (code==fNucleotideAccession)!=printNuc) continue;

      // Limit the number of lines to MaxPatterns or 2*MaxPatterns
      if( ++patternsPrinted<=MaxPatterns ||
          cnt_pat_size<=2*MaxPatterns
      ) {
        cout<< "  " << setw(wPattern) << setiosflags(IOS_BASE::left)
            << it->second                    // pattern
            << ": " << ALIGN_W( it->first ); // : count
        if(!printNuc) {
          //if(code & fSome) cout << " - some";
          //switch(code & f_category_mask)
          switch(code)
          {
            case fUnknownFormat|fSome:
            case fOneAccManyVer|fSome:
              cout << " (some local or misspelled)"; break;
            case fProtein|fSome:
              cout << " (some look like protein accessions)"; break;

            case fUnknownFormat: if(!(mixedCategories || mixedPattern)) break;
            case fOneAccManyVer: cout << " (local or misspelled)"; break;
            case fProtein      : cout << " (looks like protein accession)"; break;
          }
        }
        cout << "\n";
      }
      else {
        // accessionsSkipped += CAccPatternCounter::GetCount(*it);
        accessionsSkipped += it->first;
        patternsSkipped++;
      }
    }

    if(accessionsSkipped) {
      string s = "other ";
      s+=NStr::IntToString(patternsSkipped);
      s+=" patterns";
      cout<< "  " << setw(wPattern) << setiosflags(IOS_BASE::left) << s
          << ": " << ALIGN_W( accessionsSkipped )
          << "\n";
    }

    if(!mixedCategories || !printNuc) break;
    printNuc=false;
  }
  return mixedCategories||mixedPattern;
}

//// class CValuesCount

void CValuesCount::GetSortedValues(TValPtrVec& out)
{
  out.clear(); out.reserve( size() );
  for(iterator it = begin();  it != end(); ++it) {
    out.push_back(&*it);
  }
  std::sort( out.begin(), out.end(), x_byCount );
}

void CValuesCount::add(const string& c)
{
  iterator it = find(c);
  if(it==end()) {
    (*this)[c]=1;
  }
  else{
    it->second++;
  }
}

int CValuesCount::x_byCount( value_type* a, value_type* b )
{
  if( a->second != b->second ){
    return a->second > b->second; // by count, largest first
  }
  return a->first < b->first; // by name
}

//// class CCompSpans - stores data for all preceding components
CCompSpans::TCheckSpan CCompSpans::CheckSpan(int span_beg, int span_end, bool isPlus)
{
  // The lowest priority warning (to be ignored for draft seqs)
  TCheckSpan res( begin(), CAgpErrEx::W_DuplicateComp );

  for(iterator it = begin();  it != end(); ++it) {
    // A high priority warning
    if( it->beg <= span_beg && span_beg <= it->end )
      return TCheckSpan(it, CAgpErrEx::W_SpansOverlap);

    // A lower priority warning (to be ignored for draft seqs)
    if( ( isPlus && span_beg < it->beg) ||
        (!isPlus && span_end > it->end)
    ) {
      res.first  = it;
      res.second = CAgpErrEx::W_SpansOrder;
    }
  }

  return res;
}

void CCompSpans::AddSpan(const CCompVal& span)
{
  push_back(span);
}

END_NCBI_SCOPE

