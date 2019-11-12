#include "testing.h"
#include <unistr.h>
#include "edits.h"

typedef struct tnt_string {
    unsigned n;
    uint32_t *str;
} tnt_string;

static void
tnt_string_generate(TN_UNUSED unsigned _i, tnt_string *s)
{
#define MAX_STR_LEN 128
    unsigned i;

    s->n = (unsigned)tn_random_int(1, MAX_STR_LEN);
    s->str = NULL;
    tn_alloc_raw(TN_GLOC(s->str), sizeof(*s->str) * (s->n));

    for (i = 0; i < s->n; i++)
    {
        s->str[i] = tn_random_int(0, INT32_MAX);
    }
#undef MAX_STR_LEN
}

#define tnt_string_cleanup(_s) tn_free(TN_GLOC((_s)->str))

typedef struct tnt_edit_seq {
    unsigned n;
    tn_edit_item *edits;
} tnt_edit_seq;

static void
tnt_edit_seq_generate(TN_UNUSED unsigned _i, tnt_edit_seq *seq)
{
#define MAX_EDIT_LEN 128
    unsigned i;
    size_t last = 0;

    seq->n = (unsigned)tn_random_int(1, MAX_EDIT_LEN);
    seq->edits = NULL;
    tn_alloc_raw(TN_GLOC(seq->edits), sizeof(*seq->edits) * (seq->n));

    for (i = 0; i < seq->n; i++)
    {
        seq->edits[i].pos = tn_random_int(last, INT32_MAX - (seq->n) + i);
        last = seq->edits[i].pos + 1;
        if (tn_random_int(0, 1) != 0)
        {
            seq->edits[i].pos |= TN_EDIT_INSERT;
            seq->edits[i].ch = tn_random_int(0, INT32_MAX);
        }
        else if (tn_random_int(0, 1) != 0)
            seq->edits[i].ch = TN_INVALID_CHAR;
        else
            seq->edits[i].ch = tn_random_int(0, INT32_MAX);
    }
#undef MAX_EDIT_LEN
}

#define tnt_edit_seq_cleanup(_es) tn_free(TN_GLOC((_es)->edits))

TESTDEF_SINGLE(test_edit_distance_self,
               "Levenstein distance of two identical strings is 0")
{
    FORALL(tnt_string, s,
           size_t dist;

           dist = tn_edit_distance(s.n, s.str, s.n, s.str);
           tnt_assert_op(size_t, dist, ==, 0));
}

TESTDEF_SINGLE(test_edit_distance_empty,
               "A distance between a string and an empty string is its length")
{
    FORALL(tnt_string, s,
           size_t dist;

           dist = tn_edit_distance(s.n, s.str, 0, NULL);
           tnt_assert_op(size_t, dist, ==, s.n);
           dist = tn_edit_distance(0, NULL, s.n, s.str);
           tnt_assert_op(size_t, dist, ==, s.n));
}


TESTDEF(test_edit_distance_symm,
        "Edit distance is symmetric")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  size_t dist1;
                  size_t dist2;

                  dist1 = tn_edit_distance(s1.n, s1.str, s2.n, s2.str);
                  dist2 = tn_edit_distance(s2.n, s2.str, s1.n, s1.str);
                  tnt_assert_op(size_t, dist1, ==, dist2)));
}

TESTDEF(test_edit_distance_triangle,
        "Edit distance obeys the triangular rule")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  FORALL(tnt_string, s3,
                         size_t dist1;
                         size_t dist2;
                         size_t dist3;

                         dist1 = tn_edit_distance(s1.n, s1.str,
                                                  s2.n, s2.str);
                         dist2 = tn_edit_distance(s2.n, s2.str,
                                                  s3.n, s3.str);
                         dist3 = tn_edit_distance(s1.n, s1.str,
                                                  s3.n, s3.str);
                         tnt_assert_op(size_t, dist3, <=, dist1 + dist2))));
}

TESTDEF_SINGLE(test_edit_distance_prefix,
               "A distance between a string and its prefix is equal to "
               "the length of the suffix")
{
    FORALL(tnt_string, s,
           size_t prefix;
           size_t dist;

           prefix = tn_random_int(0, s.n - 1);

           dist = tn_edit_distance(s.n, s.str, prefix, s.str);
           tnt_assert_op(size_t, dist, ==, s.n - prefix));
}


TESTDEF(test_edit_subst_single,
        "A single substitution results in a distance 1")
{
    FORALL(tnt_string, s1,
           tnt_string s2 = {0, NULL};
           unsigned pos;
           size_t dist;

           pos = tn_random_int(0, s1.n - 1);
           tn_alloc_raw(TN_GLOC(s2.str), s1.n * sizeof(*s2.str));
           s2.n = s1.n;
           memcpy(s2.str, s1.str, s1.n * sizeof(*s2.str));
           s2.str[pos] ^= 1;
           dist = tn_edit_distance(s1.n, s1.str, s2.n, s2.str);
           tnt_assert_op(size_t, dist, ==, 1);
           tnt_string_cleanup(&s2));
}


TESTDEF(test_edit_ins_single,
        "A single insertion results in a distance 1")
{
    FORALL(tnt_string, s1,
           tnt_string s2 = {0, NULL};
           unsigned pos;
           size_t dist;

           pos = tn_random_int(0, s1.n);
           tn_alloc_raw(TN_GLOC(s2.str), (s1.n + 1) * sizeof(*s2.str));
           s2.n = s1.n + 1;
           memcpy(s2.str, s1.str, pos * sizeof(*s2.str));
           memcpy(s2.str + pos + 1, s1.str + pos,
                  (s1.n - pos) * sizeof(*s2.str));
           s2.str[pos] = tn_random_int(0, INT32_MAX);
           dist = tn_edit_distance(s1.n, s1.str, s2.n, s2.str);
           tnt_assert_op(size_t, dist, ==, 1);
           tnt_string_cleanup(&s2));
}

