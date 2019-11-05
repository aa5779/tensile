include $(top_srcdir)/common.am

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_utils_tests libtn_refs_tests libtn_ranges_tests \
				 libtn_charset_tests libtn_edits_tests

libtn_utils_tests_SOURCES = libtn_utils_tests.t
libtn_utils_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_utils_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_utils_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_refs_tests_SOURCES = libtn_refs_tests.t
libtn_refs_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_refs_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_refs_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_ranges_tests_SOURCES = libtn_ranges_tests.t
libtn_ranges_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_ranges_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_ranges_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_charset_tests_SOURCES = libtn_charset_tests.t
libtn_charset_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_charset_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_charset_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_edits_tests_SOURCES = libtn_edits_tests.t
libtn_edits_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_edits_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_edits_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

CLEANFILES = $(libtn_utils_tests_SOURCES:.t=.c) \
			 $(libtn_refs_tests_SOURCES:.t=.c) \
			 $(libtn_ranges_tests_SOURCES:.t=.c) \
			 $(libtn_charset_tests_SOURCES:.t=.c) \
			 $(libtn_edits_tests_SOURCES:.t=.c)
