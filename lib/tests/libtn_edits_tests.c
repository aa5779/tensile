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
           tn_free(TN_GLOC(s2.str)));
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
           tn_free(TN_GLOC(s2.str)));
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
           tn_free(TN_GLOC(s2.str)));
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
              unsigned len;
              unsigned i;
              uint32_t *str1 = NULL;
              uint32_t *str2 = NULL;

              len = tn_random_int(1, 128);
              tn_alloc_raw(TN_GLOC(str1), len * sizeof(*str1));
              tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));

              for (i = 0; i < len; i++)
              {
                  str1[i] = i;
                  str2[i] = len + i;
              }

              tn_edit_generate_sequence(len, str1, len, str2, &edits_buffer);
              tnt_assert_op(size_t, edits_buffer.len, ==, len * sizeof(*edits));
              for (i = 0; i < len; i++)
              {
                  tnt_assert_op(size_t, edits[i].pos, ==, i);
                  tnt_assert_op(ucs4_t, edits[i].ch, ==, str2[i]);
              }
              tn_free(TN_GLOC(str1));
              tn_free(TN_GLOC(str2));
        );
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
            tn_free(TN_GLOC(s2.str))));
}

/*
TESTDEF(test_edit_seq_ins_single, "")
    unsigned len;
    uint32_t *str;
    uint32_t *s2.str = NULL;
    unsigned pos;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len);
    tn_alloc_raw(TN_GLOC(s2.str), (len + 1) * sizeof(*s2.str));
    memcpy(s2.str, str, pos * sizeof(*s2.str));
    memcpy(s2.str + pos + 1, str + pos, (len - pos) * sizeof(*s2.str));
    s2.str[pos] = tn_random_int(0, INT32_MAX);
    tn_edit_generate_sequence(len, str, len + 1, s2.str, &dest);
    tnt_assert_op(, dest.len, ==, sizeof(*edits));
    tnt_assert_op(, edits[0].pos, ==, pos | TN_EDIT_INSERT);
    tnt_assert_op(, edits[0].ch, ==, s2.str[pos]);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(edits));

TESTDEF(test_edit_seq_ins_subst, "")
    unsigned len;
    uint32_t *str;
    uint32_t *s2.str = NULL;
    unsigned pos;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(s2.str), (len + 1) * sizeof(*s2.str));
    memcpy(s2.str, str, pos * sizeof(*s2.str));
    memcpy(s2.str + pos + 1, str + pos, (len - pos) * sizeof(*s2.str));
    s2.str[pos] = tn_random_int(0, INT32_MAX);
    s2.str[pos + 1] ^= 1;
    tn_edit_generate_sequence(len, str, len + 1, s2.str, &dest);
    tnt_assert_op(, dest.len, ==, 2 * sizeof(*edits));
    tnt_assert_op(, edits[0].pos, ==, pos | TN_EDIT_INSERT);
    tnt_assert_op(, edits[0].ch, ==, s2.str[pos]);
    tnt_assert_op(, edits[1].pos, ==, pos);
    tnt_assert_op(, edits[1].ch, ==, s2.str[pos + 1]);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(edits));


TESTDEF(test_edit_seq_del_single, "")
    unsigned len;
    uint32_t *str;
    uint32_t *s2.str = NULL;
    unsigned pos;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(s2.str), len * sizeof(*s2.str));
    memcpy(s2.str, str, pos * sizeof(*s2.str));
    memcpy(s2.str + pos, str + pos + 1, (len - pos - 1) * sizeof(*s2.str));
    tn_edit_generate_sequence(len, str, len - 1, s2.str, &dest);
    tnt_assert_op(, dest.len, ==, sizeof(*edits));
    tnt_assert_op(, edits[0].pos, ==, pos);
    tnt_assert_op(, edits[0].ch, ==, TN_INVALID_CHAR);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(edits));

TESTDEF_SINGLE(test_edit_apply_empty, "")
    unsigned len;
    uint32_t *str;
    uint32_t *s2.str = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(s2.str), 0, 0);

    str = generate_random_string(&len);
    tn_edit_apply_sequence(len, str, 0, NULL, &dest);
    tnt_assert_op(, dest.len, ==, len * sizeof(*s2.str));
    tnt_assert(memcmp(s2.str, str, len * sizeof(*s2.str)) == 0);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_apply_any, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);
    uint32_t *str3 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(str3), 0, 0);

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str, &dest);
    tn_edit_apply_sequence(s1.n, s1.str, dest.len / sizeof(*edits),
                           edits, &dest2);
    tnt_assert_op(, dest2.len, ==, s2.n * sizeof(*str3));
    tnt_assert(memcmp(str3, s2.str, s2.n * sizeof(*str3)) == 0);
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(edits));

TESTDEF(test_edit_apply_single_subst, "")
    unsigned len;
    uint32_t *str;
    uint32_t *s2.str = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(s2.str), 0, 0);
    unsigned pos;
    ucs4_t ch;

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    ch = tn_random_int(0, INT32_MAX);
    tn_edit_apply_sequence(len, str, 1, &(tn_edit_item){pos, ch}, &dest);
    str[pos] = ch;
    tnt_assert_op(, dest.len, ==, len * sizeof(*s2.str));
    tnt_assert(memcmp(s2.str, str, len * sizeof(*s2.str)) == 0);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_apply_prefix, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    unsigned newlen;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);
    uint32_t *str3 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(str3), 0, 0);

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str, &dest);
    newlen = tn_random_int(0, s1.n - 1);
    tn_edit_apply_sequence(newlen, s1.str, dest.len / sizeof(*edits),
                           edits, &dest2);
    tnt_assert_op(, dest2.len, <, s2.n * sizeof(*str3));
    tnt_assert(memcmp(str3, s2.str, dest2.len) == 0);
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(edits));
*/

