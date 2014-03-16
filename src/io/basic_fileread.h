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
#include "machdep.h"

/// Can be used to read a file. It works quite naively by reading the entire
/// file into memory. Convenience functions are available for endian-safe
/// access of common data types. Base must be derived from StreamRead.
template<typename Base> struct basic_FileRead : public Base {
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

	struct File_Boundary_Exceeded : public Base::_data_error {
		File_Boundary_Exceeded() : Base::_data_error("end of file") {}
	};
	basic_FileRead () : data(nullptr), length(0), m_fast(0) {}; /// Create the object with nothing to read.
	~basic_FileRead() {if (data) Close();} /// Close the file if open.

	/// Loads a file into memory. Reserves one additional byte which is zeroed,
	/// so that text files can be handled like a null-terminated string.
	/// \throws an exception if the file couldn't be loaded for whatever reason.
	/// \todo error handling
	void Open(FileSystem & fs, const char * const filename) {
		assert(not data);

		data = static_cast<char *>(fs.Load(filename, length));
		filepos = 0;
	}

	void fastOpen(FileSystem & fs, const char * const filename) {
		data = static_cast<char *>(fs.fastLoad(filename, length, m_fast));
		filepos = 0;
	}

	/// Works just like Open, but returns false when the load fails.
	bool TryOpen(FileSystem & fs, const char * const filename) {
		try {Open(fs, filename);} catch (const std::exception &) {return false;}
		return true;
	}

	/// Frees allocated memory.
	void Close() {
		assert(data);
		if (m_fast) {
#ifdef _WIN32
			assert(false);
#else
			munmap(data, length);
#endif
		} else {
			free(data);
		}
		data = nullptr;
	}

	size_t GetSize() const {return length;}
	bool EndOfFile() const {return length <= filepos;}

	/// Set the file pointer to the given location.
	/// \throws File_Boundary_Exceeded if the pointer is out of bound.
	void SetFilePos(Pos const pos) {
		assert(data);
		if (pos >= length)
			throw File_Boundary_Exceeded();
		filepos = pos;
	}

	/// Get the position that will be read from in the next read operation that
	/// does not specify a position.
	Pos GetPos() const {return filepos;}

	size_t Data(void * dst, size_t bufsize) {
		assert(data);
		size_t read = 0;
		for (; read < bufsize and filepos < length; ++read, ++filepos)
			static_cast<char *>(dst)[read] = data[filepos];
		return read;
	}

	char * Data(uint32_t const bytes, const Pos pos = Pos::Null()) {
		assert(data);

		Pos i = pos;
		if (pos.isNull()) {
			i = filepos;
			filepos += bytes;
		}
		if (length < i + bytes)
			throw File_Boundary_Exceeded();
		return data + i;
	}

	char * CString(Pos const pos) {
		assert(data);

		Pos i = pos.isNull() ? filepos : pos;
		if (i >= length)
			throw File_Boundary_Exceeded();
		char * const result = data + i;
		for (char * p = result; *p; ++p, ++i) {}
		++i; //  beyond the null
		if (i > (length + 1)) // allow EOF as end marker for string
			throw File_Boundary_Exceeded();
		if (pos.isNull())
			filepos = i;
		return result;
	}
	char const * CString() {return CString(Pos::Null());}

	char * ReadLine() {
		if (EndOfFile())
			return nullptr;
		char * result = data + filepos;
		for (; data[filepos] and data[filepos] != '\n'; ++filepos)
			if (data[filepos] == '\r') {
				data[filepos] = '\0';
				++filepos;
				if (data[filepos] == '\n')
					break;
				else
					throw typename Base::_data_error
						("CR not immediately followed by LF");
			}
		data[filepos] = '\0';
		++filepos;
		return result;
	}

private:
	char * data;
	size_t length;
	Pos    filepos;
	bool   m_fast;
};

#endif
