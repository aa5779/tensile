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
 * @brief XDR implementation
 *
 * @author Artem V. Andreev <artem@iling.spb.ru>
 */
#ifndef XDR_H
#define XDR_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "compiler.h"
#include "status.h"

typedef struct tn_xdr_stream {
    tn_status (*reader)(struct tn_xdr_stream * restrict,
                        void * restrict , size_t);
    tn_status (*writer)(struct tn_xdr_stream * restrict,
                        const void * restrict, size_t);

    void *data;
    size_t pos;
    size_t limit;
} tn_xdr_stream;

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_read_from_stream(tn_xdr_stream * restrict stream,
                                         void * restrict dest, size_t sz);


warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_write_to_stream(tn_xdr_stream * restrict stream,
                                        const void * restrict dest,
                                        size_t sz);

warn_unused_result
warn_any_null_arg
static inline tn_status
tn_xdr_mem_reader(tn_xdr_stream * restrict stream, void * restrict dest,
                  size_t sz)
{
    memcpy(dest, stream->data, sz);
    stream->data = (uint8_t *)stream->data + sz;
    return 0;
}

warn_unused_result
warn_any_null_arg
static inline tn_status
tn_xdr_mem_writer(tn_xdr_stream * restrict stream,
                  const void * restrict dest, size_t sz)
{
    memcpy(stream->data, dest, sz);
    stream->data = (uint8_t *)stream->data + sz;
    return 0;
}

warn_unused_result
warn_any_null_arg
static inline tn_status
tn_xdr_dummy_reader(unused tn_xdr_stream * restrict stream,
                   unused void * restrict dest, unused size_t sz)
{
    return ENOMSG;
}

warn_unused_result
warn_any_null_arg
static inline tn_status
tn_xdr_dummy_writer(unused tn_xdr_stream * restrict stream,
                    unused const void * restrict dest, unused size_t sz)
{
    return ENOSPC;
}

#define TN_XDR_STREAM_STATIC_ARRAY(_array) \
    ((tn_xdr_stream) {                     \
            .reader = tn_xdr_mem_reader,   \
            .writer = tn_xdr_mem_writer,   \
            .data = (_array),              \
            .pos = 0,                      \
            .limit = sizeof(_array)        \
            })

typedef tn_status (*warn_unused_result warn_any_null_arg tn_xdr_encoder)(
        tn_xdr_stream * restrict,
        const void * restrict);

