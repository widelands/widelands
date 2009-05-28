/*
 * Copyright (C) 2002-2005, 2007-2009 by the Widelands Development Team
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

#ifndef ZIP_FILESYSTEM_H
#define ZIP_FILESYSTEM_H

#include "filesystem.h"
#include "unzip.h"
#include "zip.h"

#include <string>
#include <cstring>

struct ZipFilesystem : public FileSystem {
	ZipFilesystem(std::string const &);
	virtual ~ZipFilesystem();

	virtual bool IsWritable() const;

	virtual int32_t FindFiles
		(std::string const & path,
		 std::string const & pattern,
		 filenameset_t     * results,
		 uint32_t            depth = 0);

	virtual bool IsDirectory(std::string const & path);
	virtual bool FileExists (std::string const & path);

	virtual void * Load(std::string const & fname, size_t & length);
	virtual void Write
		(std::string const & fname, void const * data, int32_t length);
	virtual void EnsureDirectoryExists(std::string const & dirname);
	virtual void   MakeDirectory      (std::string const & dirname);

	virtual StreamRead  * OpenStreamRead
		(const std::string & fname) __attribute__ ((noreturn));
	virtual StreamWrite * OpenStreamWrite
		(const std::string & fname) __attribute__ ((noreturn));

	virtual FileSystem &   MakeSubFileSystem(std::string const & dirname);
	virtual FileSystem & CreateSubFileSystem
		(std::string const & dirname, Type);
	virtual void Unlink(std::string const & filename)
		__attribute__ ((noreturn));
	virtual void Rename(std::string const &, std::string const &)
		__attribute__ ((noreturn));

	virtual unsigned long DiskSpace();

public:
	static FileSystem * CreateFromDirectory(std::string const & directory);

	virtual std::string getBasename() {return m_zipfilename;};

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

#endif