TESTDEF(test_edit_del_single,
        "A single deletion results in a distance 1")
{
    FORALL(tnt_string, s1,
           tnt_string s2 = {0, NULL};
           unsigned pos;
           size_t dist;

           pos = tn_random_int(0, s1.n - 1);
           tn_alloc_raw(TN_GLOC(s2.str), s1.n * sizeof(*s2.str));
           s2.n = s1.n - 1;
           memcpy(s2.str, s1.str, pos * sizeof(*s2.str));
           memcpy(s2.str + pos, s1.str + pos + 1,
                  (s1.n - pos - 1) * sizeof(*s2.str));
           dist = tn_edit_distance(s1.n, s1.str, s2.n, s2.str);
           tnt_assert_op(size_t, dist, ==, 1);
           tnt_string_cleanup(&s2));
}


TESTDEF_SINGLE(test_edit_seq_invalid, "Some edit sequences are invalid")
{
    tnt_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {0, 0}}));
    tnt_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {1, 0}}));
}


TESTDEF_SINGLE(test_edit_seq_self,
               "An edit sequence of the two identical string is empty")
{
    FORALL(tnt_string, s,
           PRODUCING(tn_edit_item, edits,
                     tn_edit_generate_sequence(s.n, s.str, s.n, s.str,
                                               &edits_buffer);
                     tnt_assert_op(size_t, edits_buffer.len, ==, 0);
                     tnt_assert_op(ptr, edits, ==, NULL);
                     tnt_assert(tn_edit_seq_valid(0, NULL))));
}


TESTDEF(test_edit_seq_dist,
        "The length of an optimal edit sequence is the edit distance")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  PRODUCING
                  (tn_edit_item, edits,
                   size_t dist;

                   dist = tn_edit_distance(s1.n, s1.str, s2.n, s2.str);
                   tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                             &edits_buffer);
                   tnt_assert_op(size_t, edits_buffer.len, ==,
                                 dist * sizeof(*edits));
                   tnt_assert(tn_edit_seq_valid(dist, edits));
#ifndef TN_DEBUG_DISABLED
                   tnt_assert_op(size_t, tn_edit_seq_memory_utilization, <=,
                                 (s1.n + 1) * (s2.n + 1));
#endif
                      )));
}

TESTDEF_SINGLE(test_edit_seq_empty_left,
               "An edit sequence from an empty string "
               "contains only insertions")
{
    FORALL(tnt_string, s,
           PRODUCING
           (tn_edit_item, edits,
            unsigned i;

            tn_edit_generate_sequence(0, NULL, s.n, s.str,
                                      &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==,
                          s.n * sizeof(*edits));
            for (i = 0; i < s.n; i++)
            {
                tnt_assert_op(size_t, edits[i].pos, ==, TN_EDIT_INSERT);
                tnt_assert_op(ucs4_t, edits[i].ch, ==, s.str[i]);
            }));
}


TESTDEF_SINGLE(test_edit_seq_empty_right,
               "An edit sequence to an empty string "
               "contains only deletions")
{
    FORALL(tnt_string, s,
           PRODUCING
           (tn_edit_item, edits,
            unsigned i;

            tn_edit_generate_sequence(s.n, s.str, 0, NULL, &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==, s.n * sizeof(*edits));
            for (i = 0; i < s.n; i++)
            {
                tnt_assert_op(size_t, edits[i].pos, ==, i);
                tnt_assert_op(ucs4_t, edits[i].ch, ==, TN_INVALID_CHAR);
            }));
}

TESTDEF_SINGLE(test_edit_seq_pure_subst,
               "An edit sequence between two strings of the same length "
               "with no common characters contains only substitutions")
{
    PRODUCING(tn_edit_item, edits,
              unsigned i;
              tnt_string s1 = {0, NULL};
              tnt_string s2 = {0, NULL};

              s1.n = s2.n = tn_random_int(1, 128);
              tn_alloc_raw(TN_GLOC(s1.str), s1.n * sizeof(*s1.str));
              tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));

              for (i = 0; i < s1.n; i++)
              {
                  s1.str[i] = i;
                  s2.str[i] = s1.n + i;
              }

              tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str, &edits_buffer);
              tnt_assert_op(size_t, edits_buffer.len, ==, s1.n * sizeof(*edits));
              for (i = 0; i < s1.n; i++)
              {
                  tnt_assert_op(size_t, edits[i].pos, ==, i);
                  tnt_assert_op(ucs4_t, edits[i].ch, ==, s2.str[i]);
              }
              tnt_string_cleanup(&s1);
              tnt_string_cleanup(&s2));
}

TESTDEF(test_edit_seq_subst_single,
        "A single substitution yields a 1-length edit sequence")
{
    FORALL(tnt_string, s1,
           PRODUCING
           (tn_edit_item, edits,
            tnt_string s2 = {0, NULL};
            unsigned pos;

            pos = tn_random_int(0, s1.n - 1);
            tn_alloc_raw(TN_GLOC(s2.str), s1.n * sizeof(*s2.str));
            s2.n = s1.n;
            memcpy(s2.str, s1.str, s1.n * sizeof(*s2.str));
            s2.str[pos] ^= 1;
            tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                      &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==, sizeof(*edits));
            tnt_assert_op(size_t, edits[0].pos, ==, pos);
            tnt_assert_op(ucs4_t, edits[0].ch, ==, s2.str[pos]);
            tnt_string_cleanup(&s2)));
}

