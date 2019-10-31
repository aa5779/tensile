#include <sys/time.h>
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

static tn_range
random_tn_range(void)
{
    tn_range r;
    r.lo = tn_random_int(INT32_MIN, INT32_MAX - 1);
    r.hi = tn_random_int(r.lo, INT32_MAX);
    return r;
}

SUITE(Values)

TCASE(References)

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

TCASE(Ranges)

TEST(test_range_rand_sanity)
      tn_range r = random_tn_range();
      ck_assert(tn_range_valid(r));

TEST(test_range_eq_refl, OK, ONCE)
      int v = tn_random_int(INT32_MIN, INT32_MAX);
      tn_range r = {v, v};
      ck_assert(tn_range_eq(r, r));

TEST(test_range_le_refl, OK, ONCE)
      int v = tn_random_int(INT32_MIN, INT32_MAX);
      tn_range r = {v, v};
      ck_assert(tn_range_le(r, r));

TEST(test_range_subrange_refl, OK, ONCE)
      int v = tn_random_int(INT32_MIN, INT32_MAX);
      tn_range r = {v, v};
      ck_assert(tn_range_subrange(r, r));

TEST(test_range_connected_refl, OK, ONCE)
      int v = tn_random_int(INT32_MIN, INT32_MAX);
      tn_range r = {v, v};
      ck_assert(tn_range_connected(r, r));

TEST(test_range_disjoint_antirefl, OK, ONCE)
      int v = tn_random_int(INT32_MIN, INT32_MAX);
      tn_range r = {v, v};
      ck_assert(!tn_range_disjoint(r, r));

TEST(test_range_le_total)
      tn_range r1 = random_tn_range();
      tn_range r2 = random_tn_range();
      ck_assert(tn_range_le(r1, r2) || tn_range_le(r2, r1));

TEST(test_range_span)
      tn_range r1 = random_tn_range();
      tn_range r2 = random_tn_range();
      tn_range span = tn_range_span(r1, r2);
      ck_assert(tn_range_valid(span));
      ck_assert(tn_range_subrange(r1, span));
      ck_assert(tn_range_subrange(r2, span));

TEST(test_range_complete, OK, ONCE)
      tn_range all = {INT32_MIN, INT32_MAX};
      tn_range r = random_tn_range();
      ck_assert(tn_range_le(all, r));
      ck_assert(tn_range_subrange(r, all));
      ck_assert(!tn_range_disjoint(r, all));
      ck_assert(tn_range_connected(r, all));
      ck_assert(tn_range_eq(all, tn_range_span(r, all)));
      ck_assert(tn_range_eq(r, tn_range_intersect(r, all)));

TEST(test_range_split)
      int32_t one = tn_random_int(INT32_MIN, INT32_MAX - 2);
      int32_t two = tn_random_int(one + 1, INT32_MAX - 1);
      int32_t three = tn_random_int(two + 1, INT32_MAX);
      tn_range r1 = {one, two - 1};
      tn_range r2 = {two, three};
      tn_range r3 = {one, three};
      ck_assert(tn_range_disjoint(r1, r2));
      ck_assert(tn_range_connected(r1, r2));
      ck_assert(tn_range_eq(tn_range_span(r1, r2), r3));
      ck_assert(!tn_range_valid(tn_range_intersect(r1, r2)));
      ck_assert(tn_range_eq(tn_range_exclude(r3, r1), r2));
      ck_assert(tn_range_eq(tn_range_exclude(r3, r2), r1));
      ck_assert(tn_range_eq(tn_range_exclude(r1, r2), r1));
      ck_assert(tn_range_eq(tn_range_exclude(r2, r1), r2));

TEST(test_range_exclude_subrange_right, OK, ONCE)
      tn_range r1 = random_tn_range();
      tn_range r2;
      tn_range rx;

      r2.lo = tn_random_int(r1.lo, r1.hi);
      r2.hi = r1.hi;
      rx = tn_range_exclude(r1, r2);
      ck_assert(!tn_range_subrange(r2, rx));

TEST(test_range_exclude_subrange_left, OK, ONCE)
      tn_range r1 = random_tn_range();
      tn_range r2;
      tn_range rx;

      r2.hi = tn_random_int(r1.lo, r1.hi);
      r2.lo = r1.lo;
      rx = tn_range_exclude(r1, r2);
      ck_assert(!tn_range_subrange(r2, rx));

TEST(test_range_disjoint_symm)
      tn_range r1 = random_tn_range();
      tn_range r2 = random_tn_range();

      ck_assert_int_eq((int)tn_range_disjoint(r1, r2),
                       (int)tn_range_disjoint(r2, r1));

TEST(test_range_connected_symm)
      tn_range r1 = random_tn_range();
      tn_range r2 = random_tn_range();

      ck_assert_int_eq((int)tn_range_connected(r1, r2),
                       (int)tn_range_connected(r2, r1));

TEST(test_range_connected_max, OK, ONCE)
      tn_range r = random_tn_range();
      ck_assert(tn_range_connected(r, (tn_range){INT32_MIN, INT32_MAX}));

TEST(test_range_not_disjoint_max, OK, ONCE)
      tn_range r = random_tn_range();
      ck_assert(!tn_range_disjoint(r, (tn_range){INT32_MIN, INT32_MAX}));

TEST(test_range_minmax, OK, ONCE)
      tn_range r1 = random_tn_range();
      tn_range r2 = random_tn_range();
      ck_assert(tn_range_le(r1, tn_range_max(r1, r2)));
      ck_assert(tn_range_le(r2, tn_range_max(r1, r2)));
      ck_assert(tn_range_le(tn_range_min(r1, r2), r1));
      ck_assert(tn_range_le(tn_range_min(r1, r2), r2));
