/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "io/filesystem/zip_filesystem.h"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <memory>

#include "base/string.h"
#include "base/wexception.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/zip_exceptions.h"
#include "io/streamread.h"
#include "io/streamwrite.h"

ZipFilesystem::ZipFile::ZipFile(const std::string& zipfile)
   : state_(State::kIdle),
     path_(zipfile),
     basename_(fs_filename(zipfile.c_str())),
     write_handle_(nullptr),
     read_handle_(nullptr) {
}

ZipFilesystem::ZipFile::~ZipFile() {
	close();
}

void ZipFilesystem::ZipFile::close() {
	if (state_ == State::kZipping) {
		zipClose(write_handle_, nullptr);
	} else if (state_ == State::kUnzipping) {
		unzClose(read_handle_);
	}
	state_ = State::kIdle;
}

void ZipFilesystem::ZipFile::open_for_zip() {
	if (state_ == State::kZipping) {
		return;
	}

	close();

	write_handle_ = zipOpen(path_.c_str(), APPEND_STATUS_ADDINZIP);
	if (write_handle_ == nullptr) {
		//  Couldn't open for append, so create new.
		write_handle_ = zipOpen(path_.c_str(), APPEND_STATUS_CREATE);
	}

	state_ = State::kZipping;
}

void ZipFilesystem::ZipFile::open_for_unzip() {
	if (state_ == State::kUnzipping) {
		return;
	}

	close();

	read_handle_ = unzOpen(path_.c_str());
	if (read_handle_ == nullptr) {
		throw FileTypeError("ZipFilesystem::open_for_unzip", path_, "not a .zip file");
	}

	std::string first_entry;
	size_t longest_prefix = 0;
	unz_file_info file_info;
	char filename_inzip[256];
	for (;;) {
		unzGetCurrentFileInfo(
		   read_handle_, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);
		if (first_entry.empty()) {
			first_entry = filename_inzip;
			longest_prefix = first_entry.size();
		} else {
			const std::string entry = filename_inzip;
			size_t pos = 0;
			while (pos < longest_prefix && pos < entry.size() && first_entry[pos] == entry[pos]) {
				++pos;
			}
			longest_prefix = pos;
		}

		if (unzGoToNextFile(read_handle_) == UNZ_END_OF_LIST_OF_FILE) {
			break;
		}
	}
	common_prefix_ = first_entry.substr(0, longest_prefix);

	state_ = State::kUnzipping;
}

std::string ZipFilesystem::ZipFile::strip_basename(const std::string& filename) {
	return filename.substr(common_prefix_.size());
}

const zipFile& ZipFilesystem::ZipFile::write_handle() {
	open_for_zip();
	return write_handle_;
}

const unzFile& ZipFilesystem::ZipFile::read_handle() {
	open_for_unzip();
	return read_handle_;
}

const std::string& ZipFilesystem::ZipFile::path() const {
	return path_;
}

/**
 * Initialize the real file-system
 */
ZipFilesystem::ZipFilesystem(const std::string& zipfile) : zip_file_(new ZipFile(zipfile)) {
	// TODO(unknown): check OS permissions on whether the file is writable
}

ZipFilesystem::ZipFilesystem(const std::shared_ptr<ZipFile>& shared_data,
                             const std::string& basedir_in_zip_file)
   : zip_file_(shared_data), basedir_in_zip_file_(basedir_in_zip_file) {
}

/**
 * Return true if this directory is writable.
 */
bool ZipFilesystem::is_writable() const {
	return true;  // should be checked in constructor
}

/**
 * Returns the number of files found, and stores the filenames (without the
 * pathname) in the results. There doesn't seem to be an even remotely
 * cross-platform way of doing this
 */
