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
 * @brief support for QuickCheck style invariant testing
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef CHECK_H
#define CHECK_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#if DO_TESTS

#include <limits.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include "utils.h"

static size_t test_max_samples = 100;
static size_t test_sample_scale = 10;

typedef union test_value_t {
    bool        b;
    uintmax_t   u;
    intmax_t    i;
    size_t      sz;
    wchar_t     ch;
    double      d;
    const char *s;
    void       *p;
} test_value_t;

enum test_value_class_t {
    TESTVAL_NONE = -1,
    TESTVAL_TRIVIAL,
    TESTVAL_BOUNDARY,
    TESTVAL_REGULAR
};

typedef NO_NULL_ARGS MUST_USE enum test_value_class_t (*test_enumerator_t)(
        unsigned i,
        test_value_t *dest);

typedef void (*test_logger_t)(testval_t v);

typedef NO_SIDE_EFFECTS bool (*test_comparator_t)(test_value_t v1,
                                                  test_value_t v2);

typedef struct test_generator_t {
    test_enumerator_t enumerate;
    test_logger_t     log;
    test_comparator_t compare;
    bool              has_boundary;
} test_generator_t;

#define DEFINE_GENERATOR_RECORD(_name, _enumname, _logname, _comparename, \
                                _hasboundary)                           \
    static UNUSED const test_generator_t test_every_##_name = {         \
        .enumerate    = test_enum_##_enumname,                          \
        .log          = test_log_##_logname,                            \
        .compare      = test_compare_##_comparename,                    \
        .has_boundary = _hasboundary                                    \
    }

#define DEFINE_TRIVIAL_COMPARE(_name, _tv)                            \
    static NO_SHARED_STATE                                            \
    bool test_compare_##_name(test_value_t v1, test_value_t v2)       \
    {                                                                 \
        return v1._tv == v2._tv;                                      \
    }                                                                 \
    struct fake

#define DEFINE_SIMPLE_GENERATOR(_name, _type, _tv, _fmt, _hasboundary)  \
    static void test_log_##_name(test_value_t v)                        \
    {                                                                   \
        fprintf(stderr, _fmt, (_type)v._tv);                            \
    }                                                                   \
                                                                        \
    DEFINE_TRIVIAL_COMPARE(_name, _tv);                                 \
    DEFINE_GENERATOR_RECORD(_name, _name, _name, _name, _hasboundary)

#define DEFINE_DERIVED_GENERATOR(_name, _basename, _hasboundary)        \
    DEFINE_GENERATOR_RECORD(_name, _name, _basename, _basename, _hasboundary)

static MUST_USE
bool test_compare_values(const test_generator_t *gen,
                         test_value_t v1,
                         test_value_t v2)
{
    if (!gen->compare(v1, v2))
    {
        fputs("Expected ", stderr);
        gen->log(v1);
        fputs(" but got ", stderr);
        gen->log(v2);
        fputs("\n", stderr);
        return false;
    }
    return true;
}

static inline NO_NULL_ARGS
void test_make_random_bytes(void *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)buf)[i] = (uint8_t)rand();
}

static inline NO_NULL_ARGS
void test_print_hex(void *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        fprintf(stderr, "%s%02x", i == 0 ? "" : ":", 
    }
}

static inline NO_NULL_ARGS
void test_print_chars(const char *s)
{
    for (size_t i = 0; s[i] != '\0'; i++)
    {
        if (isprint(s[i]))
            fputc(s[i], stderr);
        else
            fprintf(stderr, "\\x%02x", s[i]);
    }
}
    
#define DEFINE_INTEGRAL_ENUMERATOR(_name, _type, _tv, _min, _max)   \
    static MUST_USE NO_NULL_ARGS                                    \
    enum test_value_class_t                                         \
    test_enum_##_name(unsigned i, test_value_t *dest)               \
    {                                                               \
        switch (i)                                                  \
        {                                                           \
            case 0:                                                 \
                dest->_tv = _min;                                   \
                break;                                              \
            case 1:                                                 \
                dest->_tv = _max;                                   \
                break;                                              \
            default:                                                \
            {                                                       \
                _type r;                                            \
                                                                    \
                test_make_random_bytes(&r, sizeof(r));              \
                dest->_tv = r;                                      \
                break;                                              \
            }                                                       \
        }                                                           \
        return dest->_tv == _min || dest->_tv == _max ?             \
            TESTVAL_BOUNDARY : TESTVAL_REGULAR;                     \
    }                                                               \
    struct fake

