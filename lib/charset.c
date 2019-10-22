/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

#include <search.h>
#include "values.h"

bool
tn_charset_valid(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    ucs4_t limit;
    size_t i;

    if (len == 0)
        return true;

    if (set[0].lo > set[0].hi)
        return false;
    limit = set[0].hi + 1;

    for (i = 1; i < len; i++)
    {
        if (set[i].lo <= limit)
            return false;
        if (set[i].lo > set[i].hi)
            return false;
        limit = set[i].hi + 1;
    }
    return true;
}

static int
charset_compare(const void *key, const void *item)
{
    ucs4_t ch = *(const ucs4_t *)key;
    const tn_charset_range *r = item;

    if (r->lo <= ch && r->hi >= ch)
        return 0;
    return ch < r->lo ? -1 : 1;
}

bool
tn_charset_contains(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)],
                    ucs4_t ch)
{
    return bsearch(&ch, set, len, sizeof(set[0]), charset_compare);
}

unsigned
tn_charset_cardinality(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)])
{
    unsigned count = 0;

    for (; len > 0; len--, set++)
        count += set->hi - set->lo + 1;

    return count;
}

ucs4_t
tn_charset_nth(size_t len, const tn_charset_range set[TN_VAR_SIZE(len)],
               unsigned i)
{
    for (; len > 0; len--, set++)
    {
        if (i <= set->hi - set->lo)
            return set->lo + i;
        i -= set->hi - set->lo + 1;
    }
    return UNINAME_INVALID;
}

bool
tn_charset_subset(size_t sublen,
                  const tn_charset_range subset[TN_VAR_SIZE(sublen)],
                  size_t superlen,
                  const tn_charset_range superset[TN_VAR_SIZE(sublen)])
{
    while (sublen > 0)
    {
        if (superlen == 0)
            return false;
        if (subset->lo > superset->hi)
        {
            superset++;
            superlen--;
        }
        else if (subset->lo < superset->lo ||
                 subset->hi > superset->hi)
            return false;
        else
        {
            subset++;
            sublen--;
        }
    }
    return true;
}

void
tn_charset_generate_union(size_t len1,
                          const tn_charset_range set1[TN_VAR_SIZE(len1)],
                          size_t len2,
                          const tn_charset_range set2[TN_VAR_SIZE(len2)],
                          tn_buffer *dest)
{
    bool has_current = false;
    tn_charset_range current = {0, 0};

    while (len1 > 0 || len2 > 0)
    {
        const tn_charset_range *next;

        if (len1 == 0 || (len2 != 0 && set2->lo < set1->lo))
        {
            next = set2;
            set2++;
            len2--;
        }
        else
        {
            next = set1;
            set1++;
            len1--;
        }

        if (has_current && next->lo <= current.hi + 1)
        {
            current.hi = current.hi > next->hi ? current.hi : next->hi;
        }
        else
        {
            if (has_current)
                *TN_BUFFER_PUSH(dest, tn_charset_range, 1) = current;
            current = *next;
            has_current = true;
        }
    }
    if (has_current)
        *TN_BUFFER_PUSH(dest, tn_charset_range, 1) = current;
}

void
tn_charset_generate_intersect(size_t len1,
                              const tn_charset_range set1[TN_VAR_SIZE(len1)],
                              size_t len2,
                              const tn_charset_range set2[TN_VAR_SIZE(len2)],
                              tn_buffer *dest)
{
    while (len1 > 0 && len2 > 0)
    {
        if (set2->lo > set1->lo)
        {
            const tn_charset_range *tmpset;
            size_t tmplen;

            tmpset = set2;
            set2 = set1;
            set1 = tmpset;
            tmplen = len2;
            len2 = len1;
            len1 = tmplen;
        }
        while (len2 > 0 && set2->hi <= set1->hi)
        {
            if (set2->hi >= set1->lo)
            {
                *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
                    (tn_charset_range){.lo =
                                       set2->lo > set1->lo ? set2->lo :
                                       set1->lo,
                                       .hi = set2->hi};
            }
            set2++;
            len2--;
        }
        if (len2 > 0 && set2->lo < set1->hi)
        {
            *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
                (tn_charset_range){.lo =
                                   set2->lo > set1->lo ? set2->lo : set1->lo,
                                   .hi =
                                   set2->hi > set1->hi ? set1->hi : set2->lo};
        }
        set1++;
        len1--;
    }
}

void
tn_charset_generate_complement(size_t len,
                               const tn_charset_range set[TN_VAR_SIZE(len)],
                               tn_buffer *dest)
{
    ucs4_t last;

    if (len == 0)
    {
        *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
            (tn_charset_range){0, INT32_MAX};
        return;
    }
    if (set[0].lo > 0)
    {
        *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
            (tn_charset_range){0, set[0].lo - 1};
    }
    last = set[0].hi;
    set++;
    len--;

    while (len > 0)
    {
        *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
            (tn_charset_range){last + 1, set[0].lo - 1};
        last = set[0].hi;
        len--;
        set++;
    }
    if (last != INT32_MAX)
    {
        *TN_BUFFER_PUSH(dest, tn_charset_range, 1) =
            (tn_charset_range){last + 1, INT32_MAX};
    }
}
