m4_divert(-1)
m4_changecom(`/*', `*/')
m4_define(`mkt_SUITE', `0')
m4_define(`mkt_TCASE', `0')

m4_define(`mkt_CLOSE', `m4_undivert(3)')
m4_define(`mkt_ONCLOSE', `m4_divert(3)')
m4_define(`mkt_TEXT', `m4_divert(0)')
m4_define(`mkt_DECL', `m4_divert(1)')
m4_define(`mkt_SETUP', `m4_divert(2)')
m4_define(`mkt_INSDECL', `m4_undivert(1)')
m4_define(`mkt_INSSETUP', `m4_undivert(2)')
m4_define(`mkt_INCR', `m4_define(`$1', m4_incr($1))')
m4_define(`mkt_ZERO', `m4_define(`$1', 0)')
m4_define(`mkt_INCRSUB', `mkt_INCR(`$1')`'mkt_ZERO(`$2')')

m4_define(`SUITE', `
mkt_CLOSE
mkt_INCRSUB(`mkt_SUITE', `mkt_TCASE')
mkt_DECL
    Suite *s`'mkt_SUITE = suite_create("$1");
m4_ifelse(mkt_SUITE, 1, `',
`
mkt_SETUP
srunner_add_suite(sr, s`'mkt_SUITE);
')
mkt_TEXT
#line m4___line__
')
m4_define(`TCASE', `
m4_ifelse(mkt_SUITE, `0', `SUITE(`Core')')
mkt_CLOSE
mkt_INCRSUB(`mkt_TCASE', `mkt_FIXTURE')m4_dnl
m4_define(`mkt_TCV', `tc`'mkt_SUITE`'_`'mkt_TCASE')
mkt_DECL
    TCase *mkt_TCV = tcase_create("$1");
mkt_SETUP
    suite_add_tcase(s`'mkt_SUITE, mkt_TCV);
    tcase_add_checked_fixture(mkt_TCV,
                              mkt_randomize, mkt_fail_on_leaks);
mkt_ONCLOSE
} mkt_TCV`'_struct;
static mkt_TCV`'_struct mkt_TCV`'_state;
mkt_TEXT
typedef struct mkt_TCV`'_struct {
    int _dummy;
#line m4___line__
')
m4_define(`mkt_BEFORE_TCASE', `
mkt_CLOSE
mkt_INCR(`mkt_FIXTURE')m4_dnl
#define mkt_TCV`'_teardown`'mkt_FIXTURE NULL
static void mkt_TCV`'_setup`'mkt_FIXTURE(void)
{
    TN_UNUSED mkt_TCV`'_struct *state = &mkt_TCV`'_state;
mkt_ONCLOSE
}
mkt_SETUP
    tcase_add_$1_fixture(mkt_TCV,
                         mkt_TCV`'_setup`'mkt_FIXTURE,
                         mkt_TCV`'_teardown`'mkt_FIXTURE);
mkt_TEXT
#line m4___line__
')
m4_define(`BEFORE_TCASE', `mkt_BEFORE_TCASE(`unchecked')')
m4_define(`BEFORE_TEST', `mkt_BEFORE_TCASE(`checked')')
m4_define(`AFTER_TCASE', `
mkt_CLOSE
#undef mkt_TCV`'_teardown`'mkt_FIXTURE
static void mkt_TCV`'_teardown`'mkt_FIXTURE(void)
{
    TN_UNUSED mkt_TCV`'_struct *state = &mkt_TCV`'_state;
mkt_ONCLOSE
}
mkt_TEXT
#line m4___line__
')
m4_define(`AFTER_TEST', `AFTER_TCASE')

m4_define(`mkt_LOOP_', `_loop')
m4_define(`mkt_LOOPARG_', `, 0, scale - 1')

m4_define(`mkt_LOOP_ONCE', `')
m4_define(`mkt_LOOPARG_ONCE', `')

m4_define(`mkt_LOOP_RANGE', `_loop')
m4_define(`mkt_LOOPARG_RANGE', `, $1, $2')

m4_define(`mkt_EXPECT_', `_test')
m4_define(`mkt_EXPECTARG_', `')

m4_define(`mkt_EXPECT_OK', `_test')
m4_define(`mkt_EXPECTARG_OK', `')

m4_define(`mkt_EXPECT_EXIT', `_exit_test')
m4_define(`mkt_EXPECTARG_EXIT', `, $1')

m4_define(`mkt_EXPECT_SIGNAL', `_test_raise_signal')
m4_define(`mkt_EXPECTARG_SIGNAL', `, SIG$1')

m4_define(`TEST', `
m4_ifelse(mkt_TCASE, `0', `TCASE(Core)')
mkt_CLOSE
mkt_SETUP
    tcase_add`'mkt_LOOP_$3`'mkt_EXPECT_$2`'(mkt_TCV, `$1'mkt_EXPECTARG_$2`'mkt_LOOPARG_$3);
mkt_ONCLOSE
}
END_TEST
mkt_TEXT
START_TEST(`$1')
{
    TN_UNUSED mkt_TCV`'_struct *state = &mkt_TCV`'_state;
#line m4___line__
')

m4_m4wrap(`
mkt_CLOSE
#define TN_TESTS_SCALE 100
int main(void)
{
    mkt_INSDECL
    SRunner *sr = srunner_create(s1);
    const char *scale_str = getenv("TN_TESTS_SCALE");
    long scale = (scale_str == NULL ? TN_TESTS_SCALE :
                                      strtol(scale_str, NULL, 10));

    talloc_enable_leak_report();
    talloc_set_log_fn(mkt_abort_on_talloc);
    mkt_INSSETUP
    srunner_set_tap (sr, "-");
    srunner_run_all(sr, CK_ENV);
    srunner_free(sr);

    return 0;
}
')
mkt_TEXT
`/*' Automatically generated from MKT_FILE */
/* DO NOT EDIT */

#include <stdlib.h>
#include <sys/time.h>
#include <talloc.h>
#include <check.h>
#include "compiler.h"

static void
mkt_randomize(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);
}

static void
mkt_fail_on_leaks(void)
{
    size_t leaked = talloc_total_size(NULL);

    ck_assert_uint_eq(leaked, 0);
}

static void
mkt_abort_on_talloc(const char *msg)
{
    ck_abort_msg(msg);
}

#line 1 "MKT_FILE"
