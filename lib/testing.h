/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */

/** @file
 *  Testing
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

enum tnt_exit_status {
    TNT_EXIT_OK = 0,
    TNT_EXIT_FAIL,
    TNT_EXIT_TRIVIAL = 64,
    TNT_EXIT_BAILOUT,
};

#define tnt_log(_msg, ...) (printf("# " _msg "\n", __VA_ARGS__));

extern bool tnt_mark_as_trivial;

#define tnt_trivial(_cond)                      \
    do {                                        \
        if (_cond)                              \
        {                                       \
            tnt_log("%s", #_cond);              \
            tnt_mark_as_trivial = true;         \
        }                                       \
    } while (0)

#define tnt_bailout(_msg)                       \
    do {                                        \
        printf("Bail out! %s\n", (_msg));       \
        _Exit(TNT_EXIT_BAILOUT);                \
    } while (0)

#define tnt_assertion(_cond, _msg, ...)         \
    do {                                        \
        if (!(_cond))                           \
        {                                       \
            tnt_log(_msg, __VA_ARGS__);         \
            exit(TNT_EXIT_FAIL);                \
        }                                       \
    } while (0)

#define tnt_assert(_cond)                                   \
    tnt_assertion(_cond, "Assertion failed: %s", #_cond)

#define tnt_assert_op(_type, _v1, _op, _v2)                             \
    do {                                                                \
        tnt_assert_op_type_##_type __v1 = (_v1);                        \
        tnt_assert_op_type_##_type __v2 = (_v2);                        \
        tnt_assertion((__v1) _op (__v2),                                \
                      "Assertion failed: %s"                            \
                      "(" tnt_assert_fmt_spec_##_type ") "              \
                      #_op " %s "                                       \
                      "(" tnt_assert_fmt_spec_##_type ")",              \
                      #_v1, __v1, #_v2, __v2);                          \
    } while (0)

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

#define tnt_assert_fop(_f, _v1, _op, _v2)                               \
    do {                                                                \
        tnt_assert_fop_type_##_f __v1 = (_v1);                          \
        tnt_assert_fop_type_##_f __v2 = (_v2);                          \
        tnt_assertion(_f(__v1, __v2) _op 0,                             \
                      "Assertion failed: %s %s"                         \
                      "(" tnt_assert_fmt_spec_f##_f ") " #_op " %s"     \
                      "(" tnt_assert_fmt_spec_f##_f ")",                \
                      #_f, #_v1, __v1, #_v2, __v2);                     \
    } while (0)

#define tnt_assert_fop_type_strcmp const char *

#define tnt_assert_fmt_spec_fstrcmp "%s"

#define tnt_assert_equiv(_cond1, _cond2)                                \
    do {                                                                \
        bool __v1 = (_cond1);                                           \
        bool __v2 = (_cond2);                                           \
        tnt_assertion(__v1 == __v2,                                     \
                      "Assertion failed: %s (%s) <=> %s (%s)",          \
                      #_cond1, __v1 ? "true" : "false",                 \
                      #_cond2, __v2 ? "true" : "false");                \
    } while (0)

typedef void (*tnt_test_fn)(unsigned i);

typedef struct tnt_test_descr {
    tnt_test_fn test;
    const char *descr;
    int expect_status;
    int expect_signal;
    bool expect_fail;
    bool iterated;
    struct tnt_test_descr *next;
} tnt_test_descr;

extern tnt_test_descr *tnt_test_descr_first;
extern tnt_test_descr **tnt_test_descr_last;

#define TNT_STRINGIFY0(_x) #_x
#define TNT_STRINGIFY(_x) TNT_STRINGIFY0(_x)

#define TESTDEFX(_name, _descr, _expect, _expectsig,                    \
                 _expectfail, _iterated)                                \
    static void _name(unsigned _i);                                     \
    static tnt_test_descr _name##_struct = {                            \
        .test = _name,                                                  \
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

#define TESTDEF(_name, _descr) TESTDEFX(_name, _descr, 0, 0, false, true)

#define TESTDEF_SINGLE(_name, _descr)               \
        TESTDEFX(_name, _descr, 0, 0, false, false)

#define TESTDEF_XFAIL(_name, _descr)                \
        TESTDEFX(_name, _descr, 0, 0, true, false)

#define TESTDEF_SIG(_name, _descr, _sig)                    \
    TESTDEFX(_name, _descr, 0, SIG ## _sig, false, false)

#define TESTDEF_EXIT(_name, _descr, _exit)              \
    TESTDEFX(_name, _descr, _exit, 0, false, false)

#define FORALL(_type, _var, ...)                \
        do {                                    \
            _type _var;                         \
            _type##_generate(_i, &_var);        \
            {__VA_ARGS__;};                     \
            _type##_cleanup(&(_var));           \
        } while (0)

#define PRODUCING(_type, _var, ...)                                     \
    do {                                                                \
        _type *_var = NULL;                                             \
        tn_buffer _var##_buffer = TN_BUFFER_INIT(TN_GLOC(_var), 0, 0);  \
        {__VA_ARGS__;};                                                 \
        tn_free(TN_GLOC(_var));                                         \
    } while (0);

#define TNT_TESTS_SCALE 100

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_TESTING_H */
