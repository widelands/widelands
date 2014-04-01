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

#ifndef LAYERED_FILESYSTEM_H
#define LAYERED_FILESYSTEM_H

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

	virtual void    AddFileSystem(FileSystem &);
	virtual void    SetHomeFileSystem(FileSystem &);

	virtual void RemoveFileSystem(const FileSystem &);

	virtual std::set<std::string> ListDirectory(const std::string& path) override;

	virtual bool IsWritable() const override;
	virtual bool            FileExists(const std::string & path) override;
	virtual bool     IsDirectory      (const std::string & path) override;
	virtual void EnsureDirectoryExists(const std::string & dirname) override;
	virtual void   MakeDirectory      (const std::string & dirname) override;

	virtual void * Load(const std::string & fname, size_t & length) override;
	virtual void * fastLoad
		(const std::string & fname, size_t & length, bool & fast) override;
	virtual void Write
		(const std::string & fname, void const * data, int32_t length) override;

	virtual StreamRead  * OpenStreamRead (const std::string & fname) override;
	virtual StreamWrite * OpenStreamWrite(const std::string & fname) override;

	virtual FileSystem * MakeSubFileSystem(const std::string & dirname) override;
	virtual FileSystem * CreateSubFileSystem(const std::string & dirname, Type) override;
	virtual void Unlink(const std::string & file) override;
	virtual void Rename(const std::string &, const std::string &) override;

	virtual std::string getBasename() override {return std::string();};

	bool FindConflictingVersionFile(FileSystem &);
	bool FindMatchingVersionFile(FileSystem &);

	void PutRightVersionOnTop();

	virtual unsigned long long DiskSpace() override;

private:
	bool m_read_version_from_version_file(FileSystem &, std::string *);

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
class FileSystemLayer {
public:
	FileSystemLayer(FileSystem & fs) : m_fs(fs) {g_fs->AddFileSystem(fs);}
	~FileSystemLayer() {g_fs->RemoveFileSystem(m_fs);}
private:
	const FileSystem & m_fs;
};

#endif
