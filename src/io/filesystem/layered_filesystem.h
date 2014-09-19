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

#ifndef WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H
#define WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H

#include <memory>

#include "io/filesystem/filesystem.h"

/**
 * LayeredFileSystem is a file system which basically merges several layered
 * real directory structures into a single one. The really funny thing is that
 * those directories aren't represented as absolute paths, but as nested
 * FileSystems. Are you confused yet?
 * Ultimately, this provides us with the necessary flexibility to allow file
 * overrides on a per-user-basis, nested .zip files acting as Quake-like paks
 * and so on.
 *
 * Only the top-most writable filesystem is written to. A typical
 * stack would look like this in real-life:
 *
 * ~/.widelands/
 * /where/they/installed/widelands/  <-- this is the directory that the
 * executable is in
 *
 * $CWD  <-- the current-working directory; this is useful for debugging, when
 * the executable isn't in the root of the game-data directory
 */
class LayeredFileSystem : public FileSystem {
public:
	LayeredFileSystem();
	virtual ~LayeredFileSystem();

	// Add a new filesystem to the top of the stack. Take ownership of the given
	// filesystem.
	void AddFileSystem(FileSystem*);

	// Set the home filesystem (which is the preferred filesystem for writing
	// files). Take ownership of the given filesystem.
	void SetHomeFileSystem(FileSystem*);

	virtual void RemoveFileSystem(const FileSystem &);

	std::set<std::string> ListDirectory(const std::string& path) override;

	bool IsWritable() const override;
	bool            FileExists(const std::string & path) override;
	bool     IsDirectory      (const std::string & path) override;
	void EnsureDirectoryExists(const std::string & dirname) override;
	void   MakeDirectory      (const std::string & dirname) override;

	void * Load(const std::string & fname, size_t & length) override;
	virtual void write
		(const std::string & fname, void const * data, int32_t length) override;

	StreamRead  * OpenStreamRead (const std::string & fname) override;
	StreamWrite * OpenStreamWrite(const std::string & fname) override;

	FileSystem * MakeSubFileSystem(const std::string & dirname) override;
	FileSystem * CreateSubFileSystem(const std::string & dirname, Type) override;
	void Unlink(const std::string & file) override;
	void Rename(const std::string &, const std::string &) override;

	std::string getBasename() override {return std::string();}

	unsigned long long DiskSpace() override;

private:
	std::vector<std::unique_ptr<FileSystem>> m_filesystems;
	std::unique_ptr<FileSystem> m_home;
};

/// Access all game data files etc. through this FileSystem
extern LayeredFileSystem * g_fs;

#endif  // end of include guard: WL_IO_FILESYSTEM_LAYERED_FILESYSTEM_H