TESTDEF(test_edit_seq_ins_single,
        "A single insertion yields a 1-length edit sequence")
{
    FORALL(tnt_string, s1,
           PRODUCING
           (tn_edit_item, edits,
            tnt_string s2 = {0, NULL};
            unsigned pos;

            pos = tn_random_int(0, s1.n);
            s2.n = s1.n + 1;
            tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));
            memcpy(s2.str, s1.str, pos * sizeof(*s2.str));
            memcpy(s2.str + pos + 1, s1.str + pos,
                   (s1.n - pos) * sizeof(*s2.str));
            s2.str[pos] = tn_random_int(0, INT32_MAX);
            tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                      &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==, sizeof(*edits));
            tnt_assert_op(size_t, edits[0].pos, ==, pos | TN_EDIT_INSERT);
            tnt_assert_op(ucs4_t, edits[0].ch, ==, s2.str[pos]);
            tnt_string_cleanup(&s2)));
}

TESTDEF(test_edit_seq_ins_subst,
        "A insertion + substitution yields a correct edit sequence")
{
    FORALL(tnt_string, s1,
           PRODUCING
           (tn_edit_item, edits,
            tnt_string s2 = {0, NULL};
            unsigned pos;

            pos = tn_random_int(0, s1.n - 1);
            s2.n = s1.n + 1;
            tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));
            memcpy(s2.str, s1.str, pos * sizeof(*s2.str));
            memcpy(s2.str + pos + 1, s1.str + pos,
                   (s1.n - pos) * sizeof(*s2.str));
            s2.str[pos] = tn_random_int(0, INT32_MAX);
            s2.str[pos + 1] ^= 1;
            tn_edit_generate_sequence(s1.n, s1.str,
                                      s2.n, s2.str,
                                      &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==, 2 * sizeof(*edits));
            tnt_assert_op(size_t, edits[0].pos, ==, pos | TN_EDIT_INSERT);
            tnt_assert_op(ucs4_t, edits[0].ch, ==, s2.str[pos]);
            tnt_assert_op(size_t, edits[1].pos, ==, pos);
            tnt_assert_op(ucs4_t, edits[1].ch, ==, s2.str[pos + 1]);
            tnt_string_cleanup(&s2)));
}

TESTDEF(test_edit_seq_del_single,
        "A single deletion yields a 1-length edit sequence")
{
    FORALL(tnt_string, s1,
           PRODUCING
           (tn_edit_item, edits,
            tnt_string s2 = {0, NULL};
            unsigned pos;

            pos = tn_random_int(0, s1.n - 1);
            s2.n = s1.n - 1;
            tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));
            memcpy(s2.str, s1.str, pos * sizeof(*s2.str));
            memcpy(s2.str + pos, s1.str + pos + 1,
                   (s2.n - pos) * sizeof(*s2.str));
            tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                      &edits_buffer);
            tnt_assert_op(size_t, edits_buffer.len, ==, sizeof(*edits));
            tnt_assert_op(size_t, edits[0].pos, ==, pos);
            tnt_assert_op(ucs4_t, edits[0].ch, ==, TN_INVALID_CHAR);
            tnt_string_cleanup(&s2)));
}

TESTDEF_SINGLE(test_edit_apply_empty,
               "Apply an empty edit sequence changes nothing")
{
    FORALL(tnt_string, s1,
           PRODUCING
           (uint32_t, s2,
            tn_edit_apply_sequence(s1.n, s1.str, 0, NULL,
                                   &s2_buffer);
            tnt_assert_mem(s2_buffer.len, s2, s1.n * sizeof(*s1.str), s1.str)));
}

TESTDEF(test_edit_apply_any,
        "Applying an edit sequence to the first string yields the second")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  PRODUCING
                  (tn_edit_item, edits,
                   PRODUCING
                   (uint32_t, str3,
                    tn_edit_generate_sequence(s1.n, s1.str,
                                              s2.n, s2.str,
                                              &edits_buffer);
                    tn_edit_apply_sequence(s1.n, s1.str,
                                           edits_buffer.len / sizeof(*edits),
                                           edits, &str3_buffer);
                    tnt_assert_mem(str3_buffer.len, str3,
                                   s2.n * sizeof(*s2.str), s2.str)))));
}

TESTDEF(test_edit_apply_single_subst,
        "Applying an edit sequence with a single substitution "
        "results in a string with a single character replaced")
{
    FORALL(tnt_string, s,
           PRODUCING(uint32_t, str,
                     unsigned pos;
                     ucs4_t ch;

                     pos = tn_random_int(0, s.n - 1);
                     ch = tn_random_int(0, INT32_MAX);
                     tn_edit_apply_sequence(s.n, s.str, 1,
                                            &(tn_edit_item){pos, ch},
                                            &str_buffer);
                     s.str[pos] = ch;
                     tnt_assert_mem(s.n * sizeof(*s.str), s.str,
                                    str_buffer.len, str)));
}

TESTDEF(test_edit_apply_prefix,
        "An edit sequence may be safely applied to a prefix "
        " of the original string")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  PRODUCING
                  (tn_edit_item, edits,
                   PRODUCING
                   (uint32_t, str3,
                    unsigned newlen;

                    tn_edit_generate_sequence(s1.n, s1.str,
                                              s2.n, s2.str,
                                              &edits_buffer);
                    newlen = tn_random_int(0, s1.n - 1);
                    tn_edit_apply_sequence(newlen, s1.str,
                                           edits_buffer.len / sizeof(*edits),
                                           edits, &str3_buffer);
                    tnt_assert_op(size_t, str3_buffer.len, <,
                                  s2.n * sizeof(*str3));
                    tnt_assert(memcmp(str3, s2.str, str3_buffer.len) == 0)))));
}

TESTDEF_SINGLE(test_edit_random_sanity,
               "Random edit sequence is valid")
{
    FORALL(tnt_edit_seq, es,
           tnt_assert(tn_edit_seq_valid(es.n, es.edits));
           tnt_assert(tn_edit_seq_eq(es.n, es.edits, es.n, es.edits)));
}

TESTDEF(test_edit_item_shift_0,
        "An edit sequence item shifted by zero is the same")
{
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, 0);

    tnt_assert(tn_edit_same_position(&item, &sh));
    tnt_assert_op(ucs4_t, item.ch, ==, sh.ch);
}


