EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -UDONT_USE_ASM -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -O3 -g -fprofile-arcs -pg
# LDFLAGS=
CFLAGS= $(EXTRA_FLAGS)
CXXFLAGS= $(EXTRA_FLAGS)

# EXTRA_DIST=worlds pics 

bin_PROGRAMS=widelands
widelands_LDADD = @SDL_LIBS@  # -lefence 
widelands_SOURCES=main.cc \
						unix_m.cc \
						os.h \
						singleton.h \
						fileloc.cc \
						fileloc.h \
						myfile.h \
						myfile.cc \
						parser.h \
						parser.cc \
						options.cc \
						options.h \
						output.h \
						graphic.cc \
						pic.cc \
						graphic.h \
						input.h \
						input.cc \
						intro.cc \
						intro.h \
						cursor.cc \
						cursor.h \
						mainmenue.h \
						mainmenue.cc \
						ui.cc \
						ui.h \
						font.h \
						font.cc \
						errors.h \
						main.h \
						setup.cc \
						setup.h \
						counter.h \
						counter.cc \
						mydirent.cc \
						mydirent.h


#  uncomment the following if widelands requires the math library
#widelands_LDADD=-lm

# EXTRA_DIST=widelands.lsm.in #NAME.texinfo widelands.spec.in

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
# info_TEXINFOS = widelands.texinfo
# widelands_TEXINFOS = gpl.texinfo

#  install the man pages
# man_MANS=widelands.1
