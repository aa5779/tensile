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

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include "dstring.h"

#if DO_TESTS
#define TEST_START ((void)(fprintf(stderr, "%s():\n", __FUNCTION__)))
#endif

tn_string
tn_strdup(const char *str)
{
    if (str == NULL || *str == '\0')
        return TN_EMPTY_STRING;
    else
    {
        size_t len = strlen(str);
        char *copy = tn_alloc_blob(len + 1);

        memcpy(copy, str, len + 1);

        return (tn_string){.len = len, .str = copy};
    }
}

#if DO_TESTS
static void test_strdup(void)
{
    TEST_START;
    const char src[] = "abcdefghijk\0mno";
    tn_string s = tn_strdup(src);

    assert(s.str != src);
    assert(s.len == sizeof(src) - 5);
    assert(memcmp(s.str, src, sizeof(src) - 4) == 0);

    assert(tn_strdup("").str == NULL);
}
#endif

tn_string
tn_strdupmem(size_t len, const uint8_t *data)
{
    if (data == NULL || len == 0)
    {
        assert(len == 0);
        return TN_EMPTY_STRING;
    }
    else
    {
        char *copy = tn_alloc_blob(len + 1);

        memcpy(copy, data, len);
        copy[len] = '\0';

        return (tn_string){.len = len, .str = copy};
    }
}

#if DO_TESTS
static void test_strdupmem(void)
{
    TEST_START;
    const char src[] = "abcd\0\0\0efghijk";
    tn_string s = tn_strdupmem(sizeof(src) - 1, (const uint8_t *)src);

    assert(s.str != src);
    assert(s.len == sizeof(src) - 1);
    assert(memcmp(s.str, src, sizeof(src)) == 0);

    assert(tn_strdupmem(0, NULL).str == NULL);
}
#endif


const char *
tn_str2cstr(tn_string str)
{
    if (str.str == NULL)
        return "";
    /* This is safe, because any data in tn_string
     * is either NUL-terminated or a substring of another tn_string
     */
    if (str.str[str.len] == '\0')
        return str.str;
    else
    {
        char *buf = tn_alloc_blob(str.len + 1);

        memcpy(buf, str.str, str.len);
        buf[str.len] = '\0';
        return buf;
    }
}

#if DO_TESTS
static void test_str2cstr(void)
{
    TEST_START;
    static const char literal[] = "abcdefghi";
    tn_string copy;
    const char *chunk;
    
    assert(*tn_str2cstr(TN_EMPTY_STRING) == '\0');
    assert(tn_str2cstr(TN_STRING_LITERAL(literal)) == literal);
    assert(tn_str2cstr(tn_cstr2str(literal)) == literal);
    
    copy = tn_strdup(literal);
    assert(tn_str2cstr(copy) == copy.str);
    
    chunk = tn_str2cstr(tn_substr(copy, 0, 1));
    assert(chunk != copy.str);
    assert(chunk[0] == copy.str[0]);
    assert(chunk[1] == '\0');

}
#endif

int
tn_strcmp(tn_string str1, tn_string str2)
{
    size_t minlen = str1.len < str2.len ? str1.len : str2.len;
    int result = memcmp(str1.str, str2.str, minlen);

    if (!result)
        result = str1.len < str2.len ? -1 : str1.len > str2.len ? 1 : 0;

    return result;
}

#if DO_TESTS
static void test_strcmp(void)
{
    TEST_START;
    tn_string test1 = TN_STRING_LITERAL("abc");
    tn_string test2 = TN_STRING_LITERAL("def");
    tn_string test3 = TN_STRING_LITERAL("abcdef");
    tn_string test3z = TN_STRING_LITERAL("abc\0\0def");
    tn_string testz = TN_STRING_LITERAL("\0");
    
    assert(tn_strcmp(TN_EMPTY_STRING, TN_EMPTY_STRING) == 0);
    assert(tn_strcmp(TN_EMPTY_STRING, test1) < 0);
    assert(tn_strcmp(test1, TN_EMPTY_STRING) > 0);    
    assert(tn_strcmp(test1, test1) == 0);
    assert(tn_strcmp(test1, test2) < 0);
    assert(tn_strcmp(test2, test1) > 0);
    assert(tn_strcmp(test1, test3) < 0);
    assert(tn_strcmp(test3, test1) > 0);
    assert(tn_strcmp(test3z, test1) > 0);
    assert(tn_strcmp(testz, TN_EMPTY_STRING) > 0);
}
#endif