TESTDEF(test_edit_item_shift,
        "Shifting an edit item does not change its structure")
{
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    int delta = tn_random_int(-pos, INT32_MAX - pos);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, delta);

    tnt_trivial(delta == 0);
    tnt_assert_op(size_t, tn_edit_position(&item) + delta, ==,
                  tn_edit_position(&sh));
    tnt_assert_op(size_t, item.ch, ==, sh.ch);
}


TESTDEF_SINGLE(test_edit_compose_seq_empty_right,
               "∀ X : ES, ε ∘ X = X")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, result,
            tn_edit_compose_sequence(seq.n, seq.edits, 0, NULL,
                                     &result_buffer);
            tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                      result_buffer.len / sizeof(*result),
                                      result))));
}
TESTDEF_SINGLE(test_edit_compose_seq_empty_left,
               "∀ X : ES, X ∘ ε = X")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, result,
            tn_edit_compose_sequence(0, NULL, seq.n, seq.edits,
                                     &result_buffer);
            tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                      result_buffer.len / sizeof(*result),
                                      result))));
}

TESTDEF(test_edit_compose_subst,
        "Composition of two substitution is equivalent to the last one")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch1 = tn_random_int(0, INT32_MAX);
              ucs4_t ch2 = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, false, ch1);
              tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);

              tn_edit_compose_sequence(1, &item1, 1, &item2,
                                       &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, sizeof(*result));
              tnt_assert_op(size_t, result->pos, ==, pos);
              tnt_assert_op(ucs4_t, result->ch, ==, ch2));
}

TESTDEF(test_edit_compose_insert_subst,
        "Composition of an insertion and substitution results "
        "in a single insertion")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch1 = tn_random_int(0, INT32_MAX);
              ucs4_t ch2 = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
              tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);

              tn_edit_compose_sequence(1, &item1, 1, &item2,
                                       &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, sizeof(*result));
              tnt_assert_op(size_t, result->pos, ==, pos | TN_EDIT_INSERT);
              tnt_assert_op(ucs4_t, result->ch, ==, ch2));
}


TESTDEF(test_edit_compose_insert2,
        "Insertions are composed in the right order")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch1 = tn_random_int(0, INT32_MAX);
              ucs4_t ch2 = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
              tn_edit_item item2 = tn_edit_make_item(pos, true, ch2);

              tn_edit_compose_sequence(1, &item1, 1, &item2, &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, 2 * sizeof(*result));
              tnt_assert_op(size_t, result[0].pos, ==, pos | TN_EDIT_INSERT);
              tnt_assert_op(ucs4_t, result[0].ch, ==, ch2);
              tnt_assert_op(size_t, result[1].pos, ==, pos | TN_EDIT_INSERT);
              tnt_assert_op(ucs4_t, result[1].ch, ==, ch1));
}

TESTDEF(test_edit_compose_nullify,
        "Insertion + deletion are mutually cancelled")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, true, ch);
              tn_edit_item item2 = tn_edit_make_item(pos, false,
                                                     TN_INVALID_CHAR);

              tn_edit_compose_sequence(1, &item1, 1, &item2,
                                       &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, 0);
              tnt_assert_op(ptr, result, ==, NULL));
}


TESTDEF(test_edit_compose_delete_insert,
        "Deletion + insertion yield a substitution")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, false,
                                                     TN_INVALID_CHAR);
              tn_edit_item item2 = tn_edit_make_item(pos, true, ch);

              tn_edit_compose_sequence(1, &item1, 1, &item2,
                                       &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, sizeof(*result));
              tnt_assert_op(size_t, result->pos, ==, pos);
              tnt_assert_op(ucs4_t, result->ch, ==, ch));
}

TESTDEF(test_edit_compose_delete_delete,
        "Deletions are composed in the right order")
{
    PRODUCING(tn_edit_item, result,
              size_t pos = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos, false,
                                                     TN_INVALID_CHAR);
              tn_edit_item item2 = tn_edit_make_item(pos, false,
                                                     TN_INVALID_CHAR);

              tn_edit_compose_sequence(1, &item1, 1, &item2,
                                       &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, 2 * sizeof(*result));
              tnt_assert_op(size_t, result[0].pos, ==, pos);
              tnt_assert_op(ucs4_t, result[0].ch, ==, TN_INVALID_CHAR);
              tnt_assert_op(size_t, result[1].pos, ==, pos + 1);
              tnt_assert_op(ucs4_t, result[1].ch, ==, TN_INVALID_CHAR));
}

TESTDEF(test_edit_compose_disjoint,
        "Composition of two independent 1-element edit sequences is "
        "their concatenation")
{
    PRODUCING(tn_edit_item, result,
              size_t pos1 = tn_random_int(0, INT32_MAX - 1);
              size_t pos2 = tn_random_int(pos1 + 1, INT32_MAX);
              ucs4_t ch1 = tn_random_int(0, INT32_MAX);
              ucs4_t ch2 = tn_random_int(0, INT32_MAX);
              tn_edit_item item1 = tn_edit_make_item(pos1, false, ch1);
              tn_edit_item item2 = tn_edit_make_item(pos2, false, ch2);

              tn_edit_compose_sequence(1, &item1, 1, &item2, &result_buffer);
              tnt_assert_op(size_t, result_buffer.len, ==, 2 * sizeof(*result));
              tnt_assert_op(size_t, result[0].pos, ==, pos1);
              tnt_assert_op(ucs4_t, result[0].ch, ==, ch1);
              tnt_assert_op(size_t, result[1].pos, ==, pos2);
              tnt_assert_op(ucs4_t, result[1].ch, ==, ch2));
}

