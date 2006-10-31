/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#include "layeredfilesystem.h"
#include "wexception.h"

LayeredFileSystem *g_fs;

/**
 * Initialize
 */
LayeredFileSystem::LayeredFileSystem()
{
}

/**
 * Free all sub-filesystems
 */
LayeredFileSystem::~LayeredFileSystem()
{
	while(!m_filesystems.empty()) {
		FileSystem *fs = m_filesystems.back();
		delete fs;
		m_filesystems.pop_back();
	}
}

///Just a little debugging aid
void LayeredFileSystem::listSubdirs()
{
	std::vector<FileSystem*>::iterator i;
	for(i=m_filesystems.begin();i!=m_filesystems.end();i++)
	{
		(*i)->listSubdirs();
	}
}

/**
 * Just assume that at least one of our child FSs is writable
 * \todo Implement me
 */
bool LayeredFileSystem::IsWritable()
{
	return true;
}

/**
 * Add a new filesystem to the top of the stack
 * \todo use FileSystem & instead of pointer, throw on failure
 */
void LayeredFileSystem::AddFileSystem(FileSystem *fs)
{
	m_filesystems.push_back(fs);
}

/**
 * Find files in all sub-filesystems in the given path, with the given pattern.
 * Store all found files in results.
 *
 * If depth is not 0 only search this many subfilesystems.
 *
 * Returns the number of files found.
 */
int LayeredFileSystem::FindFiles(std::string path, std::string pattern, filenameset_t *results, int depth)
{
	int i=0;
	if(!depth)
		depth=10000; // Wow, if you have so many filesystem you're my hero

	for(FileSystem_rit it = m_filesystems.rbegin(); (it != m_filesystems.rend()) && (i<depth); it++, i++) {
		filenameset_t files;
		(*it)->FindFiles(path, pattern, &files);

		// need to workaround MSVC++6 issues
		//results->insert(files.begin(), files.end());
		for(filenameset_t::iterator fnit = files.begin(); fnit != files.end(); fnit++)
			results->insert(*fnit);
	}

	return results->size();
}

int LayeredFileSystem::FindFiles(std::string path, std::string pattern, filenameset_t *results) {
	return FindFiles(path,pattern,results,0);
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::FileExists(std::string path)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if ((*it)->FileExists(path))
			return true;
	}

	return false;
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
bool LayeredFileSystem::IsDirectory(std::string path)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if ((*it)->IsDirectory(path))
			return true;
	}

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
void *LayeredFileSystem::Load(std::string fname, int *length)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->FileExists(fname))
			continue;

		return (*it)->Load(fname, length);
	}

	throw FileNotFound_error(fname);
}


/**
 * Write the given block of memory out as a file to the first writable sub-FS.
 * Throws an exception if it fails.
 */
void LayeredFileSystem::Write(std::string fname, void *data, int length)
{
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->Write(fname, data, length);
		return;
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * MakeDir in first writable directory
 */
void LayeredFileSystem::MakeDirectory(std::string dirname) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->MakeDirectory(dirname);
		return;
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * EnsureDirectoryExists in first writable directory
 */
void LayeredFileSystem::EnsureDirectoryExists(std::string dirname) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		(*it)->EnsureDirectoryExists(dirname);
		return;
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * Create a subfilesystem from this directory
 */
FileSystem* LayeredFileSystem::MakeSubFileSystem(std::string dirname ) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		if(! (*it)->FileExists(dirname))
			continue;

		return (*it)->MakeSubFileSystem(dirname);
	}

	throw wexception("LayeredFileSystem: Wasn't able to create sub filesystem!");
}

/**
 * Create a new subfilesystem
 */
FileSystem* LayeredFileSystem::CreateSubFileSystem(std::string dirname, Type type ) {
	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;

		return (*it)->CreateSubFileSystem( dirname, type );
	}

	throw wexception("LayeredFileSystem: Wasn't able to create sub filesystem!");
}

/**
 * Remove this file or directory. If it is a directory, remove it recursivly
 */
void LayeredFileSystem::Unlink( std::string file ) {
	if( !FileExists( file ))
		return;

	for(FileSystem_rit it = m_filesystems.rbegin(); it != m_filesystems.rend(); it++) {
		if (!(*it)->IsWritable())
			continue;
		if (!(*it)->FileExists(file))
			continue;

		(*it)->Unlink(file);
		return;
	}
}
