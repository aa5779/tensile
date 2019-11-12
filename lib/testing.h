/** @file
 *  Testing
 * @author Artem V. Andreev
 * @copyright
 * @parblock
 * &copy; 2019 Artem V. Andreev
 *
 * @license{MIT}
 * @endparblock
 */
#ifndef TNH_TESTING_H
#define TNH_TESTING_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "compiler.h"

/**
 * @undocumented
 */
enum tnt_exit_status {
    TNT_EXIT_OK = 0,
    TNT_EXIT_FAIL,
    TNT_EXIT_TRIVIAL = 64,
    TNT_EXIT_BAILOUT,
};

/**
 * @undocumented
 */
#define tnt_log(_msg, ...) (printf("# " _msg "\n", __VA_ARGS__));

/**
 * @undocumented
 */
extern bool tnt_mark_as_trivial;

/**
 * @undocumented
 */
#define tnt_trivial(_cond)                      \
    do {                                        \
        if (TN_UNLIKELY(_cond))                 \
        {                                       \
            tnt_log("%s", #_cond);              \
            tnt_mark_as_trivial = true;         \
        }                                       \
    } while (0)

/**
 * @undocumented
 */
#define tnt_bailout(_msg)                       \
    do {                                        \
        printf("Bail out! %s\n", (_msg));       \
        _Exit(TNT_EXIT_BAILOUT);                \
    } while (0)

/**
 * @undocumented
 */
#define tnt_assertion(_cond, _msg, ...)         \
    do {                                        \
        if (!(_cond))                           \
        {                                       \
            tnt_log(_msg, __VA_ARGS__);         \
            _Exit(TNT_EXIT_FAIL);                \
        }                                       \
    } while (0)

/**
 * @undocumented
 */
#define tnt_assert(_cond)                                               \
    tnt_assertion(_cond, "Assertion failed: %s @ %d", #_cond, __LINE__)

/**
 * @undocumented
 */
#define tnt_assert_op(_type, _v1, _op, _v2)                             \
    do {                                                                \
        tnt_assert_op_type_##_type __v1 = (_v1);                        \
        tnt_assert_op_type_##_type __v2 = (_v2);                        \
        tnt_assertion((__v1) _op (__v2),                                \
                      "Assertion failed: %s"                            \
                      "(" tnt_assert_fmt_spec_##_type ") "              \
                      #_op " %s "                                       \
                      "(" tnt_assert_fmt_spec_##_type ") @ %d",         \
                      #_v1, __v1, #_v2, __v2, __LINE__);                \
    } while (0)

#ifndef __DOXYGEN__
#define tnt_assert_op_type_char char
#define tnt_assert_op_type_int int
#define tnt_assert_op_type_unsigned unsigned
#define tnt_assert_op_type_int64_t int64_t
#define tnt_assert_op_type_uint64_t uint64_t
#define tnt_assert_op_type_size_t size_t
#define tnt_assert_op_type_ptr const void *
#define tnt_assert_op_type_double double
#define tnt_assert_op_type_byte uint8_t
#define tnt_assert_op_type_hword uint16_t
#define tnt_assert_op_type_word uint32_t
#define tnt_assert_op_type_dword uint64_t
#define tnt_assert_op_type_ucs4_t ucs4_t

#define tnt_assert_fmt_spec_char "%c"
#define tnt_assert_fmt_spec_int "%d"
#define tnt_assert_fmt_spec_unsigned "%u"
#define tnt_assert_fmt_spec_int64_t "%" PRId64
#define tnt_assert_fmt_spec_uint64_t "%" PRIu64
#define tnt_assert_fmt_spec_size_t "%zu"
#define tnt_assert_fmt_spec_ptr "%p"
#define tnt_assert_fmt_spec_double "%g"
#define tnt_assert_fmt_spec_byte "%02x"
#define tnt_assert_fmt_spec_hword "%04x"
#define tnt_assert_fmt_spec_word "%08x"
#define tnt_assert_fmt_spec_dword "%016" PRIx64
#define tnt_assert_fmt_spec_ucs4_t "U+%08x"
#endif

/**
 * @undocumented
 */
#define tnt_assert_fop(_f, _v1, _op, _v2)                               \
    do {                                                                \
        tnt_assert_fop_type_##_f __v1 = (_v1);                          \
        tnt_assert_fop_type_##_f __v2 = (_v2);                          \
        tnt_assertion(_f(__v1, __v2) _op 0,                             \
                      "Assertion failed: %s %s"                         \
                      "(" tnt_assert_fmt_spec_f##_f ") " #_op " %s"     \
                      "(" tnt_assert_fmt_spec_f##_f ") @ %d",           \
                      #_f, #_v1, __v1, #_v2, __v2, __LINE__);           \
    } while (0)

#ifndef __DOXYGEN__
#define tnt_assert_fop_type_strcmp const char *

#define tnt_assert_fmt_spec_fstrcmp "%s"
#endif

/**
 * @undocumented
 */
#define tnt_assert_equiv(_cond1, _cond2)                                \
    do {                                                                \
        bool __v1 = (_cond1);                                           \
        bool __v2 = (_cond2);                                           \
        tnt_assertion(__v1 == __v2,                                     \
                      "Assertion failed: %s (%s) <=> %s (%s) @ %d",     \
                      #_cond1, __v1 ? "true" : "false",                 \
                      #_cond2, __v2 ? "true" : "false", __LINE__);      \
    } while (0)

/**
 * @undocumented
 */
extern void tnt_compare_mem(size_t len1, const void *mem1,
                            size_t len2, const void *mem2,
                            const char *msg, int line);

/**
 * @undocumented
 */
#define tnt_assert_mem(_len1, _mem1, _len2, _mem2)                      \
    (tnt_compare_mem((_len1), (_mem1), (_len2), (_mem2),                \
                     #_len1 ":" #_mem1 " == " #_len2 ":" #_mem2,        \
                     __LINE__))

/**
 * @undocumented
 */
typedef void (*tnt_test_fn)(unsigned i);

/**
 * @undocumented
 */
typedef void (*tnt_cleanup_fn)(void);

/**
 * @undocumented
 */
typedef struct tnt_test_descr {
    tnt_test_fn test; /**< @undocumented */
    tnt_cleanup_fn cleanup; /**< @undocumented */
    const char *descr; /**< @undocumented */
    int expect_status; /**< @undocumented */
    int expect_signal; /**< @undocumented */
    bool expect_fail; /**< @undocumented */
    bool iterated; /**< @undocumented */
    struct tnt_test_descr *next; /**< @undocumented */
} tnt_test_descr;

/** @cond PRIVATE */

/**
 * @undocumented
 */
extern tnt_test_descr *tnt_test_descr_first;

/**
 * @undocumented
 */
extern tnt_test_descr **tnt_test_descr_last;

/**
 * @undocumented
 */
#define TNT_STRINGIFY0(_x) #_x

/**
 * @undocumented
 */
#define TNT_STRINGIFY(_x) TNT_STRINGIFY0(_x)

/** @endcond */

/**
 * @undocumented
 */
#define TESTDEFX(_name, _descr, _cleanup, _expect, _expectsig,          \
                 _expectfail, _iterated)                                \
    static void _name(unsigned _i);                                     \
    static tnt_test_descr _name##_struct = {                            \
        .test = _name,                                                  \
        .cleanup = _cleanup,                                            \
        .descr = _descr " @ " TNT_STRINGIFY(__LINE__),                  \
        .expect_status = _expect,                                       \
        .expect_signal = _expectsig,                                    \
        .expect_fail = _expectfail,                                     \
        .iterated = _iterated                                           \
    };                                                                  \
                                                                        \
    TN_CONSTRUCTOR _name##_struct_init(void)                            \
    {                                                                   \
        *tnt_test_descr_last = &_name##_struct;                         \
        tnt_test_descr_last = &_name##_struct.next;                     \
    }                                                                   \
                                                                        \
    static void _name(TN_UNUSED unsigned _i)

/**
 * @undocumented
 */
#define TESTDEF(_name, _descr)                              \
        TESTDEFX(_name, _descr, NULL, 0, 0, false, true)

/**
 * @undocumented
 */
#define TESTDEF_SINGLE(_name, _descr)                       \
        TESTDEFX(_name, _descr, NULL, 0, 0, false, false)

/**
 * @undocumented
 */
#define TESTDEF_CLEANUP(_name, _descr, _cleanup)            \
        TESTDEFX(_name, _descr, _cleanup, 0, 0, false, true)

/**
 * @undocumented
 */
#define TESTDEF_SINGLE_CLEANUP(_name, _descr, _cleanup)     \
        TESTDEFX(_name, _descr, _cleanup, 0, 0, false, false)

/**
 * @undocumented
 */
#define TESTDEF_XFAIL(_name, _descr)                        \
        TESTDEFX(_name, _descr, NULL, 0, 0, true, false)

/**
 * @undocumented
 */
#define TESTDEF_SIG(_name, _descr, _sig)                        \
    TESTDEFX(_name, _descr, NULL, 0, SIG ## _sig, false, false)

/**
 * @undocumented
 */
#define TESTDEF_EXIT(_name, _descr, _exit)                  \
    TESTDEFX(_name, _descr, NULL, _exit, 0, false, false)

/**
 * @undocumented
 */
#define FORALL(_type, _var, ...)                \
        do {                                    \
            _type _var;                         \
            _type##_generate(_i, &_var);        \
            {__VA_ARGS__;};                     \
            _type##_cleanup(&(_var));           \
        } while (0)

/**
 * @undocumented
 */
#define PRODUCING(_type, _var, ...)                                     \
    do {                                                                \
        _type *_var = NULL;                                             \
        tn_buffer _var##_buffer = TN_BUFFER_INIT(TN_GLOC(_var), 0, 0);  \
        {__VA_ARGS__;};                                                 \
        tn_free(TN_GLOC(_var));                                         \
    } while (0);

/**
 * @undocumented
 */
#define TNT_TESTS_SCALE 100

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_TESTING_H */
