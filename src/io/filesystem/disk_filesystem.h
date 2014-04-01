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

#ifndef DISK_FILESYSTEM_H
#define DISK_FILESYSTEM_H

#include <cstring>
#include <string>

#include "io/filesystem/filesystem.h"

/// \todo const correctness
class RealFSImpl : public FileSystem {
public:
	RealFSImpl(const std::string & Directory);

	virtual std::set<std::string> ListDirectory(const std::string& path) override;

	virtual bool IsWritable() const override;
	bool FileIsWriteable(const std::string & path);
	virtual bool FileExists (const std::string & path) override;
	virtual bool IsDirectory(const std::string & path) override;
	virtual void EnsureDirectoryExists(const std::string & dirname) override;
	virtual void MakeDirectory        (const std::string & dirname) override;

	virtual void * Load(const std::string & fname, size_t & length) override;
	virtual void * fastLoad
		(const std::string & fname, size_t & length, bool & fast) override;


	void Write(const std::string & fname, void const * data, int32_t length, bool append);
	virtual void Write(const std::string & fname, void const * data, int32_t length) override
		{Write(fname, data, length, false);}

	virtual StreamRead  * OpenStreamRead (const std::string & fname) override;
	virtual StreamWrite * OpenStreamWrite(const std::string & fname) override;

	virtual FileSystem * MakeSubFileSystem(const std::string & dirname) override;
	virtual FileSystem * CreateSubFileSystem(const std::string & dirname, Type) override;
	virtual void Unlink(const std::string & file) override;
	virtual void Rename(const std::string & old_name, const std::string & new_name) override;

	virtual std::string getBasename() override {return m_directory;};
	virtual unsigned long long DiskSpace() override;

private:
	void m_unlink_directory(const std::string & file);
	void m_unlink_file     (const std::string & file);

	std::string m_directory;
};

#endif
