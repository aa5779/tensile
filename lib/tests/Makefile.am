include $(top_srcdir)/common.am

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_utils_tests libtn_values_tests libtn_charset_tests

libtn_utils_tests_SOURCES = libtn_utils_tests.t
libtn_utils_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_utils_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_utils_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_values_tests_SOURCES = libtn_values_tests.t
libtn_values_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_values_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_values_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_charset_tests_SOURCES = libtn_charset_tests.t
libtn_charset_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_charset_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_charset_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

CLEANFILES = $(libtn_utils_tests_SOURCES:.t=.c) \
			 $(libtn_values_tests_SOURCES:.t=.c) \
			 $(libtn_charset_tests_SOURCES:.t=.c)
