/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_IO_FILEREAD_H
#define WL_IO_FILEREAD_H

#include <limits>

#ifndef _WIN32
#include <sys/mman.h>
#endif

#include "io/filesystem/filesystem.h"
#include "io/streamread.h"

/// Can be used to read a file. It works quite naively by reading the entire
/// file into memory. Convenience functions are available for endian-safe
/// access of common data types.
class FileRead : public StreamRead {
public:
	struct Pos {
		Pos(size_t const p = 0) : pos(p) {  // NOLINT allow implicit conversion
		}
		/// Returns a special value indicating invalidity.
		static Pos null() {
			return Pos(std::numeric_limits<size_t>::max());
		}

		[[nodiscard]] bool operator==(const Pos& p) const {
			return pos == p.pos;
		}
		[[nodiscard]] bool is_null() const {
			return *this == null();
		}
		operator size_t() const {  // NOLINT allow implicit conversion
			return pos;
		}
		Pos& operator++() {
			++pos;
			return *this;
		}

		Pos& operator+=(Pos const other) {
			pos += other.pos;
			return *this;
		}

	private:
		size_t pos;
	};

	struct FileBoundaryExceeded : public StreamRead::DataError {
		FileBoundaryExceeded() : StreamRead::DataError("end of file") {
		}
	};

	/// Create the object with nothing to read.
	FileRead();

	~FileRead() override;

	// See base class.
	size_t data(void* dst, size_t bufsize) override;
	[[nodiscard]] bool end_of_file() const override;
	char const* c_string() override;

	/// Loads a file into memory. Reserves one additional byte which is zeroed,
	/// so that text files can be handled like a null-terminated string.
	/// \throws an exception if the file couldn't be loaded for whatever reason.

	// TODO(unknown): error handling
	void open(FileSystem& fs, const std::string& filename);

	/// Works just like open, but returns false when the load fails.
	// TODO(sirver): This method can be expressed through open() and should not
	// be part of the public API, rather a stand alone function.
	bool try_open(FileSystem& fs, const std::string& filename);

	/// Frees allocated memory.
	void close();

	// Returns the size of the file in bytes;
	[[nodiscard]] size_t get_size() const;

	/// Set the file pointer to the given location.
	/// \throws File_Boundary_Exceeded if the pointer is out of bound.
	void set_file_pos(const Pos& pos);

	/// Get the position that will be read from in the next read operation that
	/// does not specify a position.
	[[nodiscard]] Pos get_pos() const;

	// Returns the next 'bytes' starting at 'pos' in the file. Can throw
	// File_Boundary_Exceeded.
	char* data(uint32_t bytes, const Pos& pos = Pos::null());

	// Returns the whole file as a string starting from 'pos'.
	char* c_string(const Pos& pos);

	// Returns the next line.
	char* read_line();

private:
	char* data_{nullptr};
	size_t length_{0};
	Pos filepos_;
};

#endif  // end of include guard: WL_IO_FILEREAD_H
