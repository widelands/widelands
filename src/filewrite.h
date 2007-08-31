/*
 * Copyright (C) 2006-2007 by the Widelands Development Team
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

#ifndef FILEWRITE_H
#define FILEWRITE_H

#include "geometry.h"
#include "machdep.h"

#include <cassert>
#include <limits>

struct FileSystem;

/**
 * Mirror of \ref FileRead : all writes are first stored in a block of memory and finally
 * written out when Write() is called.
 */
struct FileWrite {
	typedef size_t Pos;
	static Pos NoPos() throw () {return std::numeric_limits<size_t>::max();}

	struct Exception {};
	struct Buffer_Overflow : public Exception {};

	FileWrite (); /// Set the buffer to empty.
	~FileWrite(); /// Clear any remaining allocated data.

	/**
	 * Actually write the file out to disk.
	 * If successful, this clears the buffers. Otherwise, an exception
	 * is raised but the buffer remains intact (don't worry, it will be
	 * cleared by the destructor).
	 */
	void    Write(FileSystem &, const char * const filename);

	void Clear(); /// Clears the object's buffer.

	/**
	 * Get the position that will be written to in the next write operation that
	 * does not specify a position.
	 */
	Pos GetPos() const throw () {return filepos;}

	/**
	 * Set the file pointer to a new location. The position can be beyond
	 * the current end of file.
	 */
	void SetFilePos(const Pos pos) throw () {filepos = pos;}

	/**
	 * Reserve size bytes at the current file position for filling in later. It
	 * will only advance the reading position, not increase the buffer size. That
	 * is done when writing if necessary.
	 *
	 * Returns the position of the first reserved byte.
	 */
	Pos Reserve(const size_t size) throw ()
	{const Pos result = filepos; filepos += size; return result;}

	/**
	 * Write data at the given location. If pos is NoPos(), write at the
	 * file pointer and advance the file pointer.
	 */
	void Data
		(const void * const data, const size_t size, const Pos pos = NoPos());

	/**
	 * This is a perfectly normal printf (actually it isn't because it's limited
	 * to a maximum string size)
	 */
		void Printf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));

	void   Signed8 (const Sint8  x, const Pos pos = NoPos()) {Data(&x, 1, pos);}
	void Unsigned8 (const Uint8  x, const Pos pos = NoPos()) {Data(&x, 1, pos);}
	void   Signed16(const Sint16 x, const Pos pos = NoPos())
	{const Sint16 y = Little16   (x); Data(&y, 2, pos);}
	void Unsigned16(const Uint16 x, const Pos pos = NoPos())
	{const Uint16 y = Little16   (x); Data(&y, 2, pos);}
	void   Signed32(const Sint32 x, const Pos pos = NoPos())
	{const Uint32 y = Little32   (x); Data(&y, 4, pos);}
	void Unsigned32(const Uint32 x, const Pos pos = NoPos())
	{const Uint32 y = Little32   (x); Data(&y, 4, pos);}
	void      Float(const float  x, const Pos pos = NoPos())
	{const float  y = LittleFloat(x); Data(&y, 4, pos);}
	void CString(const char * const x, const Pos pos = NoPos())
	{Data(x, strlen(x) + 1, pos);}

	void  Coords32(const Coords);

private:
	void * data;
	size_t length;
	size_t maxsize;
	Pos    filepos;

	FileWrite & operator=(const FileWrite &);
	FileWrite            (const FileWrite &);
};

inline void FileWrite::Coords32(const Coords c) {
	assert(static_cast<Uint16>(c.x) < 0x8000 or c.x == -1);
	assert(static_cast<Uint16>(c.y) < 0x8000 or c.y == -1);
	{const Uint16 x = Little16(c.x); Data(&x, 2);}
	{const Uint16 y = Little16(c.y); Data(&y, 2);}
}

#endif
