/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * Ranges.
 */
#ifndef TNH_RANGES_H
#define TNH_RANGES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "values.h"

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_valid(tn_range r)
{
    return r.lo <= r.hi;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_contains(tn_range r, int32_t v)
{
    return v >= r.lo && v <= r.hi;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_subrange(tn_range r1, tn_range r2)
{
    return tn_range_contains(r2, r1.lo) && tn_range_contains(r2, r1.hi);
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_disjoint(tn_range r1, tn_range r2)
{
    return r1.hi < r2.lo || r2.hi < r1.lo;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_eq(tn_range r1, tn_range r2)
{
    return r1.lo == r2.lo && r1.hi == r2.hi;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_le(tn_range r1, tn_range r2)
{
    return r1.lo < r2.lo || (r1.lo == r2.lo && r1.hi <= r2.hi);
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_min(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r1 : r2;
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_max(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r2 : r1;
}

/**
 * @undocumented
 */
static inline bool TN_NO_SHARED_STATE
tn_range_connected(tn_range r1, tn_range r2)
{
    int32_t mhi = r1.hi < r2.hi ? r1.hi : r2.hi;
    int32_t mlo = r1.lo > r2.lo ? r1.lo : r2.lo;

    return mhi == INT32_MAX || mhi + 1 >= mlo;
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_span_fast(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r1.lo : r2.lo,
            .hi = r1.hi > r2.hi ? r1.hi : r2.hi
            };
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_span(tn_range r1, tn_range r2)
{
    if (!tn_range_valid(r1))
        return r2;
    if (!tn_range_valid(r2))
        return r1;

    return tn_range_span_fast(r1, r2);
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_intersect(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r2.lo : r1.lo,
            .hi = r1.hi > r2.hi ? r2.hi : r1.hi
            };
}

/**
 * @undocumented
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_exclude(tn_range r1, tn_range r2)
{
    tn_range left = {r1.lo, r2.lo - 1};
    tn_range right = {r2.hi + 1, r1.hi};

    return tn_range_span(left, right);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_RANGES_H */
