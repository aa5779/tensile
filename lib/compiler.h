/*
 * Copyright (c) 2017-2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
/** @file
 * Compiler support macros.
 * Compilers offer various extensions over strict C standard, and
 * also implement various versions of ISO C (sometimes, only partially).
 * This file offers compatibility wrappers around some of those extensions.
 */
#ifndef TNH_COMPILER_H
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


/**
 * Indicates that the function returns
 * a pointer to unaliased uninitalized memory.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_MALLOC) && !defined(__DOXYGEN__)
#define TN_RESULT_IS_UNALIASED __attribute__((__malloc__))
#else
#define TN_RESULT_IS_UNALIASED
#endif

/**
 * Indicates that a function returns a non-`NULL` pointer.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_RETURNS_NONNULL) && !defined(__DOXYGEN__)
#define TN_RESULT_IS_NOT_NULL __attribute__((__returns_nonnull__))
#else
#define TN_RESULT_IS_NOT_NULL
#endif

/**
 * Requires a warning if a result value
 * of the function is thrown away.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_WARN_UNUSED_RESULT) && !defined(__DOXYGEN__)
#define TN_RESULT_IS_IMPORTANT  __attribute__((__warn_unused_result__))
#else
#define TN_RESULT_IS_IMPORTANT
#endif

/**
 * Indicates that a vararg function shall have `NULL`
 * at the end of varargs.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_SENTINEL) && !defined(__DOXYGEN__)
#define TN_MUST_HAVE_SENTINEL  __attribute__((__sentinel__))
#else
#define TN_MUST_HAVE_SENTINEL
#endif

/**
 * Indicates that no pointer arguments should
 * ever be passed `NULL`.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_NONNULL) && !defined(__DOXYGEN__)
#define TN_NO_NULL_ARGS  __attribute__ ((__nonnull__))
#else
#define TN_NO_NULL_ARGS
#endif

#if defined(HAVE_FUNC_ATTRIBUTE_ALLOC_SIZE) && !defined(__DOXYGEN__)
#define TN_LIKE_MALLOC(_arg)                \
    TN_RESULT_IS_UNALIASED                  \
    TN_RESULT_IS_IMPORTANT                  \
    __attribute__((__alloc_size__(_arg)))

#define TN_LIKE_CALLOC(_arg1, _arg2)                 \
    TN_RESULT_IS_UNALIASED                           \
    TN_RESULT_IS_IMPORTANT                           \
    __attribute__((__alloc_size__(_arg1, _arg2)))
#else

/**
 * Indicates that a function returns
 * a pointer to memory, the size of which is given in its
 * @p _arg'th argument.
 */
#define TN_LIKE_MALLOC(_arg)                    \
    TN_RESULT_IS_UNALIASED \
    TN_RESULT_IS_IMPORTANT

/**
 * Indicates that a function returns
 * a pointer to memory, which consists of a number of elements
 * given in its @p _arg1'th argument, where each element has the size
 * given in the @p _arg2'th argument.
 */
#define TN_LIKE_CALLOC(_arg1, _arg2)            \
    TN_RESULT_IS_UNALIASED \
    TN_RESULT_IS_IMPORTANT
#endif

/**
 * Indicates that a function returns
 * a pointer to memory, the alignment of which is given in its @p _arg1'th
 * argument.
 */
#if (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)
#define TN_LIKE_MEMALIGN(_arg1, _arg2)                    \
    __attribute__((__alloc_align__(_arg1)))               \
    TN_LIKE_MALLOC(_arg2)
#else
#define TN_LIKE_MEMALIGN(_arg1, _arg2) TN_LIKE_MALLOC(_arg2)
#endif

#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT) && !defined(__DOXYGEN__)

#define TN_LIKE_PRINTF(_x, _y)                          \
    __attribute__((__format__ (__printf__, _x, _y)))
#define TN_LIKE_VPRINTF(_x) TN_LIKE_PRINTF(_x, 0)
#else

/**
 * Indicates that a function is printf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y.
 */
#define TN_LIKE_PRINTF(_x, _y)

