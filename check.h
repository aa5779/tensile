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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include "utils.h"

static size_t test_max_samples = 100;
static size_t test_sample_scale = 10;
static unsigned test_timeout = 30;

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
    TVCLASS_TRIVIAL = 0,
    TVCLASS_SPECIAL,
    TVCLASS_NORMAL,
    TVCLASS_BUCKET0,
    TVCLASS_BUCKET1,
    TVCLASS_BUCKET2,
    TVCLASS_BUCKET3
};

typedef uint64_t test_value_class_set_t;

#define TVCLASS(_x) (1ull << TVCLASS_##_x)

typedef NO_NULL_ARGS MUST_USE test_value_class_set_t (*test_enumerator_t)(
        unsigned i,
        test_value_t *dest);

typedef void (*test_logger_t)(test_value_t v);

typedef bool (*test_comparator_t)(test_value_t v1, test_value_t v2);

typedef struct test_generator_t {
    test_enumerator_t      enumerate;
    test_logger_t          log;
    test_comparator_t      compare;
    test_value_class_set_t required;
    test_value_class_set_t allowed;
} test_generator_t;

#define DEFINE_GENERATOR_RECORD(_name, _enumname, _logname, _comparename, \
                                _required, _allowed)                    \
    static UNUSED const test_generator_t test_every_##_name = {         \
        .enumerate    = test_enum_##_enumname,                          \
        .log          = test_log_##_logname,                            \
        .compare      = test_compare_##_comparename,                    \
        .required     = _required,                                      \
        .allowed      = _allowed                                        \
    }

#define DEFINE_TRIVIAL_COMPARE(_name, _tv)                            \
    static NO_SHARED_STATE                                            \
    bool test_compare_##_name(test_value_t v1, test_value_t v2)       \
    {                                                                 \
        return v1._tv == v2._tv;                                      \
    }                                                                 \
    struct fake

#define DEFINE_SIMPLE_GENERATOR(_name, _type, _tv, _fmt,                \
                                _required, _allowed)                    \
    static void test_log_##_name(test_value_t v)                        \
    {                                                                   \
        fprintf(stderr, _fmt, (_type)v._tv);                            \
    }                                                                   \
                                                                        \
    DEFINE_TRIVIAL_COMPARE(_name, _tv);                                 \
    DEFINE_GENERATOR_RECORD(_name, _name, _name, _name, _required, _allowed)

#define DEFINE_DERIVED_GENERATOR(_name, _basename, _required, _allowed) \
    DEFINE_GENERATOR_RECORD(_name, _name, _basename, _basename,         \
                            _required, _allowed)

static UNUSED MUST_USE
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
    size_t i;

    for (i = 0; i < n; i++)
        ((uint8_t *)buf)[i] = (uint8_t)rand();
}

static inline NO_NULL_ARGS
void test_print_hex(void *buf, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++)
    {
        fprintf(stderr, "%s%02x", i == 0 ? "" : ":",
                ((uint8_t *)buf)[i]);
    }
}

static inline NO_NULL_ARGS
void test_print_chars(const char *s)
{
    size_t i;

    for (i = 0; s[i] != '\0'; i++)
    {
        if (isprint(s[i]))
            fputc(s[i], stderr);
        else
            fprintf(stderr, "\\x%02x", s[i]);
    }
}

static inline
void test_print_class_set(test_value_class_set_t set)
{
    bool need_comma = false;
    unsigned i;

    for (i = 0; i < CHAR_BIT * sizeof(set); i++)
    {
        if (set & (1ull << i))
        {
            fprintf(stderr, "%s%u", need_comma ? "," : "", i);
            need_comma = true;
        }
    }
}

#define DEFINE_INTEGRAL_ENUMERATOR(_name, _type, _tv, _min, _max)   \
    static MUST_USE NO_NULL_ARGS                                    \
    test_value_class_set_t                                          \
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
            TVCLASS(SPECIAL) : TVCLASS(NORMAL);                     \
    }                                                               \
    struct fake

