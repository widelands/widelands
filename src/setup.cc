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

#include "widelands.h"
#include "graphic.h"
#include "font.h"
#include "ui_basic.h"
#include "setup.h"

#ifndef WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif


/** void setup_searchpaths(int argc, char **argv)
 *
 * Sets the filelocators default searchpaths (partly OS specific)
 */
void setup_searchpaths(int argc, char **argv)
{
	// absolute fallback directory is the CWD
	g_fs->AddFileSystem(FileSystem::CreateFromDirectory("."));
	
	// the directory the executable is in is the default game data directory
	char *exename = strdup(argv[0]);
	char *slash = strrchr(exename, '/');
	char *backslash = strrchr(exename, '\\');
	
	if (backslash && (!slash || backslash > slash))
		slash = backslash;
	if (slash) {
		*slash = 0;
		if (strcmp(exename, "."))
			g_fs->AddFileSystem(FileSystem::CreateFromDirectory(exename));
	}
	
	free(exename);

	// finally, the user's config directory
	// TODO: implement this for UIWindows (yes, NT-based ones are actually multi-user)
#ifndef	WIN32
	std::string path;
	char *buf=getenv("HOME");
	
	if (buf) { // who knows, maybe the user's homeless
		path = std::string(buf) + "/.widelands";
		mkdir(path.c_str(), 0x1FF);
		g_fs->AddFileSystem(FileSystem::CreateFromDirectory(path.c_str()));
	}
#endif
}
