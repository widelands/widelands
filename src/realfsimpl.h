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

class RealFSImpl : public FileSystem {
public:
	RealFSImpl(std::string Directory);
	~RealFSImpl();

	virtual bool IsWritable();

	virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results);

	virtual bool FileExists(std::string path);
	virtual bool IsDirectory(std::string path);
	virtual void EnsureDirectoryExists(std::string dirname);
	virtual void MakeDirectory(std::string dirname);

	virtual void *Load(std::string fname, int *length);
	virtual void Write(std::string fname, void *data, int length);

	virtual FileSystem* MakeSubFileSystem( std::string dirname );
	virtual FileSystem* CreateSubFileSystem( std::string dirname, Type );
	virtual void Unlink(std::string file);

	virtual void listSubdirs();

private:
	void m_unlink_directory( std::string file );
	void m_unlink_file( std::string file );

	std::string m_directory;
};

#endif
