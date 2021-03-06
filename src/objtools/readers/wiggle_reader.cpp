/*  $Id: wiggle_reader.cpp 151845 2009-02-09 14:50:16Z ludwigf $
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
 *   WIGGLE file reader
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
#include <objtools/readers/reader_base.hpp>
#include <objtools/readers/wiggle_reader.hpp>
#include <objtools/error_codes.hpp>
#include <objtools/readers/ucscid.hpp>
#include <objtools/readers/idmapper.hpp>

#include <algorithm>
#include <objects/seqres/Seq_graph.hpp>
#include <objects/seqres/Real_graph.hpp>

#include "wiggle_data.hpp"

#define NCBI_USE_ERRCODE_X   Objtools_Rd_RepMask

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
CWiggleReader::CWiggleReader(
    int flags ) :
//  ----------------------------------------------------------------------------
    m_uCurrentRecordType( TYPE_NONE )
{
    m_pIdMapper = CIdMapper::GetIdMapper( "multi" );
    m_pSet = new CWiggleSet( m_pIdMapper );
}


//  ----------------------------------------------------------------------------
CWiggleReader::CWiggleReader(
    const CArgs& args ) :
//  ----------------------------------------------------------------------------
    m_uCurrentRecordType( TYPE_NONE )
{
    //
    //  Make sure the provided options make sense:
    //
    if ( !args["usermap"].AsString().empty() && !args["sitemap"].AsString().empty() ) {
        NCBI_THROW( CArgException, eInvalidArg,
            "CReaderBase: Options \"sitemap\" and \"usermap\" are mutually exclusive" );
    }
    if ( !args["usermap"].AsString().empty() && !args["dbmap"].AsString().empty() ) {
        NCBI_THROW( CArgException, eInvalidArg,
            "CReaderBase: Options \"dbmap\" and \"usermap\" are mutually exclusive" );
    }
    if ( !args["dbmap"].AsString().empty() && !args["sitemap"].AsString().empty() ) {
        NCBI_THROW( CArgException, eInvalidArg,
            "CReaderBase: Options \"sitemap\" and \"dbmap\" are mutually exclusive" );
    }

    //
    //  Ok, set configure the right mapper and initialize object accordingly:
    //    
    if ( !args["usermap"].AsString().empty() || !args["map"].AsString().empty() ) {
        m_pIdMapper = CIdMapper::GetIdMapper( "user" );
        m_pIdMapper->Setup( args );
        m_uIdMode = CWiggleRecord::IDMODE_CHROM;
        m_pSet = new CWiggleSet( m_pIdMapper );
        return;
    }
    if ( !args["sitemap"].AsString().empty() ) {
        m_pIdMapper = CIdMapper::GetIdMapper( "site" );
        m_pIdMapper->Setup( args );
        m_uIdMode = CWiggleRecord::IDMODE_NAME_CHROM;
        m_pSet = new CWiggleSet( m_pIdMapper );
        return;
    }
    if ( !args["dbmap"].AsString().empty() ) {
        m_pIdMapper = CIdMapper::GetIdMapper( "database" );
        m_uIdMode = CWiggleRecord::IDMODE_NAME_CHROM;
        m_pSet = new CWiggleSet( m_pIdMapper );
        return;
    }
        
    //
    //  default: builtin mappings:
    //
    m_pIdMapper = CIdMapper::GetIdMapper( "builtin" );
    m_uIdMode = CWiggleRecord::IDMODE_NAME_CHROM;
    m_pSet = new CWiggleSet( m_pIdMapper );
}


//  ----------------------------------------------------------------------------
CWiggleReader::~CWiggleReader()
//  ----------------------------------------------------------------------------
{
    delete m_pSet;
}

//  ----------------------------------------------------------------------------
bool CWiggleReader::VerifyFormat(
    CNcbiIstream& is )
//  ----------------------------------------------------------------------------
{
    bool verify = false;
    return verify;
}

//  ----------------------------------------------------------------------------
void CWiggleReader::Read( 
    CNcbiIstream& input, 
    CRef<CSeq_annot>& annot )
//
//  Note:   Expecting a sequence of: <trackline> <graph_data>*
//  ----------------------------------------------------------------------------
{
    string pending;
    unsigned int count = 0;
    CWiggleRecord record;
    
    CSeq_annot::TData::TGraph& graphset = annot->SetData().SetGraph();
    x_ReadLine( input, pending );
    while ( !input.eof() ) {
        if ( ! x_ReadTrackData( input, pending, record ) ) {
            return;
        }
        while ( x_ReadGraphData( input, pending, record ) ) {
            m_pSet->AddRecord( record );
        }
    }
    m_pSet->MakeGraph( graphset );
}

//  ----------------------------------------------------------------------------
void CWiggleReader::Dump(
    CNcbiOstream& Out )
//  ----------------------------------------------------------------------------
{
    m_pSet->Dump( Out );
}

//  ----------------------------------------------------------------------------
bool CWiggleReader::x_ReadTrackData(
    CNcbiIstream& input,
    string& pending,
    CWiggleRecord& record )
//
//  Note:   Expecting a line that starts with "track". With comments already
//          weeded out coming in, everything else triggers an error.
//  ----------------------------------------------------------------------------
{
    vector<string> parts;
    Tokenize( pending, " \t", parts );
    if ( ! record.ParseTrackDefinition( parts, m_uIdMode ) ) {
        return false;
    }
    return x_ReadLine( input, pending );
}


//  ----------------------------------------------------------------------------
bool CWiggleReader::x_ReadGraphData(
    CNcbiIstream& input,
    string& pending,
    CWiggleRecord& record )
//
//  Note:   Several possibilities here for the pending line:
//          (1) The line is a "variableStep" declaration. Such a declaration
//              initializes some data but does not represent a complete graph.
//              The following lines then in turn give the missing pieces and
//              complete the record.
//          (2) The line is a "fixedStep" declaration. Similar to (1).
//          (3) BED data. Self contained graph data.
//          (4) "variableStep" data. Completes graph record that was started in
//              the last "variableStep" declaration.
//          (5) "fixedStep" data. Completes graph record that was started in the
//              last "fixedStep" declaration.
//  ----------------------------------------------------------------------------
{
    if ( pending.empty() && input.eof() ) {
        return false;
    }
    vector<string> parts;
    Tokenize( pending, " \t", parts );
    switch ( x_GetLineType( pending ) ) {

        default:
            return false;

        case TYPE_DECLARATION_VARSTEP:
            m_uCurrentRecordType = TYPE_DATA_VARSTEP;
            record.ParseDeclarationVarstep( parts, m_uIdMode );
            x_ReadLine( input, pending );
            return x_ReadGraphData( input, pending, record );

        case TYPE_DECLARATION_FIXEDSTEP:
            m_uCurrentRecordType = TYPE_DATA_FIXEDSTEP;
            record.ParseDeclarationFixedstep( parts, m_uIdMode );
            x_ReadLine( input, pending );
            return x_ReadGraphData( input, pending, record );

        case TYPE_DATA_BED:
            if ( ! record.ParseDataBed( parts, m_uIdMode ) ) {
                return false;
            }
            break;

        case TYPE_DATA_VARSTEP:
            if ( m_uCurrentRecordType != TYPE_DATA_VARSTEP ) {
                return false;
            }
            if ( ! record.ParseDataVarstep( parts, m_uIdMode ) ) {
                return false;
            }
            break;

        case TYPE_DATA_FIXEDSTEP:
            if ( m_uCurrentRecordType != TYPE_DATA_FIXEDSTEP ) {
                return false;
            }
            if ( ! record.ParseDataFixedstep( parts, m_uIdMode ) ) {
                return false;
            }
            break;
    }
    x_ReadLine( input, pending );
    return true;
}

//  ----------------------------------------------------------------------------
bool CWiggleReader::x_ReadLine(
    CNcbiIstream& input,
    string& line )
//  ----------------------------------------------------------------------------
{
    line.clear();
    while ( ! input.eof() ) {
        NcbiGetlineEOL( input, line );
        NStr::TruncateSpacesInPlace( line );
        if ( ! x_IsCommentLine( line ) ) {
            return true;
        }
    }
    return false;
}

//  ----------------------------------------------------------------------------
bool CWiggleReader::x_IsCommentLine(
    const string& line )
//  ----------------------------------------------------------------------------
{
    if ( line.empty() ) {
        return true;
    }
    if ( NStr::StartsWith( line, "browser" ) ) {
        return true;
    }
    if ( NStr::StartsWith( line, "#" ) ) {
        return true;
    }
    return false;
}

//  ----------------------------------------------------------------------------
unsigned int CWiggleReader::x_GetLineType(
    const string& line )
//  ----------------------------------------------------------------------------
{
    vector<string> parts;
    Tokenize( line, " \t", parts );
    if ( parts.empty() ) {
        return TYPE_NONE;
    }
    if ( parts[0] == "track" ) {
        return TYPE_TRACK;
    }
    if ( parts[0] == "variableStep" ) {
        return TYPE_DECLARATION_VARSTEP;
    }
    if ( parts[0] == "fixedStep" ) {
        return TYPE_DECLARATION_FIXEDSTEP;
    }
    if ( parts.size() == 4 ) {
        return TYPE_DATA_BED;
    }
    if ( parts.size() == 2 ) {
        return TYPE_DATA_VARSTEP;
    }
    if ( parts.size() == 1 ) {
        return TYPE_DATA_FIXEDSTEP;
    }
    return TYPE_NONE;
}

//  ----------------------------------------------------------------------------
void CWiggleReader::Tokenize(
    const string& str,
    const string& delims,
    vector< string >& parts )
//  ----------------------------------------------------------------------------
{
    string temp( str );
    bool in_quote( false );
    const char joiner( '#' );

    for ( size_t i=0; i < temp.size(); ++i ) {
        switch( temp[i] ) {

            default:
                break;
            case '\"':
                in_quote = in_quote ^ true;
                break;

            case ' ':
                if ( in_quote ) {
                    temp[i] = joiner;
                }
                break;
        }
    }
    NStr::Tokenize( temp, delims, parts, NStr::eMergeDelims );
    for ( size_t j=0; j < parts.size(); ++j ) {
        for ( size_t i=0; i < parts[j].size(); ++i ) {
            if ( parts[j][i] == joiner ) {
                parts[j][i] = ' ';
            }
        }
    }
}

END_objects_SCOPE
END_NCBI_SCOPE