/**
 * Indicates that a function is vprintf-like,
 * and the format string is in the _x'th argument.
 */
#define TN_LIKE_VPRINTF(_x)
#endif

#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT) && !defined(__DOXYGEN__)
#define TN_LIKE_SCANF(_x, _y)                       \
    __attribute__((__format__ (__scanf__, _x, _y)))
#define TN_LIKE_VSCANF(_x) TN_LIKE_SCANF(_x, 0)
#else
/**
 * Indicates that a function is scanf-like,
 * and the format string is in the _x'th argument, and
 * the arguments start at _y.
 */
#define TN_LIKE_SCANF(_x, _y)

/**
 * Indicates that a function is vscanf-like,
 * and the format string is in the _x'th argument.
 */
#define TN_LIKE_VSCANF(_x)
#endif

/**
 * Indicates that a function is strftime-like,
 * and the format string is in the _x'th argument.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT) && !defined(__DOXYGEN__)
#define TN_LIKE_STRFTIME(_x)                            \
    __attribute__((__format__ (__strftime__, _x, 0)))
#else
#define TN_LIKE_STRFTIME(_x)
#endif

/**
 * Indicates that a _x'th argument to the function is
 * a printf/scanf/strftime format string that is transformed in some way
 * and returned by the function.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_FORMAT_ARG) && !defined(__DOXYGEN__)
#define TN_HAS_FORMAT_ARG(_x)                   \
    __attribute__((__format_arg__ (_x)))
#else
#define TN_HAS_FORMAT_ARG(_x)
#endif


/**
 * Indicates that pointer arguments listed in
 * `_args` are never `NULL`.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_NONNULL) && !defined(__DOXYGEN__)
#define TN_NOT_NULL_ARGS(...)                   \
    __attribute__ ((__nonnull__ (__VA_ARGS__)))
#else
#define TN_NOT_NULL_ARGS(...)
#endif

/**
 * Global state may be read but not modified. In particular,
 * that means a function cannot produce any observable side effects.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_PURE) && !defined(__DOXYGEN__)
#define TN_NO_SIDE_EFFECTS TN_RESULT_IS_IMPORTANT __attribute__((__pure__))
#else
#define TN_NO_SIDE_EFFECTS TN_RESULT_IS_IMPORTANT
#endif

/**
 * No global state is accessed by the function, and so the
 * function is genuinely function, its result depending solely on its
 * arguments.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_CONST) && !defined(__DOXYGEN__)
#define TN_NO_SHARED_STATE TN_RESULT_IS_IMPORTANT __attribute__((__const__))
#else
#define TN_NO_SHARED_STATE TN_RESULT_IS_IMPORTANT
#endif


/**
 * Weak symbol (a library symbol that may be overriden
 * by the application.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) && !defined(__DOXYGEN__)
#define TN_WEAK  __attribute__((__weak__))
#else
#define TN_WEAK
#endif

/**
 * A symbol is exported from a shared library.
 * It has no effect on POSIX systems.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT) && !defined(__DOXYGEN__)
#define TN_DLL_EXPORT  __declspec(dllexport)
#else
#define TN_DLL_EXPORT
#endif

/**
 * A symbol is imported from a shared library.
 * It has no effect on POSIX systems.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_DLLIMPORT) && !defined(__DOXYGEN__)
#define TN_DLL_IMPORT __declspec(dllimport)
#else
#define TN_DLL_IMPORT
#endif

#if defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY) && !defined(__DOXYGEN__)
#define TN_LOCAL                                \
    __attribute__ ((__visibility__ ("hidden")))
#define TN_INTERNAL                                 \
    __attribute__ ((__visibility__ ("internal")))
#else
/**
 * The symbol is *not* exported from a shared library
 * (but unlike static symbols, it is visible to all compilation units
 * comprising the library itself).
 */
#define TN_LOCAL

/**
 * Internal symbols are like local. In addition they
 * cannot be accessed by othermodules even indirectly
 * (e.g. through a function pointer).
 */
#define TN_INTERNAL
#endif


