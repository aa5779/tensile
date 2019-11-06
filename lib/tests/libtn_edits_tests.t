#include <unistr.h>
#include "edits.h"

static uint32_t *
generate_random_string(unsigned *n)
{
#define MAX_STR_LEN 128
    unsigned i;
    uint32_t *dest = NULL;

    *n = (unsigned)tn_random_int(1, MAX_STR_LEN);
    tn_alloc_raw(TN_GLOC(dest), sizeof(*dest) * (*n));

    for (i = 0; i < *n; i++)
    {
        dest[i] = tn_random_int(0, INT32_MAX);
    }
    return dest;
#undef MAX_STR_LEN
}

static tn_edit_item *
generate_random_edit_seq(unsigned *n)
{
#define MAX_EDIT_LEN 128
    unsigned i;
    tn_edit_item *dest = NULL;
    size_t last = 0;

    *n = (unsigned)tn_random_int(1, MAX_EDIT_LEN);
    tn_alloc_raw(TN_GLOC(dest), sizeof(*dest) * (*n));

    for (i = 0; i < *n; i++)
    {
        dest[i].pos = tn_random_int(last, INT32_MAX - (*n) + i);
        last = dest[i].pos + 1;
        if (tn_random_int(0, 1) != 0)
        {
            dest[i].pos |= TN_EDIT_INSERT;
            dest[i].ch = tn_random_int(0, INT32_MAX);
        }
        else if (tn_random_int(0, 1) != 0)
            dest[i].ch = TN_INVALID_CHAR;
        else
            dest[i].ch = tn_random_int(0, INT32_MAX);
    }
    return dest;
#undef MAX_EDIT_LEN
}

TEST(test_edit_distance_self, OK, ONCE)
    unsigned len;
    uint32_t *str;
    size_t dist;

    str = generate_random_string(&len);
    dist = tn_edit_distance(len, str, len, str);
    ck_assert_uint_eq(dist, 0);
    tn_free(TN_GLOC(str));

TEST(test_edit_distance_empty, OK, ONCE)
    unsigned len;
    uint32_t *str;
    size_t dist;

    str = generate_random_string(&len);
    dist = tn_edit_distance(len, str, 0, NULL);
    ck_assert_uint_eq(dist, len);
    dist = tn_edit_distance(0, NULL, len, str);
    ck_assert_uint_eq(dist, len);
    tn_free(TN_GLOC(str));

TEST(test_edit_distance_symm)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    size_t dist1;
    size_t dist2;

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    dist1 = tn_edit_distance(len1, str1, len2, str2);
    dist2 = tn_edit_distance(len2, str2, len1, str1);
    ck_assert_uint_eq(dist1, dist2);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_distance_triangle)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    unsigned len3;
    uint32_t *str3;
    size_t dist1;
    size_t dist2;
    size_t dist3;

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    str3 = generate_random_string(&len3);
    dist1 = tn_edit_distance(len1, str1, len2, str2);
    dist2 = tn_edit_distance(len2, str2, len3, str3);
    dist3 = tn_edit_distance(len1, str1, len3, str3);
    ck_assert_uint_le(dist3, dist1 + dist2);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(str3));

TEST(test_edit_distance_prefix, OK, ONCE)
    unsigned len1;
    uint32_t *str;
    unsigned len2;
    size_t dist;

    str = generate_random_string(&len1);
    len2 = tn_random_int(0, len1 - 1);

    dist = tn_edit_distance(len1, str, len2, str);
    ck_assert_uint_eq(dist, len1 - len2);
    tn_free(TN_GLOC(str));

TEST(test_edit_subst_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));
    memcpy(str2, str, len * sizeof(*str2));
    str2[pos] ^= 1;
    ck_assert_uint_eq(tn_edit_distance(len, str, len, str2), 1);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));

TEST(test_edit_ins_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;

    str = generate_random_string(&len);
    pos = tn_random_int(0, len);
    tn_alloc_raw(TN_GLOC(str2), (len + 1) * sizeof(*str2));
    memcpy(str2, str, pos * sizeof(*str2));
    memcpy(str2 + pos + 1, str + pos, (len - pos) * sizeof(*str2));
    str2[pos] = tn_random_int(0, INT32_MAX);
    ck_assert_uint_eq(tn_edit_distance(len, str, len + 1, str2), 1);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));

