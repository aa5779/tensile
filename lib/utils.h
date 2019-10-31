/*
 * Copyright (c) 2017-2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

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

extern void __gcov_dump (void);

/** @defmac TN_ARRAY_SIZE _arr
 *  Returns the number of elements in an array @var{_arr}
 *  @end defmac
 */
#define TN_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))

/** @defmac TN_ARRAY_GET _arr _idx _defval
 *  Returns @var{_idx}'th element in @var{_arr},
 *  if it is less than the total number of elements;
 *  otherwise return @var{_defval}
 *  @end defmac
 */
#define TN_ARRAY_GET(_arr, _idx, _defval)                       \
    ((_idx) < TN_ARRAY_SIZE(_arr) ? (_arr)[_idx] : (_defval))

/** @defmac TN_FLEX_SIZE _type _flexfield _count
 *  Returns the size of a flexible structure @var{_type},
 *  where its flexible member @var{_flexfield} contains
 *  @var{_count} elements
 *  @end defmac
 */
#define TN_FLEX_SIZE(_type, _flexfield, _count)                         \
    (offsetof(_type, _flexfield) +                                      \
     (_count) * sizeof(((_type *)NULL)->_flexfield[0]))

/** @defmac TN_STRINGIFY _x
 *  Returns its argument @var{_x} stringified.
 *  If @var{_x} contains macros, they are expanded,
 *  unlike the case of simple @code{#} operator
 *  @end defmac
 */
#define TN_STRINGIFY(_x) #_x

/** @deftypefun void tn_bug_on (bool @var{cond}, @
 *                              const char *@var{file}, int line, @
 *                              const char *@var{msg})
 *  @anchor{tn_bug_on}
 *  Aborts the program if @var{cond} is false, displaying a message
 *  @var{file}:@var{line}: @var{msg}
 *  @xref{TN_BUG_ON}
 *  @end deftypefun
 */
static inline void
tn_bug_on(bool cond, const char *file, int line, const char *msg)
{
    if (TN_UNLIKELY(cond))
    {
        fprintf(stderr, "%s:%d: %s\n", file, line, msg);
        __gcov_dump();
        abort();
    }
}

/** @defmac TN_BUG_ON _expr
 *  @anchor{TN_BUG_ON}
 *
 *  Aborts the program if @var{_expr} is false.
 *  Prints the stringified form of @var{_expr} and
 *  the current source code location
 *  @xref{tn_bug_on}
 *  @end defmac
 */