FilenameSet ZipFilesystem::list_directory(const std::string& path_in) const {
	assert(!path_in.empty());  //  prevent invalid read below

	std::string path = basedir_in_zip_file_;
	if (*path_in.begin() != '/') {
		path += "/";
	}
	path += path_in;
	if (*path.rbegin() != '/') {
		path += '/';
	}
	if (*path.begin() == '/') {
		path = path.substr(1);
	}

	unzCloseCurrentFile(zip_file_->read_handle());
	unzGoToFirstFile(zip_file_->read_handle());

	unz_file_info file_info;
	char filename_inzip[256];
	std::set<std::string> results;
	for (;;) {
		unzGetCurrentFileInfo(zip_file_->read_handle(), &file_info, filename_inzip,
		                      sizeof(filename_inzip), nullptr, 0, nullptr, 0);

		std::string complete_filename = zip_file_->strip_basename(filename_inzip);

		// Ensure that subdirectories will be listed - fs_filename can't handle trailing slashes.
		if (complete_filename.size() > 1 && strcmp(&complete_filename.back(), "/") == 0) {
			complete_filename.pop_back();
		}
		const std::string filename(fs_filename(complete_filename.c_str()));
		const std::string filepath(
		   complete_filename.substr(0, complete_filename.size() - filename.size()));

		//  TODO(unknown): Something strange is going on with regard to the leading slash!
		//  This is just an ugly workaround and does not solve the real
		//  problem (which remains undiscovered)
		if (!filename.empty() && ('/' + path == filepath || path == filepath || path.length() == 1)) {
			results.insert(complete_filename.substr(basedir_in_zip_file_.size()));
		}

		if (unzGoToNextFile(zip_file_->read_handle()) == UNZ_END_OF_LIST_OF_FILE) {
			break;
		}
	}
	return results;
}

/**
 * Returns true if the given file exists, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
bool ZipFilesystem::file_exists(const std::string& path) const {
	try {
		unzGoToFirstFile(zip_file_->read_handle());
	} catch (...) {
		// The zip file could not be opened. I guess this means 'path' does not
		// exist.
		return false;
	}
	unz_file_info file_info;
	char filename_inzip[256];
	memset(filename_inzip, ' ', 256);

	std::string path_in = basedir_in_zip_file_ + "/" + path;

	if (*path_in.begin() == '/') {
		path_in = path_in.substr(1);
	}

	assert(!path_in.empty());

	for (;;) {
		const int32_t success =
		   unzGetCurrentFileInfo(zip_file_->read_handle(), &file_info, filename_inzip,
		                         sizeof(filename_inzip), nullptr, 0, nullptr, 0);

		// Handle corrupt files
		if (success != UNZ_OK) {
			return false;
		}

		std::string complete_filename = zip_file_->strip_basename(filename_inzip);

		if (*complete_filename.rbegin() == '/') {
			complete_filename.resize(complete_filename.size() - 1);
		}
		if (path_in == complete_filename) {
			return true;
		}
		if (unzGoToNextFile(zip_file_->read_handle()) == UNZ_END_OF_LIST_OF_FILE) {
			break;
		}
	}
	return false;
}

/**
 * Returns true if the given file is a directory, and false if it doesn't.
 * Also returns false if the pathname is invalid
 */
bool ZipFilesystem::is_directory(const std::string& path) const {
	if (!file_exists(path)) {
		return false;
	}

	unz_file_info file_info;
	char filename_inzip[256];

	unzGetCurrentFileInfo(zip_file_->read_handle(), &file_info, filename_inzip,
	                      sizeof(filename_inzip), nullptr, 0, nullptr, 0);

	return filename_inzip[strlen(filename_inzip) - 1] == '/';
}

/**
 * Create a sub filesystem out of this filesystem
 */
FileSystem* ZipFilesystem::make_sub_file_system(const std::string& path) {
	if (path == ".") {
		return new ZipFilesystem(zip_file_, basedir_in_zip_file_);
	}
	if (!file_exists(path)) {
		throw wexception(
		   "ZipFilesystem::make_sub_file_system: The path '%s' does not exist in zip file '%s'.",
		   (basedir_in_zip_file_.empty() ? path : basedir_in_zip_file_ + "/" + path).c_str(),
		   zip_file_->path().c_str());
	}
	if (!is_directory(path)) {
		throw wexception(
		   "ZipFilesystem::make_sub_file_system: The path '%s' needs to be a directory in zip file "
		   "'%s'.",
		   (basedir_in_zip_file_.empty() ? path : basedir_in_zip_file_ + "/" + path).c_str(),
		   zip_file_->path().c_str());
	}

	std::string localpath = path;
	if (*localpath.begin() == '/') {
		localpath = localpath.substr(1);
	}
	return new ZipFilesystem(zip_file_, basedir_in_zip_file_ + "/" + localpath);
}

/**
 * Make a new Subfilesystem in this
 * \throw ZipOperationError
 */
