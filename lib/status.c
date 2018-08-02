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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <fmtmsg.h>
#define THE_COMPONENT "library"
#define THE_MODULE "status"
#include "status.h"
#include "utils.h"
#include "testing.h"

static tn_status_descr tn_errno2status_descr(unsigned short code)
{
    return (tn_status_descr){
        .origin = TN_STATUS_OS,
        .recover = TN_STATUS_RECOVER_NA,
        .def_severity = TN_SEV_ERROR,
        /* we hope strerror never returns a string with % */
        .details_fmt = strerror(code),
        .action = NULL,
        .refid = 0
    };
}

static const tn_status_namespace errno_namespace = {
    .id = TN_STATUS_NS_ERRNO,
    .describe = tn_errno2status_descr,
    .chain = NULL;
};

static const tn_status_namespace exit_namespace = {
    .id = TN_STATUS_NS_EXIT,
    .describe = tn_errno2status_descr,
    .chain = &errno_namespace;
};

static const tn_status_namespace internal_namespace = {
    .id = TN_STATUS_NS_INTERNAL,
    .describe = tn_errno2status_descr,
    .chain = &exit_namespace;
};

static const tn_status_namespace *registered_namespaces = &internal_namespace;

static const tn_status_namespace *find_namespace(unsigned short id)
{
    tn_status_namespace *ns;

    for (ns = registered_namespaces; ns != NULL; ns = ns->chain)
    {
        if (ns->id == id)
            return ns;
    }
    return NULL;
}

#define DYNAMIC_NS_BIT (1 << 15)

void tn_register_status_namespace(tn_status_namespace *ns)
{
    static unsigned short seqno;

    assert(ns->chain == NULL);
    if (table->id == TN_STATUS_NS_DYNAMIC)
    {
        table->id = DYNAMIC_NS_BIT | seqno++;
    }
    else
    {
        assert((table->ns & DYNAMIC_NS_BIT) == 0);
    }
    assert(find_namespace(ns->id) == NULL);

    ns->chain = registered_namespaces;
    registered_namespaces = ns;
}

tn_status_descr tn_describe_status(tn_status status)
{
    const tn_status_namespace *ns = find_namespace(status.ns);

    if (ns == NULL)
    {
        return (tn_status_descr){.origin = TN_STATUS_APP,
                .recover = TN_STATUS_RECOVER_NA,
                .def_severity = TN_SEV_UNDEFINED,
                .details_fmt = NULL,
                .action = NULL,
                .refid = 0u
                };
    }
    
    return ns->describe(status.code);
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

void tn_report_statusv(enum tn_severity severity, const char *module,
                       tn_status status, va_list args)
{
    static const int severity_map[] = {
        MM_NOSEV,
        MM_HALT,
        MM_ERROR,
        MM_WARNING,
        MM_INFO,
        MM_INFO,
        MM_INFO,
    };
    static const int origin_map[] = {
        MM_HARD | MM_OPSYS,
        MM_SOFT | MM_OPSYS,
        MM_SOFT | MM_UTIL,
        MM_SOFT | MM_APPL,
        MM_SOFT | MM_APPL,
    };
    static const int recover_map[] = {
        0,
        MM_RECOVER,
        MM_NRECOV,
    };
    tn_status_descr descr = tn_describe_status(status);
    char msg_buf[256];
    char tag_buf[128];

    if (severity == TN_SEV_UNDEFINED)
        severity = descr.def_severity;

    if (severity > tn_verbosity_level)
        return;

    if (descr.details_fmt == NULL)
    {
        snprintf(msg_buf, sizeof(msg_buf), "%04x:%04x",
                 status.ns, status.code);
    }
    else
    {
        vsnprintf(msg_buf, sizeof(msg_buf), descr.details_fmt, args);
    }
    
    if (descr.refid != 0)
    {
        snprintf(tag_buf, sizeof(tag_buf), "%s:%08x",
                 module != NULL ? module : "-:-",
                 descr.refid);
    }

    if (fmtmsg((tn_interactive_report ? MM_PRINT : MM_CONSOLE) |
               origin_map[descr.origin] |
               recover_map[desc.recover],
               module == NULL ? MM_NULLLBL : module,
               severity_map[severity],
               msg_buf, action, descr.refid ? tag_buf : MM_NULLTAG) ==
        MM_NOTOK)
    {
        abort();
    }

    if (severity == TN_SEV_FATAL)
        abort();
}

void
tn_report_status(enum tn_severity severity, const char *module,
                 tn_status status, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(severity, module, status, args);
    
    va_end(args);
}

void
tn_fatal_error(const char *module, tn_status status, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    tn_report_statusv(TN_SEV_FATAL, module, status, args);
    
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