tn_string
tn_strcat(tn_string str1, tn_string str2)
{
    char *buf;
    
    if (str2.len == 0)
        return str1;
    if (str1.len == 0)
        return str2;

    buf = tn_alloc_blob(str1.len + str2.len + 1);
    memcpy(buf, str1.str, str1.len);
    memcpy(buf + str1.len, str2.str, str2.len);
    buf[str1.len + str2.len] = '\0';

    return (tn_string){.str = buf, .len = str1.len + str2.len};
}

#if DO_TESTS
static void test_strcat(void)
{
    TEST_START;
    tn_string str1 = TN_STRING_LITERAL("abcd\0efghi");
    tn_string str2 = TN_STRING_LITERAL("ijkmn\0opqr");
    tn_string str3 = TN_STRING_LITERAL("abcd\0efghiijkmn\0opqr");

    assert(tn_strcmp(tn_strcat(str1, str2), str3) == 0);
    assert(tn_strcat(str1, TN_EMPTY_STRING).str == str1.str);
    assert(tn_strcat(TN_EMPTY_STRING, str2).str == str2.str);
}
#endif

tn_string
tn_straddch(tn_string str, char ch)
{
    char *buf = tn_alloc_blob(str.len + 2);

    memcpy(buf, str.str, str.len);
    buf[str.len] = ch;
    buf[str.len + 1] = '\0';

    return (tn_string){.str = buf, .len = str.len + 1};
}

#if DO_TESTS
static void test_straddch(void)
{
    TEST_START;
    tn_string str = TN_STRING_LITERAL("abcd\0efghi");
    tn_string str2 = TN_STRING_LITERAL("abcd\0efghiz");
    tn_string str3 = TN_STRING_LITERAL("abcd\0efghi\0");
    tn_string str4 = TN_STRING_LITERAL("z");
    tn_string str5 = TN_STRING_LITERAL("\0");

    assert(tn_strcmp(tn_straddch(str, 'z'), str2) == 0);
    assert(tn_strcmp(tn_straddch(str, '\0'), str3) == 0);
    assert(tn_strcmp(tn_straddch(TN_EMPTY_STRING, 'z'), str4) == 0);
    assert(tn_strcmp(tn_straddch(TN_EMPTY_STRING, '\0'), str5) == 0);
}
#endif

tn_string
tn_strcats(size_t n, tn_string strs[var_size(n)], tn_string sep)
{
    if (n == 0)
        return TN_EMPTY_STRING;
    else if (n == 1)
        return strs[0];
    else
    {
        size_t i;
        size_t len = strs[0].len;
        char *buf;
        char *ptr;

        for (i = 1; i < n; i++)
        {
            len += sep.len;
            len += strs[i].len;
        }

        buf = tn_alloc_blob(len + 1);
        memcpy(buf, strs[0].str, strs[0].len);
        for (ptr = buf + strs[0].len, i = 1; i < n; i++)
        {
            memcpy(ptr, sep.str, sep.len);
            ptr += sep.len;
            memcpy(ptr, strs[i].str, strs[i].len);
            ptr += strs[i].len;
        }
        buf[len] = '\0';

        return (tn_string){.str = buf, .len = len};
    }
}