TESTDEF(test_edit_compose_valid,
        "Any composition of edit sequences is valid")
{
    FORALL(tnt_edit_seq, seq1,
           FORALL
           (tnt_edit_seq, seq2,
            PRODUCING
            (tn_edit_item, result,
             tn_edit_compose_sequence(seq1.n, seq1.edits,
                                      seq2.n, seq2.edits,
                                      &result_buffer);
             tnt_assert(tn_edit_seq_valid(result_buffer.len / sizeof(*result),
                                          result)))));
}

TESTDEF(test_edit_compose,
        "∀ E₁, E₂ : ES, s : S, (E₂ ∘ E₁)(s) = E₂(E₁(s))")
{
    FORALL
        (tnt_string, s1,
         FORALL
         (tnt_string, s2,
          FORALL
          (tnt_string, s3,
           PRODUCING
           (tn_edit_item, diff12,
            PRODUCING
            (tn_edit_item, diff23,
             PRODUCING
             (tn_edit_item, compose,
              PRODUCING
              (uint32_t, result,
               tn_edit_generate_sequence(s1.n, s1.str,
                                         s2.n, s2.str,
                                         &diff12_buffer);
               tn_edit_generate_sequence(s2.n, s2.str,
                                         s3.n, s3.str,
                                         &diff23_buffer);
               tn_edit_compose_sequence(diff12_buffer.len / sizeof(*diff12),
                                        diff12,
                                        diff23_buffer.len / sizeof(*diff23),
                                        diff23,
                                        &compose_buffer);
               tn_edit_apply_sequence(s1.n, s1.str,
                                      compose_buffer.len / sizeof(*compose),
                                      compose, &result_buffer);
               tnt_assert_mem(s3.n * sizeof(*s3.str), s3.str,
                              result_buffer.len, result))))))));
}

TESTDEF(test_edit_compose_reverse, "∀ E : ES, E ∘ E⁻¹ = ε")
{
    FORALL(tnt_string, s1,
           FORALL
           (tnt_string, s2,
            PRODUCING
            (tn_edit_item, diff12,
             PRODUCING
             (tn_edit_item, diff21,
              PRODUCING
              (tn_edit_item, compose,
               PRODUCING
               (tn_edit_item, scompose,
                tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                          &diff12_buffer);
                tn_edit_generate_sequence(s2.n, s2.str, s1.n, s1.str,
                                          &diff21_buffer);
                tn_edit_compose_sequence(diff12_buffer.len / sizeof(*diff12),
                                         diff12,
                                         diff21_buffer.len / sizeof(*diff21),
                                         diff21,
                                         &compose_buffer);
                tn_edit_squeeze_sequence(compose_buffer.len / sizeof(*compose),
                                         compose, s1.n, s1.str,
                                         &scompose_buffer);
                tnt_assert_op(size_t, scompose_buffer.len, ==, 0);
                tnt_assert_op(ptr, scompose, ==, NULL)))))));
}


TESTDEF(test_edit_squeeze, "Squeezing a generated edit sequence is a no-op")
{
    FORALL(tnt_string, s1,
           FORALL
           (tnt_string, s2,
            PRODUCING
            (tn_edit_item, edits,
             PRODUCING
             (tn_edit_item, sq_edits,
              tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                        &edits_buffer);
              tn_edit_squeeze_sequence(edits_buffer.len / sizeof(*edits), edits,
                                       s1.n, s1.str, &sq_edits_buffer);
              tnt_assert(tn_edit_seq_eq(edits_buffer.len / sizeof(*edits),
                                        edits,
                                        sq_edits_buffer.len / sizeof(*sq_edits),
                                        sq_edits))))));
}

TESTDEF(test_edit_squeeze_prefix,
        "Squeezing relating to a prefix of an original string "
        "yields a prefix of the original edit sequence")
{
    FORALL(tnt_string, s1,
           FORALL
           (tnt_string, s2,
            PRODUCING
            (tn_edit_item, edits,
             PRODUCING
             (tn_edit_item, sq_edits,
              unsigned pfx;

              pfx = tn_random_int(0, s1.n - 1);
              tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                        &edits_buffer);
              tn_edit_squeeze_sequence(edits_buffer.len / sizeof(*edits), edits,
                                       pfx, s1.str, &sq_edits_buffer);
              tnt_assert_op(size_t, sq_edits_buffer.len, <=, edits_buffer.len);
              tnt_assert(tn_edit_seq_eq(sq_edits_buffer.len / sizeof(*sq_edits),
                                        edits,
                                        sq_edits_buffer.len / sizeof(*sq_edits),
                                        sq_edits))))));
}


TESTDEF(test_edit_apply_squeeze,
        "Applying a squeezed edit sequence yields the same result "
        "as the original sequence")
{
    FORALL(tnt_edit_seq, seq,
           FORALL
           (tnt_string, s,
            PRODUCING
            (tn_edit_item, sq_seq,
             PRODUCING
             (uint32_t, s1,
              PRODUCING
              (uint32_t, s2,
               tn_edit_apply_sequence(s.n, s.str,
                                      seq.n, seq.edits,
                                      &s1_buffer);
               tn_edit_squeeze_sequence(seq.n, seq.edits,
                                        s.n, s.str,
                                        &sq_seq_buffer);
               tnt_trivial(tn_edit_seq_eq(seq.n, seq.edits,
                                          sq_seq_buffer.len / sizeof(*sq_seq),
                                          sq_seq));
               tn_edit_apply_sequence(s.n, s.str,
                                      sq_seq_buffer.len / sizeof(*sq_seq),
                                      sq_seq, &s2_buffer);
               tnt_assert_mem(s1_buffer.len, s1, s2_buffer.len, s2))))));
}

TESTDEF_SINGLE(test_edit_lcs_identity, "∀ s : S, lcs(s, s) = s")
{
    FORALL(tnt_string, s,
           PRODUCING
           (uint32_t, lcs,
            tn_edit_generate_lcs(s.n, s.str, s.n, s.str,
                                 &lcs_buffer);
            tnt_assert_mem(s.n * sizeof(*s.str), s.str,
                           lcs_buffer.len, lcs)));
}


