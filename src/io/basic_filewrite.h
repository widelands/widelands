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

#ifndef BASIC_FILEWRITE_H
#define BASIC_FILEWRITE_H

#include <cassert>
#include <cstdarg>
#include <limits>

#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem.h"
#include "machdep.h"

/// Mirror of \ref FileRead : all writes are first stored in a block of memory
/// and finally written out when Write() is called.
template <typename Base> struct basic_FileWrite : public Base {
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

	struct Exception {};
	struct Buffer_Overflow : public Exception {};

	/// Set the buffer to empty.
	basic_FileWrite (): data(nullptr), filelength(0), maxsize(0), filepos(0) {}

	/// Clear any remaining allocated data.
	~basic_FileWrite() {Clear();}

	/// Clears the object's buffer.
	void Clear() {free(data); data = nullptr; filelength = maxsize = 0; filepos = 0;}

	/// Write the file out to disk. If successful, this clears the buffers.
	/// Otherwise, an exception is thrown but the buffer remains intact (don't
	/// worry, it will be cleared by the destructor).
	void Write(FileSystem & fs, char const * const filename) {
		fs.Write(filename, data, filelength); Clear();
	}

	/// Same as above, just that the data is appended to the file
	/// NOTE RealFSImpl is used by purpose - zip filesystems do not support appending
	void WriteAppend(RealFSImpl & fs, char const * const filename) {
		fs.Write(filename, data, filelength, true); Clear();
	}

	/// Get the position that will be written to in the next write operation that
	/// does not specify a position.
	Pos GetPos() const {return filepos;}

	/// Set the file pointer to a new location. The position can be beyond the
	/// current end of file.
	void SetPos(const Pos pos) {filepos = pos;}

	/**
	 * Write data at the given location. If pos is NoPos(), write at the
	 * file pointer and advance the file pointer.
	 */
	void Data
		(const void * const src, const size_t size, Pos const pos = Pos::Null())
	{
		assert(data or not filelength);

		Pos i = pos;
		if (pos.isNull()) {i = filepos; filepos += size;}
		if (i + size > filelength) {
			if (i + size > maxsize) {
				maxsize += 4096;
				if (maxsize < i + size)
					maxsize = i + size;
				char* new_data = static_cast<char *>(realloc(data, maxsize));
				assert(new_data);
				data = new_data;
			}
			filelength = i + size;
		}
		memcpy(data + i, src, size);
	}

	void Data(void const * const src, size_t const size) {
		Data(src, size, Pos::Null());
	}

private:
	char * data;
	size_t filelength;
	size_t maxsize;
	Pos    filepos;
};

#endif