#define DEFINE_SEQ_ENUMERATOR(_name, _type, _tv, ...)                   \
    static MUST_USE NO_NULL_ARGS                                        \
    enum test_value_class_t                                             \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        static const _type vals[] = {__VA_ARGS__};                      \
        static const size_t n_vals = sizeof(vals) / sizeof(*vals);      \
                                                                        \
        if (i >= n_vals)                                                \
            return TESTVAL_NONE;                                        \
                                                                        \
        dest->_tv = vals[i];                                            \
        return TESTVAL_REGULAR;                                         \
    }                                                                   \
    struct fake

#define DEFINE_SCALED_ENUMERATOR(_name, _type, _tv, _min)               \
    static MUST_USE NO_NULL_ARGS                                        \
    enum test_value_class_t                                             \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        if (i >= test_sample_scale)                                     \
            return TESTVAL_;                                            \
                                                                        \
        dest->_tv = _min * test_sample_scale / 2 + (_type)i;            \
                                                                        \
        return TESTVAL_REGULAR;                                         \
    }                                                                   \
    struct fake

#define DEFINE_RANGE_ENUMERATOR(_name, _type, _tv, ...)                 \
    static MUST_USE NO_NULL_ARGS                                        \
    enum test_value_class_t                                             \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        static const struct {                                           \
            _type min;                                                  \
            _type max;                                                  \
        } ranges[] = {__VA_ARGS__};                                     \
        static const size_t n_ranges = sizeof(ranges) / sizeof(*ranges); \
        size_t r = (size_t)(rand() % n_ranges);                         \
        _type v = (_type)(rand() % (ranges[r].max - ranges[r].min + 1) + \
                          ranges[r].min);                               \
        dest->_tv = _v;                                                 \
        return TESTVAL_REGULAR;                                         \
    }                                                                   \
    struct fake

#define DEFINE_CHOICE_ENUMERATOR(_name, _option1, _option2)             \
    static MUST_USE NO_NULL_ARGS                                        \
    enum test_value_class_t                                             \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        enum test_value_class_t result;                                 \
                                                                        \
        if (rand() % 2)                                                 \
            return test_enum_##_option2(i, dest);                       \
                                                                        \
        result = test_enum_##_option1(i, dest);                         \
        return result != TESTVAL_NONE ? result :                        \
            test_enum_##_option2(i, dest);                              \
    }                                                                   \
    struct fake

static MUST USE NO_NULL_ARGS
enum test_value_class_t
test_string_enumerate(unsigned i, test_value_t *dest,
                      test_enumerator_t elt_enum)
{
    size_t sz = i == 0 ? 0 : rand() % test_sample_scale;
    char *s = tn_alloc_blob(sz + 1);
    size_t j;

    for (j = 0; j < sz; j++)
    {
        test_value_t elt;
        if (elt_enum(i, &elt) == TESTVAL_NONE)
            return TESTVAL_NONE;
        s[j] = (char)elt.ch;
    }
    s[sz - 1] = '\0';
    dest->s = s;
    return sz == 0 ? TESTVAL_TRIVIAL : TESTVAL_REGULAR;
}

#define DEFINE_STRING_ENUMERATOR(_name, _eltname)                       \
    static MUST USE NO_NULL_ARGS                                        \
    enum test_value_class_t                                             \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        return test_string_enumerate(i, dest, test_enum_##_eltname);    \
    }                                                                   \
    struct fake

#define DEFINE_STRING_GENERATOR(_name)                                  \
    static void test_log_##_name(test_value_t v)                        \
    {                                                                   \
        fputc('\"', stderr);                                            \
        test_print_chars(v.s);                                          \
        fputc('\"', stderr);                                            \
    }                                                                   \
                                                                        \
    static NO_SHARED_STATE                                              \
    bool test_compare_##_name(test_value_t v1, test_value_t v2)         \
    {                                                                   \
        return strcmp(v1.s, v2.s) == 0;                                 \
    }                                                                   \
    DEFINE_GENERATOR_RECORD(_name, _name, _name, _name, false)


