/** @file
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */

#define THE_MODULE "selftest"
#include "check.h"

TESTCASE(trivial, "Trivial test", true, false, TCLASS(NORMAL), 0, _,
         { CLASSIFY(NORMAL); },
         &test_every_bool);

TESTCASE(fail, "Failing test", true, true, TCLASS(NORMAL), 0, _,
         { ASSERT(false); },
         &test_every_bool);

TESTCASE(skip, "Skipped test", false, false, TCLASS(NORMAL), 0, _,
         { ASSERT(false); },
         &test_every_bool);

TESTCASE(trivial_bool_compare, "Trivial comparison of booleans",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(bool, values[0], values[0]); },
         &test_every_bool);

TESTCASE(trivial_int_compare, "Trivial comparison of integers",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(int, values[0], values[0]); },
         &test_every_int);

TESTCASE(trivial_int_fail_compare, "Failed comparison of integers",
         true, false, TCLASS(NORMAL), 0, values,
         { ASSERT(!test_compare_values(__tc, &test_every_int,
                                       values[0],
                                       TESTVAL(i, values[0].i + 1))); },
         &test_every_int);

TESTCASE(trivial_small_int_compare, "Trivial comparison of small integers",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(small_int, values[0], values[0]); },
         &test_every_small_int);

TESTCASE(trivial_intmax_t_compare, "Trivial comparison of large integers",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(intmax_t, values[0], values[0]); },
         &test_every_intmax_t);

TESTCASE(trivial_byte_compare, "Trivial comparison of bytes",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(byte, values[0], values[0]); },
         &test_every_byte);

TESTCASE(trivial_char_compare, "Trivial comparison of chars",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(char, values[0], values[0]); },
         &test_every_char);

TESTCASE(trivial_string_compare, "Trivial comparison of strings",
         true, false, TCLASS(NORMAL), 0, values,
         { EXPECT(string, values[0], values[0]); },
         &test_every_string);

TESTCASE(iterate_bool_pairs, "Iterate over pairs of bools", true, false,
         TCLASS(NORMAL), 0, _,
         { ASSERT(true); },
         &test_every_bool, &test_every_bool);

TESTCASE(iterate_small_int_pairs, "Iterate over pairs of small ints",
         true, false, TCLASS(NORMAL), 0, _,
         { ASSERT(true); },
         &test_every_small_int, &test_every_small_int);

TESTCASE(iterate_int_triples, "Iterate over triples of ints", true, false,
         TCLASS(NORMAL), 0, _,
         { ASSERT(true); },
         &test_every_int, &test_every_int, &test_every_int);

TESTCASE(iterate_doubles, "Iterate over doubles", true, false,
         TCLASS(NORMAL), 0, values,
         { ASSERT(isfinite(values[0].d)); },
         &test_every_double);

TESTCASE(iterate_xdoubles, "Iterate over extended doubles", true, false,
         TCLASS(NORMAL) | TCLASS(SPECIAL), 0, values,
         { ASSERT(true);
             if (isfinite(values[0].d))
                 CLASSIFY(NORMAL);
             else
                 CLASSIFY(SPECIAL);
         },
         &test_every_xdouble);

TESTCASE(check_digit, "Check that digits are digits",
         true, false, TCLASS(NORMAL), 0, values,
         { ASSERT(isdigit((char)values[0].ch)); },
         &test_every_digit);

TESTCASE(check_alnum, "Check that alphanumerics are alphanumerics",
         true, false, TCLASS(BUCKET0) | TCLASS(BUCKET1), 0, values,
         { ASSERT(isalnum((char)values[0].ch));
             if (isdigit((char)values[0].ch))
                 CLASSIFY(BUCKET0);
             else
                 CLASSIFY(BUCKET1);
         },
         &test_every_alnum);        

TESTCASE(check_digits, "Check that digit strings contain only digits",
         true, false, TCLASS(NORMAL), TCLASS(TRIVIAL), values,
         {
             const char *s;
             for (s = values[0].s; *s != '\0'; s++)
             {
                 ASSERT(isdigit(*s));
             }
             if (*values[0].s == '\0')
                 CLASSIFY(TRIVIAL);
         },
         &test_every_digits);

TESTCASE(iterate_wchar_t, "Iterate wide chars", true, false,
         TCLASS(NORMAL), 0, values,
         { EXPECT(wchar_t, values[0], values[0]); },
         &test_every_wchar_t);


TESTCASE(test_insufficient_classes, "Iterate with too few observed classes",
         true, false, TCLASS(NORMAL) | TCLASS(SPECIAL) | TCLASS(TRIVIAL), 0,
         _, { ASSERT(true); },
         &test_every_bool);


