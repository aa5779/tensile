#include "charset.h"
#include "testing.h"

typedef struct tnt_charset
{
    unsigned n;
    tn_charset_range *ranges;
} tnt_charset;

static void
tnt_charset_generate(TN_UNUSED unsigned _i, tnt_charset *cs)
{
#define MAX_CS_RANGES 16
    unsigned i;
    ucs4_t min = 0;
    ucs4_t max;

    cs->ranges = NULL;
    cs->n = (unsigned)tn_random_int(1, MAX_CS_RANGES);
    max = INT32_MAX - 2 * (cs->n - 1);
    tn_alloc_raw(TN_GLOC(cs->ranges), sizeof(*cs->ranges) * cs->n);

    for (i = 0; i < cs->n; i++)
    {
        tn_charset_range *r = &cs->ranges[i];
        r->lo = (ucs4_t)tn_random_int(min, max);
        r->hi = (ucs4_t)tn_random_int(r->lo, max);
        min = r->hi + 2;
        max += 2;
    }
#undef MAX_CS_RANGES
}

#define tnt_charset_cleanup(_cs) tn_free(TN_GLOC((_cs)->ranges))

TESTDEF(test_charset_sanity, "Random charset is valid")
{
    FORALL(tnt_charset, cs,
           tnt_assert(tn_charset_valid(cs.n, cs.ranges)));
}

TESTDEF_SINGLE(test_charset_empty_valid, "Empty charset is valid")
{
    tnt_assert(tn_charset_valid(0, NULL));
}

TESTDEF_SINGLE(test_charset_invalid1, "Invalid charset: swapped limits")
{
    tn_charset_range invalid = {INT32_MAX, 0};
    tnt_assert(!tn_charset_valid(1, &invalid));
}

TESTDEF_SINGLE(test_charset_invalid2, "Invalid charset: swapped limits 2")
{
    tn_charset_range invalid[2] = {{10, 20}, {40, 30}};
    tnt_assert(!tn_charset_valid(2, invalid));
}

TESTDEF_SINGLE(test_charset_invalid_order,
               "Invalid charset: swapped range order")
{
    tn_charset_range invalid[2] = {{10, 20}, {0, 5}};
    tnt_assert(!tn_charset_valid(2, invalid));
}

TESTDEF_SINGLE(test_charset_invalid_connected,
               "Invalid charset: connected ranges")
{
    tn_charset_range invalid[2] = {{10, 20}, {21, 30}};
    tnt_assert(!tn_charset_valid(2, invalid));
}

TESTDEF(test_charset_contains, "A charset contains all of its characters")
{
    FORALL(tnt_charset, cs,
           unsigned i;
           ucs4_t ch;

           i = tn_random_int(0, cs.n - 1);
           ch = tn_random_int(cs.ranges[i].lo, cs.ranges[i].hi);
           tnt_assert(tn_charset_contains(cs.n, cs.ranges, ch)));
}

TESTDEF_SINGLE(test_charset_empty_subset,
               "Empty charset is a subset of any charset and "
               "no charset is a subset of the empty")
{
    FORALL(tnt_charset, cs,
           tnt_assert(tn_charset_subset(0, NULL, cs.n, cs.ranges));
           tnt_assert(!tn_charset_subset(cs.n, cs.ranges, 0, NULL)));
}

TESTDEF_SINGLE(test_charset_subset_full,
               "Any charset is a subset of the full charset")
{
    FORALL(tnt_charset, cs,
           tn_charset_range full = {0, INT32_MAX};

           tnt_assert(tn_charset_subset(cs.n, cs.ranges, 1, &full)));
}

TESTDEF_SINGLE(test_charset_subset, "Charset subset is its subset")
{
    FORALL(tnt_charset, cs,
           tnt_assert(tn_charset_subset(cs.n - 1, cs.ranges + 1,
                                        cs.n, cs.ranges));
           tnt_assert(tn_charset_subset(cs.n - 1, cs.ranges,
                                        cs.n, cs.ranges));
           tnt_assert(!tn_charset_subset(cs.n, cs.ranges,
                                         cs.n - 1, cs.ranges + 1));
           tnt_assert(!tn_charset_subset(cs.n, cs.ranges,
                                         cs.n - 1, cs.ranges)));
}

