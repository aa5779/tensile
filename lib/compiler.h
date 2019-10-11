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
 * @brief Compiler support
 *
 * Compilers offer various extensions over strict C standard, and
 * also implement various versions of ISO C (sometimes, only partially).
 * This file offers compatibility wrappers around some of those extensions.
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef COMPILER_H
#define COMPILER_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>
#include "tn_config.h"


/**
 * Indicates that the function returns
 * a pointer to unaliased uninitalized memory
 */
#ifdef HAVE_FUNC_ATTRIBUTE_MALLOC
#define TN_RESULT_IS_UNALIASED __attribute__((__malloc__))
#else
#define TN_RESULT_IS_UNALIASED
#endif

/**
 * Indicates that a function returns a non-NULL pointer
 */
#ifdef HAVE_FUNC_ATTRIBUTE_RETURNS_NONNULL
#define TN_RESULT_IS_NOT_NULL __attribute__((__returns_nonnull__))
#else
#define TN_RESULT_IS_NOT_NULL
#endif

/**
 * Requires a warning if a result value
 * of the function is thrown away
 */
#ifdef HAVE_FUNC_ATTRIBUTE_WARN_UNUSED_RESULT
#define TN_RESULT_IS_IMPORTANT  __attribute__((__warn_unused_result__))
#else
#define TN_RETURNS_IMPORTANT
#endif

/**
 * Indicates that a vararg function shall have NULL
 * at the end of varargs
 */
#ifdef HAVE_FUNC_ATTRIBUTE_SENTINEL
#define TN_MUST_HAVE_SENTINEL  __attribute__((__sentinel__))
#else
#define TN_MUST_HAVE_SENTINEL
#endif

/**
 * Indicates that no pointer arguments should
 * ever be passed NULL
 */
#ifdef HAVE_FUNC_ATTRIBUTE_NONNULL
#define TN_NO_NULL_ARGS  __attribute__ ((__nonnull__))
#else
#define TN_NO_NULL_ARGS
#endif

/**
 * Indicates that a function returns
 * a pointer to memory, the size of which is given in its @arg _arg'th argument.
 * Indicates that a function returns
 * a pointer to memory, which consists of a number of elements
 * given in its x'th argument, where each element has the size given in
 * the y'th argument
 */
#ifdef HAVE_FUNC_ATTRIBUTE_ALLOC_SIZE
#define TN_LIKE_MALLOC(_arg)                \
    TN_RESULT_IS_UNALIASED                  \
    TN_RESULT_IS_IMPORTANT                  \
    __attribute__((__alloc_size__(_arg)))
#define LIKE_CALLOC(_arg1, _arg2)                    \
    TN_RESULT_IS_UNALIASED                           \
    TN_RESULT_IS_IMPORTANT                           \
    __attribute__((__alloc_size__(_arg1, _arg2)))
#else
#define TN_LIKE_MALLOC(_arg) \
    TN_RESULT_IS_UNALIASED \
    TN_RESULT_IS_IMPORTANT
#define TN_LIKE_CALLOC(_arg1, _arg2) \
    TN_RESULT_IS_UNALIASED \
    TN_RESULT_IS_IMPORTANT
#endif

/**
 * Indicates that a function returns
 * a pointer to memory, the alignment of which is given in its _x'th argument.
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define TN_LIKE_MEMALIGN(_arg1, _arg2)                    \
    __attribute__((__alloc_align__(_arg1)))               \
    TN_LIKE_MALLOC(_arg2)
#else
#define TN_LIKE_MEMALIGN(_arg1, _arg2) TN_LIKE_MALLOC(_arg2)
#endif

/**
 * Indicates that a function is printf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_PRINTF(_x, _y)                          \
    __attribute__((__format__ (__printf__, _x, _y)))
#define TN_LIKE_VPRINTF(_x) TN_LIKE_PRINTF(_x, 0)
#else
#define TN_LIKE_PRINTF(_x, _y)
#define TN_LIKE_VPRINTF(_x)
#endif

/**
 * Indicates that a function is scanf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_SCANF(_x, _y)                       \
    __attribute__((__format__ (__scanf__, _x, _y)))
#define TN_LIKE_VSCANF(_x) TN_LIKE_SCANF(_x, 0)
#else
#define TN_LIKE_SCANF(_x, _y)
#define TN_LIKE_VSCANF(_x)
#endif

/**
 * Indicates that a function is strftime-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_STRFTIME(_x)                            \
    __attribute__((__format__ (__strftime__, _x, 0)))
#else
#define TN_LIKE_STRFTIME(_x)
#endif

/**
 * Indicates that a _x'th argument to the function is
 * a printf/scanf/strftime format string that is transformed in some way
 * and returned by the function
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT_ARG
#define TN_HAS_FORMAT_ARG(_x)                   \
    __attribute__((__format_arg__ (_x)))
#else
#define TN_HAS_FORMAT_ARG(_x)
#endif


/**
 * Indicates that pointer arguments listed in
 * `_args` are never `NULL`
 */
#ifdef HAVE_FUNC_ATTRIBUTE_NONNULL
#define TN_NOT_NULL_ARGS(...)                   \
    __attribute__ ((__nonnull__ (__VA_ARGS__)))
#else
#define TN_NOT_NULL_ARGS(...)
#endif

/**
 * Global state may be read but not modified. In particular,
 * that means a function cannot produce any observable side effects
 */
#ifdef HAVE_FUNC_ATTRIBUTE_PURE
#define TN_NO_SIDE_EFFECTS __attribute__((__pure__))
#else
#define TN_NO_SIDE_EFFECTS
#endif

