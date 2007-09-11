/*
 * Copyright (C) 2002, 2003, 2006 by the Widelands Development Team
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

#include "types.h"

#include <set>
#include <string>
#include <stdexcept>
#include <vector>

typedef std::set<std::string> filenameset_t;

class StreamRead;
class StreamWrite;


/**
 * FileSystem is an abstract base class representing certain filesystem operations.
 * \todo const correctness
 */
struct FileSystem {
	//TODO: this should be unneccessary. Make it so.
	enum Type {
	   DIR,
	   ZIP
	};

	virtual ~FileSystem() {}

	virtual const int FindFiles(std::string path, const std::string pattern,
	                            filenameset_t *results, uint depth=0) = 0;

	virtual const bool IsWritable() const = 0;
	virtual const bool IsDirectory(std::string path) = 0;
	virtual const bool FileExists(const std::string path) = 0;

	virtual void * Load(const std::string & fname, size_t & length) = 0;
	virtual void Write(const std::string fname, const void * const data,
	                   const int length) = 0;
	virtual void EnsureDirectoryExists(const std::string dirname) = 0;
	//TODO: use this only from inside EnsureDirectoryExists()
	virtual void MakeDirectory(const std::string dirname) = 0;

	/**
	 * Opens the given file for reading as a stream.
	 * Throws an exception if the file couldn't be opened.
	 *
	 * \return a \ref StreamRead object for the file. The caller must delete this
	 * object when done to close the file.
	 */
	virtual StreamRead * OpenStreamRead
		(const std::string & fname)
		= 0;

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
	virtual StreamWrite * OpenStreamWrite
		(const std::string & fname)
		= 0;

	virtual FileSystem* MakeSubFileSystem(const std::string dirname) = 0;
	virtual FileSystem* CreateSubFileSystem(const std::string dirname,
	      const Type) = 0;
	virtual void Unlink(const std::string) = 0;
	virtual void Rename(const std::string&, const std::string&) = 0;

	static FileSystem *Create(const std::string root)
	throw(FileType_error, FileNotFound_error, FileAccessDenied_error);

	///Retrieve the filesystem root's name == the mountpoint inside a
	///LayeredFileSystem
	virtual const std::string getBasename() = 0;

	// basic path/filename manipulation
	const std::string getWorkingDirectory() const;
	const std::string getTempDirectory();
	const std::string FS_CanonicalizeName(const std::string path) const;
	static const std::string AutoExtension(const std::string filename,
	                                       const std::string extension);
	static const char *FS_StripExtension(char * const fname);
	static const char *FS_RelativePath(char *buf, const int buflen, const char *basefile, const char *filename);
	const bool pathIsAbsolute(const std::string path) const;
	const std::string AbsolutePath(const std::string path) const;
	const std::vector<std::string> FS_Tokenize(const std::string path) const;
	static const char *FS_Filename(const char* buf);
	static std::string GetHomedir();

protected:
	///To get a filesystem, use the Create methods
	FileSystem();

	///How to address the fs' topmost component (e.g. "" on Unix, "D:" on win32)
	///\warning This is should \e not contain filesep!
	std::string m_root;

	///Character used to separate filename components
	char m_filesep;
};

#endif
