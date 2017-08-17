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

typedef no_null_args must_use bool (*test_enumerator_t)(unsigned i,
                                                        test_value_t *dest);
typedef void (*test_logger_t)(testval_t v);

typedef struct test_generator_t {
    test_enumerator_t enumerate;
    test_logger_t     log;
} test_generator_t;

#define DEFINE_SIMPLE_GENERATOR(_name, _type, _tv, _fmt)              \
    static void test_log_##_name(test_value_t v)                      \
    {                                                                 \
        fprintf(stderr, _fmt, (_type)v._tv);                          \
    }                                                                 \
                                                                      \
    static const test_generator_t test_every_##_name = {              \
        .enumerate = test_enum_##_name,                               \
        .log       = test_log_##_name                                 \
    }

#define DEFINE_DERIVED_GENERATOR(_name, _basename)              \
    static const test_generator_t test_every_##_name = {        \
        .enumerate = test_enum_##_name,                         \
        .log       = test_log_##_basename                       \
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
        switch (i) {                                                \
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
        return true;                                                \
    }                                                               \
    struct fake

#define DEFINE_SEQ_ENUMERATOR(_name, _type, _tv, ...)               \
    static must_use no_null_args                                    \
    bool test_enum_##_name(unsigned i, test_value_t *dest)          \
    {                                                               \
        static const _type vals[] = {__VA_ARGS__};                  \
                                                                    \
        if (i >= sizeof(vals) / sizeof(*vals))                      \
            return false;                                           \
                                                                    \
        dest->_tv = vals[i];                                        \
        return true;                                                \
    }                                                               \
    struct fake

#define DEFINE_SCALED_ENUMERATOR(_name, _type, _tv, _min)           \
    static must_use no_null_args                                    \
    bool test_enum_##_name(unsigned i, test_value_t *dest)          \
    {                                                               \
        if (i >= test_sample_scale)                                 \
            return false;                                           \
                                                                    \
        dest->_tv = _min * test_sample_scale / 2 + (_type)i;        \
                                                                    \
        return true;                                                \
    }                                                               \
    struct fake

typedef no_null_args void test_property_t(const test_value_t vals[]);

typedef struct testcase_t {
    const char                     *title;
    bool                            enabled;
    bool                            expect_fail;
    const test_generator_t * const *params;
    test_property_t                 property;
} testcase_t;

static inline no_null_args must_use
unsigned testcase_count_params(const testcase_t *tc)
{
    unsigned i;

    for(i = 0; tc->params[i] != NULL; i++)
        ;
    
    return i;
}

static unsigned testcase_n_run;
static unsigned testcase_n_ok;

static no_null_args
void testcase_run(const testcase_t *tc)
{
    unsigned n_params = testcase_count_params(tc);
    test_value_t values[n_params];
    unsigned i;

    fprintf(stderr, "%s...", tc->title);
    fflush(stderr);
    if (!tc->enabled) {
        fputs("SKIP\n");
        return;
    }
    if (tc->property == NULL) {
        fputs("UNTESTED\n");
        return;
    }

    testcase_n_run++;
    for (i = 0; i < test_max_samples; i++)
    {
        unsigned j;
        
        for (j = 0; j < n_params; j++) {
            if (!tc->params->enumerate(i, &values[j]))
                break;
        }
        if (j < n_params)
            break;
        fputc('[', stderr);
        for (j = 0; j < n_params; j++) {
            if (j > 0)
                fputc(',', stderr);
            tc->params->log(&values[j]);
        }
        fputc(']', stderr);
        fflush(stderr);
    }
}


#define TEST_START(_name)                                       \
    (fputs("Checking " _name "...", stderr), fflush(stderr))

#define TEST_END()                              \
    (fputs("OK\n", stderr), fflush(stderr))


#define TEST_VARIANT_START(_name)                   \
    (fputs("[" _name "= ", stderr), fflush(stderr))

#define TEST_VARIANT_END()                      \
    (fputs("]", stderr), fflush(stderr))

#define TEST_FAILURE_INTRO(_msg)                                        \
    (fprintf(stderr, "%s:%d (%s): %s",                                  \
             __FILE__, __LINE__, __FUNCTION__, _msg),                   \
     fflush(stderr))

#define TEST_FAILURE_CONT(_msg) (fputs(_msg, stderr))
#define TEST_FAILURE_FMT(_fmt, ...) (fprintf(stderr, _fmt, __VA_ARGS__))
#define TEST_FAILURE_END() (fputs("\n", stderr), fflush(stderr))

