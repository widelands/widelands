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

#include <sys/stat.h>

#ifdef _WIN32
#include <dos.h>
#include <windows.h>
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#define S_ISDIR(x) ((x&_S_IFDIR)?1:0)
#endif // _MSC_VER
#else  // not _WIN32
#include <fcntl.h>
#include <glob.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#endif

#include "compile_diagnostics.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "log.h"
#include "wexception.h"

struct FileSystemPath: public std::string
{
	bool m_exists;
	bool m_isDirectory;

	FileSystemPath(const std::string & path)
	: std::string(path)
	{
		struct stat st;

		m_exists = (stat(c_str(), &st) != -1);
		m_isDirectory = m_exists and S_ISDIR(st.st_mode);
	}
};

/**
 * Initialize the real file-system
 */
RealFSImpl::RealFSImpl(const std::string & Directory)
: m_directory(Directory)
{
	// TODO: check OS permissions on whether the directory is writable!
	m_root = FS_CanonicalizeName(Directory);
}


/**
 * SHOULD return true if this directory is writable.
 */
bool RealFSImpl::IsWritable() const {
	// Should be checked here (ondisk state can change)
	return true;
}

/// returns true, if the file is writeable
bool RealFSImpl::FileIsWriteable(const std::string & path) {
	std::string fullname;
	fullname = FS_CanonicalizeName(path);

	// we call fopen with "a" == append to be sure nothing gets overwritten
	FILE * const f = fopen(fullname.c_str(), "a");
	if (!f)
		return false;
	fclose(f);
	return true;
}


std::set<std::string> RealFSImpl::ListDirectory(const std::string & path)
{
#ifdef _WIN32
	std::string buf;
	struct _finddata_t c_file;
	long hFile;

	if (path.size())
		buf = m_directory + '\\' + path + "\\*";
	else
		buf = m_directory + "\\*";

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
		const std::string filename = FS_CanonicalizeName(realpath + c_file.name);
		const std::string result = filename.substr(m_root.size() + 1);
		results.insert(result);
	} while (_findnext(hFile, &c_file) == 0);

	_findclose(hFile);

	return results;
#else
	std::string buf;
	glob_t gl;
	int32_t ofs;

	if (path.size()) {
		if (pathIsAbsolute(path)) {
			buf = path + "/*";
			ofs = 0;
		} else {
			buf = m_directory + '/' + path + "/*";
			ofs = m_directory.length() + 1;
		}
	} else {
		buf = m_directory + "/*";
		ofs = m_directory.length() + 1;
	}
	std::set<std::string> results;

	if (glob(buf.c_str(), 0, nullptr, &gl))
		return results;

	for (size_t i = 0; i < gl.gl_pathc; ++i) {
		const std::string filename(FS_CanonicalizeName(&gl.gl_pathv[i][ofs]));
		results.insert(filename.substr(m_root.size() + 1));
	}

	globfree(&gl);

	return results;
