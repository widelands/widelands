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

#include <exception>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

#ifdef WIN32
	#pragma warning(disable : 4250) // *sigh* multiple inheritance *sigh*
	#pragma warning(disable : 4786) // identifier longer than 255 chars (STL woes)
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

#ifdef __GNUC__
#define PRINTF_FORMAT(b,c) __attribute__ (( __format__ (__printf__,b,c) ))
#else
#define PRINTF_FORMAT(b,c)
#endif

/*
==============================================================================

DATA TYPES

==============================================================================
*/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// Structure used to store map coordinates
struct Coords {
   int x;
   int y;
};

inline bool operator==(const Coords& c1, const Coords& c2) { return (c1.x == c2.x) && (c1.y == c2.y); }


// TODO: figure out a way to define these portably
#define P_LITTLE_ENDIAN
#undef P_BIG_ENDIAN

#ifdef P_LITTLE_ENDIAN
#define Little16(x)		(x)
#define Little32(x)		(x)
#define LittleFloat(x)	(x)
#define Big16(x)			Swap16((x))
#define Big32(x)			Swap32((x))
#define BigFloat(x)		SwapFloat((x))
#endif

#ifdef P_BIG_ENDIAN
#define Little16(x)		Swap16((x))
#define Little32(x)		Swap32((x))
#define LittleFloat(x)	SwapFloat((x))
#define Big16(x)			(x)
#define Big32(x)			(x)
#define BigFloat(x)		(x)
#endif

inline short Swap16(short x) {
	short s;
	((uchar *)&s)[0] = ((uchar *)&x)[1];
	((uchar *)&s)[1] = ((uchar *)&x)[0];
	return s;
}

inline int Swap32(int x) {
	int s;
	((uchar *)&s)[0] = ((uchar *)&x)[3];
	((uchar *)&s)[1] = ((uchar *)&x)[2];
	((uchar *)&s)[2] = ((uchar *)&x)[1];
	((uchar *)&s)[3] = ((uchar *)&x)[0];
	return s;
}

inline float SwapFloat(float x)
{
	float s;
	((uchar *)&s)[0] = ((uchar *)&x)[3];
	((uchar *)&s)[1] = ((uchar *)&x)[2];
	((uchar *)&s)[2] = ((uchar *)&x)[1];
	((uchar *)&s)[3] = ((uchar *)&x)[0];
	return s;
}

/*
==============================================================================

ERROR HANDLING FUNCTIONS

==============================================================================
*/

#define RET_OK 				0
#define RET_EXIT           1
#define ERR_NOT_IMPLEMENTED -1
#define ERR_INVAL_ARG -2
#define ERR_FAILED -255


// Critical errors, displayed to the user.
// Does not return (unless the user is really daring)
void critical_error(const char *, ...) PRINTF_FORMAT(1,2);

// Informational messages that can aid in debugging
void log(const char *, ...) PRINTF_FORMAT(1,2);


/** class wexception
 *
 * Stupid, simple exception class. It has the nice bonus that you can give it
 * sprintf()-style format strings
 */
class wexception : public std::exception {
	char m_string[256];

public:
	explicit wexception(const char *fmt, ...) throw() PRINTF_FORMAT(2,3);
	virtual ~wexception() throw();
	
	virtual const char *what() const throw();
};

#ifdef DEBUG
   #ifndef KEEP_STANDART_ASSERT
      #ifdef assert
         #undef assert
      #endif
	
