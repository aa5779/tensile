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

#include <arpa/inet.h>
#if DO_TESTS
#include <stdio.h>
#include <rpc/xdr.h>
#include <math.h>
#endif
#include "utils.h"
#include "xdr.h"

#if DO_TESTS
#define TEST_START ((void)(fprintf(stderr, "%s():\n", __FUNCTION__)))

static char test_buffer[4096];

#define DEFINE_DECODE_TEST(_name, _srctype, _src, _value, _dsttype, _dst, \
                           _xdrproc, _decode, _expect, _condition)      \
    static void test_##_name(void)                                      \
    {                                                                   \
        TEST_START;                                                     \
        XDR encode;                                                     \
        tn_xdr_stream stream = TN_XDR_STREAM_STATIC_ARRAY(test_buffer); \
        _srctype _src = _value;                                         \
        _dsttype _dst;                                                  \
                                                                        \
        memset(&_dst, 0, sizeof(_dst));                                 \
        xdrmem_create(&encode, test_buffer, sizeof(test_buffer),        \
                      XDR_ENCODE);                                      \
                                                                        \
        assert(_xdrproc(&encode, &_src));                               \
        assert(_decode(&stream, &_dst) == (_expect));                   \
        assert(_condition);                                             \
        xdr_destroy(&encode);                                           \
    }                                                                   \
    struct fake


#define DEFINE_ENCODE_OK_TEST(_name, _srctype, _src, _value,            \
                              _dsttype, _dst,                           \
                              _encode, _xdrproc, _condition)            \
    static void test_##_name(void)                                      \
    {                                                                   \
        TEST_START;                                                     \
        XDR decode;                                                     \
        tn_xdr_stream stream = TN_XDR_STREAM_STATIC_ARRAY(test_buffer); \
        _srctype _src = _value;                                         \
        _dsttype _dst;                                                  \
                                                                        \
        memset(&_dst, 0, sizeof(_dst));                                 \
        xdrmem_create(&decode, test_buffer, sizeof(test_buffer),        \
                      XDR_DECODE);                                      \
                                                                        \
        assert(_encode(&stream, &_src) == 0);                           \
        assert(_xdrproc(&decode, &_dst));                               \
        assert(_condition);                                             \
        xdr_destroy(&decode);                                           \
    }                                                                   \
    struct fake

#define DEFINE_ENCODE_FAIL_TEST(_name, _type, _value,_encode, _expect)  \
    static void test_##_name(void)                                      \
    {                                                                   \
        TEST_START;                                                     \
        tn_xdr_stream decode = TN_XDR_STREAM_STATIC_ARRAY(test_buffer); \
        _type val = (_value);                                           \
                                                                        \
        assert(_encode(stream, &_value) == (_expect));                  \
    }                                                                   \
    struct fake

