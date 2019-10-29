#include <sys/time.h>
#include "values.h"

static void
init_random(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);
}

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

#test test_edit_distance_self
    unsigned len;
    uint32_t *str;
    size_t dist;

    str = generate_random_string(&len);
    dist = tn_edit_distance(len, str, len, str);
    ck_assert_uint_eq(dist, 0);
    tn_free(TN_GLOC(str));

#test test_edit_distance_empty
    unsigned len;
    uint32_t *str;
    size_t dist;

    str = generate_random_string(&len);
    dist = tn_edit_distance(len, str, 0, NULL);
    ck_assert_uint_eq(dist, len);
    dist = tn_edit_distance(0, NULL, len, str);
    ck_assert_uint_eq(dist, len);
    tn_free(TN_GLOC(str));

#test-loop(0,100) test_edit_distance_symm
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

#test test_edit_distance_prefix
    unsigned len1;
    uint32_t *str;
    unsigned len2;
    size_t dist;

    str = generate_random_string(&len1);
    len2 = tn_random_int(0, len1 - 1);

    dist = tn_edit_distance(len1, str, len2, str);
    ck_assert_uint_eq(dist, len1 - len2);
    tn_free(TN_GLOC(str));

#test-loop(0,100) test_edit_subst_single
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

#test-loop(0,100) test_edit_ins_single
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

#test-loop(0,100) test_edit_del_single
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

#test test_edit_seq_invalid
    ck_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {0, 0}}));
    ck_assert(!tn_edit_seq_valid(2, (tn_edit_item[]){{1, 0}, {1, 0}}));

#test test_edit_seq_self
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

#test-loop(0,100) test_edit_seq_dist
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
    #ifndef TN_DEBUG_DISABLED
    tn_edit_seq_memory_utilization = 0;
    #endif
    tn_edit_generate_sequence(len1, str1, len2, str2, &dest);
    ck_assert_uint_eq(dest.len, dist * sizeof(*items));
    ck_assert(tn_edit_seq_valid(dist, items));
    #ifndef TN_DEBUG_DISABLED
    ck_assert_uint_le(tn_edit_seq_memory_utilization, (len1 + 1) * (len2 + 1));
    #endif
    tn_free(TN_GLOC(str1));
    tn_free(TN_GLOC(str2));
    tn_free(TN_GLOC(items));

#test test_edit_seq_empty_left
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

#test test_edit_seq_empty_right
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

#test test_edit_seq_pure_subst
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

#test-loop(0,100) test_edit_seq_subst_single
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

#test-loop(0,100) test_edit_seq_ins_single
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

#test-loop(0,100) test_edit_seq_ins_subst
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


#test-loop(0,100) test_edit_seq_del_single
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

#test test_edit_apply_empty
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

#test-loop(0,100) test_edit_apply_any
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

#test-loop(0,100) test_edit_apply_single_subst
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

#test-loop(0,100) test_edit_apply_prefix
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

#main-pre
    tcase_add_checked_fixture(tc1_1, init_random, NULL);