TEST(test_edit_del_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));
    memcpy(str2, str, pos * sizeof(*str2));
    memcpy(str2 + pos, str + pos + 1, (len - pos - 1) * sizeof(*str2));
    ck_assert_uint_eq(tn_edit_distance(len, str, len - 1, str2), 1);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));

TEST(test_edit_seq_invalid, OK, ONCE)
    ck_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {0, 0}}));
    ck_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {1, 0}}));

TEST(test_edit_seq_self, OK, ONCE)
    unsigned len;
    uint32_t *str;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_sequence(len, str, len, str, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(items, NULL);
    ck_assert(tn_edit_seq_valid(0, NULL));
    tn_free(TN_GLOC(str));

TEST(test_edit_seq_dist)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    size_t dist;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    dist = tn_edit_distance(len1, str1, len2, str2);
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    ck_assert_uint_eq(dest.len, dist * sizeof(*items));
    ck_assert(tn_edit_seq_valid(dist, items));
    #ifndef TN_DEBUG_DISABLED
    ck_assert_uint_le(tn_edit_seq_memory_utilization, (len1 + 1) * (len2 + 1));
    #endif
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_empty_left, OK, ONCE)
    unsigned len;
    uint32_t *str;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    unsigned i;

    str = generate_random_string(&len);
    tn_edit_generate_sequence(0, NULL, len, str, &dest);
    ck_assert_uint_eq(dest.len, len * sizeof(*items));
    for (i = 0; i < len; i++)
    {
        ck_assert_uint_eq(items[i].pos, TN_EDIT_INSERT);
        ck_assert_uint_eq(items[i].ch, str[i]);
    }
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_empty_right, OK, ONCE)
    unsigned len;
    uint32_t *str;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    unsigned i;

    str = generate_random_string(&len);
    tn_edit_generate_sequence(len, str, 0, NULL, &dest);
    ck_assert_uint_eq(dest.len, len * sizeof(*items));
    for (i = 0; i < len; i++)
    {
        ck_assert_uint_eq(items[i].pos, i);
        ck_assert_uint_eq(items[i].ch, TN_INVALID_CHAR);
    }
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_pure_subst, OK, ONCE)
    unsigned len;
    uint32_t *str1 = NULL;
    uint32_t *str2 = NULL;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    unsigned i;

    len = tn_random_int(1, 128);
    tn_alloc_raw(TN_GLOC(str1), len * sizeof(*str1));
    tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));

    for (i = 0; i < len; i++)
    {
        str1[i] = i;
        str2[i] = len + i;
    }
    tn_edit_generate_sequence(len, str1, len, str2, &dest);
    ck_assert_uint_eq(dest.len, len * sizeof(*items));
    for (i = 0; i < len; i++)
    {
        ck_assert_uint_eq(items[i].pos, i);
        ck_assert_uint_eq(items[i].ch, str2[i]);
    }
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_subst_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));
    memcpy(str2, str, len * sizeof(*str2));
    str2[pos] ^= 1;
    tn_edit_generate_sequence(len, str, len, str2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*items));
    ck_assert_uint_eq(items[0].pos, pos);
    ck_assert_uint_eq(items[0].ch, str2[pos]);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_ins_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len);
    tn_alloc_raw(TN_GLOC(str2), (len + 1) * sizeof(*str2));
    memcpy(str2, str, pos * sizeof(*str2));
    memcpy(str2 + pos + 1, str + pos, (len - pos) * sizeof(*str2));
    str2[pos] = tn_random_int(0, INT32_MAX);
    tn_edit_generate_sequence(len, str, len + 1, str2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*items));
    ck_assert_uint_eq(items[0].pos, pos | TN_EDIT_INSERT);
    ck_assert_uint_eq(items[0].ch, str2[pos]);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

