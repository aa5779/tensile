include $(top_srcdir)/common.am

noinst_LIBRARIES = libtn.a

libtn_a_SOURCES = values.c

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_utils_tests libtn_values_tests

libtn_utils_tests_SOURCES = libtn_utils_tests.t
libtn_utils_tests_CFLAGS = $(TEST_CFLAGS)
libtn_utils_tests_CPPFLAGS = $(TEST_CPPFLAGS)
libtn_utils_tests_LDADD = libtn.a $(TEST_LDADD)

libtn_values_tests_SOURCES = libtn_values_tests.t
libtn_values_tests_CFLAGS = $(TEST_CFLAGS)
libtn_values_tests_CPPFLAGS = $(TEST_CPPFLAGS)
libtn_values_tests_LDADD = libtn.a $(TEST_LDADD)
