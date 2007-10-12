/*
 * Copyright (C) 2002-2005, 2007 by the Widelands Development Team
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

#ifndef __S__ZIP_FILESYSTEM_H
#define __S__ZIP_FILESYSTEM_H

#include "filesystem.h"
#include "unzip.h"
#include "zip.h"

#include <string>

struct ZipFilesystem : public FileSystem {
	ZipFilesystem(const std::string);
	virtual ~ZipFilesystem();

	virtual const bool IsWritable() const;

	virtual const int32_t FindFiles(std::string path, const std::string pattern,
                                    filenameset_t *results, uint32_t depth=0);

	virtual const bool IsDirectory(std::string path);
	virtual const bool FileExists(std::string path);

	virtual void * Load(const std::string & fname, size_t & length);
	virtual void Write(const std::string fname, const void * const data,
                           const int32_t length);
	virtual void EnsureDirectoryExists(const std::string dirname);
	virtual void MakeDirectory(const std::string dirname);

	virtual StreamRead  * OpenStreamRead
		(const std::string & fname) __attribute__ ((noreturn));
	virtual StreamWrite * OpenStreamWrite
		(const std::string & fname) __attribute__ ((noreturn));

	virtual FileSystem*  MakeSubFileSystem(const std::string dirname);
	virtual FileSystem*  CreateSubFileSystem(const std::string dirname,
                                                 const Type);
	virtual void Unlink(const std::string filename) __attribute__ ((noreturn));
	virtual void Rename(const std::string&, const std::string&) __attribute__ ((noreturn));

public:
	static FileSystem *CreateFromDirectory(const std::string directory);

	virtual const std::string getBasename() {return m_zipfilename;};

private:
	void m_OpenUnzip();
	void m_OpenZip();
	void m_Close();

private:
	enum State {
		STATE_IDLE,
		STATE_ZIPPING,
		STATE_UNZIPPPING
	};

	State       m_state;
	zipFile     m_zipfile;
	unzFile     m_unzipfile;
	std::string m_zipfilename;
	std::string m_basename;

};

#endif // __S__ZIP_FILESYSTEM_H
