/* $Id: Bioseq.hpp 139754 2008-09-08 20:21:01Z kans $
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
 *   'seq.asn'.
 *
 */

#ifndef OBJECTS_SEQ_BIOSEQ_HPP
#define OBJECTS_SEQ_BIOSEQ_HPP


// generated includes
#include <objects/seq/Bioseq_.hpp>
#include <map>

// generated classes


#include <objects/seq/Seqdesc.hpp>


BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

class CSeq_entry;
class CBioseq_set;
class CSeq_loc;
class CDelta_ext;
class CSeq_id;

class NCBI_SEQ_EXPORT CBioseq : public CBioseq_Base, public CSerialUserOp
{
    typedef CBioseq_Base Tparent;
public:
    // constructor
    CBioseq(void);
    // destructor
    ~CBioseq(void);

    // Manage Seq-entry tree structure
    // get parent Seq-entry.
    // NULL means that either there is no parent Seq-entry,
    // or CSeq_entry::Parentize() was never called.
    CSeq_entry* GetParentEntry(void) const;

    // Convenience function to directly get reference to parent Bioseq-set.
    // 0 means that either there is no parent Seq-entry or Bioseq-set,
    // or CSeq_entry::Parentize() was never called.
    CConstRef<CBioseq_set> GetParentSet(void) const;

    // Convenience function that looks for the closest descriptor, first on
    // the Bioseq, then on its parent Bioseq-set, then on its grandparent, etc.
    // 0 means that either there is no parent Seq-entry or Bioseq-set,
    // or CSeq_entry::Parentize() was never called.  Optional level argument
    // is 0 if packaged on Bioseq, 1 if on parent, 2 if on grandparent, etc.
    CConstRef<CSeqdesc> GetClosestDescriptor (CSeqdesc::E_Choice choice, int* level = NULL) const;
    
    // see GetTitle in util/sequence.hpp
    //   string GetTitle(const CBioseq_Handle&, TGetTitleFlags);

    // Construct bioseq from seq-loc. The constructed bioseq
    // has id = "local|"+str_id or "local|constructed###", where
    // ### is a generated number; inst::repr = const,
    // inst::mol = other (since it is impossible to check sequence
    // type by seq-loc). The location is splitted into simple
    // locations (intervals, points, whole-s etc.) and put into
    // ext::delta.
    CBioseq(const CSeq_loc& loc, const string& str_id = kEmptyStr);

    enum ELabelType {
        eType,
        eContent,
        eBoth
    };

    // Append a label to label for a CBioseq based on type, content or both
    void GetLabel(string* label, ELabelType type, bool worst = false) const;

    const CSeq_id* GetFirstId() const;

    // check molecule type for nucleotide or protein
    bool IsNa(void) const;
    bool IsAa(void) const;

    // convenience functions for access to Bioseq length
    bool IsSetLength(void) const;
    TSeqPos GetLength(void) const;

    /// Convert a raw nucleotide sequence with occasional ambiguities
    /// or gaps into a tighter (but somewhat more complex) delta-seq
    /// representation.
    void PackAsDeltaSeq(void);

protected:
    // From CSerialUserOp
    virtual void UserOp_Assign(const CSerialUserOp& source);
    virtual bool UserOp_Equals(const CSerialUserOp& object) const;

private:
    // Prohibit copy constructor and assignment operator
    CBioseq(const CBioseq& value);
    CBioseq& operator= (const CBioseq& value);

    // Seq-entry containing the Bioseq
    void SetParentEntry(CSeq_entry* entry);
    CSeq_entry* m_ParentEntry;

    static void x_SeqLoc_To_DeltaExt(const CSeq_loc& loc, CDelta_ext& ext);

    static int sm_ConstructedId;

    friend class CSeq_entry;
};



/////////////////// CBioseq inline methods

// constructor
inline
CBioseq::CBioseq(void)
    : m_ParentEntry(0)
{
}

inline
void CBioseq::SetParentEntry(CSeq_entry* entry)
{
    m_ParentEntry = entry;
}

inline
CSeq_entry* CBioseq::GetParentEntry(void) const
{
    return m_ParentEntry;
}

/////////////////// end of CBioseq inline methods


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

#endif // OBJECTS_SEQ_BIOSEQ_HPP
/* Original file checksum: lines: 85, chars: 2191, CRC32: 21fd3921 */
