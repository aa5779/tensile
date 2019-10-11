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

/** @file
 * @brief various utility functions
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
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
#include <talloc.h>

#if defined(ck_abort)
#define abort() ck_abort()
#endif

#define TN_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))

#define TN_ARRAY_GET(_arr, _idx, _defval)                       \
    ((_idx) < TN_ARRAY_SIZE(_arr) ? (_arr)[_idx] : (_defval))

#define TN_FLEX_SIZE(_type, _flexfield, _count)                         \
    (sizeof(_type) + (_count) * sizeof(((_type *)NULL)->_flexfield[0]))

static inline void
tn_bug_on(bool cond, const char *file, int line, const char *msg)
{
    if (cond)
    {
        fprintf(stderr, "%s:%d: %s\n", file, line, msg);
        abort();
    }
}

#define TN_BUG_ON(_expr)                            \
    tn_bug_on(_expr, __FILE__, __LINE__, #_expr)

static inline TN_LIKE_MALLOC(3) TN_RESULT_IS_NOT_NULL
void *
tn_alloc_typed(const void *parent, const char *type, size_t sz,
               int (*destructor)(void *))
{
    void *obj = talloc_zero_size(parent, sz);

    TN_BUG_ON(obj == NULL);

    if (type != NULL)
        talloc_set_name_const(obj, type);
    if (destructor != NULL)
        talloc_set_destructor(obj, destructor);
    return obj;
}

static inline TN_LIKE_MALLOC(2) TN_RESULT_IS_NOT_NULL
void *
tn_alloc_raw(const void *parent, size_t sz)
{
    void *obj = talloc_size(parent, sz);

    TN_BUG_ON(obj == NULL);

    return obj;
}

static inline TN_RESULT_IS_UNALIASED TN_RESULT_IS_IMPORTANT
TN_RESULT_IS_NOT_NULL
char *
tn_strdup(const void *parent, const char *str)
{
    char *copy = talloc_strdup(parent, str);

    TN_BUG_ON(copy == NULL);

    return copy;
}

#define TN_FREE(_obj, _field)                                       \
    ((void)((_obj)->_field == NULL ? 0 :                            \
            TN_BUG_ON(talloc_unlink((_obj), (_obj)->_field) != 0),  \
            (_obj)->_field = NULL))

#define TN_ALLOC_TYPED(_parent, _type)                                  \
    ((_type *)tn_alloc_typed((_parent), #_type,                         \
                             sizeof(_type),                             \
                             _type##_destroy))

#define TN_ALLOC_TYPED_FLEX(_parent, _type, _flexfield, _count)         \
    ((_type *)tn_alloc_typed((_parent), #_type "." #_flexfield "[" #_count "]", \
                             TN_FLEX_SIZE(_type, _flexfield, _count),   \
                             _type##_destroy))

#define TN_SET(_obj, _field, _type)                                 \
    ((void)(TN_FREE((_obj), _field),                                \
            (_obj)->_field = TN_ALLOC_TYPED(_obj, _type)))

#define TN_SET_FLEX(_obj, _field, _type, _flexfield, _count)            \
    ((void)(TN_FREE((_obj), _field),                                    \
            (_obj)->_field = TN_ALLOC_TYPED_FLEX(_obj, _type,           \
                                                 _flexfield, _count)))

#define TN_SET_RAW(_obj, _field, _sz)                   \
    ((void)(TN_FREE((_obj), _field),                    \
            (_obj)->_field = tn_alloc_raw(_obj, _sz)))

#define TN_SET_STR(_obj, _field, _str)                                  \
    ((void)(TN_FREE((_obj), _field),                                    \
            (((_str) == NULL) ? 0 : (_obj)->_field = tn_strdup(_obj, _str)))

#define TN_SET_SPRINTF(_obj, _field, _fmt, ...)                         \
    ((void)(TN_FREE((_obj), _field),                                    \
            TN_BUG_ON(((_obj)->_field =                                 \
                   talloc_asprintf((_obj), (_fmt),                      \
                                   __VA_ARGS__)) == NULL)))

#define TN_COPY(_obj, _field, _value)                               \
    ((void)(TN_BUG_ON(talloc_reference((_obj), (_value)) == NULL),  \
            TN_FREE((_obj), _field),                                \
            (_obj)->_field = (_value)))

#define TN_MOVE(_dstobj, _dstfield, _srcobj, _srcfield)                 \
    ((void)(TN_FREE((_dstobj), _dstfield),                              \
        (_dstobj)->_dstfield = talloc_reparent((_srcobj),               \
                                               (_srcobj)->_srcfield,    \
                                               (_dstobj))

#define TN_NEW_GLOBAL(_type) TN_ALLOC_TYPED(NULL, _type)

#define TN_FREE_GLOBAL(_ptr)                    \
    ((void)((_ptr) == NULL ? 0 :                \
            TN_BUG_ON(talloc_free(_ptr) != 0)))

static inline TN_RESULT_IS_IMPORTANT TN_RESULT_IS_NOT_NULL
void *
tn_realloc_raw(const void *parent, void *ptr, size_t newsz)
{
    void *obj = talloc_realloc_size(parent, ptr, newsz);

    TN_BUG_ON(obj == NULL);
    return obj;
}

#define TN_REALLOC_FLEX(_obj, _field, _type, _flexfield, _newcnt)       \
    ((void)((_obj)->_field =                                            \
            tn_realloc_raw((_obj), (_obj)->_field,                      \
                           TN_FLEX_SIZE(_type, _flexfield, _newcnt))))

static inline TN_RESULT_IS_IMPORTANT TN_RESULT_IS_NOT_NULL
char *
tn_strcat(const void *parent, char *src, const char *suffix)
{
    size_t newsz = strlen(src) + strlen(suffix) + 1;
    char *newstr = tn_realloc_raw(parent, src, newsz);

    strcat(newstr, suffix);

    return newstr;
}

#define TN_APPEND_STR(_obj, _field, _suffix)                            \
    ((void)((_obj)->_field = tn_strcat(_obj, (_obj)->_field, _suffix)))

static inline bool
tn_is_shared_ptr(const void *ptr)
{
    return talloc_reference_count(ptr) == 1;
}

#define TN_COW(_obj, _field, _copy)                         \
    (!tn_is_shared_ptr((_obj)->_field) ?                    \
     (_obj)->_field :                                       \
     ((_obj)->_field = (_copy)((_obj), (_obj)->_field)))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_UTILS_H */
