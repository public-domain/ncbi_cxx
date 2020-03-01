/* $Id: SpectrumSet.hpp 104199 2007-05-18 15:55:21Z lewisg $
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
 *  and reliability of the software and data, the NLM 
 *  Although all reasonable efforts have been taken to ensure the accuracyand the U.S.
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
 * Author:  Lewis Y. Geer
 *
 * File Description:
 *   Code for loading in spectrum datasets
 *
 * Remark:
 *   This code was originally generated by application DATATOOL
 *   using specifications from the data definition file
 *   'omssa.asn'.
 */

#ifndef SPECTRUMSET_HPP
#define SPECTRUMSET_HPP


// generated includes
#include <objects/omssa/MSSpectrumset.hpp>
#include <objects/omssa/MSSpectrumFileType.hpp>

#include <iostream>
#include <deque>

#include <corelib/ncbistr.hpp>

// generated classes
BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// proton mass
const double kProton = 1.00728;

//! struct for holding a single peak
typedef struct _InputPeak
{
    //! scaled m/z value for peak
    int mz;
    //! unscaled intensity
    float Intensity;
} TInputPeak;

//! for holding a spectrum
typedef deque < TInputPeak > TInputPeaks;



class NCBI_XOMSSA_EXPORT CSpectrumSet : public CMSSpectrumset {
    //    typedef CMSSpectrumset_Base Tparent;
    typedef CMSSpectrumset Tparent;

public:

    // constructor
    CSpectrumSet(void);
    // destructor
    ~CSpectrumSet(void);

    ///
    /// wrapper for various file loaders
    ///
    int LoadFile(const EMSSpectrumFileType FileType, CNcbiIstream& DTA, int Max = 0);

    ///
    /// load in a single dta file
    ///
    int LoadDTA(
		CNcbiIstream& DTA  // stream containing dta
		);

    ///
    /// load in multiple dta files with <dta> tag separators
    ///
    /// returns -1 if more than Max spectra read
    int LoadMultDTA(
		    CNcbiIstream& DTA,  // stream containing tag delimited dtas
            int Max = 0   // maximum number of dtas to read in, 0= no limit
		    );

    ///
    /// load multiple dta's separated by a blank line
    ///
    /// returns -1 if more than Max spectra read
    int LoadMultBlankLineDTA(
			     CNcbiIstream& DTA,  // stream containing blank delimited dtas
                 int Max = 0,   // maximum number of dtas to read in, 0= no limit
                 bool isPKL = false     // pkl formatted?
			     );

    ///
    /// load mgf file
    ///
    /// returns -1 if more than Max spectra read
    int LoadMGF(
			     CNcbiIstream& DTA,  // stream containing mgf file
                 int Max = 0   // maximum number of dtas to read in, 0= no limit
			     );

protected:

    ///
    ///  Read in the header of a DTA file
    ///
    bool GetDTAHeader(
		      CNcbiIstream& DTA,  // input stream
		      CRef <CMSSpectrum>& MySpectrum,   // asn.1 container for spectra
              bool isPKL = false     // pkl formatted?
		      );

    //! Convert peak list to spectrum
    /*!
    \param InputPeaks list of the input m/z and intensity values
    \param MySpectrum the spectrum to receive the scaled input
    \return success
    */
    bool Peaks2Spectrum(const TInputPeaks& InputPeaks, CRef <CMSSpectrum>& MySpectrum) const;

    //!Read in the body of a dta like file
    /*!
    \param DTA input stream
    \param InputPeaks list of the input m/z and intensity values
    \return success
    */
    bool GetDTABody(
				  CNcbiIstream& DTA,   // input stream
				  TInputPeaks& InputPeaks   // asn.1 container for spectra
				  );

    ///
    /// Read in an ms/ms block in an mgf file
    ///
    int GetMGFBlock(CNcbiIstream& DTA, CRef <CMSSpectrum>& MySpectrum); 

private:
    // Prohibit copy constructor and assignment operator
    CSpectrumSet(const CSpectrumSet& value);
    CSpectrumSet& operator=(const CSpectrumSet& value);

};



/////////////////// CSpectrumSet inline methods

// constructor
inline
CSpectrumSet::CSpectrumSet(void)
{
}


// destructor
inline
CSpectrumSet::~CSpectrumSet(void)
{
}

/////////////////// end of CSpectrumSet inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // SPECTRUMSET_HPP
/* Original file checksum: lines: 85, chars: 2278, CRC32: c22a6458 */