// TODO(unknown): type should be recognized automatically,
// see Filesystem::create
FileSystem* ZipFilesystem::create_sub_file_system(const std::string& path, Type const type) {
	if (file_exists(path)) {
		throw wexception(
		   "ZipFilesystem::create_sub_file_system: Path '%s' already exists in zip file %s.",
		   (basedir_in_zip_file_.empty() ? path : basedir_in_zip_file_ + "/" + path).c_str(),
		   zip_file_->path().c_str());
	}

	if (type != FileSystem::DIR) {
		throw ZipOperationError("ZipFilesystem::create_sub_file_system", path, zip_file_->path(),
		                        "can not create ZipFilesystem inside another ZipFilesystem");
	}

	ensure_directory_exists(path);

	std::string localpath = path;
	if (*localpath.begin() == '/') {
		localpath = localpath.substr(1);
	}
	return new ZipFilesystem(zip_file_, basedir_in_zip_file_ + "/" + localpath);
}
/**
 * Remove a number of files
 * \throw ZipOperationError
 */
void ZipFilesystem::fs_unlink(const std::string& filename) {
	throw ZipOperationError("ZipFilesystem::unlink", filename, zip_file_->path(),
	                        "unlinking is not supported inside zipfiles");
}

/**
 * Create this directory if it doesn't exist, throws an error
 * if the dir can't be created or if a file with this name exists
 */
