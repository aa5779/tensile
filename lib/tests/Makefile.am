include $(top_srcdir)/common.am

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_sanity_tests libtn_utils_tests libtn_refs_tests \
				 libtn_ranges_tests libtn_charset_tests \
				 libtn_edits_tests libtn_storage_tests

libtn_sanity_tests_SOURCES = libtn_sanity_tests.c
libtn_sanity_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_sanity_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_sanity_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)


libtn_utils_tests_SOURCES = libtn_utils_tests.c
libtn_utils_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_utils_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_utils_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_refs_tests_SOURCES = libtn_refs_tests.c
libtn_refs_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_refs_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_refs_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_ranges_tests_SOURCES = libtn_ranges_tests.c
libtn_ranges_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_ranges_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_ranges_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_charset_tests_SOURCES = libtn_charset_tests.c
libtn_charset_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_charset_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_charset_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_edits_tests_SOURCES = libtn_edits_tests.c
libtn_edits_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_edits_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_edits_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)

libtn_storage_tests_SOURCES = libtn_storage_tests.c
libtn_storage_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_storage_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_storage_tests_LDADD = ../libtn.a $(LDADD_FOR_TESTS)