#if DO_TESTS
static void test_strcats(void)
{
    TEST_START;
    tn_string strs[] = {
        TN_STRING_LITERAL("abc"),
        TN_STRING_LITERAL("def"),
        TN_EMPTY_STRING,
        TN_STRING_LITERAL("gh\0i"),
        TN_STRING_LITERAL("jkl"),
    };
    tn_string sep = TN_STRING_LITERAL(",");
    tn_string sep1 = TN_STRING_LITERAL("\0");
    tn_string result1 = TN_STRING_LITERAL("abcdefgh\0ijkl");
    tn_string result2 = TN_STRING_LITERAL("abc,def,,gh\0i,jkl");
    tn_string result3 = TN_STRING_LITERAL("abc\0def\0\0gh\0i\0jkl");

    assert(tn_strcats(0, strs, sep).str == NULL);
    assert(tn_strcats(1, strs, sep).str == strs[0].str);
    assert(tn_strcmp(tn_strcats(5, strs, TN_EMPTY_STRING), result1) == 0);
    assert(tn_strcmp(tn_strcats(5, strs, sep), result2) == 0);
    assert(tn_strcmp(tn_strcats(5, strs, sep1), result3) == 0);
}
#endif

tn_string
tn_substr(tn_string str, size_t pos, size_t len)
{
    if (pos >= str.len)
        return TN_EMPTY_STRING;
    if (pos + len > str.len)
        len = str.len - pos;
    if (len == 0)
        return TN_EMPTY_STRING;

    return (tn_string){.str = str.str + pos, .len = len};
}

#if DO_TESTS
static void test_substr(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("abcdef");
    
    assert(tn_substr(TN_EMPTY_STRING, 1, 1000).str == NULL);
    assert(tn_strcmp(tn_substr(base, 0, 2), TN_STRING_LITERAL("ab")) == 0);
    assert(tn_strcmp(tn_substr(base, 2, 4), TN_STRING_LITERAL("cdef")) == 0);    
    assert(tn_strcmp(tn_substr(base, 0, 1000), base) == 0);
    assert(tn_substr(base, 100, 10).str == NULL);
    assert(tn_strcmp(tn_substr(base, 4, 10), TN_STRING_LITERAL("ef")) == 0);
    assert(tn_substr(base, 0, 0).str == NULL);
    
}
#endif

tn_string
tn_strcut(tn_string str, size_t pos, size_t len)
{
    if (pos >= str.len || len == 0)
        return str;

    if (pos + len >= str.len)
    {
        return pos == 0 ? TN_EMPTY_STRING :
            (tn_string){.str = str.str, .len = pos};
    }
    else if (pos == 0)
        return (tn_string){.str = str.str + len, .len = str.len - len};
    else
    {
        char *buf = tn_alloc_blob(str.len - len + 1);

        memcpy(buf, str.str, pos);
        memcpy(buf + pos, str.str + pos + len, str.len - pos - len);

        return (tn_string){.str = buf, .len = str.len - len};
    }
}

#if DO_TESTS
static void test_strcut(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("abcdef");
    
    assert(tn_strcut(TN_EMPTY_STRING, 1, 1000).str == NULL);
    assert(tn_strcmp(tn_strcut(base, 0, 2), TN_STRING_LITERAL("cdef")) == 0);
    assert(tn_strcmp(tn_strcut(base, 2, 4), TN_STRING_LITERAL("ab")) == 0);    
    assert(tn_strcut(base, 0, 1000).str == NULL);
    assert(tn_strcmp(tn_strcut(base, 100, 10), base) == 0);
    assert(tn_strcmp(tn_strcut(base, 4, 10), TN_STRING_LITERAL("abcd")) == 0);
    assert(tn_strcmp(tn_strcut(base, 0, 0), base) == 0);
    assert(tn_strcmp(tn_strcut(base, 2, 3), TN_STRING_LITERAL("abf")) == 0);
}
#endif

