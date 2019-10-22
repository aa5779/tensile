/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
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
#include <uniname.h>
#include "utils.h"

/** @deftp Enum tn_value_type
 *  @undocumented
 *  @end deftp
 */
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

/** @node References
 *  @section References
 */

/** @deftp Struct tn_ref
 *  @undocumented
 *  @end deftp
 */
typedef struct tn_ref {
    uint64_t hi;
    uint64_t lo;
} tn_ref;

/** @deftp Type tn_rel_ref
 *  @undocumented
 *  @end deftp
 */
typedef uint32_t tn_rel_ref;

/** @deftp Enum tn_ref_kind
 *  @undocumented
 *  @end deftp
 */
enum tn_ref_kind {
    TNRK_STATIC,
    TNRK_CONST,
    TNRK_VOLATILE,
    TNRK_EXTERNAL,
    TNRK_SCRATCH,
    TNRK_OP_LOCAL,
    TNRK_OP_INDIRECT,
};

/**@ifset INTERNALS */
#define TNRK_BIT_BASE 56
#define TNRK_REL_BIT_BASE 24
#define TNRK_MASK ((1ULL << TNRK_BIT_BASE) - 1)
#define TNRK_REL_MASK ((1ULL << TNRK_REL_BIT_BASE) - 1)
/**@end ifset */

/** @deftypefun bool tn_ref_is_kind(tn_ref @var{ref}, @
 *                                  enum tn_ref_kind @var{kind})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_ref_is_kind(tn_ref ref, enum tn_ref_kind kind)
{
    return (ref.hi & (1ULL << (TNRK_BIT_BASE + kind))) != 0;
}

/** @deftypefun bool tn_ref_request_kind(enum tn_ref_kind @var{kind})
 *  @undocumented
 *  @end deftypefun
 */
static inline unsigned TN_NO_SHARED_STATE
tn_ref_request_kind(enum tn_ref_kind kind)
{
    return 1 << kind;
}

/** @deftypefun tn_ref tn_ref_add(tn_ref @var{ref}, @
 *                                unsigned @var{incr})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun tn_ref tn_ref_relative(tn_ref @var{ref}, @
 *                                     tn_ref @var{rel})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_ref TN_NO_SHARED_STATE
tn_ref_relative(tn_ref ref, tn_rel_ref rel)
{
    tn_ref newref = tn_ref_add(ref, rel & TNRK_REL_MASK);
    newref.hi = (newref.hi & TNRK_MASK) |
        (((uint64_t)rel >> TNRK_REL_BIT_BASE) << TNRK_BIT_BASE);
    return newref;
}

/** @deftypefun tn_rel_ref tn_ref_mkrel(tn_ref @var{base}, @
 *                                      tn_ref @var{ref})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun tn_ref tn_ref_rebase(tn_ref @var{ref}, @
 *                                   tn_ref @var{new_base})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun bool tn_ref_eq(tn_ref @var{ref1}, @
 *                             tn_ref @var{ref2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_ref_eq(tn_ref ref1, tn_ref ref2)
{
    return ref1.lo == ref2.lo && ref1.hi == ref2.hi;
}

/** @deftypefun bool tn_ref_le(tn_ref @var{ref1}, @
 *                             tn_ref @var{ref2})
 *  @undocumented
 *  @end deftypefun
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

/** @node Ranges
 *  @section Ranges
 */

/** @deftp Struct tn_range
 *  @undocumented
 *  @end deftp
 */
typedef struct tn_range {
    int32_t lo;
    int32_t hi;
} tn_range;

/** @deftypefun bool tn_range_valid(tn_range @var{r})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_valid(tn_range r)
{
    return r.lo <= r.hi;
}

/** @deftypefun bool tn_range_contains(tn_range @var{r}, int32_t v)
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_contains(tn_range r, int32_t v)
{
    return v >= r.lo && v <= r.hi;
}

/** @deftypefun bool tn_range_subrange(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_subrange(tn_range r1, tn_range r2)
{
    return tn_range_contains(r2, r1.lo) && tn_range_contains(r2, r1.hi);
}

/** @deftypefun bool tn_range_disjoint(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_disjoint(tn_range r1, tn_range r2)
{
    return r1.hi < r2.lo || r2.hi < r1.lo;
}

/** @deftypefun bool tn_range_eq(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_eq(tn_range r1, tn_range r2)
{
    return r1.lo == r2.lo && r1.hi == r2.hi;
}

/** @deftypefun bool tn_range_le(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_le(tn_range r1, tn_range r2)
{
    return r1.lo < r2.lo || (r1.lo == r2.lo && r1.hi <= r2.hi);
}

/** @deftypefun tn_range tn_range_min(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_min(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r1 : r2;
}

/** @deftypefun tn_range tn_range_max(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_max(tn_range r1, tn_range r2)
{
    return tn_range_le(r1, r2) ? r2 : r1;
}

/** @deftypefun bool tn_range_connected(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_NO_SHARED_STATE
tn_range_connected(tn_range r1, tn_range r2)
{
    int32_t mhi = r1.hi < r2.hi ? r1.hi : r2.hi;
    int32_t mlo = r1.lo > r2.lo ? r1.lo : r2.lo;

    return mhi == INT32_MAX || mhi + 1 >= mlo;
}

/** @deftypefun tn_range tn_range_span_fast(tn_range @var{r1}, @
 *                                          tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_span_fast(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r1.lo : r2.lo,
            .hi = r1.hi > r2.hi ? r1.hi : r2.hi
            };
}

/** @deftypefun tn_range tn_range_span(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun tn_range tn_range_intersect(tn_range @var{r1}, @
 *                                          tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_intersect(tn_range r1, tn_range r2)
{
    return (tn_range){.lo = r1.lo < r2.lo ? r2.lo : r1.lo,
            .hi = r1.hi > r2.hi ? r2.hi : r1.hi
            };
}

/** @deftypefun tn_range tn_range_exclude(tn_range @var{r1}, tn_range @var{r2})
 *  @undocumented
 *  @end deftypefun
 */
