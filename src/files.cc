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

FileSystem::FileSystem():
	root("/"), filesep('/')
{
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
	if (path.find(filesep)==0) //is this an absolute path?
		pos=1;
	else //relative path
		pos=0;
	pos2=path.find(filesep, pos);
	//'current' token is now between pos and pos2

	//split path into it's components
	while (pos2!=std::string::npos) {
		components.push_back(path.substr(pos, pos2-pos));
		pos=pos2+1;
		pos2=path.find(filesep, pos);
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
std::string FileSystem::FS_CanonicalizeName(std::string path, std::string root)
{
	std::vector<std::string> components;
	std::vector<std::string>::iterator i;
#ifndef __WIN32__
	bool absolute=false;

	components=FS_Tokenize(path);
	if (path[0]=='/')
		absolute=true;

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
		char cwd[PATH_MAX+1];
		getcwd(cwd, PATH_MAX);

		std::vector<std::string> cwdcomponents;

		if (root.empty())
			cwdcomponents=FS_Tokenize(cwd);
		else
			cwdcomponents=FS_Tokenize(root);

		components.insert(components.begin(),
		                  cwdcomponents.begin(), cwdcomponents.end());
		absolute=true;
	}

	//clean up the path
	for(i=components.begin(); i!=components.end(); i++) {
		//remove empty components ("foo/bar//baz/")
		if (i->empty()) components.erase(i);

		//remove single dot
		if (*i==".") components.erase(i);

		//remove double dot and the preceding component (if any)
		if (*i=="..") {
			if(i!=components.begin()) {
				i--;
				components.erase(i);
			}
			components.erase(i);
		}
	}

    std::string canonpath="";
    if (absolute)
        canonpath="/";
        else
            canonpath="./";

    for(i=components.begin(); i!=components.end(); i++)
        canonpath+=*i+"/";
	canonpath=canonpath.substr(0, canonpath.size()-1); //remove trailing slash

	return canonpath;

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
    components=FS_Tokenize(path, '\\');
	if (path[0]=='\\') //Is a backslash in path? If yes, the path is allready absolute.
		absolutewin32=true;

    std::string canonpath="";
	if (absolutewin32==true)
		canonpath=""; //if the path is allready absolute, nothing has to be added.
    else   {
        canonpath=root.c_str();   //We simply add the root folder
        canonpath=canonpath+"\\"; //and of course an ending backslash
            }

    for(i=components.begin(); i!=components.end(); i++)
        canonpath+=*i+"\\";
	canonpath=canonpath.substr(0, canonpath.size()-1); //remove trailing slash

    return canonpath;

#endif
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
 * Create a filesystem to access the given directory as served by the OS
 */
FileSystem *FileSystem::CreateFromDirectory(std::string directory)
{
	return new RealFSImpl(directory);
}

/**
 * Create a filesystem from a zip file
 */
FileSystem *FileSystem::CreateFromZip(std::string filename)
{
	return new ZipFilesystem( filename );
}

/**
 * Read the actual name of the executable from /proc
 *
 * \todo is exename still neccessary, now that BINDIR can be seen from config.h?
 *       same question for slash/backslash detection, it's trivial with scons
*/
static const std::string getexename(const std::string argv0)
{
	static const char* const s_selfptr = "/proc/self/exe";
	char buf[PATH_MAX]="";
	int ret=0;

#ifdef __linux__
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
 */
void setup_searchpaths(const std::string argv0)
{
	// first, try the data directory used in the last scons invocation
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory(INSTALL_DATADIR)); //see config.h

#ifndef __WIN32__
	// if that fails, search it where the FHS forces us to put it (obviously UNIX-only)
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory("/usr/share/games/widelands"));
#else
	//TODO: is there a "default dir" for this on win32 ?
#endif

	// absolute fallback directory is the CWD
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory("."));

	// the directory the executable is in is the default game data directory
	std::string exename = getexename(argv0);
	std::string::size_type slash = exename.rfind('/');
	std::string::size_type backslash = exename.rfind('\\');

	if (backslash != std::string::npos && (slash == std::string::npos || backslash > slash))
		slash = backslash;

	if (slash != std::string::npos) {
		exename.erase(slash);
		if (exename != ".") {
			g_fs->AddFileSystem(FileSystem::CreateFromDirectory(exename));
#ifdef USE_DATAFILE
			exename.append ("/widelands.dat");
			g_fs->AddFileSystem(new Datafile(exename.c_str()));
#endif
		}
	}

	// finally, the user's config directory
	// TODO: implement this for UIWindows (yes, NT-based ones are actually multi-user)
#ifndef	__WIN32__
	std::string path;
	char *buf=getenv("HOME"); //do not use GetHomedir() to not accidentally create ./.widelands

	if (buf) { // who knows, maybe the user's homeless
		path = std::string(buf) + "/.widelands";
		mkdir(path.c_str(), 0x1FF);
		g_fs->AddFileSystem(FileSystem::CreateFromDirectory(path.c_str()));
	} else {
		//TODO: complain
	}
#endif
}