TEST(test_edit_seq_ins_subst)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(str2), (len + 1) * sizeof(*str2));
    memcpy(str2, str, pos * sizeof(*str2));
    memcpy(str2 + pos + 1, str + pos, (len - pos) * sizeof(*str2));
    str2[pos] = tn_random_int(0, INT32_MAX);
    str2[pos + 1] ^= 1;
    tn_edit_generate_sequence(len, str, len + 1, str2, &dest);
    ck_assert_uint_eq(dest.len, 2 * sizeof(*items));
    ck_assert_uint_eq(items[0].pos, pos | TN_EDIT_INSERT);
    ck_assert_uint_eq(items[0].ch, str2[pos]);
    ck_assert_uint_eq(items[1].pos, pos);
    ck_assert_uint_eq(items[1].ch, str2[pos + 1]);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));


TEST(test_edit_seq_del_single)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    unsigned pos;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    tn_alloc_raw(TN_GLOC(str2), len * sizeof(*str2));
    memcpy(str2, str, pos * sizeof(*str2));
    memcpy(str2 + pos, str + pos + 1, (len - pos - 1) * sizeof(*str2));
    tn_edit_generate_sequence(len, str, len - 1, str2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*items));
    ck_assert_uint_eq(items[0].pos, pos);
    ck_assert_uint_eq(items[0].ch, TN_INVALID_CHAR);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

TEST(test_edit_apply_empty, OK, ONCE)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(str2), 0, 0);

    str = generate_random_string(&len);
    tn_edit_apply_sequence(len, str, 0, NULL, &dest);
    ck_assert_uint_eq(dest.len, len * sizeof(*str2));
    ck_assert(memcmp(str2, str, len * sizeof(*str2)) == 0);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));

TEST(test_edit_apply_any)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    uint32_t *str3 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(str3), 0, 0);

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    tn_edit_apply_sequence(len1, str1, dest.len / sizeof(*items),
                           items, &dest2);
    ck_assert_uint_eq(dest2.len, len2 * sizeof(*str3));
    ck_assert(memcmp(str3, str2, len2 * sizeof(*str3)) == 0);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(items));

TEST(test_edit_apply_single_subst)
    unsigned len;
    uint32_t *str;
    uint32_t *str2 = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(str2), 0, 0);
    unsigned pos;
    ucs4_t ch;

    str = generate_random_string(&len);
    pos = tn_random_int(0, len - 1);
    ch = tn_random_int(0, INT32_MAX);
    tn_edit_apply_sequence(len, str, 1, &(tn_edit_item){pos, ch}, &dest);
    str[pos] = ch;
    ck_assert_uint_eq(dest.len, len * sizeof(*str2));
    ck_assert(memcmp(str2, str, len * sizeof(*str2)) == 0);
    tn_free(TN_GLOC(str));
    tn_free(TN_GLOC(str2));

TEST(test_edit_apply_prefix)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    unsigned newlen;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    uint32_t *str3 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(str3), 0, 0);

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    newlen = tn_random_int(0, len1 - 1);
    tn_edit_apply_sequence(newlen, str1, dest.len / sizeof(*items),
                           items, &dest2);
    ck_assert_uint_lt(dest2.len, len2 * sizeof(*str3));
    ck_assert(memcmp(str3, str2, dest2.len) == 0);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(items));

TEST(test_edit_random_sanity, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;

    items = generate_random_edit_seq(&n);
    ck_assert(tn_edit_seq_valid(n, items));
    ck_assert(tn_edit_seq_eq(n, items, n, items));
    tn_free(TN_GLOC(items));

TEST(test_edit_item_shift_0)
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, 0);

    ck_assert(tn_edit_same_position(&item, &sh));
    ck_assert_uint_eq(item.ch, sh.ch);

TEST(test_edit_item_shift)
    size_t pos = tn_random_int(0, INT32_MAX);
    bool is_insert = tn_random_int(0, 1);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    int delta = tn_random_int(-pos, INT32_MAX - pos);
    tn_edit_item item = tn_edit_make_item(pos, is_insert, ch);
    tn_edit_item sh = tn_edit_shift_item(&item, delta);

    ck_assert_uint_eq(tn_edit_position(&item) + delta,
                      tn_edit_position(&sh));
    ck_assert_uint_eq(item.ch, sh.ch);


