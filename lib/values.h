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
    TNV_VECTOR,
    TNV_RANGE,
    TNV_REF,
    TNV_WEAKREF,
    TNV_SLICE,
    TNV_ARRAY,
    TNV_TABLE,
    TNV_CHANNEL,
    TNV_NODE,
    TNV_TERM,
    TNV_TERMDEF,
    TNV_CODE,
    TNV_LOCAL,
};

/**
 * @undocumented
 */
typedef struct tn_ref {
    uint64_t hi; /**< @undocumented */
    uint64_t lo; /**< @undocumented */
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
    int32_t lo; /**< @undocumented */
    int32_t hi; /**< @undocumented */
} tn_range;

/**
 * Impossible Unicode character.
 */
#define TN_INVALID_CHAR UINT32_MAX

/**
 * @undocumented
 */
typedef struct tn_charset_range {
    ucs4_t lo; /**< @undocumented */
    ucs4_t hi; /**< @undocumented */
} tn_charset_range;

/**
 * Edit sequence element
 */
typedef struct tn_edit_item {
    size_t pos; /**< @undocumented */
    ucs4_t ch;  /**< @undocumented */
} tn_edit_item;

/**
 * @undocumented
 */
#define TN_EDIT_INSERT ((uint64_t)1 << ((sizeof(size_t) * CHAR_BIT) - 1))

struct tn_value;

/**
 * Methods for a local type
 */
typedef struct tn_local_value_type {
    void (*destroy)(struct tn_value *v); /**< Destroy method */
} tn_local_value_type;

/**
 * Value
 */
typedef struct tn_value {
    enum tn_value_type type; /**< Value type */
    union {
        int64_t i; /**< Integral value */
        double d;  /**< Real value */
        ucs4_t ch; /**< Character value */
        uint32_t str[1]; /**< String value */
        uint8_t bytes[1]; /**< Bytes */
        tn_charset_range cs[1]; /**< Charset */
        tn_edit_item edit[1]; /**< Edit sequence */
        double vec[1]; /**< Vector */
        tn_range range; /**< Range */
        tn_ref ref; /**< Reference or weak reference */
        struct {
            tn_ref ref; /**< Slice base */
            tn_range range; /**< Slice extent */
        } slice; /**< Slice */
        struct {
            tn_ref items; /**< Items base */
            size_t count; /**< Number of items */
        } array; /**< Array */
        struct {
            size_t nkeys; /**< Number of keys */
        } table;
        struct {
            tn_ref items; /**< Items base */
            size_t capacity; /**< Channel capacity */
            size_t readptr; /**< Read pointer */
            size_t wrptr; /**< Write pointer */
            tn_ref read_notify; /**< Refill procedure */
            tn_ref write_notify; /**< Consume prodedure */
        } channel; /**< Channel */
        struct {
            tn_ref head; /**< Head value (e.g. name) */
            size_t nattrs; /**< Number of attributes */
            tn_ref children; /**< Children base */
            size_t nchildren; /**< Number of children */
        } node; /**< Node */
        struct {
            tn_ref type; /**< Term definition */
            tn_ref items; /**< Items base */
            bool nf; /**< Is in normal form? */
        } term; /**< Term */
        struct {
            uint64_t envid; /**< Runtime ID */
            tn_local_value_type *type; /**< Type descriptor */
            uint8_t opaque[1]; /**< Opaque data */
        } local; /**< Local type */
    } v; /**< Variant value */
} tn_value;

/**
 * Actual size of a value with fixed size
 */
#define TN_VALUE_FIXED_SIZE(_v, _f)                 \
    (offsetof(tn_value, v._f) + sizeof((_v)->_f))

/**
 * Actual size of a value with variable size
 */
#define TN_VALUE_VAR_SIZE(_v, _f, _n)                       \
    (offsetof(tn_value, v._f) + (_n) * sizeof((_v)->_f[0]))

/**
 * Number of items in a variable-size value
 */
#define TN_VALUE_COUNT(_len, _v, _f)                            \
    (((_len) - offsetof(tn_value, v._f)) / sizeof((_v)->_f[0]))

/**
 * Size for a value with a given type and given number of items
 */
#define TN_VALUE_SIZE_FOR_TYPE(_type, _v, _n)   \
    TN_VALUE_SIZE_FOR_##_type(_v, _n)

/** @cond PRIVATE */
#define TN_VALUE_SIZE_FOR_INTEGER(_v, _n) TN_VALUE_FIXED_SIZE(_v, i)
#define TN_VALUE_SIZE_FOR_FLOAT(_v, _n) TN_VALUE_FIXED_SIZE(_v, d)
#define TN_VALUE_SIZE_FOR_CHAR(_v, _n) TN_VALUE_FIXED_SIZE(_v, ch)
#define TN_VALUE_SIZE_FOR_STRING(_v, _n) TN_VALUE_VAR_SIZE(_v, str, _n)
#define TN_VALUE_SIZE_FOR_BSTRING(_v, _n) TN_VALUE_VAR_SIZE(_v, bytes, _n)
#define TN_VALUE_SIZE_FOR_CHARSET(_v, _n) TN_VALUE_VAR_SIZE(_v, cs, _n)
#define TN_VALUE_SIZE_FOR_EDITSTRING(_v, _n) TN_VALUE_VAR_SIZE(_v, edits, _n)
#define TN_VALUE_SIZE_FOR_VECTOR(_v, _n) TN_VALUE_VAR_SIZE(_v, vec, _n)
#define TN_VALUE_SIZE_FOR_RANGE(_v, _n) TN_VALUE_FIXED_SIZE(_v, range)
#define TN_VALUE_SIZE_FOR_REF(_v, _n) TN_VALUE_FIXED_SIZE(_v, ref)
#define TN_VALUE_SIZE_FOR_WEAKREF(_v, _n) TN_VALUE_FIXED_SIZE(_v, ref)
#define TN_VALUE_SIZE_FOR_SLICE(_v, _n) TN_VALUE_FIXED_SIZE(_v, slice)
#define TN_VALUE_SIZE_FOR_ARRAY(_v, _n) TN_VALUE_FIXED_SIZE(_v, array)
#define TN_VALUE_SIZE_FOR_TABLE(_v, _n) TN_VALUE_FIXED_SIZE(_v, table)
#define TN_VALUE_SIZE_FOR_CHANNEL(_v, _n) TN_VALUE_FIXED_SIZE(_v, channel)
#define TN_VALUE_SIZE_FOR_NODE(_v, _n) TN_VALUE_FIXED_SIZE(_v, node)
#define TN_VALUE_SIZE_FOR_TERM(_v, _n) TN_VALUE_FIXED_SIZE(_v, term)
//#define TN_VALUE_SIZE_FOR_TERMDEF,
//#define TN_VALUE_SIZE_FOR_CODE,
#define TN_VALUE_SIZE_FOR_LOCAL(_v, _n)         \
    TN_VALUE_VAR_SIZE(_v, local.opaque, _n)
/** @endcond */

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_VALUES_H */