typedef NO_NULL_ARGS void test_property_t(const test_value_t vals[]);

typedef struct testcase_t {
    const char                     *title;
    bool                            enabled;
    bool                            expect_fail;
    const test_generator_t * const *params;
    test_property_t                 property;
    const struct testcase_t        *chain;
} testcase_t;

static inline NO_NULL_ARGS MUST_USE NO_SIDE_EFFECTS
unsigned testcase_count_params(const testcase_t *tc)
{
    unsigned i;

    for(i = 0; tc->params[i] != NULL; i++)
        ;

    return i;
}

static unsigned testcase_n_run;
static unsigned testcase_n_ok;

static inline NO_SIDE_EFFECTS MUST_USE
unsigned testcase_n_samples_per_param(unsigned n)
{
    unsigned n_samples = test_max_samples;

    for (; n > 1; n--)
    {
        n_samples /= test_sample_scale;
        if (n_samples == 0)
            return 1;
    }

    return n_samples;
}

typedef struct testcase_param_t {
    unsigned     index;
    test_value_t value;
    bool         observed_regular;
    bool         observed_boundary;
} testcase_param_t;

static NO_NULL_ARGS MUST_USE
bool testcase_fill_param(testcase_param_t *param,
                         test_enumerator_t enumerate,
                         unsigned limit)
{
    test_value_class_t cls;

    if (param->index == limit)
        return false;

    cls = enumerate(param->index, &param->value);
    if (cls == TESTVAL_NONE)
        return false;

    if (cls == TESTVAL_BOUNDARY)
        param->observed_boundary = true;
    if (cls == TESTVAL_REGULAR)
        param->observed_regular = true;
    param->index++;

    return true;
}

static NO_NULL_ARGS MUST_USE
bool testcase_step_all_params(testcase_param_t *params,
                              const test_enumerator_t *enums,
                              unsigned n_params,
                              unsigned limit)
{
    unsigned i;

    for (i = n_params; i > 0; i--)
    {
        if (testcase_fill_param(params[i - 1], enums[i - 1], limit))
            return true;
    }
    if (i == 0)
        return false;
    for (; i < n_params; i++)
    {
        params[i].index = 0;
        if (!testcase_fill_param(params[i], enums[i], limit))
            return false;
    }

    return true;
}

static NO_NULL_ARGS MUST_USE
bool testcase_run_variation(const testcase_t *tc,
                            unsigned n_params,
                            const testcase_param_t *params)
{
    pid_t child;
    int status;

    child = fork();
    if (child == (pid_t)(-1))
    {
        perror("fork");
        abort();
    }
    if (child == 0)
    {
        test_value_t values[n_params];
        unsigned i;

        for (i = 0; i < n_params; i++)
            values[i] = params[i].value;

        tc->property(values);
        exit(0);
    }

    if (waitpid(child, &status, 0) != child)
    {
        perror("waitpid");
        abort();
    }

    return tc->expect_fail ?
        WIFSIGNALED(status) && WTERMSIG(status) == SIGTRAP :
        WIFEXITED(status) && WEXITSTATUS(status) == 0
}

static NO_NULL_ARGS
void testcase_run(const testcase_t *tc)
{
    unsigned n_params = testcase_count_params(tc);
    testcase_param_t params[n_params];
    unsigned i;
    unsigned p;
    unsigned n_samples = testcase_n_samples_per_param(n_params);

    memset(params, 0, sizeof(params));

    fprintf(stderr, "%s...", tc->title);
    fflush(stderr);
    if (!tc->enabled)
    {
        fputs("SKIP\n");
        return;
    }
    if (tc->property == NULL)
    {
        fputs("UNTESTED\n");
        return;
    }
    for (p = 0; p < n_params; p++)
    {
        if (!test_fill_param(&params[p], tc->params[p], n_samples))
        {
            fputs("UNTESTED\n");
            return;
        }
    }

    testcase_n_run++;

    for (i = 0; i < test_max_samples; i++)
    {
        fputc('[', stderr);
        for (p = 0; p < n_params; p++)
        {
            if (p > 0)
                fputc(',', stderr);
            tc->params->log(&params[p].value);
        }
        fputc(']', stderr);
        fflush(stderr);

        if (!testcase_run_variation(tc, n_params, params))
        {
            fputs("FAIL\n", stderr);
            return;
        }

        if (i != test_max_samples - 1)
        {
            if (!testcase_step_all_params(params, tc->params,
                                          n_params, n_samples))
                break;
        }
    }
    testcase_n_ok++;
    fputs("OK\n", stderr);

    for (p = 0; p < n_params; p++)
    {
        if (tc->has_boundary && !params[p].observed_boundary)
            fprintf(stderr, "\tWarning: no boundary values for parameter %u tested\n", p);
        if (!tc->has_boundary && params[p].observed_boundary)
            fprintf(stderr, "\tWarning: some values unexpectedly marked as boundary for parameter %u \n", p);
        if (!params[p].observed_regular)
            fprintf(stderr, "\tWarning: no regular values for parameter %u tested\n", p);
    }
}

