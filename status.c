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
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */

#include <stdlib.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "status.h"
#include "check.h"

enum tn_severity tn_verbosity_level = TN_INFO;

static tn_status exception_code;
static volatile sigjmp_buf *exception_handler;
static char exception_details[256];
static const char *exception_origin;

void
tn_report_statusv(enum tn_severity severity, const char *module,
                  tn_status status, const char *fmt, va_list args)
{
    if ((severity != TN_EXCEPTION || !exception_handler) &&
        severity <= tn_verbosity_level)
    {
        com_err_va(module, status, fmt, args);
    }

    switch (severity)
    {
        case TN_EXCEPTION:
            assert(status != 0);
            if (exception_handler)
            {
                vsnprintf((char *)exception_details,
                          sizeof(exception_details),
                          fmt, args);
                exception_code = status;
                exception_origin = module;
                siglongjmp(*(sigjmp_buf *)exception_handler, 1);
            }
            /* fallthrough */
        case TN_FATAL:
            abort();
            break;
        default:
            /* do nothing */
            break;
    }
}

void
tn_report_status(enum tn_severity severity, const char *module,
                 tn_status status, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(severity, module, status, fmt, args);
    
    va_end(args);
}

void
tn_fatal_error(const char *module, tn_status status, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(TN_FATAL, module, status, fmt, args);
    
    va_end(args);
    abort();
}

void
tn_throw_exception(const char *module, tn_status status,
                   const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(TN_EXCEPTION, module, status, fmt, args);
    
    va_end(args);
    abort();
}

#if DO_TESTS

DEFINE_ENUM_ENUMERATOR(nonfatal_severity, enum tn_severity, i,
                       TN_ERROR, TN_TRACE);
DEFINE_DERIVED_GENERATOR(nonfatal_severity, int, TVCLASS(NORMAL), 0);

DEFINE_SEQ_ENUMERATOR(tn_status, tn_status, i, 0, EACCES, EINVAL);
DEFINE_DERIVED_GENERATOR(tn_status, int, TVCLASS(NORMAL), 0);

TESTCASE(report_error, "Report errors when severity level is high enough",
         true, false, values,
         {
             test_value_t output;
             REDIRECT_STDERR(&output,
                             {
                                 tn_verbosity_level =
                                     (enum tn_severity)values[0].i;
                                 tn_report_status(
                                         (enum tn_severity)values[0].i,
                                         "test",
                                         (tn_status)values[1].u, "<%s>",
                                         values[2].s);
                             }
                 );
             fprintf(stderr, "%s\n", output.s);
         },
         &test_every_nonfatal_severity,
         &test_every_tn_status,
         &test_every_alnums);

#if 0
static void test_fatal_error(bool is_fatal)
{
    pid_t child = fork();

    assert(child != (pid_t)(-1));
    if (child == 0)
    {
        if (is_fatal)
            tn_fatal_error("test", EFAULT, "test");
        else
            tn_throw_exception("test", EFAULT, "test");
        exit(0);
    }
    else
    {
        int status = 0;
        pid_t result = wait(&status);

        assert(result == child);
        assert(WIFSIGNALED(status));
        assert(WTERMSIG(status) == SIGABRT);
    }
}
#endif

#endif

tn_status
tn_with_exception(tn_status (*action)(void *),
                  tn_exception_handler handler,
                  void *data)
{
    volatile sigjmp_buf current_handler;
    volatile sigjmp_buf * volatile previous_handler = exception_handler;

    exception_handler = &current_handler;

    exception_code = 0;
    if (!sigsetjmp(*(sigjmp_buf *)exception_handler, true))
        exception_code = action(data);
    else
    {
        assert(exception_code != 0);
        if (handler)
        {
            tn_status rc = handler(data, exception_origin,
                                   exception_code, exception_details);
            if (rc == EAGAIN)
            {
                char rethrow_details[256];
                
                strcpy(rethrow_details, exception_details);
                exception_handler = previous_handler;
                tn_throw_exception(exception_origin, exception_code,
                                   "%s", rethrow_details);
            }
            exception_code = rc;
        }
    }

    exception_handler = previous_handler;
    
    return exception_code;
}

#if 0

static tn_status test_action1(unused void *arg)
{
    return 0;
}

static tn_status test_action2(unused void *arg)
{
    return EINVAL;
}

static tn_status test_action3(unused void *arg)
{
    tn_throw_exception("test", EINVAL, "test");
    return 0;
}

static tn_status test_nested_action(void *arg)
{
    tn_status status = tn_with_exception(test_action3, NULL, arg);
    assert(status == EINVAL);
    tn_throw_exception("test", EACCES, "test");
    return 0;
}

static tn_status test_handler(unused void *arg, const char *origin,
                              tn_status status,
                              const char *msg)
{
    tn_report_status(TN_ERROR, __FUNCTION__, status, "got %s from %s",
                     msg, origin);
    return EBADF;
}

static tn_status test_double_handler(unused void *arg, const char *origin,
                                     tn_status status,
                                     const char *msg)
{
    tn_report_status(TN_ERROR, __FUNCTION__, status, "got %s from %s", msg,
                     origin);
    if (status != EBADF)
        tn_throw_exception("test", EBADF, "test");
    return EACCES;
}

static tn_status test_rethrow_handler(unused void *arg, const char *origin,
                                      tn_status status,
                                      const char *msg)
{
    tn_report_status(TN_ERROR, __FUNCTION__, status, "got %s from %s", msg,
                     origin);
    return EAGAIN;
}

static tn_status test_rethrow_action(void *arg)
{
    tn_status status = tn_with_exception(test_action3, test_rethrow_handler,
                                         arg);
    assert(0);
    return status;
}


static void test_handle_exception(void)
{
    tn_status status;

    status = tn_with_exception(test_action1, NULL, NULL);
    assert(status == 0);

    status = tn_with_exception(test_action2, NULL, NULL);
    assert(status == EINVAL);

    status = tn_with_exception(test_action3, NULL, NULL);
    assert(status == EINVAL);

    status = tn_with_exception(test_action3, test_handler, NULL);
    assert(status == EBADF);

    status = tn_with_exception(test_action3, test_double_handler, NULL);
    assert(status == EACCES);

    status = tn_with_exception(test_nested_action, NULL, NULL);
    assert(status == EACCES);

    status = tn_with_exception(test_rethrow_action,
                               test_handler, NULL);
    assert(status == EBADF);
}

int main()
{
    test_simple_report();
    test_fatal_error(true);
    test_fatal_error(false);
    test_handle_exception();
    puts("OK");
    return 0;
}
#endif
