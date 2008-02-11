/*
 * Copyright (C) 2006-2008 by the Widelands Development Team
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

#ifndef DISK_FILESYSTEM_H
#define DISK_FILESYSTEM_H

#include "filesystem.h"

#include <string>
#include <cstring>

/// \todo const correctness
struct RealFSImpl : public FileSystem {
	RealFSImpl(std::string const & Directory);
	~RealFSImpl();

	virtual int32_t FindFiles
		(std::string const & path,
		 std::string const & pattern,
		 filenameset_t     * results,
		 uint32_t            depth = 0);

	virtual bool IsWritable() const;
	virtual bool FileExists (std::string const & path);
	virtual bool IsDirectory(std::string const & path);
	virtual void EnsureDirectoryExists(std::string const & dirname);
	virtual void MakeDirectory        (std::string const & dirname);

	virtual void * Load(const std::string & fname, size_t & length);
	virtual void Write
		(std::string const & fname, void const * data, int32_t length);

	virtual StreamRead  * OpenStreamRead (std::string const & fname);
	virtual StreamWrite * OpenStreamWrite(std::string const & fname);

	virtual FileSystem * MakeSubFileSystem(std::string const & dirname);
	virtual FileSystem * CreateSubFileSystem
		(std::string const & dirname, Type);
	virtual void Unlink(std::string const & file);
	virtual void Rename(const std::string&, const std::string&);

	virtual std::string getBasename() {return m_directory;};

private:
	void m_unlink_directory(std::string const & file);
	void m_unlink_file     (std::string const & file);

	std::string m_directory;
};

#endif
