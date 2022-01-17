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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "io/filesystem/layered_filesystem.h"

#include <memory>

#include "base/wexception.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/streamread.h"

LayeredFileSystem* g_fs;
LayeredFileSystem::LayeredFileSystem() : home_(nullptr) {
}

/**
 * Just assume that at least one of our child FSs is writable
 */
// TODO(unknown): Implement me
bool LayeredFileSystem::is_writable() const {
	return true;
}

void LayeredFileSystem::add_file_system(FileSystem* fs) {
	filesystems_.emplace_back(fs);
}

void LayeredFileSystem::set_home_file_system(FileSystem* fs) {
	home_.reset(fs);
}

/**
 * Find files in all sub-filesystems in the given path, with the given pattern.
 * Store all found files in results.
 *
 * If depth is not 0 only search this many subfilesystems.
 *
 * Returns the number of files found.
 */
FilenameSet LayeredFileSystem::list_directory(const std::string& path) const {
	std::set<std::string> results;
	FilenameSet files;
	// Check home system first
	if (home_) {
		files = home_->list_directory(path);
		for (const std::string& file : files) {
			results.insert(file);
		}
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		files = (*it)->list_directory(path);

		for (const std::string& file : files) {
			results.insert(file);
		}
	}
	return results;
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::file_exists(const std::string& path) const {
	if (home_ && home_->file_exists(path)) {
		return true;
	}
	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->file_exists(path)) {
			return true;
		}
	}

	return false;
}

/**
 * Returns true if path is a directory in at least one of the directories
 */
// TODO(unknown): What if it's a file in some and a dir in others?????
bool LayeredFileSystem::is_directory(const std::string& path) const {
	if (home_ && home_->is_directory(path)) {
		return true;
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_directory(path)) {
			return true;
		}
	}

	return false;
}

/**
 * Read the given file into alloced memory; called by FileRead::open.
 * Throws an exception if the file couldn't be opened.
 *
 * Note: We first query the sub-filesystem whether the file exists. Otherwise,
 * we'd have problems differentiating the errors returned by the sub-FS.
 * Let's just avoid any possible hassles with that.
 */
void* LayeredFileSystem::load(const std::string& fname, size_t& length) {
	if (home_ && home_->file_exists(fname)) {
		return home_->load(fname, length);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->file_exists(fname)) {
			return (*it)->load(fname, length);
		}
	}

	throw FileNotFoundError("LayeredFileSystem: Could not load file", paths_error_message(fname));
}

/**
 * Write the given block of memory out as a file to the first writable sub-FS.
 * Throws an exception if it fails.
 */
void LayeredFileSystem::write(const std::string& fname,
                              void const* const data,
                              size_t const length) {
	if (home_ && home_->is_writable()) {
		return home_->write(fname, data, length);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable()) {
			return (*it)->write(fname, data, length);
		}
	}

	throw wexception(
	   "LayeredFileSystem: No writable filesystem for file: %s", paths_error_message(fname).c_str());
}

/**
 * Analogously to Read, open the file from the first sub-filesystem where
 * it exists.
 */
StreamRead* LayeredFileSystem::open_stream_read(const std::string& fname) {
	if (home_ && home_->file_exists(fname)) {
		return home_->open_stream_read(fname);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->file_exists(fname)) {
			return (*it)->open_stream_read(fname);
		}
	}

	throw FileNotFoundError(
	   "LayeredFileSystem: Could not open file for stream read", paths_error_message(fname));
}

/**
 * Analogously to Write, create the file in the first writable sub-FS.
 */
StreamWrite* LayeredFileSystem::open_stream_write(const std::string& fname) {
	if (home_ && home_->is_writable()) {
		return home_->open_stream_write(fname);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable()) {
			return (*it)->open_stream_write(fname);
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem for %s!", fname.c_str());
}

/**
 * MakeDir in first writable directory
 */
void LayeredFileSystem::make_directory(const std::string& dirname) {
	if (home_ && home_->is_writable()) {
		return home_->make_directory(dirname);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable()) {
			return (*it)->make_directory(dirname);
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem for dir: %s!", dirname.c_str());
}

/**
 * ensure_directory_exists in first writable directory
 */
void LayeredFileSystem::ensure_directory_exists(const std::string& dirname) {
	if (home_ && home_->is_writable()) {
		return home_->ensure_directory_exists(dirname);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable()) {
			return (*it)->ensure_directory_exists(dirname);
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem for dir: %s!", dirname.c_str());
}

/**
 * Create a subfilesystem from an existing file/directory
 */
FileSystem* LayeredFileSystem::make_sub_file_system(const std::string& dirname) {
	if (home_ && home_->is_writable() && home_->file_exists(dirname)) {
		return home_->make_sub_file_system(dirname);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable() && (*it)->file_exists(dirname)) {
			return (*it)->make_sub_file_system(dirname);
		}
	}

	try {
		return &FileSystem::create(canonicalize_name(dirname));
	} catch (const FileError&) {
		throw FileNotFoundError(
		   "LayeredFileSystem: unable to create sub filesystem for existing directory: %s",
		   paths_error_message(dirname));
	}
}

/**
 * Create a subfilesystem from a new file/directory
 */
FileSystem* LayeredFileSystem::create_sub_file_system(const std::string& dirname, Type const type) {
	if (home_ && home_->is_writable() && !home_->file_exists(dirname)) {
		return home_->create_sub_file_system(dirname, type);
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable() && !(*it)->file_exists(dirname)) {
			return (*it)->create_sub_file_system(dirname, type);
		}
	}

	throw wexception("LayeredFileSystem: unable to create sub filesystem for new directory: %s",
	                 paths_error_message(dirname).c_str());
}

/**
 * Remove this file or directory. If it is a directory, remove it recursively
 */
void LayeredFileSystem::fs_unlink(const std::string& file) {
	if (!file_exists(file)) {
		return;
	}

	if (home_ && home_->is_writable() && home_->file_exists(file)) {
		home_->fs_unlink(file);
		return;
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable() && (*it)->file_exists(file)) {
			(*it)->fs_unlink(file);
			return;
		}
	}
}

void LayeredFileSystem::fs_rename(const std::string& old_name, const std::string& new_name) {
	if (!file_exists(old_name)) {
		return;
	}
	if (home_ && home_->is_writable() && home_->file_exists(old_name)) {
		home_->fs_rename(old_name, new_name);
		return;
	}
	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if ((*it)->is_writable() && (*it)->file_exists(old_name)) {
			(*it)->fs_rename(old_name, new_name);
			return;
		}
	}
}

// Currently we return the diskspace in either the home directory
// or the first directory on the stack we can find.
// This heuristic is justified by the fact that ths is
// where we will create new files.
unsigned long long LayeredFileSystem::disk_space() {  // NOLINT
	if (home_) {
		return home_->disk_space();
	}
	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		if (*it) {
			return (*it)->disk_space();
		}
	}
	return 0;
}

std::string LayeredFileSystem::paths_error_message(const std::string& filename) const {

	std::string message = filename + "\nI have tried the following path(s):";

	if (home_) {
		message += "\n    " + home_->get_basename() + FileSystem::file_separator() + filename;
	}

	for (auto it = filesystems_.rbegin(); it != filesystems_.rend(); ++it) {
		message += "\n    " + (*it)->get_basename() + FileSystem::file_separator() + filename;
	}

	message += "\n    " + canonicalize_name(filename);

	return message;
}
