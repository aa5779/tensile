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
#include <fmtmsg.h>
#define THE_COMPONENT "library"
#define THE_MODULE "status"
#include "status.h"
#include "testing.h"

static tn_error_table_t sys_error_table = {
    .namespace = 0,
    .errfunc   = (const char *(*)(int))strerror,
    .n_errs    = 0,
    .errs      = NULL,
    .chain     = NULL,
};

static tn_error_table_t *error_tables = &sys_error_table;
static unsigned dynamic_ns_seq = 0;

#define DYNAMIC_NS_BIT (1 << 15)

void tn_register_error_table(tn_error_table_t *table)
{
    tn_error_table_t *iter;

    assert(table->chain == NULL);
    if (table->namespace == TN_ERROR_NS_DYNAMIC)
    {
        table->namespace = DYNAMIC_NS_BIT | dynamic_ns_seq++;
    }
    for (iter = error_tables; iter != NULL; iter = iter->chain)
    {
        if (iter->namespace == table->namespace)
            abort();
    }

    table->chain = error_tables;
    error_tables = table;
}

const char *tn_error_string(tn_status status)
{
    tn_error_table_t *iter;
    unsigned ns = TN_STATUS_NS(status);
    const char *result = NULL;
    
    for (iter = error_tables; iter != NULL; iter = iter->chain)
    {
        if (iter->namespace == ns)
        {
            int code = TN_STATUS_CODE(status);

            if (iter->errfunc)
                result = iter->errfunc(code);
            if (result == NULL && iter->errs != NULL &&
                (unsigned)code < iter->n_errs)
                result = iter->errs[code];
            break;
        }
    }

    return result;
}

#if DO_TESTS
TESTCASE(system_errno, "System errno is correct", true, false,
         TCLASS(NORMAL), 0, values,
         {
             int err = (int)(values[0].u % 78 + 1);
             test_value_t s1 = {.s = strerror(err) };
             test_value_t s2 = {.s = tn_error_string((tn_status)err) };
             ASSERT(s2.s != NULL);
             EXPECT(string, s1, s2);
         },
         &test_every_uint8_t);

TESTCASE(unknown_status, "Unknown status reported as NULL", true, false,
         TCLASS(NORMAL), 0, values,
         {
             ASSERT(tn_error_string(TN_STATUS(values[0].u + 1,
                                              values[1].u)) == NULL);
         },
         &test_every_uint8_t,
         &test_every_uint8_t);
#endif

enum tn_severity tn_verbosity_level = TN_INFO;

bool tn_interactive_report = true;

static tn_status exception_code;
static volatile sigjmp_buf *exception_handler;
static char exception_details[256];
static const char *exception_origin;
static const char *exception_action;

void tn_report_statusv(enum tn_severity severity, const char *module,
                       tn_status status, const char *action,
                       const char *fmt, va_list args)
{
    static const int severity_map[] = {
        MM_HALT,
        MM_HALT,
        MM_ERROR,
        MM_WARNING,
        MM_INFO,
        MM_INFO,
        MM_NOSEV
    };
    char msg_buf[256];

    vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
    
    if (severity <= tn_verbosity_level)
    {
        char tag_buf[128];
        const char *errstr = tn_error_string(status);
        
        if (errstr != NULL)
        {
            snprintf(tag_buf, sizeof(tag_buf), "%s%s%s (%#x)",
                     module ? module : "",
                     module ? ": " : "", errstr, status);
        }
        else
        {
            snprintf(tag_buf, sizeof(tag_buf), "%s%s%#x",
                     module ? module : "",
                     module ? ": " : "",
                     status);
        }
        if (fmtmsg(tn_interactive_report ? MM_PRINT : MM_CONSOLE,
                   module, severity_map[severity],
                   msg_buf, action, tag_buf) == MM_NOTOK)
            abort();
    }

    switch (severity)
    {
        case TN_EXCEPTION:
            assert(status != 0);
            if (exception_handler)
            {
                strncpy(exception_details, msg_buf,
                        sizeof(exception_details) - 1);
                exception_code = status;
                exception_origin = module;
                exception_action = action;

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
                 tn_status status, const char *action, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(severity, module, status, action, fmt, args);
    
    va_end(args);
}

void
tn_fatal_error(const char *module, tn_status status, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(TN_FATAL, module, status, NULL, fmt, args);
    
    va_end(args);
    abort();
}

#if DO_TESTS

DEFINE_ENUM_ENUMERATOR(tn_severity, enum tn_severity, i,
                       TN_FATAL, TN_TRACE);
DEFINE_DERIVED_GENERATOR(tn_severity, int);

#define COMMA ,
TESTCASE(report_error, "Report errors when severity level is high enough",
         true, false, TCLASS(BUCKET0) | TCLASS(BUCKET1), TCLASS(TRIVIAL), values,
         {
             test_value_t output;
             enum tn_severity severity = (enum tn_severity)values[0].i;
             static const char * const severity_pfx[] =
                 {
                     [TN_FATAL] = "HALT: " COMMA
                     [TN_EXCEPTION] = "HALT: " COMMA
                     [TN_ERROR] = "ERROR: " COMMA
                     [TN_WARNING] = "WARNING: " COMMA
                     [TN_NOTICE] = "INFO: " COMMA
                     [TN_INFO] = "INFO: " COMMA
                     [TN_TRACE] = "" COMMA
                 };
             
             if (severity <= TN_EXCEPTION)
                 CLASSIFY(TRIVIAL);
             REDIRECT_STDERR(&output,
                             {
                                 tn_verbosity_level = severity -
                                     (values[1].b ? 0 : 1);
                                 TN_REPORT_STATUS(
                                         severity,
                                         ERANGE, NULL, "%s",
                                         values[2].s);
                             }
                 );
             if (values[1].b)
             {
                 EXPECT_FMT(output, "%s:%s: %s%s\n%s:%s: %s (%#x)\n",
                            THE_COMPONENT, THE_MODULE,
                            severity_pfx[severity],
                            values[2].s, THE_COMPONENT, THE_MODULE,
                            strerror(ERANGE), ERANGE);
                 CLASSIFY(BUCKET1);
             }
             else
             {
                 ASSERT(*output.s == '\0');
                 CLASSIFY(BUCKET0);
             }
         },
         &test_every_tn_severity,
         &test_every_bool,
         &test_every_pstring);

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
