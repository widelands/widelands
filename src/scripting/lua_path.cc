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

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/macros.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"

namespace {

/// A class that makes iteration over filename_?.png templates easy.
class NumberGlob {
public:
	NumberGlob(const std::string& file_template);

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
	format_ = "%0" + boost::lexical_cast<std::string>(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		to_replace_ += "?";
	}
	max_ -= 1;
}

bool NumberGlob::next(std::string* s) {
	if (current_ > max_)
		return false;

	if (max_) {
		*s = boost::replace_last_copy(
		   template_, to_replace_, (boost::format(format_) % current_).str());
	} else {
		*s = template_;
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

   Lists the full path for all files that fit the template pattern.
   Use ? as placeholders for numbers, e.g. 'directory/idle_??.png' will list
   'directory/idle_00.png', 'directory/idle_01.png' etc, and
   'directory/idle.png' will just list 'directory/idle.png'.

   :type filename_template: class:`string`
   :arg filename_template: The filename template to use for the listing.

   :returns: An :class:`array` of file paths in lexicographical order.
*/
static int L_list_files(lua_State* L) {
	std::string filename_template = luaL_checkstring(L, 1);

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

const static struct luaL_Reg path[] = {{"basename", &L_basename},
                                       {"dirname", &L_dirname},
                                       {"list_files", &L_list_files},
                                       {nullptr, nullptr}};

void luaopen_path(lua_State* L) {
	luaL_newlib(L, path);
	lua_setglobal(L, "path");
}

}  // namespace LuaPath
