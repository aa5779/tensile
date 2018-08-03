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
#include <errno.h>
#include <sysexits.h>
#include <sys/wait.h>
#include <signal.h>
#define TN_COMPONENT "library"
#define TN_MODULE "status"
#include "status.h"
#include "utils.h"
#include "testing.h"

static tn_status_descr errno2status_descr(unsigned short code)
{
    enum tn_status_recover recover;

    switch (code)
    {
        case EINTR:
        case EAGAIN:
#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
        case EINPROGRESS:
            recover = TN_STATUS_RECOVER;
            break;
        default:
            recover = TN_STATUS_RECOVER_NA;
            break;
    }
    return (tn_status_descr){
        .origin = TN_STATUS_OS,
        .recover = recover,
        .def_severity = TN_SEV_ERROR,
        /* we hope strerror never returns a string with % */
        .details_fmt = strerror(code),
        .action = NULL,
        .refid = 0
    };
}

static const tn_status_namespace errno_namespace = {
    .id = TN_STATUS_NS_ERRNO,
    .describe = errno2status_descr,
    .chain = NULL
};

static tn_status_descr exit2status_descr(unsigned short code)
{
    tn_status_descr descr = {
        .action = NULL,
        .refid  = 0
    };

    if (WIFEXITED(code))
    {
        static const char * const exit_codes[] = {
            [EXIT_SUCCESS] = "Success",
            [EXIT_FAILURE] = "Failure",
            [EX_USAGE] = "Command line usage error",
            [EX_DATAERR] = "Data format error",
            [EX_NOINPUT] = "Cannot open input",
            [EX_NOUSER] = "Addressee unknown",
            [EX_NOHOST] = "Host name unknown",
            [EX_UNAVAILABLE] = "Service unavailable",
            [EX_SOFTWARE] = "Internal software error",
            [EX_OSERR] = "System error",
            [EX_OSFILE] = "Critical OS file missing",
            [EX_CANTCREAT] = "Can't create (user) output file",
            [EX_IOERR] = "Input/output error",
            [EX_TEMPFAIL] = "Temp failure",
            [EX_PROTOCOL] = "Remote error in protocol",
            [EX_NOPERM] = "Permission denied",
            [EX_CONFIG] = "Configuration error",
        };
        unsigned exit_code = (unsigned)WEXITSTATUS(code);
        descr.details_fmt = TN_ARRAY_GET(exit_codes, exit_code, NULL);
        descr.recover = exit_code == EX_TEMPFAIL ?
            TN_STATUS_RECOVER :
            TN_STATUS_RECOVER_NA;
        descr.origin = exit_code == EX_OSERR ? TN_STATUS_OS : TN_STATUS_EXT;
        descr.def_severity = exit_code == EXIT_SUCCESS ?
            TN_SEV_INFO : TN_SEV_ERROR;
    }
    else
    {
        unsigned sig = WTERMSIG(code);
        descr.details_fmt = strsignal((int)sig);
        descr.recover = TN_STATUS_RECOVER_NA;
        descr.origin = TN_STATUS_EXT;
        descr.def_severity = sig == SIGTERM ? TN_SEV_INFO : TN_SEV_ERROR;
    }

    return descr;
}

static const tn_status_namespace exit_namespace = {
    .id = TN_STATUS_NS_EXIT,
    .describe = exit2status_descr,
    .chain = &errno_namespace
};