TEST(test_edit_compose_seq_empty_right, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    tn_edit_compose_sequence(n, items, 0, NULL, &dest);
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_seq_empty_left, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    tn_edit_compose_sequence(0, NULL, n, items, &dest);
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_subst)
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*result));
    ck_assert_uint_eq(result->pos, pos);
    ck_assert_uint_eq(result->ch, ch2);
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_insert_subst)
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*result));
    ck_assert_uint_eq(result->pos, pos | TN_EDIT_INSERT);
    ck_assert_uint_eq(result->ch, ch2);
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_insert2)
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos, true, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, 2 * sizeof(*result));
    ck_assert_uint_eq(result[0].pos, pos | TN_EDIT_INSERT);
    ck_assert_uint_eq(result[0].ch, ch2);
    ck_assert_uint_eq(result[1].pos, pos | TN_EDIT_INSERT);
    ck_assert_uint_eq(result[1].ch, ch1);
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_nullify)
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, true, ch);
    tn_edit_item item2 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(result, NULL);

TEST(test_edit_compose_delete_insert)
    size_t pos = tn_random_int(0, INT32_MAX);
    ucs4_t ch = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item item2 = tn_edit_make_item(pos, true, ch);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, sizeof(*result));
    ck_assert_uint_eq(result->pos, pos);
    ck_assert_uint_eq(result->ch, ch);
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_delete_delete)
    size_t pos = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item item2 = tn_edit_make_item(pos, false, TN_INVALID_CHAR);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, 2 * sizeof(*result));
    ck_assert_uint_eq(result[0].pos, pos);
    ck_assert_uint_eq(result[0].ch, TN_INVALID_CHAR);
    ck_assert_uint_eq(result[1].pos, pos + 1);
    ck_assert_uint_eq(result[1].ch, TN_INVALID_CHAR);
    tn_free(TN_GLOC(result));

TEST(test_edit_compose_disjoint)
    size_t pos1 = tn_random_int(0, INT32_MAX - 1);
    size_t pos2 = tn_random_int(pos1 + 1, INT32_MAX);
    ucs4_t ch1 = tn_random_int(0, INT32_MAX);
    ucs4_t ch2 = tn_random_int(0, INT32_MAX);
    tn_edit_item item1 = tn_edit_make_item(pos1, false, ch1);
    tn_edit_item item2 = tn_edit_make_item(pos2, false, ch2);
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    tn_edit_compose_sequence(1, &item1, 1, &item2, &dest);
    ck_assert_uint_eq(dest.len, 2 * sizeof(*result));
    ck_assert_uint_eq(result[0].pos, pos1);
    ck_assert_uint_eq(result[0].ch, ch1);
    ck_assert_uint_eq(result[1].pos, pos2);
    ck_assert_uint_eq(result[1].ch, ch2);
    tn_free(TN_GLOC(result));


TEST(test_edit_compose_valid)
    unsigned n1;
    tn_edit_item *items1 = NULL;
    unsigned n2;
    tn_edit_item *items2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items1 = generate_random_edit_seq(&n1);
    items2 = generate_random_edit_seq(&n2);
    tn_edit_compose_sequence(n1, items1, n2, items2, &dest);
    ck_assert(tn_edit_seq_valid(dest.len / sizeof(*result),
                                result));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items2));
    tn_free(TN_GLOC(result));

TEST(test_edit_compose)
    unsigned n1;
    unsigned n2;
    unsigned n3;
    uint32_t *str1 = NULL;
    uint32_t *str2 = NULL;
    uint32_t *str3 = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff23 = NULL;
    tn_buffer dest23 = TN_BUFFER_INIT(TN_GLOC(diff23), 0, 0);
    tn_edit_item *compose = NULL;
    tn_buffer dest_compose = TN_BUFFER_INIT(TN_GLOC(compose), 0, 0);
    uint32_t *cstr = NULL;
    tn_buffer dest_cstr = TN_BUFFER_INIT(TN_GLOC(cstr), 0, 0);

    str1 = generate_random_string(&n1);
    str2 = generate_random_string(&n2);
    str3 = generate_random_string(&n3);
    tn_edit_generate_sequence(n1, str1, n2, str2, &dest12);
    tn_edit_generate_sequence(n2, str2, n3, str3, &dest23);
    tn_edit_compose_sequence(dest12.len / sizeof(*diff12), diff12,
                             dest23.len / sizeof(*diff23), diff23,
                             &dest_compose);
    tn_edit_apply_sequence(n1, str1, dest_compose.len / sizeof(*compose),
                           compose, &dest_cstr);
    ck_assert_uint_eq(dest_cstr.len / sizeof(*cstr), n3);
    ck_assert(memcmp(str3, cstr, n3) == 0);

    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(str3));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff23));
    tn_free(TN_GLOC(compose));
    tn_free(TN_GLOC(cstr));

