#EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -DKEEP_STANDART_ASSERT=1
#EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -fomit-frame-pointer -ffast-math -fexpensive-optimizations -malign-loops=2 -malign-jumps=2 -malign-functions=2 -DKEEP_STANDART_ASSERT=1
# LDFLAGS=
EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"/usr/local/share/widelands\" -Wall -O3 -pg -fprofile-arcs -g -DKEEP_STANDART_ASSERT=1
CFLAGS= $(EXTRA_FLAGS)
CXXFLAGS= $(EXTRA_FLAGS)

EXTRA_DIST=README

bin_PROGRAMS=wtfcreate
wtfcreate_LDADD =@SDL_LIBS@  ../src/md5file.o ../src/myfile.o ../src/md5.o ../src/helper.o ../src/profile.o ../src/growablearray.o ../src/pic.o 
wtfcreate_SOURCES=main.cc \
                  wtfcreate.h \
                  building_descr.h \
                  building_descr.cc \
                  file_descr.h \
                  file_descr.cc \
                  bob_descr.h \
                  bob_descr.cc \
                  ware_descr.h \
                  ware_descr.cc \
                  fabric.h \
                  worker_descr.cc \
                  worker_descr.h \
                  soldier_descr.cc \
                  soldier_descr.h \
                  tribe_descr.h \
                  tribe_descr.cc \
                  logic_bob_descr.h \
                  logic_bob_descr.cc \
                  parse.cc \
                  parse.h \
                  need_list.h \
                  need_list.cc \
                  write_tribefile.h \
                  write_tribefile.cc \
                  pic_descr.cc \
                  pic_descr.h \
                  parse_bobs.h \
                  parse_bobs.cc

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
