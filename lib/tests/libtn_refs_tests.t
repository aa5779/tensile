#include "refs.h"

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

SUITE(References)

TEST(test_ref_incr_zero, OK, ONCE)
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2 = tn_ref_add(ref1, 0);
      ck_assert(tn_ref_eq(ref1, ref2));

TEST(test_ref_incr_nzero)
      tn_ref ref1 = random_tn_ref();
      unsigned r = (unsigned)((uint64_t)random() + 1);
      tn_ref ref2 = tn_ref_add(ref1, r);
      ck_assert(!tn_ref_eq(ref1, ref2));
      ck_assert(tn_ref_le(ref1, ref2));
      ck_assert_uint_eq(ref1.hi >> TNRK_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

TEST(test_ref_incr_overflow_lo, OK, ONCE)
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2;
      unsigned r;
      ref1.lo |= 0xffffffff00000000ull;
      r = (unsigned)(~0ull - ref1.lo) + 1;
      ref2 = tn_ref_add(ref1, r);
      ck_assert_uint_eq(ref2.hi, ref1.hi + 1);
      ck_assert_uint_eq(ref2.lo, 0);

TEST(test_ref_incr_delta_zero, OK, ONCE)
      tn_ref ref = random_tn_ref();
      tn_rel_ref rel = tn_ref_mkrel(ref, ref);
      ck_assert_uint_eq(rel & TNRK_REL_MASK, 0);

TEST(test_ref_delta_nzero)
      tn_ref ref1 = random_tn_ref();
      unsigned r = (unsigned)((random() & TNRK_REL_MASK) + 1);
      tn_ref ref2 = tn_ref_add(ref1, r);
      tn_rel_ref rel = tn_ref_mkrel(ref1, ref2);
      ck_assert_uint_eq(rel & TNRK_REL_MASK, r);
      ck_assert_uint_eq(rel >> TNRK_REL_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

TEST(test_ref_delta_overflow_lo, OK, ONCE)
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

TEST(test_ref_rel)
      tn_ref ref1 = random_tn_ref();
      tn_rel_ref rel = random();
      tn_ref ref2 = tn_ref_relative(ref1, rel);
      tn_rel_ref rel2 = tn_ref_mkrel(ref1, ref2);
      ck_assert_uint_eq(rel, rel2);
      ck_assert_uint_eq(rel >> TNRK_REL_BIT_BASE, ref2.hi >> TNRK_BIT_BASE);

TEST(test_ref_rebase)
      tn_ref ref = random_tn_ref();
      tn_ref rebase = tn_ref_rebase(ref, ref);
      ck_assert_uint_eq(rebase.lo, ref.lo << 1);
      ck_assert_uint_eq(rebase.hi & TNRK_MASK, (ref.hi & TNRK_MASK) << 1);
      ck_assert_uint_eq(rebase.hi >> TNRK_BIT_BASE, ref.hi >> TNRK_BIT_BASE);

TEST(test_ref_rebase_carry, OK, ONCE)
      tn_ref ref = {random(), ~0ull};
      tn_ref base = {0, 1};
      tn_ref rebase = tn_ref_rebase(ref, base);
      ck_assert_uint_eq(rebase.lo, 0);
      ck_assert_uint_eq(rebase.hi, ref.hi + 1);

TEST(test_ref_eq_self, OK, ONCE)
      tn_ref ref = random_tn_ref();
      ck_assert(tn_ref_eq(ref, ref));
      ck_assert(tn_ref_le(ref, ref));

TEST(test_ref_neq, OK, ONCE)
      tn_ref ref = random_tn_ref();
      tn_ref ref1 = {~ref.hi, ref.lo};
      ck_assert(!tn_ref_eq(ref, ref1));

TEST(test_ref_le, OK, ONCE)
      tn_ref ref1 = random_tn_ref();
      tn_ref ref2 = random_tn_ref();
      ref2.hi = (ref2.hi & TNRK_MASK) | (ref1.hi & ~TNRK_MASK);
      ck_assert(tn_ref_le(ref1, ref2) || tn_ref_le(ref2, ref1));

TEST(test_ref_kind, OK, RANGE(TNRK_STATIC, TNRK_OP_INDIRECT))
      tn_ref ref = {(uint64_t)tn_ref_request_kind(_i) << TNRK_BIT_BASE,
                    (uint64_t)random()};
      ck_assert(tn_ref_is_kind(ref, _i));

