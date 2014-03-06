#####################################################################
#     W I D E L A N D S       M A K E F I L E                       #
#####################################################################
# Do not change this file, instead create a new Makefile.local
# and overwrite the variables listed here

# Use of OpenGL?
ifndef OPENGL
OPENGL:=NO
endif


#doxygen
DOXYGEN=/usr/bin/doxygen

#older versions of SDL_mixer don't have RWops-Variants for loading samples and music
ifndef OLD_SDL_MIXER
OLD_SDL_MIXER:=NO
endif

#most current glibc systems implicitly contain libintl.so
IMPLICIT_LIBINTL:=NO

SDL_CONFIG:=sdl-config

# ctags
CTAGS:=/usr/bin/ctags

#ADD_CFLAGS:=-DNO_PARACHUTE


# Different build-types:
#  debug      not optimized, debugging symbols
#  release    optimized
#  profile    optimized, debugging symbols, profiling
#
ifndef BUILD
BUILD:=debug
endif

ifndef CXX
CXX:=g++
endif

TARGET:=native
VERSION:=build18

WINDRES = windres.exe
icon.o: Widelands_win.rc

-include Makefile.local

####################################################################
#  NO USER CHANGES BELOW THIS POINT                                #
####################################################################

ifeq ($(OPENGL),YES)
ADD_CFLAGS:= $(ADD_CFLAGS) -DHAS_OPENGL
endif

##############################################################################
# Flags configuration
BUILD:=$(strip $(BUILD))

ifeq ($(BUILD),release)
OPTIMIZE:=yes
# heavy optimization
ADD_CFLAGS:=$(ADD_CFLAGS) -finline-functions -ffast-math -funroll-loops -funroll-all-loops -fexpensive-optimizations
else
ifeq ($(BUILD),profile)
OPTIMIZE:=yes
DEBUG:=yes
PROFILE:=yes
else
ifeq ($(BUILD),debug)
DEBUG:=yes
endif
endif
endif

ifdef OPTIMIZE
#fix bug with mingw/gcc under windows. revert back to -O3 after(if ever) gcc is fixed
#ADD_CFLAGS += -O3 -fno-tree-ch
endif

ifdef DEBUG
ADD_CFLAGS += -g -DDEBUG  -fmessage-length=0
else
ADD_CFLAGS += -DNDEBUG
endif

ifdef PROFILE
ADD_CFLAGS += -pg -fprofile-arcs
endif

ifeq ($(OLD_SDL_MIXER),YES)
ADD_CFLAGS += -DOLD_SDL_MIXER
endif

ifeq ($(IMPLICIT_LIBINTL),NO)
ADD_LDFLAGS += -lintl
endif

##############################################################################
# Object files and directories, final compilation flags

OBJECT_DIR:=../../$(TARGET)-$(BUILD)
CFLAGS:=-Wall $(shell $(SDL_CONFIG) --cflags) $(ADD_CFLAGS)
CXXFLAGS:=$(CFLAGS)
LDFLAGS:=$(shell $(SDL_CONFIG) --libs) icon.o $(ADD_LDFLAGS) -lz -lpng -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL_net -lSDL_gfx -lwsock32 -llua

##############################################################################
# Building
all: win32tool makedirs tags $(OBJECT_DIR)/widelands.exe
	cp -r $(OBJECT_DIR)/widelands.exe ../../../
	@echo
	@echo "    Congratulations. Build seems to be completed without error."

clean:
	@-rm -rf ../../../widelands.exe
	@-rm -rf ../../../*.da ../../../src/*.da
	@-rm -rf $(OBJECT_DIR)/*.o $(OBJECT_DIR)/*/*.o $(OBJECT_DIR)/*/*/*.o $(OBJECT_DIR)/*/*/*/*.o
	@-rm -rf $(OBJECT_DIR)/*.d $(OBJECT_DIR)/*/*.d $(OBJECT_DIR)/*/*/*.d $(OBJECT_DIR)/*/*/*/*.d
	@-rm -rf $(OBJECT_DIR)/widelands.exe
	@-rm -rf ../../../*~ ../../../*/*~ ../../../*/*/*~ ../../../*/*/*/*~ ../../../*/*/*/*/*~

doc: $(SRC) $(HEADERS)
	@doxygen Doxyfile

dist:
	@rm -rf ../../../widelands-$(VERSION)
	@mkdir ../../../widelands-$(VERSION)
	@cp -r ../../../campaigns ../../../doc ../../../fonts ../../../game_server ../../../locale ../../../maps ../../../music ../../../pics ../../../sound ../../../tribes ../../../txts ../../../worlds ../../../ChangeLog ../../../COPYING ../../../CREDITS ../../../widelands.exe ../../../widelands-$(VERSION)
	@find ../../../widelands-$(VERSION) -name .svn | xargs rm -rf
	@find ../../../widelands-$(VERSION) -name .cvsignore | xargs rm -f
	@echo "    Your build was exported to widelands-$(VERSION)"
	@echo
	@echo "         THE WIDELANDS DEVELOPMENT TEAM            "

# WIDELANDS MAIN PROGRAM BUILD RULES

SUBDIRS  = ../../../src ../../../src/io ../../../src/io/filesystem ../../../src/network ../../../src/economy ../../../src/ai ../../../src/ui_fsmenu
SUBDIRS += ../../../src/ui_basic ../../../src/editor ../../../src/editor/ui_menus ../../../src/editor/tools ../../../src/events ../../../src/sound
SUBDIRS += ../../../src/trigger ../../../src/map_io ../../../src/wui ../../../src/logic ../../../src/profile ../../../src/graphic ../../../src/game_io
SUBDIRS += ../../../src/scripting
CFLAGS += $(patsubst %,-I%,$(SUBDIRS))
CXXFLAGS += $(patsubst %,-I%,$(SUBDIRS))
SRC := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cc))
HEADERS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))
OBJ := $(patsubst ../../../src/%.cc,$(OBJECT_DIR)/%.o$,$(SRC))
DEP := $(OBJ:.o=.d)

Q = @

makedirs:
	$(Q)-mkdir -p $(OBJECT_DIR) $(patsubst ../../../src/%,$(OBJECT_DIR)/%,$(SUBDIRS))

$(OBJECT_DIR)/widelands.exe: $(OBJ)
	@echo "===> LD $@"
	$(Q)$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(CFLAGS)

-include $(DEP)

$(OBJECT_DIR)/%.o: ../../../src/%.cc
	@echo "===> CXX $<"
	$(Q)$(CXX) -pipe $(CXXFLAGS) -MMD -MP -MF $@.d -c -o $@ $<
	$(Q)sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' $@.d > $(OBJECT_DIR)/$*.d
	$(Q)rm $@.d

tags: $(SRC) $(HEADERS)
	@ if [ -x $(CTAGS) ]; then $(CTAGS) $(SRC) $(HEADERS) 2>/dev/null|| true ; else true; fi

win32tool:
	@echo "===> Creating icon.o for win32"
	@$(WINDRES) -i Widelands_win.rc --input-format=rc -o icon.o -O coff
	@echo "===> Copying configuration files to \"src\""
	@rm -rf ../../../src/config.h
	@cp -r ./config.h ../../../src
	@rm -rf ../../../src/build_info.cc
	@cp -r ./build_info.cc ../../../src
	@rm -rf ../../../src/wlversion.h
	@cp -r ./wlversion.h ../../../src
	@touch ../../../src/build_info.cc
