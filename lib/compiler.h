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
/** @node Compiler support
 * @chapter Compiler support
 *
 * Compilers offer various extensions over strict C standard, and
 * also implement various versions of ISO C (sometimes, only partially).
 * This file offers compatibility wrappers around some of those extensions.
 *
 */
#ifndef TNH__COMPILER_H
#define TNH_COMPILER_H 1

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


/** @defmac TN_RESULT_IS_UNALIASED
 * Indicates that the function returns
 * a pointer to unaliased uninitalized memory
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_MALLOC
#define TN_RESULT_IS_UNALIASED __attribute__((__malloc__))
#else
#define TN_RESULT_IS_UNALIASED
#endif

/** @defmac TN_RESULT_IS_NOT_NULL
 * Indicates that a function returns a non-NULL pointer
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_RETURNS_NONNULL
#define TN_RESULT_IS_NOT_NULL __attribute__((__returns_nonnull__))
#else
#define TN_RESULT_IS_NOT_NULL
#endif

/** @defmac TN_RESULT_IS_IMPORTANT
 * Requires a warning if a result value
 * of the function is thrown away
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_WARN_UNUSED_RESULT
#define TN_RESULT_IS_IMPORTANT  __attribute__((__warn_unused_result__))
#else
#define TN_RESULT_IS_IMPORTANT
#endif

/** @defmac TN_MUST_HAVE_SENTINEL
 * Indicates that a vararg function shall have NULL
 * at the end of varargs
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_SENTINEL
#define TN_MUST_HAVE_SENTINEL  __attribute__((__sentinel__))
#else
#define TN_MUST_HAVE_SENTINEL
#endif

/** @defmac TN_NO_NULL_ARGS
 * Indicates that no pointer arguments should
 * ever be passed @code{NULL}
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_NONNULL
#define TN_NO_NULL_ARGS  __attribute__ ((__nonnull__))
#else
#define TN_NO_NULL_ARGS
#endif

/** @defmac TN_LIKE_MALLOC _arg
 *  @defmacx TN_LIKE_CALLOC _arg1 _arg2
 * Indicates that a function returns
 * a pointer to memory, the size of which is given in its
 * @var{_arg}'th argument.
 * Indicates that a function returns
 * a pointer to memory, which consists of a number of elements
 * given in its @var{_arg1}'th argument, where each element has the size
 * given in the @var{_arg2}'th argument
 * @end defmac
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

/** @defmac TN_LIKE_MEMALIGN _arg1 _arg2
 * Indicates that a function returns
 * a pointer to memory, the alignment of which is given in its @var{_arg1}'th
 * argument.
 * @end defmac
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define TN_LIKE_MEMALIGN(_arg1, _arg2)                    \
    __attribute__((__alloc_align__(_arg1)))               \
    TN_LIKE_MALLOC(_arg2)
#else
#define TN_LIKE_MEMALIGN(_arg1, _arg2) TN_LIKE_MALLOC(_arg2)
#endif

/** @defmac TN_LIKE_PRINTF _x _y
 *  @defmacx TN_LIKE_VPRINTF _x
 * Indicates that a function is printf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_PRINTF(_x, _y)                          \
    __attribute__((__format__ (__printf__, _x, _y)))
#define TN_LIKE_VPRINTF(_x) TN_LIKE_PRINTF(_x, 0)
#else
#define TN_LIKE_PRINTF(_x, _y)
#define TN_LIKE_VPRINTF(_x)
#endif

/** @defmac TN_LIKE_SCANF _x _y
 *  @defmacx TN_LIKE_VSCANF _x
 * Indicates that a function is scanf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_SCANF(_x, _y)                       \
    __attribute__((__format__ (__scanf__, _x, _y)))
#define TN_LIKE_VSCANF(_x) TN_LIKE_SCANF(_x, 0)
#else
#define TN_LIKE_SCANF(_x, _y)
#define TN_LIKE_VSCANF(_x)
#endif

/** @defmac TN_LIKE_STRFTIME _x
 * Indicates that a function is strftime-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT
#define TN_LIKE_STRFTIME(_x)                            \
    __attribute__((__format__ (__strftime__, _x, 0)))
#else
#define TN_LIKE_STRFTIME(_x)
#endif

/** @defmac TN_LIKE_FORMAT_ARG
 * Indicates that a _x'th argument to the function is
 * a printf/scanf/strftime format string that is transformed in some way
 * and returned by the function
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_FORMAT_ARG
#define TN_HAS_FORMAT_ARG(_x)                   \
    __attribute__((__format_arg__ (_x)))
#else
#define TN_HAS_FORMAT_ARG(_x)
#endif


/** @defmac TN_NOT_NULL_ARGS ...
 * Indicates that pointer arguments listed in
 * `_args` are never `NULL`
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_NONNULL
#define TN_NOT_NULL_ARGS(...)                   \
    __attribute__ ((__nonnull__ (__VA_ARGS__)))
#else
#define TN_NOT_NULL_ARGS(...)
#endif

/** @defmac TN_NO_SIDE_EFFECTS
 * Global state may be read but not modified. In particular,
 * that means a function cannot produce any observable side effects
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_PURE
#define TN_NO_SIDE_EFFECTS __attribute__((__pure__))
#else
#define TN_NO_SIDE_EFFECTS
#endif

/** @defmac TN_NO_SHARED_STATE
 * No global state is accessed by the function, and so the
 * function is genuinely function, its result depending solely on its
 * arguments
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_CONST
#define TN_NO_SHARED_STATE  __attribute__((__const__))
#else
#define TN_NO_SHARED_STATE
#endif


/** @defmac TN_WEAK
 * Weak symbol (a library symbol that may be overriden
 * by the application
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_WEAK
#define TN_WEAK  __attribute__((__weak__))
#else
#define TN_WEAK
#endif

/** @defmac TN_DLL_EXPORT
 *  @defmacx TN_DLL_IMPORT
 * A symbol is exported (resp. imported) from a shared library
 * For POSIX systems these two modes are void.
 * @end defmac
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

/** @defmac TN_LOCAL
 *  @defmacx TN_INTERNAL
 * The symbol is @emph{not} exported from a shared library
 * (but unlike static symbols, it is visible to all compilation units
 * comprising the library itself).
 *
 * Internal symbols ike `local` in addition cannot be accessed by other
 * modules even indirectly (e.g. through a function pointer)
 * @end defmac
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


/** @defmac TN_DEPRECATED
 * The symbol should not be used and triggers a warning
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_DEPRECATED
#define TN_DEPRECATED __attribute__((__deprecated__))
#else
#define TN_DEPRECATED
#endif

/** @defmac TN_UNUSED
 * Marks a symbol as explicitly unused
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_UNUSED
#define TN_UNUSED  __attribute__((__unused__))
#else
#define TN_UNUSED
#endif

/** @defmac TN_USED
 * Marks a symbol as used
 * @end defmac
 */
