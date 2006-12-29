/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#ifndef REALFSIMPL_H
#define REALFSIMPL_H

#include "filesystem.h"
#include <string>

/// \todo const correctness
class RealFSImpl : public FileSystem {
public:
	RealFSImpl(const std::string Directory);
	~RealFSImpl();

	virtual const bool IsWritable() const;

	virtual const int FindFiles(std::string path,
	                            const std::string pattern,
	                            filenameset_t *results);

	virtual const bool FileExists(const std::string path);
	virtual const bool IsDirectory(const std::string path);
	virtual void EnsureDirectoryExists(const std::string dirname);
	virtual void MakeDirectory(const std::string dirname);

	virtual void *Load(const std::string fname, int * const length);
	virtual void Write(const std::string fname, const void * const data,
	                   const int length);

	virtual FileSystem* MakeSubFileSystem(const std::string dirname);
	virtual FileSystem* CreateSubFileSystem(const std::string dirname,
	                                        const Type);
	virtual void Unlink(const std::string file);

	virtual void listSubdirs() const;

private:
	void m_unlink_directory(const std::string file);
	void m_unlink_file(const std::string file);

	std::string m_directory;
};

#endif
