/*
 * Copyright (C) 2002 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
/*
 * NOTE: Include this file first thing in all source files, and only
 * there.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <math.h>

#include <iostream.h>
#include <fstream.h>

#include <vector>
#include <map>
#include <queue>

#ifdef WIN32
	#pragma warning(disable : 4250) // *sigh* multiple inheritance *sigh*
	#pragma warning(disable : 4800) // forcing values to bool is fine to me.
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#ifdef _DEBUG			//chk ths plz. Acked by Holger
		#define DEBUG
	#endif //_DEBUG

	#define snprintf _snprintf
	#define vsnprintf _vsnprintf
	#define strcasecmp strcmpi
	#define strncasecmp strncmpi
#else //WIN32
//   #include <sys/types.h>
//   #include <sys/stat.h>
//   #include <fcntl.h>
//   #include <unistd.h>
//	#define strcmpi strcasecmp
//	#define strncmpi strncasecmp
#endif //WIN32

#include "config.h"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

/*
==============================================================================

Error handling functions

==============================================================================
*/

#define RET_OK 				0
#define RET_EXIT           1
#define ERR_NOT_IMPLEMENTED -1
#define ERR_INVAL_ARG -2
#define ERR_FAILED -255

extern ofstream out; // I don't like this. Yea right, so sue me ;p
extern ofstream err;

void critical_error(const char*, ...);

/* for critical messages that are displayed as directly as possible */
void inline tell_user(const char* str) {
#ifdef WIN32
	MessageBox(NULL, str, "Wide Lands", MB_ICONINFORMATION);
#else
	puts(str);
#endif
}

#ifdef DEBUG
   #ifndef KEEP_STANDART_ASSERT
      #ifdef assert
         #undef assert
      #endif
      #define assert(condition) myassert(__LINE__, __FILE__, (int)(condition), #condition)
      extern int graph_is_init;

      inline void myassert(int line, const char* file, int cond, const char* condt) {
         if(!cond) {
            char buf[200];
            sprintf(buf, "%s (%i): assertion \"%s\" failed!\n", file, line, condt);

            if(graph_is_init) {
               critical_error(buf);
               // User chooses, if it goes on
            } else {
               tell_user(buf);
               exit(-1);
            }
         }
      }
   #endif
#else
      #define NDEBUG 1
      #include <assert.h>
#endif

/*
======================================================================================

      COMPILE TIME CONFIGURATION 

======================================================================================
*/

// a frame is the time between two updates of the game logic
// this includes: AI, NETWORK, CMD_QUEUES
// this excludes: the whole User_Interface, this is updated independent from the 
//   logic frame length (more often)
#define FRAME_LENGTH 250   

// TODO: make this configurable through conf file or cmdline
#define MAX_OBJS 50000  // might not be enough

#define USE_SEE_AREA 1

