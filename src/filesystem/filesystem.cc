/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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
// this originally comes from Return to the Shadows (http://www.rtts.org/)
// files.cc: provides all the OS abstraction to access files

#include "filesystem.h"

#include "config.h"
#include "constants.h"
#ifdef USE_DATAFILE
#include "datafile.h"
#endif
#include "disk_filesystem.h"
#include "layered_filesystem.h"
#include "zip_exceptions.h"
#include "zip_filesystem.h"

#include <cassert>
#include <string>
#include <cstring>
#include <vector>
#include <climits>
#include <cstdlib>

#include <errno.h>
#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #define stat _stat
  #include <sys/stat.h>
#else
  #include <glob.h>
  #include <sys/stat.h>
  #include <sys/types.h>
#endif
#include <unistd.h>

FileSystem::FileSystem()
{
#ifdef __WIN32__
	//TODO: this probably needs to be overwritten later
	m_root="C:";
	m_filesep='\\';
#else
	m_root="/";
	m_filesep='/';
#endif
}

/**
 * Append extension (e.g. ".foo") to the filename if it isn't already there
 * \param filename The filename to possibly be extended
 * \param extension The extension to append, without leading dot (unless you
 * want two consecutive dots)
 */
std::string FileSystem::AutoExtension
(std::string const & filename, std::string const & extension)
{
	const SSS_T suffix_length=extension.size()+1;
	const SSS_T startpos=filename.size()-suffix_length;

	if (filename.substr(startpos, suffix_length) != "."+extension)
	{
		return filename+"."+extension;
	}

	return filename;
}

/**
 * Strip the extension (if any) from the filename
 */
const char *FileSystem::FS_StripExtension(char * const fname)
{
	char *p;
	char *dot = 0;

	for (p = fname; *p; ++p) {
		if (*p == '/' || *p == '\\')
			dot = 0;
		else if (*p == '.')
			dot = p;
	}

	if (dot)
		*dot = 0;

	return fname;
}

/**
 * Translate filename so that it is relative to basefile.
 * Basically concatenates the two strings, but removes the filename part
 * of basefile (if any)
 */
const char *FileSystem::FS_RelativePath(char *buf, const int32_t buflen, const char *basefile, const char *filename)
{
	const char *p;
	int32_t endbase;

	if (*filename == '/' || *filename == '\\') { // it's an absolute filename
		snprintf(buf, buflen, "%s", filename);
		return buf;
	}

	// find the end of the basefile name
	endbase = 0;
	for (p = basefile; *p; ++p) {
		if (*p == '/' || *p == '\\')
			endbase = p-basefile+1;
	}

	if (!endbase) { // no path in basefile
		snprintf(buf, buflen, "%s", filename);
		return buf;
	}

	// copy the base path
	snprintf(buf, buflen, "%s%s", basefile, filename);

	return buf;
}

/**
 * \param path A file or directory name
 * \return True if ref path is absolute, false otherwise
 */
bool FileSystem::pathIsAbsolute(std::string const & path) const {
	std::string::size_type const path_size = path  .size();
	std::string::size_type const root_size = m_root.size();
	if (path_size < root_size)
		return false;

	if (path_size == root_size)
		return path == m_root;

	if (path.substr(0, m_root.size())!=m_root)
		return false;

	assert(root_size < path_size); //  Otherwise an invalid read happens below.
	if (path[root_size] != m_filesep)
		return false;

	return true;
}

/**
 * Prepend FS root and/or working directory (if necessary) to make path absolute.
 * \param path A path that might or might not be absolute
 * \return An absolute path
 */
std::string FileSystem::AbsolutePath(std::string const & path) const {
	if (pathIsAbsolute(path))
		return path;

	return getWorkingDirectory()+m_filesep+path;
}

/**
 * \return The process' current working directory
 */
std::string FileSystem::getWorkingDirectory() const {
	char cwd[PATH_MAX+1];
	getcwd(cwd, PATH_MAX);

	return std::string(cwd);
}

/**
 * \return An existing directory where temporary files can be put
 * \todo Is there a temp directory on win32? Where?
 */
std::string FileSystem::getTempDirectory() {
	const char *tmpdir;

#ifdef __WIN32__
	tmpdir=".";
#else
	tmpdir="/tmp";
#endif

	if (!FileExists(tmpdir))
		throw FileNotFound_error("FileSystem::getTempDirectory", tmpdir);

	return tmpdir;
}

/// \todo Write homedir detection for non-getenv-systems
std::string FileSystem::GetHomedir()
{
	std::string homedir("");

#ifdef HAS_GETENV
	homedir=getenv("HOME");
#endif

	if (homedir.empty()) {
		printf
			("\nWARNING: either we can not detect your home directory "
			 "or you don't have one! Please contact the developers.\n\n");

		//TODO: is it really a good idea to set homedir to "." then ??

		printf("Instead of your home directory, '.' will be used.\n\n");
		homedir=".";
	}

	return homedir;
}

