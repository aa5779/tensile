include $(top_srcdir)/common.am

noinst_LIBRARIES = libtn.a

libtn_a_SOURCES = values.c

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_unit_tests

libtn_unit_tests_SOURCES = libtn_unit_tests.t
libtn_unit_tests_CFLAGS = $(AM_CFLAGS) $(CHECK_CFLAGS)
libtn_unit_tests_LDFLAGS = $(AM_LDFLAGS)
libtn_unit_tests_LDADD = libtn.a $(CHECK_LIBS)

