include $(top_srcdir)/common.am

SUBDIRS = . tests

noinst_LIBRARIES = libtn.a libtntest.a
noinst_HEADERS = compiler.h utils.h values.h ranges.h \
				 refs.h charset.h edits.h testing.h

libtn_a_SOURCES = utils.c values.c charset.c edits.c

libtntest_a_SOURCES = testing.c
