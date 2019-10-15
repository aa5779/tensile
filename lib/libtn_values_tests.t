#include <time.h>
#include "values.h"

static tn_ref
random_tn_ref(void)
{
    tn_ref ref;
    ref.lo = (uint64_t)random() << 32;
    ref.lo |= random();
    ref.hi = (uint64_t)random() << 32;
    ref.hi |= random();
    ref.hi &= ~(1ULL << (TNRK_BIT_BASE - 1));
    return ref;
}

#suite Values

#tcase References

#test test_ref_incr_zero
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2 = tn_ref_add(ref1, 0);
      ck_assert(tn_ref_eq(ref1, ref2));

#test-loop(0,100) test_ref_incr_nzero
      tn_ref ref1 = random_tn_ref();
      unsigned r = (unsigned)((uint64_t)random() + 1);
      tn_ref ref2 = tn_ref_add(ref1, r);
      ck_assert(!tn_ref_eq(ref1, ref2));
      ck_assert(tn_ref_le(ref1, ref2));
      ck_assert_uint_eq(ref1.hi >> TNRK_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

#test test_ref_incr_overflow_lo
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2;
      unsigned r;
      ref1.lo |= 0xffffffff00000000ull;
      r = (unsigned)(~0ull - ref1.lo) + 1;
      ref2 = tn_ref_add(ref1, r);
      ck_assert_uint_eq(ref2.hi, ref1.hi + 1);
      ck_assert_uint_eq(ref2.lo, 0);

#test test_ref_incr_delta_zero
      tn_ref ref = random_tn_ref();
      tn_rel_ref rel = tn_ref_mkrel(ref, ref);
      ck_assert_uint_eq(rel & TNRK_REL_MASK, 0);

#test-loop(0,100) test_ref_delta_nzero
      tn_ref ref1 = random_tn_ref();
      unsigned r = (unsigned)((random() & TNRK_REL_MASK) + 1);
      tn_ref ref2 = tn_ref_add(ref1, r);
      tn_rel_ref rel = tn_ref_mkrel(ref1, ref2);
      ck_assert_uint_eq(rel & TNRK_REL_MASK, r);
      ck_assert_uint_eq(rel >> TNRK_REL_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

#test test_ref_delta_overflow_lo
      tn_ref ref1;
      tn_ref ref2;
      unsigned r;
      tn_rel_ref rel;
      ref1.hi = random();
      r = random() & TNRK_REL_MASK;
      ref1.lo = ~0ull - (uint64_t)r;
      ref2.hi = ref1.hi + 1;
      ref2.lo = 0ull;
      rel = tn_ref_mkrel(ref1, ref2);
      ck_assert_uint_eq(rel & TNRK_REL_MASK, r + 1);

#test-loop(0,100) test_ref_rel
      tn_ref ref1 = random_tn_ref();
      tn_rel_ref rel = random();
      tn_ref ref2 = tn_ref_relative(ref1, rel);
      tn_rel_ref rel2 = tn_ref_mkrel(ref1, ref2);
      ck_assert_uint_eq(rel, rel2);
      ck_assert_uint_eq(rel >> TNRK_REL_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

#test test_ref_eq_self
      tn_ref ref = random_tn_ref();
      ck_assert(tn_ref_eq(ref, ref));
      ck_assert(tn_ref_le(ref, ref));

#test test_ref_neq
      tn_ref ref = random_tn_ref();
      tn_ref ref1 = {~ref.hi, ref.lo};
      ck_assert(!tn_ref_eq(ref, ref1));

#test test_ref_le
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2 = random_tn_ref();
      ref2.hi = (ref2.hi & TNRK_MASK) | (ref1.hi & ~TNRK_MASK);
      ck_assert(tn_ref_le(ref1, ref2) || tn_ref_le(ref2, ref1));

#test-loop(0,3) test_ref_kind
      tn_ref ref = {(uint64_t)tn_ref_request_kind(_i) << TNRK_BIT_BASE,
                    (uint64_t)random()};
      ck_assert(tn_ref_is_kind(ref, _i));

#main-pre
    srandom(time(NULL));