#ifndef FORMATGUESS__HPP
#define FORMATGUESS__HPP

/*  $Id: format_guess.hpp 151442 2009-02-04 16:23:56Z grichenk $
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
 * File Description:  Different "fuzzy-logic" methods to identify file formats.
 *
 */

#include <corelib/ncbistd.hpp>
#include <bitset>

BEGIN_NCBI_SCOPE

class CFormatGuessHints;


//////////////////////////////////////////////////////////////////
///
/// Class implements different ad-hoc unreliable file format
/// identifications.
///

class NCBI_XUTIL_EXPORT CFormatGuess
{
public:
    /// The formats are checked in the same order as declared here.
    enum EFormat {
        eUnknown = 0,           ///< unknown format
        eBinaryASN,             ///< binary ASN.1
        eRmo,                   ///< RepeatMasker Output
        eGtf,                   ///< GFF/GTF style annotations
        eGlimmer3,              ///< Glimmer3 predictions
        eAgp,                   ///< AGP format assembly
        eXml,                   ///< XML
        eWiggle,                ///< UCSC WIGGLE file format
        eBed,                   ///< UCSC BED file format
        eBed15,                 ///< UCSC BED15 or microarray format
        eNewick,                ///< Newick file
        eAlignment,             ///< Text alignment
        eDistanceMatrix,        ///< Distance matrix file
        eFlatFileSequence,      ///< GenBank/GenPept/DDBJ/EMBL flat-file
                                ///< sequence portion
        eFiveColFeatureTable,   ///< Five-column feature table
        eSnpMarkers,            ///< SNP Marker flat file
        eFasta,                 ///< FASTA format sequence record
        eTextASN,               ///< text ASN.1
        eTaxplot,               ///< Taxplot file
        ePhrapAce,              ///< Phrap ACE assembly file
        eTable,                 ///< Generic table
        eFormat_max             ///< Max value of EFormat
    };

    enum ESequenceType {
        eUndefined,
        eNucleotide,
        eProtein
    };

    enum EMode {
        eQuick,
        eThorough
    };

    enum EOnError {
        eDefault = 0,      ///< Return eUnknown
        eThrowOnBadSource, ///< Throw an exception if the data source (stream, file) can't be read
    };

    /// Hints for guessing formats. Two hint types can be used: preferred and
    /// disabled. Preferred are checked before any other formats. Disabled
    /// formats are not checked at all.
    class CFormatHints
    {
    public:
        typedef CFormatGuess::EFormat TFormat;

        CFormatHints(void) {}

        /// Mark the format as preferred.
        CFormatHints& AddPreferredFormat(TFormat fmt);
        /// Mark the format as disabled.
        CFormatHints& AddDisabledFormat(TFormat fmt);
        /// Disable all formats not marked as preferred
        CFormatHints& DisableAllNonpreferred(void);
        /// Remove format hint.
        void RemoveFormat(TFormat fmt);
        /// Remove all hints
        CFormatHints& Reset(void);

        /// Check if there are any hints are set at all.
        bool IsEmpty(void) const;
        /// Check if the format is listed as preferred.
        bool IsPreferred(TFormat fmt) const;
        /// Check if the format is listed as disabled.
        bool IsDisabled(TFormat fmt) const;

    private:
        typedef bitset<CFormatGuess::eFormat_max> THints;

        THints m_Preferred;
        THints m_Disabled;
    };

    /// Guess sequence type. Function calculates sequence alphabet and
    /// identifies if the source belongs to nucleotide or protein sequence
    static ESequenceType SequenceType(const char* str, unsigned length);

    //  ------------------------------------------------------------------------
    //  "Stateless" interface:
    //  Useful for checking for all formats in one simple call.
    //  May go away; use object interface instead.
    //  ------------------------------------------------------------------------
public:
    /// Guess file format structure.
    static
    EFormat Format(const string& path, EOnError onerror = eDefault);

    /// Format prediction based on an input stream
    static
    EFormat Format(CNcbiIstream& input, EOnError onerror = eDefault);

    //  ------------------------------------------------------------------------
    //  "Object" interface:
    //  Use when interested only in a limited number of formats, in excluding
    //  certain tests, a specific order in which formats are tested, ...
    //  ------------------------------------------------------------------------

    //  Construction, destruction
public:
    CFormatGuess();

    CFormatGuess(
        const string& /* file name */ );

    CFormatGuess(
        CNcbiIstream& );

    ~CFormatGuess();

    //  Interface:
public:

