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

#main-pre
    tcase_add_checked_fixture(tc1_1, init_random, NULL);