include $(top_srcdir)/common.am

noinst_LIBRARIES = libtn.a

libtn_a_SOURCES = values.c

TESTS = $(check_PROGRAMS)

check_PROGRAMS = utils_ts

utils_ts_SOURCES = utils_ts.t
utils_ts_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS)
utils_ts_LDFLAGS = $(AM_LDFLAGS) $(CHECK_LDFLAGS)
