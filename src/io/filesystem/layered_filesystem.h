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

#ifndef WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H
#define WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H

#include <memory>

#include "io/filesystem/filesystem.h"

/**
 * LayeredFileSystem is a file system which basically merges several layered
 * real directory structures into a single one. The really funny thing is that
 * those directories aren't represented as absolute paths, but as nested
 * FileSystems. Are you confused yet?
 * Ultimately, this provides us with the necessary flexibility to allow file
 * overrides on a per-user-basis, nested .zip files acting as Quake-like paks
 * and so on.
 *
 * Only the top-most writable filesystem is written to. A typical
 * stack would look like this in real-life:
 *
 * ~/.widelands/
 * /where/they/installed/widelands/  <-- this is the directory that the
 * executable is in
 *
 * $CWD  <-- the current-working directory; this is useful for debugging, when
 * the executable isn't in the root of the game-data directory
 */
class LayeredFileSystem : public FileSystem {
public:
	LayeredFileSystem();
	~LayeredFileSystem() override = default;

	// Add a new filesystem to the top of the stack. Take ownership of the given
	// filesystem.
	void add_file_system(FileSystem*);

	// Set the home filesystem (which is the preferred filesystem for writing
	// files). Take ownership of the given filesystem.
	void set_home_file_system(FileSystem*);

	FilenameSet list_directory(const std::string& path) const override;

	bool is_writable() const override;
	bool file_exists(const std::string& path) const override;
	bool is_directory(const std::string& path) const override;
	void ensure_directory_exists(const std::string& fs_dirname) override;
	void make_directory(const std::string& fs_dirname) override;

	void* load(const std::string& fname, size_t& length) override;
	void write(const std::string& fname, void const* data, size_t length) override;

	StreamRead* open_stream_read(const std::string& fname) override;
	StreamWrite* open_stream_write(const std::string& fname) override;

	FileSystem* make_sub_file_system(const std::string& dirname) override;
	FileSystem* create_sub_file_system(const std::string& dirname, Type type) override;
	void fs_unlink(const std::string& file) override;
	void fs_rename(const std::string& old_name, const std::string& new_name) override;

	std::string get_basename() override {
		return std::string();
	}

	unsigned long long disk_space() override;  // NOLINT

private:
	/// This is used to assemble an error message for exceptions that includes all file paths
	std::string paths_error_message(const std::string& filename) const;

	std::vector<std::unique_ptr<FileSystem>> filesystems_;
	std::unique_ptr<FileSystem> home_;
};

/// Access all game data files etc. through this FileSystem
extern LayeredFileSystem* g_fs;

#endif  // end of include guard: WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H