/**
 * The symbol should not be used and triggers a warning.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_DEPRECATED) && !defined(__DOXYGEN__)
#define TN_DEPRECATED __attribute__((__deprecated__))
#else
#define TN_DEPRECATED
#endif

/**
 * Marks a symbol as explicitly unused.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_UNUSED) && !defined(__DOXYGEN__)
#define TN_UNUSED  __attribute__((__unused__))
#else
#define TN_UNUSED
#endif

/**
 * Marks a symbol as used.
 */
#if defined(HAVE_FUNC_ATTRIBUTE_USED) && !defined(__DOXYGEN__)
#define TN_USED __attribute__((__used__))
#else
#define TN_USED
#endif

#if defined(__DOXYGEN__)
/**
 * Specifies a function as a constructor,
 * that is, a function that gets called before the main entry.
 *
 * Using constructors allows for complex initialisations for static
 * objects.
 */
#define TN_CONSTRUCTOR
#elif defined(HAVE_FUNC_ATTRIBUTE_CONSTRUCTOR)
#define TN_CONSTRUCTOR static void __attribute__((__constructor__))
#else
#define TN_CONSTRUCTOR constructors_are_not_supported
#endif

#if defined(__DOXYGEN__)
/**
 * Specifies a function as a destructor,
 * that is, a function that gets called after the main entry.
 *
 * Using destructors allows for complex finalisations for static
 * objects.
 */
#define TN_DESTRUCTOR
#elif defined(HAVE_FUNC_ATTRIBUTE_DESTRUCTOR)
#define TN_DESTRUCTOR static void __attribute__((__destructor__))
#else
#define TN_DESTRUCTOR destructors_are_not_supported
#endif

/**
 * Declares a variable _var of type _type and
 * attaches a block of code _code to initialise it
 * at startup.
 */
#define TN_GLOBAL_INIT(_type, _var, _code)      \
    _type _var;                                 \
    TN_CONSTRUCTOR _var##_init(void)            \
    {                                           \
        _code;                                  \
    }                                           \
    struct fake

#if defined(HAVE___BUILTIN_EXPECT) || defined(__DOXYGEN__)

/**
 * Informs the compiler that a condition _x is likely
 * to be held.
 */
#define TN_LIKELY(_x) __builtin_expect(!!(_x), 1)

/**
 * Informs the compiler that a condition _x is unlikely
 * to be held.
 */
#define TN_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#else
#define TN_LIKELY(_x) (_x)
#define TN_UNLIKELY(_x) (_x)
#endif


/** @defgroup annotations C99/C11 annotations
 * Compilers vary in their support for C99 and C11 features,
 * and the level of support may be altered by command-line switches.
 * @{
 */

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
#if (__STDC_VERSION__ >= 199901L && !defined(__STDC_NO_VLA__)) ||   \
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
#elif  HAVE_FUNC_ATTRIBUTE_NORETURN && !defined(__DOXYGEN__)
#define noreturn __attribute__((__noreturn__))
#else
#define noreturn
#endif

#if __STDC_VERSION__ >= 201112L
#include <stdalign.h>
#else
#if defined(__DOXYGEN__)
/**
 * In C11, this construct is defined in <stdalign.h> allow to
 * query the object's alignment.
 *
 * GCC has supported __alignof__ for ages, and alignas can be
 * simulated by a the aligned attribute + __alignof__
 */
#define alignas(_x)
#elif HAVE_VAR_ATTRIBUTE_ALIGNED
#define alignas(_n) __attribute__((__aligned__ (_n)))
#else
#define alignas(_x) alignas_not_supported
#endif
#if defined(__DOXYGEN__)
/**
 * In C11, this construct is defined in <stdalign.h> allow to
 * to request an object  * to be aligned as some other alignment.
 *
 * GCC has supported __alignof__ for ages.
 */
#define alignof(_x) (0)
#elif defined(__GNUC__)
#define alignof(_x) __alignof__(_x)
#else
#define alignof(_x) alignof_not_supported
#endif
#endif

/**@}*/

#ifdef __cplusplus
}
#endif
#endif /* TNH_COMPILER_H */
