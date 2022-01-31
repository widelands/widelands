/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_IO_FILEWRITE_H
#define WL_IO_FILEWRITE_H

#include <limits>

#include "io/streamwrite.h"

class FileSystem;

/// Mirror of \ref FileRead : all writes are first stored in a block of memory
/// and finally written out when write() is called.
class FileWrite : public StreamWrite {
public:
	struct Pos {
		Pos(size_t const p = 0) : pos(p) {
		}

		/// Returns a special value indicating invalidity.
		static Pos null() {
			return std::numeric_limits<size_t>::max();
		}

		bool is_null() const {
			return *this == null();
		}
		operator size_t() const {
			return pos;
		}
		Pos operator++() {
			return ++pos;
		}
		Pos operator+=(const Pos& other) {
			return pos += other.pos;
		}

	private:
		size_t pos;
	};

	struct Exception {};
	struct BufferOverflow : public Exception {};

	/// Set the buffer to empty.
	FileWrite();

	/// Clear any remaining allocated data.
	~FileWrite() override;

	/// Clears the object's buffer.
	void clear();

	/// Write the file out to disk. If successful, this clears the buffers.
	/// Otherwise, an exception is thrown but the buffer remains intact (don't
	/// worry, it will be cleared by the destructor).
	void write(FileSystem& fs, const std::string& filename);

	/// Get the position that will be written to in the next write operation that
	/// does not specify a position.
	Pos get_pos() const;

	/// Set the file pointer to a new location. The position can be beyond the
	/// current end of file.
	void set_pos(const Pos& pos);

	/// Write data at the given location.
	void data(const void* src, size_t size, Pos pos);

	/// Write data at the current file pointer and advance it.
	void data(void const* src, size_t size) override;

	/// Returns the current buffer. Use this for in_memory operations.
	std::string get_data() const;

private:
	char* data_;
	size_t length_;
	size_t max_size_;
	Pos filepos_;
};

#endif  // end of include guard: WL_IO_FILEWRITE_H