typedef tn_status (*warn_unused_result warn_any_null_arg tn_xdr_decoder)(
        tn_xdr_stream * restrict,
        void * restrict);

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_int32(tn_xdr_stream * restrict stream,
                    const int32_t * restrict data)
{
    int32_t val = (int32_t)htonl((uint32_t)*data);
    return tn_xdr_write_to_stream(stream, &val, sizeof(val));
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_int32(tn_xdr_stream * restrict stream, int32_t * restrict data) 
{
    int32_t val;
    tn_status rc = tn_xdr_read_from_stream(stream, &val, sizeof(val));
    
    if (rc != 0)
        return rc;
    *data = (int32_t)ntohl((uint32_t)val);
    return 0;
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_uint32(tn_xdr_stream * restrict stream,
                     const uint32_t * restrict data)
{
    uint32_t val = htonl(*data);
    return tn_xdr_write_to_stream(stream, &val, sizeof(val));
}


warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_uint32(tn_xdr_stream * restrict stream,
                     uint32_t * restrict data)
{
    uint32_t val;
    tn_status rc = tn_xdr_read_from_stream(stream, &val, sizeof(val));
    
    if (rc != 0)
        return rc;
    *data = ntohl(val);
    return 0;
}

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_int64(tn_xdr_stream * restrict stream, 
                                     const int64_t * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_int64(tn_xdr_stream * restrict stream,
                                     int64_t * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_uint64(tn_xdr_stream * restrict stream,
                                      const uint64_t * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_uint64(tn_xdr_stream * restrict stream,
                                      uint64_t * restrict data);

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_float(tn_xdr_stream * restrict stream,
                    const float * restrict data)
{
    uint32_t tmp;
    memcpy(&tmp, data, sizeof(tmp));
    return tn_xdr_encode_uint32(stream, &tmp);
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_float(tn_xdr_stream * restrict stream, float * restrict data)
{
    tn_status rc;
    uint32_t tmp;
    rc = tn_xdr_decode_uint32(stream, &tmp);
    if (rc != 0)
        return rc;
    memcpy(data, &tmp, sizeof(tmp));
    return 0;
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_double(tn_xdr_stream * restrict stream,
                     const double * restrict data)
{
    uint64_t tmp;
    memcpy(&tmp, data, sizeof(tmp));
    return tn_xdr_encode_uint64(stream, &tmp);
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_double(tn_xdr_stream * restrict stream, double * restrict data)
{
    tn_status rc;
    uint64_t tmp;
    rc = tn_xdr_decode_uint64(stream, &tmp);
    if (rc != 0)
        return rc;
    memcpy(data, &tmp, sizeof(tmp));
    return 0;
}

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_length(tn_xdr_stream * restrict stream,
                                      size_t length);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_length(tn_xdr_stream * restrict stream,
                                      size_t * restrict length);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_bytes(
        tn_xdr_stream * restrict stream,
        size_t len,
        const uint8_t data[restrict var_size(len)]);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_bytes(
        tn_xdr_stream * restrict stream,
        size_t len,
        uint8_t data[restrict var_size(len)]);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_var_bytes(
        tn_xdr_stream * restrict stream,
        size_t len,
        const uint8_t data[restrict var_size(len)]);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_var_bytes(tn_xdr_stream * restrict stream,
                                         size_t * restrict len,
                                         uint8_t ** restrict data);

warn_unused_result warn_any_null_arg
static inline tn_status tn_xdr_encode_cstring(tn_xdr_stream * restrict stream,
                                              const char * restrict str)
{
    return tn_xdr_encode_var_bytes(stream, strlen(str),
                                   (const uint8_t *)str);
}

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_cstring(tn_xdr_stream * restrict stream,
                                       char ** restrict str);


typedef struct tn_xdr_element_descr {
    size_t elsize;
    tn_xdr_encoder encode;
    tn_xdr_decoder decode;
} tn_xdr_element_descr;

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_array(
        tn_xdr_stream * restrict stream,
        const tn_xdr_element_descr * restrict elt,
        size_t len,
        const void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_array(
        tn_xdr_stream * restrict stream,
        const tn_xdr_element_descr * restrict elt,
        size_t len, void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_var_array(
        tn_xdr_stream * restrict stream,
        const tn_xdr_element_descr * restrict elt,
        size_t len,
        const void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_var_array(
        tn_xdr_stream * restrict stream,
        const tn_xdr_element_descr * restrict elt,
        size_t *len,
        void ** restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_struct(
        tn_xdr_stream * restrict stream,
        size_t nelts,
        const tn_xdr_element_descr elt[restrict var_size(nelts)],
        const void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_struct(
        tn_xdr_stream * restrict stream,
        size_t nelts,
        const tn_xdr_element_descr elt[restrict var_size(nelts)],
        void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_encode_union(
        tn_xdr_stream * restrict stream,
        size_t nelts,
        const tn_xdr_element_descr elt[restrict var_size(nelts)],
        unsigned discr,
        const void * restrict data);

warn_unused_result
warn_any_null_arg
extern tn_status tn_xdr_decode_union(
        tn_xdr_stream * restrict stream,
        size_t nelts,
        const tn_xdr_element_descr elt[restrict var_size(nelts)],
        unsigned * restrict discr,
        void * restrict data);

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_void(unused tn_xdr_stream * restrict stream,
                   unused const void * restrict data)
{
    return 0;
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_void(unused tn_xdr_stream * restrict stream,
                   unused void * restrict data)
{
    return 0;
}

#define DEFINE_XDR_SMALL_VALUE_ENCODER(_suffix, _type,                  \
                                       _xdrsuffix, _xdrtype)            \
    warn_unused_result warn_any_null_arg                                \
    static inline tn_status                                             \
    tn_xdr_encode_##_suffix(tn_xdr_stream * restrict stream,            \
                            const _type * restrict data)                \
    {                                                                   \
        return tn_xdr_encode_##_xdrsuffix(stream,                       \
                                          (_xdrtype *)data);            \
    }                                                                   \
    struct fake


#define DEFINE_XDR_SMALL_UVALUE_DECODER(_suffix, _type,         \
                                  _xdrsuffix, _xdrtype, _max)   \
    warn_unused_result warn_any_null_arg                        \
    static inline tn_status                                     \
    tn_xdr_decode_##_suffix(tn_xdr_stream * restrict stream,    \
                            _type * restrict data)              \
    {                                                           \
        tn_status rc;                                           \
        _xdrtype buf;                                           \
                                                                \
        rc = tn_xdr_decode_##_xdrsuffix(stream, &buf);          \
        if (rc != 0)                                            \
            return rc;                                          \
        if (buf > (_max))                                       \
            return EOVERFLOW;                                   \
        *data = (_type)buf;                                     \
        return 0;                                               \
    }                                                           \
    struct fake

#define DEFINE_XDR_SMALL_SVALUE_DECODER(_suffix, _type,                 \
                                        _xdrsuffix, _xdrtype,           \
                                        _min, _max)                     \
    warn_unused_result warn_any_null_arg                                \
    static inline tn_status                                             \
    tn_xdr_decode_##_suffix(tn_xdr_stream * restrict stream,            \
                            _type * restrict data)                      \
    {                                                                   \
        tn_status rc;                                                   \
        _xdrtype buf;                                                   \
                                                                        \
        rc = tn_xdr_decode_##_xdrsuffix(stream, &buf);                  \
        if (rc != 0)                                                    \
            return rc;                                                  \
        if (buf < (_min) || buf > (_max))                               \
            return EOVERFLOW;                                           \
        *data = (_type)buf;                                             \
        return 0;                                                       \
    }                                                                   \
    struct fake

DEFINE_XDR_SMALL_VALUE_ENCODER(bool, bool, int32, int32_t);
DEFINE_XDR_SMALL_SVALUE_DECODER(bool, bool, int32, int32_t, 0, 1);

DEFINE_XDR_SMALL_VALUE_ENCODER(int8, int8_t, int32, int32_t);
DEFINE_XDR_SMALL_SVALUE_DECODER(int8, int8_t, int32, int32_t,
                                INT8_MIN, INT8_MAX);

DEFINE_XDR_SMALL_VALUE_ENCODER(int16, int16_t, int32, int32_t);
DEFINE_XDR_SMALL_SVALUE_DECODER(int16, int16_t, int32, int32_t,
                                INT16_MIN, INT16_MAX);

DEFINE_XDR_SMALL_VALUE_ENCODER(uint8, uint8_t, uint32, uint32_t);
DEFINE_XDR_SMALL_UVALUE_DECODER(uint8, uint8_t, uint32, uint32_t, UINT8_MAX);

DEFINE_XDR_SMALL_VALUE_ENCODER(uint16, uint16_t, uint32, uint32_t);
DEFINE_XDR_SMALL_UVALUE_DECODER(uint16, uint16_t, uint32, uint32_t,
                                UINT16_MAX);

#if SIZE_MAX == UINT64_MAX
warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_encode_size_t(tn_xdr_stream * restrict stream,
                     const size_t * restrict data)
{
    return tn_xdr_encode_uint64(stream, data);
}

warn_unused_result warn_any_null_arg
static inline tn_status
tn_xdr_decode_size_t(tn_xdr_stream * restrict stream,
                     size_t * restrict data)
{
    return tn_xdr_decode_uint64(stream, (uint64_t *)data);
}
#else
DEFINE_XDR_SMALL_VALUE_ENCODER(size_t, size_t, uint64, uint64_t);
DEFINE_XDR_SMALL_UVALUE_DECODER(size_t, size_t, uint64, uint64_t, SIZE_MAX);
#endif

#define DEFINE_XDR_PTR_ENCODER_THUNK(_name, _type, _call)               \
    warn_unused_result warn_any_null_arg                                \
    static inline tn_status                                             \
    _name(tn_xdr_stream * restrict stream, const void * restrict data)  \
    {                                                                   \
        return _call(stream, *(_type const *)data);                     \
    }                                                                   \
    struct fake

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* XDR_H */
