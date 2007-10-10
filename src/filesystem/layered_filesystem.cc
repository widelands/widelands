/*
 * Copyright (C) 2006-2007 by the Widelands Development Team
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

#include "wexception.h"

LayeredFileSystem *g_fs;


LayeredFileSystem::LayeredFileSystem() {}

/**
 * Free all sub-filesystems
 */
LayeredFileSystem::~LayeredFileSystem()
{
	while (!m_filesystems.empty()) {
		delete m_filesystems.back();
		m_filesystems.pop_back();
	}
}

///Just a little debugging aid
void LayeredFileSystem::listSubdirs() const
{
	std::vector<FileSystem*>::const_iterator i;
	for (i=m_filesystems.begin();i!=m_filesystems.end();i++)
	{
		printf("   %s\n", (*i)->getBasename().c_str());
	}
}

/**
 * Just assume that at least one of our child FSs is writable
 * \todo Implement me
 */
const bool LayeredFileSystem::IsWritable() const
{
	return true;
}

/**
 * Add a new filesystem to the top of the stack
 * \todo use FileSystem & instead of pointer, throw on failure
 */
void LayeredFileSystem::AddFileSystem(FileSystem * const fs)
{
	m_filesystems.push_back(fs);
}

/**
 * Remove a filesystem from the stack
 * \param fs The filesystem to be removed
 */
void LayeredFileSystem::RemoveFileSystem(FileSystem * const fs)
{
	if (m_filesystems.back() == fs) {
		m_filesystems.pop_back();
	} else
		throw std::logic_error
			("LayeredFileSystem::RemoveFileSystem: interspersed add/remove "
			 "detected!");
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
const int32_t LayeredFileSystem::FindFiles(std::string path,
                                       const std::string pattern,
                                       filenameset_t *results,
                                       uint32_t depth)
{
	uint32_t i=0;
	if (depth==0)
		depth=10000; // Wow, if you have so many filesystem you're my hero

	for (FileSystem_rit it = m_filesystems.rbegin();
	      (it != m_filesystems.rend()) && (i<depth); it++, i++)
	{
		filenameset_t files;
		(*it)->FindFiles(path, pattern, &files);

		// need to workaround MSVC++6 issues
		//results->insert(files.begin(), files.end());
		for (filenameset_t::iterator fnit = files.begin();
		      fnit != files.end(); fnit++)
			results->insert(*fnit);
	}

	return results->size();
}

/**
 * Returns true if the file can be found in at least one of the sub-filesystems
 */
const bool LayeredFileSystem::FileExists(const std::string path)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if ((*it)->FileExists(path))
			return true;
	}

	return false;
}

/**
 * Returns true if path is a directory in at least one of the directories
 * \todo What if it's a file in some and a dir in others?????
 */
const bool LayeredFileSystem::IsDirectory(const std::string path)
{
	for
		(FileSystem_rit it = m_filesystems.rbegin();
		 it != m_filesystems.rend();
		 ++it)
	{
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
void * LayeredFileSystem::Load(const std::string & fname, size_t & length) {
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if ((*it)->FileExists(fname))
			return (*it)->Load(fname, length);
	}

	throw FileNotFound_error("Could not find file", fname);
}


/**
 * Write the given block of memory out as a file to the first writable sub-FS.
 * Throws an exception if it fails.
 */
void LayeredFileSystem::Write(const std::string fname, const void * const data,
                              const int32_t length)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if ((*it)->IsWritable())
		{
			(*it)->Write(fname, data, length);
			return;
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * Analogously to Read, open the file from the first sub-filesystem where
 * it exists.
 */
StreamRead * LayeredFileSystem::OpenStreamRead
(const std::string & fname)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	    it != m_filesystems.rend(); it++)
	{
		if ((*it)->FileExists(fname))
			return (*it)->OpenStreamRead(fname);
	}

	throw FileNotFound_error("Could not find file", fname);
}

/**
 * Analogously to Write, create the file in the first writable sub-FS.
 */
StreamWrite * LayeredFileSystem::OpenStreamWrite
(const std::string & fname)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	    it != m_filesystems.rend(); it++)
	{
		if ((*it)->IsWritable())
			return (*it)->OpenStreamWrite(fname);
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * MakeDir in first writable directory
 */
void LayeredFileSystem::MakeDirectory(const std::string dirname)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if ((*it)->IsWritable())
		{
			(*it)->MakeDirectory(dirname);
			return;
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * EnsureDirectoryExists in first writable directory
 */
void LayeredFileSystem::EnsureDirectoryExists(const std::string dirname)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if ((*it)->IsWritable())
		{
			(*it)->EnsureDirectoryExists(dirname);
			return;
		}
	}

	throw wexception("LayeredFileSystem: No writable filesystem!");
}

/**
 * Create a subfilesystem from an existing file/directory
 */
FileSystem* LayeredFileSystem::MakeSubFileSystem(const std::string dirname)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if (!(*it)->IsWritable())
			continue;

		if (!(*it)->FileExists(dirname))
			continue;

		return (*it)->MakeSubFileSystem(dirname);
	}

	printf("dirname %s\n", dirname.c_str());
	throw wexception("LayeredFileSystem: Wasn't able to make sub filesystem!");
}

/**
 * Create a subfilesystem from a new file/directory
 */
FileSystem* LayeredFileSystem::CreateSubFileSystem(const std::string dirname,
      const Type type)
{
	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if (!(*it)->IsWritable())
			continue;

		if ((*it)->FileExists(dirname))
			continue;

		return (*it)->CreateSubFileSystem(dirname, type);
	}

	printf("dirname %s\n", dirname.c_str());
	throw wexception("LayeredFileSystem: Wasn't able to create sub filesystem!");
}

/**
 * Remove this file or directory. If it is a directory, remove it recursively
 */
void LayeredFileSystem::Unlink(const std::string file)
{
	if (!FileExists(file))
		return;

	for (FileSystem_rit it = m_filesystems.rbegin();
	      it != m_filesystems.rend(); it++)
	{
		if (!(*it)->IsWritable())
			continue;
		if (!(*it)->FileExists(file))
			continue;

		(*it)->Unlink(file);
		return;
	}
}

void LayeredFileSystem::Rename
(const std::string & old_name, const std::string & new_name)
{
	if (!FileExists(old_name))
		return;

	for (FileSystem_rit it = m_filesystems.rbegin();
		it != m_filesystems.rend(); it++)
	{
		if (!(*it)->IsWritable())
			continue;
		if (!(*it)->FileExists(old_name))
			continue;

		(*it)->Rename(old_name, new_name);
		return;
	}
}
