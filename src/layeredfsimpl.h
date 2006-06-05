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

#ifndef LAYEREDFSIMPL_H
#define LAYEREDFSIMPL_H

#include "layeredfilesystem.h"

class LayeredFSImpl : public LayeredFileSystem {
	public:
		LayeredFSImpl();
		virtual ~LayeredFSImpl();

		virtual bool IsWritable();

		virtual void AddFileSystem(FileSystem *fs);

		virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results, int depth); // Overwritten from LayeredFileSystem
		virtual int FindFiles(std::string path, std::string pattern, filenameset_t *results); // overwritten from FileSystem

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
		typedef std::vector<FileSystem*>::reverse_iterator FileSystem_rit;

		std::vector<FileSystem*> m_filesystems;
};

#endif
