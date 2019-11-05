/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * Values.
 */
#ifndef TNH_VALUES_H
#define TNH_VALUES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unitypes.h>
#include "utils.h"

/**
 * @undocumented
 */
enum tn_value_type {
    TNV_NULL,
    TNV_INTEGER,
    TNV_FLOAT,
    TNV_CHAR,
    TNV_STRING,
    TNV_BSTRING,
    TNV_CHARSET,
    TNV_EDITSTRING,
    TNV_FMTSTRING,
    TNV_VECTOR,
    TNV_RANGE,
    TNV_REF,
    TNV_WEAKREF,
    TNV_SLICE,
    TNV_ARRAY,
    TNV_TABLE,
    TNV_NODE,
    TNV_TERM,
    TNV_TERMDEF,
    TNV_CODE,
    TNV_ERROR,
    TNV_ERRORDEF,
    TNV_LOCAL,
};

/**
 * @undocumented
 */
typedef struct tn_ref {
    uint64_t hi;
    uint64_t lo;
} tn_ref;

/**
 * @undocumented
 */
typedef uint32_t tn_rel_ref;

/**
 * @undocumented
 */
enum tn_ref_kind {
    TNRK_STATIC,
    TNRK_CONST,
    TNRK_VOLATILE,
    TNRK_EXTERNAL,
    TNRK_SCRATCH,
    TNRK_OP_LOCAL,
    TNRK_OP_INDIRECT,
};

/**
 * @undocumented
 */
typedef struct tn_range {
    int32_t lo;
    int32_t hi;
} tn_range;

/**
 * Impossible Unicode character.
 */
#define TN_INVALID_CHAR UINT32_MAX

/**
 * @undocumented
 */
typedef struct tn_charset_range {
    ucs4_t lo;
    ucs4_t hi;
} tn_charset_range;

/** @defgroup edits Edits
 * @{
 */

/**
 * Edit sequence element
 */
typedef struct tn_edit_item {
    size_t pos;
    ucs4_t ch;
} tn_edit_item;

/**
 * @undocumented
 */
#define TN_EDIT_INSERT (1ull << ((sizeof(size_t) * CHAR_BIT) - 1))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_VALUES_H */
