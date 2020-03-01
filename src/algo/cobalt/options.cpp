static char const rcsid[] = "$Id: options.cpp 155891 2009-03-27 14:31:34Z boratyng $";

/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's offical duties as a United States Government employee and
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
* ===========================================================================*/

/*****************************************************************************

File name: options.cpp

Author: Greg Boratyn

Contents: Implementation of options for CMultiAligner

******************************************************************************/

/// @file options.cpp
/// Implementation of the CMultiAlignerOptions class

#include <ncbi_pch.hpp>
#include <algo/blast/core/blast_options.h>
#include <algo/cobalt/exception.hpp>
#include <algo/cobalt/options.hpp>
#include <algo/cobalt/patterns.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(cobalt)


CMultiAlignerOptions::CMultiAlignerOptions(void)
{
    x_InitParams(CMultiAlignerOptions::fNoRpsBlast);
}

CMultiAlignerOptions::CMultiAlignerOptions(TMode mode)
{
    x_InitParams(mode);
}

CMultiAlignerOptions::CMultiAlignerOptions(const string& rps_db_name,
                                           TMode mode)
{
    x_InitParams(mode);
    m_RpsDb = rps_db_name;
}


void CMultiAlignerOptions::SetDefaultCddPatterns(void)
{
    m_Patterns.clear();
    AssignDefaultPatterns(m_Patterns);
    m_Mode = fNonStandard;
}


bool CMultiAlignerOptions::Validate(void)
{
    // Check whether m_UseQieryClusters and m_Mode are consistent
    if (!(m_Mode & fNoQueryClusters) != m_UseQueryClusters
         && !(m_Mode & fNonStandard)) {

            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "Conflicting use query clusters setting");
    }

    // Check query clustering params
    if (m_UseQueryClusters) {
        if ((int)m_KmerAlphabet < (int)TKMethods::eRegular
            || (int)m_KmerAlphabet > (int)TKMethods::eSE_B15) {

            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "Invalid alphabet for query clustering");
        }

        if (m_ClustDistMeasure != TKMethods::eFractionCommonKmersGlobal
            && m_ClustDistMeasure != TKMethods::eFractionCommonKmersLocal) {

            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "Invalid distance measure for query clustering");
        }

        if (m_InClustAlnMethod == eNone) {
            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "Method for in-cluster alignment not selected");
        }

        if (m_KmerLength > 7) {
            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "K-mer length (word size) too large,"
                       " must be smaller than 8");
        }

        if (m_KmerLength < 2) {
            m_Messages.push_back("Recommended value for k-mer length"
                                 "(word size) is at least 2");
        }
    }

    // Check if data base name is specified if option selected
    if (!(m_Mode & fNoRpsBlast) && !(m_Mode & fNonStandard)
        && m_RpsDb.empty()) {

        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "RPS BLAST database name not specified");
    }

    // Check if RPS e-value allowed if RPS BLAST used
    if (!m_RpsDb.empty() && m_RpsEvalue < 0.0) {
        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "Incorrect RPS BLAST e-value");
    }

    // Check if Blastp e-value allowed
    if (m_BlastpEvalue < 0.0) {
        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "Incorrect Blastp e-value");
    }

    // Check if patterns are specified if option selected
    if (!(m_Mode & fNoPatterns) && !(m_Mode & fNonStandard)
        && (m_Patterns.size() == 0)) {

        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "CDD patterns not specified");
    }

    // Check whether all cdd patterns are not empty
    ITERATE(vector<CPattern>, it, m_Patterns) {
        if (it->IsEmpty()) {
            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "CDD pattern is empty");
        }
    }

    // Check if pseudocount value allowed if iterative alignment is selected
    if (!(m_Mode & fNoIterate) && !(m_Mode & fNonStandard)
        && m_Pseudocount < 0.0) {

        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "Invalid pseudocount value");
    }

    // Check tree computation method
    if (m_TreeMethod != eNJ && m_TreeMethod != eFastME
        && m_TreeMethod != eClusters) {

        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "Invalid tree computation method for progressive alignment");
    }

    // Check user constraints
    ITERATE(TConstraints, it, m_UserHits) {

        int range1 = it->seq1_stop - it->seq1_start;
        int range2 = it->seq2_stop - it->seq2_start;

        if (range1 < 0 || range2 < 0) {
            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "User constraint range is invalid");
        }

        if (range1 == 1 && range2 != 1 || range1 != 1 && range2 == 1) {
            NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                       "Range specified by user constraints is degenerate");
        }
    }

    return m_Messages.empty();
}


void CMultiAlignerOptions::x_InitParams(TMode mode)
{
    if (mode & fNonStandard) {
        NCBI_THROW(CMultiAlignerException, eInvalidOptions,
                   "Invalid options mode value");
    }

    m_Mode = mode;

    // Query clusters
    m_UseQueryClusters = !(mode & fNoQueryClusters);
    m_MaxInClusterDist = 0.85;
    m_KmerLength = 6;
    m_KmerAlphabet = TKMethods::eSE_B15;
    m_ClustDistMeasure = TKMethods::eFractionCommonKmersGlobal;
    m_InClustAlnMethod = m_UseQueryClusters ? eMulti : eNone;

    // RPS Blast
    m_UsePreRpsHits = true;
    m_RpsEvalue = 0.01;
    m_DomainResFreqBoost = 0.5;

    // Blatp
    m_BlastpEvalue = 0.01;

    // Patterns
    if (!(mode & fNoPatterns)) {
        AssignDefaultPatterns(m_Patterns);
    }

    // Iterate
    m_Iterate = !(mode & fNoIterate);
    m_ConservedCutoff = 0.67;
    m_Pseudocount = 2.0;

    m_TreeMethod = eClusters;
    m_LocalResFreqBoost = 1.0;

    m_UserHitsScore = kDefaultUserConstraintsScore;

    m_MatrixName = "BLOSUM62";
    m_EndGapOpen = kDefaultGapOpen;
    m_EndGapExtend = kDefaultGapExtend;
    m_GapOpen = kDefaultGapOpen;
    m_GapExtend = kDefaultGapExtend;

    m_Verbose = false;
}

END_SCOPE(cobalt)
END_NCBI_SCOPE