    NCBI_DEPRECATED EFormat GuessFormat(EMode);
    NCBI_DEPRECATED bool TestFormat(EFormat,EMode);

    EFormat GuessFormat(EOnError onerror = eDefault);
    bool TestFormat(EFormat,EOnError onerror = eDefault);

    /// Get format hints
    CFormatHints& GetFormatHints(void) { return m_Hints; }

    // helpers:
protected:
    void Initialize();

    bool EnsureTestBuffer();
    bool EnsureStats();
    bool EnsureSplitLines();

    bool TestFormatRepeatMasker(
        EMode );
    bool TestFormatPhrapAce(
        EMode );
    bool TestFormatGtf(
        EMode );
    bool TestFormatGlimmer3(
        EMode );
    bool TestFormatAgp(
        EMode );
    bool TestFormatNewick(
        EMode );
    bool TestFormatXml(
        EMode );
    bool TestFormatAlignment(
        EMode );
    bool TestFormatBinaryAsn(
        EMode );
    bool TestFormatDistanceMatrix(
        EMode );
    bool TestFormatTaxplot(
        EMode );
    bool TestFormatFlatFileSequence(
        EMode );
    bool TestFormatFiveColFeatureTable(
        EMode );
    bool TestFormatTable(
        EMode );
    bool TestFormatFasta(
        EMode );
    bool TestFormatTextAsn(
        EMode );
    bool TestFormatSnpMarkers(
        EMode );
    bool TestFormatBed(
        EMode );
    bool TestFormatBed15(
        EMode );
    bool TestFormatWiggle(
        EMode );

    bool IsInputRepeatMaskerWithoutHeader();
    bool IsInputRepeatMaskerWithHeader();

    static bool IsLineFlatFileSequence(
        const std::string& );
    static bool IsLineNewick(
        const std::string& );
    static bool IsLabelNewick(
        const std::string& );
    static bool IsLineAgp(
        const std::string& );
    static bool IsLineGlimmer3(
        const std::string& );
    static bool IsLineGtf(
        const std::string& );
    static bool IsLinePhrapId(
        const std::string& );
    static bool IsLineRmo(
        const std::string& );
    static bool IsAsnComment(
        const vector<string>& );

private:
    static bool x_TestInput( CNcbiIstream& input, EOnError onerror );

    bool x_TestFormat(EFormat format, EMode mode);

    // data:
protected:
    static const streamsize s_iTestBufferSize = 1024;

    CNcbiIstream& m_Stream;
    bool m_bOwnsStream;
    char* m_pTestBuffer;
    streamsize m_iTestDataSize;

    bool m_bStatsAreValid;
    bool m_bSplitDone;
    unsigned int m_iStatsCountData;
    unsigned int m_iStatsCountAlNumChars;
    unsigned int m_iStatsCountDnaChars;
    unsigned int m_iStatsCountAaChars;
    std::list<std::string> m_TestLines;
    CFormatHints m_Hints;
};


inline CFormatGuess::CFormatHints&
CFormatGuess::CFormatHints::AddPreferredFormat(TFormat fmt)
{
    m_Disabled.reset(fmt);
    m_Preferred.set(fmt);
    return *this;
}


inline CFormatGuess::CFormatHints&
CFormatGuess::CFormatHints::AddDisabledFormat(TFormat fmt)
{
    m_Preferred.reset(fmt);
    m_Disabled.set(fmt);
    return *this;
}

inline CFormatGuess::CFormatHints&
CFormatGuess::CFormatHints::DisableAllNonpreferred(void)
{
    m_Disabled = ~m_Preferred;
    return *this;
}

inline void CFormatGuess::CFormatHints::RemoveFormat(TFormat fmt)
{
    m_Disabled.reset(fmt);
    m_Preferred.reset(fmt);
}

inline CFormatGuess::CFormatHints&
CFormatGuess::CFormatHints::Reset(void)
{
    m_Preferred.reset();
    m_Disabled.reset();
    return *this;
}

inline bool CFormatGuess::CFormatHints::IsEmpty(void) const
{
    return m_Preferred.count() == 0  &&  m_Disabled.count() == 0;
}

inline bool CFormatGuess::CFormatHints::IsPreferred(TFormat fmt) const
{
    return m_Preferred.test(fmt);
}

inline bool CFormatGuess::CFormatHints::IsDisabled(TFormat fmt) const
{
    return m_Disabled.test(fmt);
}

END_NCBI_SCOPE

#endif