tn_string
tn_strlcprefix(tn_string str1, tn_string str2)
{
    size_t minsize = str1.len < str2.len ? str1.len : str2.len;
    size_t i;

    for (i = 0; i < minsize; i++)
    {
        if (str1.str[i] != str2.str[i])
            break;
    }
    
    return (tn_string){.str = (i == 0 ? NULL : str1.str), .len = i};
}

tn_string
tn_strlcsuffix(tn_string str1, tn_string str2)
{
    size_t minsize = str1.len < str2.len ? str1.len : str2.len;
    size_t i;

    for (i = 0; i < minsize; i++)
    {
        if (str1.str[str1.len - i - 1] != str2.str[str2.len - i - 1])
            break;
    }
    
    return (tn_string){.str = (i == 0 ? NULL : str1.str + str1.len - i),
                       .len = i};
}

#if DO_TESTS
static void test_lc_prefix_suffix(void)
{
    TEST_START;
    tn_string str1 = TN_STRING_LITERAL("abcdef");
    tn_string str2 = TN_STRING_LITERAL("abcxyz");
    tn_string str3 = TN_STRING_LITERAL("xyzdef");

    tn_string pfx = TN_STRING_LITERAL("abc");
    tn_string sfx = TN_STRING_LITERAL("def");

    assert(tn_strcmp(tn_strlcprefix(str1, str2), pfx) == 0);
    assert(tn_strcmp(tn_strlcsuffix(str1, str3), sfx) == 0);

    assert(tn_strlcprefix(str1, str3).str == NULL);
    assert(tn_strlcsuffix(str1, str2).str == NULL);

    assert(tn_strcmp(tn_strlcprefix(pfx, str1), pfx) == 0);
    assert(tn_strcmp(tn_strlcsuffix(str3, sfx), sfx) == 0);

    assert(tn_strlcprefix(TN_EMPTY_STRING, str1).str == NULL);
    assert(tn_strlcprefix(str2, TN_EMPTY_STRING).str == NULL);
    assert(tn_strlcsuffix(TN_EMPTY_STRING, str1).str == NULL);
    assert(tn_strlcsuffix(str3, TN_EMPTY_STRING).str == NULL);
}

#endif    

bool
tn_strrchr(tn_string str, char ch, size_t *pos)
{
    size_t i;

    for (i = str.len; i > 0; i--)
    {
        if (str.str[i - 1] == ch)
        {
            if (pos != NULL)
                *pos = i - 1;
            return true;
        }
    }
  
    return false;
}

#if DO_TESTS
static void test_strchr_strrchr(void)
{
    TEST_START;
    tn_string str = TN_STRING_LITERAL("abc\0deffed\0cba");
    size_t pos = (size_t)-1;

    assert(tn_strchr(str, 'a', NULL));
    assert(tn_strchr(str, 'd', NULL));
    assert(tn_strchr(str, '\0', NULL));
    assert(!tn_strchr(str, 'x', NULL));

    assert(tn_strrchr(str, 'a', NULL));
    assert(tn_strrchr(str, 'd', NULL));
    assert(tn_strrchr(str, '\0', NULL));
    assert(!tn_strrchr(str, 'x', NULL));

    assert(tn_strchr(str, 'a', &pos) && pos == 0);
    assert(tn_strchr(str, 'd', &pos) && pos == 4);
    assert(tn_strchr(str, '\0', &pos) && pos == 3);
    assert(!tn_strchr(str, 'x', &pos) && pos == 3);

    assert(tn_strrchr(str, 'a', &pos) && pos == 13);
    assert(tn_strrchr(str, 'd', &pos) && pos == 9);
    assert(tn_strrchr(str, '\0', &pos) && pos == 10);
    assert(!tn_strrchr(str, 'x', &pos) && pos == 10);

    assert(!tn_strchr(TN_EMPTY_STRING, '\0', NULL));
    assert(!tn_strrchr(TN_EMPTY_STRING, '\0', NULL));
}
#endif

