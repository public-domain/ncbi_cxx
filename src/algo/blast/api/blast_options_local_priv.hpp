/*  $Id: blast_options_local_priv.hpp 144802 2008-11-03 20:57:20Z camacho $
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
 * Author:  Christiam Camacho
 *
 */

/** @file blast_options_local_priv.hpp
 * Private header for local representation of BLAST options.
 */

#ifndef ALGO_BLAST_API___BLAST_OPTIONS_LOCAL_PRIV__HPP
#define ALGO_BLAST_API___BLAST_OPTIONS_LOCAL_PRIV__HPP

#include <objects/seqloc/Na_strand.hpp>
#include <algo/blast/api/blast_types.hpp>
#include <algo/blast/api/blast_aux.hpp>
#include <algo/blast/composition_adjustment/composition_constants.h>

/** @addtogroup AlgoBlast
 *
 * @{
 */

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(blast)

#ifndef SKIP_DOXYGEN_PROCESSING

static const int kInvalidFilterValue = -1;

// Forward declarations
class CBlastOptionsMemento;

/// Encapsulates all blast input parameters
class NCBI_XBLAST_EXPORT CBlastOptionsLocal : public CObject
{
public:
    CBlastOptionsLocal();
    ~CBlastOptionsLocal();

    /// Validate the options
    bool Validate() const;

    /// Accessors/Mutators for individual options
    
    EProgram GetProgram() const;
    EBlastProgramType GetProgramType() const;
    void SetProgram(EProgram p);

    /******************* Lookup table options ***********************/
    double GetWordThreshold() const;
    void SetWordThreshold(double w);

    ELookupTableType GetLookupTableType() const;
    void SetLookupTableType(ELookupTableType type);

    int GetWordSize() const;
    void SetWordSize(int ws);

    /// Megablast only lookup table options
    unsigned char GetMBTemplateLength() const;
    void SetMBTemplateLength(unsigned char len);

    unsigned char GetMBTemplateType() const;
    void SetMBTemplateType(unsigned char type);

    /******************* Query setup options ************************/
    char* GetFilterString() const;
    void SetFilterString(const char* f);

    bool GetMaskAtHash() const;
    void SetMaskAtHash(bool val = true);

    bool GetDustFiltering() const;
    void SetDustFiltering(bool val = true);

    int GetDustFilteringLevel() const;
    void SetDustFilteringLevel(int m);

    int GetDustFilteringWindow() const;
    void SetDustFilteringWindow(int m);

    int GetDustFilteringLinker() const;
    void SetDustFilteringLinker(int m);

    bool GetSegFiltering() const;
    void SetSegFiltering(bool val = true);

    int GetSegFilteringWindow() const;
    void SetSegFilteringWindow(int m);

    double GetSegFilteringLocut() const;
    void SetSegFilteringLocut(double m);

    double GetSegFilteringHicut() const;
    void SetSegFilteringHicut(double m);

    bool GetRepeatFiltering() const;
    void SetRepeatFiltering(bool val = true);

    const char* GetRepeatFilteringDB() const;
    void SetRepeatFilteringDB(const char* db);

    int GetWindowMaskerTaxId() const;
    void SetWindowMaskerTaxId(int taxid);

    const char* GetWindowMaskerDatabase() const;
    void SetWindowMaskerDatabase(const char* db);

    objects::ENa_strand GetStrandOption() const;
    void SetStrandOption(objects::ENa_strand s);

    int GetQueryGeneticCode() const;
    void SetQueryGeneticCode(int gc);

    /******************* Initial word options ***********************/
    int GetWindowSize() const;
    void SetWindowSize(int w);

    double GetXDropoff() const;
    void SetXDropoff(double x);

    /******************* Gapped extension options *******************/
    double GetGapXDropoff() const;
    void SetGapXDropoff(double x);

    double GetGapXDropoffFinal() const;
    void SetGapXDropoffFinal(double x);

    EBlastPrelimGapExt GetGapExtnAlgorithm() const;
    void SetGapExtnAlgorithm(EBlastPrelimGapExt a);

    EBlastTbackExt GetGapTracebackAlgorithm() const;
    void SetGapTracebackAlgorithm(EBlastTbackExt a);

    ECompoAdjustModes GetCompositionBasedStats() const;
    void SetCompositionBasedStats(ECompoAdjustModes mode);

    bool GetSmithWatermanMode() const;
    void SetSmithWatermanMode(bool m = true);

