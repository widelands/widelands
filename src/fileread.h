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

#include "geometry.h"
#include "machdep.h"
#include <exception>
#include <limits>
#include <cassert>
#include <stdint.h>
#include <string>

struct FileSystem;

/**
 * FileRead can be used to read a file.
 * It works quite naively by reading the entire file into memory.
 * Convenience functions are available for endian-safe access of common data types
 */
struct FileRead {
	typedef size_t Pos;
	static Pos NoPos() throw () {return std::numeric_limits<size_t>::max();}

	struct FileRead_Exception : public std::exception {};
	struct File_Boundary_Exceeded : public FileRead_Exception {
		virtual const char * what() const throw()
		{return "File boundary exceeded";}
	};
	struct Buffer_Overflow : public FileRead_Exception {
		virtual const char * what() const throw() {return "Buffer overflow";}
	};

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

	size_t GetSize() const throw () {return length;}
	bool IsEOF() const throw () {return length <= filepos;}

	/**
	 * Set the file pointer to the given location.
	 * Raises File_Boundary_Exceeded when the pointer is out of bound.
	 */
	void SetFilePos(const Pos pos);

	/**
	 * Get the position that will be read from in the next read operation that
	 * does not specify a position.
	 */
	Pos GetPos() const throw () {return filepos;}

	/**
	 * Get the position that was read from in the previous read operation that
	 * did not specify a position.
	 */
	Pos GetPrevPos() const throw () {return prevpos;}

	Sint8    Signed8 (const Pos pos = NoPos())
	{return                                       Deref8    (Data(1, pos));}
	Uint8  Unsigned8 (const Pos pos = NoPos())
	{return static_cast<Uint8>             (Deref8    (Data(1, pos)));}
	Sint16   Signed16(const Pos pos = NoPos())
	{return                           Little16   (Deref16   (Data(2, pos)));}
	Uint16 Unsigned16(const Pos pos = NoPos())
	{return static_cast<Uint16>(Little16   (Deref16   (Data(2, pos))));}
	Sint32   Signed32(const Pos pos = NoPos())
	{return                           Little32   (Deref32   (Data(4, pos)));}
	Uint32 Unsigned32(const Pos pos = NoPos())
	{return static_cast<Uint32>(Little32   (Deref32   (Data(4, pos))));}
	float       Float(const Pos pos = NoPos())
	{return                           LittleFloat(DerefFloat(Data(4, pos)));}
	char * CString(const Pos pos = NoPos()); /// Read a zero-terminated string.

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

	void * Data(const uint32_t bytes, const Pos pos = NoPos()) {
			assert(data);

			Pos i = pos;
			if (pos == NoPos()) {
				prevpos = i = filepos;
				filepos += bytes;
			}
		if (i+bytes > length) throw File_Boundary_Exceeded();

			return static_cast<char *>(data) + i;
		}

	void * data;

	struct Data_Error {
		Data_Error(const Pos pos) : position(pos) {}
		Pos position;
	};
	struct Extent_Exceeded : public Data_Error
	{Extent_Exceeded(const Pos pos) : Data_Error(pos) {}};
	struct Width_Exceeded : public Extent_Exceeded {
		Width_Exceeded(const Pos pos, const Uint16 W, const X_Coordinate X) :
			Extent_Exceeded(pos), w(W), x(X)
		{}
		Uint16       w;
		X_Coordinate x;
	};
	struct Height_Exceeded : public Extent_Exceeded {
		Height_Exceeded(const Pos pos, const Uint16 H, const Y_Coordinate Y) :
			Extent_Exceeded(pos), h(H), y(Y)
		{}
		Uint16       h;
		Y_Coordinate y;
	};

	/**
	 * Read a Coords from the file. Use this when the result can only be a
	 * coordinate pair referring to a node.
	 *
	 * \throws Width_Exceeded  if extent.w is <= the x coordinate.
	 * \throws Height_Exceeded if extent.h is <= the y coordinate.
	 * Both coordinates are read from the file before checking and possibly
	 * throwing, so in case such an exception is thrown, it is guaranteed that
	 * the whole coordinate pair has been read. The file position in the
	 * exception object is that of the whole Coords object, not that of a
	 * particular out-of-range coordinate.
	 *
	 * After this call, GetPrevPos will return the file position of the whole
	 * Coords object, not the position of the last component of it that was read.
	 */
	Coords Coords32(const Extent extent);

	/**
	 * Like Coords32 but the result can have the special value indicating
	 * invalidity, as defined by Coords::Null.
	 */
	Coords Coords32_allow_null(const Extent extent);

private:
	Pos    filepos;
	Pos    prevpos;
	size_t length;

	FileRead & operator=(const FileRead &);
	FileRead            (const FileRead &);
};

inline Coords FileRead::Coords32(const Extent extent) {
	const Uint16 x = Unsigned16();
	const Uint16 y = Unsigned16();
	prevpos = filepos - 4;
	if (extent.w <= x) throw Width_Exceeded (prevpos, extent.w, x);
	if (extent.h <= y) throw Height_Exceeded(prevpos, extent.h, y);
	return Coords(x, y);
}

inline Coords FileRead::Coords32_allow_null(const Extent extent) {
	const Uint16 x = Unsigned16();
	const Uint16 y = Unsigned16();
	const Coords result(x, y);
	prevpos = filepos - 4;
	if (result) {
		if (extent.w <= x) throw Width_Exceeded (prevpos, extent.w, x);
		if (extent.h <= y) throw Height_Exceeded(prevpos, extent.h, y);
	}
	return result;
}

#endif
