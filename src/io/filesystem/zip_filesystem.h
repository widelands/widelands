/*
 * Copyright (C) 2002-2005, 2007-2009, 2013 by the Widelands Development Team
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

#ifndef WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H
#define WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H

#include <cstring>
#include <string>

#include "base/macros.h"
#include "io/filesystem/filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "third_party/minizip/unzip.h"
#include "third_party/minizip/zip.h"

class ZipFilesystem : public FileSystem {
public:
	explicit ZipFilesystem(const std::string &);
	virtual ~ZipFilesystem();

	bool is_writable() const override;

	std::set<std::string> list_directory(const std::string& path) override;

	bool is_directory(const std::string & path) override;
	bool file_exists (const std::string & path) override;

	void * load(const std::string & fname, size_t & length) override;

	virtual void write
		(const std::string & fname, void const * data, int32_t length) override;
	void ensure_directory_exists(const std::string & fs_dirname) override;
	void   make_directory      (const std::string & fs_dirname) override;

	virtual StreamRead  * open_stream_read
		(const std::string & fname) override;
	virtual StreamWrite * open_stream_write
		(const std::string & fname) override;

	FileSystem * make_sub_file_system(const std::string & fs_dirname) override;
	FileSystem * create_sub_file_system(const std::string & fs_dirname, Type) override;
	void fs_unlink(const std::string & fs_filename) override;
	void fs_rename(const std::string &, const std::string &) override;

	unsigned long long disk_space() override;

	static FileSystem * create_from_directory(const std::string & directory);

	std::string get_basename() override {return m_zipfilename;}

protected:
	void m_open_unzip();
	void m_open_zip();
	void m_close();
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

	struct ZipStreamRead : StreamRead {
		explicit ZipStreamRead(zipFile file, ZipFilesystem* zipfs);
		virtual ~ZipStreamRead();
		size_t data(void* data, size_t bufsize) override;
		bool end_of_file() const override;
	private:
		zipFile m_unzipfile;
		ZipFilesystem* m_zipfs;
	};
	struct ZipStreamWrite : StreamWrite {
		explicit ZipStreamWrite(zipFile file, ZipFilesystem* zipfs);
		virtual ~ZipStreamWrite();
		void data(const void* const data, size_t size) override;
	private:
		zipFile m_zipfile;
		ZipFilesystem* m_zipfs;
	};

};



#endif  // end of include guard: WL_IO_FILESYSTEM_ZIP_FILESYSTEM_H
