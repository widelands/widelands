/*
 * Copyright (C) 2002-2009, 2013 by the Widelands Development Team
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

#include "io/filesystem/zip_filesystem.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <boost/format.hpp>

#include "base/wexception.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_exceptions.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

/**
 * Initialize the real file-system
 */
ZipFilesystem::ZipFilesystem(const std::string & zipfile)
:
m_state      (STATE_IDLE),
m_zipfile    (nullptr),
m_unzipfile  (nullptr),
m_oldzip     (false),
m_zipfilename(zipfile),
m_basenamezip(fs_filename(zipfile.c_str())),
m_basename   ()
{
	// TODO(unknown): check OS permissions on whether the file is writable
}

/**
 * Cleanup code
 */
ZipFilesystem::~ZipFilesystem()
{
	m_close();
}

/**
 * Return true if this directory is writable.
 */
bool ZipFilesystem::is_writable() const {
	return true; // should be checked in constructor
}

/**
 * Returns the number of files found, and stores the filenames (without the
 * pathname) in the results. There doesn't seem to be an even remotely
 * cross-platform way of doing this
 */
std::set<std::string> ZipFilesystem::list_directory(const std::string& path_in) {
	m_open_unzip();

	assert(path_in.size()); //  prevent invalid read below

	std::string path = m_basename;
	if (*path_in.begin() != '/')
		path += "/";
	path += path_in;
	if (*path.rbegin() != '/')
		path += '/';
	if (*path.begin() == '/')
		path = path.substr(1);

	unzCloseCurrentFile(m_unzipfile);
	unzGoToFirstFile(m_unzipfile);

	unz_file_info file_info;
	char filename_inzip[256];
	std::set<std::string> results;
	for (;;) {
		unzGetCurrentFileInfo
			(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
			 nullptr, 0, nullptr, 0);

		std::string complete_filename = strip_basename(filename_inzip);
		std::string filename = fs_filename(complete_filename.c_str());
		std::string filepath =
			complete_filename.substr
				(0, complete_filename.size() - filename.size());

		//  TODO(unknown): Something strange is going on with regard to the leading slash!
		//  This is just an ugly workaround and does not solve the real
		//  problem (which remains undiscovered)
		if
			(('/' + path == filepath || path == filepath || path.length() == 1)
			 && filename.size())
		results.insert(complete_filename.substr(m_basename.size()));

		if (unzGoToNextFile(m_unzipfile) == UNZ_END_OF_LIST_OF_FILE)
			break;
	}
	return results;
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
bool ZipFilesystem::file_exists(const std::string & path) {
	try {
		m_open_unzip(); //  TODO(unknown): check return value
	} catch (...) {
		return false;
	}
	unzGoToFirstFile(m_unzipfile);
	unz_file_info file_info;
	char filename_inzip[256];
	memset(filename_inzip, ' ', 256);

	std::string path_in = m_basename + "/" + path;

	if (*path_in.begin() == '/')
		path_in = path_in.substr(1);

	assert(path_in.size());

	for (;;) {
		unzGetCurrentFileInfo
			(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
			 nullptr, 0, nullptr, 0);

		std::string complete_filename = strip_basename(filename_inzip);

		if (*complete_filename.rbegin() == '/')
			complete_filename.resize(complete_filename.size() - 1);

		if (path_in == complete_filename)
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
bool ZipFilesystem::is_directory(const std::string & path) {

	if (!file_exists(path))
		return false;

	unz_file_info file_info;
	char filename_inzip[256];

	unzGetCurrentFileInfo
		(m_unzipfile, &file_info, filename_inzip, sizeof(filename_inzip),
		 nullptr, 0, nullptr, 0);

	return filename_inzip[strlen(filename_inzip) - 1] == '/';
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem * ZipFilesystem::make_sub_file_system(const std::string & path) {
	m_open_unzip();

	if (!file_exists(path)) {
		throw wexception("ZipFilesystem::make_sub_file_system: The path does not exist.");
	}
	if (!is_directory(path)) {
		throw wexception("ZipFilesystem::make_sub_file_system: The path needs to be a directory.");
	}

	m_close();

	std::string localpath = path;

	if (*localpath.begin() == '/')
		localpath = localpath.substr(1);

	ZipFilesystem * newfs = new ZipFilesystem(m_zipfilename);
	newfs->m_basename = m_basename + "/" + localpath;

	return newfs;
}

/**
 * Make a new Subfilesystem in this
 * \throw ZipOperationError
 */
// TODO(unknown): type should be recognized automatically,
// see Filesystem::create
FileSystem * ZipFilesystem::create_sub_file_system(const std::string & path, Type const type)
{
	if (file_exists(path)) {
		throw wexception("ZipFilesystem::create_sub_file_system: Sub file system already exists.");
	}

	if (type != FileSystem::DIR)
		throw ZipOperationError
			("ZipFilesystem::create_sub_file_system",
			 path, m_zipfilename,
			 "can not create ZipFilesystem inside another ZipFilesystem");

	ensure_directory_exists(path);

	m_close();

	std::string localpath = path;

	if (*localpath.begin() == '/')
		localpath = localpath.substr(1);

	ZipFilesystem * newfs = new ZipFilesystem(*this);

	newfs->m_basename = m_basename + "/" + localpath;

	return newfs;
}
/**
 * Remove a number of files
 * \throw ZipOperationError
 */
void ZipFilesystem::fs_unlink(const std::string & filename) {
	throw ZipOperationError
		("ZipFilesystem::unlink",
		 filename,
		 m_zipfilename,
		 "unlinking is not supported inside zipfiles");
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void ZipFilesystem::ensure_directory_exists(const std::string & dirname) {
	if (file_exists(dirname) && is_directory(dirname))
		return;

	make_directory(dirname);
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Pleas note, this function does not honor parents,
 * make_directory("onedir/otherdir/onemoredir") will fail
 * if either ondir or otherdir is missing
 */
void ZipFilesystem::make_directory(const std::string & dirname) {
	m_open_zip();

	zip_fileinfo zi;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate     = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = m_basename;
	complete_filename += "/";
	complete_filename += dirname;

	assert(dirname.size());
	if (*complete_filename.rbegin() != '/')
		complete_filename += '/';

	switch
		(zipOpenNewFileInZip3
		 	(m_zipfile,
			 complete_filename.c_str(),
		 	 &zi,
		 	 nullptr, 0, nullptr, 0, nullptr /* comment*/,
		 	 Z_DEFLATED,
		 	 Z_BEST_COMPRESSION,
		 	 0,
		 	 -MAX_WBITS,
		 	 DEF_MEM_LEVEL,
		 	 Z_DEFAULT_STRATEGY,
		 	 nullptr,
		 	 0))
	{
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw FileError
			("ZipFilesystem::make_directory", complete_filename, strerror(errno));
	default:
		throw FileError
			("ZipFilesystem::make_directory", complete_filename);
	}

	zipCloseFileInZip(m_zipfile);
}

/**
 * Read the given file into alloced memory; called by FileRead::open.
 * \throw FileNotFoundError if the file couldn't be opened.
 */
void * ZipFilesystem::load(const std::string & fname, size_t & length) {
	if (!file_exists(fname.c_str()) || is_directory(fname.c_str()))
		throw ZipOperationError
			("ZipFilesystem::load",
			 fname,
			 m_zipfilename,
			 "could not open file from zipfile");

	char buffer[1024];
	size_t totallen = 0;
	unzOpenCurrentFile(m_unzipfile);
	for (;;) {
		const int32_t len =
			unzReadCurrentFile(m_unzipfile, buffer, sizeof(buffer));
		if (len == 0)
			break;
		if (len < 0) {
			unzCloseCurrentFile(m_unzipfile);
			const std::string errormessage = (boost::format("read error %i") % len).str();
			throw ZipOperationError
				("ZipFilesystem::load", fname, m_zipfilename, errormessage.c_str());
		}

		totallen += len;
	}
	unzCloseCurrentFile(m_unzipfile);

	void * const result = malloc(totallen + 1);
	if (!result)
		throw std::bad_alloc();
	unzOpenCurrentFile(m_unzipfile);
	unzReadCurrentFile(m_unzipfile, result, totallen);
	unzCloseCurrentFile(m_unzipfile);

	static_cast<uint8_t *>(result)[totallen] = 0;
	length = totallen;

	return result;
}

/**
 * Write the given block of memory to the repository.
 * Throws an exception if it fails.
 */
void ZipFilesystem::write
	(const std::string & fname, void const * const data, int32_t const length)
{
	std::string filename = fname;
	std::replace(filename.begin(), filename.end(), '\\', '/');

	m_open_zip();

	zip_fileinfo zi;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate     = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = m_basename + "/" + filename;

	//  create file
	switch
		(zipOpenNewFileInZip3
			(m_zipfile, complete_filename.c_str(), &zi,
		 	 nullptr, 0, nullptr, 0, nullptr /* comment*/,
		 	 Z_DEFLATED,
		 	 Z_BEST_COMPRESSION, 0,
		 	 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		 	 nullptr, 0))
	{
	case ZIP_OK:
		break;
	default:
		throw ZipOperationError
			("ZipFilesystem::write", complete_filename, m_zipfilename);
	}

	switch (zipWriteInFileInZip (m_zipfile, data, length)) {
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw FileError
			("ZipFilesystem::write", complete_filename, strerror(errno));
	default:
		throw FileError
			("ZipFilesystem::write", complete_filename);
	}

	zipCloseFileInZip(m_zipfile);
}

//
// Stream Read
//

ZipFilesystem::ZipStreamRead::ZipStreamRead(zipFile file, ZipFilesystem* zipfs)
: m_unzipfile(file), m_zipfs(zipfs) {}
ZipFilesystem::ZipStreamRead::~ZipStreamRead() {
	m_zipfs->m_close();
}

size_t ZipFilesystem::ZipStreamRead::data(void* read_data, size_t bufsize)
{
	int copied = unzReadCurrentFile(m_unzipfile, read_data, bufsize);
	if (copied < 0) {
		throw new DataError("Failed to read from zip file");
	}
	if (copied == 0) {
		throw new DataError("End of file reaced while reading zip");
	}
	return copied;
}
bool ZipFilesystem::ZipStreamRead::end_of_file() const
{
	return unzReadCurrentFile(m_unzipfile, nullptr, 1) == 0;
}

StreamRead* ZipFilesystem::open_stream_read(const std::string& fname) {
	if (!file_exists(fname.c_str()) || is_directory(fname.c_str()))
		throw ZipOperationError
			("ZipFilesystem::load",
			 fname,
			 m_zipfilename,
			 "could not open file from zipfile");

	int32_t method;
	m_open_unzip();
	int result = unzOpenCurrentFile3(m_unzipfile, &method, nullptr, 1, nullptr);
	switch (result) {
		case ZIP_OK:
			break;
		default:
			throw ZipOperationError
				("ZipFilesystem: Failed to open streamwrite", fname, m_zipfilename);
	}
	return new ZipStreamRead(m_unzipfile, this);
}

//
// Stream write
//

ZipFilesystem::ZipStreamWrite::ZipStreamWrite(zipFile file, ZipFilesystem* zipfs)
: m_zipfile(file), m_zipfs(zipfs) {}
ZipFilesystem::ZipStreamWrite::~ZipStreamWrite()
{
	m_zipfs->m_close();
}

void ZipFilesystem::ZipStreamWrite::data(const void* const write_data, const size_t size)
{
	int result = zipWriteInFileInZip(m_zipfile, write_data, size);
	switch (result) {
		case ZIP_OK:
			break;
		default:
			throw wexception("Failed to write into zipfile");
	}
}

StreamWrite * ZipFilesystem::open_stream_write(const std::string & fname) {
	m_open_zip();

	zip_fileinfo zi;

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate     = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = m_basename + "/" + fname;
	//  create file
	switch
		(zipOpenNewFileInZip3
			(m_zipfile, complete_filename.c_str(), &zi,
		 	 nullptr, 0, nullptr, 0, nullptr /* comment*/,
		 	 Z_DEFLATED,
		 	 Z_BEST_COMPRESSION, 0,
		 	 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		 	 nullptr, 0))
	{
	case ZIP_OK:
		break;
	default:
		throw ZipOperationError
			("ZipFilesystem: Failed to open streamwrite", complete_filename, m_zipfilename);
	}
	return new ZipStreamWrite(m_zipfile, this);
}


/**
 * Private Functions below
 */
void ZipFilesystem::m_close() {
	if (m_state == STATE_ZIPPING)
		zipClose(m_zipfile, nullptr);
	else if (m_state == STATE_UNZIPPPING)
		unzClose(m_unzipfile);

	m_state = STATE_IDLE;
}

/**
 * Open a zipfile for compressing
 */
void ZipFilesystem::m_open_zip() {
	if (m_state == STATE_ZIPPING)
		return;

	m_close();

	m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_ADDINZIP);
	if (!m_zipfile) {
		//  Couldn't open for append, so create new.
		m_zipfile = zipOpen(m_zipfilename.c_str(), APPEND_STATUS_CREATE);
	}

	m_state = STATE_ZIPPING;
}

/**
 * Open a zipfile for extraction
 * \throw FileTypeError
 */
void ZipFilesystem::m_open_unzip() {
	if (m_state == STATE_UNZIPPPING)
		return;

	m_close();

	m_unzipfile = unzOpen(m_zipfilename.c_str());
	if (!m_unzipfile)
		throw FileTypeError
			("ZipFilesystem::m_open_unzip", m_zipfilename, "not a .zip file");

	m_state = STATE_UNZIPPPING;
}

void ZipFilesystem::fs_rename(const std::string &, const std::string &) {
	throw wexception("rename inside zip FS is not implemented yet");
}

unsigned long long ZipFilesystem::disk_space() {
	return 0;
}

std::string ZipFilesystem::strip_basename(std::string filename)
{

	if (filename.compare(0, m_basenamezip.length(), m_basenamezip) == 0)
	{
		// filename contains the name of the zip file as first element. This means
		// this is an old zip file where all data were in a directory named as the
		// file inside the zip file.
		// return the filename without the first element
		m_oldzip = true;
		return filename.substr(m_basenamezip.length() + 1);
	}

	// seems to be a new zipfile without directory or a old zipfile was renamed
	m_oldzip = false;
	if (*filename.begin() == '/')
		return filename.substr(1);
	return filename;
}
