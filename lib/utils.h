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
#ifndef UTILS_H
#define UTILS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include "compiler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <talloc.h>

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
               int (*)
{
    void *obj = talloc_zero_size(parent, sz);

    TN_BUG_ON(obj == NULL);
    return obj;
}

static inline MUST_USE NOT_NULL
void *tn_realloc(void *ptr, size_t newsz)
{
    void *obj = talloc_realloc(NULL, ptr, newsz);

    tn_bug_on(obj == NULL);
    return obj;
}

static inline MUST_USE NOT_NULL
char *tn_cstrdup(const char *str)
{
    char *copy = GC_STRDUP(str ? str : "");
    assert(copy != NULL);
    return copy;
}

#define TN_NEW(_type) ((_type *)tn_alloc(sizeof(_type)))

typedef GC_finalization_proc tn_finalizer;

static inline
void tn_set_finalizer(void *obj, tn_finalizer fn, void *data)
{
    GC_register_finalizer(obj, fn, data, NULL, NULL);
}

#define TN_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))

#define TN_ARRAY_GET(_arr, _idx, _defval)                       \
    ((_idx) < TN_ARRAY_SIZE(_arr) ? (_arr)[_idx] : (_defval))

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* UTILS_H */
