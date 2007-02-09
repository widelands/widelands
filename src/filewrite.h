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

#include "machdep.h"

struct FileSystem;

/**
 * Mirror of \ref FileRead : all writes are first stored in a block of memory and finally
 * written out when Write() is called.
 */
struct FileWrite {

	struct FileRead_Exception {};
	struct Buffer_Overflow        : public FileRead_Exception {};

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
	 * Set the file pointer to a new location. The position can be beyond the
	 * current end of file.
	 */
		int  GetFilePos(void);

	/**
	 * Set the file pointer to a new location. The position can be beyond
	 * the current end of file.
	 */
		void SetFilePos(int pos);

	/**
	 * Write data at the given location. If pos is -1, write at the
	 * file pointer and advance the file pointer.
	 */
		void Data(const void *data, int size, int pos = -1);

	/**
	 * This is a perfectly normal printf (actually it isn't because it's limited
	 * to a maximum string size)
	 */
		void Printf(const char *fmt, ...) __attribute__((format(printf,2,3)));

		inline void Signed8(char x, int pos = -1) { Data(&x, 1, pos); }
		inline void Unsigned8(uchar x, int pos = -1) { Data(&x, 1, pos); }
		inline void Signed16(short x, int pos = -1) { short y = Little16(x); Data(&y, 2, pos); }
	void Unsigned16(ushort x, int pos = -1)
	{const short y = Little16(static_cast<const short>(x)); Data(&y, 2, pos);}
		inline void Signed32(int x, int pos = -1) { int y = Little32(x); Data(&y, 4, pos); }
	void Unsigned32(const uint x, const int pos = -1)
	{const int y = Little32(static_cast<const int>(x)); Data(&y, 4, pos);}
		inline void Float(float x, int pos = -1) { float y = LittleFloat(x); Data(&y, 4, pos); }
		inline void CString(const char *x, int pos = -1) { Data(x, strlen(x)+1, pos); }

private:
	void * data;
	int    length;
	int    maxsize;
	int    filepos;
};

#endif