TESTDEF_SINGLE(test_charset_subset_prefix, "A char range prefix is its subset")
{
    tn_charset_range r1;
    tn_charset_range r2;

    r1.lo = tn_random_int(0, INT32_MAX - 1);
    r1.hi = tn_random_int(r1.lo, INT32_MAX - 1);
    r2.lo = r1.lo;
    r2.hi = tn_random_int(r1.hi + 1, INT32_MAX);
    tnt_assert(tn_charset_subset(1, &r1, 1, &r2));
    tnt_assert(!tn_charset_subset(1, &r2, 1, &r1));
}

TESTDEF_SINGLE(test_charset_min,
               "First character of a charset is its lower limit")
{
    FORALL(tnt_charset, cs,
           ucs4_t ch;
           ucs4_t minch;

           minch = tn_charset_min(cs.n, cs.ranges);
           ch = tn_charset_nth(cs.n, cs.ranges, 0);
           tnt_assert_op(ucs4_t, ch, ==, minch));
}

TESTDEF_SINGLE(test_charset_max,
               "Last character of a charset is its upper limit")
{
    FORALL(tnt_charset, cs,
           ucs4_t ch;
           ucs4_t maxch;
           unsigned card;

           card = tn_charset_cardinality(cs.n, cs.ranges);
           tnt_assert_op(unsigned, card, >, 0);
           maxch = tn_charset_max(cs.n, cs.ranges);
           ch = tn_charset_nth(cs.n, cs.ranges, card - 1);
           tnt_assert_op(ucs4_t, ch, ==, maxch));
}

TESTDEF(test_charset_nth, "Nth character is contained in the charset")
{
    FORALL(tnt_charset, cs,
           ucs4_t ch;
           unsigned card;
           unsigned i;

           card = tn_charset_cardinality(cs.n, cs.ranges);
           tnt_assert_op(unsigned, card, >, 0);
           i = tn_random_int(0, card - 1);
           ch = tn_charset_nth(cs.n, cs.ranges, i);
           tnt_assert_op(ucs4_t, ch, !=, TN_INVALID_CHAR);
           tnt_assert(tn_charset_contains(cs.n, cs.ranges, ch));
           i = tn_random_int(card, INT32_MAX);
           tnt_assert_op(ucs4_t, tn_charset_nth(cs.n, cs.ranges, i), ==,
                         TN_INVALID_CHAR));
}

TESTDEF_SINGLE(test_charset_union_self, "∀ C : CS, C ∪ C = C")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csu,
                     tn_charset_generate_union(cs.n, cs.ranges,
                                               cs.n, cs.ranges, &csu_buffer);
                     tnt_assert_op(size_t, csu_buffer.len, ==,
                                   cs.n * sizeof(*cs.ranges));
                     tnt_assert(memcmp(cs.ranges, csu, csu_buffer.len) == 0)));
}

TESTDEF_SINGLE(test_charset_union_empty_both, "∅ : CS ∪ ∅ = ∅")
{
    PRODUCING(tn_charset_range, csu,
              tn_charset_generate_union(0, NULL, 0, NULL, &csu_buffer);
              tnt_assert_op(size_t, csu_buffer.len, ==, 0);
              tnt_assert_op(ptr, csu, ==, NULL));
}

TESTDEF_SINGLE(test_charset_union_empty_right, "∀ C : CS, C ∪ ∅ = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csu,
                     tn_charset_generate_union(cs.n, cs.ranges, 0, NULL,
                                               &csu_buffer);
                     tnt_assert_op(size_t, csu_buffer.len, ==,
                                   cs.n * sizeof(*cs.ranges));
                     tnt_assert(memcmp(cs.ranges, csu, csu_buffer.len) == 0)));
}


TESTDEF_SINGLE(test_charset_union_empty_left, "∀ C : CS, ∅ ∪ C = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csu,
                     tn_charset_generate_union(0, NULL, cs.n,
                                               cs.ranges, &csu_buffer);
                     tnt_assert_op(size_t, csu_buffer.len, ==,
                                   cs.n * sizeof(*cs.ranges));
                     tnt_assert(memcmp(cs.ranges, csu, csu_buffer.len) == 0)));
}

TESTDEF_SINGLE(test_charset_union_full, "∀ C : CS, C υ U = U")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csu,
                     tn_charset_range full = {0, INT32_MAX};

                     tn_charset_generate_union(cs.n, cs.ranges, 1, &full,
                                               &csu_buffer);
                     tnt_assert_op(size_t, csu_buffer.len, ==,
                                   sizeof(*cs.ranges));
                     tnt_assert(memcmp(csu, &full, csu_buffer.len) == 0)));
}

