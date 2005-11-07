/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cassert>
#include <set>
#include <string>
#include "machdep.h"
#include "types.h"
#include "wexception.h"

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
const char *FS_Filename(const char* buf);

/*
FileSystem is a base class representing certain filesystem operations.
*/
class FileSystem {
public:
   enum Type {
      FS_DIR,
      FS_ZIP
   };

	virtual ~FileSystem() { }

	virtual bool IsWritable() = 0;

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results) = 0;

   virtual bool IsDirectory(std::string path) = 0;
	virtual bool FileExists(std::string path) = 0;

	virtual void *Load(std::string fname, int *length) = 0;
	virtual void Write(std::string fname, void *data, int length) = 0;
   virtual void EnsureDirectoryExists(std::string dirname) = 0;
   virtual void MakeDirectory(std::string dirname) = 0;
   
   virtual FileSystem*  MakeSubFileSystem( std::string dirname ) = 0;
   virtual FileSystem*  CreateSubFileSystem( std::string dirname, Type ) = 0;
   virtual void Unlink( std::string ) = 0;

public:
	static FileSystem *CreateFromDirectory(std::string directory);
	static FileSystem *CreateFromZip(std::string file);
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
   
	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results) = 0;  // From FileSystem
	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results, int depth) = 0;

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

	void Open(FileSystem *fs, std::string fname);
	bool TryOpen(FileSystem *fs, std::string fname);
	void Close();

	inline int GetSize() const { return length; }
	inline bool IsEOF() const { if(filepos>=length) return true; return false; }
   void SetFilePos(int pos);
   inline int GetFilePos(void) { return filepos; }
   
	inline char Signed8(int pos = -1) { return Deref8(Data(1, pos)); }
	inline uchar Unsigned8(int pos = -1) { return (uchar)Deref8(Data(1, pos)); }
	inline short Signed16(int pos = -1) { return Little16(Deref16(Data(2, pos))); }
	inline ushort Unsigned16(int pos = -1) { return (ushort)Little16(Deref16(Data(2, pos))); }
	inline int Signed32(int pos = -1) { return Little32(Deref32(Data(4, pos))); }
	inline uint Unsigned32(int pos = -1) { return (uint)Little32(Deref32(Data(4, pos))); }
	inline float Float(int pos = -1) { return LittleFloat(DerefFloat(Data(4, pos))); }
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
   int   counter;
   
public:
	FileWrite();
	~FileWrite();

	void Write(FileSystem *fs, std::string filename);
	bool TryWrite(FileSystem *fs, std::string filename);
	void Clear();

   void ResetByteCounter(void);
   int  GetByteCounter(void);
   int  GetFilePos(void); 
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

#endif
