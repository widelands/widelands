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

#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_exceptions.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "wexception.h"

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
m_basenamezip(FS_Filename(zipfile.c_str())),
m_basename   ()
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
std::set<std::string> ZipFilesystem::ListDirectory(const std::string& path_in) {
	m_OpenUnzip();

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
		std::string filename = FS_Filename(complete_filename.c_str());
		std::string filepath =
			complete_filename.substr
				(0, complete_filename.size() - filename.size());

		//  FIXME Something strange is going on with regard to the leading slash!
		//  FIXME This is just an ugly workaround and does not solve the real
		//  FIXME problem (which remains undiscovered)
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
bool ZipFilesystem::FileExists(const std::string & path) {
	try {
		m_OpenUnzip(); //  FIXME check return value
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
bool ZipFilesystem::IsDirectory(const std::string & path) {

	if (!FileExists(path))
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
FileSystem * ZipFilesystem::MakeSubFileSystem(const std::string & path) {
	m_OpenUnzip();

	assert(FileExists(path));
	assert(IsDirectory(path));

	m_Close();

	std::string localpath = path;

	if (*localpath.begin() == '/')
		localpath = localpath.substr(1);

	ZipFilesystem * newfs = new ZipFilesystem(m_zipfilename);
	newfs->m_basename = m_basename + "/" + localpath;

	return newfs;
}

/**
 * Make a new Subfilesystem in this
 * \todo type should be recognized automatically, \see Filesystem::Create
 * \throw ZipOperation_error
 */
FileSystem * ZipFilesystem::CreateSubFileSystem(const std::string & path, Type const type)
{
	assert(!FileExists(path));

	if (type != FileSystem::DIR)
		throw ZipOperation_error
			("ZipFilesystem::CreateSubFileSystem",
			 path, m_zipfilename,
			 "can not create ZipFilesystem inside another ZipFilesystem");

	EnsureDirectoryExists(path);

	m_Close();

	std::string localpath = path;

	if (*localpath.begin() == '/')
		localpath = localpath.substr(1);

	ZipFilesystem * newfs = new ZipFilesystem(*this);

	newfs->m_basename = m_basename + "/" + localpath;

	return newfs;
}
/**
 * Remove a number of files
 * kthrow ZipOperation_error
 */
void ZipFilesystem::Unlink(const std::string & filename) {
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
void ZipFilesystem::EnsureDirectoryExists(const std::string & dirname) {
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
void ZipFilesystem::MakeDirectory(const std::string & dirname) {
	m_OpenZip();

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
		throw File_error
			("ZipFilesystem::MakeDirectory", complete_filename, strerror(errno));
	default:
		throw File_error
			("ZipFilesystem::MakeDirectory", complete_filename);
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
		const int32_t len =
			unzReadCurrentFile(m_unzipfile, buffer, sizeof(buffer));
		if (len == 0)
			break;
		if (len < 0) {
			unzCloseCurrentFile(m_unzipfile);
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

	void * const result = malloc(totallen + 1);
	if (not result)
		throw std::bad_alloc();
	unzOpenCurrentFile(m_unzipfile);
	unzReadCurrentFile(m_unzipfile, result, totallen);
	unzCloseCurrentFile(m_unzipfile);

	static_cast<uint8_t *>(result)[totallen] = 0;
	length = totallen;

	return result;
}

// don't have a faster way to load zip files yet.
void * ZipFilesystem::fastLoad
	(const std::string & fname, size_t & length, bool & fast)
{
	fast = false;
	return Load(fname, length);
}

/**
 * Write the given block of memory to the repository.
 * Throws an exception if it fails.
 */
void ZipFilesystem::Write
	(const std::string & fname, void const * const data, int32_t const length)
{
	std::string filename = fname;
	std::replace(filename.begin(), filename.end(), '\\', '/');

	m_OpenZip();

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
		throw ZipOperation_error
			("ZipFilesystem::Write", complete_filename, m_zipfilename);
	}

	switch (zipWriteInFileInZip (m_zipfile, data, length)) {
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw File_error
			("ZipFilesystem::Write", complete_filename, strerror(errno));
	default:
		throw File_error
			("ZipFilesystem::Write", complete_filename);
	}

	zipCloseFileInZip(m_zipfile);
}

//
// Stream Read
//

ZipFilesystem::ZipStreamRead::ZipStreamRead(zipFile file, ZipFilesystem* zipfs)
: m_unzipfile(file), m_zipfs(zipfs) {}
ZipFilesystem::ZipStreamRead::~ZipStreamRead() {
	m_zipfs->m_Close();
}

size_t ZipFilesystem::ZipStreamRead::Data(void* data, size_t bufsize)
{
	int copied = unzReadCurrentFile(m_unzipfile, data, bufsize);
	if (copied < 0) {
		throw new _data_error("Failed to read from zip file");
	}
	if (copied == 0) {
		throw new _data_error("End of file reaced while reading zip");
	}
	return copied;
}
bool ZipFilesystem::ZipStreamRead::EndOfFile() const
{
	return unzReadCurrentFile(m_unzipfile, nullptr, 1) == 0;
}

StreamRead* ZipFilesystem::OpenStreamRead(const std::string& fname) {
	if (!FileExists(fname.c_str()) || IsDirectory(fname.c_str()))
		throw ZipOperation_error
			("ZipFilesystem::Load",
			 fname,
			 m_zipfilename,
			 "could not open file from zipfile");

	int32_t method;
	m_OpenUnzip();
	int result = unzOpenCurrentFile3(m_unzipfile, &method, nullptr, 1, nullptr);
	switch (result) {
		case ZIP_OK:
			break;
		default:
			throw ZipOperation_error
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
	m_zipfs->m_Close();
}

void ZipFilesystem::ZipStreamWrite::Data(const void* const data, const size_t size)
{
	int result = zipWriteInFileInZip(m_zipfile, data, size);
	switch (result) {
		case ZIP_OK:
			break;
		default:
			throw wexception("Failed to write into zipfile");
	}
}

StreamWrite * ZipFilesystem::OpenStreamWrite(const std::string & fname) {
	m_OpenZip();

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
		throw ZipOperation_error
			("ZipFilesystem: Failed to open streamwrite", complete_filename, m_zipfilename);
	}
	return new ZipStreamWrite(m_zipfile, this);
}


/**
 * Private Functions below
 */
void ZipFilesystem::m_Close() {
	if (m_state == STATE_ZIPPING)
		zipClose(m_zipfile, nullptr);
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

void ZipFilesystem::Rename(const std::string &, const std::string &) {
	throw wexception("rename inside zip FS is not implemented yet");
}

unsigned long long ZipFilesystem::DiskSpace() {
	return 0; //  FIXME
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
