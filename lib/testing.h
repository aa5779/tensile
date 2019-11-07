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

#define tnt_log(_msg, ...) (printf("#" _msg "\n", __VA_ARGS__));

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
    (tnt_assertion(_cond, "Assertion failed: %s", #_cond))

#define tnt_assert_op(_type, _v1, _op, _v2)                             \
    (tnt_assertion((_v1) _op (_v2),                                     \
                   "Assertion failed: " #_v1                            \
                   "(" tnt_assert_fmt_type_ ## _type ") "               \
                   #_op " " #_v2                                        \
                   "(" tnt_assert_fmt_type_ ## _type ")", \
                   _v1, _v2))

#define tnt_assert_fmt_type_char "%c"
#define tnt_assert_fmt_type_int "%d"
#define tnt_assert_fmt_type_unsigned "%u"
#define tnt_assert_fmt_type_int64_t "%" PRId64
#define tnt_assert_fmt_type_uint64_t "%" PRIu64
#define tnt_assert_fmt_type_size_t "%zu"
#define tnt_assert_fmt_type_ptr "%p"
#define tnt_assert_fmt_type_double "%g"
#define tnt_assert_fmt_type_byte "%02x"
#define tnt_assert_fmt_type_hword "%04x"
#define tnt_assert_fmt_type_word "%08x"
#define tnt_assert_fmt_type_dword "%016" PRIx64

#define tnt_assert_strop( _v1, _op, _v2)                                \
    (tnt_assertion(strcmp((_v1), (_v2)) _op 0,                          \
                   "Assertion failed: " #_v1                            \
                   "(%s) " #_op " " #_v2 "(%s)",                        \
                   _v1, _v2))

typedef void (*tnt_test_fn)(unsigned i);

typedef struct tnt_test_descr {
    tnt_test_fn test;
    const char *descr;
    int expect_status;
    int expect_signal;
    bool expect_fail;
    bool iterated;
    struct tnt_test_descr
} tnt_test_descr;

extern tnt_test_descr *tnt_test_descr_first;
extern tnt_test_descr **tnt_test_descr_last;

#define TESTDEFX(_name, _descr, _expect, _expectsig, _expectfail, _iterated) \
    static void _name(unsigned _i);                                     \
    static tnt_test_descr _name##_struct = {                            \
        .test = _name,                                                  \
        .descr = _descr,                                                \
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

#define FORALL(_type, _var, _generator, _code)  \
        do {                                    \
            _type _var;                         \
            _generator##_generate(_i, &_var);   \
            _code;                              \
            _generator##_cleanup(&_var);        \
        } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TNH_TESTING_H */