    int GetUnifiedP() const;
    void SetUnifiedP(int u = 0);

    /******************* Hit saving options *************************/
    int GetHitlistSize() const;
    void SetHitlistSize(int s);

    int GetMaxNumHspPerSequence() const;
    void SetMaxNumHspPerSequence(int m);

    int GetCullingLimit() const;
    void SetCullingLimit(int s);

    // Expect value cut-off threshold for an HSP, or a combined hit if sum
    // statistics is used
    double GetEvalueThreshold() const;
    void SetEvalueThreshold(double eval);

    // Raw score cutoff threshold
    int GetCutoffScore() const;
    void SetCutoffScore(int s);

    double GetPercentIdentity() const;
    void SetPercentIdentity(double p);

    int GetMinDiagSeparation() const;
    void SetMinDiagSeparation(int d);

    /// Sum statistics options
    bool GetSumStatisticsMode() const;
    void SetSumStatisticsMode(bool m = true);

    int GetLongestIntronLength() const; // for linking HSPs with uneven gaps
    void SetLongestIntronLength(int l); // for linking HSPs with uneven gaps

    /// Returns true if gapped BLAST is set, false otherwise
    bool GetGappedMode() const;
    void SetGappedMode(bool m = true);

    double GetGapTrigger() const;
    void SetGapTrigger(double g);

    /************************ Scoring options ************************/
    const char* GetMatrixName() const;
    void SetMatrixName(const char* matrix);

    int GetMatchReward() const;
    void SetMatchReward(int r);         // r should be a positive integer

    int GetMismatchPenalty() const;
    void SetMismatchPenalty(int p);     // p should be a negative integer

    int GetGapOpeningCost() const;
    void SetGapOpeningCost(int g);      // g should be a positive integer

    int GetGapExtensionCost() const;
    void SetGapExtensionCost(int e);    // e should be a positive integer

    int GetFrameShiftPenalty() const;
    void SetFrameShiftPenalty(int p);

    bool GetOutOfFrameMode() const;
    void SetOutOfFrameMode(bool m = true);

    /******************** Effective Length options *******************/
    Int8 GetDbLength() const;
    void SetDbLength(Int8 l);

    unsigned int GetDbSeqNum() const;
    void SetDbSeqNum(unsigned int n);

    Int8 GetEffectiveSearchSpace() const;
    void SetEffectiveSearchSpace(Int8 eff);
    void SetEffectiveSearchSpace(const vector<Int8>& eff);

    int GetDbGeneticCode() const;

    // Set genetic code id
    void SetDbGeneticCode(int gc);

    /// @todo PSI-Blast options could go on their own subclass?
    const char* GetPHIPattern() const;
    void SetPHIPattern(const char* pattern, bool is_dna);

    /// Allows to dump a snapshot of the object
    void DebugDump(CDebugDumpContext ddc, unsigned int depth) const;
    
    /******************** PSIBlast options *******************/
    double GetInclusionThreshold() const;
    void SetInclusionThreshold(double incthr);
    
    int GetPseudoCount() const;
    void SetPseudoCount(int ps);
    
    /******************** Megablast Database Index *******************/
    bool GetUseIndex() const;
    bool GetForceIndex() const;
    const string GetIndexName() const;
    void SetUseIndex( bool use_index = true, const string & index_name = "", bool force_index = false );
    bool GetMBIndexLoaded() const;
    void SetMBIndexLoaded( bool index_loaded = true );

    bool operator==(const CBlastOptionsLocal& rhs) const;
    bool operator!=(const CBlastOptionsLocal& rhs) const;

private:

    /// Query sequence settings
    CQuerySetUpOptions            m_QueryOpts;

    /// Lookup table settings
    CLookupTableOptions           m_LutOpts;

    /// Word settings 
    CBlastInitialWordOptions      m_InitWordOpts;

    /// Hit extension settings
    CBlastExtensionOptions        m_ExtnOpts;

    /// Hit saving settings
    CBlastHitSavingOptions        m_HitSaveOpts;

    /// PSI-Blast settings
    CPSIBlastOptions              m_PSIBlastOpts;

    /// Blast database settings
    CBlastDatabaseOptions         m_DbOpts;

    /// Scoring options
    CBlastScoringOptions          m_ScoringOpts;

    /// Effective lengths options
    CBlastEffectiveLengthsOptions m_EffLenOpts;

    /// Blast program
    EProgram                             m_Program;

