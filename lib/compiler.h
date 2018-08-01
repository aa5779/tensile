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


/**
 * Indicates that the function returns
 * a pointer to unaliased uninitalized memory
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define UNALIASED  __attribute__((__malloc__))
#else
#define UNALIASED
#endif

/**
 * Indicates that a function returns a non-NULL pointer
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define NOT_NULL __attribute__((__returns_nonnull__))
#else
#define NOT_NULL
#endif

/**
 * Requires a warning if a result value
 * of the function is thrown away
 */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#define MUST_USE  __attribute__((__warn_unused_result__))
#else
#define MUST_USE
#endif

/**
 * Indicates that a vararg function shall have NULL
 * at the end of varargs
 */
#if __GNUC__ >= 4
#define LAST_ARG_NULL  __attribute__((__sentinel__))
#else
#define LAST_ARG_NULL
#endif

/**
 * Indicates that no pointer arguments should
 * ever be passed NULL
 */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR >= 3)
#define NO_NULL_ARGS  __attribute__ ((__nonnull__))
#else
#define NO_NULL_ARGS
#endif

/**
 * Indicates that a function returns
 * a pointer to memory, the size of which is given in its @arg _arg'th argument.
 * Indicates that a function returns
 * a pointer to memory, which consists of a number of elements
 * given in its x'th argument, where each element has the size given in
 * the y'th argument
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 3)
#define LIKE_MALLOC(_arg)                   \
    UNALIASED                               \
    __attribute__((__alloc_size__(_arg)))
#define LIKE_CALLOC(_arg1, _arg2)                    \
    UNALIASED                                        \
    __attribute__((__alloc_size__(_arg1, _arg2)))
#else
#define LIKE_MALLOC(_arg) UNALIASED
#define LIKE_CALLOC(_arg1, _arg2) UNALIASED
#endif

/**
 * Indicates that a function returns
 * a pointer to memory, the alignment of which is given in its _x'th argument.
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define LIKE_MEMALIGN(_arg1, _arg2)                       \
    __attribute__((__alloc_align__(_arg1)))               \
    LIKE_MALLOC(_arg2)
#else
#define LIKE_MEMALIGN(_arg1, _arg2) LIKE_MALLOC(_arg2)
#endif

/**
 * Indicates that a function is printf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define LIKE_PRINTF(_x, _y)                             \
    __attribute__((__format__ (__printf__, _x, _y)))
#else
#define LIKE_PRINTF(_x, _y)
#endif

/**
 * Indicates that a function is scanf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define LIKE_SCANF(_x, _y)                          \
    __attribute__((__format__ (__scanf__, _x, _y)))
#else
#define LIKE_SCANF(_x, _y)
#endif

/**
 * Indicates that a function is strftime-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define LIKE_STRFTIME(_x)                               \
    __attribute__((__format__ (__strftime__, _x, 0)))
#else
#define LIKE_STRFTIME(_x)
#endif

/**
 * Indicates that a _x'th argument to the function is
 * a printf/scanf/strftime format string that is transformed in some way
 * and returned by the function
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define FORMAT_STRING(_x)                       \
    __attribute__((__format_arg__ (_x)))
#else
#define FORMAT_STRING(_x)
#endif


/**
 * Indicates that pointer arguments listed in
 * `_args` are never `NULL`
 */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR >= 3)
#define NOT_NULL_ARGS(...)                      \
    __attribute__ ((__nonnull__ (__VA_ARGS__)))
#else
#define NOT_NULL_ARGS(...)
#endif


/**
 * Global state may be read but not modified. In particular,
 * that means a function cannot produce any observable side effects
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 96)
#define NO_SIDE_EFFECTS __attribute__((__pure__))
#else
#define NO_SIDE_EFFECTS
#endif

/**
 * No global state is accessed by the function, and so the
 * function is genuinely function, its result depending solely on its
 * arguments
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define NO_SHARED_STATE  __attribute__((__const__))
#else
#define NO_SHARED_STATE
#endif


/**
 * Weak symbol (a library symbol that may be overriden
 * by the application
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
#define WEAK_LINKAGE __attribute__((__weak__))
#else
#define WEAK_LINKAGE
#endif

/** @def DLL_EXPORT_LINKAGE
 * A symbol is exported from a shared library
 */
/** @def DLL_IMPORT_LINKAGE
 * A symbol is imported from a shared library
 * For POSIX systems these two modes are void.
 */
#if __WIN32
#define DLL_EXPORT_LINKAGE __declspec(dllexport)
#define DLL_IMPORT_LINKAGE __declspec(dllimport)
#else
#define DLL_EXPORT_LINKAGE
#define DLL_IMPORT_LINKAGE
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
#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR >= 3)) && __ELF__
#define LOCAL_LINKAGE                           \
    __attribute__ ((__visibility__ ("hidden")))
#define INTERNAL_LINKAGE                            \
    __attribute__ ((__visibility__ ("internal")))
#else
#define LOCAL_LINKAGE
#define INTERNAL_LINKAGE
#endif


/**
 * The symbol should not be used and triggers a warning
 */
#if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#define DEPRECATED __attribute__((__deprecated__))
#else
#define DEPRECATED
#endif

/**
 * Marks a symbol as explicitly unused
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

/**
 * Marks a symbol as used
 */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define USED __attribute__((__used__))
#else
#define USED
#endif

/**@}*/

/** @name C99/C11 annotations
 * Compilers vary in their support for C99 and C11 features,
 * and the level of support may be altered by command-line switches
 */
/**@{*/

/**
 * Restricted pointers are supported by GCC even in non-C99 mode
 *  but with an alternative keyword `__restrict__`
 */
#if __STDC_VERSION__ < 199901L
#if __GNUC__
#define restrict __restrict__
#else
#define restrict
#endif
#endif

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
#define AT_LEAST(_x) static _x
#else
#define AT_LEAST(_x) _x
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
#define VAR_SIZE(_x) _x
#else
#define VAR_SIZE(_x)
#endif
    
/**
 * For C11 systems, it is defined in a new header
 * <stdnoreturn.h>. For GCC in non-C11 mode define it via noreturn
 * attribute
 */
#if __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#elif     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define noreturn __attribute__((__noreturn__))
#else
#define noreturn
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define CONSTRUCTOR static __attribute__((__constructor__))
#define DESTRUCTOR static __attribute__((__destructor__))
#else
#define CONSTRUCTOR constructors_are_not_supported
#define DESTRUCTOR constructors_are_not_supported
#endif

/**@}*/

#ifdef __cplusplus
}
#endif
#endif /* COMPILER_H */
