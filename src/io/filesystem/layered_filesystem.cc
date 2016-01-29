/*
 * Copyright (C) 2006-2009, 2011 by the Widelands Development Team
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

#include <cstdio>
#include <memory>

#include "base/log.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/streamread.h"

LayeredFileSystem * g_fs;

LayeredFileSystem::LayeredFileSystem(): m_home(nullptr) {}

/**
 * Free all sub-filesystems
 */
LayeredFileSystem::~LayeredFileSystem() {
}


/**
 * Just assume that at least one of our child FSs is writable
 */
// TODO(unknown): Implement me
bool LayeredFileSystem::is_writable() const {
	return true;
}

void LayeredFileSystem::add_file_system(FileSystem* fs) {
	m_filesystems.emplace_back(fs);
}

void LayeredFileSystem::set_home_file_system(FileSystem * fs)
{
	m_home.reset(fs);
}

/**
 * Remove a filesystem from the stack
 * \param fs The filesystem to be removed
 */
void LayeredFileSystem::remove_file_system(const FileSystem & fs)
{
	if (m_filesystems.back().get() != &fs)
		throw std::logic_error
			("LayeredFileSystem::remove_file_system: interspersed add/remove "
			 "detected!");
	m_filesystems.pop_back();
}

/**
 * Find files in all sub-filesystems in the given path, with the given pattern.
 * Store all found files in results.
 *
 * If depth is not 0 only search this many subfilesystems.
 *
 * Returns the number of files found.
 */
std::set<std::string> LayeredFileSystem::list_directory(const std::string& path) {
	std::set<std::string> results;
	FilenameSet files;
	//check home system first
	if (m_home) {
		files = m_home->list_directory(path);
		for
			(FilenameSet::iterator fnit = files.begin();
			 fnit != files.end();
			 ++fnit)
				results.insert(*fnit);
	}

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it) {
		files = (*it)->list_directory(path);

		for (FilenameSet::iterator fnit = files.begin(); fnit != files.end(); ++fnit)
			   results.insert(*fnit);
	}
	return results;
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::file_exists(const std::string & path) {
	if (m_home && m_home->file_exists(path))
		return true;
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->file_exists(path))
			return true;

	return false;
}

/**
 * Returns true if path is a directory in at least one of the directories
 */
// TODO(unknown): What if it's a file in some and a dir in others?????
bool LayeredFileSystem::is_directory(const std::string & path) {
	if (m_home && m_home->is_directory(path))
		return true;

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_directory(path))
			return true;

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
void * LayeredFileSystem::load(const std::string & fname, size_t & length) {
	if (m_home && m_home->file_exists(fname))
		return m_home->load(fname, length);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->file_exists(fname))
			return (*it)->load(fname, length);

	throw FileNotFoundError("Could not find file", fname);
}

/**
 * Write the given block of memory out as a file to the first writable sub-FS.
 * Throws an exception if it fails.
 */
void LayeredFileSystem::write
	(const std::string & fname, void const * const data, int32_t const length)
{
	if (m_home && m_home->is_writable())
		return m_home->write(fname, data, length);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable())
			return (*it)->write(fname, data, length);

	throw wexception("LayeredFileSystem: No writable filesystem for %s!", fname.c_str());
}

/**
 * Analogously to Read, open the file from the first sub-filesystem where
 * it exists.
 */
StreamRead  * LayeredFileSystem::open_stream_read (const std::string & fname) {
	if (m_home && m_home->file_exists(fname))
		return m_home->open_stream_read(fname);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->file_exists(fname))
			return (*it)->open_stream_read(fname);

	throw FileNotFoundError("Could not find file", fname);
}

/**
 * Analogously to Write, create the file in the first writable sub-FS.
 */
StreamWrite * LayeredFileSystem::open_stream_write(const std::string & fname) {
	if (m_home && m_home->is_writable())
		return m_home->open_stream_write(fname);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable())
			return (*it)->open_stream_write(fname);

	throw wexception("LayeredFileSystem: No writable filesystem for %s!", fname.c_str());
}

/**
 * MakeDir in first writable directory
 */
void LayeredFileSystem::make_directory(const std::string & dirname) {
	if (m_home && m_home->is_writable())
		return m_home->make_directory(dirname);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable())
			return (*it)->make_directory(dirname);

	throw wexception("LayeredFileSystem: No writable filesystem for dir: %s!", dirname.c_str());
}

/**
 * ensure_directory_exists in first writable directory
 */
void LayeredFileSystem::ensure_directory_exists(const std::string & dirname) {
	if (m_home && m_home->is_writable())
		return m_home->ensure_directory_exists(dirname);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable())
			return (*it)->ensure_directory_exists(dirname);

	throw wexception("LayeredFileSystem: No writable filesystem for dir: %s!", dirname.c_str());
}

/**
 * Create a subfilesystem from an existing file/directory
 */
FileSystem * LayeredFileSystem::make_sub_file_system(const std::string & dirname)
{
	if (m_home && m_home->is_writable() && m_home->file_exists(dirname))
		return m_home->make_sub_file_system(dirname);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable() && (*it)->file_exists(dirname))
			return (*it)->make_sub_file_system(dirname);

	throw wexception("LayeredFileSystem: unable to create sub filesystem for existing dir: %s",
						  dirname.c_str());
}

/**
 * Create a subfilesystem from a new file/directory
 */
FileSystem * LayeredFileSystem::create_sub_file_system(const std::string & dirname, Type const type)
{
	if (m_home && m_home->is_writable() && !m_home->file_exists(dirname))
		return m_home->create_sub_file_system(dirname, type);

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable() && !(*it)->file_exists(dirname))
			return (*it)->create_sub_file_system(dirname, type);

	throw wexception("LayeredFileSystem: unable to create sub filesystem for new dir: %s", dirname.c_str());
}

/**
 * Remove this file or directory. If it is a directory, remove it recursively
 */
void LayeredFileSystem::fs_unlink(const std::string & file) {
	if (!file_exists(file))
		return;

	if (m_home && m_home->is_writable() && m_home->file_exists(file)) {
		m_home->fs_unlink(file);
		return;
	}

	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable() && (*it)->file_exists(file)) {
			(*it)->fs_unlink(file);
			return;
		}
}

void LayeredFileSystem::fs_rename
	(const std::string & old_name, const std::string & new_name)
{
	if (!file_exists(old_name))
		return;
	if (m_home && m_home->is_writable() && m_home->file_exists(old_name)) {
		m_home->fs_rename(old_name, new_name);
		return;
	}
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if ((*it)->is_writable() && (*it)->file_exists(old_name)) {
			(*it)->fs_rename(old_name, new_name);
			return;
		}
}

// Currently we return the diskspace in either the home directory
// or the first directory on the stack we can find.
// This heuristic is justified by the fact that ths is
// where we will create new files.
unsigned long long LayeredFileSystem::disk_space() {
	if (m_home) {
		return m_home->disk_space();
	}
	for (auto it = m_filesystems.rbegin(); it != m_filesystems.rend(); ++it)
		if (*it)
			return (*it)->disk_space();
	return 0;
}
