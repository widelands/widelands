/*
 * Copyright (C) 2002 by the Widelands Development Team
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

// 2002-02-10	sft+	made setup_searchpaths work for win32
// 2002-02-11	sft+	made setup_searchpaths work PROPERLY for win32
// 2002-08-07  nh		setup_searchpaths changed for saner default paths

#include <cerrno>
#include <cstring>
#include <string>

#include "config.h"
#include "filesystem.h"
#include "setup.h"
#include "error.h"

#ifdef USE_DATAFILE
#include "datafile.h"
#endif

#ifndef WIN32
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif


#ifdef __linux__
/*
==============
getexename

Read the actual name of the executable from /proc
==============
*/
static std::string getexename()
{
	static const char* const s_selfptr = "/proc/self/exe";

	char buf[PATH_MAX];
	int ret;

	ret = readlink(s_selfptr, buf, sizeof(buf));
	if (ret == -1) {
		log("readlink(%s) failed: %s\n", s_selfptr, strerror(errno));
		return "";
	}

	return std::string(buf, ret);
}
#endif

/** void setup_searchpaths(int argc, char **argv)
 *
 * Sets the filelocators default searchpaths (partly OS specific)
 */
void setup_searchpaths(int argc, char **argv)
{
	// first, try the install directory used in the last compile
	//TODO: not good - won't work on at least Gentoo
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory(INSTALL_DATADIR)); //see config.h for INSTALL_DATADIR

	// if everything else fails, search it where the FHS forces us to put it (obviously UNIX-only)
#ifndef WIN32
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory("/usr/share/widelands"));
#endif

	// absolute fallback directory is the CWD
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory("."));

	// the directory the executable is in is the default game data directory
	std::string exename;

#ifdef __linux__
	exename = getexename();
	if (!exename.size())
		exename = argv[0];
#else
	exename = argv[0];
#endif

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
#ifndef	WIN32
	std::string path;
	char *buf=getenv("HOME"); //do not use FS_GetHomedir() to not accidentally create ./.widelands

	if (buf) { // who knows, maybe the user's homeless
		path = std::string(buf) + "/.widelands";
		mkdir(path.c_str(), 0x1FF);
		g_fs->AddFileSystem(FileSystem::CreateFromDirectory(path.c_str()));
	}
#endif
}
