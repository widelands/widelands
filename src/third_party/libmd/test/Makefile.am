## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(top_srcdir)/include \
	# EOL

LDADD = $(top_builddir)/src/libmd.la

check_HEADERS = \
	test.h \
	# EOL

check_PROGRAMS = \
	md2 \
	md4 \
	md5 \
	rmd160 \
	sha1 \
	sha2 \
	# EOL

TESTS = $(check_PROGRAMS)
