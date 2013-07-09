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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef ZIP_FILESYSTEM_H
#define ZIP_FILESYSTEM_H

#include <string>
#include <cstring>

#include <minizip/unzip.h>
#include <minizip/zip.h>

#include "port.h"
#include "filesystem.h"

class ZipFilesystem : public FileSystem {
public:
	ZipFilesystem(const std::string &);
	virtual ~ZipFilesystem();

	virtual bool IsWritable() const;

	virtual int32_t FindFiles
		(const std::string & path,
		 const std::string & pattern,
		 filenameset_t     * results,
		 uint32_t            depth = 0);

	virtual bool IsDirectory(const std::string & path);
	virtual bool FileExists (const std::string & path);

	virtual void * Load(const std::string & fname, size_t & length);
	virtual void * fastLoad
		(const std::string & fname, size_t & length, bool & fast);

	virtual void Write
		(const std::string & fname, void const * data, int32_t length);
	virtual void EnsureDirectoryExists(const std::string & dirname);
	virtual void   MakeDirectory      (const std::string & dirname);

	virtual StreamRead  * OpenStreamRead
		(const std::string & fname);
	virtual StreamWrite * OpenStreamWrite
		(const std::string & fname);

	virtual FileSystem * MakeSubFileSystem(const std::string & dirname);
	virtual FileSystem * CreateSubFileSystem(const std::string & dirname, Type);
	virtual void Unlink(const std::string & filename);
	virtual void Rename(const std::string &, const std::string &);

	virtual unsigned long long DiskSpace();

	static FileSystem * CreateFromDirectory(const std::string & directory);

	virtual std::string getBasename() {return m_zipfilename;};

private:
	void m_OpenUnzip();
	void m_OpenZip();
	void m_Close();
	std::string strip_basename(std::string);

	enum State {
		STATE_IDLE,
		STATE_ZIPPING,
		STATE_UNZIPPPING
	};

	State       m_state;
	zipFile     m_zipfile;
	unzFile     m_unzipfile;
	/// if true data is in a directory named as the zipfile. This is set by
	/// strip_basename()
	bool        m_oldzip;
	std::string m_zipfilename;
	std::string m_basenamezip;
	std::string m_basename;

};

#endif
