#####################################################################
#     W I D E L A N D S			M A K E F I L E                       #
#####################################################################

# Do not change this file, instead create a new Makefile.local
# and overwrite the vaiables listed here

VERSION=svn`build/scons-tools/detect_revision.py`

########################### GLOBAL SECTION ##########################
# NON CROSS COMPILE

# Compile time includes
# Use the GGZ Gamingzone? Enable for releases.
ifndef USE_GGZ
USE_GGZ:=NO
endif

#doxygen
DOXYGEN=/usr/bin/doxygen

#older versions of SDL_mixer don't have RWops-Variants for loading samples and music
ifndef OLD_SDL_MIXER
OLD_SDL_MIXER:=NO
endif

#most current glibc systems implicitly contain libintl.so
ifndef IMPLICIT_LIBINTL
IMPLICIT_LIBINTL:=NO
endif

# Is this a cross compile?
ifndef CROSS
CROSS=NO
endif

SDL_CONFIG:=sdl-config

# ctags
CTAGS:=/usr/bin/ctags

ifeq ($(CROSS),NO)
# c++ compiler
ifndef CXX
CXX:=g++
endif

#ADD_CFLAGS:=-DNO_PARACHUTE

# additional build flags. if you're not a developer, you don't want
# to change this
#ADD_CFLAGS:= inherit flags from command line!

# additional link flags. if you're not a developer, you don't want
# to change this
#ADD_LDFLAGS:=


# Different build-types:
#  debug-no-parachute optimized, debugging symbols, disable SDLs parachute
#  debug-slow debugging symbols
#  debug      optimized, debugging symbols
#  release    optimized
#  profile    optimized, debugging symbols, profiling
#
ifndef BUILD
BUILD:=debug
endif

endif

-include Makefile.local

####################################################################
#  NO USER CHANGES BELOW THIS POINT											 #
####################################################################

###
#
# COMPILE TIME configuration
ifeq ($(USE_GGZ),YES)
ADD_CFLAGS:= $(ADD_CFLAGS) -DUSE_GGZ
ADD_LDFLAGS:=-lggzmod -lggzcore -lggz
endif

##############################################################################
# Cross compiling options

ifneq ($(CROSS),NO)
# CROSS COMPILE, for developer only
TARGET:=i586-mingw32msvc
PREFIX:=/usr/local/cross-tools
PATH:=$(PREFIX)/$(TARGET)/bin:$(PREFIX)/bin:$(PATH)

CXX=$(TARGET)-g++

# manually overwrite
#SDL_CONFIG=$(PREFIX)/bin/$(TARGET)-sdl-config
SDL_CONFIG=$(PREFIX)/$(TARGET)/bin/$(TARGET)-sdl-config

else
TARGET:=native
endif


##############################################################################
# Flags configuration
BUILD:=$(strip $(BUILD))

ifeq ($(BUILD),release)
OPTIMIZE:=yes
# heavy optimization
#ADD_CFLAGS:=$(ADD_CFLAGS) -fomit-frame-pointer -finline-functions -ffast-math -funroll-loops -funroll-all-loops -fexpensive-optimizations
# !!!! -fomit-frame-pointer breaks execeptions !!!!
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
ADD_CFLAGS += -O3
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

OBJECT_DIR:=src/$(TARGET)-$(BUILD)
CFLAGS:=-Wall $(shell $(SDL_CONFIG) --cflags) $(ADD_CFLAGS)
CXXFLAGS:=$(CFLAGS)
LDFLAGS:=$(shell $(SDL_CONFIG) --libs) $(ADD_LDFLAGS) -lz -lpng -lSDL_image -lSDL_mixer -lSDL_ttf -lSDL_net -lSDL_gfx

##############################################################################
# Building
warning:
	@echo "==============================================================="
	@echo "    Building Widelands using the Makefile is deprecated."
	@echo "    I'm trying to run scons now."
	@echo "==============================================================="
	@echo
	scons

all: makedirs tags $(OBJECT_DIR)/widelands
	cp $(OBJECT_DIR)/widelands .
	@echo
	@echo "    Congratulations. Build seems to be completed without error."
	@echo "    You can run the game now. just type: './widelands' and enjoy!"
	@echo "    Comments go to widelands-public@lists.sf.net"
	@echo
	@echo "            THE WIDELANDS DEVELOPMENT TEAM"

clean:
	@-rm -rf widelands
	@-rm -rf *.da src/*.da
	@-rm -rf $(OBJECT_DIR)/*.o $(OBJECT_DIR)/*/*.o $(OBJECT_DIR)/*/*/*.o $(OBJECT_DIR)/*/*/*/*.o
	@-rm -rf $(OBJECT_DIR)/*.d $(OBJECT_DIR)/*/*.d $(OBJECT_DIR)/*/*/*.d $(OBJECT_DIR)/*/*/*/*.d
	@-rm -rf $(OBJECT_DIR)/widelands
	@-rm -rf *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~

doc: $(SRC) $(HEADERS)
	@doxygen Doxyfile

dist:
	@rm -rf widelands-$(VERSION)
	@mkdir widelands-$(VERSION)
	@cp -r AUTHORS ChangeLog fonts maps pics README-compiling.txt COPYING Makefile README README.developers tribes worlds widelands-$(VERSION)
	@for n in `find src -name '*.cc'` `find src -name '*.h'` ; do \
	   mkdir -p widelands-$(VERSION)/`dirname $$n` ; \
	   cp $$n widelands-$(VERSION)/`dirname $$n` ; \
	 done
	@find widelands-$(VERSION) -name CVS | xargs rm -rf
	@find widelands-$(VERSION) -name .cvsignore | xargs rm -f
	@tar cf - widelands-$(VERSION) | gzip -9c - > widelands-$(VERSION).tar.gz
	@rm -rf widelands-$(VERSION)

# WIDELANDS MAIN PROGRAM BUILD RULES

SUBDIRS=src src/ui/ui_fs_menus src/ui/ui_basic src/editor src/editor/ui_menus src/editor/tools src/filesystem src/sound src/trigger src/events

CFLAGS += $(patsubst %,-I%,$(SUBDIRS))
CXXFLAGS += $(patsubst %,-I%,$(SUBDIRS))
SRC := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cc))
HEADERS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))
OBJ := $(patsubst src/%.cc,$(OBJECT_DIR)/%.o$,$(SRC))
DEP := $(OBJ:.o=.d)

Q = @

makedirs:
	$(Q)-mkdir -p $(OBJECT_DIR) $(patsubst src/%,$(OBJECT_DIR)/%,$(SUBDIRS))

$(OBJECT_DIR)/widelands: $(OBJ)
	@echo "===> LD $@"
	$(Q)$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(CFLAGS)

-include $(DEP)

$(OBJECT_DIR)/%.o: src/%.cc src/config.h
	@echo "===> CXX $<"
	$(Q)$(CXX) -pipe $(CXXFLAGS) -MMD -MP -MF $@.d -c -o $@ $<
	$(Q)sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' $@.d > $(OBJECT_DIR)/$*.d
	$(Q)rm $@.d

tags: $(SRC) $(HEADERS)
	@ if [ -x $(CTAGS) ]; then $(CTAGS) $(SRC) $(HEADERS) 2>/dev/null|| true ; else true; fi

update up:
	@cvs -q up -APd

src/config.h:
	@cp src/config.h.default $@

src/build_id.cc:
	sed -e "s/UNKNOWN/$(VERSION)/" src/build_id.cc.default > $@

