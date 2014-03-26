/*
 * Copyright (C) 2002-2003, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <cstring>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include <stdint.h>

#include "io/filesystem/filesystem_exceptions.h"

typedef std::set<std::string> filenameset_t;

class StreamRead;
class StreamWrite;


/**
 * FileSystem is an abstract base class representing certain filesystem
 * operations.
 * \todo const correctness
 */
class FileSystem {
public:
	//  TODO This should be unnecessary. Make it so.
	enum Type {
		DIR,
		ZIP
	};

	virtual ~FileSystem() {}

	// Returns all files and directories (full path) in the given directory 'directory'.
	virtual std::set<std::string> ListDirectory(const std::string& directory) = 0;

	virtual bool IsWritable() const = 0;
	virtual bool IsDirectory(const std::string & path) = 0;
	virtual bool FileExists (const std::string & path) = 0;

	virtual void * Load(const std::string & fname, size_t & length) = 0;
	virtual void * fastLoad
		(const std::string & fname, size_t & length, bool & fast) = 0;

	virtual void Write
		(const std::string & fname, void const * data, int32_t length)
		= 0;
	virtual void EnsureDirectoryExists(const std::string & dirname) = 0;
	//TODO: use this only from inside EnsureDirectoryExists()
	virtual void MakeDirectory(const std::string & dirname) = 0;

	/**
	 * Opens the given file for reading as a stream.
	 * Throws an exception if the file couldn't be opened.
	 *
	 * \return a \ref StreamRead object for the file. The caller must delete this
	 * object when done to close the file.
	 */
	virtual StreamRead * OpenStreamRead(const std::string & fname) = 0;

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
	virtual StreamWrite * OpenStreamWrite(const std::string & fname) = 0;

	/**
	 * Creates a subfilesystem from an existing file/directory.
	 * Passes ownership to caller.
	 */
	virtual FileSystem * MakeSubFileSystem(const std::string & dirname) = 0;
	/**
	 * Creates a subfilesystem from a new file/directory.
	 * Passes ownership to caller.
	 */
	virtual FileSystem * CreateSubFileSystem(const std::string & dirname, Type) = 0;
	virtual void Unlink(const std::string &) = 0;
	virtual void Rename(const std::string &, const std::string &) = 0;

	static FileSystem & Create(const std::string & root)
;

	///Retrieve the filesystem root's name == the mountpoint inside a
	///LayeredFileSystem
	virtual std::string getBasename() = 0;

	// basic path/filename manipulation
	std::string fixCrossFile(const std::string &) const;
	char fileSeparator() {return m_filesep;}
	std::string getWorkingDirectory() const;
	std::string FS_CanonicalizeName(std::string path) const;
	bool pathIsAbsolute(const std::string & path) const;

	///Given a filename, return the name with any path stripped off.
	static const char * FS_Filename(const char * n);
	static std::string FS_Dirname(const std::string& full_path);

	///Given a filename (without any path), return the extension, if any.
	static std::string FS_FilenameExt(const std::string & f);

	///Given a filename, return the name with any path or extension stripped off.
	static std::string FS_FilenameWoExt(const char * n);
	static std::string GetHomedir();

	virtual unsigned long long DiskSpace() = 0;

protected:
	///To get a filesystem, use the Create methods
	FileSystem();

	///How to address the fs' topmost component (e.g. "" on Unix, "D:" on win32)
	///\warning This is should \e not contain filesep!
	std::string m_root;

	///Character used to separate filename components
	char m_filesep;

#ifdef _WIN32
private:
	static bool check_writeable_for_data(char const * path);
#endif

};

#endif
