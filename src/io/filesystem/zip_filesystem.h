/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H
#define WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H

#include <memory>

#include <unzip.h>
#include <zip.h>

#include "io/filesystem/filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

class ZipFilesystem : public FileSystem {
public:
	explicit ZipFilesystem(const std::string&);
	~ZipFilesystem() override = default;

	bool is_writable() const override;

	FilenameSet list_directory(const std::string& path) const override;

	bool is_directory(const std::string& path) const override;
	bool file_exists(const std::string& path) const override;

	void* load(const std::string& fname, size_t& length) override;

	void write(const std::string& fname, void const* data, size_t length) override;
	void ensure_directory_exists(const std::string& fs_dirname) override;
	void make_directory(const std::string& fs_dirname) override;

	StreamRead* open_stream_read(const std::string& fname) override;
	StreamWrite* open_stream_write(const std::string& fname) override;

	FileSystem* make_sub_file_system(const std::string& path) override;
	FileSystem* create_sub_file_system(const std::string& path, Type type) override;
	void fs_unlink(const std::string& filename) override;
	void fs_rename(const std::string& old_name, const std::string& new_name) override;

	unsigned long long disk_space() override;  // NOLINT

	static FileSystem* create_from_directory(const std::string& directory);

	std::string get_basename() override;

private:
	enum class State { kIdle, kZipping, kUnzipping };

	// All zip filesystems that use the same zip file have a shared
	// state, which is represented in this struct. This is usually
	// shared between the root file system, plus every filesystem generated
	// through 'make_sub_file_system'.
	class ZipFile {
	public:
		explicit ZipFile(const std::string& zipfile);

		// Calls 'close()'.
		~ZipFile();

		// Make 'filename' into a relative part, dealing with legacy Widelands
		// zip file format.
		std::string strip_basename(const std::string& filename);

		// Full path to the zip file.
		const std::string& path() const;

		// Closes the file if it is open, reopens it for writing, and
		// returns the minizip handle.
		const zipFile& write_handle();

		// Closes the file if it is open, reopens it for reading, and returns the
		// minizip handle.
		const unzFile& read_handle();

	private:
		// Closes 'path_' and reopens it for unzipping (read).
		void open_for_unzip();

		// Closes 'path_' and reopens it for zipping (write).
		void open_for_zip();

		// Closes 'path_' if it is opened.
		void close();

		State state_;

		// E.g. "path/to/filename.zip"
		std::string path_;

		// E.g. "filename.zip"
		std::string basename_;

		// All files in our zipfile start with this prefix. We remember this to deal
		// with legacy Widelands files that used to keep files in sub directories
		// that had the same name than the containing zip file.
		std::string common_prefix_;

		// File handles for zipping and unzipping.
		zipFile write_handle_;
		unzFile read_handle_;
	};

	struct ZipStreamRead : StreamRead {
		explicit ZipStreamRead(const std::shared_ptr<ZipFile>& shared_data);
		~ZipStreamRead() override = default;
		size_t data(void* data, size_t bufsize) override;
		bool end_of_file() const override;

	private:
		std::shared_ptr<ZipFile> zip_file_;
	};

	struct ZipStreamWrite : StreamWrite {
		explicit ZipStreamWrite(const std::shared_ptr<ZipFile>& shared_data);
		~ZipStreamWrite() override = default;
		void data(const void* const data, size_t size) override;

	private:
		std::shared_ptr<ZipFile> zip_file_;
	};

	// Used for creating sub filesystems.
	ZipFilesystem(const std::shared_ptr<ZipFile>& shared_data,
	              const std::string& basedir_in_zip_file);

	// Place current time in tm_zip struct
	void set_time_info(tm_zip& time);

	// The data shared between all zip filesystems with the same
	// underlying zip file.
	std::shared_ptr<ZipFile> zip_file_;

	// If we are a sub filesystem, this points to our base
	// directory, otherwise it is the empty string.
	std::string basedir_in_zip_file_;
};

#endif  // end of include guard: WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H
