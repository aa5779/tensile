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
#include <gc.h>

static inline MUST_USE NOT_NULL LIKE_MALLOC(1)
void *tn_alloc(size_t sz)
{
    void *obj = GC_MALLOC(sz);
    assert(obj != NULL);
    return obj;
}

static inline MUST_USE NOT_NULL LIKE_MALLOC(1)
void *tn_alloc_blob(size_t sz)
{
    void *obj = GC_MALLOC_ATOMIC(sz);
    assert(obj != NULL);
    return obj;
}

static inline MUST_USE NOT_NULL
void *tn_realloc(void *ptr, size_t newsz)
{
    void *obj = GC_REALLOC(ptr, newsz);
    assert(obj != NULL);
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* UTILS_H */
