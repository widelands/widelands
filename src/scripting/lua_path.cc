/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
	// NOCOM(#sirver): use FS_Filename
	std::string filename = luaL_checkstring(L, -1);
	const size_t found = filename.rfind("/");
	if (found != std::string::npos) {
		lua_pushstring(L, filename.substr(found + 1));
	}
	return 1;
}

/* RST
.. function:: dirname(filename)

	Everything before the final / in filename.

	:type filename: class:`string`
	:arg filename: filename to return direname for.

   :returns: The dirname.
*/
static int L_dirname(lua_State * L) {
	// NOCOM(#sirver): trim / from return value
	// // NOCOM(#sirver): use FS_Dirname
	std::string filename = luaL_checkstring(L, -1);
	const size_t found = filename.rfind("/");
	if (found != std::string::npos) {
		lua_pushstring(L, filename.substr(0, found + 1));
	}
	return 1;
}

// NOCOM(#sirver): document
// NOCOM(#sirver): test?
// // NOCOM(#sirver): rename - not glob, but find files?
static int L_glob(lua_State * L) {
	const std::string dir = luaL_checkstring(L, -2);
	const std::string re_as_string = luaL_checkstring(L, -1);

	boost::regex re(re_as_string);
	std::set<std::string> files = filter(g_fs->ListDirectory(dir), [&re](const std::string& filename) {
		return boost::regex_match(FileSystem::FS_Filename(filename.c_str()), re);
	});
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
	{"glob", &L_glob},
	{nullptr, nullptr}
};

void luaopen_path(lua_State * L) {
	luaL_newlib(L, path);
	lua_setglobal(L, "path");
}

} // namespace LuaPath
