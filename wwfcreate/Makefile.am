EXTRA_FLAGS=@SDL_CFLAGS@ -Wall -O2 -I../src 
#EXTRA_FLAGS=@SDL_CFLAGS@ -DDEBUG -DPKGDATADIR=\"$(pkgdatadir)\" -Wall -fomit-frame-pointer -ffast-math -fexpensive-optimizations -malign-loops=2 -malign-jumps=2 -malign-functions=2
# LDFLAGS=
CFLAGS= $(EXTRA_FLAGS)
CXXFLAGS= $(EXTRA_FLAGS)

EXTRA_DIST=README

bin_PROGRAMS=wwfcreate 
wwfcreate_LDADD =@SDL_LIBS@ ../wtfcreate/file_descr.o ../wtfcreate/pic_descr.o ../wtfcreate/parse_bobs.o ../wtfcreate/logic_bob_descr.o ../wtfcreate/need_list.o ../wtfcreate/bob_descr.o ../src/md5file.o ../src/myfile.o ../src/md5.o ../src/helper.o ../src/profile.o ../src/growablearray.o ../src/pic.o 
wwfcreate_SOURCES=main.cc \
                  parse.cc \
                  parse.h \
                  wwfcreate.h \
                  write_worldfile.cc \
                  write_worldfile.h \
                  resource_descr.h \
                  resource_descr.cc \
                  terrain_descr.h \
                  terrain_descr.cc 

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
