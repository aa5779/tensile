#include <sys/time.h>
#include "values.h"

static void
init_random(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);
}

static tn_charset_range *
generate_random_ranges(unsigned *n)
{
#define MAX_CS_RANGES 16
    unsigned i;
    ucs4_t min = 0;
    ucs4_t max;
    tn_charset_range *dest = NULL;

    *n = (unsigned)tn_random_int(1, MAX_CS_RANGES);
    max = INT32_MAX - 2 * (*n - 1);
    tn_alloc_raw(TN_GLOC(dest), sizeof(*dest) * (*n));

    for (i = 0; i < *n; i++)
    {
        tn_charset_range *r = &dest[i];
        r->lo = (ucs4_t)tn_random_int(min, max);
        r->hi = (ucs4_t)tn_random_int(r->lo, max);
        min = r->hi + 2;
        max += 2;
    }
    return dest;
#undef MAX_CS_RANGES
}

#test-loop(0,100) test_charset_sanity
     unsigned n;
     tn_charset_range *cs;

     cs = generate_random_ranges(&n);
     ck_assert(tn_charset_valid(n, cs));
     tn_free(TN_GLOC(cs));

#test test_charset_empty_valid
     ck_assert(tn_charset_valid(0, NULL));

#test test_charset_invalid1
      tn_charset_range invalid = {INT32_MAX, 0};
      ck_assert(!tn_charset_valid(1, &invalid));

#test test_charset_invalid2
      tn_charset_range invalid[2] = {{10, 20}, {40, 30}};
      ck_assert(!tn_charset_valid(2, invalid));

#test test_charset_invalid_order
      tn_charset_range invalid[2] = {{10, 20}, {0, 5}};
      ck_assert(!tn_charset_valid(2, invalid));

#test test_charset_invalid_connected
      tn_charset_range invalid[2] = {{10, 20}, {21, 30}};
      ck_assert(!tn_charset_valid(2, invalid));

#test-loop(0,100) test_charset_contains
     unsigned n;
     tn_charset_range *cs;
     unsigned i;
     ucs4_t ch;

     cs = generate_random_ranges(&n);
     i = tn_random_int(0, n - 1);
     ch = tn_random_int(cs[i].lo, cs[i].hi);
     ck_assert(tn_charset_contains(n, cs, ch));
     tn_free(TN_GLOC(cs));

#test test_charset_empty_subset
     unsigned n;
     tn_charset_range *cs;

     cs = generate_random_ranges(&n);
     ck_assert(tn_charset_subset(0, NULL, n, cs));
     ck_assert(!tn_charset_subset(n, cs, 0, NULL));
     tn_free(TN_GLOC(cs));

#test test_charset_subset_full
     tn_charset_range full = {0, INT32_MAX};
     unsigned n;
     tn_charset_range *cs;

     cs = generate_random_ranges(&n);
     ck_assert(tn_charset_subset(n, cs, 1, &full));
     tn_free(TN_GLOC(cs));

#test test_charset_subset
     unsigned n;
     tn_charset_range *cs;

     cs = generate_random_ranges(&n);
     ck_assert(tn_charset_subset(n - 1, cs + 1, n, cs));
     ck_assert(tn_charset_subset(n - 1, cs, n, cs));
     ck_assert(!tn_charset_subset(n, cs, n - 1, cs + 1));
     ck_assert(!tn_charset_subset(n, cs, n - 1, cs));
     tn_free(TN_GLOC(cs));

#test test_charset_subset_prefix
     tn_charset_range r1;
     tn_charset_range r2;

     r1.lo = tn_random_int(0, INT32_MAX - 1);
     r1.hi = tn_random_int(r1.lo, INT32_MAX - 1);
     r2.lo = r1.lo;
     r2.hi = tn_random_int(r1.hi + 1, INT32_MAX);
     ck_assert(tn_charset_subset(1, &r1, 1, &r2));
     ck_assert(!tn_charset_subset(1, &r2, 1, &r1));

#test test_charset_min
     unsigned n;
     tn_charset_range *cs;
     ucs4_t ch;
     ucs4_t minch;

     cs = generate_random_ranges(&n);
     minch = tn_charset_min(n, cs);
     ch = tn_charset_nth(n, cs, 0);
     ck_assert_uint_eq(ch, minch);
     tn_free(TN_GLOC(cs));

#test test_charset_max
     unsigned n;
     tn_charset_range *cs;
     ucs4_t ch;
     ucs4_t maxch;
     unsigned card;

     cs = generate_random_ranges(&n);
     card = tn_charset_cardinality(n, cs);
     ck_assert_uint_gt(card, 0);
     maxch = tn_charset_max(n, cs);
     ch = tn_charset_nth(n, cs, card - 1);
     ck_assert_uint_eq(ch, maxch);
     tn_free(TN_GLOC(cs));

