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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef LAYERED_FILESYSTEM_H
#define LAYERED_FILESYSTEM_H

#include "filesystem.h"

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
struct LayeredFileSystem : public FileSystem {
	LayeredFileSystem();
	virtual ~LayeredFileSystem();

	virtual void    AddFileSystem(FileSystem &);
	virtual void    SetHomeFileSystem(FileSystem &);

	virtual void RemoveFileSystem(FileSystem const &);

	virtual int32_t FindFiles
		(std::string const & path,
		 std::string const & pattern,
		 filenameset_t     * results,
		 uint32_t            depth = 0);

	virtual bool IsWritable() const;
	virtual bool            FileExists(std::string const & path);
	virtual bool     IsDirectory      (std::string const & path);
	virtual void EnsureDirectoryExists(std::string const & dirname);
	virtual void   MakeDirectory      (std::string const & dirname);

	virtual void * Load(std::string const & fname, size_t & length);
	virtual void * fastLoad
		(std::string const & fname, size_t & length, bool & fast);
	virtual void Write
		(std::string const & fname, void const * data, int32_t length);

	virtual StreamRead  * OpenStreamRead (std::string const & fname);
	virtual StreamWrite * OpenStreamWrite(std::string const & fname);

	virtual FileSystem &   MakeSubFileSystem(std::string const & dirname);
	virtual FileSystem & CreateSubFileSystem
		(std::string const & dirname, Type);
	virtual void Unlink(std::string const & file);
	virtual void Rename(std::string const &, std::string const &);

	virtual std::string getBasename() {return std::string();};

	bool FindConflictingVersionFile(FileSystem &);
	bool FindMatchingVersionFile(FileSystem &);

	void PutRightVersionOnTop();

	virtual unsigned long DiskSpace();

private:
	typedef std::vector<FileSystem *>::reverse_iterator FileSystem_rit;

	std::vector<FileSystem *> m_filesystems;
	FileSystem * m_home;
};

/// Access all game data files etc. through this FileSystem
extern LayeredFileSystem * g_fs;

/// Create an object of this type to add a new filesystem to the top of the
/// stack and make sure that it is removed when the object goes out of scope.
/// This is exception-safe, unlike calling g_fs->AddFileSystem and
/// g_fs->RemoveFileSystem directly.
struct FileSystemLayer {
	FileSystemLayer(FileSystem & fs) : m_fs(fs) {g_fs->AddFileSystem(fs);}
	~FileSystemLayer() {g_fs->RemoveFileSystem(m_fs);}
private:
	FileSystem const & m_fs;
};

#endif