    /// Use megablast database index.
    bool m_UseMBIndex;
    bool m_ForceMBIndex;

    /// Database index has been loaded.
    bool m_MBIndexLoaded;

    /// Megablast database index name.
    string m_MBIndexName;

    /// Prohibit copy c-tor 
    CBlastOptionsLocal(const CBlastOptionsLocal& bo);
    /// Prohibit assignment operator
    CBlastOptionsLocal& operator=(const CBlastOptionsLocal& bo);

    friend class CBlastOptions;

    /// Friend class which allows extraction of this class' data members for
    /// internal use in the C++ API APIs
    friend class CBlastOptionsMemento;
    friend class CEffectiveSearchSpacesMemento;
    
    /// @internal
    QuerySetUpOptions * GetQueryOpts() const
    {
        return m_QueryOpts;
    }
    
    /// @internal
    LookupTableOptions * GetLutOpts() const
    {
        return m_LutOpts;
    }
    
    /// @internal
    BlastInitialWordOptions * GetInitWordOpts() const
    {
        return m_InitWordOpts;
    }
    
    /// @internal
    BlastExtensionOptions * GetExtnOpts() const
    {
        return m_ExtnOpts;
    }
    
    /// @internal
    BlastHitSavingOptions * GetHitSaveOpts() const
    {
        return m_HitSaveOpts;
    }
    
    /// @internal
    PSIBlastOptions * GetPSIBlastOpts() const
    {
        return m_PSIBlastOpts;
    }
    
    /// @internal
    BlastDatabaseOptions * GetDbOpts() const
    {
        return m_DbOpts;
    }
    
    /// @internal
    BlastScoringOptions * GetScoringOpts() const
    {
        return m_ScoringOpts;
    }
    
    /// @internal
    BlastEffectiveLengthsOptions * GetEffLenOpts() const
    {
        return m_EffLenOpts;
    }
};

inline EProgram
CBlastOptionsLocal::GetProgram() const
{
    return m_Program;
}

inline void
CBlastOptionsLocal::SetProgram(EProgram p)
{
    _ASSERT(p >= eBlastn && p < eBlastProgramMax);
    m_Program = p;
    const EBlastProgramType prog_type = EProgramToEBlastProgramType(p);
    if (prog_type == eBlastTypeUndefined) {
        return;
    }

    GetScoringOpts()->program_number = prog_type;
    GetLutOpts()->program_number = prog_type;
    GetInitWordOpts()->program_number = prog_type;
    GetExtnOpts()->program_number = prog_type;
    GetHitSaveOpts()->program_number = prog_type;
}

inline const char*
CBlastOptionsLocal::GetMatrixName() const
{
    return m_ScoringOpts->matrix;
}

inline void
CBlastOptionsLocal::SetMatrixName(const char* matrix)
{
    if (!matrix)
        return;

    sfree(m_ScoringOpts->matrix);
    m_ScoringOpts->matrix = strdup(matrix);
}

inline double
CBlastOptionsLocal::GetWordThreshold() const
{
    return m_LutOpts->threshold;
}

inline void
CBlastOptionsLocal::SetWordThreshold(double w)
{
    m_LutOpts->threshold = w;
}

inline ELookupTableType
CBlastOptionsLocal::GetLookupTableType() const
{
    return m_LutOpts->lut_type;
}

inline void
CBlastOptionsLocal::SetLookupTableType(ELookupTableType type)
{
    m_LutOpts->lut_type = type;
    if (type == eMBLookupTable) {
       m_LutOpts->word_size = BLAST_WORDSIZE_MEGABLAST;
    } 
}

inline int
CBlastOptionsLocal::GetWordSize() const
{
    return m_LutOpts->word_size;
}

inline void
CBlastOptionsLocal::SetWordSize(int ws)
{
    m_LutOpts->word_size = ws;
}

inline unsigned char
CBlastOptionsLocal::GetMBTemplateLength() const
{
    return m_LutOpts->mb_template_length;
}

inline void
CBlastOptionsLocal::SetMBTemplateLength(unsigned char len)
{
    m_LutOpts->mb_template_length = len;
}

inline unsigned char
CBlastOptionsLocal::GetMBTemplateType() const
{
    return m_LutOpts->mb_template_type;
}

inline void
CBlastOptionsLocal::SetMBTemplateType(unsigned char type)
{
    m_LutOpts->mb_template_type = type;
}

