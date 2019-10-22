/*
 * Copyright (c) 2019 Artem V. Andreev
 *
 * SPDX-License-Identifier: MIT
 */
#include "utils.h"

void
tn_sprintf(tn_ptr_location loc, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    tn_vsprintf(loc, fmt, args);
    va_end(args);
}

void *
tn_buffer_append(tn_buffer *buf, size_t sz)
{
#define DELTA_BUF_SIZE 64
    void *endptr;

    if (*buf->location.loc == NULL)
    {
        TN_BUG_ON(buf->len != 0);
        buf->len = sz;
        buf->bufsize = sz + DELTA_BUF_SIZE;
        tn_alloc_raw(buf->location, buf->bufsize);
        return (uint8_t *)*buf->location.loc + buf->offset;
    }
    if (buf->offset + buf->len + sz > buf->bufsize)
    {
        buf->bufsize += sz + DELTA_BUF_SIZE;
        tn_realloc(buf->location, buf->bufsize);
    }
    endptr = (uint8_t *)*buf->location.loc + buf->offset + buf->len;
    buf->len += sz;
    return endptr;
#undef DELTA_BUF_SIZE
}
