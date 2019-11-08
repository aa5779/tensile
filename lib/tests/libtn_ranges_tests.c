#include "ranges.h"
#include "testing.h"

static void
tn_range_generate(TN_UNUSED unsigned _i, tn_range *r)
{
    r->lo = tn_random_int(INT32_MIN, INT32_MAX - 1);
    r->hi = tn_random_int(r->lo, INT32_MAX);
}

#define tn_range_cleanup(_ref) ((void)0)

TESTDEF(test_range_rand_sanity, "Random range is valid")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_valid(r)));
}

TESTDEF_SINGLE(test_range_eq_refl, "Range equality is reflexive")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_eq(r, r)));
}

TESTDEF_SINGLE(test_range_le_refl, "Range ordering is reflexive")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_le(r, r)));
}

TESTDEF_SINGLE(test_range_subrange_refl, "Subrange relation is reflexive")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_subrange(r, r)));
}

TESTDEF_SINGLE(test_range_connected_refl, "Connected relation is reflexive")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_connected(r, r)));
}

TESTDEF_SINGLE(test_range_disjoint_antirefl,
               "Disjoint relation is antireflexive")
{
    FORALL(tn_range, r,
           tnt_assert(!tn_range_disjoint(r, r)));
}

TESTDEF(test_range_le_total, "Range ordering is total")
{
    FORALL(tn_range, r1,
           FORALL(tn_range, r2,
                  tnt_assert(tn_range_le(r1, r2) ||
                             tn_range_le(r2, r1))));
}

TESTDEF(test_range_span, "Merging two ranges is valid")
{
    FORALL(tn_range, r1,
           FORALL(tn_range, r2,
                  tn_range span = tn_range_span(r1, r2);
                  tnt_assert(tn_range_valid(span));
                  tnt_assert(tn_range_subrange(r1, span));
                  tnt_assert(tn_range_subrange(r2, span))));
}

TESTDEF_SINGLE(test_range_complete,
               "Merging with the full range yields the full range")
{
    tn_range all = {INT32_MIN, INT32_MAX};
    FORALL(tn_range, r,
           tnt_assert(tn_range_le(all, r));
           tnt_assert(tn_range_subrange(r, all));
           tnt_assert(!tn_range_disjoint(r, all));
           tnt_assert(tn_range_connected(r, all));
           tnt_assert(tn_range_eq(all, tn_range_span(r, all)));
           tnt_assert(tn_range_eq(r, tn_range_intersect(r, all))));
}


TESTDEF(test_range_split, "A split range is disjoint")
{
    int32_t one = tn_random_int(INT32_MIN, INT32_MAX - 2);
    int32_t two = tn_random_int(one + 1, INT32_MAX - 1);
    int32_t three = tn_random_int(two + 1, INT32_MAX);
    tn_range r1 = {one, two - 1};
    tn_range r2 = {two, three};
    tn_range r3 = {one, three};

    tnt_assert(tn_range_disjoint(r1, r2));
    tnt_assert(tn_range_connected(r1, r2));
    tnt_assert(tn_range_eq(tn_range_span(r1, r2), r3));
    tnt_assert(!tn_range_valid(tn_range_intersect(r1, r2)));
    tnt_assert(tn_range_eq(tn_range_exclude(r3, r1), r2));
    tnt_assert(tn_range_eq(tn_range_exclude(r3, r2), r1));
    tnt_assert(tn_range_eq(tn_range_exclude(r1, r2), r1));
    tnt_assert(tn_range_eq(tn_range_exclude(r2, r1), r2));
}

TESTDEF_SINGLE(test_range_exclude_subrange_right,
               "An excluded suffix range is not a subrange")
{
    FORALL(tn_range, r1,
           tn_range r2;
           tn_range rx;

           r2.lo = tn_random_int(r1.lo, r1.hi);
           r2.hi = r1.hi;
           rx = tn_range_exclude(r1, r2);
           tnt_assert(!tn_range_subrange(r2, rx)));
}

TESTDEF_SINGLE(test_range_exclude_subrange_left,
               "An excluded prefix range is not a subrange")
{
    FORALL(tn_range, r1,
           tn_range r2;
           tn_range rx;

           r2.hi = tn_random_int(r1.lo, r1.hi);
           r2.lo = r1.lo;
           rx = tn_range_exclude(r1, r2);
           tnt_assert(!tn_range_subrange(r2, rx)));
}

TESTDEF(test_range_disjoint_symm, "Disjoint relation is symmetric")
{
    FORALL(tn_range, r1,
           FORALL(tn_range, r2,
                  tnt_assert_equiv(tn_range_disjoint(r1, r2),
                                   tn_range_disjoint(r2, r1))));
}

TESTDEF(test_range_connected_symm, "Connected relation is symmetric")
{
    FORALL(tn_range, r1,
           FORALL(tn_range, r2,
                  tnt_assert_equiv(tn_range_connected(r1, r2),
                                   tn_range_connected(r2, r1))));
}

TESTDEF_SINGLE(test_range_connected_max,
               "Everything is connected with the full range")
{
    FORALL(tn_range, r,
           tnt_assert(tn_range_connected(r,
                                         (tn_range){INT32_MIN, INT32_MAX})));
}

TESTDEF_SINGLE(test_range_not_disjoint_max,
               "Nothing is disjoint with the full range")
{
    FORALL(tn_range, r,
           tnt_assert(!tn_range_disjoint(r,
                                         (tn_range){INT32_MIN, INT32_MAX})));
}

TESTDEF_SINGLE(test_range_minmax,
               "Maximal and minimal ranges are properly ordered")
{
    FORALL(tn_range, r1,
           FORALL(tn_range, r2,
                  tnt_assert(tn_range_le(r1, tn_range_max(r1, r2)));
                  tnt_assert(tn_range_le(r2, tn_range_max(r1, r2)));
                  tnt_assert(tn_range_le(tn_range_min(r1, r2), r1));
                  tnt_assert(tn_range_le(tn_range_min(r1, r2), r2))));
}