static inline tn_range TN_NO_SHARED_STATE
tn_range_exclude(tn_range r1, tn_range r2)
{
    tn_range left = {r1.lo, r2.lo - 1};
    tn_range right = {r2.hi + 1, r1.hi};

    return tn_range_span(left, right);
}

/** @node Charsets
 *  @section Charsets
 */

/** @deftp Struct tn_charset_range
 *  @undocumented
 *  @end deftp
 */
typedef struct tn_charset_range {
    ucs4_t lo;
    ucs4_t hi;
} tn_charset_range;

/** @deftypefun bool tn_charset_valid(size_t @var{len}, @
 *                                const tn_charset_range @
 *                                @var{set}[@var{len}])
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
extern bool tn_charset_valid(size_t len,
                             const tn_charset_range set[TN_VAR_SIZE(len)]);

/** @deftypefun unsigned tn_charset_cardinality(size_t @var{len}, @
 *                                              const tn_charset_range @
 *                                              @var{set}[@var{len}])
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
extern unsigned tn_charset_cardinality(size_t len,
                                       const tn_charset_range \
                                       set[TN_VAR_SIZE(len)]);

/** @deftypefun ucs4_t tn_charset_nth(size_t @var{len}, @
 *                                    const tn_charset_range @
 *                                    @var{set}[@var{len}], @
 *                                    unsigned @var{i})
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
extern ucs4_t tn_charset_nth(size_t len,
                             const tn_charset_range set[TN_VAR_SIZE(len)],
                             unsigned i);

/** @deftypefun bool tn_charset_contains(size_t @var{len}, @
 *                                       const tn_charset_range @
 *                                       @var{set}[@var{len}], @
 *                                       ucs4_t ch)
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
extern bool tn_charset_contains(size_t len,
                                const tn_charset_range set[TN_VAR_SIZE(len)],
                                ucs4_t ch);

/** @deftypefun ucs4_t tn_charset_min(size_t @var{len}, @
 *                                    const tn_charset_range @
 *                                    @var{set}[@var{len}])
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
static inline ucs4_t
tn_charset_min(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    return len == 0 ? UNINAME_INVALID : set[0].lo;
}

/** @deftypefun ucs4_t tn_charset_max(size_t @var{len}, @
 *                                    const tn_charset_range @
 *                                    @var{set}[@var{len}])
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
static inline ucs4_t
tn_charset_max(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    return len == 0 ? UNINAME_INVALID : set[len - 1].hi;
}

/** @deftypefun bool tn_charset_subset(size_t @var{sublen}, @
 *                                     const tn_charset_range @
 *                                     @var{subset}[@var{sublen}],
 *                                     size_t @var{superlen}, @
 *                                     const tn_charset_range @
 *                                     @var{superset}[@var{superlen}])
 *  @undocumented
 *  @end deftypefun
 */
TN_NO_SHARED_STATE
extern bool tn_charset_subset(size_t sublen,
                              const tn_charset_range        \
                              subset[TN_VAR_SIZE(sublen)],
                              size_t superlen,
                              const tn_charset_range            \
                              superset[TN_VAR_SIZE(superlen)]);

/** @deftypefun bool tn_charset_generate_union(size_t @var{len1}, @
 *                                             const tn_charset_range @
 *                                             @var{set1}[@var{len1}],
 *                                             size_t @var{len2}, @
 *                                             const tn_charset_range @
 *                                             @var{set2}[@var{len2}], @
 *                                             tn_buffer *@var{dest})
 *  @undocumented
 *  @end deftypefun
 */
TN_NOT_NULL_ARGS(5)
extern void tn_charset_generate_union(size_t len1,
                                      const tn_charset_range    \
                                      set1[TN_VAR_SIZE(len1)],
                                      size_t len2,
                                      const tn_charset_range    \
                                      set2[TN_VAR_SIZE(len2)],
                                      tn_buffer *dest);

/** @deftypefun bool tn_charset_generate_intersect(size_t @var{len1}, @
 *                                                 const tn_charset_range @
 *                                                 @var{set1}[@var{len1}],
 *                                                 size_t @var{len2}, @
 *                                                 const tn_charset_range @
 *                                                 @var{set2}[@var{len2}], @
 *                                                 tn_buffer *@var{dest})
 *  @undocumented
 *  @end deftypefun
 */
TN_NOT_NULL_ARGS(5)
extern void tn_charset_generate_intersect(size_t len1,
                                          const tn_charset_range    \
                                          set1[TN_VAR_SIZE(len1)],
                                          size_t len2,
                                          const tn_charset_range    \
                                          set2[TN_VAR_SIZE(len2)],
                                          tn_buffer *dest);

/** @deftypefun bool tn_charset_generate_complement(size_t @var{len}, @
 *                                                 const tn_charset_range @
 *                                                 @var{set}[@var{len}],
 *                                                 size_t @var{len2}, @
 *                                                 const tn_charset_range @
 *                                                 @var{set2}[@var{len2}], @
 *                                                 tn_buffer *@var{dest})
 *  @undocumented
 *  @end deftypefun
 */
TN_NOT_NULL_ARGS(3)
extern void tn_charset_generate_complement(size_t len,
                                           const tn_charset_range   \
                                           set[TN_VAR_SIZE(len)],
                                           tn_buffer *dest);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_VALUES_H */
