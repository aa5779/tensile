/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

#include "values.h"

size_t
tn_edit_distance(size_t len1, const uint32_t str1[TN_VAR_SIZE(len1)],
                 size_t len2, const uint32_t str2[TN_VAR_SIZE(len2)])
{
    size_t *row = NULL;
    size_t prev;
    size_t i, j;

    if (len2 < len1)
    {
        size_t tmp = len1;
        const uint32_t *tmpptr = str1;

        len1 = len2;
        str1 = str2;
        len2 = tmp;
        str2 = tmpptr;
    }

    tn_alloc_raw(TN_GLOC(row), sizeof(*row) * (len1 + 1));

    for (i = 0; i <= len1; i++)
        row[i] = i;

    for (i = 0; i < len2; i++)
    {
        prev = row[0];
        row[0]++;
        for (j = 0; j < len1; j++)
        {
            size_t min = row[j] < row[j + 1] ? row[j] : row[j + 1];

            min++;
            if (str2[i] != str1[j])
                prev++;
            min = min < prev ? min : prev;
            prev = row[j + 1];
            row[j + 1] = min;
        }
    }
    prev = row[len1];
    tn_free(TN_GLOC(row));

    return prev;
}