TESTDEF(test_charset_union_subset,
        "∀ C₁, C₂ : CS, C₁ ⊂ C₁ ∪ C₂ ∧ C₂ ⊂ C₁ ∪ C₂")
{
    FORALL(tnt_charset, cs1,
           FORALL(tnt_charset, cs2,
                  PRODUCING
                  (tn_charset_range, csu,
                   unsigned n;

                   tn_charset_generate_union(cs1.n, cs1.ranges,
                                             cs2.n, cs2.ranges,
                                             &csu_buffer);
                   tnt_assert_op(size_t, csu_buffer.len % sizeof(*csu), ==, 0);
                   n = csu_buffer.len / sizeof(*csu);
                   tnt_assert_op(unsigned, n, >, 0);
                   tnt_assert(tn_charset_valid(n, csu));
                   tnt_assert(tn_charset_subset(cs1.n, cs1.ranges, n, csu));
                   tnt_assert(tn_charset_subset(cs2.n, cs2.ranges, n, csu)))));
}


TESTDEF(test_charset_union_comm, "∀ C₁, C₂ : CS, C₁ ∪ C₂ = C₂ ∪ C₁")
{
    FORALL(tnt_charset, cs1,
           FORALL(tnt_charset, cs2,
                  PRODUCING
                  (tn_charset_range, csu1,
                   PRODUCING
                   (tn_charset_range, csu2,
                    tn_charset_generate_union(cs1.n, cs1.ranges,
                                              cs2.n, cs2.ranges,
                                              &csu1_buffer);
                    tn_charset_generate_union(cs2.n, cs2.ranges,
                                              cs1.n, cs1.ranges,
                                              &csu2_buffer);
                    tnt_assert_op(size_t, csu1_buffer.len, ==, csu2_buffer.len);
                    tnt_assert(memcmp(csu1, csu2, csu1_buffer.len) == 0)))));
}

TESTDEF_SINGLE(test_charset_intersect_self, "∀ C : CS, C ∩ C = C")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csi,
                     tn_charset_generate_intersect(cs.n, cs.ranges,
                                                   cs.n, cs.ranges,
                                                   &csi_buffer);
                     tnt_assert_op(size_t, csi_buffer.len, ==,
                                   cs.n * sizeof(*cs.ranges));
                     tnt_assert(memcmp(cs.ranges, csi, csi_buffer.len) == 0)));
}


TESTDEF_SINGLE(test_charset_intersect_empty_both, "∅ : CS ∩ ∅ = ∅")
{
    PRODUCING(tn_charset_range, csi,
              tn_charset_generate_intersect(0, NULL, 0, NULL,
                                            &csi_buffer);
              tnt_assert_op(size_t, csi_buffer.len, ==, 0);
              tnt_assert_op(ptr, csi, ==, NULL));
}

TESTDEF_SINGLE(test_charset_intersect_empty_right, "∀ C : CS, C ∩ ∅ = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csi,
                     tn_charset_generate_intersect(cs.n, cs.ranges, 0, NULL,
                                                   &csi_buffer);
                     tnt_assert_op(size_t, csi_buffer.len, ==, 0);
                     tnt_assert_op(ptr, csi, ==, NULL)));
}

TESTDEF_SINGLE(test_charset_intersect_empty_left, "∀ C : CS, ∅ ∩ C = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csi,
                     tn_charset_generate_intersect(0, NULL,
                                                   cs.n, cs.ranges,
                                                   &csi_buffer);
                     tnt_assert_op(size_t, csi_buffer.len, ==, 0);
                     tnt_assert_op(ptr, csi, ==, NULL)));
}


TESTDEF_SINGLE(test_charset_intersect_full, "∀ C : CS, C ∩ U = C")
{
    FORALL(tnt_charset, cs,
           PRODUCING(tn_charset_range, csi,
                     tn_charset_range full = {0, INT32_MAX};

                     tn_charset_generate_intersect(cs.n, cs.ranges,
                                                   1, &full,
                                                   &csi_buffer);
                     tnt_assert_op(size_t, csi_buffer.len, ==,
                                   cs.n * sizeof(*csi));
                     tnt_assert(memcmp(csi, cs.ranges, csi_buffer.len) == 0)));
}

