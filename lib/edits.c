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

typedef struct tn_edit_cell {
    size_t refcnt;
    tn_edit_item item;
    struct tn_edit_cell *chain;
} tn_edit_cell;

bool
tn_edit_seq_valid(size_t len, const tn_edit_item seq[TN_VAR_SIZE(len)])
{
    size_t pos;
    size_t i;

    if (len == 0)
        return true;
    pos = seq[0].pos & ~TN_EDIT_INSERT;
    for (i = 1; i < len; i++)
    {
        size_t this = seq[i].pos & ~TN_EDIT_INSERT;
        if (this < pos ||
            (this == pos &&
             ((seq[i - 1].pos & TN_EDIT_INSERT) == 0)))
        {
            return false;
        }
        pos = this;
    }
    return true;
}

#ifndef TN_DEBUG_DISABLED
size_t tn_edit_seq_memory_utilization = 0;
#endif

static tn_edit_cell *
tn_new_cell(tn_edit_cell **freelist)
{
    tn_edit_cell *next = NULL;
    if (*freelist != NULL)
    {
        next = *freelist;
        *freelist = next->chain;
    }
    else
    {
        tn_alloc_raw(TN_RLOC(freelist, next), sizeof(*next));
    }
    next->refcnt = 0;
    next->chain = NULL;
#ifndef TN_DEBUG_DISABLED
    tn_edit_seq_memory_utilization++;
#endif

    return next;
}

static void tn_unlink_cell(tn_edit_cell **freelist, tn_edit_cell *cell);

static void
tn_cleanup_cell(tn_edit_cell **freelist, tn_edit_cell *cell)
{
    if (cell != NULL && cell->refcnt == 0)
    {
        if (cell->chain != NULL)
            tn_unlink_cell(freelist, cell->chain);
        cell->chain = *freelist;
        *freelist = cell;
#ifndef TN_DEBUG_DISABLED
        tn_edit_seq_memory_utilization--;
#endif

    }
}

static void
tn_link_cell(tn_edit_cell *parent, tn_edit_cell *child)
{
    TN_BUG_ON(parent->chain != NULL);
    if (child != NULL)
        child->refcnt++;
    parent->chain = child;
}

static void
tn_unlink_cell(tn_edit_cell **freelist, tn_edit_cell *cell)
{
    TN_BUG_ON(cell->refcnt == 0);
    if (--cell->refcnt == 0)
        tn_cleanup_cell(freelist, cell);
}

typedef struct tn_edit_cell_ptr {
    tn_edit_cell *cell;
    size_t dist;
} tn_edit_cell_ptr;