bool
tn_strstr(tn_string str, tn_string sub, size_t *pos)
{
    size_t i;

    if (sub.len == 0)
    {
        if (pos != NULL)
            *pos = 0;
        return true;
    }

    if (sub.len > str.len)
        return false;
    
    for (i = 0; i < str.len - sub.len + 1; i++)
    {
        size_t j;

        for (j = 0; j < sub.len; j++)
        {
            if (str.str[i + j] != sub.str[j])
                break;
        }
        if (j == sub.len)
        {
            if (pos)
                *pos = i;
            return true;
        }
    }
    return false;
}

#if DO_TESTS
static void test_strstr(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("abc\0def\0ghi");
    size_t pos = (size_t)(-1);

    assert(tn_strstr(base, TN_EMPTY_STRING, NULL));
    assert(tn_strstr(base, TN_STRING_LITERAL("abc"), NULL));
    assert(tn_strstr(base, TN_STRING_LITERAL("\0ghi"), NULL));
    assert(!tn_strstr(base, TN_STRING_LITERAL("xyz"), NULL));
    assert(!tn_strstr(base, TN_STRING_LITERAL("averyveryveryverylongsubstring"), NULL));
    assert(tn_strstr(TN_EMPTY_STRING, TN_EMPTY_STRING, NULL));
    assert(!tn_strstr(TN_EMPTY_STRING, TN_STRING_LITERAL("abc"), NULL));

    assert(tn_strstr(base, TN_EMPTY_STRING, &pos) && pos == 0);
    assert(tn_strstr(base, TN_STRING_LITERAL("abc"), &pos) && pos == 0);
    assert(tn_strstr(base, TN_STRING_LITERAL("\0ghi"), &pos) && pos == 7);
    assert(!tn_strstr(base, TN_STRING_LITERAL("xyz"), &pos) && pos == 7);
}
#endif

bool
tn_strisprefix(tn_string prefix, tn_string str)
{
    if (prefix.len > str.len)
        return false;

    return memcmp(str.str, prefix.str, prefix.len) == 0;
}

bool
tn_strissuffix(tn_string suffix, tn_string str)
{
    if (suffix.len > str.len)
        return false;

    return memcmp(str.str + str.len - suffix.len,
                  suffix.str, suffix.len) == 0;
}

#if DO_TESTS
static void test_isprefix_issuffix(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("abc\0def");

    assert(tn_strisprefix(TN_STRING_LITERAL("abc\0"), base));
    assert(tn_strissuffix(TN_STRING_LITERAL("\0def"), base));
    assert(tn_strisprefix(TN_EMPTY_STRING, base));
    assert(tn_strissuffix(TN_EMPTY_STRING, base));
    assert(!tn_strisprefix(TN_STRING_LITERAL("abc\0z"), base));
    assert(!tn_strissuffix(TN_STRING_LITERAL("z\0def"), base));

    assert(!tn_strisprefix(TN_STRING_LITERAL("abc\0defghi"), base));
    assert(!tn_strissuffix(TN_STRING_LITERAL("abc\0defghi"), base));
}
#endif

bool
tn_strtok(tn_string *src, bool (*predicate)(char c), tn_string *tok)
{
    size_t i;
    size_t start;

    for (i = 0; i < src->len; i++)
    {
        if (!predicate(src->str[i]))
            break;
    }
    if (i == src->len)
        return false;

    start = i;
    for (; i < src->len; i++)
    {
        if (predicate(src->str[i]))
            break;
    }
    if (tok)
    {
        tok->str = src->str + start;
        tok->len = i - start;
    }
    src->str += i;
    src->len -= i;

    return true;
}

#if DO_TESTS
hint_no_side_effects
static bool test_eq_space(char c)
{
    return c == ' ';
}

