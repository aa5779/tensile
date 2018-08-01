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
#include <errno.h>
#include <string.h>
#include "compiler.h"

#ifndef THE_COMPONENT
#error "THE_COMPONENT should be defined"
#endif

#ifndef THE_MODULE
#error "THE_MODULE should be defined"
#endif

/**
 * Generic return status, 0 always means success
 */
typedef unsigned tn_status;

typedef struct tn_error_table_t {
    unsigned                 namespace;
    const char            *(*errfunc)(int errcode);
    unsigned                 n_errs;
    const char      * const *errs;
    struct tn_error_table_t *chain;
} tn_error_table_t;

#define TN_ERROR_NS_DYNAMIC ((unsigned)(~0))

#define TN_STATUS(_ns, _code) (((tn_status)(_ns) << 16) | (tn_status)(_code))

#define TN_STATUS_NS(_status) (((_status) >> 16) & 0xffffu)
#define TN_STATUS_CODE(_status) ((int)((_status) & 0xffffu))

/**
 * Register an error table
 */
NO_NULL_ARGS
extern void tn_register_error_table(tn_error_table_t *table);

MUST_USE
extern const char *tn_error_string(tn_status status);

/**
 * Error severity levels
 */
enum tn_severity {
    TN_FATAL,     /*< Fatal error, abort */
    TN_EXCEPTION, /*< Exception, jump to the handler or abort */
    TN_ERROR,     /*< Recoverable error */
    TN_WARNING,   /*< Warning */
    TN_NOTICE,    /*< Important event notification */
    TN_INFO,      /*< Informational messages */
    TN_TRACE,     /*< Debug tracing */
};

/**
 * Messages with a severity greater than this value won't be reported
 */
extern enum tn_severity tn_verbosity_level;

extern bool tn_interactive_report;

/**
 * Report a status
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param fmt      printf-style format string
 * @param ...      Format arguments
 */
LIKE_PRINTF(5, 6) NOT_NULL_ARGS(5)
extern void tn_report_status(enum tn_severity severity,
                             const char *module,
                             tn_status status,
                             const char *action,
                             const char *fmt, ...);


#define TN_REPORT_STATUS_MODULE THE_COMPONENT ":" THE_MODULE

#define __TN_REPORT_STATUS_WITH_CHECK(_report)                          \
    do {                                                                \
        static_assert(sizeof(THE_COMPONENT) <= 11,                      \
                      "THE_COMPONENT is too long");                     \
        static_assert(sizeof(THE_MODULE) <= 15,                         \
                      "THE_MODULE is too long");                        \
        _report;                                                        \
    } while(0)

#define TN_REPORT_STATUS(_severity, _status, _action, ...)              \
    __TN_REPORT_STATUS_WITH_CHECK(tn_report_status((_severity),         \
                                                   TN_REPORT_STATUS_MODULE, \
                                                   (_status), (_action), \
                                                   __VA_ARGS__))

/**
 * Report a status (va_list version)
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param fmt      printf-style format string
 * @param args     Format arguments as va_list
 */
LIKE_PRINTF(5, 0) NOT_NULL_ARGS(5)
extern void tn_report_statusv(enum tn_severity severity,
                              const char *module,
                              tn_status status,
                              const char *action,
                              const char *fmt, va_list args);

/**
 * Equivalent of `tn_report_status(TN_FATAL, ...)`
 */
LIKE_PRINTF(3, 4) NOT_NULL_ARGS(3)
extern noreturn void tn_fatal_error(const char *module, tn_status status,
                                    const char *fmt, ...);

#define TN_FATAL_ERROR(_status, ...)                                    \
    __TN_REPORT_STATUS_WITH_CHECK(tn_fatal_error(TN_REPORT_STATUS_MODULE, \
                                                 (_status), __VA_ARGS__))

/**
 * Equivalent of `tn_report_status(TN_EXCEPTION, ...)`
 */
LIKE_PRINTF(4, 5) NOT_NULL_ARGS(3)
extern noreturn void tn_throw_exception(const char *module, tn_status status,
                                        const char *action, const char *fmt,
                                        ...);

#define TN_THROW(_status, _action, ...)                                 \
    __TN_REPORT_STATUS_WITH_CHECK(tn_report_status(TN_REPORT_STATUS_MODULE, \
                                                   (_status), (_action), \
                                                   __VA_ARGS__))

#define TN_INTERNAL_ERROR(_severity, _status, _fmt, ...)                \
    TN_REPORT_STATUS(_severity, _status, "%s():%d: " _fmt,              \
                     __FUNCTION__, __LINE__, __VA_ARGS__)

/**
 * Type for error handlers
 */
typedef tn_status (*tn_exception_handler)(void *data, const char *module,
                                          tn_status status,
                                          const char *action,
                                          const char *msg);

/**
 * Execute @a action guarded by the exception handler.
 * If @a handler is specified, it is executed after the exception is
 * throw with the originating module, status code, and formatted message.
 * The status code of the handler is returned
 * @note If an exception is thrown from @a handler, it will be called again,
 * so care must be taken as not to enter an infinite loop
 *
 * @param action   Main action
 * @param handler  Optional exception handler
 * @param data     User data to pass to @a action and @a handler
 */
MUST_USE NOT_NULL_ARGS(1)
extern tn_status tn_with_exception(tn_status (*action)(void *),
                                   tn_exception_handler handler,
                                   void *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* STATUS_H */
