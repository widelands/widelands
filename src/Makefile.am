EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -O3 -g -pg -fprofile-arcs -g # -O3
#EXTRA_FLAGS=@SDL_CFLAGS@ -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -fomit-frame-pointer -ffast-math -fexpensive-optimizations -malign-loops=2 -malign-jumps=2 -malign-functions=2 -O3
# LDFLAGS=
CFLAGS= $(EXTRA_FLAGS)
CXXFLAGS= $(EXTRA_FLAGS)

# EXTRA_DIST=worlds pics

bin_PROGRAMS=widelands
widelands_LDADD = @SDL_LIBS@
widelands_SOURCES=main.cc \
						singleton.h \
						files.cc \
						parser.h \
						parser.cc \
						options.cc \
						options.h \
						graphic.cc \
						pic.cc \
						graphic.h \
						input.h \
						input.cc \
						cursor.cc \
						cursor.h \
						mainmenue.h \
						mainmenue.cc \
						ui.cc \
						ui.h \
						font.h \
						font.cc \
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
						map.h \
						map.cc \
						game.cc \
						game.h \
						IntPlayer.cc \
						IntPlayer.h \
						mapview.h \
						mapview.cc \
						minimap.h \
						minimap.cc \
						watchwindow.h \
						watchwindow.cc \
						fieldaction.h \
						fieldaction.cc \
						field.cc \
						profile.cc \
						profile.h \
						world.cc \
						world.h \
						bob.cc \
						bob.h \
						button.cc \
						textarea.cc \
						multiline_textarea.cc \
						window.cc \
						checkbox.cc \
						radiobutton.cc \
						listselect.cc \
						scrollbar.cc \
						launchmenu.cc \
						mapselectmenue.h \
						mapselectmenue.cc \
						optionsmenu.h \
						optionsmenu.cc \
						s2map.h \
						s2map.cc \
						criterr.cc \
						md5.cc \
						md5.h \
                  pic.h \
                  auto_pic.h \
                  auto_pic.cc \
                  helper.cc \
                  helper.h \
                  tribe.h \
                  tribe.cc \
                  tribedata.h \
                  player.h \
                  player.cc \
                  descr_maintainer.h \
                  cmd_queue.h \
                  cmd_queue.cc \
                  widelands.h \
                  building.h \
                  building.cc \
                  creature.h \
                  creature.cc \
                  instances.h \
                  instances.cc \
                  ware.h \
                  ware.cc \
                  worker.h \
                  worker.cc \
                  worlddata.h \
                  field.h

# TO INCLUDE IN CVS: 


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