TESTDEF_SINGLE(test_edit_lcs_empty_right, "∀ s : S, lcs(s, ε) = ε")
{
    FORALL(tnt_string, s,
           PRODUCING(uint32_t, lcs,
                     tn_edit_generate_lcs(s.n, s.str, 0, NULL, &lcs_buffer);
                     tnt_assert_op(size_t, lcs_buffer.len, ==, 0);
                     tnt_assert_op(ptr, lcs, ==, NULL)));
}

TESTDEF_SINGLE(test_edit_lcs_empty_left,
               "∀ s : S, lcs(ε, s) = ε")
{
    FORALL(tnt_string, s,
           PRODUCING(uint32_t, lcs,
                     tn_edit_generate_lcs(0, NULL, s.n, s.str, &lcs_buffer);
                     tnt_assert_op(size_t, lcs_buffer.len, ==, 0);
                     tnt_assert_op(ptr, lcs, ==, NULL)));
}

TESTDEF(test_edit_lcs_symm, "∀ s₁, s₂ : S, lcs(s₁, s₂) = lcs(s₂, s₁)")
{
    FORALL(tnt_string, s1,
           FORALL
           (tnt_string, s2,
            PRODUCING
            (uint32_t, lcs1,
             PRODUCING
             (uint32_t, lcs2,

              tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str,
                                   &lcs1_buffer);
              tn_edit_generate_lcs(s2.n, s2.str, s1.n, s1.str,
                                   &lcs2_buffer);
              tnt_assert_mem(lcs1_buffer.len, lcs1, lcs2_buffer.len, lcs2)))));
}

TESTDEF(test_edit_lcs_disjoint,
        "Strings that have no common characters "
        "have empty LCS")
{
    PRODUCING(uint32_t, lcs,
              tnt_string s1 = {0, NULL};
              tnt_string s2 = {0, NULL};
              unsigned i;

              s1.n = tn_random_int(1, 128);
              s2.n = tn_random_int(1, 128);
              tn_alloc_raw(TN_GLOC(s1.str), s1.n * sizeof(*s1.str));
              tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));
              for (i = 0; i < s1.n; i++)
                  s1.str[i] = i;
              for (i = 0; i < s2.n; i++)
                  s2.str[i] = s1.n + i;
              tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str,
                                   &lcs_buffer);
              tnt_assert_op(size_t, lcs_buffer.len, ==, 0);
              tnt_assert_op(ptr, lcs, ==, NULL);
              tnt_string_cleanup(&s1);
              tnt_string_cleanup(&s2));
}

TESTDEF(test_edit_lcs_dist,
        "∀ s₁, s₂ : S, Δ(s₁, lcs(s₁, s₂)) = |s₁| - |lcs(s₁, s₂)|")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  PRODUCING
                  (uint32_t, lcs,
                   size_t dist1;
                   size_t dist2;

                   tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str,
                                        &lcs_buffer);
#ifndef TN_DEBUG_DISABLED
                   tnt_assert_op(size_t, tn_edit_seq_memory_utilization, <=,
                                 (s1.n + 1) * (s2.n + 1));
#endif
                   dist1 = tn_edit_distance(s1.n, s1.str,
                                            lcs_buffer.len / sizeof(*lcs), lcs);
                   dist2 = tn_edit_distance(s2.n, s2.str,
                                            lcs_buffer.len / sizeof(*lcs), lcs);
                   tnt_assert_op(size_t, dist1, ==,
                                 s1.n - lcs_buffer.len / sizeof(*lcs));
                   tnt_assert_op(size_t, dist2, ==,
                                 s2.n - lcs_buffer.len / sizeof(*lcs)))));
}

TESTDEF(test_edit_lcs_common,
        "A lcs of two strings is indeed include in both of them")
{
    FORALL(tnt_string, s1,
           FORALL(tnt_string, s2,
                  PRODUCING
                  (uint32_t, lcs,
                   uint32_t *iter1;
                   uint32_t *iter2;
                   unsigned i;

                   tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str,
                                        &lcs_buffer);
                   iter1 = s1.str;
                   iter2 = s2.str;
                   for (i = 0; i < lcs_buffer.len / sizeof(*lcs); i++)
                   {
                       uint32_t *next1 = u32_chr(iter1, s1.n - (iter1 - s1.str),
                                                 lcs[i]);
                       uint32_t *next2 = u32_chr(iter2, s2.n - (iter2 - s2.str),
                                                 lcs[i]);

                       tnt_assert_op(ptr, next1, !=, NULL);
                       tnt_assert_op(ptr, next2, !=, NULL);
                       iter1 = next1 + 1;
                       iter2 = next2 + 1;
                   })));
}

TESTDEF_SINGLE(test_edit_merge_seq_self, "∀ E : ES, E ∪ E = E")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, merge,
            tnt_assert(tn_edit_merge_sequence(seq.n, seq.edits,
                                              seq.n, seq.edits,
                                              &merge_buffer));
            tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                      merge_buffer.len / sizeof(*merge),
                                      merge))));
}

TESTDEF_SINGLE(test_edit_merge_seq_empty_right, "∀ E : ES, E ∪ ∅ = E")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, merge,
            tnt_assert(tn_edit_merge_sequence(seq.n, seq.edits,
                                              0, NULL,
                                              &merge_buffer));
            tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                      merge_buffer.len / sizeof(*merge),
                                      merge))));
}


TESTDEF_SINGLE(test_edit_merge_seq_empty_left, "∀ E : ES, ∅ ∪ E = E")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, merge,
            tnt_assert(tn_edit_merge_sequence(0, NULL,
                                              seq.n, seq.edits,
                                              &merge_buffer));
            tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                      merge_buffer.len / sizeof(*merge),
                                      merge))));
}