void
tn_edit_generate_sequence(size_t len1,
                          const uint32_t str1[TN_VAR_SIZE(len1)],
                          size_t len2,
                          const uint32_t str2[TN_VAR_SIZE(len2)],
                          tn_buffer *dest)
{
    tn_edit_cell **freelist = NULL;
    tn_edit_cell_ptr *prev_row = NULL;
    tn_edit_cell_ptr *this_row = NULL;
    tn_edit_cell *iter;

    size_t i, j;

    tn_alloc_raw(TN_GLOC(freelist), sizeof(*freelist));
    *freelist = NULL;
    tn_alloc_raw(TN_RLOC(freelist, prev_row),
                 (len2 + 1) * sizeof(*prev_row));
    tn_alloc_raw(TN_RLOC(freelist, this_row),
                 (len2 + 1) * sizeof(*this_row));

    prev_row[0].dist = 0;
    prev_row[0].cell = NULL;

    for (i = 0; i < len2; i++)
    {
        prev_row[i + 1].dist = i + 1;
        prev_row[i + 1].cell = tn_new_cell(freelist);
        prev_row[i + 1].cell->item.pos = TN_EDIT_INSERT;
        prev_row[i + 1].cell->item.ch = str2[i];
        tn_link_cell(prev_row[i + 1].cell, prev_row[i].cell);
    }

    for (i = 0; i < len1; i++)
    {
        this_row[0].dist = i + 1;
        this_row[0].cell = tn_new_cell(freelist);
        this_row[0].cell->item.pos = i;
        this_row[0].cell->item.ch  = TN_INVALID_CHAR;
        tn_link_cell(this_row[0].cell, prev_row[0].cell);

        for (j = 0; j < len2; j++)
        {
            size_t ins_cost = this_row[j].dist + 1;
            size_t del_cost = prev_row[j + 1].dist + 1;
            size_t subst_cost = prev_row[j].dist + (str1[i] != str2[j]);

            if (ins_cost < del_cost && ins_cost < subst_cost)
            {
                this_row[j + 1].dist = ins_cost;
                this_row[j + 1].cell = tn_new_cell(freelist);
                this_row[j + 1].cell->item.pos = TN_EDIT_INSERT | (i + 1);
                this_row[j + 1].cell->item.ch  = str2[j];
                tn_link_cell(this_row[j + 1].cell, this_row[j].cell);
            }
            else if (del_cost < ins_cost && del_cost < subst_cost)
            {
                this_row[j + 1].dist = del_cost;
                this_row[j + 1].cell = tn_new_cell(freelist);
                this_row[j + 1].cell->item.pos = i;
                this_row[j + 1].cell->item.ch = TN_INVALID_CHAR;
                tn_link_cell(this_row[j + 1].cell, prev_row[j + 1].cell);
            }
            else if (str1[i] != str2[j])
            {
                this_row[j + 1].dist = subst_cost;
                this_row[j + 1].cell = tn_new_cell(freelist);
                this_row[j + 1].cell->item.pos = i;
                this_row[j + 1].cell->item.ch = str2[j];
                tn_link_cell(this_row[j + 1].cell, prev_row[j].cell);
            }
            else
            {
                this_row[j + 1].dist = subst_cost;
                this_row[j + 1].cell = prev_row[j].cell;
                if (this_row[j + 1].cell != NULL)
                    this_row[j + 1].cell->refcnt++;
            }
        }

        for (j = 0; j < len2 + 1; j++)
        {
            tn_cleanup_cell(freelist, prev_row[j].cell);
            prev_row[j] = this_row[j];
            this_row[j].cell = NULL;
        }
    }

    if (prev_row[len2].dist != 0)
    {
        tn_edit_item *result = TN_BUFFER_PUSH(dest, tn_edit_item,
                                              prev_row[len2].dist);

        for (iter = prev_row[len2].cell, i = prev_row[len2].dist;
             iter != NULL; iter = iter->chain, i--)
        {
            TN_BUG_ON(i == 0);
            result[i - 1] = iter->item;
        }
        TN_BUG_ON(i != 0);
    }

    tn_free(TN_GLOC(freelist));
}


void
tn_edit_apply_sequence(size_t len,
                       const uint32_t str[TN_VAR_SIZE(len)],
                       size_t editlen,
                       const tn_edit_item edit[TN_VAR_SIZE(editlen)],
                       tn_buffer *dest)
{
    size_t last = 0;
    size_t i;

    for (i = 0; i < editlen; i++)
    {
        size_t realpos = edit[i].pos & ~TN_EDIT_INSERT;

        if (realpos > len)
            break;

        if (realpos > last)
        {
            uint32_t *chunk = TN_BUFFER_PUSH(dest, uint32_t, realpos - last);
            memcpy(chunk, &str[last], (realpos - last) * sizeof(*str));
        }
        if ((edit[i].pos & TN_EDIT_INSERT) != 0)
        {
            TN_BUG_ON(edit[i].ch == TN_INVALID_CHAR);
            *TN_BUFFER_PUSH(dest, uint32_t, 1) = edit[i].ch;
            last = realpos;
        }
        else if (edit[i].ch == TN_INVALID_CHAR)
        {
            last = realpos + 1;
        }
        else if (realpos != len)
        {
            *TN_BUFFER_PUSH(dest, uint32_t, 1) = edit[i].ch;
            last = realpos + 1;
        }
    }
    if (last < len)
    {
        uint32_t *chunk = TN_BUFFER_PUSH(dest, uint32_t, len - last);
        memcpy(chunk, &str[last], (len - last) * sizeof(*str));
    }
}