#test-loop(0,100) test_charset_nth
     unsigned n;
     tn_charset_range *cs;
     ucs4_t ch;
     unsigned card;
     unsigned i;

     cs = generate_random_ranges(&n);
     card = tn_charset_cardinality(n, cs);
     ck_assert_uint_gt(card, 0);
     i = tn_random_int(0, card - 1);
     ch = tn_charset_nth(n, cs, i);
     ck_assert_uint_ne(ch, TN_INVALID_CHAR);
     ck_assert(tn_charset_contains(n, cs, ch));
     i = tn_random_int(card, INT32_MAX);
     ck_assert_uint_eq(tn_charset_nth(n, cs, i), TN_INVALID_CHAR);
     tn_free(TN_GLOC(cs));

#test test_charset_union_self
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_union(n, cs, n, cs, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*cs));
      ck_assert(memcmp(cs, csu, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csu));

#test test_charset_union_empty_both
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      tn_charset_generate_union(0, NULL, 0, NULL, &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csu, NULL);

#test test_charset_union_empty_right
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_union(n, cs, 0, NULL, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*cs));
      ck_assert(memcmp(cs, csu, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csu));

#test test_charset_union_empty_left
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_union(0, NULL, n, cs, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*cs));
      ck_assert(memcmp(cs, csu, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csu));

#test test_charset_union_full
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range full = {0, INT32_MAX};
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_union(n, cs, 1, &full, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*cs));
      ck_assert(memcmp(csu, &full, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csu));

#test-loop(0,100) test_charset_union_subset
      unsigned n1;
      tn_charset_range *cs1;
      unsigned n2;
      tn_charset_range *cs2;
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);
      unsigned n;

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);
      tn_charset_generate_union(n1, cs1, n2, cs2, &dest);
      ck_assert_uint_eq(dest.len % sizeof(*csu), 0);
      n = dest.len / sizeof(*csu);
      ck_assert_uint_gt(n, 0);
      ck_assert(tn_charset_valid(n, csu));
      ck_assert(tn_charset_subset(n1, cs1, n, csu));
      ck_assert(tn_charset_subset(n2, cs2, n, csu));
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csu));

#test-loop(0,100) test_charset_union_comm
      unsigned n1;
      tn_charset_range *cs1;
      unsigned n2;
      tn_charset_range *cs2;
      tn_charset_range *csu1 = NULL;
      tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(csu1), 0, 0);
      tn_charset_range *csu2 = NULL;
      tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(csu2), 0, 0);

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);
      tn_charset_generate_union(n1, cs1, n2, cs2, &dest1);
      tn_charset_generate_union(n2, cs2, n1, cs1, &dest2);
      ck_assert_uint_eq(dest1.len, dest2.len);
      ck_assert(memcmp(csu1, csu2, dest1.len) == 0);
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csu1));
      tn_free(TN_GLOC(csu2));

#test test_charset_intersect_self
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_intersect(n, cs, n, cs, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*cs));
      ck_assert(memcmp(cs, csu, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csu));

#test test_charset_intersect_empty_both
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      tn_charset_generate_union(0, NULL, 0, NULL, &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csi, NULL);

#test test_charset_intersect_empty_right
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_intersect(n, cs, 0, NULL, &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csi, NULL);
      tn_free(TN_GLOC(cs));

#test test_charset_intersect_empty_left
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_intersect(0, NULL, n, cs, &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csi, NULL);
      tn_free(TN_GLOC(cs));

#test test_charset_intersect_full
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range full = {0, INT32_MAX};
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_intersect(n, cs, 1, &full, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*csi));
      ck_assert(memcmp(csi, cs, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csi));

#test test_charset_intersect_bisect
      tn_charset_range r1;
      tn_charset_range r2;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      r1.lo = tn_random_int(0, INT32_MAX);
      r1.hi = tn_random_int(r1.lo, INT32_MAX);
      r2.hi = tn_random_int(r1.lo, r1.hi);
      r2.lo = r1.lo;
      tn_charset_generate_intersect(1, &r1, 1, &r2, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csi));
      ck_assert_uint_eq(csi->lo, r2.lo);
      ck_assert_uint_eq(csi->hi, r2.hi);
      tn_free(TN_GLOC(csi));

#test-loop(0,10) test_charset_intersect_inner
      tn_charset_range r1;
      tn_charset_range r2;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      r1.lo = tn_random_int(0, INT32_MAX);
      r1.hi = tn_random_int(r1.lo, INT32_MAX);
      r2.lo = tn_random_int(r1.lo, r1.hi);
      r2.hi = tn_random_int(r2.lo, r1.hi);
      tn_charset_generate_intersect(1, &r1, 1, &r2, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csi));
      ck_assert_uint_eq(csi->lo, r2.lo);
      ck_assert_uint_eq(csi->hi, r2.hi);
      tn_free(TN_GLOC(csi));