#endif
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid (obviously, because the file
 * \e can't exist then)
 * \todo Can this be rewritten to just using exceptions? Should it?
 */
bool RealFSImpl::FileExists(const std::string & path) {
	return FileSystemPath(FS_CanonicalizeName(path)).m_exists;
}

/**
 * Returns true if the given file is a directory, and false if it isn't.
 * Also returns false if the pathname is invalid (obviously, because the file
 * \e can't exist then)
 */
bool RealFSImpl::IsDirectory(const std::string & path) {
	return FileSystemPath(FS_CanonicalizeName(path)).m_isDirectory;
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem * RealFSImpl::MakeSubFileSystem(const std::string & path) {
	FileSystemPath fspath(FS_CanonicalizeName(path));
	assert(fspath.m_exists); //TODO: throw an exception instead

	if (fspath.m_isDirectory)
		return new RealFSImpl   (fspath);
	else
		return new ZipFilesystem(fspath);
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem * RealFSImpl::CreateSubFileSystem(const std::string & path, Type const fs)
{
	FileSystemPath fspath(FS_CanonicalizeName(path));
	if (fspath.m_exists)
		throw wexception
			("path %s already exists, can not create a filesystem from it",
			 path.c_str());

	if (fs == FileSystem::DIR) {
		EnsureDirectoryExists(path);
		return new RealFSImpl(fspath);
	} else
		return new ZipFilesystem(fspath);
}

/**
 * Remove a number of files
 */
void RealFSImpl::Unlink(const std::string & file) {
	FileSystemPath fspath(FS_CanonicalizeName(file));
	if (!fspath.m_exists)
		return;

	if (fspath.m_isDirectory)
		m_unlink_directory(file);
	else
		m_unlink_file(file);
}

/**
 * Remove a single directory or file
 */
void RealFSImpl::m_unlink_file(const std::string & file) {
	FileSystemPath fspath(FS_CanonicalizeName(file));
	assert(fspath.m_exists);  //TODO: throw an exception instead
	assert(!fspath.m_isDirectory); //TODO: throw an exception instead

#ifndef _WIN32
	unlink(fspath.c_str());
#else
	DeleteFile(fspath.c_str());
#endif
}

/**
 * Recursively remove a directory
 */
void RealFSImpl::m_unlink_directory(const std::string & file) {
	FileSystemPath fspath(FS_CanonicalizeName(file));
	assert(fspath.m_exists);  //TODO: throw an exception instead
	assert(fspath.m_isDirectory);  //TODO: throw an exception instead

	filenameset_t files = ListDirectory(file);
	for
		(filenameset_t::iterator pname = files.begin();
		 pname != files.end();
		 ++pname)
	{
		std::string filename = FS_Filename(pname->c_str());
		if (filename == "..")
			continue;
		if (filename == ".")
			continue;

		if (IsDirectory(*pname))
			m_unlink_directory(*pname);
		else
			m_unlink_file(*pname);
	}

	// NOTE: this might fail if this directory contains CVS dir,
	// so no error checking here
#ifndef _WIN32
	rmdir(fspath.c_str());
#else
	RemoveDirectory(fspath.c_str());
#endif
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void RealFSImpl::EnsureDirectoryExists(const std::string & dirname)
{
	try {
		std::string::size_type it = 0;
		while (it < dirname.size()) {
			it = dirname.find(m_filesep, it);

			FileSystemPath fspath(FS_CanonicalizeName(dirname.substr(0, it)));
			if (fspath.m_exists and !fspath.m_isDirectory)
				throw wexception
					("%s exists and is not a directory",
					 dirname.substr(0, it).c_str());
			if (!fspath.m_exists)
				MakeDirectory(dirname.substr(0, it));

			if (it == std::string::npos)
				break;
			++it;
		}
	} catch (const std::exception & e) {
		throw wexception
			("RealFSImpl::EnsureDirectoryExists(%s): %s",
			 dirname.c_str(), e.what());
	}
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Pleas note, this function does not honor parents,
 * MakeDirectory("onedir/otherdir/onemoredir") will fail
 * if either onedir or otherdir is missing
 */
void RealFSImpl::MakeDirectory(const std::string & dirname) {
	FileSystemPath fspath(FS_CanonicalizeName(dirname));
	if (fspath.m_exists)
		throw wexception
			("a file with the name \"%s\" already exists", dirname.c_str());

	if
		(mkdir
#ifdef _WIN32
		 	(fspath.c_str())
#else
		 	(fspath.c_str(), 0x1FF)
#endif
		 ==
		 -1)
		throw DirectoryCannotCreate_error
			("RealFSImpl::MakeDirectory",
			 dirname,
			 strerror(errno));
}

/**
 * Read the given file into alloced memory; called by FileRead::Open.
 * Throws an exception if the file couldn't be opened.
 */
void * RealFSImpl::Load(const std::string & fname, size_t & length) {
	const std::string fullname = FS_CanonicalizeName(fname);

	FILE * file = nullptr;
	void * data = nullptr;

	try {
		//debug info
		//printf("------------------------------------------\n");
		//printf("RealFSImpl::Load():\n");
		//printf("     fname       = %s\n", fname.c_str());
		//printf("     m_directory = %s\n", m_directory.c_str());
		//printf("     fullname    = %s\n", fullname.c_str());
		//printf("------------------------------------------\n");

		file = fopen(fullname.c_str(), "rb");
		if (not file)
			throw File_error("RealFSImpl::Load", fullname.c_str());

		// determine the size of the file (rather quirky, but it doesn't require
		// potentially unportable functions)
		fseek(file, 0, SEEK_END);
		size_t size;
		{
			const int32_t ftell_pos = ftell(file);
			if (ftell_pos < 0)
				throw wexception
					("RealFSImpl::Load: error when loading \"%s\" (\"%s\"): file "
					 "size calculation yielded negative value %i",
					 fname.c_str(), fullname.c_str(), ftell_pos);
			size = ftell_pos;
		}
		fseek(file, 0, SEEK_SET);

		// allocate a buffer and read the entire file into it
		data = malloc(size + 1); //  FIXME memory leak!
		int result = fread(data, size, 1, file);
		if (size and (result != 1)) {
			assert(false);
			throw wexception
				("RealFSImpl::Load: read failed for %s (%s) with size %" PRIuS "",
				 fname.c_str(), fullname.c_str(), size);
		}
		static_cast<int8_t *>(data)[size] = 0;

		fclose(file);
		file = nullptr;

		length = size;
	} catch (...) {
		if (file)
			fclose(file);
		free(data);
		throw;
	}

	return data;
}

void * RealFSImpl::fastLoad
	(const std::string & fname, size_t & length, bool & fast)
{
#ifdef _WIN32
	fast = false;
	return Load(fname, length);
#else
	const std::string fullname = FS_CanonicalizeName(fname);
	int file = 0;
	void * data = nullptr;

#ifdef __APPLE__
	file = open(fullname.c_str(), O_RDONLY);
#elif defined (__FreeBSD__) || defined (__OpenBSD__)
	file = open(fullname.c_str(), O_RDONLY);
#else
	file = open(fullname.c_str(), O_RDONLY|O_NOATIME);
#endif
	length = lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);

	data = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, file, 0);

	//if mmap doesn't work for some strange reason try the old way
GCC_DIAG_OFF("-Wold-style-cast")
	if (data == MAP_FAILED) {
GCC_DIAG_ON("-Wold-style-cast")
		return Load(fname, length);
	}

	fast = true;

	assert(data);
GCC_DIAG_OFF("-Wold-style-cast")
	assert(data != MAP_FAILED);
GCC_DIAG_ON("-Wold-style-cast")

	close(file);

	return data;
#endif
}

/**
 * Write the given block of memory to the repository.
 * if \arg append is true and a file of name \arg fname is already existing the data will be appended to
 * that file.
 * Throws an exception if it fails.
 */
void RealFSImpl::Write(const std::string & fname, void const * const data, int32_t const length, bool append)
{
	std::string fullname;

	fullname = FS_CanonicalizeName(fname);

	FILE * const f = fopen(fullname.c_str(), append ? "a" : "wb");
	if (!f)
		throw wexception
			("could not open %s (%s) for writing", fname.c_str(), fullname.c_str());

	size_t const c = fwrite(data, length, 1, f);
	fclose(f);

	if (length and c != 1) // data might be 0 blocks long
		throw wexception
			("Write to %s (%s) failed", fname.c_str(), fullname.c_str());
}

// rename a file or directory
void RealFSImpl::Rename
	(const std::string & old_name, const std::string & new_name)
{
	const std::string fullname1 = FS_CanonicalizeName(old_name);
	const std::string fullname2 = FS_CanonicalizeName(new_name);
	rename(fullname1.c_str(), fullname2.c_str());
}


/*****************************************************************************

Implementation of OpenStreamRead

*****************************************************************************/

namespace {

struct RealFSStreamRead : public StreamRead {
	RealFSStreamRead(const std::string & fname)
		: m_file(fopen(fname.c_str(), "rb"))
	{
		if (!m_file)
			throw wexception("could not open %s for reading", fname.c_str());
	}

	~RealFSStreamRead()
	{
		fclose(m_file);
	}

	size_t Data(void * data, size_t const bufsize) override {
		return fread(data, 1, bufsize, m_file);
	}

	bool EndOfFile() const override
	{
		return feof(m_file);
	}

private:
	FILE * m_file;
};

};

StreamRead * RealFSImpl::OpenStreamRead(const std::string & fname) {
	const std::string fullname = FS_CanonicalizeName(fname);

	return new RealFSStreamRead(fullname);
}


/*****************************************************************************

Implementation of OpenStreamWrite

*****************************************************************************/

namespace {

struct RealFSStreamWrite : public StreamWrite {
	RealFSStreamWrite(const std::string & fname)
		: m_filename(fname)
	{
		m_file = fopen(fname.c_str(), "wb");
		if (!m_file)
			throw wexception("could not open %s for writing", fname.c_str());
	}

	~RealFSStreamWrite() {fclose(m_file);}

	void Data(const void * const data, const size_t size) override
	{
		size_t ret = fwrite(data, 1, size, m_file);

		if (ret != size)
			throw wexception("Write to %s failed", m_filename.c_str());
	}

	void Flush() override
	{
		fflush(m_file);
	}

private:
	std::string m_filename;
	FILE * m_file;
};

};

StreamWrite * RealFSImpl::OpenStreamWrite(const std::string & fname) {
	const std::string fullname = FS_CanonicalizeName(fname);

	return new RealFSStreamWrite(fullname);
}

unsigned long long RealFSImpl::DiskSpace() {
#ifdef _WIN32
	ULARGE_INTEGER freeavailable;
	return
		GetDiskFreeSpaceEx
			(FS_CanonicalizeName(m_directory).c_str(), &freeavailable, 0, 0)
		?
		//if more than 2G free space report that much
		freeavailable.HighPart ? std::numeric_limits<unsigned long>::max() :
		freeavailable.LowPart : 0;
#else
	struct statvfs svfs;
	if (statvfs(FS_CanonicalizeName(m_directory).c_str(), &svfs) != -1) {
		return static_cast<unsigned long long>(svfs.f_bsize) * svfs.f_bavail;
	}
#endif
	return 0; //  can not check disk space
}