static void test_strtok(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("    abc\t    def ghi    ");
    tn_string token = TN_EMPTY_STRING;

    assert(tn_strtok(&base, test_eq_space, &token) &&
           tn_strcmp(token, TN_STRING_LITERAL("abc\t")) == 0);
    assert(tn_strtok(&base, test_eq_space, &token) &&
           tn_strcmp(token, TN_STRING_LITERAL("def")) == 0);
    assert(tn_strtok(&base, test_eq_space, &token) &&
           tn_strcmp(token, TN_STRING_LITERAL("ghi")) == 0);
    assert(!tn_strtok(&base, test_eq_space, &token) &&
           tn_strcmp(base, TN_STRING_LITERAL("    ")) == 0 &&
           tn_strcmp(token, TN_STRING_LITERAL("ghi")) == 0);
}
#endif

tn_string
tn_strmap(tn_string str, char (*func)(char ch))
{
    size_t i;
    char *buf;
    
    if (str.len == 0)
        return TN_EMPTY_STRING;

    buf = tn_alloc_blob(str.len + 1);
    for (i = 0; i < str.len; i++)
        buf[i] = func(str.str[i]);

    return (tn_string){.str = buf, .len = str.len};
}

#if DO_TESTS
hint_no_side_effects
static char test_cvt(char c)
{
    return c == ' ' ? '+' : c;
}

static void test_strmap(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("abc\0def  ghi ");

    assert(tn_strcmp(tn_strmap(base, test_cvt),
                     TN_STRING_LITERAL("abc\0def++ghi+")) == 0);
    assert(tn_strmap(TN_EMPTY_STRING, test_cvt).str == NULL);
}
#endif

tn_string
tn_strfilter(tn_string str, bool (*predicate)(char ch))
{
    char *buf;
    size_t i, j;
    
    if (str.len == 0)
        return TN_EMPTY_STRING;

    buf = tn_alloc_blob(str.len + 1);
    for (i = 0, j = 0; i < str.len; i++)
    {
        if (predicate(str.str[i]))
            buf[j++] = str.str[i];
    }
    buf[j] = '\0';
    buf = tn_realloc(buf, j + 1);

    return (tn_string){.str = buf, .len = j};
}

#if DO_TESTS
hint_no_side_effects
static bool test_neq_space(char c)
{
    return c != ' ';
}


static void test_strfilter(void)
{
    TEST_START;
    tn_string base = TN_STRING_LITERAL("  abc   de\0fg i j k   ");

    assert(tn_strcmp(tn_strfilter(base, test_neq_space),
                     TN_STRING_LITERAL("abcde\0fgijk")) == 0);
    assert(tn_strfilter(TN_EMPTY_STRING, test_neq_space).str == NULL);
}
#endif

tn_string
tn_strrepeat(tn_string str, unsigned n)
{
    if (n == 0 || str.len == 0)
        return TN_EMPTY_STRING;
    if (n == 1)
        return str;
    else
    {
        char *buf = tn_alloc_blob(str.len * n + 1);
        unsigned i;

        for (i = 0; i < n; i++)
            memcpy(buf + str.len * i, str.str, str.len);
        buf[str.len * n] = '\0';

        return (tn_string){.str = buf, .len = str.len * n};
    }
}

#if DO_TESTS
static void test_strrepeat(void)
{
    tn_string base = TN_STRING_LITERAL("abc\0");

    assert(tn_strrepeat(base, 0).str == NULL);
    assert(tn_strrepeat(base, 1).str == base.str);
    assert(tn_strcmp(tn_strrepeat(base, 2),
                     TN_STRING_LITERAL("abc\0abc\0")) == 0);
    assert(tn_strrepeat(TN_EMPTY_STRING, 2).str == NULL);
}
#endif

static size_t
min_size3(size_t s1, size_t s2, size_t s3)
{
    size_t min12 = s1 < s2 ? s1 : s2;

    return min12 < s3 ? min12 : s3;
}

