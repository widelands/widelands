/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "base/macros.h"
#include "base/string.h"
#include "io/filesystem/layered_filesystem.h"

namespace {

/// A class that makes iteration over filename_?.png templates easy.
// TODO(GunChleoc): Code duplication with g_fs->get_sequential_files.
// Get rid of this and list_files when conversion to spritemaps has been done.
class NumberGlob {
public:
	explicit NumberGlob(const std::string& file_template);

	/// If there is a next filename, puts it in 's' and returns true.
	bool next(std::string* s);

private:
	std::string template_;
	std::string format_;
	std::string to_replace_;
	uint32_t current_;
	uint32_t max_;

	DISALLOW_COPY_AND_ASSIGN(NumberGlob);
};

/**
 * Implementation for NumberGlob.
 */
NumberGlob::NumberGlob(const std::string& file_template) : template_(file_template), current_(0) {
	int nchars = count(file_template.begin(), file_template.end(), '?');
	format_ = "%0" + as_string(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		to_replace_ += "?";
	}
	--max_;
}

bool NumberGlob::next(std::string* s) {
	if (current_ > max_) {
		return false;
	}

	*s = template_;
	if (max_) {
		replace_last(*s, to_replace_, bformat(format_, current_));
	}
	++current_;
	return true;
}
}  // namespace

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
static int L_basename(lua_State* L) {
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
static int L_dirname(lua_State* L) {
	lua_pushstring(L, FileSystem::fs_dirname(luaL_checkstring(L, -1)));
	return 1;
}

/* RST
.. function:: list_files(filename_template)

   **DEPRECATED**. Lists the full path for all files that fit the template pattern.
   Use ? as placeholders for numbers, e.g. 'directory/idle\_??.png' will list
   'directory/idle_00.png', 'directory/idle_01.png' etc, and
   'directory/idle.png' will just list 'directory/idle.png'.
   Lua Tables need lots of memory, so only use this when you have to.

   :type filename_template: class:`string`
   :arg filename_template: The filename template to use for the listing.

   :returns: An :class:`array` of file paths in lexicographical order.
*/
static int L_list_files(lua_State* L) {
	const std::string filename_template = luaL_checkstring(L, 1);

	NumberGlob glob(filename_template);
	std::string filename;
	lua_newtable(L);
	int idx = 1;

	while (glob.next(&filename)) {
		if (!g_fs->file_exists(filename)) {
			break;
		}
		lua_pushint32(L, idx++);
		lua_pushstring(L, filename);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
.. function:: list_directory(filename)

   Returns all file names contained in the given directory.
   Lua Tables need lots of memory, so only use this when you have to.

   :type filename: class:`string`
   :arg filename: The directory to read.

   :returns: An :class:`array` of file names.
*/
static int L_list_directory(lua_State* L) {
	lua_newtable(L);
	int idx = 1;
	for (const std::string& filename : g_fs->list_directory(luaL_checkstring(L, 1))) {
		lua_pushint32(L, idx++);
		lua_pushstring(L, filename);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
.. function:: is_directory(filename)

   Checks whether the given filename points to a directory.

   :type filename: class:`string`
   :arg filename: The filename to check.

   :returns: ``true`` if the given path is a directory.
*/
static int L_is_directory(lua_State* L) {
	lua_pushboolean(L, g_fs->is_directory(luaL_checkstring(L, -1)));
	return 1;
}

/* RST
.. function:: file_exists(filename)

   Checks whether the given filename points to a file or directory.

   :type filename: class:`string`
   :arg filename: The filename to check.

   :returns: ``true`` if the given path is a file or directory.
*/
static int L_file_exists(lua_State* L) {
	lua_pushboolean(L, g_fs->file_exists(luaL_checkstring(L, -1)));
	return 1;
}

const static struct luaL_Reg path[] = {{"basename", &L_basename},
                                       {"dirname", &L_dirname},
                                       {"list_files", &L_list_files},
                                       {"list_directory", &L_list_directory},
                                       {"is_directory", &L_is_directory},
                                       {"file_exists", &L_file_exists},
                                       {nullptr, nullptr}};

void luaopen_path(lua_State* L) {
	luaL_newlib(L, path);
	lua_setglobal(L, "path");
}

}  // namespace LuaPath
