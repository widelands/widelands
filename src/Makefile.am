EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -O3 -pg -fprofile-arcs -g
#EXTRA_FLAGS=@SDL_CFLAGS@ -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -fomit-frame-pointer -ffast-math -fexpensive-optimizations -malign-loops=2 -malign-jumps=2 -malign-functions=2 -O3
# LDFLAGS=
CFLAGS= $(EXTRA_FLAGS)
CXXFLAGS= $(EXTRA_FLAGS)

# EXTRA_DIST=worlds pics 

bin_PROGRAMS=widelands 
widelands_LDADD = @SDL_LIBS@
widelands_SOURCES=main.cc \
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
						setup.cc \
						setup.h \
						counter.h \
						counter.cc \
						mydirent.cc \
						mydirent.h \
						singlepmenue.cc \
						singlepmenue.h \
						menuecommon.cc \
						menuecommon.h \
						growablearray.cc \
						growablearray.h \
						mytypes.h \
						map.h \
						map.cc \
						game.cc \
						game.h \
						IntPlayer.cc \
						IntPlayer.h \
						mapview.h \
						mapview.cc \
						field.cc \
						profile.cc \
						profile.h \
						world.cc \
						world.h \
						bob.cc \
						bob.h \
						worldfiletypes.h \
						tribefiletypes.h \
						button.cc \
						textarea.cc \
						multiline_textarea.cc \
						window.cc \
						checkbox.cc \
						listselect.cc \
						mapselectmenue.h \
						mapselectmenue.cc \
						s2map.h \
						s2map.cc \
						criterr.h \
						criterr.cc \
						myassert.h \
						md5.cc \
						md5.h
						


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
