/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "zip_filesystem.h"

#include "filesystem_exceptions.h"
#include "zip_exceptions.h"
#include "wexception.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>

/**
 * Initialize the real file-system
 */
ZipFilesystem::ZipFilesystem(std::string const & zipfile)
:
m_state      (STATE_IDLE),
m_zipfile    (0),
m_unzipfile  (0),
m_zipfilename(zipfile),
m_basename   (FS_Filename(zipfile.c_str()))
{
	// TODO: check OS permissions on whether the file is writable
}

/**
 * Cleanup code
 */
ZipFilesystem::~ZipFilesystem()
{
	m_Close();
}

/**
 * Return true if this directory is writable.
 */
bool ZipFilesystem::IsWritable() const {
	return true; // should be checked in constructor
}

/**
 * Returns the number of files found, and stores the filenames (without the
 * pathname) in the results. There doesn't seem to be an even remotely
 * cross-platform way of doing this
 */
int32_t ZipFilesystem::FindFiles
	(std::string const & path_in,
#ifndef NDEBUG
	 std::string const & pattern,
#else
	 std::string const &,
#endif
	 filenameset_t     * const results,
	 uint32_t)
{
	m_OpenUnzip();

	// If you need something else, implement a proper glob() here. I do not want
	// to! -- Holger
	assert(pattern == "*");

	std::string path;
	assert(path_in.size()); //  prevent invalid read below
	if (path_in[0] != '/')
		path = '/';
	path += path_in;
	if (path[path.size() - 1] != '/')
		path += '/';

	unzCloseCurrentFile(m_unzipfile);
	unzGoToFirstFile(m_unzipfile);

	unz_file_info file_info;
	char filename_inzip[256];
	for (;;) {
		unzGetCurrentFileInfo
			(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
			 0, 0, 0, 0);

		std::string complete_filename = &filename_inzip[m_basename.size()];
		std::string filename = FS_Filename(complete_filename.c_str());
		std::string filepath =
			complete_filename.substr
				(0, complete_filename.size() - filename.size());

		//  FIXME Something strange is going on with regard to the leading slash!
		//  FIXME This is just an ugly workaround and does not solve the real
		//  FIXME problem (which remains undiscovered)
		if (('/' + path == filepath || path == filepath) && filename.size())
			results->insert(complete_filename);

		if (unzGoToNextFile(m_unzipfile) == UNZ_END_OF_LIST_OF_FILE)
			break;
	}

	return results->size();
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
bool ZipFilesystem::FileExists(std::string const & path_in) {
	try {
		m_OpenUnzip(); //  FIXME check return value
	} catch (...) {
		return false;
	}

	unzGoToFirstFile(m_unzipfile);
	unz_file_info file_info;
	char filename_inzip[256];
	memset(filename_inzip, ' ', 256);

	std::string path;
	assert(path_in.size()); //  prevent invalid read below
	if (path_in[0] != '/')
		path = '/';
	path += path_in;

	for (;;) {
		unzGetCurrentFileInfo
			(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
			 0, 0, 0, 0);

		std::string complete_filename = &filename_inzip[m_basename.size()];
		if (complete_filename[complete_filename.size() - 1] == '/')
			complete_filename.resize(complete_filename.size() - 1);

		if (path == complete_filename)
			return true;

		if (unzGoToNextFile(m_unzipfile) == UNZ_END_OF_LIST_OF_FILE)
			break;
	}

	return false;
}

/**
 * Returns true if the given file is a directory, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
bool ZipFilesystem::IsDirectory(std::string const & path) {

	if (!FileExists(path))
		return false;

	unz_file_info file_info;
	char filename_inzip[256];

	unzGetCurrentFileInfo
		(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
		 0, 0, 0, 0);

	return filename_inzip[strlen(filename_inzip) - 1] == '/';
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem & ZipFilesystem::MakeSubFileSystem(std::string const & path) {
	m_OpenUnzip();

	assert(FileExists(path));
	assert(IsDirectory(path));

	m_Close();

	ZipFilesystem & newfs = *new ZipFilesystem(m_zipfilename);
	newfs.m_basename = m_basename + '/' + path;

	return newfs;
}

/**
 * Make a new Subfilesystem in this
 * \todo type should be recognized automatically, \see Filesystem::Create
 * \throw ZipOperation_error
 */
FileSystem & ZipFilesystem::CreateSubFileSystem
	(std::string const & path, Type const type)
{
	assert(!FileExists(path));

	if (type != FileSystem::DIR)
		throw ZipOperation_error
			("ZipFilesystem::CreateSubFileSystem",
			 path, m_zipfilename,
			 "can not create ZipFilesystem inside another ZipFilesystem");

	EnsureDirectoryExists(path);

	m_Close();

	ZipFilesystem & newfs = *new ZipFilesystem(*this);
	newfs.m_basename = m_basename + '/' + path;

	return newfs;
}
/**
 * Remove a number of files
 * \throw ZipOperation_error
 */
void ZipFilesystem::Unlink(std::string const & filename) {
	throw ZipOperation_error
		("ZipFilesystem::Unlink",
		 filename,
		 m_zipfilename,
		 "unlinking is not supported inside zipfiles");
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void ZipFilesystem::EnsureDirectoryExists(std::string const & dirname) {
	if (FileExists(dirname) && IsDirectory(dirname))
		return;

	MakeDirectory(dirname);
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Pleas note, this function does not honor parents,
 * MakeDirectory("onedir/otherdir/onemoredir") will fail
 * if either ondir or otherdir is missing
 */
void ZipFilesystem::MakeDirectory(std::string const & dirname) {
	m_OpenZip();

	zip_fileinfo zi;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate     = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_file = m_basename;
	complete_file            += '/';
	complete_file            += dirname;
	assert(dirname.size());
	if (complete_file[complete_file.size() - 1] != '/')
		complete_file += '/';

	switch
		(zipOpenNewFileInZip3
		 	(m_zipfile,
		 	 complete_file.c_str(),
		 	 &zi,
		 	 0, 0, 0, 0, 0 /* comment*/,
		 	 Z_DEFLATED,
		 	 Z_BEST_COMPRESSION,
		 	 0,
		 	 -MAX_WBITS,
		 	 DEF_MEM_LEVEL,
		 	 Z_DEFAULT_STRATEGY,
		 	 0,
		 	 0))
	{
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw File_error
			("ZipFilesystem::MakeDirectory", complete_file, strerror(errno));
	default:
		throw File_error
			("ZipFilesystem::MakeDirectory", complete_file);
	}

	zipCloseFileInZip(m_zipfile);
}

/**
 * Read the given file into alloced memory; called by FileRead::Open.
 * \throw FileNotFound_error if the file couldn't be opened.
 */
void * ZipFilesystem::Load(const std::string & fname, size_t & length) {
	if (!FileExists(fname.c_str()) || IsDirectory(fname.c_str()))
		throw ZipOperation_error
			("ZipFilesystem::Load",
			 fname,
			 m_zipfilename,
			 "could not open file from zipfile");

	char buffer[1024];
	size_t totallen = 0;
	unzOpenCurrentFile(m_unzipfile);
	for (;;) {
		const int32_t len =  unzReadCurrentFile(m_unzipfile, buffer, sizeof(buffer));
		if (len == 0)
			break;
		if (len < 0) {
			char buf[200];
			snprintf(buf, sizeof(buf), "read error %i", len);
			throw ZipOperation_error
				("ZipFilesystem::Load",
				 fname,
				 m_zipfilename,
				 buf);
		}

		totallen += len;
	}
	unzCloseCurrentFile(m_unzipfile);

	void * const result = malloc(totallen);
	unzOpenCurrentFile(m_unzipfile);
	unzReadCurrentFile(m_unzipfile, result, totallen);
	unzCloseCurrentFile(m_unzipfile);

	length = totallen;

	return result;
}

/**
 * Write the given block of memory to the repository.
 * Throws an exception if it fails.
 */
void ZipFilesystem::Write
	(std::string const & fname, void const * const data, int32_t const length)
{
	m_OpenZip();

	zip_fileinfo zi;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate     = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	//  create file
	std::string const complete_file = m_basename + '/' + fname;
	switch
		(zipOpenNewFileInZip3
		 	(m_zipfile, complete_file.c_str(), &zi,
		 	 0, 0, 0, 0, 0 /* comment*/,
		 	 Z_DEFLATED,
		 	 Z_BEST_COMPRESSION, 0,
		 	 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		 	 0, 0))
	{
	case ZIP_OK:
		break;
	default:
		throw ZipOperation_error
			("ZipFilesystem::Write", complete_file, m_zipfilename);
	}

	switch (zipWriteInFileInZip (m_zipfile, data, length)) {
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw File_error
			("ZipFilesystem::Write", complete_file, strerror(errno));
	default:
		throw File_error
			("ZipFilesystem::Write", complete_file);
	}

	zipCloseFileInZip(m_zipfile);
}

StreamRead  * ZipFilesystem::OpenStreamRead (std::string const & fname) {
	throw wexception
		("OpenStreamRead(%s) not yet supported in ZipFilesystem",
		 fname.c_str());
}

StreamWrite * ZipFilesystem::OpenStreamWrite(std::string const & fname) {
	throw wexception
		("OpenStreamWrite(%s) not yet supported in ZipFilesystem",
		 fname.c_str());
}


/**
 * Private Functions below
 */
void ZipFilesystem::m_Close() {
	if (m_state == STATE_ZIPPING)
		zipClose(m_zipfile, 0);
	else if (m_state == STATE_UNZIPPPING)
		unzClose(m_unzipfile);

	m_state = STATE_IDLE;
}

/**
 * Open a zipfile for compressing
 */
void ZipFilesystem::m_OpenZip() {
	if (m_state == STATE_ZIPPING)
		return;

	m_Close();

	m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_ADDINZIP);
	if (!m_zipfile) {
		//  Couldn't open for append, so create new.
		m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_CREATE);
	}

	m_state = STATE_ZIPPING;
}

/**
 * Open a zipfile for extraction
 * \throw FileType_error
 */
void ZipFilesystem::m_OpenUnzip() {
	if (m_state == STATE_UNZIPPPING)
		return;

	m_Close();

	m_unzipfile = unzOpen(m_zipfilename.c_str());
	if (!m_unzipfile)
		throw FileType_error
			("ZipFilesystem::m_OpenUnzip", m_zipfilename, "not a .zip file");

	m_state = STATE_UNZIPPPING;
}

void ZipFilesystem::Rename(std::string const &, std::string const &) {
	throw wexception("rename inside zip FS is not implemented yet");
}