/**
 * Split a string into components separated by a certain character.
 *
 * \param path The path to parse
 * \return a list of path components
 *
 * \todo This does not really belong into a filesystem class
 */
std::vector<std::string> FileSystem::FS_Tokenize
(std::string const & path) const
{
	std::vector<std::string> components;
	SSS_T pos;  //start of token
	SSS_T pos2; //next filesep character

	//extract the first path component
	if (path.find(m_filesep)==0) //is this an absolute path?
		pos=1;
	else //relative path
		pos=0;
	pos2=path.find(m_filesep, pos);
	//'current' token is now between pos and pos2

	//split path into it's components
	while (pos2!=std::string::npos) {
		components.push_back(path.substr(pos, pos2-pos));
		pos=pos2+1;
		pos2=path.find(m_filesep, pos);
	}

	//extract the last component (most probably a filename)
	components.push_back(path.substr(pos));

	return components;
}

/**
 * Transform any valid, unique pathname into a well-formed absolute path
 *
 * \todo Enable non-Unix paths
 */
std::string FileSystem::FS_CanonicalizeName
(std::string const & path) const
{
	std::vector<std::string> components;
	std::vector<std::string>::iterator i;

#ifndef __WIN32__
	bool absolute=pathIsAbsolute(path);

	components=FS_Tokenize(path);

	//tilde expansion
	if (*components.begin()=="~") {
		components.erase(components.begin());

		std::vector<std::string> homecomponents;
		homecomponents=FS_Tokenize(GetHomedir());
		components.insert
			(components.begin(), homecomponents.begin(), homecomponents.end());

		absolute=true;
	}

	//make relative paths absolute (so that "../../foo" can work)
	if (!absolute) {
		std::vector<std::string> cwdcomponents;

		if (m_root.empty())
			cwdcomponents=FS_Tokenize(getWorkingDirectory());
		else
			cwdcomponents=FS_Tokenize(m_root);

		components.insert
			(components.begin(), cwdcomponents.begin(), cwdcomponents.end());
		absolute=true;
	}

	//clean up the path
	for (i=components.begin(); i!=components.end();) {
		bool erase = false;
		bool erase_prev = false;

		//remove empty components ("foo/bar//baz/")
		if (i->empty()) erase = true;

		//remove single dot
		if (*i==".") erase = true;

		//remove double dot and the preceding component (if any)
		if (*i=="..") {
			if (i!=components.begin())
				erase_prev = true;
			erase = true;
		}

		std::vector<std::string>::iterator nexti = i;

		if (erase_prev && erase) {
			components.erase(i-1, i+1);
			i = components.begin();
			continue;
		}
		if (erase) {
			components.erase(i);
			i = components.begin();
			continue;
		}

		++i;
	}

	std::string canonpath="";
	if (absolute)
		canonpath="/";
	else
		canonpath="./";

	for (i = components.begin(); i != components.end(); ++i)
		canonpath+=*i+"/";

	canonpath.erase(canonpath.end() - 1); //remove trailing slash

#else // ifndef __WIN32__
	// Still a stupid workaround: some paths still might be relative,
	// but this is acceptable on windows, as long as all data is stored
	// in the working directory.

	bool absolute=false;
	std::string canonpath;

	//  Is a ":" at the second position? If yes, it is already absolute (like
	//  "C:\...").
	if (path[1] == ':')
		absolute=true;

	if (absolute == true) {
		canonpath = path;

	} else {
		if (m_root.empty())
			canonpath = getWorkingDirectory() + "\\" + path;
		else
			canonpath = m_root + "\\" + path;
	}

#endif // ifndef __WIN32__
	return canonpath;
}

/**
 * Returns the filename of this path, everything after the last
 * / or \  (or the whole string)
 */
const char *FileSystem::FS_Filename(const char* buf) {
	int32_t i=strlen(buf)-1;
	while (i>=0) {
		if (buf[i]=='/' || buf[i]=='\\') return &buf[i+1];
		--i;
	}
	return buf;
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
FileSystem * FileSystem::Create(std::string const & root)
throw (FileType_error, FileNotFound_error, FileAccessDenied_error)
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
		if (errno==EACCES) {
			throw FileAccessDenied_error("FileSystem::Create", root);
		}
	}

	if (S_ISDIR(statinfo.st_mode)) {
		return new RealFSImpl(root);
	}
	if (S_ISREG(statinfo.st_mode)) { //TODO: ensure root is a zipfile
		return new ZipFilesystem(root);
	}

	throw FileType_error
		("FileSystem::Create", root,
		 "cannot create virtual filesystem from file or directory");
}