#test-loop(0,10) test_charset_intersect_outer
      tn_charset_range r1;
      tn_charset_range r2;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      r1.lo = tn_random_int(0, INT32_MAX);
      r1.hi = tn_random_int(r1.lo, INT32_MAX);
      r2.lo = tn_random_int(r1.lo, r1.hi);
      r2.hi = tn_random_int(r1.hi, INT32_MAX);
      tn_charset_generate_intersect(1, &r1, 1, &r2, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csi));
      ck_assert_uint_eq(csi->lo, r2.lo);
      ck_assert_uint_eq(csi->hi, r1.hi);
      tn_free(TN_GLOC(csi));

#test test_charset_intersect_bisect2
      tn_charset_range r1;
      tn_charset_range r2;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);

      r1.lo = tn_random_int(0, INT32_MAX);
      r1.hi = tn_random_int(r1.lo, INT32_MAX);
      r2.lo = tn_random_int(r1.lo, r1.hi);
      r2.hi = r1.hi;
      tn_charset_generate_intersect(1, &r1, 1, &r2, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csi));
      ck_assert_uint_eq(csi->lo, r2.lo);
      ck_assert_uint_eq(csi->hi, r2.hi);
      tn_free(TN_GLOC(csi));

#test-loop(0,100) test_charset_intersect_subset
      unsigned n1;
      tn_charset_range *cs1;
      unsigned n2;
      tn_charset_range *cs2;
      tn_charset_range *csi = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);
      unsigned n;

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);
      tn_charset_generate_intersect(n1, cs1, n2, cs2, &dest);
      ck_assert_uint_eq(dest.len % sizeof(*csi), 0);
      n = dest.len / sizeof(*csi);
      ck_assert(tn_charset_valid(n, csi));
      ck_assert(tn_charset_subset(n, csi, n1, cs1));
      ck_assert(tn_charset_subset(n, csi, n2, cs2));
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csi));

#test-loop(0,100) test_charset_intersect_comm
      unsigned n1;
      tn_charset_range *cs1;
      unsigned n2;
      tn_charset_range *cs2;
      tn_charset_range *csi1 = NULL;
      tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(csi1), 0, 0);
      tn_charset_range *csi2 = NULL;
      tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(csi2), 0, 0);

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);
      tn_charset_generate_intersect(n1, cs1, n2, cs2, &dest1);
      tn_charset_generate_intersect(n2, cs2, n1, cs1, &dest2);
      ck_assert_uint_eq(dest1.len, dest2.len);
      ck_assert(memcmp(csi1, csi2, dest1.len) == 0);
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csi1));
      tn_free(TN_GLOC(csi2));

#test test_charset_complement_empty
      tn_charset_range *csc = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);

      tn_charset_generate_complement(0, NULL, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csc));
      ck_assert_uint_eq(csc->lo, 0);
      ck_assert_uint_eq(csc->hi, INT32_MAX);
      tn_free(TN_GLOC(csc));

#test test_charset_complement_full
      tn_charset_range *csc = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);

      tn_charset_generate_complement(1, &(tn_charset_range){0, INT32_MAX},
                                     &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csc, NULL);

#test-loop(0,100) test_charset_complement_valid
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csc = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_complement(n, cs, &dest);
      ck_assert_uint_eq(dest.len % sizeof(*csc), 0);
      ck_assert(tn_charset_valid(dest.len / sizeof(*csc), csc));
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csc));

#test test_charset_complement_bisect
      ucs4_t mid = tn_random_int(0, INT32_MAX - 1);
      tn_charset_range pfx = {0, mid};
      tn_charset_range *csc = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);

      tn_charset_generate_complement(1, &pfx, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csc));
      ck_assert_uint_eq(csc->lo, mid + 1);
      ck_assert_uint_eq(csc->hi, INT32_MAX);
      tn_free(TN_GLOC(csc));

#test test_charset_complement_bisect2
      ucs4_t mid = tn_random_int(1, INT32_MAX);
      tn_charset_range pfx = {mid, INT32_MAX};
      tn_charset_range *csc = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);

      tn_charset_generate_complement(1, &pfx, &dest);
      ck_assert_uint_eq(dest.len, sizeof(*csc));
      ck_assert_uint_eq(csc->lo, 0);
      ck_assert_uint_eq(csc->hi, mid - 1);
      tn_free(TN_GLOC(csc));

