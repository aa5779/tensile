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
 * @brief dynamic strings
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef DSTRING_H
#define DSTRING_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include "utils.h"
#include "status.h"

typedef struct tn_string {
    size_t len;
    const char *str;
} tn_string;

#define TN_EMPTY_STRING ((tn_string){.len = 0, .str = NULL})

warn_unused_result
static inline tn_string
tn_cstr2str(const char *str)
{
    return (tn_string){.len = str ? strlen(str) : 0, .str = str};
}

#define TN_STRING_LITERAL(_literal) \
    ((tn_string){.len = sizeof(_literal) - 1, .str = _literal})

warn_unused_result
extern tn_string tn_strdup(const char *str);

warn_unused_result
extern tn_string tn_strdupmem(size_t len, const uint8_t *data);

warn_unused_result
hint_returns_not_null
extern const char *tn_str2cstr(tn_string str);

warn_unused_result
static inline size_t
tn_strlen(tn_string str)
{
    return str.len;
}

warn_unused_result
static inline int
tn_strget(tn_string str, size_t i)
{
    return i >= str.len ? '\0' : str.str[i];
}

hint_no_side_effects
warn_unused_result
extern int tn_strcmp(tn_string str1, tn_string str2);

hint_no_side_effects
warn_unused_result
extern bool tn_strisprefix(tn_string str1, tn_string str2);

hint_no_side_effects
warn_unused_result
extern bool tn_strissuffix(tn_string str1, tn_string str2);

warn_unused_result
extern tn_string tn_strcat(tn_string str1, tn_string str2);

warn_unused_result
extern tn_string tn_straddch(tn_string str, char ch);

warn_unused_result
extern tn_string tn_strcats(size_t n, tn_string strs[var_size(n)], tn_string sep);

hint_no_side_effects
warn_unused_result
extern tn_string tn_substr(tn_string str, size_t pos, size_t len);

warn_unused_result
extern tn_string tn_strcut(tn_string str, size_t pos, size_t len);

hint_no_side_effects
warn_unused_result
extern tn_string tn_strlcprefix(tn_string str1, tn_string str2);

hint_no_side_effects
warn_unused_result
extern tn_string tn_strlcsuffix(tn_string str1, tn_string str2);

warn_unused_result
static inline bool
tn_strchr(tn_string str, char ch, size_t *pos)
{
    const char *found = memchr(str.str, ch, str.len);

    if (found != NULL && pos != NULL)
        *pos = (size_t)(found - str.str);

    return (found != NULL);
}

warn_unused_result
extern bool tn_strrchr(tn_string str, char ch, size_t *pos);

warn_unused_result
extern bool tn_strstr(tn_string str, tn_string sub, size_t *pos);

warn_unused_result
extern size_t tn_strdistance(tn_string str1, tn_string str2);

warn_unused_result
warn_null_args(1, 2)
extern bool tn_strtok(tn_string * restrict src, bool (*predicate)(char c),
                      tn_string * restrict tok);

warn_unused_result
warn_null_args(2)
extern tn_string tn_strmap(tn_string str, char (*func)(char ch));

warn_unused_result
warn_null_args(2)
extern tn_string tn_strfilter(tn_string str, bool (*predicate)(char ch));

warn_unused_result
extern tn_string tn_strrepeat(tn_string str, unsigned n);

warn_unused_result
warn_null_args(1, 2)
hint_printf_like(2, 3)
extern tn_status tn_strprintf(tn_string * restrict dest,
                              const char * restrict fmt, ...);

warn_unused_result
hint_printf_like(2, 0)
extern tn_status tn_strvprintf(tn_string * restrict dest,
                               const char * restrict fmt, va_list args);

warn_unused_result
warn_null_args(2)
hint_scanf_like(3, 4)
extern tn_status tn_strscanf(tn_string src, unsigned *count, const char * restrict fmt, ...);

warn_unused_result
warn_null_args(2)
hint_scanf_like(3, 0)    
extern tn_status tn_strvscanf(tn_string src, unsigned *count, const char * restrict fmt,
                              va_list args);

warn_unused_result
warn_any_null_arg
hint_strftime_like(2)
extern tn_status tn_strftime(tn_string * restrict dest, const char * restrict fmt,
                             const struct tm * restrict tm);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* DSTRING_H */
