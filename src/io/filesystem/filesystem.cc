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

#include <config.h>
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

#include "io/filesystem/disk_filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filesystem/zip_exceptions.h"
#include "io/filesystem/zip_filesystem.h"
#include "log.h"

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
	m_root = "";
#ifdef _WIN32
	m_filesep = '\\';
#else
	m_filesep = '/';
#endif
}


/**
 * \param path A file or directory name
 * \return True if ref path is absolute and within this FileSystem, false otherwise
 */
bool FileSystem::pathIsAbsolute(const std::string & path) const {
	std::string::size_type const path_size = path  .size();
	std::string::size_type const root_size = m_root.size();

	if (path_size < root_size)
		return false;

	if (path_size == root_size)
		return path == m_root;

	if (path.compare(0, m_root.size(), m_root))
		return false;

#ifdef _WIN32
	if (path.size() >= 3 && path[1] == ':' && path[2] == '\\') //"C:\"
	{
		return true;
	}
#endif
	assert(root_size < path_size); //  Otherwise an invalid read happens below.
	if (path[root_size] != m_filesep)
		return false;

	return true;
}


/**
 * Fix a path that might come from another OS.
 * This function is used to make sure that paths send via network are usable
 * on locale OS.
 */
std::string FileSystem::fixCrossFile(const std::string & path) const {
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
			fixedPath.at(i) = m_filesep;
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
std::string FileSystem::getWorkingDirectory() const {
	char cwd[PATH_MAX + 1];
	char * const result = getcwd(cwd, PATH_MAX);
	if (! result)
		throw File_error("FileSystem::getWorkingDirectory()", "widelands", "can not run getcwd");

	return std::string(cwd);
}


/// \todo Write homedir detection for non-getenv-systems
std::string FileSystem::GetHomedir()
{
	std::string homedir;
#ifdef _WIN32
	// trying to get it compatible to ALL windows versions...
	// Could anybody please hit the Megasoft devs for not keeping
	// their own "standards"?
#define TRY_USE_AS_HOMEDIR(name)                                              \
   homedir = getenv(name);                                                    \
   if (homedir.size() and check_writeable_for_data(homedir.c_str()))          \
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
		printf
			("\nWARNING: either we can not detect your home directory "
			 "or you do not have one! Please contact the developers.\n\n");

		//TODO: is it really a good idea to set homedir to "." then ??

		printf("Instead of your home directory, '.' will be used.\n\n");
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
static void FS_Tokenize
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
 *
 * \todo Enable non-Unix paths
 */
std::string FileSystem::FS_CanonicalizeName(std::string path) const {
	std::list<std::string> components;
	std::list<std::string>::iterator i;

#ifdef _WIN32
	// remove all slashes with backslashes so following can work.
	for (uint32_t j = 0; j < path.size(); ++j) {
		if (path[j] == '/')
			path[j] = '\\';
	}
#endif

	FS_Tokenize(path, m_filesep, std::inserter(components, components.begin()));

	//tilde expansion
	if (!components.empty() && *components.begin() == "~") {
		components.erase(components.begin());
		FS_Tokenize
			(GetHomedir(),
			 m_filesep,
			 std::inserter(components, components.begin()));
	} else if (!pathIsAbsolute(path))
		//  make relative paths absolute (so that "../../foo" can work)
		FS_Tokenize
			(m_root.empty() ? getWorkingDirectory() : m_root, m_filesep,
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
const char * FileSystem::FS_Filename(const char * p) {
	const char * result = p;

	while (*p != '\0') {
		if (*p == '/' || *p == '\\')
			result = p + 1;
		++p;
	}

	return result;
}

std::string FileSystem::FS_Dirname(const std::string& full_path) {
	const std::string filename = FS_Filename(full_path.c_str());
	return full_path.substr(0, full_path.size() - filename.size());
}

std::string FileSystem::FS_FilenameExt(const std::string & f)
{
	// Find last '.' - denotes start of extension
	size_t ext_start = f.rfind('.');

	if (std::string::npos == ext_start)
		return "";
	else
		return f.substr(ext_start);
}

std::string FileSystem::FS_FilenameWoExt(const char * const p)
{
	std::string fname(p ? FileSystem::FS_Filename(p) : "");
	std::string ext(FileSystem::FS_FilenameExt(fname));
	return fname.substr(0, fname.length() - ext.length());
}

/// Create a filesystem from a zipfile or a real directory
/// \todo Catch FileType_error in all users
/// \todo Check for existence before doing anything with the file/dir
/// \todo Catch FileNotFound_error in all users
/// \throw FileNotFound_error if root does not exist, is some kind of special
/// file, loops around (via symlinks) or is too long for the OS/filesystem.
/// \throw FileAccessDenied_error if the OS denies access (of course ;-)
/// \throw FileTypeError if root is neither a directory or regular file
/// \todo throw FileTypeError if root is not a zipfile (exception from
/// ZipFilesystem)
FileSystem & FileSystem::Create(const std::string & root)
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
			throw FileNotFound_error("FileSystem::Create", root);
		}
		if (errno == EACCES)
			throw FileAccessDenied_error("FileSystem::Create", root);
	}

	if (S_ISDIR(statinfo.st_mode)) {
		return *new RealFSImpl(root);
	}
	if (S_ISREG(statinfo.st_mode)) { //TODO: ensure root is a zipfile
		return *new ZipFilesystem(root);
	}

	throw FileType_error
		("FileSystem::Create", root,
		 "cannot create virtual filesystem from file or directory");
}

#ifdef _WIN32
/// hack that is unfortunately needed for windows to check whether Widelands
/// can write in the directory
bool FileSystem::check_writeable_for_data(char const * const path)
{
	RealFSImpl fs(path);

	if (fs.IsDirectory(".widelands"))
		return true;
	try {
		// throws an exception if not writable
		fs.EnsureDirectoryExists(".widelands");
		fs.Unlink(".widelands");
		return true;
	} catch (...) {
		log("Directory %s is not writeable - next try\n", path);
	}

	return false;
}
#endif
