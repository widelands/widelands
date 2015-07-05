/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include "scripting/lua_path.h"

#include <boost/regex.hpp>

#include "helper.h"
#include "io/filesystem/layered_filesystem.h"

namespace LuaPath {

/* RST
:mod:`path`
=============

.. module:: path
   :synopsis: Provides access to path modifications, globbing and so on.

.. moduleauthor:: The Widelands development team

.. currentmodule:: path

*/

/* RST
Module Functions
^^^^^^^^^^^^^^^^

*/

/* RST
.. function:: basename(filename)

   Returns everything behind the last /.

	:type filename: class:`string`
	:arg filename: filename to return basename for.

   :returns: The basename.
*/
static int L_basename(lua_State * L) {
	lua_pushstring(L, FileSystem::fs_filename(luaL_checkstring(L, -1)));
	return 1;
}

/* RST
.. function:: dirname(filename)

	Everything before the final / in filename. The returned value is either the empty string or
	ends with a '/'.

	:type filename: class:`string`
	:arg filename: filename to return dirname for.

   :returns: The dirname.
*/
static int L_dirname(lua_State * L) {
	lua_pushstring(L, FileSystem::fs_dirname(luaL_checkstring(L, -1)));
	return 1;
}


/* RST
.. function:: list_directory(directory[, regexp])

	Lists all files and directories in the given directory. If regexp is not
	:const:`nil` only the files whose basename matches the regular expression
	will be returned. The method never returns "." or ".." in its results.

	:type directory: class:`string`
	:arg directory: The directory to list files for.
	:type regexp: class:`string`
	:arg regexp: The regular expression each files must match.

   :returns: An :class:`array` of filenames in lexicographical order.
*/
static int L_list_directory(lua_State * L) {
	const std::string dir = luaL_checkstring(L, 1);
	std::set<std::string> files = g_fs->list_directory(dir);

	if (lua_gettop(L) > 1) {
		boost::regex re(luaL_checkstring(L, 2));
		files = filter(files, [&re](const std::string& filename) {
			return boost::regex_match(FileSystem::fs_filename(filename.c_str()), re);
		});
	}

	lua_newtable(L);
	int idx = 1;

	for (const std::string& filename : files) {
		lua_pushint32(L, idx++);
		lua_pushstring(L, filename);
		lua_settable(L, -3);
	}
	return 1;
}

const static struct luaL_Reg path [] = {
	{"basename", &L_basename},
	{"dirname", &L_dirname},
	{"list_directory", &L_list_directory},
	{nullptr, nullptr}
};

void luaopen_path(lua_State * L) {
	luaL_newlib(L, path);
	lua_setglobal(L, "path");
}

} // namespace LuaPath
