/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * String editing.
 */
#ifndef TNH_EDITS_H
#define TNH_EDITS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "values.h"

static inline TN_NO_SHARED_STATE
TN_NO_NULL_ARGS size_t
tn_edit_position(const tn_edit_item *item)
{
    return item->pos & ~TN_EDIT_INSERT;
}

static inline TN_NO_SHARED_STATE
TN_NO_NULL_ARGS bool
tn_edit_same_position(const tn_edit_item *item1,
                      const tn_edit_item *item2)
{
    return tn_edit_position(item1) == tn_edit_position(item2);
}

static inline TN_NO_SHARED_STATE
TN_NO_NULL_ARGS bool
tn_edit_is_insert(const tn_edit_item *item)
{
    return (item->pos & TN_EDIT_INSERT) != 0;
}

static inline TN_NO_SHARED_STATE
TN_NO_NULL_ARGS bool
tn_edit_is_delete(const tn_edit_item *item)
{
    return item->ch == TN_INVALID_CHAR;
}

static inline TN_NO_SHARED_STATE tn_edit_item
tn_edit_make_item(size_t pos, bool insert, ucs4_t ch)
{
    return (tn_edit_item){
             .pos = pos | (insert ? TN_EDIT_INSERT : 0),
             .ch = ch
            };
}

static inline TN_NO_SHARED_STATE
TN_NO_NULL_ARGS tn_edit_item
tn_edit_shift_item(const tn_edit_item *item, int delta)
{
    return tn_edit_make_item(tn_edit_position(item) + delta,
                             tn_edit_is_insert(item),
                             item->ch);
}

extern TN_NO_SHARED_STATE bool
tn_edit_seq_eq(size_t len1,
               const tn_edit_item seq1[TN_VAR_SIZE(len1)],
               size_t len2,
               const tn_edit_item seq2[TN_VAR_SIZE(len2)]);

/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern size_t tn_edit_distance(size_t len1,
                               const uint32_t str1[TN_VAR_SIZE(len1)],
                               size_t len2,
                               const uint32_t str2[TN_VAR_SIZE(len2)]);

#ifndef TN_DEBUG_DISABLED
/**
 * @undocumented
 */
extern size_t tn_edit_seq_memory_utilization;
#endif


/**
 * @undocumented
 */
TN_NO_SHARED_STATE
extern bool tn_edit_seq_valid(size_t len,
                              const tn_edit_item seq[TN_VAR_SIZE(len)]);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_generate_sequence(size_t len1,
                                      const uint32_t str1[TN_VAR_SIZE(len1)],
                                      size_t len2,
                                      const uint32_t str2[TN_VAR_SIZE(len2)],
                                      tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_generate_lcs(size_t len1,
                                 const uint32_t str1[TN_VAR_SIZE(len1)],
                                 size_t len2,
                                 const uint32_t str2[TN_VAR_SIZE(len2)],
                                 tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_apply_sequence(size_t len,
                                   const uint32_t str[TN_VAR_SIZE(len)],
                                   size_t editlen,
                                   const tn_edit_item \
                                   edit[TN_VAR_SIZE(editlen)],
                                   tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_compose_sequence(size_t editlen1,
                                     const tn_edit_item \
                                     edit1[TN_VAR_SIZE(editlen1)],
                                     size_t editlen2,
                                     const tn_edit_item \
                                     edit2[TN_VAR_SIZE(editlen2)],
                                     tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern bool tn_edit_merge_sequence(size_t editlen1,
                                   const tn_edit_item \
                                   edit1[TN_VAR_SIZE(editlen1)],
                                   size_t editlen2,
                                   const tn_edit_item \
                                   edit2[TN_VAR_SIZE(editlen2)],
                                   tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_intersect_sequence(size_t editlen1,
                                       const tn_edit_item           \
                                       edit1[TN_VAR_SIZE(editlen1)],
                                       size_t editlen2,
                                       const tn_edit_item           \
                                       edit2[TN_VAR_SIZE(editlen2)],
                                       tn_buffer *dest);

/**
 * @undocumented
 */
TN_NOT_NULL_ARGS(5)
extern void tn_edit_squeeze_sequence(size_t editlen,
                                     const tn_edit_item \
                                     edit[TN_VAR_SIZE(editlen)],
                                     size_t baselen,
                                     const ucs4_t base[TN_VAR_SIZE(baselen)],
                                     tn_buffer *dest);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_EDITS_H */
