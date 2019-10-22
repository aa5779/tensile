include $(top_srcdir)/common.am

noinst_LIBRARIES = libtn.a
noinst_HEADERS = compiler.h utils.h values.h
info_TEXINFOS = libtn.texi
libtn_TEXINFOS = $(noinst_HEADERS:.h=.texi) $(top_srcdir)/common.texi

libtn_a_SOURCES = utils.c values.c charset.c

TESTS = $(check_PROGRAMS)

check_PROGRAMS = libtn_utils_tests libtn_values_tests libtn_charset_tests

libtn_utils_tests_SOURCES = libtn_utils_tests.t
libtn_utils_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_utils_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_utils_tests_LDADD = libtn.a $(LDADD_FOR_TESTS)

libtn_values_tests_SOURCES = libtn_values_tests.t
libtn_values_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_values_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_values_tests_LDADD = libtn.a $(LDADD_FOR_TESTS)

libtn_charset_tests_SOURCES = libtn_charset_tests.t
libtn_charset_tests_CFLAGS = $(CFLAGS_FOR_TESTS)
libtn_charset_tests_CPPFLAGS = $(CPPFLAGS_FOR_TESTS)
libtn_charset_tests_LDADD = libtn.a $(LDADD_FOR_TESTS)

CLEANFILES = $(noinst_HEADERS:.h=.texi) \
			 $(libtn_utils_tests_SOURCES:.t=.c) \
			 $(libtn_values_tests_SOURCES:.t=.c) \
			 $(libtn_charset_tests_SOURCES:.t=.c) \
			 $(info_TEXINFOS:.texi=.info)

check-docs: $(noinst_LIBRARIES) $(noinst_HEADERS) \
			$(info_TEXINFOS) $(libtn_TEXINFOS)
