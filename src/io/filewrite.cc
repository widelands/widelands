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

#include "io/filewrite.h"

#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem.h"

FileWrite::FileWrite() : data_(nullptr), length_(0), max_size_(0), filepos_(0) {
}

FileWrite::~FileWrite() {
	Clear();
}

void FileWrite::Clear() {
	free(data_);
	data_ = nullptr;
	length_ = max_size_ = 0;
	filepos_ = 0;
}

void FileWrite::Write(FileSystem& fs, char const* const filename) {
	fs.Write(filename, data_, length_);
	Clear();
}

void FileWrite::WriteAppend(RealFSImpl& fs, char const* const filename) {
	fs.Write(filename, data_, length_, true);
	Clear();
}

FileWrite::Pos FileWrite::GetPos() const {
	return filepos_;
}

void FileWrite::SetPos(const Pos pos) {
	filepos_ = pos;
}

void FileWrite::Data(const void* const src, const size_t size, Pos const pos = Pos::Null()) {
	assert(data_ or not length_);

	Pos i = pos;
	if (pos.isNull()) {
		i = filepos_;
		filepos_ += size;
	}
	if (i + size > length_) {
		if (i + size > max_size_) {
			max_size_ += 4096;
			if (max_size_ < i + size)
				max_size_ = i + size;
			char* new_data = static_cast<char*>(realloc(data_, max_size_));
			assert(new_data);
			data_ = new_data;
		}
		length_ = i + size;
	}
	memcpy(data_ + i, src, size);
}

void FileWrite::Data(void const* const src, size_t const size) {
	Data(src, size, Pos::Null());
}

std::string FileWrite::GetData() const {
	return std::string(data_, length_);
}