void ZipFilesystem::ensure_directory_exists(const std::string& dirname) {
	if (file_exists(dirname) && is_directory(dirname)) {
		return;
	}

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
void ZipFilesystem::make_directory(const std::string& dirname) {
	zip_fileinfo zi;

	set_time_info(zi.tmz_date);
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = basedir_in_zip_file_;
	complete_filename += "/";
	complete_filename += dirname;

	assert(!dirname.empty());
	if (*complete_filename.rbegin() != '/') {
		complete_filename += '/';
	}

	switch (zipOpenNewFileInZip3(zip_file_->write_handle(), complete_filename.c_str(), &zi, nullptr,
	                             0, nullptr, 0, nullptr /* comment*/, Z_DEFLATED, Z_BEST_COMPRESSION,
	                             0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0)) {
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw FileError("ZipFilesystem::make_directory", complete_filename, strerror(errno));
	default:
		throw FileError("ZipFilesystem::make_directory", complete_filename);
	}
	zipCloseFileInZip(zip_file_->write_handle());
}

void ZipFilesystem::set_time_info(tm_zip& time) {
	std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm* now = std::localtime(&timestamp);

	time.tm_sec = now->tm_sec;
	time.tm_min = now->tm_min;
	time.tm_hour = now->tm_hour;
	time.tm_mday = now->tm_mday;
	time.tm_mon = now->tm_mon;
	time.tm_year = now->tm_year;
}

/**
 * Read the given file into alloced memory; called by FileRead::open.
 * \throw FileNotFoundError if the file couldn't be opened.
 */
void* ZipFilesystem::load(const std::string& fname, size_t& length) {
	if (!file_exists(fname) || is_directory(fname)) {
		throw ZipOperationError(
		   "ZipFilesystem::load", fname, zip_file_->path(), "could not open file from zipfile");
	}

	char buffer[1024];
	size_t totallen = 0;
	unzOpenCurrentFile(zip_file_->read_handle());
	for (;;) {
		const int32_t len = unzReadCurrentFile(zip_file_->read_handle(), buffer, sizeof(buffer));
		if (len == 0) {
			break;
		}
		if (len < 0) {
			unzCloseCurrentFile(zip_file_->read_handle());
			const std::string errormessage = format("read error %i", len);
			throw ZipOperationError("ZipFilesystem::load", fname, zip_file_->path(), errormessage);
		}

		totallen += len;
	}
	unzCloseCurrentFile(zip_file_->read_handle());

	void* const result = malloc(totallen + 1);
	if (result == nullptr) {
		throw std::bad_alloc();
	}
	unzOpenCurrentFile(zip_file_->read_handle());
	unzReadCurrentFile(zip_file_->read_handle(), result, totallen);
	unzCloseCurrentFile(zip_file_->read_handle());

	static_cast<uint8_t*>(result)[totallen] = 0;
	length = totallen;

	return result;
}

/**
 * Write the given block of memory to the repository.
 * Throws an exception if it fails.
 */
void ZipFilesystem::write(const std::string& fname, void const* const data, size_t const length) {
	std::string filename = fname;
	std::replace(filename.begin(), filename.end(), '\\', '/');

	zip_fileinfo zi;
	set_time_info(zi.tmz_date);

	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = basedir_in_zip_file_ + "/" + filename;

	//  create file
	switch (zipOpenNewFileInZip3(zip_file_->write_handle(), complete_filename.c_str(), &zi, nullptr,
	                             0, nullptr, 0, nullptr /* comment*/, Z_DEFLATED, Z_BEST_COMPRESSION,
	                             0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0)) {
	case ZIP_OK:
		break;
	default:
		throw ZipOperationError("ZipFilesystem::write", complete_filename, zip_file_->path());
	}

	switch (zipWriteInFileInZip(zip_file_->write_handle(), data, length)) {
	case ZIP_OK:
		break;
	case ZIP_ERRNO:
		throw FileError("ZipFilesystem::write", complete_filename,
		                format("in path '%s'', Error", zip_file_->path(), strerror(errno)));
	default:
		throw FileError(
		   "ZipFilesystem::write", complete_filename, format("in path '%s'", zip_file_->path()));
	}

	zipCloseFileInZip(zip_file_->write_handle());
}

StreamRead* ZipFilesystem::open_stream_read(const std::string& fname) {
	if (!file_exists(fname) || is_directory(fname)) {
		throw ZipOperationError(
		   "ZipFilesystem::load", fname, zip_file_->path(), "could not open file from zipfile");
	}

	int32_t method;
	int result = unzOpenCurrentFile3(zip_file_->read_handle(), &method, nullptr, 1, nullptr);
	switch (result) {
	case ZIP_OK:
		break;
	default:
		throw ZipOperationError(
		   "ZipFilesystem: Failed to open streamwrite", fname, zip_file_->path());
	}
	return new ZipStreamRead(zip_file_);
}

StreamWrite* ZipFilesystem::open_stream_write(const std::string& fname) {
	zip_fileinfo zi;

	set_time_info(zi.tmz_date);

	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	std::string complete_filename = basedir_in_zip_file_ + "/" + fname;
	//  create file
	switch (zipOpenNewFileInZip3(zip_file_->write_handle(), complete_filename.c_str(), &zi, nullptr,
	                             0, nullptr, 0, nullptr /* comment*/, Z_DEFLATED, Z_BEST_COMPRESSION,
	                             0, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, nullptr, 0)) {
	case ZIP_OK:
		break;
	default:
		throw ZipOperationError(
		   "ZipFilesystem: Failed to open streamwrite", complete_filename, zip_file_->path());
	}
	return new ZipStreamWrite(zip_file_);
}

void ZipFilesystem::fs_rename(const std::string& /* old_name */,
                              const std::string& /* new_name */) {
	throw wexception("rename inside zip FS is not implemented yet");
}

unsigned long long ZipFilesystem::disk_space() {  // NOLINT
	return 0;
}

std::string ZipFilesystem::get_basename() {
	return zip_file_->path();
}

ZipFilesystem::ZipStreamRead::ZipStreamRead(const std::shared_ptr<ZipFile>& shared_data)
   : zip_file_(shared_data) {
}

size_t ZipFilesystem::ZipStreamRead::data(void* read_data, size_t bufsize) {
	int copied = unzReadCurrentFile(zip_file_->read_handle(), read_data, bufsize);
	if (copied < 0) {
		throw DataError("Failed to read from zip file %s", zip_file_->path().c_str());
	}
	if (copied == 0) {
		throw DataError("End of file reached while reading zip %s", zip_file_->path().c_str());
	}
	return copied;
}

bool ZipFilesystem::ZipStreamRead::end_of_file() const {
	return unzReadCurrentFile(zip_file_->read_handle(), nullptr, 1) == 0;
}

ZipFilesystem::ZipStreamWrite::ZipStreamWrite(const std::shared_ptr<ZipFile>& shared_data)
   : zip_file_(shared_data) {
}

void ZipFilesystem::ZipStreamWrite::data(const void* const write_data, const size_t size) {
	int result = zipWriteInFileInZip(zip_file_->write_handle(), write_data, size);
	switch (result) {
	case ZIP_OK:
		break;
	default:
		throw wexception("Failed to write into zipfile %s", zip_file_->path().c_str());
	}
}
