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

#include "io/fileread.h"

#include <cassert>

FileRead::FileRead() : data_(nullptr), length_(0) {
}

FileRead::~FileRead() {
	if (data_) {
		close();
	}
}

void FileRead::open(FileSystem& fs, const std::string& filename) {
	assert(!data_);
	data_ = static_cast<char*>(fs.load(filename, length_));
	filepos_ = 0;
}

bool FileRead::try_open(FileSystem& fs, const std::string& filename) {
	try {
		open(fs, filename);
	} catch (const std::exception&) {
		return false;
	}
	return true;
}

void FileRead::close() {
	assert(data_);
	free(data_);
	data_ = nullptr;
}

size_t FileRead::get_size() const {
	return length_;
}

bool FileRead::end_of_file() const {
	return length_ <= filepos_;
}

void FileRead::set_file_pos(const Pos& pos) {
	assert(data_);
	if (pos >= length_) {
		throw FileBoundaryExceeded();
	}
	filepos_ = pos;
}

FileRead::Pos FileRead::get_pos() const {
	return filepos_;
}

size_t FileRead::data(void* dst, size_t bufsize) {
	assert(data_);
	size_t read = 0;
	for (; read < bufsize && filepos_ < length_; ++read, ++filepos_) {
		static_cast<char*>(dst)[read] = data_[filepos_];
	}
	return read;
}

char* FileRead::data(uint32_t const bytes, const Pos& pos) {
	assert(data_);
	Pos i = pos;
	if (pos.is_null()) {
		i = filepos_;
		filepos_ += bytes;
	}
	if (length_ < i + bytes) {
		throw FileBoundaryExceeded();
	}
	return data_ + i;
}

char* FileRead::c_string(const Pos& pos) {
	assert(data_);

	Pos i = pos.is_null() ? filepos_ : pos;
	if (i >= length_) {
		throw FileBoundaryExceeded();
	}
	char* const result = data_ + i;
	for (char* p = result; *p; ++p, ++i) {
	}
	++i;                      //  beyond the null
	if (i > (length_ + 1)) {  // allow EOF as end marker for string
		throw FileBoundaryExceeded();
	}
	if (pos.is_null()) {
		filepos_ = i;
	}
	return result;
}

char const* FileRead::c_string() {
	return c_string(Pos::null());
}

char* FileRead::read_line() {
	if (end_of_file()) {
		return nullptr;
	}
	char* result = data_ + filepos_;
	for (; data_[filepos_] && data_[filepos_] != '\n'; ++filepos_) {
		if (data_[filepos_] == '\r') {
			data_[filepos_] = '\0';
			++filepos_;
			if (data_[filepos_] == '\n') {
				break;
			} else {
				throw typename StreamRead::DataError("CR not immediately followed by LF");
			}
		}
	}
	data_[filepos_] = '\0';
	++filepos_;
	return result;
}
