/* $Id: Int_fuzz.cpp 103491 2007-05-04 17:18:18Z kazimird $
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
 *   'general.asn'.
 *
 * ===========================================================================
 */

// standard includes

// generated includes
#include <ncbi_pch.hpp>
#include <objects/general/Int_fuzz.hpp>

#include <algorithm>
#include <set>
#include <math.h>

// generated classes

BEGIN_NCBI_SCOPE

BEGIN_objects_SCOPE // namespace ncbi::objects::

// destructor
CInt_fuzz::~CInt_fuzz(void)
{
}


void CInt_fuzz::GetLabel(string* label, TSeqPos pos, bool right) const
{
    char lim = 0;
    switch (Which()) {
    case CInt_fuzz::e_P_m:
        (*label) += "<+-" + NStr::IntToString(GetP_m()) + ">";
        break;
    case CInt_fuzz::e_Range:
        (*label) += "<" + NStr::IntToString(GetRange().GetMin()) +
            "." + NStr::IntToString(GetRange().GetMax()) + ">";
        break;
    case CInt_fuzz::e_Pct:
        (*label) += "<" + NStr::IntToString(GetPct()) + "%>";
        break;
    case CInt_fuzz::e_Lim:
        switch (GetLim()) {
        case eLim_unk:
        case eLim_other:
            (*label) += "<?>";
            break;
        case eLim_gt:
            lim = '>';
            break;
        case eLim_lt:
            lim = '<';
            break;
        case eLim_tr:
            lim = 'r';
            break;
        case eLim_tl:
            lim = '^';
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    if (lim  &&  lim != 'r') {
        (*label) += lim;
        lim = 0;
    }

    if (right) {
        (*label) += NStr::IntToString(pos + 1);
    }

    if (lim == 'r') {
        (*label) += '^';
    }

    if (!right) {
        (*label) += NStr::IntToString(pos + 1);
    }
}


void CInt_fuzz::AssignTranslated(const CInt_fuzz& f2, TSeqPos n1, TSeqPos n2)
{
    switch (f2.Which()) {
    case e_Range:
        SetRange().SetMin(f2.GetRange().GetMin() + n1 - n2);
        SetRange().SetMax(f2.GetRange().GetMax() + n1 - n2);
        break;
    case e_Pct:
        // use double to avoid overflow
        SetPct((TSeqPos)((double)f2.GetPct() * n1 / n2));
        break;
    case e_Alt:
        ITERATE (TAlt, it, f2.GetAlt()) {
            SetAlt().push_back(*it + n1 - n2);
        }
        break;
    default:
        Assign(f2);
        break;
    }
}


void CInt_fuzz::Add(const CInt_fuzz& f2, TSeqPos& n1, TSeqPos n2,
                    ECombine mode)
{
    static const double kInfinity = 1.0e20;
    bool                hit_pct = false, hit_unk = false, hit_circle = false;
    double              min_delta = 0.0, max_delta = 0.0;
    set<TSignedSeqPos>  offsets;
    for (int i = 0;  i < 2;  ++i) {
        const CInt_fuzz& f = i ? f2 : *this;
        TSeqPos          n = i ? n2 : n1;

        switch (f.Which()) {
        case e_P_m:
            min_delta -= f.GetP_m();
            max_delta += f.GetP_m();
            break;

        case e_Range:
            min_delta -= n - f.GetRange().GetMin();
            max_delta += f.GetRange().GetMax() - n;
            break;

        case e_Pct:
            min_delta -= 0.001 * n * f.GetPct();
            max_delta += 0.001 * n * f.GetPct();
            hit_pct = true;
            break;

        case e_Lim:
            switch (f.GetLim()) {
            case eLim_unk:
                hit_unk   = (!hit_unk  ||  mode != eReduce);
                min_delta = -kInfinity;
                // fall through
            case eLim_gt:
                max_delta = kInfinity;
                break;
            case eLim_lt:
                min_delta = -kInfinity;
                break;
            case eLim_tr:
                min_delta += 0.5;
                max_delta += 0.5;
                break;
            case eLim_tl:
                min_delta -= 0.5;
                max_delta -= 0.5;
                break;
            case eLim_circle:
                hit_circle = (!hit_circle  ||  mode != eReduce);
                break;
            default:
                break;
            }
            break;

        case e_Alt:
        {
            if (f.GetAlt().empty()) {
                break;
            }
            if (offsets.empty()) {
                ITERATE (TAlt, it, f.GetAlt()) {
                    offsets.insert(*it - n);
                }
            } else if (mode == eReduce) {
                // possibly too optimistic; endpoints may balance better
                // than interior points
                min_delta = max_delta = f.GetAlt().front();
                ITERATE (TAlt, it, f.GetAlt()) {
                    if (*it < min_delta) {
                        min_delta = *it;
                    } else if (*it > max_delta) {
                        max_delta = *it;
                    }
                }
                min_delta += *offsets.rbegin();
                max_delta += *offsets.begin();
                offsets.clear();
            } else {
                set<TSignedSeqPos> offsets0(offsets);
                offsets.clear();
                ITERATE (set<TSignedSeqPos>, it, offsets0) {
                    ITERATE (TAlt, it2, f.GetAlt()) {
                        offsets.insert(*it + *it2 - n);
                    }
                }
            }
            break;
        }

        default:
            break;
        }

        if (mode == eReduce  &&  max_delta - min_delta < kInfinity / 2) {
            swap(min_delta, max_delta);
        }
    }

    if (min_delta > max_delta) {
        swap(min_delta, max_delta);
    }

    TSignedSeqPos min_delta_sp = TSignedSeqPos(floor(min_delta + 0.5));
    TSignedSeqPos max_delta_sp = TSignedSeqPos(floor(max_delta + 0.5));

    if (min_delta < -kInfinity / 2) {
        if (max_delta > kInfinity / 2) {
            if ( /* mode == eReduce  && */ !hit_unk ) {
                // assume cancellation
                SetP_m(0);
            } else {
                SetLim(eLim_unk);
            }
        } else {
            if ( !offsets.empty() ) {
                n1 += *offsets.rbegin();
            }
            n1 += max_delta_sp;
            SetLim(eLim_lt);
        }
    } else if (max_delta > kInfinity / 2) {
        if ( !offsets.empty() ) {
            n1 += *offsets.begin();
        }
        n1 += min_delta_sp;
        SetLim(eLim_gt);
    } else if ( !offsets.empty() ) {
        if (max_delta - min_delta < 0.5) {
            TAlt& alt = SetAlt();
            alt.clear();
            ITERATE(set<TSignedSeqPos>, it, offsets) {
                alt.push_back(n1 + *it + min_delta_sp);
                }
        } else if (mode == eReduce) {
            TRange& r = SetRange();
            min_delta += *offsets.rbegin();
            max_delta += *offsets.begin();
            if (min_delta > max_delta) {
                swap(min_delta, max_delta);
            }
            r.SetMin(n1 + min_delta_sp);
            r.SetMax(n1 + max_delta_sp);
        } else {
            // assume there's enough spread to cover any gaps
                TRange& r = SetRange();
                r.SetMin(n1 + min_delta_sp + *offsets.begin());
                r.SetMax(n1 + max_delta_sp + *offsets.rbegin());
        }
    } else if (max_delta - min_delta < 0.5) { // single point identified
        double delta  = 0.5 * (min_delta + max_delta);
        double rdelta = floor(delta + 0.5);
        n1 += (TSignedSeqPos)rdelta;
        if (delta - rdelta > 0.25) {
            SetLim(eLim_tr);
        } else if (delta - rdelta < -0.25) {
            SetLim(eLim_tl);
        } else if (hit_circle) {
            SetLim(eLim_circle);
        } else {
            SetP_m(0);
        }
    } else if (hit_pct) {
        n1 += (TSignedSeqPos)floor(0.5 * (min_delta + max_delta + 1));
        SetPct((TSeqPos)floor(500.0 * (max_delta - min_delta) / n1 + 0.5));
    } else if (min_delta + max_delta < 0.5) { // symmetric
        SetP_m(max_delta_sp);
    } else {
        TRange& r = SetRange();
        r.SetMin(n1 + min_delta_sp);
        r.SetMax(n1 + max_delta_sp);
    }
}


void CInt_fuzz::Negate(TSeqPos n)
{
    switch (Which()) {
    case e_P_m:
    case e_Pct:
        break; // already symmetric

    case e_Range:
    {
        TRange& r       = SetRange();
        TSeqPos old_max = r.GetMax();
        r.SetMax(n + n - r.GetMin());
        r.SetMin(n + n - old_max);
        break;
    }

    case e_Lim:
        switch (GetLim()) {
        case eLim_gt:  SetLim(eLim_lt);  break;
        case eLim_lt:  SetLim(eLim_gt);  break;
        case eLim_tr:  SetLim(eLim_tl);  break;
        case eLim_tl:  SetLim(eLim_tr);  break;
        default:       break;
        }
        break;

    case e_Alt:
        NON_CONST_ITERATE(TAlt, it, SetAlt()) {
            *it = n + n - *it;
        }
        break;

    default:
        break;
    }
}


END_objects_SCOPE // namespace ncbi::objects::

END_NCBI_SCOPE

/* Original file checksum: lines: 61, chars: 1885, CRC32: bf6aceba */
