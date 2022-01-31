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

#ifndef WL_IO_FILESYSTEM_DISK_FILESYSTEM_H
#define WL_IO_FILESYSTEM_DISK_FILESYSTEM_H

#include "io/filesystem/filesystem.h"

// TODO(unknown): const correctness
class RealFSImpl : public FileSystem {
public:
	explicit RealFSImpl(const std::string& Directory);

	FilenameSet list_directory(const std::string& path) const override;

	bool is_writable() const override;
	bool file_exists(const std::string& path) const override;
	bool is_directory(const std::string& path) const override;
	void ensure_directory_exists(const std::string& fs_dirname) override;
	void make_directory(const std::string& fs_dirname) override;

	void* load(const std::string& fname, size_t& length) override;

	void write(const std::string& fname, void const* data, size_t length, bool append);
	void write(const std::string& fname, void const* data, size_t length) override {
		write(fname, data, length, false);
	}

	StreamRead* open_stream_read(const std::string& fname) override;
	StreamWrite* open_stream_write(const std::string& fname) override;

	FileSystem* make_sub_file_system(const std::string& fs_dirname) override;
	FileSystem* create_sub_file_system(const std::string& fs_dirname, Type) override;
	void fs_unlink(const std::string& file) override;
	void fs_rename(const std::string& old_name, const std::string& new_name) override;

	std::string get_basename() override {
		return directory_;
	}
	unsigned long long disk_space() override;  // NOLINT

private:
	void unlink_directory(const std::string& file);
	void unlink_file(const std::string& file);

	std::string directory_;
};

#endif  // end of include guard: WL_IO_FILESYSTEM_DISK_FILESYSTEM_H