TESTDEF(test_edit_merge_seq_comm, "∀ E₁, E₂ : ES, E₁ ∪ E₂ = E₂ ∪ E₁")
{
    FORALL(tnt_edit_seq, seq1,
           FORALL
           (tnt_edit_seq, seq2,
            PRODUCING
            (tn_edit_item, merge1,
             PRODUCING
             (tn_edit_item, merge2,
              bool ok1, ok2;

              ok1 = tn_edit_merge_sequence(seq1.n, seq1.edits,
                                           seq2.n, seq2.edits,
                                           &merge1_buffer);
              ok2 = tn_edit_merge_sequence(seq2.n, seq2.edits,
                                           seq1.n, seq1.edits,
                                           &merge2_buffer);
              tnt_assert_equiv(ok1, ok2);
              if (ok1)
              {
                  tnt_assert(tn_edit_seq_eq(merge1_buffer.len / sizeof(*merge1),
                                            merge1,
                                            merge2_buffer.len / sizeof(*merge2),
                                            merge2));
              }))));
}

TESTDEF(test_edit_merge_seq_split,
        "A merge of a randomly split sequence brings it back")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, edits1,
            PRODUCING
            (tn_edit_item, edits2,
             PRODUCING
             (tn_edit_item, merge,
              tn_buffer *current = &edits1_buffer;
              unsigned i;

              for (i = 0; i < seq.n; i++)
              {
                  if (!tn_edit_is_insert(&seq.edits[i]))
                  {
                      current = tn_random_int(0, 1) ?
                          &edits1_buffer :
                          &edits2_buffer;
                  }
                  *TN_BUFFER_PUSH(current, tn_edit_item, 1) = seq.edits[i];
              }

              tnt_assert(tn_edit_merge_sequence(edits1_buffer.len /
                                                sizeof(*edits1), edits1,
                                                edits2_buffer.len /
                                                sizeof(*edits2),
                                                edits2,
                                                &merge_buffer));
              tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                        merge_buffer.len / sizeof(*merge),
                                        merge))))));
}

TESTDEF(test_edit_merge_seq_reverse_conflict,
        "Merging an edit sequence with its reverse results in a conflict")
{
    FORALL(tnt_string, s1,
           FORALL
           (tnt_string, s2,
            PRODUCING
            (tn_edit_item, diff12,
             PRODUCING
             (tn_edit_item, diff21,
              PRODUCING
              (tn_edit_item, merge,

               if (u32_cmp2(s1.str, s1.n, s2.str, s2.n) == 0)
               {
                   tnt_trivial(true);
                   return;
               }
               tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str,
                                         &diff12_buffer);
               tn_edit_generate_sequence(s2.n, s2.str, s1.n, s1.str,
                                         &diff21_buffer);
               tnt_assert(!tn_edit_merge_sequence(diff12_buffer.len /
                                                  sizeof(*diff12),
                                                  diff12,
                                                  diff21_buffer.len /
                                                  sizeof(*diff21),
                                                  diff21,
                                                  &merge_buffer)))))));
}

TESTDEF_SINGLE(test_edit_intersect_seq_self, "∀ E : ES, E ∩ E = E")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, inter,
                     tn_edit_intersect_sequence(seq.n, seq.edits,
                                                seq.n, seq.edits,
                                                &inter_buffer);
                     tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                               inter_buffer.len /
                                               sizeof(*inter),
                                               inter))));
}

TESTDEF_SINGLE(test_edit_intersect_seq_empty_right, "∀ E : ES, E ∩ ∅ = ∅")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, inter,
                     tn_edit_intersect_sequence(seq.n, seq.edits, 0, NULL,
                                                &inter_buffer);
                     tnt_assert_op(size_t, inter_buffer.len, ==, 0);
                     tnt_assert_op(ptr, inter, ==, NULL)));
}

TESTDEF_SINGLE(test_edit_intersect_seq_empty_left, "∀ E : ES, ∅ ∩ E = ∅")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, inter,
                     tn_edit_intersect_sequence(0, NULL, seq.n, seq.edits,
                                                &inter_buffer);
                     tnt_assert_op(size_t, inter_buffer.len, ==, 0);
                     tnt_assert_op(ptr, inter, ==, NULL)));
}

TESTDEF(test_edit_intersect_seq_comm, "∀ E₁, E₂ : ES, E₁ ∩ E₂ = E₂ ∩ E₁")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, seq1,
            PRODUCING
            (tn_edit_item, seq2,
             PRODUCING
             (tn_edit_item, inter1,
              PRODUCING
              (tn_edit_item, inter2,
               unsigned i;

               for (i = 0; i < seq.n; i++)
               {
                   switch (tn_random_int(0, 2))
                   {
                       case 0:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 1:
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 2:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       default:
                           abort();
                   }
               }
               tn_edit_intersect_sequence(seq1_buffer.len / sizeof(*seq1), seq1,
                                          seq2_buffer.len / sizeof(*seq2), seq2,
                                          &inter1_buffer);
               tn_edit_intersect_sequence(seq2_buffer.len / sizeof(*seq2), seq2,
                                          seq1_buffer.len / sizeof(*seq1), seq1,
                                          &inter2_buffer);

               tnt_assert(tn_edit_seq_eq(inter1_buffer.len / sizeof(*inter1),
                                         inter1,
                                         inter2_buffer.len / sizeof(*inter2),
                                         inter2)))))));
}