TESTDEF_SINGLE(test_charset_intersect_lower,
               "An intersection of a character range and "
               "its lower part is this part")
{
    PRODUCING(tn_charset_range, csi,
              tn_charset_range r1;
              tn_charset_range r2;

              r1.lo = tn_random_int(0, INT32_MAX);
              r1.hi = tn_random_int(r1.lo, INT32_MAX);
              r2.hi = tn_random_int(r1.lo, r1.hi);
              r2.lo = r1.lo;
              tn_charset_generate_intersect(1, &r1, 1, &r2,
                                            &csi_buffer);
              tnt_assert_op(size_t, csi_buffer.len, ==, sizeof(*csi));
              tnt_assert_op(ucs4_t, csi->lo, ==, r2.lo);
              tnt_assert_op(ucs4_t, csi->hi, ==, r2.hi));
}

TESTDEF(test_charset_intersect_inner,
        "An intersection of a character range and "
        "its subrange is this subrange")
{
    PRODUCING(tn_charset_range, csi,
              tn_charset_range r1;
              tn_charset_range r2;

              r1.lo = tn_random_int(0, INT32_MAX);
              r1.hi = tn_random_int(r1.lo, INT32_MAX);
              r2.lo = tn_random_int(r1.lo, r1.hi);
              r2.hi = tn_random_int(r2.lo, r1.hi);
              tn_charset_generate_intersect(1, &r1, 1, &r2, &csi_buffer);
              tnt_assert_op(size_t, csi_buffer.len, ==, sizeof(*csi));
              tnt_assert_op(ucs4_t, csi->lo, ==, r2.lo);
              tnt_assert_op(ucs4_t, csi->hi, ==, r2.hi));
}

TESTDEF(test_charset_intersect_outer,
        "Overlapping character ranges intersect")
{
    PRODUCING(tn_charset_range, csi,
              tn_charset_range r1;
              tn_charset_range r2;

              r1.lo = tn_random_int(0, INT32_MAX);
              r1.hi = tn_random_int(r1.lo, INT32_MAX);
              r2.lo = tn_random_int(r1.lo, r1.hi);
              r2.hi = tn_random_int(r1.hi, INT32_MAX);
              tn_charset_generate_intersect(1, &r1, 1, &r2, &csi_buffer);
              tnt_assert_op(size_t, csi_buffer.len, ==, sizeof(*csi));
              tnt_assert_op(ucs4_t, csi->lo, ==, r2.lo);
              tnt_assert_op(ucs4_t, csi->hi, ==, r1.hi));
}

TESTDEF_SINGLE(test_charset_intersect_upper,
               "An intersection of a character range and "
               "its upper part is this part")
{
    PRODUCING(tn_charset_range, csi,
              tn_charset_range r1;
              tn_charset_range r2;

              r1.lo = tn_random_int(0, INT32_MAX);
              r1.hi = tn_random_int(r1.lo, INT32_MAX);
              r2.lo = tn_random_int(r1.lo, r1.hi);
              r2.hi = r1.hi;
              tn_charset_generate_intersect(1, &r1, 1, &r2,
                                            &csi_buffer);
              tnt_assert_op(size_t, csi_buffer.len, ==, sizeof(*csi));
              tnt_assert_op(ucs4_t, csi->lo, ==, r2.lo);
              tnt_assert_op(ucs4_t, csi->hi, ==, r2.hi));
}

TESTDEF(test_charset_intersect_subset,
        "∀ C₁, C₂ : CS, C₁ ∩ C₂ ⊂ C₁ ∧ C₁ ∩ C₂ ⊂ C₂")
{
    FORALL(tnt_charset, cs1,
           FORALL(tnt_charset, cs2,
                  PRODUCING
                  (tn_charset_range, csi,
                   unsigned n;

                   tn_charset_generate_intersect(cs1.n, cs1.ranges,
                                                 cs2.n, cs2.ranges,
                                                 &csi_buffer);
                   tnt_trivial(csi_buffer.len == 0);
                   tnt_assert_op(size_t,
                                 csi_buffer.len % sizeof(*csi), ==, 0);
                   n = csi_buffer.len / sizeof(*csi);
                   tnt_assert(tn_charset_valid(n, csi));
                   tnt_assert(tn_charset_subset(n, csi, cs1.n,
                                                cs1.ranges));
                   tnt_assert(tn_charset_subset(n, csi, cs2.n,
                                                cs2.ranges)))));
}

