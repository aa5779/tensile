include $(top_srcdir)/common.am

SUBDIRS = lib

AM_YFLAGS = -d -Wall -Werror

bin_PROGRAMS = tensilec
noinst_LIBRARIES = libtngram.a

BUILT_SOURCES = grammar.h

tensilec_SOURCES = tensilec.c
tensilec_LDADD = libtngram.a lib/libtn.a

libtngram_a_SOURCES = lexeme.l grammar.y
libtngram_a_CFLAGS = $(AM_CFLAGS) -Wno-switch-default -Wno-unused-function
