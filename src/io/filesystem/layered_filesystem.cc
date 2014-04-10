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

#include "build_info.h"
#include "container_iterate.h"
#include "io/fileread.h"
#include "io/streamread.h"
#include "log.h"
#include "wexception.h"

LayeredFileSystem * g_fs;

LayeredFileSystem::LayeredFileSystem(): m_home(nullptr) {}

/**
 * Free all sub-filesystems
 */
LayeredFileSystem::~LayeredFileSystem()
{
	while (m_filesystems.size()) {
		delete m_filesystems.back();
		m_filesystems.pop_back();
	}
	delete m_home;
}


/**
 * Just assume that at least one of our child FSs is writable
 * \todo Implement me
 */
bool LayeredFileSystem::IsWritable() const {
	return true;
}

/**
 * Add a new filesystem to the top of the stack.
 * Take ownership of the given filesystem.
 * \todo throw on failure
 */
void LayeredFileSystem::AddFileSystem(FileSystem & fs)
{
	//only add it to the stack if there isn't a conflicting version file in
	//the directory
	if (!FindConflictingVersionFile(fs)) {
		m_filesystems.push_back(&fs);
	} else {
		delete &fs;
		log("File system NOT added\n");
	}
}

/**
 * Set the home filesystem (which is the preferred filesystem for writing
 * files). Take ownership of the given filesystem.
 */
void LayeredFileSystem::SetHomeFileSystem(FileSystem & fs)
{
	delete m_home;
	m_home = nullptr;

	if (!FindConflictingVersionFile(fs)) {
		m_home = &fs;
	} else {
		delete &fs;
		log("File system NOT added\n");
	}
}

/**
 * Remove a filesystem from the stack
 * \param fs The filesystem to be removed
 */
void LayeredFileSystem::RemoveFileSystem(const FileSystem & fs)
{
	if (m_filesystems.back() != &fs)
		throw std::logic_error
			("LayeredFileSystem::RemoveFileSystem: interspersed add/remove "
			 "detected!");
	m_filesystems.pop_back();
}

bool LayeredFileSystem::m_read_version_from_version_file
	(FileSystem & fs, std::string * rv)
{
	FileRead fr;
	if (fr.TryOpen(fs, "VERSION")) {
		if (!fr.EndOfFile()) {
			std::string version = fr.CString();
			// Truncate extra information like branch name , (debug)
			version = version.substr(0, build_id().size());
			*rv = version;
		} else
			*rv = std::string();
		return true;
	}
	return false;
}

/**
 * Check if there is a 'version' file with a version lower than minimal version
 * number */
bool LayeredFileSystem::FindConflictingVersionFile(FileSystem & fs) {
	std::string version;

	if (m_read_version_from_version_file(fs, &version)) {
		log
			("Version file found with id \"%s\" (real \"%s\" )\n",
			 version.c_str(), build_id().c_str());
		if (version != build_id()) {
			log ("not equal strings\n");
			return true;
		}
	}
	log ("No version file found\n");
	return false;
}

/**
 * Check if there is a  'version' file with a version number equal to our
 * current version */

bool LayeredFileSystem::FindMatchingVersionFile(FileSystem & fs) {
	std::string version;

	if (m_read_version_from_version_file(fs, &version))
		if (version == build_id())
			return true;

	return false;
}

/**
 * PutRightVersionOnTop
 *
 * Look through the stack and see if we have a directory with a right version
 * file. if so always put it on top of the stack
 */

void LayeredFileSystem::PutRightVersionOnTop() {
	for
		(wl_range<std::vector<FileSystem *> >
		 i(m_filesystems);
		 i; ++i)
	{
		//check if we matching version file and it's not already on top of
		//the stack
		FileSystem & t = **i.current;
		if (FindMatchingVersionFile(t)) {
			//TODO: transform this to for loop with range (current, end()-1)
			std::vector<FileSystem *>::iterator const last = i.get_end() - 1;
			while (i.current < last) {
				FileSystem * & target = *i.current;
				target = *++i.current;
			}
			*last = &t;
			break;
		}
	}
}

/**
 * Find files in all sub-filesystems in the given path, with the given pattern.
 * Store all found files in results.
 *
 * If depth is not 0 only search this many subfilesystems.
 *
 * Returns the number of files found.
 */
std::set<std::string> LayeredFileSystem::ListDirectory(const std::string& path) {
	std::set<std::string> results;
	filenameset_t files;
	//check home system first
	if (m_home) {
		files = m_home->ListDirectory(path);
		for
			(filenameset_t::iterator fnit = files.begin();
			 fnit != files.end();
			 ++fnit)
				results.insert(*fnit);
	}

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
	{
		files = (*it)->ListDirectory(path);

		// need to workaround MSVC++6 issues
		for
			(filenameset_t::iterator fnit = files.begin();
			 fnit != files.end();
			 ++fnit)
				results.insert(*fnit);
	}
	return results;
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::FileExists(const std::string & path) {
	if (m_home and m_home->FileExists(path))
		return true;
	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->FileExists(path))
			return true;

	return false;
}