TEST(test_edit_compose_reverse)
    unsigned n1;
    unsigned n2;
    uint32_t *str1 = NULL;
    uint32_t *str2 = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff21 = NULL;
    tn_buffer dest21 = TN_BUFFER_INIT(TN_GLOC(diff21), 0, 0);
    tn_edit_item *compose = NULL;
    tn_buffer dest_compose = TN_BUFFER_INIT(TN_GLOC(compose), 0, 0);
    tn_edit_item *scompose = NULL;
    tn_buffer dest_scompose = TN_BUFFER_INIT(TN_GLOC(scompose), 0, 0);

    str1 = generate_random_string(&n1);
    str2 = generate_random_string(&n2);
    tn_edit_generate_sequence(n1, str1, n2, str2, &dest12);
    tn_edit_generate_sequence(n2, str2, n1, str1, &dest21);
    tn_edit_compose_sequence(dest12.len / sizeof(*diff12), diff12,
                             dest21.len / sizeof(*diff21), diff21,
                             &dest_compose);
    tn_edit_squeeze_sequence(dest_compose.len / sizeof(*compose),
                             compose, n1, str1, &dest_scompose);
    ck_assert_uint_eq(dest_scompose.len, 0);
    ck_assert_ptr_eq(scompose, NULL);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff21));
    tn_free(TN_GLOC(compose));

TEST(test_edit_squeeze)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    tn_edit_item *items1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(items1), 0, 0);

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    tn_edit_squeeze_sequence(dest.len / sizeof(*items), items,
                             len1, str1, &dest1);
    ck_assert(tn_edit_seq_eq(dest.len / sizeof(*items), items,
                             dest1.len / sizeof(*items1), items1));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_squeeze_prefix)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    tn_edit_item *items = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(items), 0, 0);
    tn_edit_item *items1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(items1), 0, 0);
    unsigned pfx;

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    pfx = tn_random_int(0, len1 - 1);
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    tn_edit_squeeze_sequence(dest.len / sizeof(*items), items,
                             pfx, str1, &dest1);
    ck_assert_uint_le(dest1.len, dest.len);
    ck_assert(tn_edit_seq_eq(dest1.len / sizeof(*items), items,
                             dest1.len / sizeof(*items), items1));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_lcs_identity, OK, ONCE)
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(len, str, len, str, &dest);
    ck_assert_uint_eq(dest.len / sizeof(*nstr), len);
    ck_assert(memcmp(nstr, str, len * sizeof(*str)) == 0);
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(str));

