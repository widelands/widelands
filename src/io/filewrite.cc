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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "io/filewrite.h"

#include <cassert>

#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem.h"

FileWrite::FileWrite() : data_(nullptr), length_(0), max_size_(0), filepos_(0) {
}

FileWrite::~FileWrite() {
	clear();
}

void FileWrite::clear() {
	free(data_);
	data_ = nullptr;
	length_ = max_size_ = 0;
	filepos_ = 0;
}

void FileWrite::write(FileSystem& fs, const std::string& filename) {
	fs.write(filename, data_, length_);
	clear();
}

FileWrite::Pos FileWrite::get_pos() const {
	return filepos_;
}

void FileWrite::set_pos(const Pos& pos) {
	filepos_ = pos;
}

void FileWrite::data(const void* const src, const size_t size, Pos const pos = Pos::null()) {
	assert(data_ || !length_);

	Pos i = pos;
	if (pos.is_null()) {
		i = filepos_;
		filepos_ += size;
	}
	if (i + size > length_) {
		if (i + size > max_size_) {
			max_size_ += 4096;
			if (max_size_ < i + size) {
				max_size_ = i + size;
			}
			char* new_data = static_cast<char*>(realloc(data_, max_size_));
			assert(new_data);
			data_ = new_data;
		}
		length_ = i + size;
	}
	memcpy(data_ + i, src, size);
}

void FileWrite::data(void const* const src, size_t const size) {
	data(src, size, Pos::null());
}

std::string FileWrite::get_data() const {
	return std::string(data_, length_);
}