/******************* Query setup options ************************/

inline char*
CBlastOptionsLocal::GetFilterString() const
{
    if (m_QueryOpts->filter_string == NULL) {
        // Don't cache this in case the filtering options are changed
        return BlastFilteringOptionsToString(m_QueryOpts->filtering_options);
    }
    _ASSERT(m_QueryOpts->filter_string != NULL);
    return strdup(m_QueryOpts->filter_string);
}
inline void
CBlastOptionsLocal::SetFilterString(const char* f)
{
   if (!f)
        return;

   sfree(m_QueryOpts->filter_string);
   m_QueryOpts->filter_string = strdup(f);

   SBlastFilterOptions* new_opts = NULL;
   BlastFilteringOptionsFromString(GetProgramType(), f, &(new_opts), NULL);

   if (m_QueryOpts->filtering_options)
   {
      SBlastFilterOptions* old_opts = m_QueryOpts->filtering_options;
      m_QueryOpts->filtering_options = NULL;
      SBlastFilterOptionsMerge(&(m_QueryOpts->filtering_options), old_opts, new_opts);
      old_opts = SBlastFilterOptionsFree(old_opts);
      new_opts = SBlastFilterOptionsFree(new_opts);
   } 
   else
   {
       if (m_QueryOpts->filtering_options)
           m_QueryOpts->filtering_options = 
               SBlastFilterOptionsFree(m_QueryOpts->filtering_options);
       m_QueryOpts->filtering_options = new_opts;
       new_opts = NULL;
   }

   // Repeat filtering is only allowed for blastn.
   if (GetProgramType() != eBlastTypeBlastn && 
       m_QueryOpts->filtering_options->repeatFilterOptions)
       m_QueryOpts->filtering_options->repeatFilterOptions =
           SRepeatFilterOptionsFree(m_QueryOpts->filtering_options->repeatFilterOptions);

   return;
}

inline bool
CBlastOptionsLocal::GetMaskAtHash() const
{
    if (m_QueryOpts->filtering_options->mask_at_hash)
       return true;
    else
       return false;
}
inline void
CBlastOptionsLocal::SetMaskAtHash(bool val)
{

   m_QueryOpts->filtering_options->mask_at_hash = val;

   return;
}

inline bool
CBlastOptionsLocal::GetDustFiltering() const
{
    if (m_QueryOpts->filtering_options->dustOptions)
       return true;
    else
       return false;
}
inline void
CBlastOptionsLocal::SetDustFiltering(bool val)
{

   if (m_QueryOpts->filtering_options->dustOptions)  // free previous structure so we provide defaults.
        m_QueryOpts->filtering_options->dustOptions = 
             SDustOptionsFree(m_QueryOpts->filtering_options->dustOptions);
     
   if (val == false)  // filtering should be turned off
       return;

   SDustOptionsNew(&(m_QueryOpts->filtering_options->dustOptions));

   return;
}

inline int
CBlastOptionsLocal::GetDustFilteringLevel() const
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       return kInvalidFilterValue;

    return m_QueryOpts->filtering_options->dustOptions->level;
}
inline void
CBlastOptionsLocal::SetDustFilteringLevel(int level)
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       SDustOptionsNew(&(m_QueryOpts->filtering_options->dustOptions)); 
      
    m_QueryOpts->filtering_options->dustOptions->level = level;

    return;
}
inline int
CBlastOptionsLocal::GetDustFilteringWindow() const
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       return kInvalidFilterValue;

    return m_QueryOpts->filtering_options->dustOptions->window;
}

inline void
CBlastOptionsLocal::SetDustFilteringWindow(int window)
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       SDustOptionsNew(&(m_QueryOpts->filtering_options->dustOptions)); 
      
    m_QueryOpts->filtering_options->dustOptions->window = window;

    return;
}
inline int
CBlastOptionsLocal::GetDustFilteringLinker() const
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       return kInvalidFilterValue;

    return m_QueryOpts->filtering_options->dustOptions->linker;
}

inline void
CBlastOptionsLocal::SetDustFilteringLinker(int linker)
{
    if (m_QueryOpts->filtering_options->dustOptions == NULL)
       SDustOptionsNew(&(m_QueryOpts->filtering_options->dustOptions)); 
      
    m_QueryOpts->filtering_options->dustOptions->linker = linker;

    return;
}

inline bool
CBlastOptionsLocal::GetSegFiltering() const
{
    if (m_QueryOpts->filtering_options->segOptions)
      return true;
    else
      return false;
}

