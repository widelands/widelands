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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_IO_FILESYSTEM_DISK_FILESYSTEM_H
#define WL_IO_FILESYSTEM_DISK_FILESYSTEM_H

#include <cstring>
#include <string>

#include "io/filesystem/filesystem.h"

// TODO(unknown): const correctness
class RealFSImpl : public FileSystem {
public:
	RealFSImpl(const std::string & Directory);

	std::set<std::string> ListDirectory(const std::string& path) override;

	bool IsWritable() const override;
	bool FileIsWriteable(const std::string & path);
	bool FileExists (const std::string & path) override;
	bool IsDirectory(const std::string & path) override;
	void EnsureDirectoryExists(const std::string & dirname) override;
	void MakeDirectory        (const std::string & dirname) override;

	void * Load(const std::string & fname, size_t & length) override;


	void write(const std::string & fname, void const * data, int32_t length, bool append);
	void write(const std::string & fname, void const * data, int32_t length) override
		{write(fname, data, length, false);}

	StreamRead  * OpenStreamRead (const std::string & fname) override;
	StreamWrite * OpenStreamWrite(const std::string & fname) override;

	FileSystem * MakeSubFileSystem(const std::string & dirname) override;
	FileSystem * CreateSubFileSystem(const std::string & dirname, Type) override;
	void Unlink(const std::string & file) override;
	void Rename(const std::string & old_name, const std::string & new_name) override;

	std::string getBasename() override {return m_directory;}
	unsigned long long DiskSpace() override;

private:
	void m_unlink_directory(const std::string & file);
	void m_unlink_file     (const std::string & file);

	std::string m_directory;
};

#endif  // end of include guard: WL_IO_FILESYSTEM_DISK_FILESYSTEM_H