#define ASSERT(_expr)                                                   \
    do                                                                  \
    {                                                                   \
        if (!(_expr))                                                   \
        {                                                               \
            fprintf(stderr, "%s:%d: Assertion " #_expr                  \
                    " failed in %s()\n",                                \
                    __FILE__, __LINE__, __FUNCTION__);                  \
            raise(SIGTRAP);                                             \
        }                                                               \
    } while (0)

#define INVARIANT(_expr) ASSERT(_expr)

#define EXPECT(_gen, _v1, _v2)                                  \
    do                                                          \
    {                                                           \
        if (!test_compare_values(&test_every_##_gen, _v1, _v2)) \
            raise(SIGTRAP);                                     \
    } while (0)

#define TESTVAL(_tv, _v) ((test_value_t){._tv = (_v)})

static const testcase_t *test_case_first;
static const testcase_t **test_case_last = &test_case_first;

#define TESTCASE(_name, _title, _enabled, _expect_fail, _values,        \
                 _body, ...)                                            \
    static void test_property_##_name(const test_value_t _values[])     \
    {                                                                   \
        _body;                                                          \
    }                                                                   \
                                                                        \
    static const testcase_t test_case_##_name =                         \
    {                                                                   \
        .title       = _title,                                          \
        .enabled     = _enabled,                                        \
        .expect_fail = _expect_fail,                                    \
        .params      = (const test_generator_t *[]){                    \
            __VA_ARGS__,                                                \
            NULL                                                        \
        },                                                              \
        .property    = test_property_##_name,                           \
    };                                                                  \
                                                                        \
    static CONSTRUCTOR                                                  \
    void test_case_##_name##_init(void)                                 \
    {                                                                   \
        *test_case_last = &test_case_##_name;                           \
        test_case_last  = &test_case_##_name.chain;                     \
    }                                                                   \
                                                                        \
    struct fake


DEFINE_INTEGRAL_ENUMERATOR(int, int, i, INT_MIN, INT_MAX);
DEFINE_SIMPLE_GENERATOR(int, int, i, "%d", true);

DEFINE_INTEGRAL_ENUMERATOR(unsigned, unsigned, u, 0, UINT_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned, unsigned, "%u", true);

DEFINE_INTEGRAL_ENUMERATOR(short, short, i, SHRT_MIN, SHRT_MAX);
DEFINE_SIMPLE_GENERATOR(short, short, i, "%hd", true);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_short, unsigned short, u, 0, USHRT_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_short, unsigned short, u, "%hu", true);

DEFINE_INTEGRAL_ENUMERATOR(long, long, i, LONG_MIN, LONG_MAX);
DEFINE_SIMPLE_GENERATOR(long, long, i, "%ld", true);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_long, unsigned long, u, 0, ULONG_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_long, unsigned long, u, "%lu", true);

DEFINE_INTEGRAL_ENUMERATOR(long_long, long long, i, LLONG_MIN, LLONG_MAX);
DEFINE_SIMPLE_GENERATOR(long_long, long long, i, "%lld", true);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_long_long, unsigned long long,
                           u, 0, ULLONG_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_long_long, unsigned long long, i,
                        "%llu", true);

DEFINE_INTEGRAL_ENUMERATOR(intmax_t, intmax_t, i, INTMAX_MIN, INTMAX_MAX);
DEFINE_SIMPLE_GENERATOR(intmax_t, intmax_t, i, "%" PRIdMAX, true);