inline void
CBlastOptionsLocal::SetSegFiltering(bool val)
{

   if (m_QueryOpts->filtering_options->segOptions)  // free previous structure so we provide defaults.
        m_QueryOpts->filtering_options->segOptions = 
             SSegOptionsFree(m_QueryOpts->filtering_options->segOptions);
     
   if (val == false)  // filtering should be turned off
       return;

   SSegOptionsNew(&(m_QueryOpts->filtering_options->segOptions));

   return;
}

inline int
CBlastOptionsLocal::GetSegFilteringWindow() const
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       return kInvalidFilterValue;
      
    return m_QueryOpts->filtering_options->segOptions->window;
}

inline void
CBlastOptionsLocal::SetSegFilteringWindow(int window)
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       SSegOptionsNew(&(m_QueryOpts->filtering_options->segOptions)); 
      
    m_QueryOpts->filtering_options->segOptions->window = window;

    return;
}

inline double
CBlastOptionsLocal::GetSegFilteringLocut() const
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       return kInvalidFilterValue;
      
    return m_QueryOpts->filtering_options->segOptions->locut;
}

inline void
CBlastOptionsLocal::SetSegFilteringLocut(double locut)
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       SSegOptionsNew(&(m_QueryOpts->filtering_options->segOptions)); 
      
    m_QueryOpts->filtering_options->segOptions->locut = locut;

    return;
}

inline double
CBlastOptionsLocal::GetSegFilteringHicut() const
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       return kInvalidFilterValue;
      
    return m_QueryOpts->filtering_options->segOptions->hicut;
}

inline void
CBlastOptionsLocal::SetSegFilteringHicut(double hicut)
{
    if (m_QueryOpts->filtering_options->segOptions == NULL)
       SSegOptionsNew(&(m_QueryOpts->filtering_options->segOptions)); 
      
    m_QueryOpts->filtering_options->segOptions->hicut = hicut;

    return;
}

inline bool
CBlastOptionsLocal::GetRepeatFiltering() const
{
    if (m_QueryOpts->filtering_options->repeatFilterOptions)
      return true;
    else
      return false;
}

inline void
CBlastOptionsLocal::SetRepeatFiltering(bool val)
{

   if (m_QueryOpts->filtering_options->repeatFilterOptions)  // free previous structure so we provide defaults.
        m_QueryOpts->filtering_options->repeatFilterOptions = 
             SRepeatFilterOptionsFree(m_QueryOpts->filtering_options->repeatFilterOptions);
     
   if (val == false)  // filtering should be turned off
       return;

   SRepeatFilterOptionsNew(&(m_QueryOpts->filtering_options->repeatFilterOptions));

   return;
}

inline const char*
CBlastOptionsLocal::GetRepeatFilteringDB() const
{
    if (m_QueryOpts->filtering_options->repeatFilterOptions == NULL)
      return NULL;

    return m_QueryOpts->filtering_options->repeatFilterOptions->database;
}

inline void
CBlastOptionsLocal::SetRepeatFilteringDB(const char* db)
{
   if (!db)
      return;

   SRepeatFilterOptionsResetDB(&(m_QueryOpts->filtering_options->repeatFilterOptions), db);

   return;
}

inline int
CBlastOptionsLocal::GetWindowMaskerTaxId() const
{
    if (m_QueryOpts->filtering_options->windowMaskerOptions == NULL)
        return 0;
    
    return m_QueryOpts->filtering_options->windowMaskerOptions->taxid;
}

inline void
CBlastOptionsLocal::SetWindowMaskerTaxId(int taxid)
{
    if (m_QueryOpts->filtering_options->windowMaskerOptions == NULL)
        SWindowMaskerOptionsNew
            (&(m_QueryOpts->filtering_options->windowMaskerOptions));
    
    m_QueryOpts->filtering_options->windowMaskerOptions->taxid = taxid;
}

inline const char*
CBlastOptionsLocal::GetWindowMaskerDatabase() const
{
    if (! m_QueryOpts->filtering_options->windowMaskerOptions)
        return NULL;
    
    return m_QueryOpts->filtering_options->windowMaskerOptions->database;
}

