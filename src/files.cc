/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#include "config.h"
#include "constants.h"
#include "errno.h"
#include "error.h"
#include "filesystem.h"
#include "layeredfilesystem.h"
#include "realfsimpl.h"
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "zip_filesystem.h"

#ifdef USE_DATAFILE
#include "datafile.h"
#endif

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #define stat _stat
#else
  #include <glob.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

FileSystem::FileSystem()
{
#ifdef __WIN32__
	//TODO: this probably needs to be overwritten later
	m_root="C:\\";
	m_filesep='\\';
#else
	m_root="/";
	m_filesep='/';
#endif
}

/**
 * Append extension (e.g. ".foo") to the filename if it doesn't already have an extension
 */
char *FileSystem::FS_AutoExtension(char *buf, int bufsize, const char *ext)
{
	char *dot;
	char *p;
	int extlen;

	dot = 0;

	for(p = buf; *p; p++) {
		if (*p == '/' || *p == '\\')
			dot = 0;
		else if (*p == '.')
			dot = p;
	}

	if (!dot) {
		extlen = strlen(ext);

		if (p - buf + extlen < bufsize)
			memcpy(p, ext, extlen + 1);
	}

	return buf;
}

/**
 * Strip the extension (if any) from the filename
 */
char *FileSystem::FS_StripExtension(char *fname)
{
	char *p;
	char *dot = 0;

	for(p = fname; *p; p++) {
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
char *FileSystem::FS_RelativePath(char *buf, int buflen, const char *basefile, const char *filename)
{
	const char *p;
	int endbase;

	if (*filename == '/' || *filename == '\\') { // it's an absolute filename
		snprintf(buf, buflen, "%s", filename);
		return buf;
	}

	// find the end of the basefile name
	endbase = 0;
	for(p = basefile; *p; p++) {
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
 * \return True if ref path is absolute
 */
const bool FileSystem::pathIsAbsolute(const std::string path)
{
	if (path.substr(0,m_root.size())!=m_root)
		return false;

	if (path[m_root.size()]!=m_filesep)
		return false;

	return true;
}

/**
 * Prepend FS root and/or working directory (if neccessary) to make path absolute.
 * \param path A path that might or might not be absolute
 * \return An absolute path
 */
const std::string FileSystem::AbsolutePath(const std::string path)
{
	if (pathIsAbsolute(path))
		return path;

#ifdef __WIN32__
	//For "nearly" absoute paths, just append drive letter
	if (path[0]==m_filesep)
		return m_root+path;
#endif

	return getWorkingDirectory()+m_filesep+path;
}

/**
 * \return The process' current working directory
 */
const std::string FileSystem::getWorkingDirectory()
{
	char cwd[PATH_MAX+1];
	getcwd(cwd, PATH_MAX);

	return std::string(cwd);
}

/**
 * \return An existing directory where temporary files can be put
 * \todo win32 ?
 * \todo Make sure the directory exists
 */
const std::string FileSystem::getTempDirectory()
{
#ifdef __WIN32__
	return ".";
#else
	return "/tmp";
#endif
}

/// \todo Write homedir detection for non-getenv-systems
std::string FileSystem::GetHomedir()
{
	std::string homedir("");

#ifdef HAS_GETENV
	homedir=getenv("HOME");
#endif

	if (homedir.empty()) {
		printf("\nWARNING: either we can not detect your home directory "
		       "or you don't have one! Please contact the developers.\n\n");

		//TODO: is it really a good idea to set homedir to "." then ??

		printf("Instead of your home directory, '.' will be used.\n\n");
		homedir=".";
	}

	return homedir;
}

/**
 * Split a string into components seperated by a certain character
 *
 * \param path The path to parse
 * \param filesep A character seperating the components
 * \return a list of path components
 *
 * \todo This does not really belong into a filesystem class
 */
std::vector<std::string> FileSystem::FS_Tokenize(std::string path)
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
std::string FileSystem::FS_CanonicalizeName(std::string path)
{
	std::vector<std::string> components;
	std::vector<std::string>::iterator i;

#ifndef __WIN32__
	bool absolute=pathIsAbsolute(path);

	components=FS_Tokenize(path);

	//tilde expansion
	if(*components.begin()=="~") {
		components.erase(components.begin());

		std::vector<std::string> homecomponents;
		homecomponents=FS_Tokenize(GetHomedir());
		components.insert(components.begin(),
		                  homecomponents.begin(), homecomponents.end());

		absolute=true;
	}

	//make relative paths absolute (so that "../../foo" can work)
	if (!absolute) {
		std::vector<std::string> cwdcomponents;

		if (m_root.empty())
			cwdcomponents=FS_Tokenize(getWorkingDirectory());
		else
			cwdcomponents=FS_Tokenize(m_root);

		components.insert(components.begin(),
		                  cwdcomponents.begin(), cwdcomponents.end());
		absolute=true;
	}

	//clean up the path
	for(i=components.begin(); i!=components.end(); ) {
		bool erase = false;
		bool erase_prev = false;

		//remove empty components ("foo/bar//baz/")
		if (i->empty()) erase = true;

		//remove single dot
		if (*i==".") erase = true;

		//remove double dot and the preceding component (if any)
		if (*i=="..") {
			if(i!=components.begin())
				erase_prev = true;
			erase = true;
		}

		std::vector<std::string>::iterator nexti = i;

		if( erase_prev && erase ) {
			components.erase( i-1, i+1);
			i = components.begin();
			continue;
		}
		if( erase ) {
			components.erase( i );
			i = components.begin();
			continue;
		}

		i++;
	}

	std::string canonpath="";
	if (absolute)
		canonpath="/";
	else
		canonpath="./";

	for(i=components.begin(); i!=components.end(); i++)
		canonpath+=*i+"/";

#else

	/*===============================================
	Windows-filesystems work different than Unix-FS
	so here is a solution, to get it run.

	Every path will be absolute afterwards.

	Only problem:
	Widelands will be started in the directory, the
	the executable is in, even if you run it from
	different directory. (Real problem???)
	===============================================*/

	bool absolutewin32=false;
	components=FS_Tokenize(path);
	if (path[0]=='\\') //Is a backslash in path? If yes, the path is allready absolute.
		absolutewin32=true;

	std::string canonpath="";
	if (absolutewin32==true)
		canonpath=""; //if the path is allready absolute, nothing has to be added.
	else   {
		canonpath=m_root.c_str();   //We simply add the root folder
		canonpath=canonpath+"\\"; //and of course an ending backslash
	}

	for(i=components.begin(); i!=components.end(); i++)
		canonpath+=*i+"\\";

#endif
	canonpath.erase(canonpath.end() - 1); //remove trailing slash
	return canonpath;
}

/**
 * Returns the filename of this path, everything after the last
 * / or \  (or the whole string)
 */
const char *FileSystem::FS_Filename(const char* buf) {
	int i=strlen(buf)-1;
	while(i>=0) {
		if(buf[i]=='/' || buf[i]=='\\') return &buf[i+1];
		--i;
	}
	return buf;
}

/**
 * Create a filesystem from a zipfile or a real directory
 * \todo Catch FileType_error in all users
 * \todo Check for existence before doing anything with the file/dir
 * \todo Catch FileNotFound_error in all users
 * \throw FileNotFound_error if root does not exist, is some kind of special file,
 * loops around (via symlinks) or is too long for the OS/filesystem
 * \throw FileAccessDenied_error if the OS denies access (of course ;-)
 * \throw FileTypeError if root is neither a directory or regular file
 * \todo throw FileTypeError if root is not a zipfile (exception from ZipFilesystem)
 */
FileSystem *FileSystem::Create(std::string root) throw(FileType_error,
      FileNotFound_error,
      FileAccessDenied_error)
{
	struct stat statinfo;

	if (stat(root.c_str(), &statinfo) == -1) {
		if ( errno==EBADF ||
		      errno==ENOENT ||
		      errno==ENOTDIR ||
#ifdef ELOOP
		      errno==ELOOP || //MinGW does not support ELOOP (yet)
#endif
		      errno==ENAMETOOLONG )
		{
			throw FileNotFound_error("FileSystem::Create", root);
		}
		if ( errno==EACCES ) {
			throw FileAccessDenied_error("FileSystem::Create", root);
		}
	}

	if(S_ISDIR(statinfo.st_mode)) {
		return new RealFSImpl(root);
	}
	if(S_ISREG(statinfo.st_mode)) {
		return new ZipFilesystem(root);
	}

	throw FileType_error("FileSystem::Create", root,
								"cannot create virtual filesystem from file/directory");
}

/**
 * Read the actual name of the executable from /proc
 *
 * \todo is exename still neccessary, now that BINDIR can be seen from config.h?
 *       same question for slash/backslash detection, it's trivial with scons
*/
static const std::string getexename(const std::string argv0)
{
	char buf[PATH_MAX]="";
	int ret=0;

#ifdef __linux__
	static const char* const s_selfptr = "/proc/self/exe";

	ret = readlink(s_selfptr, buf, sizeof(buf));
	if (ret == -1) {
		log("readlink(%s) failed: %s\n", s_selfptr, strerror(errno));
		return "";
	}
#endif

	if (ret>0)
		return std::string(buf, ret);
	else
		return argv0;
}

/**
 * Sets the filelocators default searchpaths (partly OS specific)
 * \todo This belongs into WLApplication
 * \todo Handle exception FileType_error
 * \todo Handle case when \e no data can be found
 */
void setup_searchpaths(const std::string argv0)
{
	try {
#ifdef __APPLE__
		// on mac, the default Data Dir ist Relative to the current directory
		g_fs->AddFileSystem(FileSystem::Create("Widelands.app/Contents/Resources/"));
#else
		// first, try the data directory used in the last scons invocation
		g_fs->AddFileSystem(FileSystem::Create(INSTALL_DATADIR)); //see config.h
#endif
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	try {
#ifndef __WIN32__
		// if that fails, search it where the FHS forces us to put it (obviously UNIX-only)
		g_fs->AddFileSystem(FileSystem::Create("/usr/share/games/widelands"));
#else
		//TODO: is there a "default dir" for this on win32 and mac ?
#endif
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	try {
		// absolute fallback directory is the CWD
		g_fs->AddFileSystem(FileSystem::Create("."));
	}
	catch (FileNotFound_error e) {}
	catch (FileAccessDenied_error e) {
		log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
	}
	catch (FileType_error e) {
		//TODO: handle me
	}

	//TODO: what if all the searching failed? Bail out!

	// the directory the executable is in is the default game data directory
	std::string exename = getexename(argv0);
	std::string::size_type slash = exename.rfind('/');
	std::string::size_type backslash = exename.rfind('\\');

	if (backslash != std::string::npos && (slash == std::string::npos || backslash > slash))
		slash = backslash;

	if (slash != std::string::npos) {
		exename.erase(slash);
		if (exename != ".") {
			try {
				g_fs->AddFileSystem(FileSystem::Create(exename));
#ifdef USE_DATAFILE
				exename.append ("/widelands.dat");
				g_fs->AddFileSystem(new Datafile(exename.c_str()));
#endif
			}
			catch (FileNotFound_error e) {}
			catch (FileAccessDenied_error e) {
				log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
			}
			catch (FileType_error e) {
				//TODO: handle me
			}
		}
	}

	// finally, the user's config directory
	// TODO: implement this for Windows (yes, NT-based ones are actually multi-user)
#ifndef	__WIN32__
	std::string path;
	char *buf=getenv("HOME"); //do not use GetHomedir() to not accidentally create ./.widelands

	if (buf) { // who knows, maybe the user's homeless
		path = std::string(buf) + "/.widelands";
		mkdir(path.c_str(), 0x1FF);
		try {
			g_fs->AddFileSystem(FileSystem::Create(path.c_str()));
		}
		catch (FileNotFound_error e) {}
		catch (FileAccessDenied_error e) {
			log("Access denied on %s. Continuing.\n", e.m_filename.c_str());
		}
		catch (FileType_error e) {
			//TODO: handle me
		}
	} else {
		//TODO: complain
	}
#endif
}