TESTCASE(test_unexpected_classes, "Iterate with unexpected observed classes",
         true, false, 0, TCLASS(NORMAL),
         _, { ASSERT(true); CLASSIFY(SPECIAL); },
         &test_every_bool);


TESTCASE(test_stdout_redirect, "Redirect stdout", true, false,
         TCLASS(NORMAL), TCLASS(TRIVIAL), values,
         {
             test_value_t result;

             if (!*values[0].s)
                 CLASSIFY(TRIVIAL);
             else
             {
                 REDIRECT_STDOUT(&result, 
                                 {
                                     fputs(values[0].s, stdout);
                                     fflush(stdout);
                                 });
                 EXPECT(pstring, result, values[0]);
                 ASSERT(fputs("<should be printed>\n", stdout) != EOF);
             }
         },
         &test_every_pstring);

TESTCASE(test_stdout_stderr_redirect, "Redirect stdout and stderr", true, false,
         TCLASS(NORMAL), TCLASS(TRIVIAL), values,
         {
             test_value_t result1;
             test_value_t result2;

             if (!*values[0].s || !*values[1].s)
                 CLASSIFY(TRIVIAL);
             else
             {
                 REDIRECT_STDOUT(&result1,
                                 REDIRECT_STDERR(&result2, 
                                 {
                                     fputs(values[0].s, stdout);
                                     fflush(stdout);
                                     fputs(values[1].s, stderr);
                                     fflush(stderr);
                                 }));
                 EXPECT(pstring, result1, values[0]);
                 ASSERT(fputs("<should be printed>\n", stdout) != EOF);
                 EXPECT(pstring, result2, values[1]);
                 ASSERT(fputs("<should be printed to stderr>\n", stderr) != EOF);
             }
         },
         &test_every_pstring, &test_every_pstring);
         
TESTCASE(test_stdin_redirect, "Redirect stdin", true, false,
         TCLASS(NORMAL), 0, values,
         {
             REDIRECT_STDIN(values[0],
                            {
                                char buf[strlen(values[0].s) + 1];
                                fgets(buf, (int)sizeof(buf), stdin);
                                EXPECT(pstring, values[0], TESTVAL(s, buf));
                            });
         },
         &test_every_pstring);

TESTCASE(test_subprocess_exit, "Test subprocess exit status", true, false,
         TCLASS(NORMAL), 0, values,
         {
             test_value_t is_exit;
             test_value_t exit_status;
             
             ISOLATED(&is_exit, &exit_status,
                      {
                          exit((int)values[0].u);
                      }
                 );
             ASSERT(is_exit.b);
             EXPECT(small_unsigned, values[0], exit_status);
         },
         &test_every_small_unsigned);

DEFINE_SEQ_ENUMERATOR(signal, int, i, SIGTERM, SIGKILL, SIGHUP,
                      SIGINT, SIGUSR1, SIGUSR2);
DEFINE_TRIVIAL_COMPARE(signal, i);

static NO_NULL_ARGS
void test_log_signal(test_value_t v, test_log_buffer_t *dest)
{
    test_log_string_into(dest, sys_siglist[v.i]);
}

DEFINE_GENERATOR_RECORD(signal, signal, signal, signal);

TESTCASE(subprocess_signal, "Test subprocess signal", true, false,
         TCLASS(NORMAL), 0, values,
         {
             test_value_t is_exit;
             test_value_t termsig;
             
             ISOLATED(&is_exit, &termsig,
                      {
                          raise((int)values[0].i);
                      }
                 );
             ASSERT(!is_exit.b);
             EXPECT(signal, values[0], termsig);
         },
         &test_every_signal);

TESTCASE(match, "Test MATCH", true, false,
         TCLASS(NORMAL), 0, values,
         {
             MATCH("*", values[0]);
         },
         &test_every_string);

TESTCASE(no_match, "Test NO_MATCH", true, false,
         TCLASS(NORMAL), 0, values,
         {
             NO_MATCH("[0-9]*", values[0]);
         },
         &test_every_alphas);

TESTCASE(expect_fmt, "Test EXPECT_FMT", true, false,
         TCLASS(NORMAL), 0, values,
         {
             char buf[128];
             snprintf(buf, sizeof(buf), "Test %s.%d",
                      values[0].s, (int)values[1].i);
             EXPECT_FMT(TESTVAL(s, buf), "Test %s.%d",
                        values[0].s, (int)values[1].i);
         },
         &test_every_string,
         &test_every_int);