inline void
CBlastOptionsLocal::SetWindowMaskerDatabase(const char* db)
{
    if (m_QueryOpts->filtering_options->windowMaskerOptions == NULL)
        SWindowMaskerOptionsNew
            (&(m_QueryOpts->filtering_options->windowMaskerOptions));
    
    SWindowMaskerOptionsResetDB
        (&(m_QueryOpts->filtering_options->windowMaskerOptions), db);
}

inline objects::ENa_strand
CBlastOptionsLocal::GetStrandOption() const
{
    return (objects::ENa_strand) m_QueryOpts->strand_option;
}

inline void
CBlastOptionsLocal::SetStrandOption(objects::ENa_strand s)
{
    m_QueryOpts->strand_option = (unsigned char) s;
}

inline int
CBlastOptionsLocal::GetQueryGeneticCode() const
{
    return m_QueryOpts->genetic_code;
}

inline void
CBlastOptionsLocal::SetQueryGeneticCode(int gc)
{
    m_QueryOpts->genetic_code = gc;
}

/******************* Initial word options ***********************/
inline int
CBlastOptionsLocal::GetWindowSize() const
{
    return m_InitWordOpts->window_size;
}

inline void
CBlastOptionsLocal::SetWindowSize(int s)
{
    m_InitWordOpts->window_size = s;
}

inline double
CBlastOptionsLocal::GetXDropoff() const
{
    return m_InitWordOpts->x_dropoff;
}

inline void
CBlastOptionsLocal::SetXDropoff(double x)
{
    m_InitWordOpts->x_dropoff = x;
}

inline double
CBlastOptionsLocal::GetGapTrigger() const
{
    return m_InitWordOpts->gap_trigger;
}

inline void
CBlastOptionsLocal::SetGapTrigger(double g)
{
    m_InitWordOpts->gap_trigger = g;
}

/******************* Gapped extension options *******************/
inline double
CBlastOptionsLocal::GetGapXDropoff() const
{
    return m_ExtnOpts->gap_x_dropoff;
}

inline void
CBlastOptionsLocal::SetGapXDropoff(double x)
{
    m_ExtnOpts->gap_x_dropoff = x;
}

inline double
CBlastOptionsLocal::GetGapXDropoffFinal() const
{
    return m_ExtnOpts->gap_x_dropoff_final;
}

inline void
CBlastOptionsLocal::SetGapXDropoffFinal(double x)
{
    m_ExtnOpts->gap_x_dropoff_final = x;
}

inline EBlastPrelimGapExt
CBlastOptionsLocal::GetGapExtnAlgorithm() const
{
    return m_ExtnOpts->ePrelimGapExt;
}

inline void
CBlastOptionsLocal::SetGapExtnAlgorithm(EBlastPrelimGapExt a)
{
    m_ExtnOpts->ePrelimGapExt = a;
}

inline EBlastTbackExt
CBlastOptionsLocal::GetGapTracebackAlgorithm() const
{
    return m_ExtnOpts->eTbackExt;
}

inline void
CBlastOptionsLocal::SetGapTracebackAlgorithm(EBlastTbackExt a)
{
    m_ExtnOpts->eTbackExt = a;
}

inline ECompoAdjustModes
CBlastOptionsLocal::GetCompositionBasedStats() const
{
    return static_cast<ECompoAdjustModes>(m_ExtnOpts->compositionBasedStats);
}

inline void
CBlastOptionsLocal::SetCompositionBasedStats(ECompoAdjustModes mode)
{
    m_ExtnOpts->compositionBasedStats = static_cast<Int4>(mode);
}

inline bool
CBlastOptionsLocal::GetSmithWatermanMode() const
{
    if (m_ExtnOpts->eTbackExt == eSmithWatermanTbck)
        return true;
    else
        return false;
}

inline void
CBlastOptionsLocal::SetSmithWatermanMode(bool m)
{
    if (m == true)
       m_ExtnOpts->eTbackExt = eSmithWatermanTbck;
    else
       m_ExtnOpts->eTbackExt = eDynProgTbck;
}

inline int
CBlastOptionsLocal::GetUnifiedP() const
{
   return m_ExtnOpts->unifiedP;
}

inline void
CBlastOptionsLocal::SetUnifiedP(int u)
{
   m_ExtnOpts->unifiedP = u;
}

/******************* Hit saving options *************************/
inline int
CBlastOptionsLocal::GetHitlistSize() const
{
    return m_HitSaveOpts->hitlist_size;
}

inline void
CBlastOptionsLocal::SetHitlistSize(int s)
{
    m_HitSaveOpts->hitlist_size = s;
}

