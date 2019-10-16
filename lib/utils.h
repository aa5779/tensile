/**********************************************************************
 * Copyright (c) 2017 Artem V. Andreev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

/** @node Utility functions
 * @chapter Utility functions
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

#ifdef UNIT_TESTING
#define abort() exit(1)
#endif

#define TN_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))

#define TN_ARRAY_GET(_arr, _idx, _defval)                       \
    ((_idx) < TN_ARRAY_SIZE(_arr) ? (_arr)[_idx] : (_defval))

#define TN_FLEX_SIZE(_type, _flexfield, _count)                         \
    (sizeof(_type) + (_count) * sizeof(((_type *)NULL)->_flexfield[0]))

#define TN_STRINGIFY(_x) #_x

static inline void
tn_bug_on(bool cond, const char *file, int line, const char *msg)
{
    if (TN_UNLIKELY(cond))
    {
        fprintf(stderr, "%s:%d: %s\n", file, line, msg);
        abort();
    }
}

#define TN_BUG_ON(_expr)                            \
    tn_bug_on(_expr, __FILE__, __LINE__, #_expr)

#define tn_log_error(_file, _line, _fmt, ...)                           \
    ((void)fprintf(stderr, "error:%s:%d: " _fmt "\n", _file, _line,     \
                   __VA_ARGS__))

#define tn_log_warning(_file, _line, _fmt, ...)                         \
    ((void)fprintf(stderr, "warning:%s:%d: " _fmt "\n", _file, _line,   \
                   __VA_ARGS__))

#define tn_fatal_error(_file, _line, _fmt, ...)                         \
    ((void)fprintf(stderr, "FATAL:%s:%d: " _fmt "\n", _file, _line,     \
                   __VA_ARGS__), abort())

#define TN_INTERNAL_ERROR(_fmt, ...)                    \
    tn_log_error(__FILE__, __LINE__, _fmt, __VA_ARGS__)

typedef struct tn_ptr_location {
    const void *context;
    void **loc;
} tn_ptr_location;

static inline void
tn_free(tn_ptr_location loc)
{
    if (TN_LIKELY(*loc.loc != NULL))
    {
        TN_BUG_ON(talloc_unlink(loc.context, *loc.loc) != 0);
        *loc.loc = NULL;
    }
}

static inline void
tn_set_loc(tn_ptr_location loc, void *ptr)
{
    tn_free(loc);
    *loc.loc = ptr;
}

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

static inline void
tn_alloc_raw(tn_ptr_location loc, size_t sz)
{
    void *obj = talloc_size(loc.context, sz);

    TN_BUG_ON(obj == NULL);

    tn_set_loc(loc, obj);
}

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

static inline void TN_LIKE_VPRINTF(2)
tn_vsprintf(tn_ptr_location loc, const char *fmt, va_list args)
{
    char *buf = talloc_vasprintf(loc.context, fmt, args);

    TN_BUG_ON(buf == NULL);

    tn_set_loc(loc, buf);
}

extern void TN_LIKE_PRINTF(2, 3) tn_sprintf(tn_ptr_location loc,
                                            const char *fmt, ...);

static inline void
tn_realloc(tn_ptr_location loc, size_t newsz)
{
    void *obj = talloc_realloc_size(loc.context, *loc.loc, newsz);

    TN_BUG_ON(obj == NULL);
    *loc.loc = obj;
}

static inline void
tn_strcat(tn_ptr_location loc, const char *suffix)
{
    size_t newsz = strlen(*loc.loc) + strlen(suffix) + 1;

    tn_realloc(loc, newsz);
    strcat(*loc.loc, suffix);
}

static inline bool TN_RESULT_IS_IMPORTANT
tn_is_shared_ptr(const void *ptr)
{
    return talloc_reference_count(ptr) == 1;
}

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

static inline void
tn_cow(tn_ptr_location loc, void (*copier)(tn_ptr_location loc))
{
    TN_BUG_ON(*loc.loc == NULL);
    if (tn_is_shared_ptr(*loc.loc))
        copier(loc);
}

#define TN_GLOC(_var)                           \
    ((tn_ptr_location){NULL, (void **)&(_var)})

#define TN_FLOC(_obj, _field)                               \
    ((tn_ptr_location){(_obj), (void **)&((_obj)->_field)})

#define TN_FREE(_obj, _field)                       \
    tn_free_loc((_obj), (void **)&(_obj)->_field)

#define TN_ALLOC_TYPED(_loc, _type)                                     \
    tn_alloc_typed((_loc), #_type,                                      \
                   sizeof(_type),                                       \
                   _type##_destroy)

#define TN_ALLOC_TYPED_FLEX(_loc, _type, _flexfield, _count)            \
    tn_alloc_typed((_loc),                                              \
                   #_type "." #_flexfield "[" #_count "]",              \
                   TN_FLEX_SIZE(_type, _flexfield, (_count)),           \
                   _type##_destroy)

#define TN_REALLOC_FLEX(_loc, _type, _flexfield, _newcnt)           \
    tn_realloc((_loc), TN_FLEX_SIZE(_type, _flexfield, _newcnt))

static inline int
TN_RESULT_IS_IMPORTANT
tn_random_int(int min, int max)
{
    long d = (long)max - min + 1;
    if (d > RAND_MAX)
        return (int)(random() * (d / RAND_MAX) + min);
    else
        return (int)(random() % d + min);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_UTILS_H */