TEST(test_edit_lcs_empty_right, OK, ONCE)
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(len, str, 0, NULL, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(nstr, NULL);
    tn_free(TN_GLOC(str));

TEST(test_edit_lcs_empty_left, OK, ONCE)
    unsigned len;
    uint32_t *str;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);

    str = generate_random_string(&len);
    tn_edit_generate_lcs(0, NULL, len, str, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(nstr, NULL);
    tn_free(TN_GLOC(str));


TEST(test_edit_lcs_symm)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    uint32_t *nstr1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(nstr1), 0, 0);

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_lcs(len1, str1, len2, str2, &dest);
    tn_edit_generate_lcs(len2, str2, len1, str1, &dest1);
    ck_assert_uint_eq(dest.len, dest1.len);
    ck_assert(memcmp(nstr, nstr1, dest.len) == 0);
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(nstr1));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_lcs_disjoint)
    unsigned len1;
    uint32_t *str1 = NULL;
    unsigned len2;
    uint32_t *str2 = NULL;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    unsigned i;

    len1 = tn_random_int(1, 128);
    len2 = tn_random_int(1, 128);
    tn_alloc_raw(TN_GLOC(str1), len1 * sizeof(*str1));
    tn_alloc_raw(TN_GLOC(str2), len2 * sizeof(*str2));
    for (i = 0; i < len1; i++)
        str1[i] = i;
    for (i = 0; i < len2; i++)
        str2[i] = len1 + i;
    tn_edit_generate_lcs(len1, str1, len2, str2, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(nstr, NULL);
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_lcs_dist)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    size_t dist1;
    size_t dist2;

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_lcs(len1, str1, len2, str2, &dest);
    #ifndef TN_DEBUG_DISABLED
    ck_assert_uint_le(tn_edit_seq_memory_utilization, (len1 + 1) * (len2 + 1));
    #endif
    dist1 = tn_edit_distance(len1, str1, dest.len / sizeof(*nstr), nstr);
    dist2 = tn_edit_distance(len2, str2, dest.len / sizeof(*nstr), nstr);
    ck_assert_uint_eq(dist1, len1 - dest.len / sizeof(*nstr));
    ck_assert_uint_eq(dist2, len2 - dest.len / sizeof(*nstr));
    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_lcs_common)
    unsigned len1;
    uint32_t *str1;
    unsigned len2;
    uint32_t *str2;
    uint32_t *nstr = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(nstr), 0, 0);
    uint32_t *iter1;
    uint32_t *iter2;
    unsigned i;

    str1 = generate_random_string(&len1);
    str2 = generate_random_string(&len2);
    tn_edit_generate_lcs(len1, str1, len2, str2, &dest);
    iter1 = str1;
    iter2 = str2;
    for (i = 0; i < dest.len / sizeof(*nstr); i++)
    {
        uint32_t *next1 = u32_chr(iter1, len1 - (iter1 - str1), nstr[i]);
        uint32_t *next2 = u32_chr(iter2, len2 - (iter2 - str2), nstr[i]);

        ck_assert_ptr_ne(next1, NULL);
        ck_assert_ptr_ne(next2, NULL);
        iter1 = next1 + 1;
        iter2 = next2 + 1;
    }

    tn_free(TN_GLOC(nstr));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));

TEST(test_edit_merge_seq_self, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    ck_assert(tn_edit_merge_sequence(n, items, n, items, &dest));
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));


TEST(test_edit_merge_seq_empty_right, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    ck_assert(tn_edit_merge_sequence(n, items, 0, NULL, &dest));
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));

TEST(test_edit_merge_seq_empty_left, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    ck_assert(tn_edit_merge_sequence(0, NULL, n, items, &dest));
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));

TEST(test_edit_merge_seq_comm)
    unsigned n1;
    unsigned n2;
    tn_edit_item *items1 = NULL;
    tn_edit_item *items2 = NULL;
    tn_edit_item *result1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(result1), 0, 0);
    tn_edit_item *result2 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(result2), 0, 0);
    bool ok1, ok2;

    items1 = generate_random_edit_seq(&n1);
    items2 = generate_random_edit_seq(&n2);
    ok1 = tn_edit_merge_sequence(n1, items1, n2, items2, &dest1);
    ok2 = tn_edit_merge_sequence(n2, items2, n1, items1, &dest2);
    if (!ok1)
       ck_assert(!ok2);
    else
    {
        ck_assert(ok2);
        ck_assert(tn_edit_seq_eq(dest1.len / sizeof(*result1), result1,
                                 dest2.len / sizeof(*result2), result2));
    }
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items2));
    tn_free(TN_GLOC(result1));
    tn_free(TN_GLOC(result2));


TEST(test_edit_merge_seq_split)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *items1 = NULL;
    tn_edit_item *items2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(items1), 0, 0);
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(items2), 0, 0);
    tn_buffer *current = &dest1;
    unsigned i;

    items = generate_random_edit_seq(&n);
    for (i = 0; i < n; i++)
    {
        if (!tn_edit_is_insert(&items[i]))
        {
            current = tn_random_int(0, 1) ? &dest1 : &dest2;
        }
        *TN_BUFFER_PUSH(current, tn_edit_item, 1) = items[i];
    }

    ck_assert(tn_edit_merge_sequence(dest1.len / sizeof(*items1), items1,
                                     dest2.len / sizeof(*items2), items2,
                                     &dest));
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result), result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items2));
    tn_free(TN_GLOC(result));