TESTDEF(test_charset_intersect_comm, "∀ C₁, C₂ : CS, C₁ ∩ C₂ = C₂ ∩ C₁")
{
    FORALL(tnt_charset, cs1,
           FORALL(tnt_charset, cs2,
                  PRODUCING
                  (tn_charset_range, csi1,
                   PRODUCING
                   (tn_charset_range, csi2,
                    tn_charset_generate_intersect(cs1.n, cs1.ranges,
                                                  cs2.n, cs2.ranges,
                                                  &csi1_buffer);
                    tn_charset_generate_intersect(cs2.n, cs2.ranges,
                                                  cs1.n, cs1.ranges,
                                                  &csi2_buffer);
                    tnt_trivial(csi1_buffer.len == 0);
                    tnt_assert_op(size_t, csi1_buffer.len, ==, csi2_buffer.len);
                    tnt_assert(memcmp(csi1, csi2, csi1_buffer.len) == 0)))));
}

TESTDEF_SINGLE(test_charset_complement_empty, "∁ ∅ : CS = U")
{
    PRODUCING(tn_charset_range, csc,
              tn_charset_generate_complement(0, NULL, &csc_buffer);
              tnt_assert_op(size_t, csc_buffer.len, ==, sizeof(*csc));
              tnt_assert_op(ucs4_t, csc->lo, ==, 0);
              tnt_assert_op(ucs4_t, csc->hi, ==, INT32_MAX));
}


TESTDEF_SINGLE(test_charset_complement_full, "∁ U : CS = ∅")
{
    PRODUCING
        (tn_charset_range, csc,
         tn_charset_generate_complement(1, &(tn_charset_range){0, INT32_MAX},
                                        &csc_buffer);
         tnt_assert_op(size_t, csc_buffer.len, ==, 0);
         tnt_assert_op(ptr, csc, ==, NULL));
}

TESTDEF(test_charset_complement_valid, "∀ C : CS, ∁ C is valid")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csc,
            tn_charset_generate_complement(cs.n, cs.ranges, &csc_buffer);
            tnt_assert_op(size_t, csc_buffer.len % sizeof(*csc), ==, 0);
            tnt_assert(tn_charset_valid(csc_buffer.len / sizeof(*csc), csc))));
}

TESTDEF_SINGLE(test_charset_complement_bisect,
               "A complement of a lower half of the full set is "
               "its upper half")
{
    PRODUCING(tn_charset_range, csc,
              ucs4_t mid = tn_random_int(0, INT32_MAX - 1);
              tn_charset_range pfx = {0, mid};

              tn_charset_generate_complement(1, &pfx, &csc_buffer);
              tnt_assert_op(size_t, csc_buffer.len, ==, sizeof(*csc));
              tnt_assert_op(ucs4_t, csc->lo, ==, mid + 1);
              tnt_assert_op(ucs4_t, csc->hi, ==, INT32_MAX));
}

TESTDEF_SINGLE(test_charset_complement_bisect2,
               "A complement of an upper half of the full set is "
               "its lower half")
{
    PRODUCING(tn_charset_range, csc,
              ucs4_t mid = tn_random_int(1, INT32_MAX);
              tn_charset_range pfx = {mid, INT32_MAX};

              tn_charset_generate_complement(1, &pfx, &csc_buffer);
              tnt_assert_op(size_t, csc_buffer.len, ==, sizeof(*csc));
              tnt_assert_op(ucs4_t, csc->lo, ==, 0);
              tnt_assert_op(ucs4_t, csc->hi, ==, mid - 1));
}

TESTDEF(test_charset_complement_complement, "∀ C : CS, ∁ ∁ C = C")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csc,
            PRODUCING
            (tn_charset_range, csc2,
             tn_charset_generate_complement(cs.n, cs.ranges, &csc_buffer);
             tn_charset_generate_complement(csc_buffer.len / sizeof(*csc),
                                            csc, &csc2_buffer);
             tnt_assert_op(size_t, csc2_buffer.len, ==, cs.n * sizeof(*csc2));
             tnt_assert(memcmp(csc2, cs.ranges, csc2_buffer.len) == 0))));
}

TESTDEF(test_charset_union_complement, "∀ C : CS, ∁ C ∪ C = U")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csc,
            PRODUCING
            (tn_charset_range, cscu,
             tn_charset_generate_complement(cs.n, cs.ranges, &csc_buffer);
             tn_charset_generate_union(cs.n, cs.ranges,
                                       csc_buffer.len / sizeof(*csc), csc,
                                       &cscu_buffer);
             tnt_assert_op(size_t, cscu_buffer.len, ==, sizeof(*cscu));
             tnt_assert_op(ucs4_t, cscu->lo, ==, 0);
             tnt_assert_op(ucs4_t, cscu->hi, ==, INT32_MAX))));
}

