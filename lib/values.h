/** @file
 * @brief
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef VALUES_H
#define VALUES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unitypes.h>

enum tn_value_type {
    TNV_NULL,
    TNV_INTEGER,
    TNV_FLOAT,
    TNV_CHAR,
    TNV_STRING,
    TNV_BSTRING,
    TNV_CHARSET,
    TNV_EDITSTRING,
    TNV_VECTOR,
    TNV_RANGE,
    TNV_REF,
    TNV_WEAKREF,
    TNV_SLICE,
    TNV_ARRAY,
    TNV_TABLE,
    TNV_NODE,
    TNV_COMPOUND,
};

typedef struct tn_ref {
    uint64_t hi;
    uint64_t lo;
} tn_ref;


enum tn_ref_kind {
    TNRK_STATIC,
    TNRK_CONST,
    TNRK_VOLATILE,
};

#define TNRK_BIT_BASE 56
#define TNRK_MASK ((1ULL << TNRK_BIT_BASE) - 1)

static inline bool
tn_ref_check(const tn_ref *ref, enum tn_ref_kind kind)
{
    return (ref->hi & (1ULL << (TNRK_BIT_BASE + kind))) != 0;
}

static inline void
tn_ref_incr(tn_ref *ref, unsigned incr)
{
    uint64_t newlo = ref->lo + incr;

    if (newlo <= ref->lo)
    {
        uint64_t newhigh = ref->hi + 1;
        if ((newhigh >> TNRK_BIT_BASE)  != (ref->hi >> TNRK_BIT_BASE))
            abort();
        ref->hi = newhigh;
    }
    ref->lo = newlo;
}

static inline bool
tn_ref_eq(const tn_ref *ref1, const tn_ref *ref2)
{
    return ref1->lo == ref2->lo && ref1->hi == ref2->hi;
}

static inline bool
tn_ref_le(const tn_ref *ref1, const tn_ref *ref2)
{
    if (ref1->hi == ref2->hi)
        return ref1->lo <= ref2->lo;
    else
        return (ref1->hi & TNRK_MASK) < (ref2->hi & TNRK_MASK);
}    

#undef TNRK_MASK
#undef TNRK_BIT_BASE

union 

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* VALUES_H */
