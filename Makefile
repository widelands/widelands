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
CROSS=NO

# on some systems (BSD) this is named sdl12-config or so
SDL_CONFIG:=sdl-config

ifeq ($(CROSS),NO)
# C compiler
CC=gcc

# c++ compiler
CXX=c++

# additional build flags. if you're not a developer, you don't want
# to change this
ADD_CFLAGS:=-g -O3 -DDEBUG

# additional link flags. if you're not a developer, you don't want 
# to change this
ADD_LDFLAGS:=
endif

########################### LINUX SECTION ##########################

# nothing special about linux at the moment


####################################################################
#  NO USER CHANGES BELOW THIS POINT											 #
####################################################################

ifneq ($(CROSS),NO) 
# CROSS COMPILE, for developer only
PREFIX:=/usr/local/cross-tools
TARGET:=i386-mingw32msvc
PATH:=$(PREFIX)/$(TARGET)/bin:$(PREFIX)/bin:$(PATH)

CC=$(TARGET)-gcc
CXX=$(TARGET)-c++

# manually overwrite
SDL_CONFIG=$(PREFIX)/$(TARGET)/bin/sdl-config

# is for sure a release
ADD_CFLAGS=-O3 

endif

all: dotest widelands 
	@echo -ne "\nCongrats. Build seems to be complete. If there was no "
	@echo -ne "error (ignore file not found errors), you can run the game "
	@echo -ne "now. just type: './widelands' and enjoy!\n\n"
	@echo -e "\tTHE WIDELANDS DEVELOPMENT TEAM"

dotest:
ifneq ($(IS_MAKEFILE_EDITED),YES)
		$(error Please edit the Makefile and set the IS_MAKEFILE_EDITED \
		variable to YES)
endif

clean:
	@rm -rf widelands 
	@rm -rf src/*.da src/*.o
	@rm -rf *~ */*~ */*/*/*~

# WIDELANDS MAIN PROGRAM BUILD RULES

# note: all src/*.cc files are considered source code
WIDELANDS_SRC:=$(wildcard src/*.cc)
WIDELANDS_OBJ:=$(WIDELANDS_SRC:.cc=.o)

CFLAGS:=-Wall $(shell $(SDL_CONFIG) --cflags) $(ADD_CFLAGS)
CXXFLAGS=$(CFLAGS)
LDFLAGS=$(shell $(SDL_CONFIG) --libs) $(ADD_LDFLAGS)

include $(WIDELANDS_OBJ:.o=.d)

%.d: %.cc
	$(CC) $(CFLAGS) -MM -MG "$<" | sed -e 's@^\(.*\)\.o:@src/\1.d src/\1.o:@' > $@ 

%.h: 
	

widelands: $(WIDELANDS_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(CLAGS)  