/* reintroduce when we figure out a way to actually manage the beast that is autotools
   (problem is: tools include this as well)
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
*/
		void myassert(int line, const char* file, const char* condt) throw(wexception);
   
      #define assert(condition) \
			do { if (!(condition)) myassert(__LINE__, __FILE__, #condition); } while(0)

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

// the actual game logic doesn't know about frames (it works with millisecond-precise
// timing)
// FRAME_LENGTH is just the default animation speed
#define FRAME_LENGTH 250   



/*
==============================================================================

FILESYSTEM ACCESS

==============================================================================
*/

typedef std::set<std::string> filenameset_t;

// basic path/filename manipulation
char *FS_AutoExtension(char *buf, int bufsize, const char *ext);
char *FS_StripExtension(char *fname);
char *FS_RelativePath(char *buf, int buflen, const char *basefile, const char *filename);
bool FS_CanonicalizeName(char *buf, int bufsize, const char *path);

/*
FileSystem is a base class representing certain filesystem operations.
*/
class FileSystem {
public:
	virtual ~FileSystem() { }

	virtual bool IsWritable() = 0;
	
	virtual int FindFiles(const char *path, const char *pattern, filenameset_t *results) = 0;
	
	virtual bool FileExists(const char *path) = 0;

	virtual void *Load(const char *fname, int *length) = 0;
	virtual void Write(const char *fname, void *data, int length) = 0;

public:
	static FileSystem *CreateFromDirectory(const char *directory);
};

/*
LayeredFileSystem is a file system which basically merges several layered
real directory structures into a single one. The really funny thing is that 
those directories aren't represented as absolute paths, but as nested 
FileSystems. Are you confused yet?
Ultimately, this provides us with the necessary flexibility to allow file
overrides on a per-user-basis, nested .zip files acting as Quake-like paks
and so on.

Note that only the top-most writable filesystem is written to. A typical 
stack would look like this in real-life:

~/.widelands/
/where/they/installed/widelands/  <-- this is the directory that the executable is in
$CWD  <-- the current-working directory; this is useful for debugging, when the executable
          isn't in the root of the game-data directory
*/
class LayeredFileSystem : public FileSystem {
public:
	virtual void AddFileSystem(FileSystem *fs) = 0;

public:
	static LayeredFileSystem *Create();
};


/*
FileRead can be used to read a file.
It works quite naively by reading the entire file into memory.
Convenience functions are available for endian-safe access of common data types
*/
class FileRead {
public:
	void	*data;
	int	filepos;
	int	length;

public:
	FileRead();
	~FileRead();

	void Open(FileSystem *fs, const char *fname);
	bool TryOpen(FileSystem *fs, const char *fname);
	void Close();

	inline int GetSize() const { return length; }
	void SetFilePos(int pos);

	inline char Signed8(int pos = -1) { return *(char *)Data(1, pos); }
	inline uchar Unsigned8(int pos = -1) { return *(uchar *)Data(1, pos); }
	inline short Signed16(int pos = -1) { return Little16(*(short *)Data(2, pos)); }
	inline ushort Unsigned16(int pos = -1) { return (ushort)Little16(*(short *)Data(2, pos)); }
	inline int Signed32(int pos = -1) { return Little32(*(int *)Data(4, pos)); }
	inline uint Unsigned32(int pos = -1) { return (uint)Little32(*(int *)Data(4, pos)); }
	inline float Float(int pos = -1) { return LittleFloat(*(float *)Data(4, pos)); }
	char *CString(int pos = -1);
	bool ReadLine(char *buf, int buflen);
	
	void *Data(int bytes, int pos = -1) {
		int i;

		assert(data);

		i = pos;
		if (pos < 0) {
			i = filepos;
			filepos += bytes;
		}
		if (i+bytes > length)
			throw wexception("File boundary exceeded");

		return (char*)data + i;
	}
};

/*
Mirror of FileRead: all writes are first stored in a block of memory and finally
written out when Write() is called.
*/
class FileWrite {
public:
	void	*data;
	int	length;
	int	maxsize;
	int	filepos;

public:
	FileWrite();
	~FileWrite();

	void Write(FileSystem *fs, const char *filename);
	bool TryWrite(FileSystem *fs, const char *filename);
	void Clear();

	void SetFilePos(int pos);
	void Data(const void *data, int size, int pos = -1);

	void Printf(const char *fmt, ...);

	inline void Signed8(char x, int pos = -1) { Data(&x, 1, pos); }
	inline void Unsigned8(uchar x, int pos = -1) { Data(&x, 1, pos); }
	inline void Signed16(short x, int pos = -1) { short y = Little16(x); Data(&y, 2, pos); }
	inline void Unsigned16(ushort x, int pos = -1) { short y = Little16((short)x); Data(&y, 2, pos); }
	inline void Signed32(int x, int pos = -1) { int y = Little32(x); Data(&y, 4, pos); }
	inline void Unsigned32(uint x, int pos = -1) { int y = Little32((int)x); Data(&y, 4, pos); }
	inline void Float(float x, int pos = -1) { float y = LittleFloat(x); Data(&y, 4, pos); }
	inline void CString(const char *x, int pos = -1) { Data(x, strlen(x)+1, pos); }
};

// Access all game data files etc.. through this FileSystem
extern LayeredFileSystem *g_fs;


/*
==============================================================================

SYSTEM ABSTRACTION

==============================================================================
*/

#include "keycodes.h"

enum { // use 1<<MOUSE_xxx for bitmasks
	MOUSE_LEFT = 0,
	MOUSE_MIDDLE,
	MOUSE_RIGHT
};

/** class System

All interactions with the OS except for file access and graphics are handled 
by the System class. Most notably:
 - timing
 - input
 - low-level networking

System will be the one place for complete session playback technology.
Using a command line option, all input etc.. that passes through System can be
saved in a file and played back later for intensive and slow profiling and 
testing (valgrind comes to mind).
(This is completely independent from recorded games; recorded games consist of
saved player commands and can be recorded and played back from the GUI)

Note/TODO: Graphics are currently not handled by System, and it is non-essential
for playback anyway. Additionally, we'll want several rendering backends 
(software and OpenGL).
Maybe the graphics backend loader code should be in System, while the actual
graphics work is done elsewhere.

Mouse: Some mouse functions deal with button mask bits. Bits are simply obtained
as (1 << btnnr), so bitmask 5 = (1<<0)|(1<<2) means: "left and right are pressed"
*/
class System {
public:
	System();
	~System();
	
	bool should_die();
	
	// Timing
	int get_time();
	
	// Input
	struct InputCallback {
		void (*mouse_click)(bool down, int btn, uint btns, int x, int y);
		void (*mouse_move)(uint btns, int x, int y, int xdiff, int ydiff);
		void (*key)(bool down, int code, char c);
	};
	
	void handle_input(InputCallback *cb);
			
	uint get_mouse_buttons();
	int get_mouse_x();
	int get_mouse_y();
	void set_mouse_pos(int x, int y);
	
	void set_mouse_swap(bool swap);
	void set_mouse_speed(float speed);
	
	// pseudo-private kludge	
	void set_max_mouse_coords(int x, int y);

private:
	bool		m_active;
	bool		m_should_die;
	
	// Input
	bool		m_mouse_swapped;
	float		m_mouse_speed;
	uint		m_mouse_buttons;
	float		m_mouse_x, m_mouse_y;
	int		m_mouse_maxx, m_mouse_maxy;
};

extern System *g_system;

#define g_sys	(*g_system)