#define DEFINE_ENUM_ENUMERATOR(_name, _type, _tv, _min, _max)           \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(UNUSED unsigned i, test_value_t *dest)            \
    {                                                                   \
        dest->_tv = (_type)(rand() % (int)(_max - _min + 1)) + _min;    \
        return TVCLASS(NORMAL);                                         \
    }                                                                   \
    struct fake

#define DEFINE_SEQ_ENUMERATOR(_name, _type, _tv, ...)                   \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        static const _type vals[] = {__VA_ARGS__};                      \
        static const size_t n_vals = sizeof(vals) / sizeof(*vals);      \
                                                                        \
        if (i >= n_vals)                                                \
            return 0;                                                   \
                                                                        \
        dest->_tv = vals[i];                                            \
        return TVCLASS(NORMAL);                                         \
    }                                                                   \
    struct fake

#define DEFINE_SCALED_ENUMERATOR(_name, _type, _tv, _min)               \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        if (i >= test_sample_scale)                                     \
            return 0;                                                   \
                                                                        \
        dest->_tv = _min * (_type)test_sample_scale / 2 + (_type)i;     \
                                                                        \
        return TVCLASS(NORMAL);                                         \
    }                                                                   \
    struct fake

#define DEFINE_RANGE_ENUMERATOR(_name, _type, _tv, ...)                 \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(UNUSED unsigned i, test_value_t *dest)            \
    {                                                                   \
        static const struct {                                           \
            _type               min;                                    \
            _type               max;                                    \
            test_value_class_set_t class;                               \
        } ranges[] = {__VA_ARGS__};                                     \
        static const size_t n_ranges = sizeof(ranges) / sizeof(*ranges); \
        size_t r = (size_t)rand() % n_ranges;                           \
        _type v = (_type)(rand() % (ranges[r].max - ranges[r].min + 1) + \
                          ranges[r].min);                               \
        dest->_tv = v;                                                  \
        return ranges[r].class;                                         \
    }                                                                   \
    struct fake

#define DEFINE_CHOICE_ENUMERATOR(_name, _class1, _option1,              \
                                 _class2, _option2)                     \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        test_value_class_set_t result;                                  \
                                                                        \
        if (rand() % 2)                                                 \
        {                                                               \
            result = test_enum_##_option2(i, dest);                     \
            if (result)                                                 \
                result |= _class2;                                      \
            else {                                                      \
                result = test_enum_##_option1(i, dest);                 \
                if (result)                                             \
                    result |= _class1;                                  \
            }                                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            result = test_enum_##_option1(i, dest);                     \
            if (result)                                                 \
                result |= _class1;                                      \
            else {                                                      \
                result = test_enum_##_option2(i, dest);                 \
                if (result)                                             \
                    result |= _class2;                                  \
            }                                                           \
        }                                                               \
        return result;                                                  \
    }                                                                   \
    struct fake

#define DEFINE_INTERLEAVE_ENUMERATOR(_name, _class1, _branch1,          \
                                     _class2, _branch2)                 \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        test_value_class_set_t result;                                  \
                                                                        \
        if (i % 2)                                                      \
        {                                                               \
            result = test_enum_##_branch2(i / 2, dest);                 \
            if (result)                                                 \
                result |= _class2;                                      \
            else {                                                      \
                result = test_enum_##_branch1(i / 2, dest);             \
                if (result)                                             \
                    result |= _class1;                                  \
            }                                                           \
        }                                                               \
        else                                                            \
        {                                                               \
            result = test_enum_##_branch1(i / 2, dest);                 \
            if (result)                                                 \
                result |= _class1;                                      \
            else {                                                      \
                result = test_enum_##_branch2(i / 2, dest);             \
                if (result)                                             \
                    result |= _class2;                                  \
            }                                                           \
        }                                                               \
        return result;                                                  \
    }                                                                   \
    struct fake


#define DEFINE_AUGMENTED_ENUMERATOR(_name, _basename, _type, _tv, ...)  \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
    test_enum_##_name(unsigned i, test_value_t *dest)                   \
    {                                                                   \
        static const _type vals[] = {__VA_ARGS__};                      \
        static const size_t n_vals = sizeof(vals) / sizeof(*vals);      \
                                                                        \
        if (i >= n_vals)                                                \
            return test_enum_##_basename(i - (unsigned)n_vals, dest);   \
                                                                        \
        dest->_tv = vals[i];                                            \
        return TVCLASS(SPECIAL);                                        \
    }                                                                   \
    struct fake

