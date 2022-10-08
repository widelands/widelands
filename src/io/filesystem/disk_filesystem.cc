/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "io/filesystem/disk_filesystem.h"

#include <cerrno>
#ifdef _WIN32
#ifdef _MSC_VER
#include <cstdio>
#else
#include <cstdint>
#endif
#endif
#include <cstdlib>
#include <cstring>
#include <limits>

#include <sys/stat.h>
#ifdef _WIN32
#include <dos.h>
#include <windows.h>
#ifdef _MSC_VER
#include <corecrt_io.h>
#define S_ISDIR(x) ((x & _S_IFDIR) ? 1 : 0)
#endif
#else  // not _WIN32
#include <fcntl.h>
#include <glob.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "base/macros.h"
#include "base/string.h"
#include "base/wexception.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_filesystem.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

struct FileSystemPath : public std::string {
	bool exists_;
	bool is_directory_;

	explicit FileSystemPath(const std::string& path) : std::string(path) {
		struct stat st;

		exists_ = (stat(c_str(), &st) != -1);
		is_directory_ = exists_ && S_ISDIR(st.st_mode);
	}
};

/**
 * Initialize the real file-system
 */
RealFSImpl::RealFSImpl(const std::string& Directory) : directory_(Directory) {
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

FilenameSet RealFSImpl::list_directory(const std::string& path) const {
#ifdef _WIN32
	std::string buf;
	struct _finddata_t c_file;
	intptr_t hFile;

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
		replace_all(result, "\\", "/");
		results.insert(result);
	} while (_findnext(hFile, &c_file) == 0);

	_findclose(hFile);

	return results;
#else
	std::string buf;
	glob_t gl;
	int32_t ofs;

	if (!path.empty()) {
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

	if (glob(buf.c_str(), 0, nullptr, &gl) != 0) {
		return results;
	}

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
bool RealFSImpl::file_exists(const std::string& path) const {
	return FileSystemPath(canonicalize_name(path)).exists_;
}

/**
 * Returns true if the given file is a directory, and false if it isn't.
 * Also returns false if the pathname is invalid (obviously, because the file
 * \e can't exist then)
 */
bool RealFSImpl::is_directory(const std::string& path) const {
	return FileSystemPath(canonicalize_name(path)).is_directory_;
}

/**
 * Make a sub filesystem out of this filesystem
 */
FileSystem* RealFSImpl::make_sub_file_system(const std::string& path) {
	FileSystemPath fspath(canonicalize_name(path));

	if (!fspath.exists_) {
		throw FileNotFoundError("RealFSImpl::make_sub_file_system", fspath);
	}

	if (fspath.is_directory_) {
		return new RealFSImpl(fspath);
	}
	return new ZipFilesystem(fspath);
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem* RealFSImpl::create_sub_file_system(const std::string& path, Type const fs) {
	FileSystemPath fspath(canonicalize_name(path));
	if (fspath.exists_) {
		throw FileError("RealFSImpl::create_sub_file_system", fspath,
		                "path already exists, cannot create new filesystem from it");
	}
	if (fs == FileSystem::DIR) {
		ensure_directory_exists(path);
		return new RealFSImpl(fspath);
	}
	return new ZipFilesystem(fspath);
}

/**
 * Remove a number of files
 */
void RealFSImpl::fs_unlink(const std::string& file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_) {
		return;
	}

	if (fspath.is_directory_) {
		unlink_directory(file);
	} else {
		unlink_file(file);
	}
}

/**
 * Remove a single directory or file
 */
void RealFSImpl::unlink_file(const std::string& file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_) {
		throw FileNotFoundError("RealFSImpl::unlink_file", fspath);
	}
	if (fspath.is_directory_) {
		throw FileTypeError("RealFSImpl::unlink_file", fspath, "path is a directory");
	}
#ifndef _WIN32
	if (unlink(fspath.c_str()) == -1) {
		throw FileError("RealFSImpl::unlink_file", fspath, strerror(errno));
	}
#else
	// Note: We could possibly replace this with _unlink()
	//       which would then also work with errno instead of GetLastError(),
	//       but I am not sure if this is available (and works properly)
	//       on all windows platforms or only with Visual Studio.
	if (!DeleteFile(fspath.c_str())) {
		throw FileError(
		   "RealFSImpl::unlink_file", fspath,
		   std::string("file error (Windows error code ") + std::to_string(GetLastError()) + ")");
	}
// TODO(Arty): generate proper system message from GetLastError() via FormatMessage
#endif
}

/**
 * Recursively remove a directory
 */
void RealFSImpl::unlink_directory(const std::string& file) {
	FileSystemPath fspath(canonicalize_name(file));
	if (!fspath.exists_) {
		throw FileNotFoundError("RealFSImpl::unlink_directory", fspath);
	}
	if (!fspath.is_directory_) {
		throw FileTypeError("RealFSImpl::unlink_directory", fspath, "path is not a directory");
	}

	FilenameSet files = list_directory(file);
	for (const std::string& unlinkme : files) {
		std::string filename = fs_filename(unlinkme.c_str());
		if (filename == "..") {
			continue;
		}
		if (filename == ".") {
			continue;
		}

		if (is_directory(unlinkme)) {
			unlink_directory(unlinkme);
		} else {
			unlink_file(unlinkme);
		}
	}

// NOTE: this might fail if this directory contains CVS dir
#ifndef _WIN32
	if (rmdir(fspath.c_str()) == -1) {
		throw FileError("RealFSImpl::unlink_directory", fspath, strerror(errno));
	}
#else
	// Note: We could possibly replace this with _rmdir()
	//       which would then also work with errno instead of GetLastError(),
	//       but I am not sure if this is available (and works properly)
	//       on all windows platforms or only with Visual Studio.
	if (!RemoveDirectory(fspath.c_str())) {
		throw FileError(
		   "RealFSImpl::unlink_directory", fspath,
		   std::string("file error (Windows error code ") + std::to_string(GetLastError()) + ")");
	}
// TODO(Arty): generate proper system message from GetLastError() via FormatMessage
#endif
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void RealFSImpl::ensure_directory_exists(const std::string& dirname) {
#ifdef _WIN32
	std::string clean_dirname = dirname;
	// Make sure we always use "/" for splitting the directory, because
	// directory names might be hardcoded in C++ or come from the file system.
	// Calling canonicalize_name will take care of this working for all file systems.
	replace_all(clean_dirname, "\\", "/");
#else
	const std::string& clean_dirname = dirname;
#endif

	std::string::size_type it = 0;
	while (it < clean_dirname.size()) {
		it = clean_dirname.find('/', it);

		FileSystemPath fspath(canonicalize_name(clean_dirname.substr(0, it)));
		if (fspath.exists_ && !fspath.is_directory_) {
			throw FileTypeError(
			   "RealFSImpl::ensure_directory_exists", fspath, "path is not a directory");
		}
		if (!fspath.exists_) {
			make_directory(clean_dirname.substr(0, it));
		}

		if (it == std::string::npos) {
			break;
		}
		++it;
	}
}

/**
 * Create this directory, throw an error if it already exists or
 * if a file is in the way or if the creation fails.
 *
 * Please note, this function does not honor parents,
 * make_directory("onedir/otherdir/onemoredir") will fail
 * if either onedir or otherdir is missing
 */
void RealFSImpl::make_directory(const std::string& dirname) {
	FileSystemPath fspath(canonicalize_name(dirname));
	if (fspath.exists_) {
		throw FileError(
		   "RealFSImpl::make_directory", fspath, "a file or directory with that name already exists");
	}

#ifndef _WIN32
	if (mkdir(fspath.c_str(), 0x1FF) == -1) {
		throw FileError("RealFSImpl::make_directory", fspath, strerror(errno));
	}
#else
	// Note: We could possibly replace this with _mkdir()
	//       which would then also work with errno instead of GetLastError(),
	//       but I am not sure if this is available (and works properly)
	//       on all windows platforms or only with Visual Studio.
	if (!CreateDirectory(fspath.c_str(), NULL)) {
		throw FileError(
		   "RealFSImpl::make_directory", fspath,
		   std::string("file error (Windows error code ") + std::to_string(GetLastError()) + ")");
	}
// TODO(Arty): generate proper system message from GetLastError() via FormatMessage
#endif
}

/**
 * Read the given file into allocated memory; called by FileRead::open.
 * Throws an exception if the file couldn't be opened.
 */
void* RealFSImpl::load(const std::string& fname, size_t& length) {
	const std::string fullname = canonicalize_name(fname);
	if (is_directory(fullname)) {
		throw FileTypeError("RealFSImpl::load", fullname, "path is a directory");
	}

	FILE* file = nullptr;
	void* data = nullptr;

	try {
		file = fopen(fullname.c_str(), "rb");
		if (file == nullptr) {
			throw FileError("RealFSImpl::load", fullname, "could not open file for reading");
		}

		// determine the size of the file (rather quirky, but it doesn't require
		// potentially unportable functions)
		fseek(file, 0, SEEK_END);
		size_t size;
		{
			const int32_t ftell_pos = ftell(file);
			if (ftell_pos < 0) {
				throw wexception("RealFSImpl::load: error when loading \"%s\" (\"%s\"): file "
				                 "size calculation yielded negative value %i",
				                 fname.c_str(), fullname.c_str(), ftell_pos);
			}
			size = ftell_pos;
		}
		fseek(file, 0, SEEK_SET);

		// allocate a buffer and read the entire file into it
		data = malloc(size + 1);  //  TODO(unknown): memory leak!
		if (data == nullptr) {
			throw wexception(
			   "RealFSImpl::load: memory allocation failed for reading file %s (%s) with size %" PRIuS
			   "",
			   fname.c_str(), fullname.c_str(), size);
		}
		int result = fread(data, size, 1, file);
		if ((size != 0u) && (result != 1)) {
			throw wexception("RealFSImpl::load: read failed for %s (%s) with size %" PRIuS "",
			                 fname.c_str(), fullname.c_str(), size);
		}
		static_cast<int8_t*>(data)[size] = 0;

		try {
			fclose(file);
			file = nullptr;
		} catch (...) {
			file = nullptr;
			throw;
		}

		length = size;
	} catch (...) {
		if (file != nullptr) {
			fclose(file);
		}
		if (data != nullptr) {
			free(data);
		}
		throw;
	}

	return data;
}

/**
 * Write the given block of memory to the repository.
 * if \arg append is true and a file of name \arg fname is already existing the data will be
 * appended to
 * that file.
 * Throws an exception if it fails.
 */
void RealFSImpl::write(const std::string& fname,
                       void const* const data,
                       size_t const length,
                       bool append) {
	std::string fullname;

	fullname = canonicalize_name(fname);

	FILE* const f = fopen(fullname.c_str(), append ? "a" : "wb");
	if (f == nullptr) {
		throw FileError("RealFSImpl::write", fullname, "could not open file for writing");
	}

	size_t const c = fwrite(data, length, 1, f);
	fclose(f);

	if ((length != 0u) && c != 1) {  // data might be 0 blocks long
		throw wexception("Write to %s (%s) failed", fname.c_str(), fullname.c_str());
	}
}

// rename a file or directory
void RealFSImpl::fs_rename(const std::string& old_name, const std::string& new_name) {
	const std::string fullname1 = canonicalize_name(old_name);
	const std::string fullname2 = canonicalize_name(new_name);
	if (rename(fullname1.c_str(), fullname2.c_str()) != 0) {
		throw FileError(
		   "RealFSImpl::fs_rename", fullname1,
		   std::string("unable to rename file to ") + fullname2 + ", " + strerror(errno));
	}
}

/*****************************************************************************

Implementation of OpenStreamRead

*****************************************************************************/

namespace {

struct RealFSStreamRead : public StreamRead {
	explicit RealFSStreamRead(const std::string& fname) : file_(fopen(fname.c_str(), "rb")) {
		if (file_ == nullptr) {
			throw FileError(
			   "RealFSStreamRead::RealFSStreamRead", fname, "could not open file for reading");
		}
	}

	~RealFSStreamRead() override {
		fclose(file_);
	}

	size_t data(void* read_data, size_t const bufsize) override {
		return fread(read_data, 1, bufsize, file_);
	}

	bool end_of_file() const override {
		return feof(file_) != 0;
	}

private:
	FILE* file_;
};
}  // namespace

StreamRead* RealFSImpl::open_stream_read(const std::string& fname) {
	const std::string fullname = canonicalize_name(fname);

	return new RealFSStreamRead(fullname);
}

/*****************************************************************************

Implementation of OpenStreamWrite

*****************************************************************************/

namespace {

struct RealFSStreamWrite : public StreamWrite {
	explicit RealFSStreamWrite(const std::string& fname) : filename_(fname) {
		file_ = fopen(fname.c_str(), "wb");
		if (file_ == nullptr) {
			throw FileError(
			   "RealFSStreamWrite::RealFSStreamWrite", fname, "could not open file for writing");
		}
	}

	~RealFSStreamWrite() override {
		fclose(file_);
	}

	void data(const void* const write_data, const size_t size) override {
		size_t ret = fwrite(write_data, 1, size, file_);

		if (ret != size) {
			throw wexception("Write to %s failed", filename_.c_str());
		}
	}

	void flush() override {
		fflush(file_);
	}

private:
	std::string filename_;
	FILE* file_;
};
}  // namespace

StreamWrite* RealFSImpl::open_stream_write(const std::string& fname) {
	const std::string fullname = canonicalize_name(fname);

	return new RealFSStreamWrite(fullname);
}

// Disk space is system dependent anyway, so we allow unspecific long long
unsigned long long RealFSImpl::disk_space() {  // NOLINT
#ifdef _WIN32
	ULARGE_INTEGER freeavailable;

	return GetDiskFreeSpaceEx(root_.c_str(), &freeavailable, 0, 0) ?
             // If more than 2G free space report that much
             freeavailable.HighPart ? std::numeric_limits<unsigned long>::max() :  // NOLINT
                                      freeavailable.LowPart :
             0;
#else
	struct statvfs svfs;
	if (statvfs(root_.c_str(), &svfs) != -1) {
		return static_cast<unsigned long long>(svfs.f_bsize) * svfs.f_bavail;  // NOLINT
	}
#endif
	return 0;  //  can not check disk space
}
