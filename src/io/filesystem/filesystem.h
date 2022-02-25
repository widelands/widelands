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

#ifndef WL_IO_FILESYSTEM_FILESYSTEM_H
#define WL_IO_FILESYSTEM_FILESYSTEM_H

#include <set>
#include <string>
#include <vector>

using FilenameSet = std::set<std::string>;

class StreamRead;
class StreamWrite;

/**
 * FileSystem is an abstract base class representing certain filesystem
 * operations.
 */
// TODO(unknown): const correctness
class FileSystem {
public:
	//  TODO(unknown): This should be unnecessary. Make it so.
	enum Type { DIR, ZIP };

	virtual ~FileSystem() {
	}

	// Returns all files and directories (full path) in the given directory 'directory'.
	virtual FilenameSet list_directory(const std::string& directory) const = 0;

	virtual bool is_writable() const = 0;
	virtual bool is_directory(const std::string& path) const = 0;
	virtual bool file_exists(const std::string& path) const = 0;

	virtual void* load(const std::string& fname, size_t& length) = 0;

	virtual void write(const std::string& fname, void const* data, size_t length) = 0;
	virtual void ensure_directory_exists(const std::string& fs_dirname) = 0;
	// TODO(unknown): use this only from inside ensure_directory_exists()
	virtual void make_directory(const std::string& fs_dirname) = 0;

	/**
	 * Opens the given file for reading as a stream.
	 * Throws an exception if the file couldn't be opened.
	 *
	 * \return a \ref StreamRead object for the file. The caller must delete this
	 * object when done to close the file.
	 */
	virtual StreamRead* open_stream_read(const std::string& fname) = 0;

	/**
	 * Opens the given file for writing as a stream.
	 * A pre-existing file will be overwritten.
	 *
	 * Throws an exception if the file couldn't be opened.
	 *
	 * \return a \ref StreamWrite object for the file. The caller must
	 * delete this object when done to close the file (which will implicitly
	 * flush unwritten data).
	 */
	virtual StreamWrite* open_stream_write(const std::string& fname) = 0;

	/**
	 * Creates a subfilesystem from an existing file/directory.
	 * Passes ownership to caller.
	 */
	virtual FileSystem* make_sub_file_system(const std::string& fs_dirname) = 0;
	/**
	 * Creates a subfilesystem from a new file/directory.
	 * Passes ownership to caller.
	 */
	virtual FileSystem* create_sub_file_system(const std::string& fs_dirname, Type) = 0;
	virtual void fs_unlink(const std::string&) = 0;
	virtual void fs_rename(const std::string&, const std::string&) = 0;

	static FileSystem& create(const std::string& root);

	/// Retrieve the filesystem root's name == the mountpoint inside a
	/// LayeredFileSystem
	virtual std::string get_basename() = 0;

	// basic path/filename manipulation
	std::string fix_cross_file(const std::string&) const;
	std::string canonicalize_name(const std::string& path) const;
	bool is_path_absolute(const std::string& path) const;

	// Returns the path separator, i.e. \ on windows and / everywhere else.
	static char file_separator();

	// Returns the working directory of the process.
	static std::string get_working_directory();

	/// Given a filename, return the name with any path stripped off.
	static const char* fs_filename(const char* p);

	// Everything before the final separator (/ or \) in 'full_path'. The
	// returned value is either the empty string or ends with a separator.
	static std::string fs_dirname(const std::string& full_path);

	/// Given a filename (without any path), return the extension, if any.
	static std::string filename_ext(const std::string& f);

	/// Given a filename, return the name with any path or extension stripped off.
	static std::string filename_without_ext(const char* p);
	static std::string get_homedir();

#ifdef USE_XDG
	// Get XDG dirs for Linux
	// https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
	static std::string get_userdatadir();
	static std::string get_userconfigdir();
	static std::vector<std::string> get_xdgdatadirs();
#endif

	/// Return the files in the given 'directory' that match the condition in 'test', i.e. 'test'
	/// returned 'true' for their filenames.
	template <class UnaryPredicate>
	FilenameSet filter_directory(const std::string& directory, UnaryPredicate test) const {
		FilenameSet result = list_directory(directory);
		for (auto it = result.begin(); it != result.end();) {
			if (!test(*it)) {
				it = result.erase(it);
			} else {
				++it;
			}
		}
		return result;
	}

	/// Returns all files in the given 'directory' that match 'basename' followed by 1-3 numbers,
	/// followed by '.', followed by 'extension'
	std::vector<std::string> get_sequential_files(const std::string& directory,
	                                              const std::string& basename,
	                                              const std::string& extension) const;

	virtual unsigned long long disk_space() = 0;  // NOLINT

protected:
	/// To get a filesystem, use the Create methods
	FileSystem() = default;

	/// How to address the fs' topmost component (e.g. "" on Unix, "D:" on win32)
	/// \warning This is should \e not contain filesep!
	std::string root_;

#ifdef _WIN32
private:
	static bool check_writeable_for_data(char const* path);
#endif
};

#endif  // end of include guard: WL_IO_FILESYSTEM_FILESYSTEM_H