static MUST_USE NO_NULL_ARGS
test_value_class_set_t
test_string_enumerate(unsigned i, test_value_t *dest,
                      test_enumerator_t elt_enum)
{
    size_t sz = i == 0 ? 0 : (unsigned)rand() % test_sample_scale;
    char *s = tn_alloc_blob(sz + 1);
    size_t j;

    for (j = 0; j < sz; j++)
    {
        test_value_t elt;
        if (elt_enum(i, &elt) == 0)
            break;
        s[j] = (char)elt.ch;
    }
    s[sz] = '\0';
    dest->s = s;
    return sz == 0 ? TVCLASS(TRIVIAL) : TVCLASS(NORMAL);
}

#define DEFINE_STRING_ENUMERATOR(_name, _eltname)                       \
    static MUST_USE NO_NULL_ARGS                                        \
    test_value_class_set_t                                              \
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
    DEFINE_GENERATOR_RECORD(_name, _name, _name, _name,                 \
                            TVCLASS(NORMAL) | TVCLASS(TRIVIAL), 0)


#define DEFINE_BLOB_GENERATOR(_name, _type, _var, _sizeexpr, _dataexpr, \
                              _required, _allowed)                      \
    static void test_log_##_name(test_value_t v)                        \
    {                                                                   \
        const _type *_var = v.p;                                        \
        test_print_hex(_dataexpr, _sizeexpr);                           \
    }                                                                   \
                                                                        \
    static NO_SHARED_STATE                                              \
    bool test_compare_##_name(test_value_t v1, test_value_t v2)         \
    {                                                                   \
        void *v1_buf;                                                   \
        size_t v1_len;                                                  \
        void *v2_buf;                                                   \
        size_t v2_len;                                                  \
        {                                                               \
            const _type *_var = v1.p;                                   \
            v1_buf = (_dataexpr);                                       \
            v1_len = (_sizeexpr);                                       \
        }                                                               \
        {                                                               \
            const _type *_var = v2.p;                                   \
            v2_buf = (_dataexpr);                                       \
            v2_len = (_sizeexpr);                                       \
        }                                                               \
                                                                        \
        if (v1_len != v2_len)                                           \
            return false;                                               \
                                                                        \
        return memcmp(v1_buf, v2_buf, v1_len) == 0;                     \
    }                                                                   \
                                                                        \
    DEFINE_GENERATOR_RECORD(_name, _name, _name, _name, _required, _allowed)

typedef NO_NULL_ARGS void (*test_property_t)(const test_value_t vals[]);

