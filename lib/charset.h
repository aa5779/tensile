/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * Character sets.
 */
#ifndef TNH_CHARSET_H
#define TNH_CHARSET_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "values.h"

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern bool tn_charset_valid(size_t len,
                             const tn_charset_range set[TN_VAR_SIZE(len)]);

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern unsigned tn_charset_cardinality(size_t len,
                                       const tn_charset_range \
                                       set[TN_VAR_SIZE(len)]);

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern ucs4_t tn_charset_nth(size_t len,
                             const tn_charset_range set[TN_VAR_SIZE(len)],
                             unsigned i);

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern bool tn_charset_contains(size_t len,
                                const tn_charset_range set[TN_VAR_SIZE(len)],
                                ucs4_t ch);

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
static inline ucs4_t
tn_charset_min(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    return len == 0 ? TN_INVALID_CHAR : set[0].lo;
}

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
static inline ucs4_t
tn_charset_max(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    return len == 0 ? TN_INVALID_CHAR : set[len - 1].hi;
}

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern bool tn_charset_subset(size_t sublen,
                              const tn_charset_range        \
                              subset[TN_VAR_SIZE(sublen)],
                              size_t superlen,
                              const tn_charset_range            \
                              superset[TN_VAR_SIZE(superlen)]);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_charset_generate_union(size_t len1,
                                      const tn_charset_range    \
                                      set1[TN_VAR_SIZE(len1)],
                                      size_t len2,
                                      const tn_charset_range    \
                                      set2[TN_VAR_SIZE(len2)],
                                      tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_charset_generate_intersect(size_t len1,
                                          const tn_charset_range    \
                                          set1[TN_VAR_SIZE(len1)],
                                          size_t len2,
                                          const tn_charset_range    \
                                          set2[TN_VAR_SIZE(len2)],
                                          tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(3)
extern void tn_charset_generate_complement(size_t len,
                                           const tn_charset_range   \
                                           set[TN_VAR_SIZE(len)],
                                           tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_charset_generate_diff(size_t len1,
                                     const tn_charset_range     \
                                     set1[TN_VAR_SIZE(len1)],
                                     size_t len2,
                                     const tn_charset_range     \
                                     set2[TN_VAR_SIZE(len2)],
                                     tn_buffer *dest);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_CHARSET_H */
