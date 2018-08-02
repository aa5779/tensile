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
 * @brief Unit testing support through Check framework
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef TESTING_H
#define TESTING_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#if DO_TESTS

#ifndef TN_MODULE
#error "TN_MODULE is unknown"
#endif

#include <check.h>
#include "compiler.h"
#include "utils.h"

#define TN_START_TEST(_name) START_TEST(test_##_name)
#define TN_END_TEST END_TEST struct fake

#define TN_START_TEST_FORALL(_name, _var, _type, ...)           \
    static _type test_##_name##_iterations[] = {__VA_ARGS__};   \
    TN_START_TEST(_name)                                        \
    _type _var = test_##_name##_iterations[_i];

GLOBAL_INIT(static Suite *, the_test_suite,
            the_test_suite = suite_create(TN_MODULE));

#define TN_TEST(_test) tcase_add_test(the_test_case, test_##_test)
#define TN_TEST_ABORT(_test) \
    tcase_add_test_raise_signal(the_test_case, test_##_test, SIGABRT)
#define TN_FIXTURE(_setup, _teardown)                           \
    tcase_add_checked_fixture(the_test_case, _setup, _teardown)
#define TN_GLOBAL_FIXTURE(_setup, _teardown)                        \
    tcase_add_unchecked_fixture(the_test_case, _setup, _teardown)
#define TN_TEST_FORALL(_test)                                           \
    tcase_add_loop_test(the_test_case, test_##_test,                    \
                        0, TN_ARRAY_SIZE(test_##_name##_iterations) - 1)
#define TN_TEST_RANGE(_test, _start, _end)      \
    tcase_add_loop_test(the_test_case, test_##_test, _start, _end)

#define TN_TESTCASE(_tcname, ...)                       \
    CONSTRUCTOR void _tcname##_init(void)               \
    {                                                   \
        TCase *the_test_case = tcase_create(#_tcname);  \
        suite_add_test_case(the_test_suite, the_test_case); \
        __VA_ARGS__;                                        \
    }                                                       \
    struct fake

int main(int argc, char *argv[])
{
    SRunner *runner = srunner_create(the_test_suite);
    int n_run;
    int n_failed;

    GC_INIT();
    srunner_run_all(runner, CK_ENV);
    n_failed = srunner_ntests_failed(runner);
    n_run = srunner_ntests_run(runner);
    if (n_failed > 0)
        fprintf(stderr, "%d of %d tests failed\n", n_failed, n_run);
    if (n_run == 0)
        fprintf(stderr, "WARNING: no tests have been run!");
    return n_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#else

/* nothing */

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CHECK_H */
