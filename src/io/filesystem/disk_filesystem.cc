/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include "io/filesystem/disk_filesystem.h"

#include <cassert>
#include <cerrno>

#include <boost/algorithm/string/replace.hpp>
#include <sys/stat.h>
#ifdef _WIN32
#include <dos.h>
#include <windows.h>
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#define S_ISDIR(x) ((x&_S_IFDIR)?1:0)
#endif
#else  // not _WIN32
#include <fcntl.h>
#include <glob.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

struct FileSystemPath: public std::string
{
	bool exists_;
	bool is_directory_;

	FileSystemPath(const std::string & path)
	: std::string(path)
	{
		struct stat st;

		exists_ = (stat(c_str(), &st) != -1);
		is_directory_ = exists_ && S_ISDIR(st.st_mode);
	}
};

/**
 * Initialize the real file-system
 */
RealFSImpl::RealFSImpl(const std::string & Directory)
: directory_(Directory)
{
	// TODO(unknown): check OS permissions on whether the directory is writable!
	root_ = canonicalize_name(Directory);
}


/**
 * SHOULD return true if this directory is writable.
 */
bool RealFSImpl::is_writable() const {
	// Should be checked here (ondisk state can change)
	return true;
}

/// returns true, if the file is writeable
bool RealFSImpl::file_is_writeable(const std::string & path) {
	std::string fullname;
	fullname = canonicalize_name(path);

	// we call fopen with "a" == append to be sure nothing gets overwritten
	FILE * const f = fopen(fullname.c_str(), "a");
	if (!f)
		return false;
	fclose(f);
	return true;
}