TESTDEF(test_edit_intersect_seq_split_disjoint,
        "Intersection of disjoint sequences is empty")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, edits1,
            PRODUCING
            (tn_edit_item, edits2,
             PRODUCING
             (tn_edit_item, inter,
              unsigned i;

              for (i = 0; i < seq.n; i++)
              {
                  tn_buffer *current = tn_random_int(0, 1) ?
                      &edits1_buffer : &edits2_buffer;
                  *TN_BUFFER_PUSH(current, tn_edit_item, 1) = seq.edits[i];
              }

              tn_edit_intersect_sequence(edits1_buffer.len / sizeof(*edits1),
                                         edits1,
                                         edits2_buffer.len / sizeof(*edits2),
                                         edits2,
                                         &inter_buffer);
              tnt_assert_op(size_t, inter_buffer.len, ==, 0);
              tnt_assert_op(ptr, inter, ==, NULL)))));
}

TESTDEF(test_edit_intersect_seq_split,
        "Intersection of a split sequence is its common elements")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, seq1,
            PRODUCING
            (tn_edit_item, seq2,
             PRODUCING
             (tn_edit_item, common,
              PRODUCING
              (tn_edit_item, inter,
               unsigned i;

               for (i = 0; i < seq.n; i++)
               {
                   switch (tn_random_int(0, 2))
                   {
                       case 0:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 1:
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 2:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           *TN_BUFFER_PUSH(&common_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       default:
                           abort();
                   }
               }

               tn_edit_intersect_sequence(seq1_buffer.len / sizeof(*seq1), seq1,
                                          seq2_buffer.len / sizeof(*seq2), seq2,
                                          &inter_buffer);
               tnt_assert(tn_edit_seq_eq(common_buffer.len / sizeof(*common),
                                         common,
                                         inter_buffer.len / sizeof(*inter),
                                         inter)))))));
}

TESTDEF_SINGLE(test_edit_diff_seq_self, "∀ E : ES, E ∖ E = ∅")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, diff,
                     tn_edit_except_sequence(seq.n, seq.edits,
                                             seq.n, seq.edits,
                                             &diff_buffer);
                     tnt_assert_op(size_t, diff_buffer.len, ==, 0);
                     tnt_assert_op(ptr, diff, ==, NULL)));
}

TESTDEF_SINGLE(test_edit_diff_seq_empty_right, "∀ E : ES, E ∖ ∅ = E")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, diff,
                     tn_edit_except_sequence(seq.n, seq.edits, 0, NULL,
                                             &diff_buffer);
                     tnt_assert(tn_edit_seq_eq(seq.n, seq.edits,
                                               diff_buffer.len /
                                               sizeof(*diff),
                                               diff))));
}

TESTDEF_SINGLE(test_edit_diff_seq_empty_left, "∀ E : ES, ∅ ∖ E = ∅")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING(tn_edit_item, diff,
                     tn_edit_except_sequence(0, NULL, seq.n, seq.edits,
                                             &diff_buffer);
                     tnt_assert_op(size_t, diff_buffer.len, ==, 0);
                     tnt_assert_op(ptr, diff, ==, NULL)));
}

TESTDEF(test_edit_diff_seq_split_disjoint,
        "Difference of disjoint sequences is equal to the first one")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, edits1,
            PRODUCING
            (tn_edit_item, edits2,
             PRODUCING
             (tn_edit_item, diff,
              unsigned i;

              for (i = 0; i < seq.n; i++)
              {
                  tn_buffer *current = tn_random_int(0, 1) ?
                      &edits1_buffer : &edits2_buffer;
                  *TN_BUFFER_PUSH(current, tn_edit_item, 1) = seq.edits[i];
              }

              tn_edit_except_sequence(edits1_buffer.len / sizeof(*edits1),
                                      edits1,
                                      edits2_buffer.len / sizeof(*edits2),
                                      edits2,
                                      &diff_buffer);
              tnt_assert(tn_edit_seq_eq(edits1_buffer.len / sizeof(*edits1),
                                        edits1,
                                        diff_buffer.len / sizeof(*diff),
                                        diff))))));
}

TESTDEF(test_edit_diff_seq_split,
        "Difference of a split sequence does not include common elements")
{
    FORALL(tnt_edit_seq, seq,
           PRODUCING
           (tn_edit_item, seq1,
            PRODUCING
            (tn_edit_item, seq2,
             PRODUCING
             (tn_edit_item, nocommon,
              PRODUCING
              (tn_edit_item, diff,
               unsigned i;

               for (i = 0; i < seq.n; i++)
               {
                   switch (tn_random_int(0, 2))
                   {
                       case 0:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           *TN_BUFFER_PUSH(&nocommon_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 1:
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       case 2:
                           *TN_BUFFER_PUSH(&seq1_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           *TN_BUFFER_PUSH(&seq2_buffer, tn_edit_item, 1) =
                               seq.edits[i];
                           break;
                       default:
                           abort();
                   }
               }

               tn_edit_except_sequence(seq1_buffer.len / sizeof(*seq1), seq1,
                                       seq2_buffer.len / sizeof(*seq2), seq2,
                                       &diff_buffer);
               tnt_assert(tn_edit_seq_eq(nocommon_buffer.len /
                                         sizeof(*nocommon),
                                         nocommon,
                                         diff_buffer.len / sizeof(*diff),
                                         diff)))))));
}

TESTDEF(test_edit_diff_samepos,
        "Difference at the same position but with different operations "
        "work as expected")
{
    PRODUCING(tn_edit_item, diff,
              size_t pos = tn_random_int(0, INT32_MAX);
              ucs4_t ch1 = tn_random_int(0, INT32_MAX);
              ucs4_t ch2 = tn_random_int(0, INT32_MAX);
              bool is_insert1 = tn_random_int(0, 1);
              bool is_insert2 = tn_random_int(0, 1);
              tn_edit_item seq1 = tn_edit_make_item(pos, is_insert1, ch1);
              tn_edit_item seq2 = tn_edit_make_item(pos, is_insert2, ch2);

              tn_edit_except_sequence(1, &seq1, 1, &seq2, &diff_buffer);
              tnt_assert_equiv(is_insert1 != is_insert2 || ch1 != ch2,
                               diff_buffer.len == sizeof(*diff)));
}
