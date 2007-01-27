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

#ifndef FILEREAD_H
#define FILEREAD_H

#include "machdep.h"

#include <cassert>
#include <string>

struct FileSystem;

/**
 * FileRead can be used to read a file.
 * It works quite naively by reading the entire file into memory.
 * Convenience functions are available for endian-safe access of common data types
 */
struct FileRead {

	struct FileRead_Exception {};
	struct File_Boundary_Exceeded : public FileRead_Exception {};
	struct Buffer_Overflow        : public FileRead_Exception {};

	FileRead (); /// Create the object with nothing to read.
	~FileRead(); /// Close the file if open.

	/**
	 * Loads a file into memory.
	 * Reserves one additional byte which is zeroed, so that text files can
	 * be handled like a normal C string.
	 * Throws an exception if the file couldn't be loaded for whatever reason.
	 */
	void    Open(FileSystem &, const char * const filename);

	/// Works just like Open, but returns false when the load fails.
	bool TryOpen(FileSystem &, const char * const filename);

	void Close(); /// Frees allocated memory.

		inline int GetSize() const { return length; }
		inline bool IsEOF() const { if(filepos>=length) return true; return false; }

	/**
	 * Set the file pointer to the given location.
	 * Raises File_Boundary_Exceeded when the pointer is out of bound.
	 */
		void SetFilePos(int pos);

		inline int GetFilePos(void) { return filepos; }

		inline char Signed8(int pos = -1) { return Deref8(Data(1, pos)); }
	uchar Unsigned8(const int pos = -1)
	{return static_cast<const uchar>(Deref8(Data(1, pos)));}
		inline short Signed16(int pos = -1) { return Little16(Deref16(Data(2, pos))); }
	ushort Unsigned16(const int pos = -1)
	{return static_cast<const ushort>(Little16(Deref16(Data(2, pos))));}
		inline int Signed32(int pos = -1) { return Little32(Deref32(Data(4, pos))); }
	uint Unsigned32(const int pos = -1)
	{return static_cast<const uint>(Little32(Deref32(Data(4, pos))));}
		inline float Float(int pos = -1) { return LittleFloat(DerefFloat(Data(4, pos))); }
	char * CString(int pos = -1); /// Read a zero-terminated string.

	/**
	 * This function copies characters from the file to the memory starting at
	 * buf until it encounters a newline. The newline is not copied. The
	 * characters including the newline are consumed.
	 *
	 * This function will not write to the memory at buf_end or beyond.
	 *
	 *  Requirement: buf < buf_end
	 */
	bool ReadLine(char *buf, const char * const buf_end);

	void * Data(const int bytes, const int pos = -1) {
			assert(data);

			int i = pos;
			if (pos < 0) {
				i = filepos;
				filepos += bytes;
			}
		if (i+bytes > length) throw File_Boundary_Exceeded();

			return static_cast<char * const>(data) + i;
		}

	void * data;
private:
	int    filepos;
	int    length;
};

#endif