inline int
CBlastOptionsLocal::GetMaxNumHspPerSequence() const
{
    return m_HitSaveOpts->hsp_num_max;
}

inline void
CBlastOptionsLocal::SetMaxNumHspPerSequence(int m)
{
    m_HitSaveOpts->hsp_num_max = m;
}

inline int
CBlastOptionsLocal::GetCullingLimit() const
{
    return m_HitSaveOpts->culling_limit;
}

inline void
CBlastOptionsLocal::SetCullingLimit(int s)
{
    m_HitSaveOpts->culling_limit = s;
}

inline double
CBlastOptionsLocal::GetEvalueThreshold() const
{
    return m_HitSaveOpts->expect_value;
}

inline void
CBlastOptionsLocal::SetEvalueThreshold(double eval)
{
    m_HitSaveOpts->expect_value = eval;
}

inline int
CBlastOptionsLocal::GetCutoffScore() const
{
    return m_HitSaveOpts->cutoff_score;
}

inline void
CBlastOptionsLocal::SetCutoffScore(int s)
{
    m_HitSaveOpts->cutoff_score = s;
}

inline double
CBlastOptionsLocal::GetPercentIdentity() const
{
    return m_HitSaveOpts->percent_identity;
}

inline void
CBlastOptionsLocal::SetPercentIdentity(double p)
{
    m_HitSaveOpts->percent_identity = p;
}

inline int
CBlastOptionsLocal::GetMinDiagSeparation() const
{
    return m_HitSaveOpts->min_diag_separation;
}

inline void
CBlastOptionsLocal::SetMinDiagSeparation(int d)
{
    m_HitSaveOpts->min_diag_separation = d;
}

inline bool
CBlastOptionsLocal::GetSumStatisticsMode() const
{
    return m_HitSaveOpts->do_sum_stats ? true : false;
}

inline void
CBlastOptionsLocal::SetSumStatisticsMode(bool m)
{
    m_HitSaveOpts->do_sum_stats = m;
}

inline int
CBlastOptionsLocal::GetLongestIntronLength() const
{
    return m_HitSaveOpts->longest_intron;
}

inline void
CBlastOptionsLocal::SetLongestIntronLength(int l)
{
    m_HitSaveOpts->longest_intron = l;
}

inline bool
CBlastOptionsLocal::GetGappedMode() const
{
    return m_ScoringOpts->gapped_calculation ? true : false;
}

inline void
CBlastOptionsLocal::SetGappedMode(bool m)
{
    m_ScoringOpts->gapped_calculation = m;
}

/************************ Scoring options ************************/
inline int 
CBlastOptionsLocal::GetMatchReward() const
{
    return m_ScoringOpts->reward;
}

inline void 
CBlastOptionsLocal::SetMatchReward(int r)
{
    m_ScoringOpts->reward = r;
}

inline int 
CBlastOptionsLocal::GetMismatchPenalty() const
{
    return m_ScoringOpts->penalty;
}

inline void 
CBlastOptionsLocal::SetMismatchPenalty(int p)
{
    m_ScoringOpts->penalty = p;
}

inline int 
CBlastOptionsLocal::GetGapOpeningCost() const
{
    return m_ScoringOpts->gap_open;
}

inline void 
CBlastOptionsLocal::SetGapOpeningCost(int g)
{
    m_ScoringOpts->gap_open = g;
}

inline int 
CBlastOptionsLocal::GetGapExtensionCost() const
{
    return m_ScoringOpts->gap_extend;
}

inline void 
CBlastOptionsLocal::SetGapExtensionCost(int e)
{
    m_ScoringOpts->gap_extend = e;
}

inline int 
CBlastOptionsLocal::GetFrameShiftPenalty() const
{
    return m_ScoringOpts->shift_pen;
}

inline void 
CBlastOptionsLocal::SetFrameShiftPenalty(int p)
{
    m_ScoringOpts->shift_pen = p;
}

inline bool 
CBlastOptionsLocal::GetOutOfFrameMode() const
{
    return m_ScoringOpts->is_ooframe ? true : false;
}

inline void 
CBlastOptionsLocal::SetOutOfFrameMode(bool m)
{
    m_ScoringOpts->is_ooframe = m;
}

/******************** Effective Length options *******************/
inline Int8 
CBlastOptionsLocal::GetDbLength() const
{
    return m_EffLenOpts->db_length;
}

