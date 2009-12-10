/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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

#include "layered_filesystem.h"
#include "io/streamread.h"

#include "build_info.h"
#include "log.h"
#include "wexception.h"

#include "container_iterate.h"

#include <cstdio>
#include <memory>

LayeredFileSystem * g_fs;

LayeredFileSystem::LayeredFileSystem(): m_home(0) {}

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
 * Add a new filesystem to the top of the stack
 * \todo throw on failure
 */
void LayeredFileSystem::AddFileSystem(FileSystem & fs)
{
	//only add it to the stack if there isn't a conflicting version file in
	//the directory
	if (! FindConflictingVersionFile(fs)) {
		m_filesystems.push_back(&fs);
	} else {
		log("File system NOT added\n");
	}
}
void LayeredFileSystem::SetHomeFileSystem(FileSystem & fs)
{
	if (! FindConflictingVersionFile(fs)) {
		m_home = &fs;
	} else {
		log("File system NOT added\n");
	}
}

/**
 * Remove a filesystem from the stack
 * \param fs The filesystem to be removed
 */
void LayeredFileSystem::RemoveFileSystem(FileSystem const & fs)
{
	if (m_filesystems.back() != &fs)
		throw std::logic_error
			("LayeredFileSystem::RemoveFileSystem: interspersed add/remove "
			 "detected!");
	m_filesystems.pop_back();
}

/**
 * Check if there is a 'version' file with a version lower than minimal version
 * number */
bool LayeredFileSystem::FindConflictingVersionFile(FileSystem & fs) {
	if (fs.FileExists("VERSION")) {
		std::auto_ptr<StreamRead> sr (fs.OpenStreamRead("VERSION"));
		if (sr->EndOfFile())
			return false;
		std::string version = sr->String();
		version = version.substr(0, build_id().size());
		log
			("Version file found with id \"%s\" (real \"%s\" )\n",
			 version.c_str(), build_id().c_str());
		if (version.compare(build_id()) != 0) {
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
	if (fs.FileExists("VERSION")) {
		std::auto_ptr<StreamRead> sr (fs.OpenStreamRead("VERSION"));
		if (sr->EndOfFile())
			return false;
		std::string version = sr->String();
		version = version.substr(0, build_id().size());
		if (version.compare(build_id()) == 0) {
			return true;
		}
	}
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
		(struct {
		 	std::vector<FileSystem *>::iterator       current;
		 	std::vector<FileSystem *>::iterator const end;
		 } i = {m_filesystems.begin(), m_filesystems.end()};
		 i.current != i.end;
		 ++i.current)
	{
		//check if we matching version file and it's not already on top of
		//the stack
		FileSystem & t = **i.current;
		if (FindMatchingVersionFile(t)) {
			std::vector<FileSystem *>::iterator const last = i.end - 1;
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
//TODO: return type is wrong
int32_t LayeredFileSystem::FindFiles
	(std::string const &       path,
	 std::string const &       pattern,
	 filenameset_t     * const results,
	 uint32_t                  depth)
{
	uint32_t i = 0;
	if (depth == 0)
		depth = 10000; //  Wow, if you have so many filesystem you're my hero

	filenameset_t files;
	//check home system first
	if (m_home) {
		m_home->FindFiles(path, pattern, &files);
		for
			(filenameset_t::iterator fnit = files.begin();
			 fnit != files.end();
			 ++fnit)
				results->insert(*fnit);
	}

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend() && i < depth;
		 ++it, ++i)
	{
		(*it)->FindFiles(path, pattern, &files);

		// need to workaround MSVC++6 issues
		//results->insert(files.begin(), files.end());
		for
			(filenameset_t::iterator fnit = files.begin();
			 fnit != files.end();
			 ++fnit)
				results->insert(*fnit);
	}

	return results->size();
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::FileExists(std::string const & path) {
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
bool LayeredFileSystem::IsDirectory(std::string const & path) {
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
	(std::string const & fname, void const * const data, int32_t const length)
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
StreamWrite * LayeredFileSystem::OpenStreamWrite(std::string const & fname) {
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
void LayeredFileSystem::MakeDirectory(std::string const & dirname) {
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
void LayeredFileSystem::EnsureDirectoryExists(std::string const & dirname) {
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
FileSystem & LayeredFileSystem::MakeSubFileSystem(std::string const & dirname)
{
	if (m_home and m_home->IsWritable() and m_home->FileExists(dirname))
		return m_home->MakeSubFileSystem(dirname);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and (*it)->FileExists(dirname))
			return (*it)->MakeSubFileSystem(dirname);

	printf("dirname %s\n", dirname.c_str());
	throw wexception("LayeredFileSystem: unable to create sub filesystem");
}

/**
 * Create a subfilesystem from a new file/directory
 */
FileSystem & LayeredFileSystem::CreateSubFileSystem
	(std::string const & dirname, Type const type)
{
	if (m_home and m_home->IsWritable() and not m_home->FileExists(dirname))
		return m_home->CreateSubFileSystem(dirname, type);

	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
		if ((*it)->IsWritable() and not (*it)->FileExists(dirname))
			return (*it)->CreateSubFileSystem(dirname, type);

	printf("dirname %s\n", dirname.c_str());
	throw wexception("LayeredFileSystem: unable to create sub filesystem");
}

/**
 * Remove this file or directory. If it is a directory, remove it recursively
 */
void LayeredFileSystem::Unlink(std::string const & file) {
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
	(std::string const & old_name, std::string const & new_name)
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

//FIXME: need more control about where we are checking for disk space
// Currently we return the diskspace in either the home directory
// or the first directory on the stack we can find
// this will probally work ok since this is where we will create new files
unsigned long LayeredFileSystem::DiskSpace() {
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