/**
 * Returns true if path is a directory in at least one of the directories
 * \todo What if it's a file in some and a dir in others?????
 */
bool LayeredFileSystem::IsDirectory(const std::string & path) {
	if (m_home and m_home->IsDirectory(path))
		return true;

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsDirectory(path))
			return true;

	return false;
}

/**
 * Read the given file into alloced memory; called by FileRead::Open.
 * Throws an exception if the file couldn't be opened.
 *
 * Note: We first query the sub-filesystem whether the file exists. Otherwise,
 * we'd have problems differentiating the errors returned by the sub-FS.
 * Let's just avoid any possible hassles with that.
 */
void * LayeredFileSystem::Load(const std::string & fname, size_t & length) {
	if (m_home && m_home->FileExists(fname))
		return m_home->Load(fname, length);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->FileExists(fname))
			return (*it)->Load(fname, length);

	throw FileNotFound_error("Could not find file", fname);
}

void * LayeredFileSystem::fastLoad
	(const std::string & fname, size_t & length, bool & fast)
{
	if (m_home && m_home->FileExists(fname))
		return m_home->fastLoad(fname, length, fast);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->FileExists(fname))
			return (*it)->fastLoad(fname, length, fast);


	throw FileNotFound_error("Could not find filename", fname);
}

/**
 * Write the given block of memory out as a file to the first writable sub-FS.
 * Throws an exception if it fails.
 */
void LayeredFileSystem::Write
	(const std::string & fname, void const * const data, int32_t const length)
{
	if (m_home and m_home->IsWritable())
		return m_home->Write(fname, data, length);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable())
			return (*it)->Write(fname, data, length);

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * Analogously to Read, open the file from the first sub-filesystem where
 * it exists.
 */
StreamRead  * LayeredFileSystem::OpenStreamRead (const std::string & fname) {
	if (m_home && m_home->FileExists(fname))
		return m_home->OpenStreamRead(fname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->FileExists(fname))
			return (*it)->OpenStreamRead(fname);

	throw FileNotFound_error("Could not find file", fname);
}

/**
 * Analogously to Write, create the file in the first writable sub-FS.
 */
StreamWrite * LayeredFileSystem::OpenStreamWrite(const std::string & fname) {
	if (m_home && m_home->IsWritable())
		return m_home->OpenStreamWrite(fname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable())
			return (*it)->OpenStreamWrite(fname);

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * MakeDir in first writable directory
 */
void LayeredFileSystem::MakeDirectory(const std::string & dirname) {
	if (m_home && m_home->IsWritable())
		return m_home->MakeDirectory(dirname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable())
			return (*it)->MakeDirectory(dirname);

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * EnsureDirectoryExists in first writable directory
 */
void LayeredFileSystem::EnsureDirectoryExists(const std::string & dirname) {
	if (m_home && m_home->IsWritable())
		return m_home->EnsureDirectoryExists(dirname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable())
			return (*it)->EnsureDirectoryExists(dirname);

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * Create a subfilesystem from an existing file/directory
 */
FileSystem * LayeredFileSystem::MakeSubFileSystem(const std::string & dirname)
{
	if (m_home and m_home->IsWritable() and m_home->FileExists(dirname))
		return m_home->MakeSubFileSystem(dirname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and (*it)->FileExists(dirname))
			return (*it)->MakeSubFileSystem(dirname);

	throw wexception("LayeredFileSystem: unable to create sub filesystem");
}

/**
 * Create a subfilesystem from a new file/directory
 */
FileSystem * LayeredFileSystem::CreateSubFileSystem(const std::string & dirname, Type const type)
{
	if (m_home and m_home->IsWritable() and not m_home->FileExists(dirname))
		return m_home->CreateSubFileSystem(dirname, type);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and not (*it)->FileExists(dirname))
			return (*it)->CreateSubFileSystem(dirname, type);

	throw wexception("LayeredFileSystem: unable to create sub filesystem");
}

/**
 * Remove this file or directory. If it is a directory, remove it recursively
 */
void LayeredFileSystem::Unlink(const std::string & file) {
	if (!FileExists(file))
		return;

	if (m_home and m_home->IsWritable() and m_home->FileExists(file)) {
		m_home->Unlink(file);
		return;
	}
	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and (*it)->FileExists(file)) {
			(*it)->Unlink(file);
			return;
		}
}

void LayeredFileSystem::Rename
	(const std::string & old_name, const std::string & new_name)
{
	if (!FileExists(old_name))
		return;
	if (m_home and m_home->IsWritable() and m_home->FileExists(old_name)) {
		m_home->Rename(old_name, new_name);
		return;
	}
	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and (*it)->FileExists(old_name)) {
			(*it)->Rename(old_name, new_name);
			return;
		}
}

// Currently we return the diskspace in either the home directory
// or the first directory on the stack we can find.
// This heuristic is justified by the fact that ths is
// where we will create new files.
unsigned long long LayeredFileSystem::DiskSpace() {
	if (m_home) {
		return m_home->DiskSpace();
	}
	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if (*it)
			return (*it)->DiskSpace();
	return 0;
}
