#####################################################################
#     W I D E L A N D S			M A K E F I L E                       #
#####################################################################

# Please edit everything in the general and in the OS specific sections
# after this, set the first variable to YES and start the build by 
# running GNU make

########################### GLOBAL SECTION ##########################

# C compiler
CC=gcc

# c++ compiler
CXX=c++

#  set this to YES if you're done here
IS_MAKEFILE_EDITED:=YES

# on some systems (BSD) this is named sdl12-config or so
SDL_CONFIG:=sdl-config

# additional build flags. if you're not a developer, you don't want
# to change this
ADD_CFLAGS:=-O3 -DDEBUG

# additional link flags. if you're not a developer, you don't want 
# to change this
ADD_LDFLAGS:=

########################### LINUX SECTION ##########################

# nothing special about linux at the moment


####################################################################
#  NO USER CHANGES BELOW THIS POINT											 #
####################################################################

all: dotest widelands 
	@echo -ne "\nCongrats. Build seems to be complete. If there was no "
	@echo -ne "error (ignore file not found errors), you can run the game "
	@echo -ne "now. just type: 'src/widelands' and enjoy!\n\n"
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

CFLAGS:=$(shell $(SDL_CONFIG) --cflags) $(ADD_CFLAGS)
CXXFLAGS=$(CFLAGS)
LDFLAGS=$(shell $(SDL_CONFIG) --libs) $(ADD_LDFALGS)

include $(WIDELANDS_OBJ:.o=.d)

%.d: %.cc
	$(CC) $(CFLAGS) -MM -MG "$<" | sed -e 's@^\(.*\)\.o:@src/\1.d src/\1.o:@' > $@ 

widelands: $(WIDELANDS_OBJ)
	$(CXX) $(LDFLAGS) $(CLAGS) $^ -o $@