#ifdef HAVE_FUNC_ATTRIBUTE_USED
#define TN_USED __attribute__((__used__))
#else
#define TN_USED
#endif

/** @defmac TN_CONSTRUCTOR
 *  @defmacx TN_DESTRUCTOR
 * Specifies a function as a constructor (resp. destructor),
 * that is, a function that gets called before or after main entry.
 *
 * Using constructors allows for complex initialisations for static
 * objecs.
 * @end defmac
 */
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

/** @defmac TN_GLOBAL_INIT _type _var _code
 * Declares a variable _var of type _type and
 * attaches a block of code _code to initialise it
 * at startup
 * @end defmac
 */
#define TN_GLOBAL_INIT(_type, _var, _code)      \
    _type _var;                                 \
    TNA_constructor void _var##_init(void)      \
    {                                           \
        _code;                                  \
    }                                           \
    struct fake

/** @defmac TN_LIKELY _x
 *  @defmacx TN_UNLIKELY x
 *
 * Informs the compiler that a condition _x is likely/unlikely
 * to be held.
 * @end defmac
 */
#ifdef HAVE___BUILTIN_EXPECT
#define TN_LIKELY(_x) __builtin_expect(!!(_x), 1)
#define TN_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#else
#define TN_LIKELY(_x) (_x)
#define TN_UNLIKELY(_x) (_x)
#endif


/** @section C99/C11 annotations
 * Compilers vary in their support for C99 and C11 features,
 * and the level of support may be altered by command-line switches
 */

/** @defmac static_assert _cond _msg
 * Defined in <assert.h> by C11-compliant systems.
 * If not defined, revert to an old trick of using negative array size
 * @end defmac
 */
#if !defined(static_assert)
#define static_assert(_cond, _msg) ((void)sizeof(char[(_cond) ? 1 : -1]))
#endif

/** @defmac TN_AT_LEAST
 * A C99 feature to specify a mininum required number of
 * array elements when an array is passed as parameter. Supported by GCC
 * in its default mode if no strict ISO compliance is requested.
 * Since C99 reuses `static' for this purpose, we need to conditionally
 * define our own macro
 * @end defmac
 */
#if __STDC_VERSION__ >= 199901L || (defined(__GNUC__) && !__STRICT_ANSI__)
#define TN_AT_LEAST static
#else
#define TN_AT_LEAST
#endif

/** @defmac TN_VAR_SIZE
 * C99 allows arrays to be declared in function declarations
 * with non-constant dimensions, depending on previous arguments.
 * It does not affect the produced code and no compile- or run-time
 * checks are usually performed, but the code intent is more evident
 * this way
 * @end defmac
 */
#if (__STDC_VERSION__ >= 199901L && !defined(__STDC_NO_VLA__)) ||   \
    (defined(__GNUC__) && !__STRICT_ANSI__)
#define TN_VAR_SIZE(_x) _x
#else
#define TN_VAR_SIZE(_x)
#endif

/** @defmac noreturn
 * For C11 systems, it is defined in a new header
 * <stdnoreturn.h>. For GCC in non-C11 mode define it via noreturn
 * attribute
 * @end defmac
 */
#if __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#elif  HAVE_FUNC_ATTRIBUTE_NORETURN
#define noreturn __attribute__((__noreturn__))
#else
#define noreturn
#endif

/** @defmac alignas _x
 *  @defmacx alignof _x
 * In C11, these constructs defined in <stdalign.h> allow to
 * query the object's alignment and to request an object
 * to be aligned as some other alignment.
 *
 * GCC has supported __alignof__ for ages, and alignas can be
 * simulated by a the aligned attribute + __alignof__
 * @end defmac
 */
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
#define alignof(_x) alignof_not_supported
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* TNH_COMPILER_H */