DEFINE_INTEGRAL_ENUMERATOR(uintmax_t, uintmax_t, u, 0, UINTMAX_MAX);
DEFINE_SIMPLE_GENERATOR(uintmax_t, uintmax_t, u, "%" PRIuMAX, true);

DEFINE_INTEGRAL_ENUMERATOR(int8_t, int8_t, i, INT8_MIN, INT8_MAX);
DEFINE_SIMPLE_GENERATOR(int8_t, int8_t, i, "%" PRId8, true);

DEFINE_INTEGRAL_ENUMERATOR(uint8_t, uint8_t, u, 0, UINT8_MAX);
DEFINE_SIMPLE_GENERATOR(uint8_t, uint8_t, u, "%" PRIu8, true);

DEFINE_INTEGRAL_ENUMERATOR(byte, uint8_t, u, 0, UINT8_MAX);
DEFINE_SIMPLE_GENERATOR(byte, uint8_t, u, "%02" PRIx8, true);

DEFINE_INTEGRAL_ENUMERATOR(int16_t, int16_t, i, INT16_MIN, INT16_MAX);
DEFINE_SIMPLE_GENERATOR(int16_t, int16_t, i, "%" PRId16, true);

DEFINE_INTEGRAL_ENUMERATOR(uint16_t, uint16_t, u, 0, UINT16_MAX);
DEFINE_SIMPLE_GENERATOR(uint16_t, uint16_t, u, "%" PRIu16, true);

DEFINE_INTEGRAL_ENUMERATOR(word, uint16_t, u, 0, UINT16_MAX);
DEFINE_SIMPLE_GENERATOR(word, uint16_t, u, "%04" PRIx16, true);

DEFINE_INTEGRAL_ENUMERATOR(int32_t, int32_t, i, INT32_MIN, INT32_MAX);
DEFINE_SIMPLE_GENERATOR(int32_t, int32_t, i, "%" PRId32, true);

DEFINE_INTEGRAL_ENUMERATOR(uint32_t, uint32_t, u, 0, UINT32_MAX);
DEFINE_SIMPLE_GENERATOR(uint32_t, uint32_t, u, "%" PRIu32, true);

DEFINE_INTEGRAL_ENUMERATOR(dword, uint32_t, u, 0, UINT32_MAX);
DEFINE_SIMPLE_GENERATOR(dword, uint32_t, u, "%08" PRIx32, true);

DEFINE_INTEGRAL_ENUMERATOR(int64_t, int64_t, i, INT64_MIN, INT64_MAX);
DEFINE_SIMPLE_GENERATOR(int64_t, int64_t, i, "%" PRId64, true);

DEFINE_INTEGRAL_ENUMERATOR(uint64_t, uint64_t, u, 0, UINT64_MAX);
DEFINE_SIMPLE_GENERATOR(uint64_t, uint64_t, u, "%" PRIu64, true);

DEFINE_INTEGRAL_ENUMERATOR(qword, uint64_t, u, 0, UINT64_MAX);
DEFINE_SIMPLE_GENERATOR(qword, uint64_t, u, "%016" PRIx64, true);

DEFINE_INTEGRAL_ENUMERATOR(size_t, size_t, sz, 0, SIZE_MAX);
DEFINE_SIMPLE_GENERATOR(size_t, size_t, sz, "%zu", true);

DEFINE_SCALED_ENUMERATOR(small_int, int, i, -1);
DEFINE_DERIVED_GENERATOR(small_int, int, false);

DEFINE_SCALED_ENUMERATOR(small_unsigned, unsigned, u, 0);
DEFINE_DERIVED_GENERATOR(small_unsigned, unsigned, false);

DEFINE_SEQ_ENUMERATOR(bool, bool, b, false, true);

static void test_log_bool(test_value_t v)
{
    fprintf(stderr, "%s", v.b ? "true" : "false");
}

DEFINE_TRIVIAL_COMPARE(bool, b);
DEFINE_GENERATOR_RECORD(bool, bool, bool, bool, false);

DEFINE_RANGE_ENUMERATOR(char, char, ch,
                        {'\x1', '\x1f'},
                        {' ', ' '},
                        {'!', '/'},
                        {'0', '9'},
                        {':', '@'},
                        {'A', 'Z'},
                        {'[', '`'},
                        {'a', 'z'},
                        {'{', '~'},
                        {'\x7F', '\x7F'});

