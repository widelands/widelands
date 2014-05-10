/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "io/fileread.h"

FileRead::FileRead() : data_(nullptr), length_(0), fast_(0) {};

FileRead::~FileRead() {
	if (data_) {
		Close();
	}
}

void FileRead::Open(FileSystem& fs, const char* const filename) {
	assert(!data_);

	data_ = static_cast<char*>(fs.Load(filename, length_));
	filepos_ = 0;
}

void FileRead::fastOpen(FileSystem& fs, const char* const filename) {
	data_ = static_cast<char*>(fs.fastLoad(filename, length_, fast_));
	filepos = 0;
}

bool FileRead::TryOpen(FileSystem& fs, const char* const filename) {
	try {
		Open(fs, filename);
	}
	catch (const std::exception&) {
		return false;
	}
	return true;
}

void FileRead::Close() {
	assert(data_);
	if (fast_) {
#ifdef _WIN32
		assert(false);
#else
		munmap(data_, length_);
#endif
	} else {
		free(data_);
	}
	data_ = nullptr;
}

size_t FileRead::GetSize() const {
	return length_;
}

bool FileRead::EndOfFile() const {
	return length_ <= filepos_;
}

void FileRead::SetFilePos(Pos const pos) {
	assert(data_);
	if (pos >= length_)
		throw File_Boundary_Exceeded();
	filepos_ = pos;
}

Pos FileRead::GetPos() const {
	return filepos_;
}

size_t FileRead::Data(void* dst, size_t bufsize) {
	assert(data_);
	size_t read = 0;
	for (; read < bufsize and filepos_ < length_; ++read, ++filepos_) {
		static_cast<char*>(dst)[read] = data_[filepos_];
	}
	return read;
}

char* FileRead::Data(uint32_t const bytes, const Pos pos = Pos::Null()) {
	assert(data_);

	Pos i = pos;
	if (pos.isNull()) {
		i = filepos_;
		filepos_ += bytes;
	}
	if (length_ < i + bytes)
		throw File_Boundary_Exceeded();
	return data_ + i;
}

char* FileRead::CString(Pos const pos) {
	assert(data_);

	Pos i = pos.isNull() ? filepos_ : pos;
	if (i >= length_)
		throw File_Boundary_Exceeded();
	char* const result = data_ + i;
	for (char* p = result; *p; ++p, ++i) {
	}
	++i;                   //  beyond the null
	if (i > (length_ + 1))  // allow EOF as end marker for string
		throw File_Boundary_Exceeded();
	if (pos.isNull())
		filepos_ = i;
	return result;
}

char const* FileRead::CString() {
	return CString(Pos::Null());
}

char* FileRead::ReadLine() {
	if (EndOfFile())
		return nullptr;
	char* result = data_ + filepos_;
	for (; data_[filepos_] and data_[filepos_] != '\n'; ++filepos_)
		if (data_[filepos_] == '\r') {
			data_[filepos_] = '\0';
			++filepos_;
			if (data_[filepos_] == '\n')
				break;
			else
				throw typename StreamRead::_data_error("CR not immediately followed by LF");
		}
	data_[filepos_] = '\0';
	++filepos_;
	return result;
}
