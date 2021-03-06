/*  $Id: reader_base.hpp 149788 2009-01-15 12:16:16Z ludwigf $
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
 * Author: Frank Ludwig
 *
 * File Description:
 *   Basic reader interface
 *
 */

#ifndef OBJTOOLS_READERS___READERBASE__HPP
#define OBJTOOLS_READERS___READERBASE__HPP

#include <corelib/ncbistd.hpp>
#include <objects/seq/Seq_annot.hpp>
#include <util/format_guess.hpp>

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

//  ----------------------------------------------------------------------------
class NCBI_XOBJREAD_EXPORT CReaderBase
//  ----------------------------------------------------------------------------
{
public:
//    enum FileFormat {
//        FMT_UNKNOWN,
//        FMT_BED,
//        FMT_MICROARRAY,
//        FMT_WIGGLE,
//        FMT_GFF
//    };
    enum ObjectType {
        OT_UNKNOWN,
        OT_SEQANNOT,
        OT_SEQENTRY
    };
    
public:

    virtual ~CReaderBase() {}

    //
    //  Class interface:
    //
    static CReaderBase* GetReader(
        const string&,
        int =0 );

    static CReaderBase* GetReader(
        CFormatGuess::EFormat,
        int =0 );

    static CReaderBase* GetReader(
        const string&,
        const CArgs& );

    static CReaderBase* GetReader(
        CFormatGuess::EFormat,
        const CArgs& );

    static bool VerifyFormat(               // for reference only, should be
        CNcbiIstream& );                    //  reimplemented in concrete sub-
                                            //  classes

    static bool VerifyFormat(               // for reference only, should be
        const char*,                        //  reimplemented in concrete sub-
        size_t ) { return false; };         //  classes
                                            

    //
    //  Object interface:
    //
    virtual unsigned int 
    ObjectType() const { return OT_SEQANNOT; };
    
    virtual void Read( 
        CNcbiIstream&, 
        CRef<CSeq_annot>& ) { return; };
        
    virtual void Read( 
        CNcbiIstream&, 
        CRef<CSeq_entry>& ) { return; };
        
    virtual void Dump(
        CNcbiOstream& ) { return; };

    //
    //  Class helper functions:
    //
protected:
    static bool SplitLines( 
        const char* pcBuffer, 
        size_t uSize,
        list< string>& lines );

};

END_objects_SCOPE
END_NCBI_SCOPE

#endif // OBJTOOLS_READERS___READERBASE__HPP
