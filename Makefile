#####################################################################
#     W I D E L A N D S			M A K E F I L E                       #
#####################################################################

# Please edit everything in the general and in the OS specific sections
# after this, set the first variable to YES and start the build by
# running GNU make

########################### GLOBAL SECTION ##########################
# NON CROSS COMPILE
#
#  set this to YES if you're done here
IS_MAKEFILE_EDITED:=YES

# Is this a cross compile?
ifndef CROSS
CROSS=NO
endif

# on some systems (BSD) this is named sdl12-config or so
SDL_CONFIG:=sdl-config

ifeq ($(CROSS),NO)
# c++ compiler
ifndef CXX
CXX:=g++
endif

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

########################### LINUX SECTION ##########################

# nothing special about linux at the moment


####################################################################
#  NO USER CHANGES BELOW THIS POINT											 #
####################################################################

ifneq ($(IS_MAKEFILE_EDITED),YES)
$(error Please edit the Makefile and set the IS_MAKEFILE_EDITED variable to YES)
endif

##############################################################################
# Cross compiling options

ifneq ($(CROSS),NO)
# CROSS COMPILE, for developer only
TARGET:=i586-mingw32msvc
PREFIX:=/usr
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
ifeq ($(BUILD),debug-slow)
DEBUG:=yes
else
ifeq ($(BUILD),debug)
BUILD:=debug
OPTIMIZE:=yes
DEBUG:=yes
else
BUILD:=debug-no-parachute
OPTIMIZE:=YES
DEBUG:=YES
ADD_CFLAGS:=-DNO_PARACHUTE
endif
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

##############################################################################
# Object files and directories, final compilation flags

OBJECT_DIR:=src/$(TARGET)-$(BUILD)
CFLAGS:=-Wall $(shell $(SDL_CONFIG) --cflags) $(ADD_CFLAGS)
CXXFLAGS:=$(CFLAGS)
LDFLAGS:=$(shell $(SDL_CONFIG) --libs) $(ADD_LDFLAGS) -lSDL_image -lSDL_ttf -lefence

##############################################################################
# Building
all: makedirs tags $(OBJECT_DIR)/widelands
	cp $(OBJECT_DIR)/widelands .
	@echo
	@echo "    Congrats. Build seems to be complete. If there was no "
	@echo "    error (ignore file not found errors), you can run the game "
	@echo "    now. just type: './widelands' and enjoy!"
	@echo
	@echo "            THE WIDELANDS DEVELOPMENT TEAM"
	@echo

clean:
	@-rm -rf widelands
	@-rm -rf *.da src/*.da
	@-rm -rf $(OBJECT_DIR)/*.o $(OBJECT_DIR)/*/*.o $(OBJECT_DIR)/*/*/*.o $(OBJECT_DIR)/*/*/*/*.o 
	@-rm -rf $(OBJECT_DIR)/*.d $(OBJECT_DIR)/*/*.d $(OBJECT_DIR)/*/*/*.d $(OBJECT_DIR)/*/*/*/*.d 
	@-rm -rf *~ */*~ */*/*~ */*/*/*~

# WIDELANDS MAIN PROGRAM BUILD RULES

SUBDIRS=src src/ui/ui_fs_menus src/ui/ui_basic src/editor src/editor/ui_menus src/editor/tools

CFLAGS += $(patsubst %,-I%,$(SUBDIRS))
CXXFLAGS += $(patsubst %,-I%,$(SUBDIRS))
SRC := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cc))
HEADERS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.h))
OBJ := $(patsubst src/%.cc,$(OBJECT_DIR)/%.o$,$(SRC))
DEP := $(OBJ:.o=.d)

makedirs:
	-mkdir -p $(OBJECT_DIR) $(patsubst src/%,$(OBJECT_DIR)/%,$(SUBDIRS))

$(OBJECT_DIR)/widelands: $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS) $(CFLAGS)

-include $(DEP)

$(OBJECT_DIR)/%.o: src/%.cc
	$(CXX) -pipe $(CXXFLAGS) -MMD -MP -MF $@.d -c -o $@ $<
	sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' $@.d > $(OBJECT_DIR)/$*.d
	rm $@.d

tags: $(SRC) $(HEADERS)
	@ if [ -x /usr/bin/ctags ]; then ctags $(SRC) $(HEADERS) 2>/dev/null|| true ; else true; fi

update up:
	@cvs -q up -APd
