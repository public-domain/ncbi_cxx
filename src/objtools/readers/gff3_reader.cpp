/*  $Id: gff3_reader.cpp 148710 2008-12-31 14:11:04Z ludwigf $
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
 * Author:  Frank Ludwig
 *
 * File Description:
 *   GFF file reader
 *
 */

#include <ncbi_pch.hpp>
#include <corelib/ncbistd.hpp>
#include <corelib/ncbiapp.hpp>
#include <corelib/ncbithr.hpp>
#include <corelib/ncbiutil.hpp>
#include <corelib/ncbiexpt.hpp>
#include <corelib/stream_utils.hpp>

#include <util/static_map.hpp>

#include <serial/iterator.hpp>
#include <serial/objistrasn.hpp>

// Objects includes
#include <objects/general/Int_fuzz.hpp>
#include <objects/general/Object_id.hpp>
#include <objects/general/User_object.hpp>
#include <objects/general/User_field.hpp>
#include <objects/general/Dbtag.hpp>

#include <objects/seqloc/Seq_id.hpp>
#include <objects/seqloc/Seq_loc.hpp>
#include <objects/seqloc/Seq_interval.hpp>
#include <objects/seqloc/Seq_point.hpp>

#include <objects/seq/Seq_annot.hpp>
#include <objects/seq/Annotdesc.hpp>
#include <objects/seq/Annot_descr.hpp>
#include <objects/seqfeat/SeqFeatData.hpp>

#include <objects/seqfeat/Seq_feat.hpp>
#include <objects/seqfeat/BioSource.hpp>
#include <objects/seqfeat/Org_ref.hpp>
#include <objects/seqfeat/OrgName.hpp>
#include <objects/seqfeat/SubSource.hpp>
#include <objects/seqfeat/OrgMod.hpp>
#include <objects/seqfeat/Gene_ref.hpp>
#include <objects/seqfeat/Cdregion.hpp>
#include <objects/seqfeat/Code_break.hpp>
#include <objects/seqfeat/Genetic_code.hpp>
#include <objects/seqfeat/Genetic_code_table.hpp>
#include <objects/seqfeat/RNA_ref.hpp>
#include <objects/seqfeat/Trna_ext.hpp>
#include <objects/seqfeat/Imp_feat.hpp>
#include <objects/seqfeat/Gb_qual.hpp>
#include <objects/seqfeat/Feat_id.hpp>

#include <objtools/readers/reader_exception.hpp>
#include <objtools/readers/gff_reader.hpp>
#include <objtools/readers/gff3_reader.hpp>
#include <objtools/error_codes.hpp>

#include <algorithm>


#define NCBI_USE_ERRCODE_X   Objtools_Rd_RepMask

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
CGff3Reader::CGff3Reader(
    int iFlags ):
//  ----------------------------------------------------------------------------
    m_iReaderFlags( iFlags )
{
}


//  ----------------------------------------------------------------------------
CGff3Reader::CGff3Reader(
    const CArgs& args )
//  ----------------------------------------------------------------------------
{
    m_iReaderFlags = CGFFReader::fDefaults;
    if ( args[ "numeric-ids-to-local" ] ) {
        m_iReaderFlags |= CGFFReader::fNumericIdsAsLocal;
    }
    if ( args[ "all-ids-to-local" ] ) {
        m_iReaderFlags |= CGFFReader::fAllIdsAsLocal;
    }
    if ( args[ "attribute-to-gbqual" ] ) {
        m_iReaderFlags |= CGFFReader::fGBQuals;
    }
    if ( args[ "id-to-product" ] ) {
        m_iReaderFlags |= CGFFReader::fSetProducts;
    }
    if ( args[ "no-gtf" ] ) {
        m_iReaderFlags |= CGFFReader::fNoGTF;
    }
}


//  ----------------------------------------------------------------------------
CGff3Reader::~CGff3Reader()
//  ----------------------------------------------------------------------------
{
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::VerifyFormat(
    CNcbiIstream& is )
//  ----------------------------------------------------------------------------
{
    return false;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::VerifyFormat(
    const char* pcBuffer,
    size_t uSize )
//  ----------------------------------------------------------------------------
{
    list<string> lines;
    if ( ! CReaderBase::SplitLines( pcBuffer, uSize, lines ) ) {
        //  seemingly not even ASCII ...
        return false;
    }
    if ( ! lines.empty() ) {
        //  the last line is probably incomplete. We won't even bother with it.
        lines.pop_back();
    }
    if ( lines.empty() ) {
        return false;
    }

    list<string>::iterator it = lines.begin();
    for ( ;  it != lines.end();  ++it) {
        if ( !it->empty()  &&  (*it)[0] != '#') {
            break;
        }
    }
    
    for ( ;  it != lines.end();  ++it) {
        if ( !VerifyLine( *it ) ) {
            return false;
        }
    }
    return true;
}


//  ----------------------------------------------------------------------------
bool CGff3Reader::VerifyLine(
    const string& line )
//  ----------------------------------------------------------------------------
{
    vector<string> tokens;
    if ( NStr::Tokenize( line, " \t", tokens, NStr::eMergeDelims ).size() < 8 ) {
        return false;
    }
    try {
        NStr::StringToInt( tokens[3] );
        NStr::StringToInt( tokens[4] );
        if ( tokens[5] != "." ) {
            NStr::StringToDouble( tokens[5] );
        }
    }
    catch( ... ) {
        return false;
    }
        
    if ( tokens[6] != "+" && tokens[6] != "." && tokens[6] != "-" ) {
        return false;
    }
    if ( tokens[6].size() != 1 || NPOS == tokens[6].find_first_of( ".+-" ) ) {
        return false;
    }
    if ( tokens[7].size() != 1 || NPOS == tokens[7].find_first_of( ".0123" ) ) {
        return false;
    }
    return true;
}


//  ----------------------------------------------------------------------------
void CGff3Reader::Read( 
    CNcbiIstream& input, 
    CRef<CSeq_entry>& annot )
//  ----------------------------------------------------------------------------
{
    CGFFReader reader;
    annot.Reset( reader.Read( input, m_iReaderFlags ) );
}


END_objects_SCOPE
END_NCBI_SCOPE
