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
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

typedef std::set<std::string> filenameset_t;

/**
 * FileSystem is a base class representing certain filesystem operations.
 * \todo const correctness
 */
class FileSystem {
public:
	enum Type {
	   FS_DIR,
	   FS_ZIP
	};

	virtual ~FileSystem() { }

	virtual bool IsWritable() = 0;

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results) = 0;

	virtual bool IsDirectory(std::string path) = 0;
	virtual bool FileExists(std::string path) = 0;

	virtual void *Load(std::string fname, int *length) = 0;
	virtual void Write(std::string fname, void *data, int length) = 0;
	virtual void EnsureDirectoryExists(std::string dirname) = 0;
	virtual void MakeDirectory(std::string dirname) = 0;

	virtual FileSystem*  MakeSubFileSystem( std::string dirname ) = 0;
	virtual FileSystem*  CreateSubFileSystem( std::string dirname, Type ) = 0;
	virtual void Unlink( std::string ) = 0;

	static FileSystem *Create(std::string root)
	throw(FileType_error, FileNotFound_error, FileAccessDenied_error);

	virtual void listSubdirs()=0;

	// basic path/filename manipulation
	const std::string getWorkingDirectory();
	const std::string getTempDirectory();
	std::string FS_CanonicalizeName(std::string path);
	static char *FS_AutoExtension(char *buf, int bufsize, const char *ext);
	static char *FS_StripExtension(char *fname);
	static char *FS_RelativePath(char *buf, int buflen, const char *basefile, const char *filename);
	const bool pathIsAbsolute(const std::string path);
	const std::string AbsolutePath(const std::string path);
	std::vector<std::string> FS_Tokenize(std::string path);
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