std::set<std::string> RealFSImpl::list_directory(const std::string & path)
{
#ifdef _WIN32
	std::string buf;
	struct _finddata_t c_file;
	long hFile;

	if (path.size())
		buf = directory_ + '\\' + path + "\\*";
	else
		buf = directory_ + "\\*";

	std::set<std::string> results;

	hFile = _findfirst(buf.c_str(), &c_file);
	if (hFile == -1)
		return results;

	std::string realpath = path;

	if (!realpath.empty()) {
		realpath.append("\\");
	}
	do {
		if ((strcmp(c_file.name, ".") == 0) || (strcmp(c_file.name, "..") == 0)) {
			continue;
		}
		const std::string filename = canonicalize_name(realpath + c_file.name);
		std::string result = filename.substr(root_.size() + 1);

		// Paths should not contain any windows line separators.
		boost::replace_all(result, "\\", "/");
		results.insert(result);
	} while (_findnext(hFile, &c_file) == 0);

	_findclose(hFile);

	return results;
#else
	std::string buf;
	glob_t gl;
	int32_t ofs;

	if (path.size()) {
		if (is_path_absolute(path)) {
			buf = path + "/*";
			ofs = 0;
		} else {
			buf = directory_ + '/' + path + "/*";
			ofs = directory_.length() + 1;
		}
	} else {
		buf = directory_ + "/*";
		ofs = directory_.length() + 1;
	}
	std::set<std::string> results;

	if (glob(buf.c_str(), 0, nullptr, &gl))
		return results;

	for (size_t i = 0; i < gl.gl_pathc; ++i) {
		const std::string filename(canonicalize_name(&gl.gl_pathv[i][ofs]));
		results.insert(filename.substr(root_.size() + 1));
	}

	globfree(&gl);

	return results;
#endif
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid (obviously, because the file
 * \e can't exist then)
 */
// TODO(unknown): Can this be rewritten to just using exceptions? Should it?
bool RealFSImpl::file_exists(const std::string & path) {
	return FileSystemPath(canonicalize_name(path)).exists_;
}

/**
 * Returns true if the given file is a directory, and false if it isn't.
 * Also returns false if the pathname is invalid (obviously, because the file
 * \e can't exist then)
 */
bool RealFSImpl::is_directory(const std::string & path) {
	return FileSystemPath(canonicalize_name(path)).is_directory_;
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem * RealFSImpl::make_sub_file_system(const std::string & path) {
	FileSystemPath fspath(canonicalize_name(path));

	if (!fspath.exists_) {
		throw wexception("RealFSImpl: unable to create sub filesystem, path does not exist for '%s'"
							  " in directory '%s'",
							  fspath.c_str(), directory_.c_str());
	}

	if (fspath.is_directory_)
		return new RealFSImpl   (fspath);
	else
		return new ZipFilesystem(fspath);
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem * RealFSImpl::create_sub_file_system(const std::string & path, Type const fs)
{
	FileSystemPath fspath(canonicalize_name(path));
	if (fspath.exists_)
		throw wexception
			("path '%s'' already exists in directory '%s', can not create a filesystem from it",
			 path.c_str(), directory_.c_str());

	if (fs == FileSystem::DIR) {
		ensure_directory_exists(path);
		return new RealFSImpl(fspath);
	} else
		return new ZipFilesystem(fspath);
}

/**
 * Remove a number of files
 */
void RealFSImpl::fs_unlink(const std::string & file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_)
		return;

	if (fspath.is_directory_)
		unlink_directory(file);
	else
		unlink_file(file);
}

/**
 * Remove a single directory or file
 */
void RealFSImpl::unlink_file(const std::string & file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_) {
		throw wexception("RealFSImpl: unable to unlink file, path does not exist for '%s' in directory '%s'",
							  fspath.c_str(), directory_.c_str());
	}
	if (fspath.is_directory_) {
		throw wexception("RealFSImpl: unable to unlink file, path '%s' in directory '%s' is a directory",
							  fspath.c_str(), directory_.c_str());
	}

#ifndef _WIN32
	unlink(fspath.c_str());
#else
	DeleteFile(fspath.c_str());
#endif
}

/**
 * Recursively remove a directory
 */
void RealFSImpl::unlink_directory(const std::string & file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_) {
		throw wexception("RealFSImpl: unable to unlink directory, path does not exist for '%s'"
							  " in directory '%s'",
							  fspath.c_str(), directory_.c_str());
	}
	if (!fspath.is_directory_) {
		throw wexception("RealFSImpl: unable to unlink directoy, path '%s' in directory '%s'"
							  " is not a directory", fspath.c_str(), directory_.c_str());
	}

	FilenameSet files = list_directory(file);
	for
		(FilenameSet::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		std::string filename = fs_filename(pname->c_str());
		if (filename == "..")
			continue;
		if (filename == ".")
			continue;

		if (is_directory(*pname))
			unlink_directory(*pname);
		else
			unlink_file(*pname);
	}

	// NOTE: this might fail if this directory contains CVS dir,
	// so no error checking here
#ifndef _WIN32
	rmdir(fspath.c_str());
#else
	if (!RemoveDirectory(fspath.c_str()))
		throw wexception("'%s' could not be deleted in directory '%s'.", fspath.c_str(), directory_.c_str());
#endif
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void RealFSImpl::ensure_directory_exists(const std::string & dirname)
{
	try {
		std::string::size_type it = 0;
		while (it < dirname.size()) {
			it = dirname.find(file_separator(), it);

			FileSystemPath fspath(canonicalize_name(dirname.substr(0, it)));
			if (fspath.exists_ && !fspath.is_directory_)
				throw wexception
					("'%s' in directory '%s' exists and is not a directory",
					 dirname.substr(0, it).c_str(), directory_.c_str());
			if (!fspath.exists_)
				make_directory(dirname.substr(0, it));

			if (it == std::string::npos)
				break;
			++it;
		}
	} catch (const std::exception & e) {
		throw wexception
			("RealFSImpl::ensure_directory_exists(%s) in directory '%s': %s",
			 dirname.c_str(), directory_.c_str(), e.what());
	}
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Pleas note, this function does not honor parents,
 * make_directory("onedir/otherdir/onemoredir") will fail
 * if either onedir or otherdir is missing
 */
void RealFSImpl::make_directory(const std::string & dirname) {
	FileSystemPath fspath(canonicalize_name(dirname));
	if (fspath.exists_)
		throw wexception
			("a file/directory with the name '%s' already exists in directory '%s'",
			 dirname.c_str(), directory_.c_str());

	if
#ifdef _WIN32
		(!CreateDirectory(fspath.c_str(), NULL))
#else
		(mkdir(fspath.c_str(), 0x1FF) == -1)
#endif
		throw DirectoryCannotCreateError
			("RealFSImpl::make_directory",
			 dirname,
			 strerror(errno));
}

/**
 * Read the given file into alloced memory; called by FileRead::open.
 * Throws an exception if the file couldn't be opened.
 */
void * RealFSImpl::load(const std::string & fname, size_t & length) {
	const std::string fullname = canonicalize_name(fname);
	if (is_directory(fullname)) {
		throw FileError("RealFSImpl::load", fullname.c_str());
	}

	FILE * file = nullptr;
	void * data = nullptr;

	try {
		file = fopen(fullname.c_str(), "rb");
		if (!file)
			throw FileError("RealFSImpl::load", fullname.c_str());

		// determine the size of the file (rather quirky, but it doesn't require
		// potentially unportable functions)
		fseek(file, 0, SEEK_END);
		size_t size;
		{
			const int32_t ftell_pos = ftell(file);
			if (ftell_pos < 0)
				throw wexception
					("RealFSImpl::load: error when loading \"%s\" (\"%s\"): file "
					 "size calculation yielded negative value %i",
					 fname.c_str(), fullname.c_str(), ftell_pos);
			size = ftell_pos;
		}
		fseek(file, 0, SEEK_SET);

		// allocate a buffer and read the entire file into it
		data = malloc(size + 1); //  TODO(unknown): memory leak!
		int result = fread(data, size, 1, file);
		if (size && (result != 1)) {
			throw wexception
				("RealFSImpl::load: read failed for %s (%s) with size %" PRIuS "",
				 fname.c_str(), fullname.c_str(), size);
		}
		static_cast<int8_t *>(data)[size] = 0;

		fclose(file);
		file = nullptr;

		length = size;
	} catch (...) {
		if (file) {
			fclose(file);
		}
		free(data);
		throw;
	}

	return data;
}

/**
 * Write the given block of memory to the repository.
 * if \arg append is true and a file of name \arg fname is already existing the data will be appended to
 * that file.
 * Throws an exception if it fails.
 */
void RealFSImpl::write(const std::string & fname, void const * const data, int32_t const length, bool append)
{
	std::string fullname;

	fullname = canonicalize_name(fname);

	FILE * const f = fopen(fullname.c_str(), append ? "a" : "wb");
	if (!f)
		throw wexception
			("could not open %s (%s) for writing", fname.c_str(), fullname.c_str());

	size_t const c = fwrite(data, length, 1, f);
	fclose(f);

	if (length && c != 1) // data might be 0 blocks long
		throw wexception
			("Write to %s (%s) failed", fname.c_str(), fullname.c_str());
}

// rename a file or directory
void RealFSImpl::fs_rename
	(const std::string & old_name, const std::string & new_name)
{
	const std::string fullname1 = canonicalize_name(old_name);
	const std::string fullname2 = canonicalize_name(new_name);
	if (rename(fullname1.c_str(), fullname2.c_str()) != 0)
		throw wexception("DiskFileSystem: unable to rename %s to %s: %s",
							fullname1.c_str(),
							fullname2.c_str(),
							strerror(errno));
}


/*****************************************************************************

Implementation of OpenStreamRead

*****************************************************************************/

namespace {

struct RealFSStreamRead : public StreamRead {
	RealFSStreamRead(const std::string & fname)
		: file_(fopen(fname.c_str(), "rb"))
	{
		if (!file_)
			throw wexception("could not open %s for reading", fname.c_str());
	}

	~RealFSStreamRead()
	{
		fclose(file_);
	}

	size_t data(void * read_data, size_t const bufsize) override {
		return fread(read_data, 1, bufsize, file_);
	}

	bool end_of_file() const override
	{
		return feof(file_);
	}

private:
	FILE * file_;
};

}

StreamRead * RealFSImpl::open_stream_read(const std::string & fname) {
	const std::string fullname = canonicalize_name(fname);

	return new RealFSStreamRead(fullname);
}


/*****************************************************************************

Implementation of OpenStreamWrite

*****************************************************************************/

namespace {

struct RealFSStreamWrite : public StreamWrite {
	RealFSStreamWrite(const std::string & fname)
		: filename_(fname)
	{
		file_ = fopen(fname.c_str(), "wb");
		if (!file_)
			throw wexception("could not open %s for writing", fname.c_str());
	}

	~RealFSStreamWrite() {fclose(file_);}

	void data(const void * const write_data, const size_t size) override
	{
		size_t ret = fwrite(write_data, 1, size, file_);

		if (ret != size)
			throw wexception("Write to %s failed", filename_.c_str());
	}

	void flush() override
	{
		fflush(file_);
	}

private:
	std::string filename_;
	FILE * file_;
};

}

StreamWrite * RealFSImpl::open_stream_write(const std::string & fname) {
	const std::string fullname = canonicalize_name(fname);

	return new RealFSStreamWrite(fullname);
}

unsigned long long RealFSImpl::disk_space() {
#ifdef _WIN32
	ULARGE_INTEGER freeavailable;
	return
		GetDiskFreeSpaceEx
			(canonicalize_name(directory_).c_str(), &freeavailable, 0, 0)
		?
		// If more than 2G free space report that much
		freeavailable.HighPart ? std::numeric_limits<unsigned long>::max() :
		freeavailable.LowPart : 0;
#else
	struct statvfs svfs;
	if (statvfs(canonicalize_name(directory_).c_str(), &svfs) != -1) {
		return static_cast<unsigned long long>(svfs.f_bsize) * svfs.f_bavail;
	}
#endif
	return 0; //  can not check disk space
}