#define DEFINE_BASIC_DECODE_TEST(_name, _type, _value, _decode) \
    DEFINE_DECODE_TEST(_name, _type, src, _value, _type, dst,   \
                       xdr_##_type, _decode, 0, src == dst)

#define DEFINE_BASIC_INTEROP_TESTS(_suffix, _type, _value)              \
    DEFINE_BASIC_DECODE_TEST(decode_##_suffix, _type, _value,           \
                             tn_xdr_decode_##_suffix);                  \
                                                                        \
    DEFINE_ENCODE_OK_TEST(encode_##_suffix, _type, src, _value,         \
                          _type, dst,                                   \
                          tn_xdr_encode_##_suffix, xdr_##_type,         \
                          src == dst)

#define DEFINE_OVERFLOW_TEST(_suffix, _limit, _type, _value)            \
    DEFINE_DECODE_TEST(decode_##_suffix##_##_limit, int32_t, src, _value, \
                       _type, dst,                                      \
                       xdr_int32_t, tn_xdr_decode_##_suffix,            \
                       EOVERFLOW, dst == 0)

#define DEFINE_BOUNDARY_VALUE_TEST(_suffix, _limit, _type, _value)      \
    DEFINE_BASIC_DECODE_TEST(decode_##_suffix##_##_limit, _type, _value, \
                             tn_xdr_decode_##_suffix)

#define DEFINE_UNSIGNED_INT_TESTS(_suffix, _type, _norm, _maxval)       \
    DEFINE_BASIC_INTEROP_TESTS(_suffix, _type, _norm);                  \
    DEFINE_BOUNDARY_VALUE_TEST(_suffix, max, _type, _maxval);           \
    DEFINE_OVERFLOW_TEST(_suffix, ovrmax, _type, (_maxval) + 1)

#define DEFINE_SIGNED_INT_TESTS(_suffix, _type, _norm, _minval, _maxval) \
    DEFINE_UNSIGNED_INT_TESTS(_suffix, _type, _norm, _maxval);          \
    DEFINE_BOUNDARY_VALUE_TEST(_suffix, min, _type, _minval);           \
    DEFINE_OVERFLOW_TEST(_suffix, ovrmin, _type, (_minval) - 1)

DEFINE_BASIC_INTEROP_TESTS(uint32, uint32_t, 0x12345678u);
DEFINE_BOUNDARY_VALUE_TEST(uint32, max, uint32_t, UINT32_MAX);

DEFINE_BASIC_INTEROP_TESTS(int32, int32_t, -0x12345678);
DEFINE_BOUNDARY_VALUE_TEST(int32, max, int32_t, INT32_MAX);
DEFINE_BOUNDARY_VALUE_TEST(int32, min, int32_t, INT32_MIN);

DEFINE_UNSIGNED_INT_TESTS(uint16, uint16_t, 0x1234u, UINT16_MAX);
DEFINE_SIGNED_INT_TESTS(int16, int16_t, -0x1234, INT16_MIN, INT16_MAX);

DEFINE_UNSIGNED_INT_TESTS(uint8, uint8_t, 0x12u, UINT8_MAX);
DEFINE_SIGNED_INT_TESTS(int8, int8_t, -0x12, INT8_MIN, INT8_MAX);

DEFINE_BASIC_INTEROP_TESTS(uint64, uint64_t,
                           0x123456789abcdef0ull);
DEFINE_BOUNDARY_VALUE_TEST(uint64, max, uint64_t, UINT64_MAX);
DEFINE_BASIC_INTEROP_TESTS(int64, int64_t,
                           -0x123456789abcdef0ll);
DEFINE_BOUNDARY_VALUE_TEST(int64, max, int64_t, INT64_MAX);
DEFINE_BOUNDARY_VALUE_TEST(int64, min, int64_t, INT64_MIN);

DEFINE_BASIC_INTEROP_TESTS(float, float, 0.1f);

DEFINE_DECODE_TEST(decode_float_inf, float, src, INFINITY, float, dst,  \
                   xdr_float, tn_xdr_decode_float, 0, isinff(dst));

DEFINE_BASIC_INTEROP_TESTS(double, double, 0.1);

DEFINE_DECODE_TEST(decode_double_inf, double, src, (double)INFINITY,    \
                   double, dst,                                         \
                   xdr_double, tn_xdr_decode_double, 0, isinf(dst));

DEFINE_DECODE_TEST(decode_bool, bool_t, src, TRUE, bool, dst,
                   xdr_bool, tn_xdr_decode_bool, 0, dst);
DEFINE_ENCODE_OK_TEST(encode_bool, bool, src, true, bool_t, dst,
                      tn_xdr_encode_bool, xdr_bool, dst == TRUE);
DEFINE_OVERFLOW_TEST(bool, ovrmax, bool, 2);

#endif

tn_status
tn_xdr_read_from_stream(tn_xdr_stream * restrict stream,
                        void * restrict dest, size_t sz)
{
    tn_status status;

    if (stream->limit - stream->pos < sz)
        return ENOMSG;

    status = stream->reader(stream, dest, sz);
    if (status == 0)
        stream->pos += sz;

    return status;
}

#if DO_TESTS
static void test_read_not_enough_data(void)
{
    TEST_START;
    char buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    tn_xdr_stream stream = TN_XDR_STREAM_STATIC_ARRAY(buffer);
    char rdbuf[8] = {};
    int32_t tmp32;
    uint32_t tmpu32;
    float tmpf;
    double tmpd;

    assert(tn_xdr_read_from_stream(&stream, rdbuf, 16) == ENOMSG);
    assert(*rdbuf == 0);
    assert(tn_xdr_read_from_stream(&stream, rdbuf, 9) == ENOMSG);
    assert(tn_xdr_read_from_stream(&stream, rdbuf, sizeof(rdbuf)) == 0);
    assert(memcmp(rdbuf, buffer, sizeof(rdbuf)) == 0);
    assert(tn_xdr_read_from_stream(&stream, rdbuf, 1) == ENOMSG);
    assert(tn_xdr_read_from_stream(&stream, rdbuf, 0) == 0);
    assert(tn_xdr_decode_int32(&stream, &tmp32) == ENOMSG);
    assert(tn_xdr_decode_uint32(&stream, &tmpu32) == ENOMSG);
    assert(tn_xdr_decode_float(&stream, &tmpf) == ENOMSG);
    assert(tn_xdr_decode_double(&stream, &tmpd) == ENOMSG);
    assert(tn_xdr_decode_void(&stream, rdbuf) == 0);
}
#endif

tn_status
tn_xdr_write_to_stream(tn_xdr_stream * restrict stream,
                       const void * restrict dest, size_t sz)
{
    tn_status status;

    if (stream->limit - stream->pos < sz)
        return ENOSPC;

    status = stream->writer(stream, dest, sz);
    if (status == 0)
        stream->pos += sz;

    return status;
}

#if DO_TESTS
static void test_write_not_enough_space(void)
{
    TEST_START;
    char buffer[4] = {};
    tn_xdr_stream stream = TN_XDR_STREAM_STATIC_ARRAY(buffer);
    char wrbuf[8] = {1, 2, 3, 4, 5, 6, 7, 8};

    assert(tn_xdr_write_to_stream(&stream, wrbuf, 8) == ENOSPC);
    assert(tn_xdr_write_to_stream(&stream, wrbuf, 5) == ENOSPC);
    assert(tn_xdr_write_to_stream(&stream, wrbuf, sizeof(buffer)) == 0);
    assert(memcmp(wrbuf, buffer, sizeof(buffer)) == 0);
    assert(tn_xdr_write_to_stream(&stream, wrbuf, 1) == ENOSPC);
    assert(tn_xdr_write_to_stream(&stream, wrbuf, 0) == 0);
    assert(tn_xdr_encode_void(&stream, wrbuf) == 0);
}
#endif


tn_status
tn_xdr_encode_int64(tn_xdr_stream * restrict stream,
                    const int64_t * restrict data)
{
    tn_status rc = 0;
    int32_t high = (int32_t)(*data >> 32);
    uint32_t low = (uint32_t)(*data & 0xffffffffu);

    rc = tn_xdr_encode_int32(stream, &high);
    if (rc == 0)
        rc = tn_xdr_encode_uint32(stream, &low);

    return rc;
}

tn_status
tn_xdr_decode_int64(tn_xdr_stream * restrict stream, int64_t * restrict data)
{
    tn_status rc = 0;
    int32_t high;
    uint32_t low;

    rc = tn_xdr_decode_int32(stream, &high);
    if (rc == 0)
        rc = tn_xdr_decode_uint32(stream, &low);
    if (rc == 0)
        *data = ((int64_t)high << 32) | low;

    return rc;
}

tn_status
tn_xdr_encode_uint64(tn_xdr_stream * restrict stream,
                    const uint64_t * restrict data)
{
    tn_status rc = 0;
    uint32_t high = (uint32_t)(*data >> 32);
    uint32_t low = (uint32_t)(*data & 0xffffffffu);

    rc = tn_xdr_encode_uint32(stream, &high);
    if (rc == 0)
        rc = tn_xdr_encode_uint32(stream, &low);

    return rc;
}

tn_status
tn_xdr_decode_uint64(tn_xdr_stream * restrict stream, uint64_t * restrict data)
{
    tn_status rc = 0;
    uint32_t high;
    uint32_t low;

    rc = tn_xdr_decode_uint32(stream, &high);
    if (rc == 0)
        rc = tn_xdr_decode_uint32(stream, &low);
    if (rc == 0)
        *data = ((uint64_t)high << 32) | low;

    return rc;
}

tn_status
tn_xdr_encode_length(tn_xdr_stream * restrict stream, size_t length)
{
    uint32_t len32;

#if SIZE_MAX > UINT32_MAX
    if (length > UINT32_MAX)
        return EOVERFLOW;
#endif

    /* This only checks against the minimum possible space requirement */
    if (stream->limit - stream->pos < length + sizeof(uint32_t))
        return ENOSPC;

    len32 = (uint32_t)length;
    return tn_xdr_encode_uint32(stream, &len32);
}

tn_status
tn_xdr_decode_length(tn_xdr_stream * restrict stream,
                     size_t * restrict length)
{
    uint32_t len32;
    tn_status rc = tn_xdr_decode_uint32(stream, &len32);

    if (rc != 0)
        return rc;

    if (stream->limit - stream->pos < len32)
        return EPROTO;

    *length = (size_t)len32;
    return 0;
}

#if DO_TESTS
//static void test
#endif

warn_unused_result warn_any_null_arg
static tn_status
xdr_add_padding(tn_xdr_stream * stream, size_t len)
{
    uint32_t zero = 0;

    if (len % 4 == 0)
        return 0;

    return tn_xdr_write_to_stream(stream, &zero, len % 4);
}


warn_unused_result warn_any_null_arg
static tn_status
xdr_skip_padding(tn_xdr_stream * stream, size_t len)
{
    tn_status rc;
    uint32_t scratch = 0;

    if (len % 4 == 0)
        return 0;

    rc = tn_xdr_read_from_stream(stream, &scratch, len % 4);
    if (rc != 0)
        return rc;

    return scratch == 0 ? 0 : EPROTO;
}


tn_status
tn_xdr_encode_bytes(tn_xdr_stream * restrict stream,
                    size_t len,
                    const uint8_t data[restrict var_size(len)])
{
    tn_status rc =
        tn_xdr_write_to_stream(stream, data, len);

    if (rc == 0)
        rc = xdr_add_padding(stream, len);

    return rc;
}

tn_status
tn_xdr_decode_bytes(tn_xdr_stream * restrict stream,
                    size_t len,
                    uint8_t data[restrict var_size(len)])
{
    tn_status rc =
        tn_xdr_read_from_stream(stream, data, len);

    if (rc == 0)
        rc = xdr_skip_padding(stream, len);

    return rc;
}

tn_status
tn_xdr_encode_var_bytes(tn_xdr_stream * restrict stream,
                        size_t len,
                        const uint8_t data[restrict var_size(len)])
{
    tn_status rc = tn_xdr_encode_length(stream, len);

    if (rc != 0)
        return rc;

    return tn_xdr_encode_bytes(stream, len, data);
}


tn_status
tn_xdr_decode_var_bytes(tn_xdr_stream * restrict stream,
                        size_t * restrict len,
                        uint8_t ** restrict data)
{
    tn_status rc;
    size_t len0;
    uint8_t *buf;

    rc = tn_xdr_decode_length(stream, &len0);
    if (rc != 0)
        return rc;
    if (len0 == 0)
        *data = NULL;
    else
    {
        buf = tn_alloc_blob(len0);
        rc = tn_xdr_decode_bytes(stream, len0, buf);
        if (rc != 0)
            return rc;
        *data = buf;
    }
    *len = len0;
    return 0;
}

tn_status
tn_xdr_decode_cstring(tn_xdr_stream * restrict stream, char ** restrict str)
{
    tn_status rc;
    size_t len;
    char *buf;

    rc = tn_xdr_decode_length(stream, &len);
    if (rc != 0)
        return rc;

    buf = tn_alloc_blob(len + 1);
    rc = tn_xdr_decode_bytes(stream, len, (uint8_t *)buf);
    if (rc != 0)
        return rc;

    buf[len] = '\0';
    *str = buf;
    return 0;
}


tn_status
tn_xdr_encode_array(tn_xdr_stream * restrict stream,
                    const tn_xdr_element_descr * restrict elt,
                    size_t len,
                    const void * restrict data)
{
    tn_status rc;
    size_t i;

    for (i = 0; i < len; i++,
             data = (const uint8_t *restrict)data + elt->elsize)
    {
        rc = elt->encode(stream, data);
        if (rc != 0)
            return rc;
    }
    return 0;
}

tn_status
tn_xdr_decode_array(tn_xdr_stream * restrict stream,
                    const tn_xdr_element_descr * restrict elt,
                    size_t len,
                    void * restrict data)
{
    tn_status rc;
    size_t i;

    for (i = 0; i < len; i++,
             data = (uint8_t *restrict)data + elt->elsize)
    {
        rc = elt->decode(stream, data);
        if (rc != 0)
            return rc;
    }
    return 0;
}

tn_status
tn_xdr_encode_var_array(tn_xdr_stream * restrict stream,
                        const tn_xdr_element_descr * restrict elt,
                        size_t len,
                        const void * restrict data)
{
    tn_status rc = tn_xdr_encode_length(stream, len);

    if (rc != 0)
        return rc;

    return tn_xdr_encode_array(stream, elt, len, data);
}

tn_status
tn_xdr_decode_var_array(tn_xdr_stream * restrict stream,
                        const tn_xdr_element_descr * restrict elt,
                        size_t *len,
                        void ** restrict data)
{
    size_t declen;
    tn_status rc = tn_xdr_decode_length(stream, &declen);
    void *buf;

    buf = tn_alloc(elt->elsize * declen);
    rc = tn_xdr_decode_array(stream, elt, declen, buf);
    if (rc != 0)
        return rc;

    *len = declen;
    *data = buf;

    return 0;
}

tn_status
tn_xdr_encode_struct(tn_xdr_stream * restrict stream,
                     size_t nelts,
                     const tn_xdr_element_descr elt[restrict var_size(nelts)],
                     const void * restrict data)
{
    tn_status rc;
    size_t i;

    for (i = 0; i < nelts; i++)
    {
        rc = elt[i].encode(stream, data);
        if (rc != 0)
            return rc;

        data = (const uint8_t * restrict)data + elt[i].elsize;
    }
    return 0;
}

tn_status
tn_xdr_decode_struct(tn_xdr_stream * restrict stream,
                     size_t nelts,
                     const tn_xdr_element_descr elt[restrict var_size(nelts)],
                     void * restrict data)
{
    tn_status rc;
    size_t i;

    for (i = 0; i < nelts; i++)
    {
        rc = elt[i].decode(stream, data);
        if (rc != 0)
            return rc;

        data = (uint8_t * restrict)data + elt[i].elsize;
    }
    return 0;
}

tn_status
tn_xdr_encode_union(tn_xdr_stream * restrict stream,
                    size_t nelts,
                    const tn_xdr_element_descr elt[restrict var_size(nelts)],
                    unsigned discr,
                    const void * restrict data)
{
    tn_status rc;
    uint32_t discr32;
    
    assert(discr < nelts);
    discr32 = (uint32_t)discr;
    rc = tn_xdr_encode_uint32(stream, &discr32);
    if (rc != 0)
        return rc;

    return elt[discr].encode(stream, data);
}

tn_status
tn_xdr_decode_union(tn_xdr_stream * restrict stream,
                    size_t nelts,
                    const tn_xdr_element_descr elt[restrict var_size(nelts)],
                    unsigned * restrict discr,
                    void * restrict data)
{
    uint32_t discr32;
    tn_status rc = tn_xdr_decode_uint32(stream, &discr32);

    if (rc != 0)
        return rc;
    if (discr32 >= nelts)
        return EPROTO;

    *discr = discr32;
    return elt[discr32].decode(stream, data);
}

#if DO_TESTS

#define CALL_BASIC_INTEROP(_suffix)             \
    test_encode_##_suffix();                    \
    test_decode_##_suffix()

#define CALL_UNSIGNED(_suffix)                  \
    CALL_BASIC_INTEROP(_suffix);                \
    test_decode_##_suffix##_max()

#define CALL_SIGNED(_suffix)                  \
    CALL_UNSIGNED(_suffix);                   \
    test_decode_##_suffix##_min()

#define CALL_UNSIGNED_OVR(_suffix)               \
    CALL_UNSIGNED(_suffix);                      \
    test_decode_##_suffix##_ovrmax()

#define CALL_SIGNED_OVR(_suffix)                 \
    CALL_SIGNED(_suffix);                        \
    test_decode_##_suffix##_ovrmax();            \
    test_decode_##_suffix##_ovrmin()

int main()
{
    CALL_UNSIGNED(uint32);
    CALL_SIGNED(int32);
    CALL_UNSIGNED_OVR(uint16);
    CALL_SIGNED_OVR(int16);
    CALL_UNSIGNED_OVR(uint8);
    CALL_SIGNED_OVR(int8);
    CALL_UNSIGNED(uint64);
    CALL_SIGNED(int64);
    CALL_BASIC_INTEROP(float);
    test_decode_float_inf();
    CALL_BASIC_INTEROP(double);
    test_decode_double_inf();
    CALL_BASIC_INTEROP(bool);
    test_decode_bool_ovrmax();

    test_read_not_enough_data();
    test_write_not_enough_space();
    
    puts("OK");
    return 0;
}
#endif
