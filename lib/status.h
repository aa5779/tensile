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
 * @brief error handling routines
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef STATUS_H
#define STATUS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdbool.h>
#include "compiler.h"

#ifndef THE_COMPONENT
#error "THE_COMPONENT should be defined"
#endif

#ifndef THE_MODULE
#error "THE_MODULE should be defined"
#endif

typedef struct tn_status {
    unsigned short ns;
    unsigned short code;
} tn_status;

#define TN_STATUS_NS_DYNAMIC ((unsigned short)(~0))

#define TN_STATUS(_ns, _code)                           \
    ((struct tn_status){.ns = (_ns), .code = (_code)})

#define TN_IS_SUCCESS(_status)                      \
    (((_status).ns == 0) && ((_status).code) == 0))

/**
 * Status severity levels
 */
enum tn_severity {
    TN_SEV_UNDEFINED, /*< Undefined severity */
    TN_SEV_FATAL,     /*< Fatal error, abort */
    TN_SEV_ERROR,     /*< Recoverable error */
    TN_SEV_WARNING,   /*< Warning */
    TN_SEV_NOTICE,    /*< Important event notification */
    TN_SEV_INFO,      /*< Informational messages */
    TN_SEV_TRACE,     /*< Debug tracing */
};

/**
 * Error status classification
 */
enum tn_status_origin {
    TN_STATUS_HW,    /*< Hardware error */
    TN_STATUS_OS,    /*< Error reported by OS */
    TN_STATUS_EXT,   /*< Error reported by an external tool */
    TN_STATUS_APP,   /*< Application (internal) error */
    TN_STATUS_USER,  /*< User error */
};

/**
 * Error recoverability
 */
enum tn_status_recover {
    TN_STATUS_RECOVER_NA,    /*< Recoverability status unknown */
    TN_STATUS_RECOVER,       /*< Recoverable error */
    TN_STATUS_NO_RECOVER,    /*< Non-recoverable error */
};

/**
 * Status description
 */
typedef struct tn_status_descr {
    enum tn_status_origin origin;   /*< Error origin */
    enum tn_status_recover recover; /*< Recoverability status */
    enum tn_severity def_severity;  /*< Default severity */
    const char *details_fmt;        /*< Format string for details */
    const char *action;             /*< Action message */
    unsigned refid;                 /*< Reference ID */
} tn_status_descr;

/**
 * A function type to map from  a status code to its description
 */
typedef tn_error_descr (*tn_status_describer)(unsigned short code);

typedef struct tn_status_namespace {
    unsigned short id;                 /*< Status namespace ID */
    tn_status_describer describe;      /*< Status description function */
    const struct tn_status_namespace *chain; /*< Next status namespace */
} tn_status_namespace;

enum tn_status_base_namespace_id {
    TN_STATUS_NS_ERRNO,
    TN_STATUS_NS_EXIT,
    TN_STATUS_NS_INTERNAL,
};

/**
 * Register a status namespace
 */
NO_NULL_ARGS
extern void tn_register_status_namespace(tn_status_namespace *ns);

#define TN_REGISTER_STATUS_NS(_nsvar, _nsval, _describer)           \
    static tn_status_namespace _nsvar = {                           \
        .id = (_nsval),                                             \
        .describe = (_describer)                                    \
    };                                                              \
                                                                    \
    CONSTRUCTOR _nsvar##_init(void)                                 \
    {                                                               \
        tn_register_status_describer(&(_nsvar));                    \
    }                                                               \
    struct fake

MUST_USE
extern tn_status_descr tn_describe_status(tn_status status);

/**
 * Messages with a severity greater than this value won't be reported
 */
extern enum tn_severity tn_verbosity_level;

/**
 * A flag to indicate whether to report statuses to stderr or syslog
 */
extern bool tn_interactive_report;

/**
 * Report a status
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param ...      Format arguments
 */
extern void tn_report_status(enum tn_severity severity,
                             const char *module, tn_status status, ...);


#define TN_REPORT_STATUS_MODULE THE_COMPONENT ":" THE_MODULE

#define __TN_REPORT_STATUS_WITH_CHECK(_report)                          \
    do {                                                                \
        static_assert(sizeof(THE_COMPONENT) <= 11,                      \
                      "THE_COMPONENT is too long");                     \
        static_assert(sizeof(THE_MODULE) <= 15,                         \
                      "THE_MODULE is too long");                        \
        _report;                                                        \
    } while(0)

#define TN_REPORT_STATUS(_severity, _status, ...)                       \
    __TN_REPORT_STATUS_WITH_CHECK(tn_report_status((_severity),         \
                                                   TN_REPORT_STATUS_MODULE, \
                                                   (_status), __VA_ARGS__))

/**
 * Report a status (va_list version)
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param fmt      printf-style format string
 * @param args     Format arguments as va_list
 */
extern void tn_report_statusv(enum tn_severity severity,
                              const char *module,
                              tn_status status,
                              va_list args);

/**
 * Equivalent of `tn_report_status(TN_FATAL, ...)`
 */
extern noreturn void tn_fatal_error(const char *module, tn_status status, ...);

#define TN_FATAL_ERROR(_status, ...)                                    \
    __TN_REPORT_STATUS_WITH_CHECK(tn_fatal_error(TN_REPORT_STATUS_MODULE, \
                                                 (_status), __VA_ARGS__))


#define TN_INTERNAL_ERROR(_severity, _status, _fmt, ...)                \
    TN_REPORT_STATUS(_severity, _status, "%s():%d: " _fmt,              \
                     __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* STATUS_H */