TESTDEF_SINGLE(test_edit_random_sanity,
               "Random edit sequence is valid")
{
    FORALL(tnt_edit_seq, es,
           tnt_assert(tn_edit_seq_valid(es.n, es.edits));
           tnt_assert(tn_edit_seq_eq(es.n, es.edits, es.n, es.edits)));
}

/*
TESTDEF(test_edit_item_shift_0, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, 0);

    tnt_assert(tn_edit_same_position(&item, &sh));
    tnt_assert_op(, item.ch, ==, sh.ch);

TESTDEF(test_edit_item_shift, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    int delta = tn_random_int(-pos, INT32_MAX - pos);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, delta);

    tnt_assert_op(, tn_edit_position(&item) + delta, ==,
                      tn_edit_position(&sh));
    tnt_assert_op(, item.ch, ==, sh.ch);


TESTDEF_SINGLE(test_edit_compose_seq_empty_right, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tn_edit_compose_sequence(n, edits, 0, NULL, &dest);
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));

TESTDEF_SINGLE(test_edit_compose_seq_empty_left, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tn_edit_compose_sequence(0, NULL, n, edits, &dest);
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_subst, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, sizeof(*result));
    tnt_assert_op(, result->pos, ==, pos);
    tnt_assert_op(, result->ch, ==, ch2);
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_insert_subst, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, sizeof(*result));
    tnt_assert_op(, result->pos, ==, pos | TN_EDIT_INSERT);
    tnt_assert_op(, result->ch, ==, ch2);
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_insert2, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, true, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, 2 * sizeof(*result));
    tnt_assert_op(, result[0].pos, ==, pos | TN_EDIT_INSERT);
    tnt_assert_op(, result[0].ch, ==, ch2);
    tnt_assert_op(, result[1].pos, ==, pos | TN_EDIT_INSERT);
    tnt_assert_op(, result[1].ch, ==, ch1);
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_nullify, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch);
    tn_edit_item item2 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, result, ==, NULL);

TESTDEF(test_edit_compose_delete_insert, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item item2 = tn_edit_make_item(pos, true, ch);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, sizeof(*result));
    tnt_assert_op(, result->pos, ==, pos);
    tnt_assert_op(, result->ch, ==, ch);
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_delete_delete, "")
    size_t pos = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item item2 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, 2 * sizeof(*result));
    tnt_assert_op(, result[0].pos, ==, pos);
    tnt_assert_op(, result[0].ch, ==, TN_INVALID_CHAR);
    tnt_assert_op(, result[1].pos, ==, pos + 1);
    tnt_assert_op(, result[1].ch, ==, TN_INVALID_CHAR);
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose_disjoint, "")
    size_t pos1 = tn_random_int(0, INT32_MAX - 1);
    size_t pos2 = tn_random_int(pos1 + 1, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos1, false, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos2, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    tnt_assert_op(, dest.len, ==, 2 * sizeof(*result));
    tnt_assert_op(, result[0].pos, ==, pos1);
    tnt_assert_op(, result[0].ch, ==, ch1);
    tnt_assert_op(, result[1].pos, ==, pos2);
    tnt_assert_op(, result[1].ch, ==, ch2);
    tn_free(TN_GLOC(result));


TESTDEF(test_edit_compose_valid, "")
    unsigned n1;
    tn_edit_item *edits1 = NULL;
    unsigned n2;
    tn_edit_item *edits2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits1 = generate_random_edit_seq(&n1);
    edits2 = generate_random_edit_seq(&n2);
    tn_edit_compose_sequence(n1, edits1, n2, edits2, &dest);
    tnt_assert(tn_edit_seq_valid(dest.len / sizeof(*result),
                                result));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits2));
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_compose, "")
    unsigned n1;
    unsigned n2;
    unsigned n3;
    uint32_t *s1.str = NULL;
    uint32_t *s2.str = NULL;
    uint32_t *str3 = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff23 = NULL;
    tn_buffer dest23 = TN_BUFFER_INIT(TN_GLOC(diff23), 0, 0);
    tn_edit_item *compose = NULL;
    tn_buffer dest_compose = TN_BUFFER_INIT(TN_GLOC(compose), 0, 0);
    uint32_t *cstr = NULL;
    tn_buffer dest_cstr = TN_BUFFER_INIT(TN_GLOC(cstr), 0, 0);

    s1.str = generate_random_string(&n1);
    s2.str = generate_random_string(&n2);
    str3 = generate_random_string(&n3);
    tn_edit_generate_sequence(n1, s1.str, n2, s2.str, &dest12);
    tn_edit_generate_sequence(n2, s2.str, n3, str3, &dest23);
    tn_edit_compose_sequence(dest12.len / sizeof(*diff12), diff12,
                             dest23.len / sizeof(*diff23), diff23,
                             &dest_compose);
    tn_edit_apply_sequence(n1, s1.str, dest_compose.len / sizeof(*compose),
                           compose, &dest_cstr);
    tnt_assert_op(, dest_cstr.len / sizeof(*cstr), ==, n3);
    tnt_assert(memcmp(str3, cstr, n3) == 0);

    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff23));
    tn_free(TN_GLOC(compose));
    tn_free(TN_GLOC(cstr));

TESTDEF(test_edit_compose_reverse, "")
    unsigned n1;
    unsigned n2;
    uint32_t *s1.str = NULL;
    uint32_t *s2.str = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff21 = NULL;
    tn_buffer dest21 = TN_BUFFER_INIT(TN_GLOC(diff21), 0, 0);
    tn_edit_item *compose = NULL;
    tn_buffer dest_compose = TN_BUFFER_INIT(TN_GLOC(compose), 0, 0);
    tn_edit_item *scompose = NULL;
    tn_buffer dest_scompose = TN_BUFFER_INIT(TN_GLOC(scompose), 0, 0);

    s1.str = generate_random_string(&n1);
    s2.str = generate_random_string(&n2);
    tn_edit_generate_sequence(n1, s1.str, n2, s2.str, &dest12);
    tn_edit_generate_sequence(n2, s2.str, n1, s1.str, &dest21);
    tn_edit_compose_sequence(dest12.len / sizeof(*diff12), diff12,
                             dest21.len / sizeof(*diff21), diff21,
                             &dest_compose);
    tn_edit_squeeze_sequence(dest_compose.len / sizeof(*compose),
                             compose, n1, s1.str, &dest_scompose);
    tnt_assert_op(, dest_scompose.len, ==, 0);
    tnt_assert_op(ptr, scompose, ==, NULL);
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff21));
    tn_free(TN_GLOC(compose));

TESTDEF(test_edit_squeeze, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);
    tn_edit_item *edits1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(edits1), 0, 0);

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str, &dest);
    tn_edit_squeeze_sequence(dest.len / sizeof(*edits), edits,
                             s1.n, s1.str, &dest1);
    tnt_assert(tn_edit_seq_eq(dest.len / sizeof(*edits), edits,
                             dest1.len / sizeof(*edits1), edits1));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_squeeze_prefix, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    tn_edit_item *edits = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(edits), 0, 0);
    tn_edit_item *edits1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(edits1), 0, 0);
    unsigned pfx;

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    pfx = tn_random_int(0, s1.n - 1);
    tn_edit_generate_sequence(s1.n, s1.str, s2.n, s2.str, &dest);
    tn_edit_squeeze_sequence(dest.len / sizeof(*edits), edits,
                             pfx, s1.str, &dest1);
    tnt_assert_op(, dest1.len, <=, dest.len);
    tnt_assert(tn_edit_seq_eq(dest1.len / sizeof(*edits), edits,
                             dest1.len / sizeof(*edits), edits1));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF_SINGLE(test_edit_lcs_identity, "")
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(len, str, len, str, &dest);
    tnt_assert_op(, dest.len / sizeof(*nstr), ==, len);
    tnt_assert(memcmp(nstr, str, len * sizeof(*str)) == 0);
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(str));

TESTDEF_SINGLE(test_edit_lcs_empty_right, "")
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(len, str, 0, NULL, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, nstr, ==, NULL);
    tn_free(TN_GLOC(str));

TESTDEF_SINGLE(test_edit_lcs_empty_left, "")
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(0, NULL, len, str, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, nstr, ==, NULL);
    tn_free(TN_GLOC(str));


TESTDEF(test_edit_lcs_symm, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    uint32_t *ns1.str = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(ns1.str), 0, 0);

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str, &dest);
    tn_edit_generate_lcs(s2.n, s2.str, s1.n, s1.str, &dest1);
    tnt_assert_op(, dest.len, ==, dest1.len);
    tnt_assert(memcmp(nstr, ns1.str, dest.len) == 0);
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(ns1.str));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_lcs_disjoint, "")
    unsigned s1.n;
    uint32_t *s1.str = NULL;
    unsigned s2.n;
    uint32_t *s2.str = NULL;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    unsigned i;

    s1.n = tn_random_int(1, 128);
    s2.n = tn_random_int(1, 128);
    tn_alloc_raw(TN_GLOC(s1.str), s1.n * sizeof(*s1.str));
    tn_alloc_raw(TN_GLOC(s2.str), s2.n * sizeof(*s2.str));
    for (i = 0; i < s1.n; i++)
        s1.str[i] = i;
    for (i = 0; i < s2.n; i++)
        s2.str[i] = s1.n + i;
    tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, nstr, ==, NULL);
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_lcs_dist, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    size_t dist1;
    size_t dist2;

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str, &dest);
    #ifndef TN_DEBUG_DISABLED
    tnt_assert_op(, tn_edit_seq_memory_utilization, <=, (s1.n + 1) * (s2.n + 1));
    #endif
    dist1 = tn_edit_distance(s1.n, s1.str, dest.len / sizeof(*nstr), nstr);
    dist2 = tn_edit_distance(s2.n, s2.str, dest.len / sizeof(*nstr), nstr);
    tnt_assert_op(, dist1, ==, s1.n - dest.len / sizeof(*nstr));
    tnt_assert_op(, dist2, ==, s2.n - dest.len / sizeof(*nstr));
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF(test_edit_lcs_common, "")
    unsigned s1.n;
    uint32_t *s1.str;
    unsigned s2.n;
    uint32_t *s2.str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    uint32_t *iter1;
    uint32_t *iter2;
    unsigned i;

    s1.str = generate_random_string(&s1.n);
    s2.str = generate_random_string(&s2.n);
    tn_edit_generate_lcs(s1.n, s1.str, s2.n, s2.str, &dest);
    iter1 = s1.str;
    iter2 = s2.str;
    for (i = 0; i < dest.len / sizeof(*nstr); i++)
    {
        uint32_t *next1 = u32_chr(iter1, s1.n - (iter1 - s1.str), nstr[i]);
        uint32_t *next2 = u32_chr(iter2, s2.n - (iter2 - s2.str), nstr[i]);

        tnt_assert_op(ptr, next1, !=, NULL);
        tnt_assert_op(ptr, next2, !=, NULL);
        iter1 = next1 + 1;
        iter2 = next2 + 1;
    }

    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));

TESTDEF_SINGLE(test_edit_merge_seq_self, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tnt_assert(tn_edit_merge_sequence(n, edits, n, edits, &dest));
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));


TESTDEF_SINGLE(test_edit_merge_seq_empty_right, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tnt_assert(tn_edit_merge_sequence(n, edits, 0, NULL, &dest));
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));

TESTDEF_SINGLE(test_edit_merge_seq_empty_left, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tnt_assert(tn_edit_merge_sequence(0, NULL, n, edits, &dest));
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_merge_seq_comm, "")
    unsigned n1;
    unsigned n2;
    tn_edit_item *edits1 = NULL;
    tn_edit_item *edits2 = NULL;
    tn_edit_item *result1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(result1), 0, 0);
    tn_edit_item *result2 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(result2), 0, 0);
    bool ok1, ok2;

    edits1 = generate_random_edit_seq(&n1);
    edits2 = generate_random_edit_seq(&n2);
    ok1 = tn_edit_merge_sequence(n1, edits1, n2, edits2, &dest1);
    ok2 = tn_edit_merge_sequence(n2, edits2, n1, edits1, &dest2);
    if (!ok1)
       tnt_assert(!ok2);
    else
    {
        tnt_assert(ok2);
        tnt_assert(tn_edit_seq_eq(dest1.len / sizeof(*result1), result1,
                                 dest2.len / sizeof(*result2), result2));
    }
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits2));
    tn_free(TN_GLOC(result1));
    tn_free(TN_GLOC(result2));


TESTDEF(test_edit_merge_seq_split, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *edits1 = NULL;
    tn_edit_item *edits2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(edits1), 0, 0);
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(edits2), 0, 0);
    tn_buffer *current = &dest1;
    unsigned i;

    edits = generate_random_edit_seq(&n);
    for (i = 0; i < n; i++)
    {
        if (!tn_edit_is_insert(&edits[i]))
        {
            current = tn_random_int(0, 1) ? &dest1 : &dest2;
        }
        *TN_BUFFER_PUSH(current, tn_edit_item, 1) = edits[i];
    }

    tnt_assert(tn_edit_merge_sequence(dest1.len / sizeof(*edits1), edits1,
                                     dest2.len / sizeof(*edits2), edits2,
                                     &dest));
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result), result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits2));
    tn_free(TN_GLOC(result));

TESTDEF(test_edit_merge_seq_reverse_conflict, "")
    unsigned n1;
    unsigned n2;
    uint32_t *s1.str = NULL;
    uint32_t *s2.str = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff21 = NULL;
    tn_buffer dest21 = TN_BUFFER_INIT(TN_GLOC(diff21), 0, 0);
    tn_edit_item *merge = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(merge), 0, 0);

    s1.str = generate_random_string(&n1);
    s2.str = generate_random_string(&n2);
    if (TN_UNLIKELY(u32_cmp2(s1.str, n1, s2.str, n2) == 0))
       return;
    tn_edit_generate_sequence(n1, s1.str, n2, s2.str, &dest12);
    tn_edit_generate_sequence(n2, s2.str, n1, s1.str, &dest21);
    tnt_assert(!tn_edit_merge_sequence(dest12.len / sizeof(*diff12), diff12,
                                      dest21.len / sizeof(*diff21), diff21,
                                      &dest));
    tn_free(TN_GLOC(s1.str));
    tn_free(TN_GLOC(s2.str));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff21));
    tn_free(TN_GLOC(merge));


TESTDEF_SINGLE(test_edit_intersect_seq_self, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(n, edits, n, edits, &dest);
    tnt_assert(tn_edit_seq_eq(n, edits,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(result));


TESTDEF_SINGLE(test_edit_intersect_seq_empty_right, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(n, edits, 0, NULL, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, result, ==, NULL);
    tn_free(TN_GLOC(edits));

TESTDEF_SINGLE(test_edit_intersect_seq_empty_left, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    edits = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(0, NULL, n, edits, &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, result, ==, NULL);
    tn_free(TN_GLOC(edits));

TESTDEF(test_edit_intersect_seq_comm, "")
    unsigned n1;
    unsigned n2;
    tn_edit_item *edits1 = NULL;
    tn_edit_item *edits2 = NULL;
    tn_edit_item *result1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(result1), 0, 0);
    tn_edit_item *result2 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(result2), 0, 0);

    edits1 = generate_random_edit_seq(&n1);
    edits2 = generate_random_edit_seq(&n2);
    tn_edit_intersect_sequence(n1, edits1, n2, edits2, &dest1);
    tn_edit_intersect_sequence(n2, edits2, n1, edits1, &dest2);
    tnt_assert(tn_edit_seq_eq(dest1.len / sizeof(*result1), result1,
                             dest2.len / sizeof(*result2), result2));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits2));
    tn_free(TN_GLOC(result1));
    tn_free(TN_GLOC(result2));


TESTDEF(test_edit_intersect_seq_split, "")
    unsigned n;
    tn_edit_item *edits = NULL;
    tn_edit_item *edits1 = NULL;
    tn_edit_item *edits2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(edits1), 0, 0);
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(edits2), 0, 0);
    unsigned i;

    edits = generate_random_edit_seq(&n);
    for (i = 0; i < n; i++)
    {
        tn_buffer *current = tn_random_int(0, 1) ? &dest1 : &dest2;
        *TN_BUFFER_PUSH(current, tn_edit_item, 1) = edits[i];
    }

    tn_edit_intersect_sequence(dest1.len / sizeof(*edits1), edits1,
                               dest2.len / sizeof(*edits2), edits2,
                               &dest);
    tnt_assert_op(, dest.len, ==, 0);
    tnt_assert_op(ptr, result, ==, NULL);
    tn_free(TN_GLOC(edits));
    tn_free(TN_GLOC(edits1));
    tn_free(TN_GLOC(edits2));
*/