static void test_log_char(test_value_t v)
{
    char c = (char)v.ch;

    if (isprint(c))
        fprintf(stderr, "'%c'", c);
    else
        fprintf(stderr, "'\\x%02x'", c);
}

DEFINE_TRIVIAL_COMPARE(char, ch);
DEFINE_GENERATOR_RECORD(char, char, char, char, false);

DEFINE_INTEGRAL_ENUMERATOR(pchar, char, ch, ' ', '~');
DEFINE_SIMPLE_GENERATOR(pchar, char, ch, "'%c'", true);

DEFINE_INTEGRAL_ENUMERATOR(digit, char, ch, '0', '9');
DEFINE_DERIVED_GENERATOR(digit, char, true);

DEFINE_RANGE_ENUMERATOR(xdigit, char, ch,
                        {'0', '9'},
                        {'a', 'f'},
                        {'A', 'F'});
DEFINE_DERIVED_GENERATOR(xdigit, char, false);

DEFINE_INTEGRAL_ENUMERATOR(lowercase, char, ch, 'a', 'z');
DEFINE_DERIVED_GENERATOR(lowercase, char, true);

DEFINE_INTEGRAL_ENUMERATOR(uppercase, char, ch, 'A', 'Z');
DEFINE_DERIVED_GENERATOR(uppercase, char, true);

DEFINE_CHOICE_ENUMERATOR(alpha, lowercase, uppercase);
DEFINE_DERIVED_GENERATOR(alpha, char, true);

DEFINE_CHOICE_ENUMERATOR(alnum, alpha, digit);
DEFINE_DERIVED_GENERATOR(alnum, char, true);

DEFINE_RANGE_ENUMERATOR(wchar_t, wchar_t, ch,
                        {L'\u0001', L'\u001f'},
                        {L' ', L'~'},
                        {L'\u007F', L'\u007F'},
                        {L'\u0080', L'\u009F'},
                        {L'\u00A0', L'\u00FF'},
                        {L'\u0100', L'\u07FF'}
                        {L'\u0800', L'\uDFFF'},
                        {L'\uE000', L'\uF8FF'},
                        {L'\uF900', L'\uFFEF'},
                        {L'\uFFF0', L'\uFFFF'},
                        {L'\U10000', L'\UDFFFF'},
                        {L'\UE0000', L'\UE01FF'},
                        {L'\UF0000', L'\U10FFFF'},
                        {L'\U200000', L'\U3FFFFFF'},
                        {L'\U4000000', L'\U7FFFFFFF'});

static void test_log_wchar_t(test_value_t v)
{
    wchar_t c = v.ch;

    if (iswprint(c))
        fprintf(stderr, "'%lc' (U+%08x)", c, c);
    else
        fprintf(stderr, "U+%08x", c);
}

DEFINE_TRIVIAL_COMPARE(wchar_t, ch);
DEFINE_GENERATOR_RECORD(wchar_t, wchar_t, wchar_t, wchar_t, false);

int main(int argc, char *argv[])
{
    unsigned seed = 0;
    const testcase_t *cases;
    unsigned i;

    for (i = 1; i < argc; i++)
    {
        if (sscanf(argv[i], "--seed=%u", &seed) == 1)
            continue;
        if (sscanf(argv[i], "--samples=%zu", &test_max_samples) == 1)
            continue;
        if (sscanf(argv[i], "--scale=%zu", &test_sample_scale) == 1)
            continue;
        fprintf(stderr, "Unknown option '%s'\n", argv[i]);
        return EXIT_FAILURE;
    }

    if (seed == 0)
        srand((unsigned)time(NULL));

    for (cases = test_case_first; cases != NULL; cases = cases->chain)
    {
        testcase_run(cases);
    }

    fprintf(stderr, "%u cases run, %u succeded, random seed was %u\n",
            testcase_n_run, testcase_n_ok, seed);
    return testcase_n_run == testcase_n_ok ? 0 : EXIT_FAILURE;
}

#else

#define INVARIANT(_inv) ((void)0)
#define TESTCASE(_name, _title, _enabled, _expect_fail, _values, \
                 _body, ...) struct fake

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CHECK_H */