size_t
tn_strdistance(tn_string str1, tn_string str2)
{
    size_t *prev_row;
    size_t *cur_row;
    size_t *swap;
    size_t i, j;
    
    if (tn_strcmp(str1, str2) == 0)
        return 0;
    if (str1.len == 0)
        return str2.len;
    if (str2.len == 0)
        return str1.len;

    prev_row = tn_alloc_blob(sizeof(*prev_row) * (str2.len + 1));
    cur_row  = tn_alloc_blob(sizeof(*cur_row) * (str2.len + 1));

    for (i = 0; i <= str2.len; i++)
        prev_row[i] = i;

    for (i = 1; i <= str1.len; i++)
    {
        cur_row[0] = i;
        for (j = 1; j <= str2.len; j++)
        {
            cur_row[j] = min_size3(cur_row[j - 1] + 1,
                                   prev_row[j] + 1,
                                   prev_row[j - 1] +
                                   (str2.str[j - 1] != str1.str[i - 1]));
        }
        swap = prev_row;
        prev_row = cur_row;
        cur_row = swap;
    }
    return prev_row[str2.len];
}

#if DO_TESTS

static void test_distance(void)
{
    TEST_START;
    tn_string str1 = TN_STRING_LITERAL("abc");
    tn_string str2 = TN_STRING_LITERAL("adbecf");

    assert(tn_strdistance(str1, str1) == 0);
    assert(tn_strdistance(TN_EMPTY_STRING, TN_EMPTY_STRING) == 0);
    assert(tn_strdistance(str1, TN_EMPTY_STRING) == str1.len);
    assert(tn_strdistance(TN_EMPTY_STRING, str2) == str2.len);

    assert(tn_strdistance(str1, tn_substr(str1, 0, 2)) == 1);
    assert(tn_strdistance(str1, str2) == 3);
    assert(tn_strdistance(str2, str1) == 3);
}

#endif

tn_status
tn_strprintf(tn_string * restrict dest, const char * restrict fmt, ...)
{
    va_list args;
    tn_status rc;
    
    va_start(args, fmt);
    rc = tn_strvprintf(dest, fmt, args);
    va_end(args);

    return rc;
}

tn_status
tn_strvprintf(tn_string * restrict dest, const char * restrict fmt,
              va_list args)
{
    char buf[64];
    tn_status status = 0;
    char *result = buf;
    int rc;
    va_list args2;

    va_copy(args2, args);
    rc = vsnprintf(buf, sizeof(buf), fmt, args);
    if (rc < 0)
        status = errno;
    else if ((size_t)rc >= sizeof(buf))
    {
        int rc2;

        result = tn_alloc_blob((size_t)rc + 1);
        
        rc2 = vsnprintf(result, (size_t)rc + 1, fmt, args2);
        if (rc2 < 0)
            status = errno;
        else if (rc2 != rc)
            status = EPROTO;
        else
        {
            dest->str = result;
            dest->len = (size_t)rc;
        }
    }
    else
    {
        *dest = tn_strdup(buf);
    }
    va_end(args2);

    return status;
}

#if DO_TESTS
static void test_sprintf(void)
{
    TEST_START;
    tn_string buffer1 = TN_EMPTY_STRING;
    tn_string buffer2 = TN_EMPTY_STRING;
    tn_string longresult =
        TN_STRING_LITERAL("01234567890123456789012345678901234567890123456789"
                          "01234567890123456789012345678901234567890123456789");

    assert(tn_strprintf(&buffer1, "%d", 123) == 0);
    assert(tn_strcmp(buffer1, TN_STRING_LITERAL("123")) == 0);

    assert(tn_strprintf(&buffer2, "%s", longresult.str) == 0);
    assert(tn_strcmp(buffer2, longresult) == 0);
    assert(buffer1.str != buffer2.str);
    
}
#endif

tn_status
tn_strscanf(tn_string src, unsigned *count, const char * restrict fmt, ...)
{
    va_list args;
    tn_status rc;
    
    va_start(args, fmt);
    rc = tn_strvscanf(src, count, fmt, args);
    va_end(args);

    return rc;
}