#define TN_BUG_ON(_expr)                            \
    tn_bug_on(_expr, __FILE__, __LINE__, #_expr)

/** @defmac  tn_log_error _file _line _fmt ...
 *  @defmacx tn_log_warning _file _line _fmt ...
 *  @defmacx tn_fatal_error _file _line _fmt ...
 *  @defmacx TN_INTERNAL_ERROR _fmt ...
 *  Prints a formatted message related to @var{_file}:@var{_line}.
 *  @code{tn_log_error} and @code{TN_INTERNAL_ERROR} produce
 *  an error message, @code{tn_log_warning} produces a warning,
 *  and @code{tn_fatal_error} produces a fatal error and aborts
 *  the program. @code{TN_INTERNAL_ERROR} is a wrapper around
 *  @code{tn_log_error} that uses the current source code location
 *  as a relevant filename and line.
 *  @end defmac
 */
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

/** @deftp Structure tn_ptr_location
 * A data type to store pooled pointer information.
 * @code{context} is a memory pool owning the pointer;
 * @code{loc} is an address of a variable holding the pointer
 * @end deftp
 */
typedef struct tn_ptr_location {
    const void *context;
    void **loc;
} tn_ptr_location;

/** @deftypefun void tn_free (tn_ptr_location @var{loc})
 *  Releases the memory pointed to by @var{loc}.
 *  The memory is detached from the context and if there's
 *  no other contexts referring to it, it is freed
 *  @end deftypefun
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


/** @deftypefun void tn_set_loc (tn_ptr_location @var{loc}, void *@var{ptr})
 *  @undocumented
 *  @end deftypefun
 */
static inline void
tn_set_loc(tn_ptr_location loc, void *ptr)
{
    tn_free(loc);
    *loc.loc = ptr;
}

/** @deftypefun void tn_alloc_typed (tn_ptr_location @var{loc}, @
 *                                   const char *@var{type}, @
 *                                   size_t @var{sz}, @
 *                                   int (*@var{destructor})(void *))
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun void tn_alloc_raw (tn_ptr_location @var{loc}, size_t @var{sz})
 *  @undocumented
 *  @end deftypefun
 */
static inline void
tn_alloc_raw(tn_ptr_location loc, size_t sz)
{
    void *obj = talloc_size(loc.context, sz);

    TN_BUG_ON(obj == NULL);

    tn_set_loc(loc, obj);
}

/** @deftypefun void tn_strdup (tn_ptr_location @var{loc}, @
 *                              const char *@var{str})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun void tn_vsprintf (tn_ptr_location @var{loc}, @
 *                                const char *@var{fmt}, @
 *                                va_list @var{args})
 *  @undocumented
 *  @end deftypefun
 */
static inline void TN_LIKE_VPRINTF(2)
tn_vsprintf(tn_ptr_location loc, const char *fmt, va_list args)
{
    char *buf = talloc_vasprintf(loc.context, fmt, args);

    TN_BUG_ON(buf == NULL);

    tn_set_loc(loc, buf);
}

/** @deftypefun void tn_sprintf (tn_ptr_location @var{loc}, @
 *                               const char *@var{fmt}, @
 *                               ...)
 *  @undocumented
 *  @end deftypefun
 */
extern void TN_LIKE_PRINTF(2, 3) tn_sprintf(tn_ptr_location loc,
                                            const char *fmt, ...);

/** @deftypefun void tn_realloc(tn_ptr_location @var{loc}, @
 *                              size_t @var{newsz})
 *  @undocumented
 *  @end deftypefun
 */
static inline void
tn_realloc(tn_ptr_location loc, size_t newsz)
{
    void *obj = talloc_realloc_size(loc.context, *loc.loc, newsz);

    TN_BUG_ON(obj == NULL);
    *loc.loc = obj;
}

/** @deftypefun void tn_strcat(tn_ptr_location @var{loc}, @
 *                             const char *@var{suffix})
 *  @undocumented
 *  @end deftypefun
 */
static inline void
tn_strcat(tn_ptr_location loc, const char *suffix)
{
    size_t newsz = strlen(*loc.loc) + strlen(suffix) + 1;

    tn_realloc(loc, newsz);
    strcat(*loc.loc, suffix);
}

/** @deftypefun bool tn_is_shared_ptr(const void *@var{ptr})
 *  @undocumented
 *  @end deftypefun
 */
static inline bool TN_RESULT_IS_IMPORTANT
tn_is_shared_ptr(const void *ptr)
{
    return talloc_reference_count(ptr) == 1;
}

/** @deftypefun void tn_copy_ptr(tn_ptr_location @var{dst}, @
 *                               tn_ptr_location @var{src})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun void tn_move_ptr(tn_ptr_location @var{dst}, @
 *                               tn_ptr_location @var{src})
 *  @undocumented
 *  @end deftypefun
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

/** @deftypefun void tn_cow(tn_ptr_location @var{loc}, @
 *                          void (*@var{copier})(tn_ptr_location @var{loc}))
 *  @undocumented
 *  @end deftypefun
 */
static inline void
tn_cow(tn_ptr_location loc, void (*copier)(tn_ptr_location loc))
{
    TN_BUG_ON(*loc.loc == NULL);
    if (tn_is_shared_ptr(*loc.loc))
        copier(loc);
}

/** @defmac TN_GLOC _var
 *  @undocumented
 *  @end defmac
 */
#define TN_GLOC(_var)                           \
    ((tn_ptr_location){NULL, (void **)&(_var)})

/** @defmac TN_FLOC _obj _field
 *  @undocumented
 *  @end defmac
 */
#define TN_FLOC(_obj, _field)                               \
    ((tn_ptr_location){(_obj), (void **)&((_obj)->_field)})

/** @defmac TN_RLOC _base _var
 *  @undocumented
 *  @end defmac
 */
#define TN_RLOC(_base, _var)                        \
    ((tn_ptr_location){(_base), (void **)&(_var)})

/** @defmac TN_FREE _obj _field
 *  @undocumented
 *  @end defmac
 */
#define TN_FREE(_obj, _field)                       \
    tn_free_loc((_obj), (void **)&(_obj)->_field)

/** @defmac TN_ALLOC_TYPED _loc _type
 *  @undocumented
 *  @end defmac
 */
#define TN_ALLOC_TYPED(_loc, _type)                                     \
    tn_alloc_typed((_loc), #_type,                                      \
                   sizeof(_type),                                       \
                   _type##_destroy)

/** @defmac TN_ALLOC_TYPED_FLEX _loc _type _flexfield _count
 *  @undocumented
 *  @end defmac
 */
#define TN_ALLOC_TYPED_FLEX(_loc, _type, _flexfield, _count)            \
    tn_alloc_typed((_loc),                                              \
                   #_type "." #_flexfield "[" #_count "]",              \
                   TN_FLEX_SIZE(_type, _flexfield, (_count)),           \
                   _type##_destroy)

/** @defmac TN_REALLOC_FLEX _loc _type _flexfield _newcnt
 *  @undocumented
 *  @end defmac
 */
#define TN_REALLOC_FLEX(_loc, _type, _flexfield, _newcnt)           \
    tn_realloc((_loc), TN_FLEX_SIZE(_type, _flexfield, _newcnt))

/** @deftp Structure tn_buffer
 *  @undocumented
 *  @end deftp
 */
typedef struct tn_buffer {
    size_t len;
    size_t bufsize;
    size_t offset;
    tn_ptr_location location;
} tn_buffer;

/** @defmac TN_BUFFER_INIT _loc _len _offset
 *  @undocumented
 *  @end defmac
 */
#define TN_BUFFER_INIT(_loc, _len, _offset)     \
    {.len = (_len),                             \
            .offset = (_offset),                \
            .location = (_loc)}

/** @deftypefun void tn_buffer_append(tn_buffer @var{buf}, @
 *                                    size_t @var{sz})
 *  @undocumented
 *  @end deftypefun
 */
TN_RESULT_IS_NOT_NULL TN_NO_NULL_ARGS
extern void *tn_buffer_append(tn_buffer *buf, size_t sz);

/** @defmac TN_BUFFER_PUSH _buf _type _n
 *  @undocumented
 *  @end defmac
 */
#define TN_BUFFER_PUSH(_buf, _type, _n)                         \
    ((_type *)tn_buffer_append((_buf), sizeof(_type) * (_n)))

/** @deftypefun int tn_random_int (int @var{min}, int @var{max})
 *  @undocumented
 *  @end deftypefun
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
