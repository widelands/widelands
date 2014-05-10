/*
 * Copyright (C) 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef BASIC_FILEREAD_H
#define BASIC_FILEREAD_H

#include <cassert>
#include <limits>

#ifndef _WIN32
#include <sys/mman.h>
#endif

#include "io/filesystem/filesystem.h"
#include "io/streamread.h"
#include "machdep.h"

// NOCOM(#sirver): kill mmap methods?
// NOCOM(#sirver): kille use of bare char* pointers?

/// Can be used to read a file. It works quite naively by reading the entire
/// file into memory. Convenience functions are available for endian-safe
/// access of common data types.
class FileRead : public StreamRead {
public:
	struct Pos {
		Pos(size_t const p = 0) : pos(p) {}
		/// Returns a special value indicating invalidity.
		static Pos Null() {return std::numeric_limits<size_t>::max();}

		bool isNull() const {return *this == Null();}
		operator size_t() const {return pos;}
		Pos operator++ () {return ++pos;}
		Pos operator+= (Pos const other) {return pos += other.pos;}
	private:
		size_t pos;
	};

	struct File_Boundary_Exceeded : public StreamRead::_data_error {
		File_Boundary_Exceeded() : StreamRead::_data_error("end of file") {}
	};

	/// Create the object with nothing to read.
	FileRead();

	~FileRead() override;

	// See base class.
	size_t Data(void* dst, size_t bufsize) override;
	bool EndOfFile() const override;
	char const * CString() override;

	/// Loads a file into memory. Reserves one additional byte which is zeroed,
	/// so that text files can be handled like a null-terminated string.
	/// \throws an exception if the file couldn't be loaded for whatever reason.
	/// \todo error handling
	void Open(FileSystem & fs, const char * const filename);

	// As open, but tries to use mmap.
	void fastOpen(FileSystem & fs, const char * const filename);

	/// Works just like Open, but returns false when the load fails.
	bool TryOpen(FileSystem & fs, const char * const filename);

	/// Frees allocated memory.
	void Close();

	// Returns the size of the file in bytes;
	size_t GetSize() const;

	/// Set the file pointer to the given location.
	/// \throws File_Boundary_Exceeded if the pointer is out of bound.
	void SetFilePos(Pos const pos);

	/// Get the position that will be read from in the next read operation that
	/// does not specify a position.
	Pos GetPos() const;

	// Returns the next 'bytes' starting at 'pos' in the file. Can throw
	// File_Boundary_Exceeded.
	char * Data(uint32_t const bytes, const Pos pos = Pos::Null()) ;

	// Returns the whole file as a string starting from 'pos'.
	char* CString(Pos const pos);

	// Returns the next line.
	char * ReadLine();

private:
	char * data_;
	size_t length_;
	Pos    filepos_;
	bool   fast_;
};

#endif
