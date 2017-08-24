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

#include <com_err.h>
#include <stdarg.h>
#include "compiler.h"

/**
 * Generic return status, 0 always means success
 */
typedef errcode_t tn_status;

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

/**
 * Report a status
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param fmt      printf-style format string
 * @param ...      Format arguments
 */
LIKE_PRINTF(4, 5)
extern void tn_report_status(enum tn_severity severity,
                             const char *module,
                             tn_status status,
                             const char *fmt, ...);

/**
 * Report a status (va_list version)
 *
 * @param severity Status severity
 * @param module   Identifying module (may be NULL)
 * @param status   Status code to report
 * @param fmt      printf-style format string
 * @param args     Format arguments as va_list
 */
LIKE_PRINTF(4, 0)
extern void tn_report_statusv(enum tn_severity severity,
                              const char *module,
                              tn_status status,
                              const char *fmt, va_list args);

/**
 * Equivalent of `tn_report_status(TN_FATAL, ...)`
 */
LIKE_PRINTF(3, 4)
extern noreturn void tn_fatal_error(const char *module, tn_status status,
                                    const char *fmt, ...);

/**
 * Equivalent of `tn_report_status(TN_EXCEPTION, ...)`
 */
LIKE_PRINTF(3, 4)
extern noreturn void tn_throw_exception(const char *module, tn_status status,
                                        const char *fmt, ...);

/**
 * Report an internal error via tn_report_status(), specifying source
 * code location
 */
#define tn_internal_error(_severity, _status, _fmt, ...)                \
    (tn_report_status(_severity, __FILE__, _status,                     \
                     "%s():%d: " _fmt, __FUNCTION__, __LINE__, __VA_ARGS__))

/**
 * Type for error handlers
 */
typedef tn_status (*tn_exception_handler)(void *data, const char *module,
                                          tn_status status, const char *msg);

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
