/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * References.
 */
#ifndef TNH_REFS_H
#define TNH_REFS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "values.h"

#ifndef __DOXYGEN__
#define TNRK_BIT_BASE 56
#define TNRK_REL_BIT_BASE 24
#define TNRK_MASK ((1ULL << TNRK_BIT_BASE) - 1)
#define TNRK_REL_MASK ((1ULL << TNRK_REL_BIT_BASE) - 1)
#endif

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_ref_is_kind(tn_ref ref, enum tn_ref_kind kind)
{
    return (ref.hi & (1ULL << (TNRK_BIT_BASE + kind))) != 0;
}

/**
 * @undocumented
 */
static inline unsigned TN_NO_SHARED_STATE
tn_ref_request_kind(enum tn_ref_kind kind)
{
    return 1 << kind;
}

/**
 * @undocumented
 */
static inline tn_ref TN_NO_SHARED_STATE
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

/**
 * @undocumented
 */
static inline tn_ref TN_NO_SHARED_STATE
tn_ref_relative(tn_ref ref, tn_rel_ref rel)
{
    tn_ref newref = tn_ref_add(ref, rel & TNRK_REL_MASK);
    newref.hi = (newref.hi & TNRK_MASK) |
        (((uint64_t)rel >> TNRK_REL_BIT_BASE) << TNRK_BIT_BASE);
    return newref;
}

/**
 * @undocumented
 */
static inline tn_rel_ref TN_NO_SHARED_STATE
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

/**
 * @undocumented
 */
static inline tn_ref TN_NO_SHARED_STATE
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

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_ref_eq(tn_ref ref1, tn_ref ref2)
{
    return ref1.lo == ref2.lo && ref1.hi == ref2.hi;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_REFS_H */
