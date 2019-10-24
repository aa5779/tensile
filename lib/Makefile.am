include $(top_srcdir)/common.am

SUBDIRS = tests

noinst_LIBRARIES = libtn.a
noinst_HEADERS = compiler.h utils.h values.h
info_TEXINFOS = libtn.texi
libtn_TEXINFOS = $(noinst_HEADERS:.h=.texi) $(top_srcdir)/common.texi

libtn_a_SOURCES = utils.c values.c charset.c

CLEANFILES = $(noinst_HEADERS:.h=.texi) \
			 $(info_TEXINFOS:.texi=.info)

check-docs: $(noinst_LIBRARIES) $(noinst_HEADERS) \
			$(info_TEXINFOS) $(libtn_TEXINFOS)
