/*
 * Copyright (C) 2002, 2006, 2008-2010 by the Widelands Development Team
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
// this originally comes from Return to the Shadows (http://www.rtts.org/)
// files.cc: provides all the OS abstraction to access files

#include "io/filesystem/filesystem.h"

#include <cassert>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <list>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#endif
#include <sys/stat.h>
#include <unistd.h>

#include "base/log.h"
#include "config.h"
#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_exceptions.h"
#include "io/filesystem/zip_filesystem.h"

#ifdef _WIN32
#define stat _stat
#endif

#ifdef _MSC_VER
#define S_ISDIR(x) ((x&_S_IFDIR)?1:0)
#define S_ISREG(x) ((x&_S_IFREG)?1:0)
#define PATH_MAX MAX_PATH
#endif

FileSystem::FileSystem()
{
	root_ = "";
}


/**
 * \param path A file or directory name
 * \return True if ref path is absolute and within this FileSystem, false otherwise
 */
bool FileSystem::is_path_absolute(const std::string & path) const {
	std::string::size_type const path_size = path  .size();
	std::string::size_type const root_size = root_.size();

	if (path_size < root_size)
		return false;

	if (path_size == root_size)
		return path == root_;

	if (path.compare(0, root_.size(), root_))
		return false;

#ifdef _WIN32
	if (path.size() >= 3 && path[1] == ':' && path[2] == '\\') //"C:\"
	{
		return true;
	}
#endif
	assert(root_size < path_size); //  Otherwise an invalid read happens below.
	if (path[root_size] != file_separator())
		return false;

	return true;
}


/**
 * Fix a path that might come from another OS.
 * This function is used to make sure that paths send via network are usable
 * on locale OS.
 */
std::string FileSystem::fix_cross_file(const std::string & path) const {
	uint32_t path_size = path.size();
	std::string fixedPath(path);
	std::string temp;
	for (uint32_t i = 0; i < path_size; ++i) {
		temp = path.at(i);
#ifdef _WIN32
		if (temp == ":")
			fixedPath.at(i) = '-';
		else if (temp == "/")
#else
		if (temp == "\\")
#endif
			fixedPath.at(i) = file_separator();
		// As a security measure, eat all:
		// * tildes
		// * double dots
		// * dots with following slash/backslash (but not a single dot - we need it in e.g. "xyz.wmf")
		// away to avoid misuse of the file transfer function.
		if (temp == "~")
			fixedPath.at(i) = '_';
		if (temp == "." && (i + 1 < path_size)) {
			std::string temp2;
			temp2 = path.at(i + 1);
			if (temp2 == "." || temp2 == "\\" || temp2 == "/")
				fixedPath.at(i) = '_';
		}
	}
	return fixedPath;
}

/**
 * \return The process' current working directory
 */
// static
std::string FileSystem::get_working_directory() {
	char cwd[PATH_MAX + 1];
	char * const result = getcwd(cwd, PATH_MAX);
	if (! result)
		throw FileError("FileSystem::get_working_directory()", "widelands", "can not run getcwd");

	return std::string(cwd);
}