TEST(test_edit_merge_seq_reverse_conflict)
    unsigned n1;
    unsigned n2;
    uint32_t *str1 = NULL;
    uint32_t *str2 = NULL;
    tn_edit_item *diff12 = NULL;
    tn_buffer dest12 = TN_BUFFER_INIT(TN_GLOC(diff12), 0, 0);
    tn_edit_item *diff21 = NULL;
    tn_buffer dest21 = TN_BUFFER_INIT(TN_GLOC(diff21), 0, 0);
    tn_edit_item *merge = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(merge), 0, 0);

    str1 = generate_random_string(&n1);
    str2 = generate_random_string(&n2);
    if (TN_UNLIKELY(u32_cmp2(str1, n1, str2, n2) == 0))
       return;
    tn_edit_generate_sequence(n1, str1, n2, str2, &dest12);
    tn_edit_generate_sequence(n2, str2, n1, str1, &dest21);
    ck_assert(!tn_edit_merge_sequence(dest12.len / sizeof(*diff12), diff12,
                                      dest21.len / sizeof(*diff21), diff21,
                                      &dest));
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(diff12));
    tn_free(TN_GLOC(diff21));
    tn_free(TN_GLOC(merge));


TEST(test_edit_intersect_seq_self, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(n, items, n, items, &dest);
    ck_assert(tn_edit_seq_eq(n, items,
                             dest.len / sizeof(*result),
                             result));
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(result));


TEST(test_edit_intersect_seq_empty_right, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(n, items, 0, NULL, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(result, NULL);
    tn_free(TN_GLOC(items));

TEST(test_edit_intersect_seq_empty_left, OK, ONCE)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);

    items = generate_random_edit_seq(&n);
    tn_edit_intersect_sequence(0, NULL, n, items, &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(result, NULL);
    tn_free(TN_GLOC(items));

TEST(test_edit_intersect_seq_comm)
    unsigned n1;
    unsigned n2;
    tn_edit_item *items1 = NULL;
    tn_edit_item *items2 = NULL;
    tn_edit_item *result1 = NULL;
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(result1), 0, 0);
    tn_edit_item *result2 = NULL;
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(result2), 0, 0);

    items1 = generate_random_edit_seq(&n1);
    items2 = generate_random_edit_seq(&n2);
    tn_edit_intersect_sequence(n1, items1, n2, items2, &dest1);
    tn_edit_intersect_sequence(n2, items2, n1, items1, &dest2);
    ck_assert(tn_edit_seq_eq(dest1.len / sizeof(*result1), result1,
                             dest2.len / sizeof(*result2), result2));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items2));
    tn_free(TN_GLOC(result1));
    tn_free(TN_GLOC(result2));


TEST(test_edit_intersect_seq_split)
    unsigned n;
    tn_edit_item *items = NULL;
    tn_edit_item *items1 = NULL;
    tn_edit_item *items2 = NULL;
    tn_edit_item *result = NULL;
    tn_buffer dest = TN_BUFFER_INIT(TN_GLOC(result), 0, 0);
    tn_buffer dest1 = TN_BUFFER_INIT(TN_GLOC(items1), 0, 0);
    tn_buffer dest2 = TN_BUFFER_INIT(TN_GLOC(items2), 0, 0);
    unsigned i;

    items = generate_random_edit_seq(&n);
    for (i = 0; i < n; i++)
    {
        tn_buffer *current = tn_random_int(0, 1) ? &dest1 : &dest2;
        *TN_BUFFER_PUSH(current, tn_edit_item, 1) = items[i];
    }

    tn_edit_intersect_sequence(dest1.len / sizeof(*items1), items1,
                               dest2.len / sizeof(*items2), items2,
                               &dest);
    ck_assert_uint_eq(dest.len, 0);
    ck_assert_ptr_eq(result, NULL);
    tn_free(TN_GLOC(items));
    tn_free(TN_GLOC(items1));
    tn_free(TN_GLOC(items2));
