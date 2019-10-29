m4_divert(-1)
m4_changecom(`/*', `*/')
m4_define(`MKTESTS_SUITE', `0')

m4_define(`TN_SUITE', `
m4_undivert(3)
m4_define(`MKTESTS_SUITE', m4_incr(MKTESTS_SUITE))m4_dnl
m4_define(`MKTESTS_TCASE', `0')m4_dnl
m4_divert(1)
    Suite *s`'MKTESTS_SUITE = suite_create("$1");
m4_ifelse(MKTESTS_SUITE, 1, `',
`
m4_divert(2)
srunner_add_suite(sr, s`'MKTESTS_SUITE);
')
m4_divert(0)
')
m4_define(`TN_TCASE', `
m4_undivert(3)
m4_define(`MKTESTS_TCASE', m4_incr(MKTESTS_TCASE))m4_dnl
m4_define(`MKTESTS_FIXTURE', `0')m4_dnl
m4_divert(1)
    TCase *tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE = tcase_create("$1");
m4_divert(2)
    suite_add_tcase(s`'MKTESTS_SUITE, tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE);
    tcase_add_checked_fixture(tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE,
                              mktests_randomize, NULL);
m4_divert(0)
')
m4_define(`TN_BEFORE_TCASE', `
m4_undivert(3)
m4_define(`MKTESTS_FIXTURE', m4_incr(MKTESTS_FIXTURE))m4_dnl
#define tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown NULL
static void tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_setup(void)
{
m4_divert(3)
}
m4_divert(2)
    tcase_add_unchecked_fixture(tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE,
                                tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_setup,
                                tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown);
m4_divert(0)
')
m4_define(`TN_AFTER_TCASE', `
m4_undivert(3)
static void tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown(void)
{
m4_divert(3)
}
m4_divert(0)
')
m4_define(`TN_BEFORE_TEST', `
m4_undivert(3)
m4_define(`MKTESTS_FIXTURE', m4_incr(MKTESTS_FIXTURE))m4_dnl
#define tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown NULL
static void tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_setup(void)
{
m4_divert(3)
}
m4_divert(2)
    tcase_add_unchecked_fixture(tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE,
                                tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_setup,
                                tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown);
m4_divert(0)
')
m4_define(`TN_AFTER_TCASE', `
m4_undivert(3)
static void tc`'MKTESTS_SUITE`'_`'MKTESTS_TCASE`'_teardown(void)
{
m4_divert(3)
}
m4_divert(0)
')

m4_m4wrap(`
m4_undivert(3)
#define TN_TESTS_SCALE 100
int main(void)
{
    m4_undivert(1)
    SRunner *sr = srunner_create(s1);
    const char *scale_str = getenv("TN_TESTS_SCALE");
    long scale = (scale_str == NULL ? TN_TESTS_SCALE :
                                      strtol(scale_str, NULL, 10));

    m4_undivert(2)
    srunner_run_all(sr, CK_ENV);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);

    return nf == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
')
m4_divert(0)m4_dnl
`/*' Automatically generated from __file__ */
/* DO NOT EDIT */

#include <stdlib.h>
#include <sys/time.h>
#include <check.h>

static void
mktests_randomize(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);
}


#line 1 "__file__"
