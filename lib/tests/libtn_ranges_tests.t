#include "ranges.h"

static tn_range
random_tn_range(void)
{
    tn_range r;
    r.lo = tn_random_int(INT32_MIN, INT32_MAX - 1);
    r.hi = tn_random_int(r.lo, INT32_MAX);
    return r;
}

SUITE(Ranges)

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