typedef struct testcase_t {
    const char                     *title;
    bool                            enabled;
    bool                            expect_fail;
    const test_generator_t * const *params;
    test_property_t                 property;
    struct testcase_t              *chain;
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
size_t testcase_n_samples_per_param(unsigned n)
{
    return (size_t)pow((double)test_max_samples, 1.0 / n);
}

typedef struct testcase_param_t {
    unsigned               index;
    test_value_t           value;
    test_value_class_set_t observed;
} testcase_param_t;

static NO_NULL_ARGS MUST_USE
bool testcase_fill_param(testcase_param_t *param,
                         test_enumerator_t enumerate,
                         size_t limit)
{
    test_value_class_set_t cls;

    if (param->index == limit)
        return false;

    cls = enumerate(param->index, &param->value);
    if (cls == 0)
        return false;

    param->observed |= cls;
    param->index++;

    return true;
}

static NO_NULL_ARGS MUST_USE
bool testcase_step_all_params(testcase_param_t *params,
                              const test_generator_t * const *gens,
                              size_t n_params,
                              size_t limit)
{
    size_t i;

    for (i = n_params; i > 0; i--)
    {
        if (testcase_fill_param(&params[i - 1], gens[i - 1]->enumerate,
                                limit))
            break;
    }
    if (i == 0)
        return false;
    for (; i < n_params; i++)
    {
        params[i].index = 0;
        if (!testcase_fill_param(&params[i], gens[i]->enumerate, limit))
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

        if (tc->expect_fail) {
            if (setrlimit(RLIMIT_CORE,
                          &(struct rlimit){.rlim_cur = 0,
                                  .rlim_max = 0}) != 0)
            {
                perror("setrlimit(RLIMIT_CORE)");
                abort();
            }
        }
        if (setrlimit(RLIMIT_CPU,
                      &(struct rlimit){.rlim_cur = test_timeout,
                              .rlim_max = test_timeout + 1}) != 0)
        {
            perror("setrlimit(RLIMIT_CPU)");
            abort();
        }
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
        WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

static NO_NULL_ARGS
void testcase_run(const testcase_t *tc)
{
    unsigned n_params = testcase_count_params(tc);
    testcase_param_t params[n_params];
    unsigned i;
    unsigned p;
    size_t n_samples = testcase_n_samples_per_param(n_params);

    memset(params, 0, sizeof(params));

    fprintf(stderr, "%s...", tc->title);
    fflush(stderr);
    if (!tc->enabled)
    {
        fputs("SKIP\n", stderr);
        return;
    }
    if (tc->property == NULL)
    {
        fputs("UNTESTED\n", stderr);
        return;
    }
    for (p = 0; p < n_params; p++)
    {
        if (!testcase_fill_param(&params[p], tc->params[p]->enumerate,
                                 n_samples))
        {
            fputs("UNTESTED\n", stderr);
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
            tc->params[p]->log(params[p].value);
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
        test_value_class_set_t unobserved =
            tc->params[p]->required & ~params[p].observed;
        test_value_class_set_t unexpected =
            ~(tc->params[p]->required | tc->params[p]->allowed) &
            params[p].observed;
        if (unobserved)
        {
            fprintf(stderr, "\tWarning: some required value classes for parameter %u were not observed: ", p);
            test_print_class_set(unobserved);
            fputc('\n', stderr);
        }
        if (unexpected)
        {
            fprintf(stderr, "\tWarning: some unexpected value classes for parameter %u were observed: ", p);
            test_print_class_set(unexpected);
            fputc('\n', stderr);
        }
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

#define ISOLATED(_isexitp, _statusp, _code)         \
    do {                                            \
        int status;                                 \
        pid_t child = fork();                       \
                                                    \
        if (child == (pid_t)-1)                     \
        {                                           \
            perror("fork");                         \
            abort();                                \
        }                                           \
        else if (child == 0)                        \
        {                                           \
            _code;                                  \
            exit(0);                                \
        }                                           \
        else                                        \
        {                                           \
            if (waitpid(child, &status, 0) != child)    \
            {                                       \
                perror("waitpid");                  \
                abort();                            \
            }                                       \
            (_isexitp)->b = WIFEXITED(status);      \
            (_statusp)->i = WIFEXITED(status) ?     \
                WEXITSTATUS(status) :               \
                WTERMSIG(status);                   \
        }                                           \
    } while (0)

static UNUSED NO_NULL_ARGS
void test_redirect_output(int target_fd, int *saved_fd, int *read_fd)
{
    int pipe_fds[2];

    *saved_fd = dup(target_fd);
    if (*saved_fd < 0)
    {
        perror("dup");
        abort();
    }
    if (pipe(pipe_fds) != 0)
    {
        perror("pipe");
        abort();
    }
    if (dup2(pipe_fds[1], target_fd) != target_fd)
    {
        perror("dup2");
        abort();
    }
    *read_fd = pipe_fds[0];
    if (close(pipe_fds[1]) != 0)
    {
        perror("close");
        abort();
    }
}

static NO_NULL_ARGS
void test_end_redirect(int target_fd, int saved_fd, int read_fd)
{
    if (dup2(saved_fd, target_fd) != target_fd)
    {
        perror("dup2");
        abort();
    }
    if (close(saved_fd) != 0 || (read_fd >= 0 && close(read_fd) != 0))
    {
        perror("close");
        abort();
    }
}

static UNUSED NO_NULL_ARGS
void test_read_redirected(int target_fd, int read_fd, int saved_fd, test_value_t *dest)
{
    static char test_iobuf[4096];
    ssize_t len;

    len = read(read_fd, test_iobuf, sizeof(test_iobuf) - 1);
    if (len < 0)
    {
        perror("read");
        abort();
    }
    test_iobuf[len] = '\0';
    dest->s = tn_cstrdup(test_iobuf);
    test_end_redirect(target_fd, saved_fd, read_fd);
}

#define REDIRECT_OUTPUT_FD(_fd, _valuep, _code)                         \
    do {                                                                \
        int saved_##_fd;                                                \
        int read_from_##_fd;                                            \
                                                                        \
        test_redirect_output(_fd, &saved_##_fd, &read_from_##_fd);      \
        _code;                                                          \
        test_read_redirected(_fd, read_from_##_fd, saved_##_fd, _valuep); \
    } while (0)

#define REDIRECT_STDOUT(_valuep, _code)                 \
    REDIRECT_OUTPUT_FD(STDOUT_FILENO, _valuep, _code)

#define REDIRECT_STDERR(_valuep, _code)                 \
    REDIRECT_OUTPUT_FD(STDERR_FILENO, _valuep, _code)


static UNUSED NO_NULL_ARGS
void test_redirect_input(int target_fd, int *saved_fd, test_value_t src)
{
    int pipe_fds[2];
    ssize_t len = (ssize_t)strlen(src.s);

    *saved_fd = dup(target_fd);
    if (*saved_fd < 0)
    {
        perror("dup");
        abort();
    }
    if (pipe(pipe_fds) != 0)
    {
        perror("pipe");
        abort();
    }
    if (dup2(pipe_fds[0], target_fd) != target_fd)
    {
        perror("dup2");
        abort();
    }
    if (close(pipe_fds[0]) != 0)
    {
        perror("close");
        abort();
    }
    if (write(pipe_fds[1], src.s, (size_t)len) != len)
    {
        perror("write");
        abort();
    }
    if (close(pipe_fds[1]) != 0)
    {
        perror("close");
        abort();
    }
}

#define REDIRECT_STDIN(_src, _code)                             \
    do {                                                        \
        int saved_stdin;                                        \
        test_redirect_input(STDIN_FILENO, &saved_stdin, _src);  \
        _code;                                                  \
        test_end_redirect(STDIN_FILENO, saved_stdin, -1);       \
    } while (0)

static testcase_t *test_case_first;
static UNUSED testcase_t **test_case_last = &test_case_first;

#define TESTCASE(_name, _title, _enabled, _expect_fail, _values,        \
                 _body, ...)                                            \
    static void test_property_##_name(UNUSED const test_value_t _values[]) \
    {                                                                   \
        _body;                                                          \
    }                                                                   \
                                                                        \
    static testcase_t test_case_##_name =                               \
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
    CONSTRUCTOR                                                         \
    void test_case_##_name##_init(void)                                 \
    {                                                                   \
        *test_case_last = &test_case_##_name;                           \
        test_case_last  = &test_case_##_name.chain;                     \
    }                                                                   \
                                                                        \
    struct fake


DEFINE_INTEGRAL_ENUMERATOR(int, int, i, INT_MIN, INT_MAX);
DEFINE_SIMPLE_GENERATOR(int, int, i, "%d",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(unsigned, unsigned, u, 0, UINT_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned, unsigned, u, "%u",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(short, short, i, SHRT_MIN, SHRT_MAX);
DEFINE_SIMPLE_GENERATOR(short, short, i, "%hd",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_short, unsigned short, u, 0, USHRT_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_short, unsigned short, u, "%hu",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(long, long, i, LONG_MIN, LONG_MAX);
DEFINE_SIMPLE_GENERATOR(long, long, i, "%ld",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_long, unsigned long, u, 0, ULONG_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_long, unsigned long, u, "%lu",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(long_long, long long, i, LLONG_MIN, LLONG_MAX);
DEFINE_SIMPLE_GENERATOR(long_long, long long, i, "%lld",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(unsigned_long_long, unsigned long long,
                           u, 0, ULLONG_MAX);
DEFINE_SIMPLE_GENERATOR(unsigned_long_long, unsigned long long, i,
                        "%llu", TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(intmax_t, intmax_t, i, INTMAX_MIN, INTMAX_MAX);
DEFINE_SIMPLE_GENERATOR(intmax_t, intmax_t, i, "%" PRIdMAX,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(uintmax_t, uintmax_t, u, 0, UINTMAX_MAX);
DEFINE_SIMPLE_GENERATOR(uintmax_t, uintmax_t, u, "%" PRIuMAX,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(int8_t, int8_t, i, INT8_MIN, INT8_MAX);
DEFINE_SIMPLE_GENERATOR(int8_t, int8_t, i, "%" PRId8,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(uint8_t, uint8_t, u, 0, UINT8_MAX);
DEFINE_SIMPLE_GENERATOR(uint8_t, uint8_t, u, "%" PRIu8,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(byte, uint8_t, u, 0, UINT8_MAX);
DEFINE_SIMPLE_GENERATOR(byte, uint8_t, u, "%02" PRIx8,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(int16_t, int16_t, i, INT16_MIN, INT16_MAX);
DEFINE_SIMPLE_GENERATOR(int16_t, int16_t, i, "%" PRId16,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(uint16_t, uint16_t, u, 0, UINT16_MAX);
DEFINE_SIMPLE_GENERATOR(uint16_t, uint16_t, u, "%" PRIu16,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(word, uint16_t, u, 0, UINT16_MAX);
DEFINE_SIMPLE_GENERATOR(word, uint16_t, u, "%04" PRIx16,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(int32_t, int32_t, i, INT32_MIN, INT32_MAX);
DEFINE_SIMPLE_GENERATOR(int32_t, int32_t, i, "%" PRId32,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(uint32_t, uint32_t, u, 0, UINT32_MAX);
DEFINE_SIMPLE_GENERATOR(uint32_t, uint32_t, u, "%" PRIu32,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(dword, uint32_t, u, 0, UINT32_MAX);
DEFINE_SIMPLE_GENERATOR(dword, uint32_t, u, "%08" PRIx32,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(int64_t, int64_t, i, INT64_MIN, INT64_MAX);
DEFINE_SIMPLE_GENERATOR(int64_t, int64_t, i, "%" PRId64,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(uint64_t, uint64_t, u, 0, UINT64_MAX);
DEFINE_SIMPLE_GENERATOR(uint64_t, uint64_t, u, "%" PRIu64,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(qword, uint64_t, u, 0, UINT64_MAX);
DEFINE_SIMPLE_GENERATOR(qword, uint64_t, u, "%016" PRIx64,
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_INTEGRAL_ENUMERATOR(size_t, size_t, sz, 0, SIZE_MAX);
DEFINE_SIMPLE_GENERATOR(size_t, size_t, sz, "%zu",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_SCALED_ENUMERATOR(small_int, int, i, -1);
DEFINE_DERIVED_GENERATOR(small_int, int, TVCLASS(NORMAL), 0);

DEFINE_SCALED_ENUMERATOR(small_unsigned, unsigned, u, 0);
DEFINE_DERIVED_GENERATOR(small_unsigned, unsigned, TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(byte_bit, unsigned, u, 0, CHAR_BIT - 1);
DEFINE_DERIVED_GENERATOR(byte_bit, unsigned, TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(word_bit, unsigned, u, 0,
                       CHAR_BIT * sizeof(uint16_t) - 1);
DEFINE_DERIVED_GENERATOR(word_bit, unsigned, TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(dword_bit, unsigned, u, 0,
                       CHAR_BIT * sizeof(uint32_t) - 1);
DEFINE_DERIVED_GENERATOR(dword_bit, unsigned, TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(qword_bit, unsigned, u, 0,
                       CHAR_BIT * sizeof(uint64_t) - 1);
DEFINE_DERIVED_GENERATOR(qword_bit, unsigned, TVCLASS(NORMAL), 0);

DEFINE_SEQ_ENUMERATOR(bool, bool, b, false, true);

static void test_log_bool(test_value_t v)
{
    fprintf(stderr, "%s", v.b ? "true" : "false");
}

DEFINE_TRIVIAL_COMPARE(bool, b);
DEFINE_GENERATOR_RECORD(bool, bool, bool, bool, TVCLASS(NORMAL), 0);

DEFINE_RANGE_ENUMERATOR(char, char, ch,
                        {'\x1', '\x1f', TVCLASS(SPECIAL)},
                        {' ', ' ', TVCLASS(BUCKET0)},
                        {'!', '/', TVCLASS(BUCKET1)},
                        {'0', '9', TVCLASS(BUCKET2)},
                        {':', '@', TVCLASS(BUCKET1)},
                        {'A', 'Z', TVCLASS(BUCKET3)},
                        {'[', '`', TVCLASS(BUCKET1)},
                        {'a', 'z', TVCLASS(BUCKET3)},
                        {'{', '~', TVCLASS(BUCKET1)},
                        {'\x7F', '\x7F', TVCLASS(SPECIAL)});

static void test_log_char(test_value_t v)
{
    char c = (char)v.ch;

    if (isprint(c))
        fprintf(stderr, "'%c'", c);
    else
        fprintf(stderr, "'\\x%02x'", c);
}

DEFINE_TRIVIAL_COMPARE(char, ch);
DEFINE_GENERATOR_RECORD(char, char, char, char,
                        TVCLASS(SPECIAL) | TVCLASS(BUCKET0) |
                        TVCLASS(BUCKET1) | TVCLASS(BUCKET2) |
                        TVCLASS(BUCKET3), 0);

DEFINE_ENUM_ENUMERATOR(pchar, char, ch, ' ', '~');
DEFINE_SIMPLE_GENERATOR(pchar, char, ch, "'%c'", TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(digit, char, ch, '0', '9');
DEFINE_DERIVED_GENERATOR(digit, char, TVCLASS(NORMAL), 0);

DEFINE_RANGE_ENUMERATOR(xdigit, char, ch,
                        {'0', '9', TVCLASS(BUCKET0)},
                        {'a', 'f', TVCLASS(BUCKET1)},
                        {'A', 'F', TVCLASS(BUCKET1)});
DEFINE_DERIVED_GENERATOR(xdigit, char,
                         TVCLASS(BUCKET0) | TVCLASS(BUCKET1), 0);

DEFINE_ENUM_ENUMERATOR(lowercase, char, ch, 'a', 'z');
DEFINE_DERIVED_GENERATOR(lowercase, char, TVCLASS(NORMAL), 0);

DEFINE_ENUM_ENUMERATOR(uppercase, char, ch, 'A', 'Z');
DEFINE_DERIVED_GENERATOR(uppercase, char, TVCLASS(NORMAL), 0);

DEFINE_CHOICE_ENUMERATOR(alpha, TVCLASS(BUCKET0), lowercase,
                         TVCLASS(BUCKET1), uppercase);
DEFINE_DERIVED_GENERATOR(alpha, char,
                         TVCLASS(NORMAL) | TVCLASS(BUCKET0) |
                         TVCLASS(BUCKET1), 0);

DEFINE_CHOICE_ENUMERATOR(alnum, 0, alpha, TVCLASS(BUCKET2), digit);
DEFINE_DERIVED_GENERATOR(alnum, char,
                         TVCLASS(NORMAL) | TVCLASS(BUCKET0) |
                         TVCLASS(BUCKET1) | TVCLASS(BUCKET2), 0);

DEFINE_RANGE_ENUMERATOR(wchar_t, wchar_t, ch,
                        {0x1, 0x1f, TVCLASS(SPECIAL)},
                        {L' ', L'~', TVCLASS(TRIVIAL)},
                        {0x7F, 0x7F, TVCLASS(SPECIAL)},
                        {0x80, 0x9F, TVCLASS(SPECIAL)},
                        {0xA0, 0xFF, TVCLASS(BUCKET0)},
                        {0x0100, 0x07FF, TVCLASS(BUCKET0)},
                        {0x0800, 0xD7FF, TVCLASS(BUCKET1)},
                        {0xE000, 0xF8FF, TVCLASS(SPECIAL)},
                        {0xF900, 0xFFEF, TVCLASS(SPECIAL)},
                        {0xFFF0, 0xFFFF, TVCLASS(SPECIAL)},
                        {0x00010000, 0x000DFFFF, TVCLASS(BUCKET2)},
                        {0x000E0000, 0x000E01FF, TVCLASS(SPECIAL)},
                        {0x000F0000, 0x0010FFFF, TVCLASS(BUCKET2)},
                        {0x00200000, 0x03FFFFFF, TVCLASS(BUCKET3)},
                        {0x04000000, 0x7FFFFFFF, TVCLASS(BUCKET3)});

static void test_log_wchar_t(test_value_t v)
{
    wchar_t c = v.ch;

    if (iswprint((wint_t)c))
        fprintf(stderr, "'%lc' (U+%08x)", c, c);
    else
        fprintf(stderr, "U+%08x", c);
}

DEFINE_TRIVIAL_COMPARE(wchar_t, ch);
DEFINE_GENERATOR_RECORD(wchar_t, wchar_t, wchar_t, wchar_t,
                        TVCLASS(TRIVIAL) | TVCLASS(SPECIAL) |
                        TVCLASS(BUCKET0) | TVCLASS(BUCKET1) |
                        TVCLASS(BUCKET2) | TVCLASS(BUCKET3), 0);

DEFINE_STRING_ENUMERATOR(string, char);
DEFINE_STRING_GENERATOR(string);

DEFINE_STRING_ENUMERATOR(pstring, pchar);
DEFINE_STRING_GENERATOR(pstring);

DEFINE_STRING_ENUMERATOR(digits, digit);
DEFINE_STRING_GENERATOR(digits);

DEFINE_STRING_ENUMERATOR(xdigits, xdigit);
DEFINE_STRING_GENERATOR(xdigits);

DEFINE_STRING_ENUMERATOR(alnums, alnum);
DEFINE_STRING_GENERATOR(alnums);

static MUST_USE NO_NULL_ARGS
test_value_class_set_t
test_enum_double(unsigned i, test_value_t *dest)
{
    static const double bvalues[] = {0.0, DBL_MAX, -DBL_MAX,
                                     DBL_MIN, -DBL_MIN};
    static const unsigned n_bvalues = sizeof(bvalues) / sizeof(*bvalues);
    double m;
    int exp;

    if (i < n_bvalues)
    {
        dest->d = bvalues[i];
        return TVCLASS(SPECIAL);
    }
    m = (double)rand() / (RAND_MAX / 2) - 1.0;
    exp = rand() % (FLT_MAX_EXP - FLT_MIN_EXP + 1) + FLT_MIN_EXP;

    dest->d = scalbn(m, exp);
    return TVCLASS(NORMAL);
}

DEFINE_SIMPLE_GENERATOR(double, double, d, "%g",
                        TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

DEFINE_AUGMENTED_ENUMERATOR(xdouble, double, double, d, -0.0,
                            DBL_MIN / 2, -DBL_MIN / 2,
                            INFINITY, -INFINITY, NAN);
DEFINE_DERIVED_GENERATOR(xdouble, double,
                         TVCLASS(NORMAL) | TVCLASS(SPECIAL), 0);

static MUST_USE NO_NULL_ARGS
test_value_class_set_t
test_enum_fraction(UNUSED unsigned i, test_value_t *dest)
{
    dest->d = (double)rand() / RAND_MAX;
    return TVCLASS(NORMAL);
}
DEFINE_DERIVED_GENERATOR(fraction, double, TVCLASS(NORMAL), 0);

int main(int argc, char *argv[])
{
    unsigned seed = 0;
    const testcase_t *cases;
    int i;

    GC_INIT();
    for (i = 1; i < argc; i++)
    {
        if (sscanf(argv[i], "--seed=%u", &seed) == 1)
            continue;
        if (sscanf(argv[i], "--samples=%zu", &test_max_samples) == 1)
            continue;
        if (sscanf(argv[i], "--scale=%zu", &test_sample_scale) == 1)
            continue;
        if (sscanf(argv[i], "--timeout=%u", &test_timeout) == 1)
            continue;
        fprintf(stderr, "Unknown option '%s'\n", argv[i]);
        return EXIT_FAILURE;
    }

    if (seed == 0)
        seed = (unsigned)time(NULL);
    srand(seed);

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