// static
char FileSystem::file_separator() {
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

// TODO(unknown): Write homedir detection for non-getenv-systems
std::string FileSystem::get_homedir()
{
	std::string homedir;
#ifdef _WIN32
	// trying to get it compatible to ALL windows versions...
	// Could anybody please hit the Megasoft devs for not keeping
	// their own "standards"?
#define TRY_USE_AS_HOMEDIR(name)                                              \
   homedir = getenv(name);                                                    \
   if (homedir.size() && check_writeable_for_data(homedir.c_str()))           \
      return homedir;                                                         \

	TRY_USE_AS_HOMEDIR("USERPROFILE");
	TRY_USE_AS_HOMEDIR("HOMEPATH");
	TRY_USE_AS_HOMEDIR("HOME");
	TRY_USE_AS_HOMEDIR("APPDATA");

	log("None of the directories was useable - falling back to \".\"\n");
#else
#ifdef HAS_GETENV
	if (char const * const h = getenv("HOME"))
		homedir = h;
#endif
#endif

	if (homedir.empty()) {
		log
			("\nWARNING: either we can not detect your home directory "
			 "or you do not have one! Please contact the developers.\n\n");

		//TODO(unknown): is it really a good idea to set homedir to "." then ??

		log("Instead of your home directory, '.' will be used.\n\n");
		homedir = ".";
	}

	return homedir;
}

/**
 * Split a string into components separated by a certain character.
 *
 * \param path The path to parse
 * \param filesep The file path separator used by the native filesystem
 * \param components The output iterator to place the path nodes into
 */
template<typename Inserter>
static void fs_tokenize
	(const std::string & path, char const filesep, Inserter components)
{
	std::string::size_type pos;  //  start of token
	std::string::size_type pos2; //  next filesep character

	//extract the first path component
	if (path.find(filesep) == 0) //is this an absolute path?
		pos = 1;
	else //relative path
		pos = 0;
	pos2 = path.find(filesep, pos);
	//'current' token is now between pos and pos2

	//split path into it's components
	while (pos2 != std::string::npos) {
		if (pos != pos2)
		{
			std::string node = path.substr(pos, pos2 - pos);
			*components++ = node;
		}
		pos = pos2 + 1;
		pos2 = path.find(filesep, pos);
	}

	//extract the last component (most probably a filename)
	std::string node = path.substr(pos);
	if (!node.empty())
		*components++ = node;
}

/**
 * Transform any valid, unique pathname into a well-formed absolute path
 */
// TODO(unknown): Enable non-Unix paths
std::string FileSystem::canonicalize_name(std::string path) const {
	std::list<std::string> components;
	std::list<std::string>::iterator i;

#ifdef _WIN32
	// remove all slashes with backslashes so following can work.
	for (uint32_t j = 0; j < path.size(); ++j) {
		if (path[j] == '/')
			path[j] = '\\';
	}
#endif

	fs_tokenize(path, file_separator(), std::inserter(components, components.begin()));

	//tilde expansion
	if (!components.empty() && *components.begin() == "~") {
		components.erase(components.begin());
		fs_tokenize
			(get_homedir(),
			 file_separator(),
			 std::inserter(components, components.begin()));
	} else if (!is_path_absolute(path))
		//  make relative paths absolute (so that "../../foo" can work)
		fs_tokenize
			(root_.empty() ? get_working_directory() : root_, file_separator(),
			 std::inserter(components, components.begin()));

	//clean up the path
	for (i = components.begin(); i != components.end();) {
		char const * str = i->c_str();
		if (*str == '.') {
			++str;

			//remove single dot
			if (*str == '\0') {
				i = components.erase(i);
				continue;
			}
			//remove double dot and the preceding component (if any)
			else if (*str == '.' && *(str + 1) == '\0') {
				if (i != components.begin()) {
#ifdef _WIN32
					// On windows don't remove driveletter in this error condition
					if (--i != components.begin())
						i = components.erase(i);
					else ++i;
#else
					i = components.erase(--i);
#endif
				}
				i = components.erase(i);
				continue;
			}
		}

		++i;
	}

	std::string canonpath;
	canonpath.reserve(path.length());
#ifndef _WIN32
	for (i = components.begin(); i != components.end(); ++i) {
		canonpath.push_back('/');
		canonpath += *i;
	}
#else
	for (i = components.begin(); i != components.end(); ++i) {
		canonpath += *i;
		canonpath += '\\';
	}

	//remove trailing slash
	if (canonpath.size() > 1) canonpath.erase(canonpath.end() - 1);
#endif

	return canonpath;
}

/**
 * Returns the filename of this path, everything after the last
 * / or \  (or the whole string)
 */
const char * FileSystem::fs_filename(const char * p) {
	const char * result = p;

	while (*p != '\0') {
		if (*p == '/' || *p == '\\')
			result = p + 1;
		++p;
	}

	return result;
}

std::string FileSystem::fs_dirname(const std::string& full_path) {
	const std::string filename = fs_filename(full_path.c_str());
	return full_path.substr(0, full_path.size() - filename.size());
}

std::string FileSystem::filename_ext(const std::string & f)
{
	// Find last '.' - denotes start of extension
	size_t ext_start = f.rfind('.');

	if (std::string::npos == ext_start)
		return "";
	else
		return f.substr(ext_start);
}

std::string FileSystem::filename_without_ext(const char * const p)
{
	std::string fname(p ? FileSystem::fs_filename(p) : "");
	std::string ext(FileSystem::filename_ext(fname));
	return fname.substr(0, fname.length() - ext.length());
}

/// Create a filesystem from a zipfile or a real directory
/// \throw FileNotFoundError if root does not exist, is some kind of special
/// file, loops around (via symlinks) or is too long for the OS/filesystem.
/// \throw FileAccessDeniedError if the OS denies access (of course ;-)
/// \throw FileTypeError if root is neither a directory or regular file
// TODO(unknown): Catch FileTypeError in all users
// TODO(unknown): Check for existence before doing anything with the file/dir
// TODO(unknown): Catch FileNotFoundError in all users
// TODO(unknown): throw FileTypeError if root is not a zipfile (exception from
// ZipFilesystem)
FileSystem & FileSystem::create(const std::string & root)
{
	struct stat statinfo;

	if (stat(root.c_str(), &statinfo) == -1) {
		if
			(errno == EBADF   ||
			 errno == ENOENT  ||
			 errno == ENOTDIR ||
#ifdef ELOOP
			 errno == ELOOP   || //MinGW does not support ELOOP (yet)
#endif
			 errno == ENAMETOOLONG)
		{
			throw FileNotFoundError("FileSystem::create", root);
		}
		if (errno == EACCES)
			throw FileAccessDeniedError("FileSystem::create", root);
	}

	if (S_ISDIR(statinfo.st_mode)) {
		return *new RealFSImpl(root);
	}
	if (S_ISREG(statinfo.st_mode)) { //TODO(unknown): ensure root is a zipfile
		return *new ZipFilesystem(root);
	}

	throw FileTypeError
		("FileSystem::create", root,
		 "cannot create virtual filesystem from file or directory");
}

#ifdef _WIN32
/// hack that is unfortunately needed for windows to check whether Widelands
/// can write in the directory
bool FileSystem::check_writeable_for_data(char const * const path)
{
	RealFSImpl fs(path);

	if (fs.is_directory(".widelands"))
		return true;
	try {
		// throws an exception if not writable
		fs.ensure_directory_exists(".widelands");
		fs.fs_unlink(".widelands");
		return true;
	} catch (...) {
		log("Directory %s is not writeable - next try\n", path);
	}

	return false;
}
#endif
