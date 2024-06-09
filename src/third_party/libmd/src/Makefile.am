## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-I$(top_builddir) \
	-I$(srcdir) \
	-I$(top_srcdir)/include \
	# EOL

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	libmd.pc \
	# EOL

lib_LTLIBRARIES = libmd.la

libmd_la_helper_sources = \
	md2hl.c \
	md4hl.c \
	md5hl.c \
	rmd160hl.c \
	sha1hl.c \
	sha224hl.c \
	sha256hl.c \
	sha384hl.c \
	sha512hl.c \
	sha512_256hl.c \
	# EOL

EXTRA_libmd_la_DEPENDENCIES = \
	libmd.map

libmd_la_LDFLAGS = \
	-no-undefined \
	-version-number $(SOVERSION) \
	# EOL
if HAVE_LINKER_VERSION_SCRIPT
libmd_la_LDFLAGS += \
	-Wl,--version-script=$(srcdir)/libmd.map \
	# EOL
else
libmd_la_LDFLAGS += \
	-export-symbols libmd.sym \
	# EOL
EXTRA_libmd_la_DEPENDENCIES += \
	libmd.sym \
	# EOL
endif
EXTRA_libmd_la_DEPENDENCIES += \
	helper.c \
	# EOL
libmd_la_SOURCES = \
	local-link.h \
	md2.c \
	md4.c \
	md5.c \
	rmd160.c \
	sha1.c \
	sha2.c \
	$(libmd_la_helper_sources) \
	# EOL

BUILT_SOURCES = \
	$(libmd_la_helper_sources) \
	# EOL

EXTRA_DIST = \
	libmd.map \
	libmd.pc.in \
	helper.c \
	# EOL

CLEANFILES = \
	$(libmd_la_helper_sources) \
	# EOL

DISTCLEANFILES = \
	libmd.sym \
	# EOL

# Generate a simple libtool symbol export list to be used as a fallback if
# there is no version script support.
libmd.sym: libmd.map
	$(AM_V_GEN) $(SED) -ne 's/^[[:space:]]\{1,\}\([A-Za-z0-9_]\{1,\}\);/\1/p' libmd.map > $@

md2hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/md2.h/g' -e 's/HASH/MD2/g' "$(srcdir)/helper.c" > $@

md4hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/md4.h/g' -e 's/HASH/MD4/g' "$(srcdir)/helper.c" > $@

md5hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/md5.h/g' -e 's/HASH/MD5/g' "$(srcdir)/helper.c" > $@

rmd160hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/rmd160.h/g' -e 's/HASH/RMD160/g' "$(srcdir)/helper.c" > $@

sha1hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha1.h/g' -e 's/HASH/SHA1/g' "$(srcdir)/helper.c" > $@

sha224hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha2.h/g' -e 's/HASH/SHA224/g' \
	                -e 's/SHA[0-9][0-9][0-9]_CTX/SHA2_CTX/g' "$(srcdir)/helper.c" > $@

sha256hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha2.h/g' -e 's/HASH/SHA256/g' \
	                -e 's/SHA[0-9][0-9][0-9]_CTX/SHA2_CTX/g' "$(srcdir)/helper.c" > $@

sha384hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha2.h/g' -e 's/HASH/SHA384/g' \
	                -e 's/SHA[0-9][0-9][0-9]_CTX/SHA2_CTX/g' "$(srcdir)/helper.c" > $@

sha512hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha2.h/g' -e 's/HASH/SHA512/g' \
	                -e 's/SHA[0-9][0-9][0-9]_CTX/SHA2_CTX/g' "$(srcdir)/helper.c" > $@

sha512_256hl.c: helper.c
	$(AM_V_GEN) $(SED) -e 's/hashinc/sha2.h/g' -e 's/HASH/SHA512_256/g' \
	                -e 's/SHA512_256_CTX/SHA2_CTX/g' "$(srcdir)/helper.c" > $@

runtimelibdir = $(libdir)

install-exec-hook:
	if [ "$(libdir)" != "$(runtimelibdir)" ]; then \
		$(MKDIR_P) $(DESTDIR)$(runtimelibdir); \
		mv $(DESTDIR)$(libdir)/libmd*.so.* \
		   $(DESTDIR)$(runtimelibdir)/; \
		soname=`readlink $(DESTDIR)$(libdir)/libmd.so`; \
		sorelprefix=`echo $(libdir) | $(SED) -r -e 's:(^/)?[^/]+:..:g'`; \
		ln -sf $$sorelprefix$(runtimelibdir)/$$soname \
		       $(DESTDIR)$(libdir)/libmd.so; \
	fi

uninstall-hook:
	rm -f $(DESTDIR)$(runtimelibdir)/libmd*.so*