#define TEST_FAILURE(_msg) (TEST_FAILURE_INRO(_msg), TEST_FAILUR_END())

struct PROP_list_t {
    const PROP_list_t *chain;
    void (*prop)(void);
};

static PROP_list_t *PROP_the_list;

#define PROPERTY(_name, body)                   \
    static void PROP_##_name(void)              \
    {                                           \
        TEST_START(#_name);                     \
        _body;                                  \
        TEST_END();                             \
    }                                           \
                                                \
    static constructor                          \
    void PROP_INIT_##_name(void)                \
    {                                           \
        static PROP_list_t current;             \
        current.prop = PROP_##_name;            \
        current.chain = PROP_the_list;          \
        PROP_the_list = &current;               \
    }                                           \
    struct fake

#define INVARIANT(_inv) ASSERT(_inv)

#define ASSERT(_cond)                                       \
    do {                                                    \
        if (!(_cond))                                       \
        {                                                   \
            TEST_FAILURE("Assertion '" #_cond "' failed");  \
            raise(SIGTRAP);                                 \
        }                                                   \
    } while(0)

#define EXPECT(_gen, _exp, _val)                            \
    do {                                                    \
        GEN_type_##_gen __expv = _exp;                      \
        GEN_type_##_gen __valv = _val;                      \
        if (!GEN_equal_##_gen(__expv, __valv))              \
        {                                                   \
            TEST_FAILURE_INTRO("Expected " #_val " = ");    \
            GEN_log_##_gen(__expv);                         \
            TEST_FAILURE_CONT(", but got ");                \
            GEN_log_##_gen(__valv);                         \
            TEST_FAILURE_END();                             \
            raise(SIGTRAP);                                 \
        }                                                   \
    } while(0)

#define EXPECT_CRASH(_sig, _body)                                   \
    do {                                                            \
        pid_t __child = fork();                                     \
                                                                    \
        if (__child == (pid_t)(-1))                                 \
        {                                                           \
            perror("Cannot fork");                                  \
            abort();                                                \
        }                                                           \
        else if (__child == 0)                                      \
        {                                                           \
            signal(SIGTRAP, SIG_DFL);                               \
            _body;                                                  \
            raise(SIGTRAP);                                         \
        }                                                           \
        else {                                                      \
            int __status = 0;                                       \
            if (waitpid(__child, &__status, 0) == (pid_t)(-1))      \
            {                                                       \
                perror("Cannot wait");                              \
                abort();                                            \
            }                                                       \
            if (WIFEXITED(__status))                                \
            {                                                       \
                TEST_FAILURE_INTRO("Expected kill by " #_sig);      \
                TEST_FAILURE_FMT(", but terminated with code %d",   \
                                 WEXITSTATUS(__status));            \
                TEST_FAILURE_END();                                 \
                raise(SIGTRAP);                                     \
            }                                                       \
            else if (WTERMSIG(__status) != _sig)                    \
            {                                                       \
                TEST_FAILURE_FMT("Expected kill by " #_sig);        \
                TEST_FAILURE_FMT", but actually killed by %d",                      \
                                 WTERMSIG(__status));               \
                raise(SIGTRAP);                                     \
            }                                                       \
        }                                                           \
    } while (0)


#define FOREACH(_gen, _var, _body)                                      \
    do {                                                                \
        for (size_t ITER_##_var##_index = 0;                            \
             ITER_##_var##_index < CHECK_n_samples;                     \
             ITER_##_var##_index++)                                     \
        {                                                               \
            GEN_type_##_gen _var;                                       \
            if (!GEN_generate_##_gen(ITER_##_var##_index, &_var))       \
                break;                                                  \
                                                                        \
            TEST_VARIANT_START(#_var);                                  \
            GEN_log_##_gen(_var);                                       \
            _body;                                                      \
            TEST_VARIANT_END();                                         \
        }                                                               \
    } while(0)


typedef bool GEN_type_bool;

static inline bool warn_unused_result warn_any_null_arg
GEN_generate_bool(size_t idx, bool *val)
{
    switch (idx)
    {
        case 0:
            *val = false;
            break;
        case 1:
            *val = true;
            break;
        default:
            return false;
    }
    return true;
}

static inline void
GEN_log_bool(bool val)
{
    fputs(val ? "true" : "false", stderr);
}

#define GEN_equal_bool(_v1, _v2) ((_v1) == (_v2))

static inline void warn_any_null_arg
GEN_random_bytes(void *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)buf)[i] = rand() % (UINT8_MAX + 1);
}

static inline void warn_any_null_arg
GEN_log_bytes(void *buf, size_t n)
{
    for (size_t i = 0; i < n; i++)
        fprintf(stderr, "%s%02x", i > 0 ? ":" : "", ((uint8_t *)buf)[i]);
}

#define TEST_SIMPLE_GENERATOR(_gen, _fmt)                       \
    static inline void                                          \
    GEN_log_##_gen(GEN_type_##_gen val)                         \
    {                                                           \
        fprintf(stderr, _fmt, val);                             \
    }                                                           \
    struct fake

#define TEST_TRIVIAL_EQ(_gen)                                   \
    static inline bool warn_unused_result                       \
    GEN_equal_##_gen(GEN_type_##_gen v1, GEN_type_##_gen v2)    \
    {                                                           \
        return v1 == v2;                                        \
    }                                                           \
    struct fake

#define TEST_DERIVED_GENERATOR(_gen, _basegen)                  \
    typedef GEN_type_##_basegen GEN_type##_gen;                 \
                                                                \
    static inline void                                          \
    GEN_log_##_gen(GEN_type_##_gen val)                         \
    {                                                           \
        GEN_log_##_basegen(val);                                \
    }                                                           \
                                                                \
    static inline bool warn_unused_result                       \
    GEN_equal_##_gen(GEN_type_##_gen v1, GEN_type_##_gen v2)    \
    {                                                           \
        return GEN_equal_##_basegen(v1, v2);                    \
    }                                                           \
    struct fake

#define TEST_SIGNED_INT_GENERATOR(_gen, _fmt, _min, _max)        \
    typedef _gen GEN_type_##_gen;                                \
                                                                 \
    static inline bool warn_unused_result warn_any_null_arg      \
    GEN_generate_##_gen(size_t idx, GEN_type_##_gen *result)     \
    {                                                            \
        switch (idx)                                             \
        {                                                        \
            case 0:                                              \
                *result = _min;                                  \
                break;                                           \
            case 1:                                              \
                *result = 0;                                     \
                break;                                           \
            case 2:                                              \
                *result = _max;                                  \
                break;                                           \
            default:                                             \
            {                                                    \
                GEN_random_bytes(result, sizeof(*result));       \
                if ((idx % 2 == 0) != (*result < 0))             \
                    *result = -*result;                          \
                break;                                           \
            }                                                    \
        }                                                        \
        return true;                                             \
    }                                                            \
                                                                 \
    TEST_TRIVIAL_EQ(_gen)                                        \
    TEST_SIMPLE_GENERATOR(_gen, _fmt)

#define TEST_UNSIGNED_INT_GENERATOR(_gen, _fmt, _max)           \
    typedef _gen GEN_type_##_gen;                               \
                                                                \
    static inline GEN_type_##_gen warn_any_null_arg             \
    GEN_generate_##_gen(size_t idx, GEN_type_##_gen *result)    \
    {                                                           \
        switch (idx)                                            \
        {                                                       \
            case 0:                                             \
                *result = 0;                                    \
                break;                                          \
            case 1:                                             \
                *result = _max;                                 \
                break;                                          \
            default:                                            \
                GEN_random_bytes(result, sizeof(*result));      \
        }                                                       \
        return true;                                            \
    }                                                           \
                                                                \
    TEST_SIMPLE_GENERATOR(_gen, _fmt)
    

TEST_SIGNED_INT_GENERATOR(int, "%d", INT_MIN, INT_MAX);
TEST_SIGNED_INT_GENERATOR(intmax_t, "%jd", INTMAX_MIN, INTMAX_MAX);

#define TEST_SIGNED_BITS_GENERATOR(_n)                              \
    TEST_SIGNED_INT_GENERATOR(int##_n##_t, "%" PRId##_n,            \
                              INT##_n##_MIN, INT##_n##_MAX)

TEST_SIGNED_BITS_GENERATOR(8);
TEST_SIGNED_BITS_GENERATOR(16);
TEST_SIGNED_BITS_GENERATOR(32);
TEST_SIGNED_BITS_GENERATOR(64);

#undef TEST_SIGNED_BITS_GENERATOR

TEST_UNSIGNED_INT_GENERATOR(unsigned, "%u", UINT_MAX);
TEST_UNSIGNED_INT_GENERATOR(uintmax_t, "%ju", UINTMAX_MAX);
TEST_UNSIGNED_INT_GENERATOR(size_t, "%zu", SIZE_MAX);

#define TEST_UNSIGNED_BITS_GENERATOR(_n)                              \
    TEST_SIGNED_INT_GENERATOR(uint##_n##_t, "%" PRIu##_n,             \
                              UINT##_n##_MAX)

TEST_UNSIGNED_BITS_GENERATOR(8);
TEST_UNSIGNED_BITS_GENERATOR(16);
TEST_UNSIGNED_BITS_GENERATOR(32);
TEST_UNSIGNED_BITS_GENERATOR(64);

#undef TEST_UNSIGNED_BITS_GENERATOR

#define TEST_SMALL_INT_GENERATOR(_gen, _basetype, _fmt, _min, _max) \
    typedef _basetype GEN_type_##_gen;                              \
                                                                    \
    static inline bool warn_unused_result warn_any_null_arg         \
    GEN_generate_##_gen(size_t idx, GEN_type_##_gen *result)        \
    {                                                               \
        if (idx > (_max) - (_min))                                  \
            return false;                                           \
        *result = (_min) + (_basetype)(idx);                        \
        return true;                                                \
    }                                                               \
                                                                    \
    TEST_TRIVIAL_EQ(_gen)                                           \
    TEST_SIMPLE_GENERATOR(_gen, _fmt)

TEST_SMALL_INT_GENERATOR(small_int, int, "%d", -5, 5);
TEST_SMALL_INT_GENERATOR(small_unsigned, unsigned, "%u", 0,
                         (unsigned)CHECK_sample_scale);
TEST_SMALL_INT_GENERATOR(all_digits, char, "%c", '0', '9');
    

#define TEST_RANGE_GENERATOR(_gen, _basetype, _fmt, ...)                \
    typedef _basetype GEN_type_##_gen;                                  \
                                                                        \
    static inline bool warn_unused_result warn_any_null_arg             \
    GEN_generate_##_gen(unused size_t idx, GEN_type_##_gen *result)     \
    {                                                                   \
        static const GEN_type_##_gen ranges[][2] =                      \
            {__VA_ARGS__};                                              \
        unsigned nr = rand() % (sizeof(ranges) / sizeof(*ranges));      \
                                                                        \
        *result = ranges[nr][0] + rand() %                              \
            (ranges[nr][1] - ranges[nr][0] + 1);                        \
        return true;                                                    \
    }                                                                   \
                                                                        \
    TEST_TRIVIAL_EQ(_gen)                                               \
    TEST_SIMPLE_GENERATOR(_gen, _fmt)

TEST_RANGE_GENERATOR(ascii, char, "%2.2x", 
                     {'\1', '\037'}, 
                     {' ', ' '},
                     {'!', '/'},
                     {'0', '9'},
                     {':', '@'},
                     {'A', 'Z'},
                     {'[', '`'},
                     {'a', 'z'},
                     {'{', '~'},
                     {'\177', '\177'});                     

TEST_RANGE_GENERATOR(digit, char, "%c", 
                     {'0', '9'});
TEST_RANGE_GENERATOR(xdigit, char, "%c", 
                     {'0', '9'}, {'a', 'f'}, {'A', 'F'});
TEST_RANGE_GENERATOR(alpha, char, "%c", 
                     {'a', 'z'}, {'A', 'Z'});
TEST_RANGE_GENERATOR(alnum, char, "%c", 
                     {'0', '9'}, {'a', 'z'}, {'A', 'Z'});

TEST_RANGE_GENERATOR(latin1, unsigned char, "%2.2x",
                     {'\1', '\037'},
                     {' ', ' '},
                     {'!', '~'},
                     {'\x7F', '\x9F'},
                     {'\xA0', '\xA0'},
                     {'\xA1', '\xBF'},
                     {'\xC0', '\xD6'},
                     {'\xD7', '\xD7'},
                     {'\xD8', '\xDE'},
                     {'\xDF', '\xF6'},
                     {'\xF7', '\xF7'},
                     {'\xF8', '\xFF'});

TEST_RANGE_GENERATOR(unicode, wchar_t, "U+%8.8x",
                     {L'\1', L'\x7F'},
                     {L'\u0080', L'\u00ff'},
                     {L'\u0100', L'\ud7ff'},
                     {L'\ud800', L'\udfff'},
                     {L'\ue000', L'\uf8ff'},
                     {L'\uf900', L'\uffef'},
                     {L'\ufff0', L'\uffff'},
                     {L'\U10000', L'\U1F8FF'},
                     {L'\U20000', L'\U2FA1F'},
                     {L'\UE0000', L'\UE007F'},
                     {L'\
                     
                     
                     
                     

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
