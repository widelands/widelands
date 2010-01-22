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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "filesystem_exceptions.h"

#include <stdint.h>
#include <cstring>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
#include <memory>


typedef std::set<std::string> filenameset_t;

class StreamRead;
class StreamWrite;


/**
 * FileSystem is an abstract base class representing certain filesystem
 * operations.
 * \todo const correctness
 */
struct FileSystem {
	//  TODO This should be unnecessary. Make it so.
	enum Type {
		DIR,
		ZIP
	};

	virtual ~FileSystem() {}

	virtual int32_t FindFiles
		(std::string const & path,
		 std::string const & pattern,
		 filenameset_t     * results,
		 uint32_t            depth = 0)
		= 0;

	virtual bool IsWritable() const = 0;
	virtual bool IsDirectory(std::string const & path) = 0;
	virtual bool FileExists (std::string const & path) = 0;

	virtual void * Load(const std::string & fname, size_t & length) = 0;
	virtual void * fastLoad
		(const std::string & fname, size_t & length, bool & fast) = 0;

	virtual void Write
		(std::string const & fname, void const * data, int32_t length)
		= 0;
	virtual void EnsureDirectoryExists(std::string const & dirname) = 0;
	//TODO: use this only from inside EnsureDirectoryExists()
	virtual void MakeDirectory(std::string const & dirname) = 0;

	/**
	 * Opens the given file for reading as a stream.
	 * Throws an exception if the file couldn't be opened.
	 *
	 * \return a \ref StreamRead object for the file. The caller must delete this
	 * object when done to close the file.
	 */
	virtual StreamRead * OpenStreamRead(std::string const & fname) = 0;

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
	virtual StreamWrite * OpenStreamWrite(std::string const & fname) = 0;

	virtual FileSystem &   MakeSubFileSystem(std::string const & dirname) = 0;
	virtual FileSystem & CreateSubFileSystem
		(std::string const & dirname, Type) = 0;
	virtual void Unlink(std::string const &) = 0;
	virtual void Rename(std::string const &, std::string const &) = 0;

	static FileSystem & Create(std::string const & root)
	throw (FileType_error, FileNotFound_error, FileAccessDenied_error);

	///Retrieve the filesystem root's name == the mountpoint inside a
	///LayeredFileSystem
	virtual std::string getBasename() = 0;

	// basic path/filename manipulation
	std::string fixCrossFile(std::string);
	std::string getWorkingDirectory() const;
	std::string FS_CanonicalizeName(std::string path) const;
	bool pathIsAbsolute(std::string const & path) const;
	static char const * FS_Filename(char const *);
	static char const * FS_Filename(char const *, char const * & extension);
	static std::string GetHomedir();

	virtual unsigned long DiskSpace() = 0;

protected:
	///To get a filesystem, use the Create methods
	FileSystem();

	///How to address the fs' topmost component (e.g. "" on Unix, "D:" on win32)
	///\warning This is should \e not contain filesep!
	std::string m_root;

	///Character used to separate filename components
	char m_filesep;

#ifdef WIN32
private:
	static bool check_writeable_for_data(char const * path);
#endif

};

#endif
