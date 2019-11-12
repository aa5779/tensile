/*
 * Copyright (c) 2017-2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

/** @file
 *  Utility functions.
 */
#ifndef TNH_UTILS_H
#define TNH_UTILS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "compiler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <talloc.h>

/**
 * Returns the number of elements in an array @p _arr.
 */
#define TN_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))

/**
 * Returns @p _idx'th element in @p _arr,
 * if it is less than the total number of elements;
 * otherwise return @p _defval.
 */
#define TN_ARRAY_GET(_arr, _idx, _defval)                       \
    ((_idx) < TN_ARRAY_SIZE(_arr) ? (_arr)[_idx] : (_defval))

/**
 * Returns the size of a flexible structure @p _type,
 * where its flexible member @p _flexfield contains
 * @p _count elements.
 */
#define TN_FLEX_SIZE(_type, _flexfield, _count)                         \
    (offsetof(_type, _flexfield) +                                      \
     (_count) * sizeof(((_type *)NULL)->_flexfield[0]))

/**
 * Returns its argument @p _x stringified.
 * If @p _x contains macros, they are expanded,
 * unlike the case of simple `#` operator.
 */
#define TN_STRINGIFY(_x) #_x

/**
 * Aborts the program if @p cond is false, displaying a message
 * <code><var>file</var>:<var>line</var>: <var>msg</var></code>.
 * @sa TN_BUG_ON
 */
static inline void
tn_bug_on(bool cond, const char *file, int line, const char *msg)
{
    if (TN_UNLIKELY(cond))
    { /* LCOV_EXCL_START */
        fprintf(stderr, "%s:%d: %s\n", file, line, msg);
        abort();
    } /* LCOV_EXCL_STOP */
}

/**
 * Aborts the program if @p _expr is false.
 * Prints the stringified form of @p _expr and
 * the current source code location.
 * @sa tn_bug_on
 * @test
 * @snippet libtn_utils_tests.c TN_BUG_ON
 */
