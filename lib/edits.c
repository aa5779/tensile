/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

#include "edits.h"

bool
tn_edit_seq_eq(size_t len1,
               const tn_edit_item seq1[TN_VAR_SIZE(len1)],
               size_t len2,
               const tn_edit_item seq2[TN_VAR_SIZE(len2)])
{
    size_t i;

    if (len1 != len2)
        return false;

    for (i = 0; i < len1; i++)
    {
        if (seq1[i].pos != seq2[i].pos || seq1[i].ch != seq2[i].ch)
            return false;
    }
    return true;
}


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
    pos = tn_edit_position(&seq[0]);

    for (i = 1; i < len; i++)
    {
        size_t this = tn_edit_position(&seq[i]);
        if (this < pos ||
            (this == pos &&
             !tn_edit_is_insert(&seq[i - 1])))
        {
            return false;
        }
        pos = this;
        if (tn_edit_is_insert(&seq[i]) && tn_edit_is_delete(&seq[i]))
        {
            return false;
        }
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
tn_edit_generate_lcs(size_t len1,
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
        prev_row[i + 1].dist = 0;
        prev_row[i + 1].cell = NULL;
    }

    for (i = 0; i < len1; i++)
    {
        this_row[0].dist = 0;
        this_row[0].cell = NULL;

        for (j = 0; j < len2; j++)
        {
            if (str1[i] == str2[j])
            {
                this_row[j + 1].dist = prev_row[j].dist + 1;
                this_row[j + 1].cell = tn_new_cell(freelist);
                this_row[j + 1].cell->item.ch = str1[i];
                tn_link_cell(this_row[j + 1].cell, prev_row[j].cell);
            }
            else if (this_row[j].dist > prev_row[j + 1].dist)
            {
                this_row[j + 1].dist = this_row[j].dist;
                this_row[j + 1].cell = this_row[j].cell;
                if (this_row[j].cell != NULL)
                    this_row[j].cell->refcnt++;
            }
            else
            {
                this_row[j + 1].dist = prev_row[j + 1].dist;
                this_row[j + 1].cell = prev_row[j + 1].cell;
                if (this_row[j].cell != NULL)
                    this_row[j].cell->refcnt++;
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
        uint32_t *result = TN_BUFFER_PUSH(dest, uint32_t,
                                          prev_row[len2].dist);

        for (iter = prev_row[len2].cell, i = prev_row[len2].dist;
             iter != NULL; iter = iter->chain, i--)
        {
            TN_BUG_ON(i == 0);
            result[i - 1] = iter->item.ch;
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
        size_t realpos = tn_edit_position(&edit[i]);

        if (realpos > len)
            break;

        if (realpos > last)
        {
            uint32_t *chunk = TN_BUFFER_PUSH(dest, uint32_t, realpos - last);
            memcpy(chunk, &str[last], (realpos - last) * sizeof(*str));
        }
        if (tn_edit_is_insert(&edit[i]))
        {
            TN_BUG_ON(tn_edit_is_delete(&edit[i]));
            *TN_BUFFER_PUSH(dest, uint32_t, 1) = edit[i].ch;
            last = realpos;
        }
        else if (tn_edit_is_delete(&edit[i]))
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

static TN_NO_SHARED_STATE int
compare_edit_items(const tn_edit_item *item1,
                   const tn_edit_item *item2, int delta)
{
    if (tn_edit_position(item1) + delta < tn_edit_position(item2))
        return -1;
    else if (tn_edit_position(item1) + delta > tn_edit_position(item2))
        return 1;
    else if (tn_edit_is_insert(item2) && !tn_edit_is_delete(item1))
        return 1;
    else if (tn_edit_is_delete(item1) && !tn_edit_is_insert(item2))
        return -1;
    else
        return 0;
}

void
tn_edit_compose_sequence(size_t editlen1,
                         const tn_edit_item edit1[TN_VAR_SIZE(editlen1)],
                         size_t editlen2,
                         const tn_edit_item edit2[TN_VAR_SIZE(editlen2)],
                         tn_buffer *dest)
{
    int delta = 0;

    while (editlen1 > 0 && editlen2 > 0)
    {
        int compare = compare_edit_items(edit1, edit2, delta);

        if (compare < 0)
        {
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit1;
            delta += tn_edit_is_insert(edit1);
            delta -= tn_edit_is_delete(edit1);
            edit1++;
            editlen1--;
        }
        else if (compare > 0)
        {
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) =
                tn_edit_shift_item(edit2, -delta);
            edit2++;
            editlen2--;
        }
        else
        {
            if (!tn_edit_is_insert(edit1) || !tn_edit_is_delete(edit2))
            {
                *TN_BUFFER_PUSH(dest, tn_edit_item, 1) =
                    (tn_edit_item){.pos = edit1->pos, .ch = edit2->ch};
            }
            delta += tn_edit_is_insert(edit1);
            delta -= tn_edit_is_delete(edit1);
            edit1++;
            editlen1--;
            edit2++;
            editlen2--;
        }
    }

    if (editlen1 > 0)
    {
        memcpy(TN_BUFFER_PUSH(dest, tn_edit_item, editlen1), edit1,
               editlen1 * sizeof(*edit1));
    }
    else if (editlen2 > 0)
    {
        tn_edit_item *tail = TN_BUFFER_PUSH(dest, tn_edit_item, editlen2);

        while (editlen2 > 0)
        {
            *tail++ = tn_edit_shift_item(edit2, -delta);
            editlen2--;
            edit2++;
        }
    }
}

bool
tn_edit_merge_sequence(size_t editlen1,
                       const tn_edit_item edit1[TN_VAR_SIZE(editlen1)],
                       size_t editlen2,
                       const tn_edit_item edit2[TN_VAR_SIZE(editlen2)],
                       tn_buffer *dest)
{
    while (editlen1 > 0 && editlen2 > 0)
    {
        size_t pos1 = tn_edit_position(edit1);
        size_t pos2 = tn_edit_position(edit2);

        if (pos1 < pos2)
        {
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit1;
            edit1++;
            editlen1--;
        }
        else if (pos2 < pos1)
        {
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit2;
            edit2++;
            editlen2--;
        }
        else
        {
            if (edit1->pos != edit2->pos || edit1->ch != edit2->ch)
                return false;
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit1;
            edit1++;
            edit2++;
            editlen1--;
            editlen2--;
        }
    }
    if (editlen1 > 0)
    {
        memcpy(TN_BUFFER_PUSH(dest, tn_edit_item, editlen1),
               edit1, editlen1 * sizeof(*edit1));
    }
    else if (editlen2 > 0)
    {
        memcpy(TN_BUFFER_PUSH(dest, tn_edit_item, editlen2),
               edit2, editlen2 * sizeof(*edit2));
    }
    return true;
}

void
tn_edit_intersect_sequence(size_t editlen1,
                           const tn_edit_item edit1[TN_VAR_SIZE(editlen1)],
                           size_t editlen2,
                           const tn_edit_item edit2[TN_VAR_SIZE(editlen2)],
                           tn_buffer *dest)
{
    while (editlen1 > 0 && editlen2 > 0)
    {
        size_t pos1 = tn_edit_position(edit1);
        size_t pos2 = tn_edit_position(edit2);

        if (pos1 < pos2)
        {
            edit1++;
            editlen1--;
        }
        else if (pos2 < pos1)
        {
            edit2++;
            editlen2--;
        }
        else
        {
            if (edit1->pos == edit2->pos && edit1->ch == edit2->ch)
                *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit1;
            edit1++;
            edit2++;
            editlen1--;
            editlen2--;
        }
    }
}

void
tn_edit_squeeze_sequence(size_t editlen,
                         const tn_edit_item edit[TN_VAR_SIZE(editlen)],
                         size_t baselen,
                         const ucs4_t base[TN_VAR_SIZE(baselen)],
                         tn_buffer *dest)
{
    while (editlen > 0)
    {
        if (tn_edit_position(edit) > baselen ||
            (!tn_edit_is_insert(edit) && edit->pos == baselen))
            return;
        if (tn_edit_is_insert(edit) ||
            edit->ch != base[edit->pos])
        {
            *TN_BUFFER_PUSH(dest, tn_edit_item, 1) = *edit;
        }
        editlen--;
        edit++;
    }
}
