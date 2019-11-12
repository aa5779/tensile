/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
#define _GNU_SOURCE 1
#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <talloc.h>
#include "testing.h"

void
tnt_compare_mem(size_t len1, const void *mem1,
                size_t len2, const void *mem2,
                const char *msg, int line)
{
    size_t i;
    const uint8_t *bytes1, *bytes2;
    bool failed = false;

    if (len1 != len2)
    {
        tnt_log("Assertion failed: %s: %zu != %zu @ %d",
                msg, len1, len2, line);
        _Exit(TNT_EXIT_FAIL);
    }

    if (len1 != 0)
    {
        if (mem1 == NULL || mem2 == NULL)
        {
            tnt_log("Assertion failed: %s: NULL pointer @ %d",
                    msg, line);
            _Exit(TNT_EXIT_FAIL);
        }
    }

    bytes1 = mem1;
    bytes2 = mem2;
    for (i = 0; i < len1; i++)
    {
        if (bytes1[i] != bytes2[i])
        {
            if (!failed)
                tnt_log("Assertion failed: %s @ %d", msg, line);
            tnt_log("\t%zu: %02x != %02x", i, bytes1[i], bytes2[i]);
            failed = true;
        }
    }
    if (failed)
        _Exit(TNT_EXIT_FAIL);
}

bool tnt_mark_as_trivial = false;

tnt_test_descr *tnt_test_descr_first = NULL;
tnt_test_descr **tnt_test_descr_last = &tnt_test_descr_first;

static void
fail_on_leaks(void)
{
    size_t leaked = talloc_total_size(NULL);

    if (leaked != 0)
    {
        tnt_log("Memory leak detected: %zu bytes", leaked);
        _Exit(TNT_EXIT_FAIL);
    }
}

static void
abort_on_talloc(const char *msg)
{
    tnt_log("%s", msg);
    _Exit(TNT_EXIT_FAIL);
}

noreturn static void
test_function(const tnt_test_descr *descr,
              unsigned long ntest, unsigned i)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec ^ ntest);
    if (descr->expect_signal == SIGABRT)
        setrlimit(RLIMIT_CORE, &(struct rlimit){0, 0});
    talloc_enable_leak_report();
    talloc_set_log_fn(abort_on_talloc);
    descr->test(i);
    fail_on_leaks();
    exit(tnt_mark_as_trivial ? TNT_EXIT_TRIVIAL : TNT_EXIT_OK);
}

int
main(void)
{
    const char *scale_str = getenv("TNT_TESTS_SCALE");
    unsigned long scale = (scale_str == NULL ? TNT_TESTS_SCALE :
                           strtoul(scale_str, NULL, 10));
    unsigned long ntests = 0;
    tnt_test_descr *iter;
    unsigned long testno = 1;

    for (iter = tnt_test_descr_first; iter != NULL; iter = iter->next)
    {
        if (iter->iterated)
            ntests += scale;
        else
            ntests++;
    }
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
    printf("1..%lu\n", ntests);

    for (iter = tnt_test_descr_first; iter != NULL; iter = iter->next)
    {
        unsigned i;

        for (i = 0; i < (iter->iterated ? scale : 1); i++, testno++)
        {
            pid_t child = fork();
            if (child == (pid_t)-1)
                tnt_bailout("fork() does not work");
            else if (child == 0)
                test_function(iter, testno, i);
            else
            {
                int status;
                bool failed = true;

                if (waitpid(child, &status, 0) != child)
                    tnt_bailout("waitpid() failed");

                if (iter->cleanup != NULL)
                    iter->cleanup();
                if (WIFSIGNALED(status))
                {
                    failed = WTERMSIG(status) != iter->expect_signal;
                    if (failed)
                    {
                        tnt_log("Killed by signal %d (%s)", WTERMSIG(status),
                                strsignal(WTERMSIG(status)));
                    }
                }
                else if (WIFEXITED(status))
                {
                    switch (WEXITSTATUS(status))
                    {
                        case TNT_EXIT_BAILOUT:
                            exit(TNT_EXIT_BAILOUT);
                            break;
                        case TNT_EXIT_TRIVIAL:
                            printf("ok %lu %s # SKIP Trivial\n", testno,
                                   iter->descr);
                            continue;
                        default:
                            failed = iter->expect_signal != 0 ||
                                WEXITSTATUS(status) != iter->expect_status;
                            break;
                    }
                }
                printf("%s %lu %s%s\n", failed ? "not ok" : "ok", testno,
                       iter->descr, iter->expect_fail ? " # TODO" : "");
            }
        }
    }
    return 0;
}