/**
 * No global state is accessed by the function, and so the
 * function is genuinely function, its result depending solely on its
 * arguments
 */
#ifdef HAVE_FUNC_ATTRIBUTE_CONST
#define TN_NO_SHARED_STATE  __attribute__((__const__))
#else
#define TN_NO_SHARED_STATE
#endif


/**
 * Weak symbol (a library symbol that may be overriden
 * by the application
 */
#ifdef HAVE_FUNC_ATTRIBUTE_WEAK
#define TN_WEAK  __attribute__((__weak__))
#else
#define TN_WEAK
#endif

/** @def DLL_EXPORT_LINKAGE
 * A symbol is exported from a shared library
 */
/** @def DLL_IMPORT_LINKAGE
 * A symbol is imported from a shared library
 * For POSIX systems these two modes are void.
 */
#ifdef HAVE_FUNC_ATTRIBUTE_DLLEXPORT
#define TN_DLL_EXPORT  __declspec(dllexport)
#else
#define TN_DLL_EXPORT
#endif

#ifdef HAVE_FUNC_ATTRIBUTE_DLLIMPORT
#define TN_DLL_IMPORT __declspec(dllimport)
#else
#define TN_DLL_IMPORT
#endif

/** @def LOCAL_LINKAGE
 * symbol is **not** exported from a shared library
 * (but unlike static symbols, it is visible to all compilation units
 * comprising the library itself)
 */
/**  @def INTERNAL_LINKAGE
 * Like `local`, but the symbol cannot be accessed by other
 * modules even indirectly (e.g. through a function pointer)
 */
#ifdef HAVE_FUNC_ATTRIBUTE_VISIBILITY
#define TN_LOCAL                                \
    __attribute__ ((__visibility__ ("hidden")))
#define TN_INTERNAL                                 \
    __attribute__ ((__visibility__ ("internal")))
#else
#define TN_LOCAL
#define TN_INTERNAL
#endif


/**
 * The symbol should not be used and triggers a warning
 */
#ifdef HAVE_FUNC_ATTRIBUTE_DEPRECATED
#define TN_DEPRECATED __attribute__((__deprecated__))
#else
#define TN_DEPRECATED
#endif

/**
 * Marks a symbol as explicitly unused
 */
#ifdef HAVE_FUNC_ATTRIBUTE_UNUSED
#define TN_UNUSED  __attribute__((__unused__))
#else
#define TN_UNUSED
#endif

/**
 * Marks a symbol as used
 */
#ifdef HAVE_FUNC_ATTRIBUTE_USED
#define TN_USED __attribute__((__used__))
#else
#define TN_USED
#endif

/**@}*/

/** @name C99/C11 annotations
 * Compilers vary in their support for C99 and C11 features,
 * and the level of support may be altered by command-line switches
 */
/**@{*/

/**
 * Defined in <assert.h> by C11-compliant systems.
 * If not defined, revert to an old trick of using negative array size
 */
#if !defined(static_assert)
#define static_assert(_cond, _msg) ((void)sizeof(char[(_cond) ? 1 : -1]))
#endif

/**
 * A C99 feature to specify a mininum required number of
 * array elements when an array is passed as parameter. Supported by GCC
 * in its default mode if no strict ISO compliance is requested.
 * Since C99 reuses `static' for this purpose, we need to conditionally
 * define our own macro
 */
#if __STDC_VERSION__ >= 199901L || (defined(__GNUC__) && !__STRICT_ANSI__)
#define TN_AT_LEAST static
#else
#define TN_AT_LEAST
#endif

/**
 * C99 allows arrays to be declared in function declarations
 * with non-constant dimensions, depending on previous arguments.
 * It does not affect the produced code and no compile- or run-time
 * checks are usually performed, but the code intent is more evident
 * this way
 */
#if (__STDC_VERSION__ >= 199901L && !__STDC_NO_VLA__) ||    \
    (defined(__GNUC__) && !__STRICT_ANSI__)
#define TN_VAR_SIZE(_x) _x
#else
#define TN_VAR_SIZE(_x)
#endif

/**
 * For C11 systems, it is defined in a new header
 * <stdnoreturn.h>. For GCC in non-C11 mode define it via noreturn
 * attribute
 */
#if __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#elif  HAVE_FUNC_ATTRIBUTE_NORETURN
#define noreturn __attribute__((__noreturn__))
#else
#define noreturn
#endif

#if __STDC_VERSION__ >= 201112L
#include <stdalign.h>
#else
#if HAVE_VAR_ATTRIBUTE_ALIGNED
#define alignas(_n) __attribute__((__aligned__ (_n)))
#else
#define alignas(_x) alignas_not_supported
#endif
#if defined(__GNUC__)
#define alignof(_x) __alignof__(_x)
#else
#define alignof(_x) alignaof_not_supported
#endif
#endif

#ifdef HAVE_FUNC_ATTRIBUTE_CONSTRUCTOR
#define TN_CONSTRUCTOR static __attribute__((__constructor__))
#else
#define TN_CONSTRUCTOR constructors_are_not_supported
#endif

#ifdef HAVE_FUNC_ATTRIBUTE_DESTRUCTOR
#define TN_DESTRUCTOR static __attribute__((__destructor__))
#else
#define TN_DESTRUCTOR destructors_are_not_supported
#endif

#define TN_GLOBAL_INIT(_type, _var, _code)      \
    _type _var;                                 \
    TNA_constructor void _var##_init(void)      \
    {                                           \
        _code;                                  \
    }                                           \
    struct fake


/**@}*/

#ifdef __cplusplus
}
#endif
#endif /* COMPILER_H */
