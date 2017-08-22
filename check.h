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

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "utils.h"

static size_t test_max_samples;
static size_t test_sample_scale;

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

typedef no_null_args must_use bool (*test_enumerator_t)(unsigned i,
                                                        test_value_t *dest);

typedef void (*test_logger_t)(testval_t v);

typedef bool no_side_effects (*test_comparator_t)(test_value_t v1,
                                                  test_value_t v2);

typedef struct test_generator_t {
    test_enumerator_t enumerate;
    test_logger_t     log;
    test_comparator_t compare;
} test_generator_t;

#define DEFINE_SIMPLE_GENERATOR(_name, _type, _tv, _fmt)              \
    static void test_log_##_name(test_value_t v)                      \
    {                                                                 \
        fprintf(stderr, _fmt, (_type)v._tv);                          \
    }                                                                 \
                                                                      \
    static no_shared_state                                            \
    bool test_compare_##_name(test_value_t v1, test_value_t v2)       \
    {                                                                 \
        return v1._tv == v2._tv;                                      \
    }                                                                 \
                                                                      \
    static const test_generator_t test_every_##_name = {              \
        .enumerate = test_enum_##_name,                               \
        .log       = test_log_##_name,                                \
        .compare   = test_compare_##_name                             \
    }

#define DEFINE_DERIVED_GENERATOR(_name, _basename)              \
    static const test_generator_t test_every_##_name = {        \
        .enumerate = test_enum_##_name,                         \
        .log       = test_log_##_basename,                      \
        .compare   = test_compare_##_basename                   \
    }

static must_use
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

static inline no_null_args
void test_make_random_bytes(void *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)buf)[i] = (uint8_t)rand();
}

#define DEFINE_INTEGRAL_ENUMERATOR(_name, _type, _tv, _min, _max)   \
    static must_use no_null_args                                    \
    bool test_enum_##_name(unsigned i, test_value_t *dest)          \
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
                test_make_random_bytes(&r, r);                      \
                dest->_tv = r;                                      \
                break;                                              \
            }                                                       \
        }                                                           \
        return dest->_tv == _min || dest->_tv == _max ?             \
            TESTVAL_BOUNDARY : TESTVAL_REGULAR;                     \
    }                                                               \
    struct fake

#define DEFINE_SEQ_ENUMERATOR(_name, _type, _tv, ...)               \
    static must_use no_null_args                                    \
    bool test_enum_##_name(unsigned i, test_value_t *dest)          \
    {                                                               \
        static const _type vals[] = {__VA_ARGS__};                  \
                                                                    \
        if (i >= sizeof(vals) / sizeof(*vals))                      \
            return TESTVAL_NONE;                                    \
                                                                    \
        dest->_tv = vals[i];                                        \
        return i == 0 || i == sizeof(vals) / sizeof(*vals) - 1 ?    \
            TESTVAL_BOUNDARY : TESTVAL_REGULAR;                     \
    }                                                               \
    struct fake

#define DEFINE_SCALED_ENUMERATOR(_name, _type, _tv, _min)               \
    static must_use no_null_args                                        \
    bool test_enum_##_name(unsigned i, test_value_t *dest)              \
    {                                                                   \
        if (i >= test_sample_scale)                                     \
            return TESTVAL_;                                            \
                                                                        \
        dest->_tv = _min * test_sample_scale / 2 + (_type)i;            \
                                                                        \
        return i == 0 || i == test_sample_scale - 1 ?                   \
            TESTVAL_BOUNDARY : TESTVAL_REGULAR;                         \
    }                                                                   \
    struct fake

typedef no_null_args void test_property_t(const test_value_t vals[]);

typedef struct testcase_t {
    const char                     *title;
    bool                            enabled;
    bool                            expect_fail;
    const test_generator_t * const *params;
    test_property_t                 property;
    const struct testcase_t        *chain;
} testcase_t;

static inline no_null_args must_use no_side_effects
unsigned testcase_count_params(const testcase_t *tc)
{
    unsigned i;

    for(i = 0; tc->params[i] != NULL; i++)
        ;
    
    return i;
}

static unsigned testcase_n_run;
static unsigned testcase_n_ok;

static inline no_side_effects must_use
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

static no_null_args must_use
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

static no_null_args must_use
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

static no_null_args must_use
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

static no_null_args
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
        if (!params[p].observed_boundary)
            fprintf(stderr, "\tWarning: no boundary values for parameter %u tested\n", p);
        if (!params[p].observed_regular)
            fprintf(stderr, "\tWarning: no regular values for parameter %u tested\n", p);
    }
}

#define ASSERT(_expr)                                       \
    do                                                      \
    {                                                       \
        if (!(_expr))                                       \
        {                                                   \
            fputs(stderr, "Assertion " #_expr " failed\n"); \
            raise(SIGTRAP);                                 \
        }                                                   \
    } while (0)

#define EXPECT(_gen, _v1, _v2)                              \
    do                                                      \
    {                                                       \
        if (!test_compare_values(&every_##_gen, _v1, _v2))  \
            raise(SIGTRAP);                                 \
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
    static constructor                                                  \
    void test_case_##_name##_init(void)                                 \
    {                                                                   \
        *test_case_last = &test_case_##_name;                           \
        test_case_last  = &test_case_##_name.chain;                     \
    }                                                                   \
                                                                        \
    struct fake

int main(int argc, char *argv[])
{
    PROP_list_t *iter;
    
    if (argc > 1)
        srand(strtoul(argv[1], NULL, 10));
    else
    {
        unsigned seed = (unsigned)time(NULL);
        fprintf(stderr, "Random seed is %u\n", seed);
        srand(seed);
    }

    for (iter = PROP_the_list; iter != NULL; iter = iter->chain)
    {
        iter->prop();
    }

    return 0;
}

#else

#define PROPERTY(_name, _body) struct fake
#define INVARIANT(_inv) ((void)0)

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CHECK_H */