static tn_status_descr internal_descr(unsigned short code)
{
    static const tn_status_descr table[] = {
        [TN_STATUS_INTERNAL_MSG] = {
            .origin = TN_STATUS_APP,
            .recover = TN_STATUS_RECOVER_NA,
            .def_severity = TN_SEV_ERROR,
            .details_fmt = "%s(): %s:%d %s",
        },
        [TN_STATUS_INTERNAL_ENTRY] = {
            .origin = TN_STATUS_APP,
            .recover = TN_STATUS_RECOVER,
            .def_severity = TN_SEV_TRACE,
            .details_fmt = "Enter %s()",
        },        
        [TN_STATUS_INTERNAL_EXIT] = {
            .origin = TN_STATUS_APP,
            .recover = TN_STATUS_RECOVER,
            .def_severity = TN_SEV_TRACE,
            .details_fmt = "Exit %s()"
        },
        [TN_STATUS_INTERNAL_EXIT_RC] = {
            .origin = TN_STATUS_APP,
            .recover = TN_STATUS_RECOVER,
            .def_severity = TN_SEV_TRACE,
            .details_fmt = "Exit %s(), rc=%04x:%04x"
        }
    };
    assert(code < TN_ARRAY_SIZE(table));
    return table[code];
}

static const tn_status_namespace internal_namespace = {
    .id = TN_STATUS_NS_INTERNAL,
    .describe = internal_descr,
    .chain = &exit_namespace
};

static const tn_status_namespace *registered_namespaces = &internal_namespace;

static const tn_status_namespace *find_namespace(unsigned short id)
{
    const tn_status_namespace *ns;

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
    if (ns->id == TN_STATUS_NS_DYNAMIC)
    {
        ns->id = DYNAMIC_NS_BIT | seqno++;
    }
    else
    {
        assert((ns->id & DYNAMIC_NS_BIT) == 0);
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
TN_START_TEST(system_errno)
{
    const char *msg = tn_describe_status(TN_ERRNO2STATUS(_i)).details_fmt;
    ck_assert_ptr_ne(msg, NULL);
    ck_assert_str_eq(msg, strerror(_i));
}
TN_END_TEST;

TN_START_TEST(signal_names)
{
    const char *msg =
        tn_describe_status(TN_STATUS(TN_STATUS_NS_EXIT,
                                     (unsigned short)__W_EXITCODE(0, _i))).details_fmt;
    ck_assert_ptr_ne(msg, NULL);
    ck_assert_str_eq(msg, strsignal(_i));
}
TN_END_TEST;

TN_START_TEST(generic_exit_failure)
{
    const char *msg =
        tn_describe_status(TN_STATUS(TN_STATUS_NS_EXIT,
                                     (unsigned short)__W_EXITCODE(EXIT_FAILURE, 0))).details_fmt;
    ck_assert_ptr_ne(msg, NULL);
    ck_assert_str_eq(msg, "Failure");
}
TN_END_TEST;


TN_START_TEST(unknown_status)
{
    ck_assert_ptr_eq(tn_describe_status(TN_STATUS(0x7fffu, 0xffffu)).details_fmt, NULL);
}
TN_END_TEST;

TN_TESTCASE(status_format,
            TN_TEST_RANGE(system_errno, 1, sys_nerr - 1),
            TN_TEST_RANGE(signal_names, 1, NSIG - 1),
            TN_TEST(generic_exit_failure),
            TN_TEST(unknown_status));

#endif

enum tn_severity tn_verbosity_level = TN_SEV_INFO;

bool tn_interactive_report = true;

/* this is required to circumvent GCC bug 77366 */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-format-attribute"
#endif
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
               recover_map[descr.recover],
               module == NULL ? MM_NULLLBL : module,
               severity_map[severity],
               msg_buf, descr.action, descr.refid ? tag_buf : MM_NULLTAG) ==
        MM_NOTOK)
    {
        abort();
    }

    if (severity == TN_SEV_FATAL)
        abort();
}
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

void
tn_report_status(enum tn_severity severity, const char *module,
                 tn_status status, ...)
{
    va_list args;
    va_start(args, status);

    tn_report_statusv(severity, module, status, args);
    
    va_end(args);
}

void
tn_fatal_error(const char *module, tn_status status, ...)
{
    va_list args;
    va_start(args, status);

    tn_report_statusv(TN_SEV_FATAL, module, status, args);
    
    va_end(args);
    abort();
}
