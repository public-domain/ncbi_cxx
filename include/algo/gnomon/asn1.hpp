#ifndef ALGO_GNOMON___ASN1__HPP
#define ALGO_GNOMON___ASN1__HPP

/*  $Id: asn1.hpp 116164 2007-12-27 16:09:40Z chetvern $
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
 * Authors:  Vyacheslav Chetvernin
 *
 * File Description:
 * conversion to/from ASN1
 *
 */

#include <objects/seqset/Seq_entry.hpp>
#include <algo/gnomon/gnomon_model.hpp>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(gnomon)

class CAnnotationASN1 {
public:
    CAnnotationASN1(const string& contig_name, const CResidueVec& seq);
    ~CAnnotationASN1();

    void AddModel(const CGeneModel& model);
    CRef<objects::CSeq_entry> GetASN1() const;
private:
    class CImplementationData;
    auto_ptr<CImplementationData> m_data;
};

END_SCOPE(gnomon)
END_NCBI_SCOPE

#endif // ALGO_GNOMON___ASN1__HPP