TESTDEF(test_charset_intersect_complement, "∀ C : CS, ∁ C ∩ C = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csc,
            PRODUCING
            (tn_charset_range, csci,
             tn_charset_generate_complement(cs.n, cs.ranges, &csc_buffer);
             tn_charset_generate_intersect(cs.n, cs.ranges,
                                           csc_buffer.len / sizeof(*csc),
                                           csc, &csci_buffer);
             tnt_assert_op(size_t, csci_buffer.len, ==, 0);
             tnt_assert_op(ptr, csci, ==, NULL))));
}

TESTDEF_SINGLE(test_charset_empty_diff, "∀ C : CS, ∅ ∖ C = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csd,
            tn_charset_generate_diff(0, NULL, cs.n, cs.ranges, &csd_buffer);
            tnt_assert_op(size_t, csd_buffer.len, ==, 0);
            tnt_assert_op(ptr, csd, ==, NULL)));
}

TESTDEF_SINGLE(test_charset_diff_empty, "∀ C : CS, C ∖ ∅ = C")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csd,
            tn_charset_generate_diff(cs.n, cs.ranges, 0, NULL, &csd_buffer);
            tnt_assert_op(size_t, csd_buffer.len, ==, cs.n * sizeof(*csd));
            tnt_assert(memcmp(csd, cs.ranges, csd_buffer.len) == 0)));
}

TESTDEF_SINGLE(test_charset_full_diff, "∀ C : CS, U ∖ C = ∁ C")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csc,
            PRODUCING
            (tn_charset_range, csd,
             tn_charset_generate_complement(cs.n, cs.ranges, &csc_buffer);
             tn_charset_generate_diff(1, &(tn_charset_range){0, INT32_MAX},
                                      cs.n, cs.ranges, &csd_buffer);
             tnt_assert_op(size_t, csc_buffer.len, ==, csd_buffer.len);
             tnt_assert(memcmp(csc, csd, csd_buffer.len) == 0))));
}


TESTDEF_SINGLE(test_charset_diff_full, "∀ C : CS, C ∖ U = ∅")
{
    FORALL(tnt_charset, cs,
           PRODUCING
           (tn_charset_range, csd,
            tn_charset_generate_diff(cs.n, cs.ranges,
                                     1, &(tn_charset_range){0, INT32_MAX},
                                     &csd_buffer);
            tnt_assert_op(size_t, csd_buffer.len, ==, 0);
            tnt_assert_op(ptr, csd, ==, NULL)));
}

TESTDEF(test_charset_diff_intersect_compl,
        "∀ C₁, C₂ : CS, C₁ ∖ C₂ = C₁ ∩ ∁ C₂")
{
    FORALL(tnt_charset, cs1,
           FORALL
           (tnt_charset, cs2,
            PRODUCING
            (tn_charset_range, csc,
             PRODUCING
             (tn_charset_range, csi,
              PRODUCING
              (tn_charset_range, csd,
               tn_charset_generate_diff(cs1.n, cs1.ranges,
                                        cs2.n, cs2.ranges,
                                        &csd_buffer);
               tnt_assert_op(size_t, csd_buffer.len % sizeof(*csd), ==, 0);
               tnt_assert(tn_charset_valid(csd_buffer.len / sizeof(*csd), csd));
               tn_charset_generate_complement(cs2.n, cs2.ranges, &csc_buffer);
               tn_charset_generate_intersect(cs1.n, cs1.ranges,
                                             csc_buffer.len / sizeof(*csc),
                                             csc, &csi_buffer);
               tnt_assert_op(size_t, csd_buffer.len, ==, csi_buffer.len);
               tnt_assert(memcmp(csd, csi, csd_buffer.len) == 0))))));
}

TESTDEF_SINGLE(test_charset_diff_subset,
               "∀ C₁, C₂ : CS, C₁ ∖ C₂ ⊂ C₁ ∧ C₂ ⊄ C₁ ∖ C₂")
{
    FORALL(tnt_charset, cs1,
           FORALL
           (tnt_charset, cs2,
            PRODUCING
            (tn_charset_range, csd,
             tn_charset_generate_diff(cs1.n, cs1.ranges, cs2.n, cs2.ranges,
                                      &csd_buffer);
             tnt_assert(tn_charset_subset(csd_buffer.len / sizeof(*csd),
                                          csd, cs1.n, cs1.ranges));
             tnt_assert(!tn_charset_subset(cs2.n, cs2.ranges,
                                           csd_buffer.len / sizeof(*csd),
                                           csd)))));
}