#test-loop(0,100) test_charset_complement_complement
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csc = NULL;
      tn_charset_range *csc2 = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);
      tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(csc2), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_complement(n, cs, &dest);
      tn_charset_generate_complement(dest.len / sizeof(*csc), csc, &dest2);
      ck_assert_uint_eq(dest2.len, n * sizeof(*csc));
      ck_assert(memcmp(csc2, cs, dest2.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csc));
      tn_free(TN_GLOC(csc2));

#test-loop(0,100) test_charset_union_complement
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csc = NULL;
      tn_charset_range *cscu = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);
      tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(cscu), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_complement(n, cs, &dest);
      tn_charset_generate_union(n, cs,
                                dest.len / sizeof(*csc), csc,
                                &dest2);
      ck_assert_uint_eq(dest2.len, sizeof(*cscu));
      ck_assert_uint_eq(cscu->lo, 0);
      ck_assert_uint_eq(cscu->hi, INT32_MAX);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csc));
      tn_free(TN_GLOC(cscu));

#test-loop(0,100) test_charset_intersect_complement
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csc = NULL;
      tn_charset_range *csci = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);
      tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(csci), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_complement(n, cs, &dest);
      tn_charset_generate_intersect(n, cs,
                                    dest.len / sizeof(*csc), csc,
                                    &dest2);
      ck_assert_uint_eq(dest2.len, 0);
      ck_assert_ptr_eq(csci, NULL);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csc));

#test test_charset_empty_diff
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csd = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_diff(0, NULL, n, cs, &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csd, NULL);
      tn_free(TN_GLOC(cs));

#test test_charset_diff_empty
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csd = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_diff(n, cs, 0, NULL, &dest);
      ck_assert_uint_eq(dest.len, n * sizeof(*csd));
      ck_assert(memcmp(csd, cs, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csd));

#test test_charset_full_diff
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csc = NULL;
      tn_buffer destc = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);
      tn_charset_range *csd = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_complement(n, cs, &destc);
      tn_charset_generate_diff(1, &(tn_charset_range){0, INT32_MAX},
                               n, cs, &dest);
      ck_assert_uint_eq(dest.len, destc.len);
      ck_assert(memcmp(csc, csd, dest.len) == 0);
      tn_free(TN_GLOC(cs));
      tn_free(TN_GLOC(csc));
      tn_free(TN_GLOC(csd));

#test test_charset_diff_full
      unsigned n;
      tn_charset_range *cs;
      tn_charset_range *csd = NULL;
      tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs = generate_random_ranges(&n);
      tn_charset_generate_diff(n, cs, 1, &(tn_charset_range){0, INT32_MAX},
                               &dest);
      ck_assert_uint_eq(dest.len, 0);
      ck_assert_ptr_eq(csd, NULL);
      tn_free(TN_GLOC(cs));

#test-loop(0,100) test_charset_diff_intersect_compl
      unsigned n1;
      unsigned n2;
      tn_charset_range *cs1;
      tn_charset_range *cs2;
      tn_charset_range *csc = NULL;
      tn_charset_range *csi = NULL;
      tn_charset_range *csd = NULL;
      tn_buffer destc = TN_BUFFER_INIT(TN_GLOC(csc), 0, 0);
      tn_buffer desti = TN_BUFFER_INIT(TN_GLOC(csi), 0, 0);
      tn_buffer destd = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);

      tn_charset_generate_diff(n1, cs1, n2, cs2, &destd);
      ck_assert_uint_eq(destd.len % sizeof(*csd), 0);
      ck_assert(tn_charset_valid(destd.len / sizeof(*csd), csd));
      tn_charset_generate_complement(n2, cs2, &destc);
      tn_charset_generate_intersect(n1, cs1, destc.len / sizeof(*csc),
                                    csc, &desti);

      ck_assert_uint_eq(destd.len, desti.len);
      ck_assert(memcmp(csd, csi, destd.len) == 0);
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csc));
      tn_free(TN_GLOC(csd));
      tn_free(TN_GLOC(csi));

#test test_charset_diff_subset
      unsigned n1;
      unsigned n2;
      tn_charset_range *cs1;
      tn_charset_range *cs2;
      tn_charset_range *csd = NULL;
      tn_buffer destd = TN_BUFFER_INIT(TN_GLOC(csd), 0, 0);

      cs1 = generate_random_ranges(&n1);
      cs2 = generate_random_ranges(&n2);

      tn_charset_generate_diff(n1, cs1, n2, cs2, &destd);
      ck_assert(tn_charset_subset(destd.len / sizeof(*csd), csd, n1, cs1));
      ck_assert(!tn_charset_subset(n2, cs2, destd.len / sizeof(*csd), csd));
      tn_free(TN_GLOC(cs1));
      tn_free(TN_GLOC(cs2));
      tn_free(TN_GLOC(csd));

#main-pre
    tcase_add_checked_fixture(tc1_1, init_random, NULL);