#define TN_BUG_ON(_expr)                            \
    tn_bug_on(_expr, __FILE__, __LINE__, #_expr)

/**
 * Prints a formatted message related to @p _file:@p _line.
 * `tn_log_error` and `TN_INTERNAL_ERROR` produce
 * an error message, `tn_log_warning` produces a warning,
 * and `tn_fatal_error` produces a fatal error and aborts
 * the program. `TN_INTERNAL_ERROR` is a wrapper around
 * `tn_log_error` that uses the current source code location
 * as a relevant filename and line.
 */
#define tn_log_error(_file, _line, _fmt, ...)                           \
    ((void)fprintf(stderr, "error:%s:%d: " _fmt "\n", _file, _line,     \
                   __VA_ARGS__))

/**
 * @undocumented
 */
#define tn_log_warning(_file, _line, _fmt, ...)                         \
    ((void)fprintf(stderr, "warning:%s:%d: " _fmt "\n", _file, _line,   \
                   __VA_ARGS__))

/**
 * @undocumented
 */
#define tn_fatal_error(_file, _line, _fmt, ...)                         \
    ((void)fprintf(stderr, "FATAL:%s:%d: " _fmt "\n", _file, _line,     \
                   __VA_ARGS__), abort())

/**
 * @undocumented
 */
#define TN_INTERNAL_ERROR(_fmt, ...)                    \
    tn_log_error(__FILE__, __LINE__, _fmt, __VA_ARGS__)

/**
 * A data type to store pooled pointer information.
 */
typedef struct tn_ptr_location {
    const void *context;  /**< a memory pool owning the pointer */
    void **loc;           /**< an address of a variable holding the pointer */
} tn_ptr_location;

/**
 * Releases the memory pointed to by @p loc.
 * The memory is detached from the context and if there's
 * no other contexts referring to it, it is freed.
 */
static inline void
tn_free(tn_ptr_location loc)
{
    if (TN_LIKELY(*loc.loc != NULL))
    {
        TN_BUG_ON(talloc_unlink(loc.context, *loc.loc) != 0);
        *loc.loc = NULL;
    }
}


/**
 * @undocumented
 */
static inline void
tn_set_loc(tn_ptr_location loc, void *ptr)
{
    tn_free(loc);
    *loc.loc = ptr;
}

/**
 * @undocumented
 */
static inline void
tn_alloc_typed(tn_ptr_location loc, const char *type, size_t sz,
               int (*destructor)(void *))
{
    void *obj = talloc_zero_size(loc.context, sz);

    TN_BUG_ON(obj == NULL);

    if (type != NULL)
        talloc_set_name_const(obj, type);
    if (destructor != NULL)
        talloc_set_destructor(obj, destructor);
    tn_set_loc(loc, obj);
}

/**
 * @undocumented
 */
static inline void
tn_alloc_raw(tn_ptr_location loc, size_t sz)
{
    void *obj = talloc_size(loc.context, sz);

    TN_BUG_ON(obj == NULL);

    tn_set_loc(loc, obj);
}

/**
 * @undocumented
 */
static inline void
tn_strdup(tn_ptr_location loc, const char *str)
{
    if (TN_UNLIKELY(str == NULL))
        tn_free(loc);
    else
    {
        char *copy = talloc_strdup(loc.context, str);

        TN_BUG_ON(copy == NULL);

        tn_set_loc(loc, copy);
    }
}

/**
 * @undocumented
 */
static inline void TN_LIKE_VPRINTF(2)
tn_vsprintf(tn_ptr_location loc, const char *fmt, va_list args)
{
    char *buf = talloc_vasprintf(loc.context, fmt, args);

    TN_BUG_ON(buf == NULL);

    tn_set_loc(loc, buf);
}

/**
 * @undocumented
 */
extern void TN_LIKE_PRINTF(2, 3) tn_sprintf(tn_ptr_location loc,
                                            const char *fmt, ...);

/**
 * @undocumented
 */
static inline void
tn_realloc(tn_ptr_location loc, size_t newsz)
{
    void *obj = talloc_realloc_size(loc.context, *loc.loc, newsz);

    TN_BUG_ON(obj == NULL);
    *loc.loc = obj;
}

/**
 * @undocumented
 */
static inline void
tn_strcat(tn_ptr_location loc, const char *suffix)
{
    size_t newsz = strlen(*loc.loc) + strlen(suffix) + 1;

    tn_realloc(loc, newsz);
    strcat(*loc.loc, suffix);
}

/**
 * @undocumented
 */
static inline bool TN_RESULT_IS_IMPORTANT
tn_is_shared_ptr(const void *ptr)
{
    return talloc_reference_count(ptr) == 1;
}

/**
 * @undocumented
 */
static inline void
tn_copy_ptr(tn_ptr_location dst, tn_ptr_location src)
{
    if (TN_LIKELY(dst.loc != src.loc))
    {
        TN_BUG_ON(dst.context == src.context);
        TN_BUG_ON(talloc_reference(dst.context, *src.loc) == NULL);
        tn_set_loc(dst, *src.loc);
    }
}

/**
 * @undocumented
 */
static inline void
tn_move_ptr(tn_ptr_location dst, tn_ptr_location src)
{
    if (TN_LIKELY(dst.loc != src.loc))
    {
        if (dst.context != src.context)
            talloc_reparent(src.context, dst.context, *src.loc);
        tn_set_loc(dst, *src.loc);
        *src.loc = NULL;
    }
}

/**
 * @undocumented
 */
static inline void
tn_cow(tn_ptr_location loc, void (*copier)(tn_ptr_location loc))
{
    TN_BUG_ON(*loc.loc == NULL);
    if (tn_is_shared_ptr(*loc.loc))
        copier(loc);
}

/**
 * @undocumented
 */
#define TN_GLOC(_var)                           \
    ((tn_ptr_location){NULL, (void **)&(_var)})

/**
 * @undocumented
 */
#define TN_FLOC(_obj, _field)                               \
    ((tn_ptr_location){(_obj), (void **)&((_obj)->_field)})

/**
 * @undocumented
 */
#define TN_RLOC(_base, _var)                        \
    ((tn_ptr_location){(_base), (void **)&(_var)})

/**
 * @undocumented
 */
#define TN_FREE(_obj, _field)                       \
    tn_free_loc((_obj), (void **)&(_obj)->_field)

/**
 * @undocumented
 */
#define TN_ALLOC_TYPED(_loc, _type)                                     \
    tn_alloc_typed((_loc), #_type,                                      \
                   sizeof(_type),                                       \
                   _type##_destroy)

/**
 * @undocumented
 */
#define TN_ALLOC_TYPED_FLEX(_loc, _type, _flexfield, _count)            \
    tn_alloc_typed((_loc),                                              \
                   #_type "." #_flexfield "[" #_count "]",              \
                   TN_FLEX_SIZE(_type, _flexfield, (_count)),           \
                   _type##_destroy)

/**
 * @undocumented
 */
#define TN_REALLOC_FLEX(_loc, _type, _flexfield, _newcnt)           \
    tn_realloc((_loc), TN_FLEX_SIZE(_type, _flexfield, _newcnt))

/**
 * @undocumented
 */
typedef struct tn_buffer {
    size_t len; /**< @undocumented */
    size_t bufsize; /**< @undocumented */
    size_t offset; /**< @undocumented */
    tn_ptr_location location; /**< @undocumented */
} tn_buffer;

/**
 * @undocumented
 */
#define TN_BUFFER_INIT(_loc, _len, _offset)     \
    {.len = (_len),                             \
            .offset = (_offset),                \
            .location = (_loc)}

/**
 * @undocumented
 */
TN_RESULT_IS_NOT_NULL TN_NO_NULL_ARGS
extern void *tn_buffer_append(tn_buffer *buf, size_t sz);

/**
 * @undocumented
 */
#define TN_BUFFER_PUSH(_buf, _type, _n)                         \
    ((_type *)tn_buffer_append((_buf), sizeof(_type) * (_n)))

/**
 * @undocumented
 */
static inline int
TN_RESULT_IS_IMPORTANT
tn_random_int(int min, int max)
{
    long d = (long)max - min + 1;

    TN_BUG_ON(d <= 0);
    if (d > RAND_MAX)
        return (int)(random() * (d / RAND_MAX) + min);
    else
        return (int)(random() % d + min);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_UTILS_H */
