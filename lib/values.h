/**********************************************************************
 * Copyright (c) 2019 Artem V. Andreev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/
/** @node Values
 * @chapter Values
 */
#ifndef TNH_VALUES_H
#define TNH_VALUES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unitypes.h>
#include "utils.h"

enum tn_value_type {
    TNV_NULL,
    TNV_INTEGER,
    TNV_FLOAT,
    TNV_CHAR,
    TNV_STRING,
    TNV_BSTRING,
    TNV_CHARSET,
    TNV_EDITSTRING,
    TNV_FMTSTRING,
    TNV_VECTOR,
    TNV_RANGE,
    TNV_REF,
    TNV_WEAKREF,
    TNV_SLICE,
    TNV_ARRAY,
    TNV_TABLE,
    TNV_NODE,
    TNV_TERM,
    TNV_TERMDEF,
    TNV_CODE,
    TNV_ERROR,
    TNV_ERRORDEF,
    TNV_LOCAL,
};

typedef struct tn_ref {
    uint64_t hi;
    uint64_t lo;
} tn_ref;

typedef uint32_t tn_rel_ref;

enum tn_ref_kind {
    TNRK_STATIC,
    TNRK_CONST,
    TNRK_VOLATILE,
    TNRK_EXTERNAL,
    TNRK_SCRATCH,
};

#define TNRK_BIT_BASE 56
#define TNRK_REL_BIT_BASE 24
#define TNRK_MASK ((1ULL << TNRK_BIT_BASE) - 1)
#define TNRK_REL_MASK ((1ULL << TNRK_REL_BIT_BASE) - 1)

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_ref_is_kind(tn_ref ref, enum tn_ref_kind kind)
{
    return (ref.hi & (1ULL << (TNRK_BIT_BASE + kind))) != 0;
}

static inline unsigned
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_ref_request_kind(enum tn_ref_kind kind)
{
    return 1 << kind;
}

static inline tn_ref
TN_RESULT_IS_IMPORTANT
tn_ref_add(tn_ref ref, unsigned incr)
{
    tn_ref newref = ref;

    newref.lo += incr;

    if (newref.lo < ref.lo)
    {
        newref.hi++;
        TN_BUG_ON((newref.hi >> TNRK_BIT_BASE) != (ref.hi >> TNRK_BIT_BASE));
    }
    return newref;
}

static inline tn_ref
TN_RESULT_IS_IMPORTANT
tn_ref_relative(tn_ref ref, tn_rel_ref rel)
{
    tn_ref newref = tn_ref_add(ref, rel & TNRK_REL_MASK);
    newref.hi = (newref.hi & TNRK_MASK) |
        (((uint64_t)rel >> TNRK_REL_BIT_BASE) << TNRK_BIT_BASE);
    return newref;
}

static inline tn_rel_ref
TN_RESULT_IS_IMPORTANT
tn_ref_mkrel(tn_ref base, tn_ref ref)
{
    tn_ref newref = ref;

    newref.lo -= base.lo;
    newref.hi -= base.hi & TNRK_MASK;
    if (ref.lo < base.lo)
        newref.hi--;
    TN_BUG_ON((newref.hi >> TNRK_BIT_BASE) != (ref.hi >> TNRK_BIT_BASE));
    TN_BUG_ON((newref.hi & TNRK_MASK) != 0);
    TN_BUG_ON((newref.lo & ~TNRK_REL_MASK) != 0);

    return (tn_rel_ref)newref.lo |
        (ref.hi >> TNRK_BIT_BASE << TNRK_REL_BIT_BASE);
}

static inline tn_ref
TN_RESULT_IS_IMPORTANT
tn_ref_rebase(tn_ref ref, tn_ref new_base)
{
    tn_ref newref = ref;

    newref.lo += new_base.lo;
    if (newref.lo < ref.lo)
        newref.hi++;
    newref.hi += new_base.hi & TNRK_MASK;
    TN_BUG_ON((newref.hi >> TNRK_BIT_BASE) != (ref.hi >> TNRK_BIT_BASE));

    return newref;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_ref_eq(tn_ref ref1, tn_ref ref2)
{
    return ref1.lo == ref2.lo && ref1.hi == ref2.hi;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_ref_le(tn_ref ref1, tn_ref ref2)
{
    if (ref1.hi == ref2.hi)
        return ref1.lo <= ref2.lo;
    else
        return (ref1.hi & TNRK_MASK) < (ref2.hi & TNRK_MASK);
}

#ifndef UNIT_TESTING
#undef TNRK_REL_MASK
#undef TNRK_MASK
#undef TNRK_BIT_BASE
#undef TNRK_REL_BIT_BASE
#endif

typedef struct tn_range {
    int32_t lo;
    int32_t hi;
} tn_range;

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_valid(tn_range r)
{
    return r.lo <= r.hi;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_contains(tn_range r, int32_t v)
{
    return v >= r.lo && v <= r.hi;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_subrange(tn_range r1, tn_range r2)
{
    return tn_range_contains(r2, r1.lo) && tn_range_contains(r2, r1.hi);
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_disjoint(tn_range r1, tn_range r2)
{
    return r1.hi < r2.lo || r2.hi < r1.lo;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_eq(tn_range r1, tn_range r2)
{
    return r1.lo == r2.lo && r1.hi == r2.hi;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_le(tn_range r1, tn_range r2)
{
    return r1.lo < r2.lo || (r1.lo == r2.lo && r1.hi <= r2.hi);
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_min(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r1 : r2;
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_max(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r2 : r1;
}

static inline bool
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_connected(tn_range r1, tn_range r2)
{
    int32_t mhi = r1.hi < r2.hi ? r1.hi : r2.hi;
    int32_t mlo = r1.lo > r2.lo ? r1.lo : r2.lo;

    return mhi == INT32_MAX || mhi + 1 >= mlo;
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_span_fast(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r1.lo : r2.lo,
            .hi = r1.hi > r2.hi ? r1.hi : r2.hi
            };
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_span(tn_range r1, tn_range r2)
{
    if (!tn_range_valid(r1))
        return r2;
    if (!tn_range_valid(r2))
        return r1;

    return tn_range_span_fast(r1, r2);
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_intersect(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r2.lo : r1.lo,
            .hi = r1.hi > r2.hi ? r2.hi : r1.hi
            };
}

static inline tn_range
TN_RESULT_IS_IMPORTANT TN_NO_SHARED_STATE
tn_range_exclude(tn_range r1, tn_range r2)
{
    tn_range left = {r1.lo, r2.lo - 1};
    tn_range right = {r2.hi + 1, r1.hi};

    return tn_range_span(left, right);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_VALUES_H */