tn_status
tn_strvscanf(tn_string src, unsigned *count, const char * restrict fmt,
             va_list args)
{
    int rc;

    errno = 0;
    rc = vsscanf(tn_str2cstr(src), fmt, args);
    if (rc == EOF)
    {
        if (errno == 0)
            *count = 0;
        return errno;
    }

    *count = (unsigned)rc;

    return 0;
}

#if DO_TESTS
static void test_sscanf(void)
{
    TEST_START;
    tn_string buffer = TN_STRING_LITERAL("123 134");
    int v1 = 0, v2 = 0;
    unsigned count = 0;

    assert(tn_strscanf(buffer, &count, "%d %d", &v1, &v2) == 0);
    assert(count == 2);
    assert(v1 == 123 && v2 == 134);

    v1 = v2 = 0;
    assert(tn_strscanf(tn_substr(buffer, 0, 4), &count, "%d %d", &v1, &v2) == 0);
    assert(count == 1);
    assert(v1 == 123);
    assert(v2 == 0);

    assert(tn_strscanf(TN_EMPTY_STRING, &count, "%d %d", &v1, &v2) == 0);
    assert(count == 0);
}
#endif

tn_status
tn_strftime(tn_string * restrict dest, const char * restrict fmt,
            const struct tm * restrict tm)
{
    char buf[64];
    /* This is necessary to shut up the compiler about non-literal format
     * string, see e.g https://gcc.gnu.org/bugzilla/show_bug.cgi?id=39438
     */
#define STRFTIME                                                        \
    ((size_t (*)(char *, size_t, const char *, const struct tm *))strftime)
    size_t sz = STRFTIME(buf, sizeof(buf), fmt, tm);

    if (sz != 0)
    {
        *dest = tn_strdup(buf);
    }
    else
    {
        char *dynbuf = NULL;
        size_t maxsize = 2 * sizeof(buf);

        while (sz == 0)
        {
            dynbuf = tn_realloc(dynbuf, maxsize);
            sz = STRFTIME(dynbuf, maxsize, fmt, tm);
            maxsize *= 2;
        }

        dynbuf = tn_realloc(dynbuf, sz + 1);
        dest->str = dynbuf;
        dest->len = sz;
    }
#undef STRFTIME

    return 0;
}

#if DO_TESTS
static void test_strftime(void)
{
    TEST_START;
    tn_string output = TN_EMPTY_STRING;
    struct tm test_tm = {
        .tm_year = 114,
        .tm_mon  = 11,
        .tm_mday  = 22,
        .tm_hour = 13,
        .tm_min  = 28,
        .tm_sec  = 12,
    };

    assert(tn_strftime(&output, "%Y-%m-%d %H:%M:%S", &test_tm) == 0);
    assert(tn_strcmp(output, TN_STRING_LITERAL("2014-12-22 13:28:12")) == 0);

    assert(tn_strftime(&output, "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S"
                       "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S"
                       "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S"
                       "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S"
                       "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S"
                       "%Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S %Y-%m-%d %H:%M:%S", &test_tm) == 0);
    assert(tn_strcmp(output,
                     TN_STRING_LITERAL("2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12"
                                       "2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12"
                                       "2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12"
                                       "2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12"
                                       "2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12"
                                       "2014-12-22 13:28:12 2014-12-22 13:28:12 2014-12-22 13:28:12")) == 0);
}
#endif

#if DO_TESTS
int main()
{
    GC_INIT();
    test_strdup();
    test_strdupmem();
    test_strcmp();
    test_str2cstr();
    test_strcat();
    test_straddch();
    test_strcats();
    test_substr();
    test_strcut();
    test_lc_prefix_suffix();
    test_strchr_strrchr();
    test_isprefix_issuffix();
    test_strstr();
    test_strtok();
    test_strmap();
    test_strfilter();
    test_strrepeat();
    test_distance();
    test_sprintf();
    test_sscanf();
    test_strftime();
    
    puts("OK");
    
    return 0;
}
#endif