inline void 
CBlastOptionsLocal::SetDbLength(Int8 l)
{
    m_EffLenOpts->db_length = l;
}

inline unsigned int 
CBlastOptionsLocal::GetDbSeqNum() const
{
    return (unsigned int) m_EffLenOpts->dbseq_num;
}

inline void 
CBlastOptionsLocal::SetDbSeqNum(unsigned int n)
{
    m_EffLenOpts->dbseq_num = (Int4) n;
}

inline Int8 
CBlastOptionsLocal::GetEffectiveSearchSpace() const
{
    if (m_EffLenOpts->num_searchspaces == 0)
        return 0;

    return m_EffLenOpts->searchsp_eff[0];
}
 
inline void 
CBlastOptionsLocal::SetEffectiveSearchSpace(Int8 eff)
{
    if (m_EffLenOpts->num_searchspaces < 1) {
        m_EffLenOpts->num_searchspaces = 1;
        if (m_EffLenOpts->searchsp_eff) sfree(m_EffLenOpts->searchsp_eff);
        m_EffLenOpts->searchsp_eff = (Int8 *)malloc(sizeof(Int8));
    }

    fill(m_EffLenOpts->searchsp_eff,
         m_EffLenOpts->searchsp_eff+m_EffLenOpts->num_searchspaces,
         eff);
}

inline void 
CBlastOptionsLocal::SetEffectiveSearchSpace(const vector<Int8>& eff)
{
    if (m_EffLenOpts->num_searchspaces < static_cast<Int4>(eff.size())) {
        m_EffLenOpts->num_searchspaces = static_cast<Int4>(eff.size());
        if (m_EffLenOpts->searchsp_eff) sfree(m_EffLenOpts->searchsp_eff);
        m_EffLenOpts->searchsp_eff = (Int8 *)malloc(eff.size() * sizeof(Int8));
    }

    copy(eff.begin(), eff.end(), m_EffLenOpts->searchsp_eff);
}

inline int 
CBlastOptionsLocal::GetDbGeneticCode() const
{
    return m_DbOpts->genetic_code;
}

inline const char* 
CBlastOptionsLocal::GetPHIPattern() const
{
    return m_LutOpts->phi_pattern;
}

inline double
CBlastOptionsLocal::GetInclusionThreshold() const
{
    return m_PSIBlastOpts->inclusion_ethresh;
}

inline void
CBlastOptionsLocal::SetInclusionThreshold(double incthr)
{
    m_PSIBlastOpts->inclusion_ethresh = incthr;
}

inline int
CBlastOptionsLocal::GetPseudoCount() const
{
    return m_PSIBlastOpts->pseudo_count;
}

inline void
CBlastOptionsLocal::SetPseudoCount(int pc)
{
    m_PSIBlastOpts->pseudo_count = pc;
}

inline void 
CBlastOptionsLocal::SetPHIPattern(const char* pattern, bool is_dna)
{
    if (is_dna)
       m_LutOpts->lut_type = ePhiNaLookupTable;
    else
       m_LutOpts->lut_type = ePhiLookupTable;

    if (pattern)
        m_LutOpts->phi_pattern = strdup(pattern);
    else if (m_LutOpts->phi_pattern)
        sfree(m_LutOpts->phi_pattern);
}

/******************** Megablast Database Index *******************/
inline bool CBlastOptionsLocal::GetUseIndex() const
{
    return m_UseMBIndex;
}

inline bool CBlastOptionsLocal::GetForceIndex() const
{
    return m_ForceMBIndex;
}

inline bool CBlastOptionsLocal::GetMBIndexLoaded() const
{
    return m_MBIndexLoaded;
}

inline const string CBlastOptionsLocal::GetIndexName() const
{
    return m_MBIndexName;
}

inline void CBlastOptionsLocal::SetMBIndexLoaded( bool index_loaded )
{
    m_MBIndexLoaded = index_loaded;
}

inline void CBlastOptionsLocal::SetUseIndex( 
        bool use_index, const string & index_name, bool force_index )
{
    m_UseMBIndex = use_index;

    if( m_UseMBIndex ) {
        m_ForceMBIndex = force_index;
        m_MBIndexName  = index_name;
    }
}

#endif /* SKIP_DOXYGEN_PROCESSING */

END_SCOPE(blast)
END_NCBI_SCOPE

/* @} */

#endif /* ALGO_BLAST_API___BLAST_OPTIONS_LOCAL_PRIV__HPP */