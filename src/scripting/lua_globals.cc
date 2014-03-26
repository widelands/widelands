/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include "scripting/lua_globals.h"

#include <exception>

#include <boost/format.hpp>
#include <libintl.h>

#include "build_info.h"
#include "i18n.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "scripting/c_utils.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"

namespace LuaGlobals {

/* RST
Global functions
======================

The following functions are imported into the global namespace
of all scripts that are running inside widelands. They provide convenient
access to other scripts in other locations, localisation features and more.

There is also a global variable called __file__ defined that is the current
files name.
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */


/*
 * ========================================================================
 *                         MODULE FUNCTIONS
 * ========================================================================
 */

/* RST
.. function:: string.bformat

	Not really a global function. But we add a method to string built in type in
	Lua that has similar functionality to the built in string.format, but
	instead uses boost::format. This allows for better control of the formatting
	as well as reordering of arguments which is needed for proper localisation.

   :returns: :const:`nil`
*/
static int L_string_bformat(lua_State * L) {
	try {
		boost::format fmt(luaL_checkstring(L, 1));
		const int nargs = lua_gettop(L);

		// Start with argument, the first is already consumed
		for (int i = 2; i <= nargs; ++i) {
			switch (lua_type(L, i)) {
				case LUA_TNIL:
					fmt % "nil";
					break;

				case LUA_TNUMBER:
					{
						int d = lua_tointeger(L, i);
						if (d == 0 && !lua_isnumber(L, 1)) {
							fmt % d;
						} else {
							fmt % luaL_checknumber(L, i);
						}
					}
					break;

				case LUA_TBOOLEAN:
					fmt % luaL_checkboolean(L, i);
					break;

				case LUA_TSTRING:
					fmt % luaL_checkstring(L, i);
					break;

				case LUA_TTABLE:
				case LUA_TFUNCTION:
				case LUA_TUSERDATA:
				case LUA_TTHREAD:
				case LUA_TLIGHTUSERDATA:
					report_error(L, "Cannot format the given type %s at index %i", lua_typename(L, i), i);
					break;
			}
		}

		lua_pushstring(L, fmt.str());
		return 1;
	} catch (const boost::io::format_error& err) {
		return report_error(L, "Error in bformat: %s", err.what());
	}
}
/* RST
	.. function:: set_textdomain(domain)

		Sets the textdomain for all further calls to :func:`_`.

		:arg domain: The textdomain
		:type domain: :class:`string`
		:returns: :const:`nil`
*/
static int L_set_textdomain(lua_State * L) {
	luaL_checkstring(L, -1);
	lua_setglobal(L, "__TEXTDOMAIN");
	return 0;
}

/* RST
	.. function:: _(str)

		This peculiar function is used to translate texts in your scenario into
		another language. The function takes a single string, grabs the
		textdomain of your map (which is usually the maps name) and returns the
		translated string. Make sure that you separate translatable and untranslatable
		stuff:

		.. code-block:: lua

			s = "<p><br>" .. _ "Only this should be translated" .. "<br></p>"

		:arg str: text to translate.
		:type str: :class:`string`
		:returns: The translated string.
*/
static int L__(lua_State * L) {
	lua_getglobal(L, "__TEXTDOMAIN");

	if (not lua_isnil(L, -1)) {
		i18n::Textdomain dom(luaL_checkstring(L, -1));
		lua_pushstring(L, i18n::translate(luaL_checkstring(L, 1)));
	} else {
		lua_pushstring(L, i18n::translate(luaL_checkstring(L, 1)));
	}
	return 1;
}

/* RST
.. function:: ngettext(msgid, msgid_plural, n)

	A wrapper for the ngettext() function, needed for translations of numbered
	strings.

	:arg msgid: text to translate (singular)
	:type msgid: :class:`string`
	:arg msgid_plural: text to translate (plural)
	:type msgid_plural :class:`string`
	:arg n: The number of elements.
	:type n: An unsigned integer.

	:returns: The translated string.
*/
// UNTESTED
static int L_ngettext(lua_State * L) {
	//  S: msgid msgid_plural n
	const std::string msgid = luaL_checkstring(L, 1);
	const std::string msgid_plural = luaL_checkstring(L, 2);
	const uint32_t n = luaL_checkuint32(L, 3);

	lua_getglobal(L, "__TEXTDOMAIN");
	if (not lua_isnil(L, -1)) {
		i18n::Textdomain dom(luaL_checkstring(L, -1));
		lua_pushstring(L, ngettext(msgid.c_str(), msgid_plural.c_str(), n));
	} else {
		lua_pushstring(L, ngettext(msgid.c_str(), msgid_plural.c_str(), n));
	}
	return 1;
}

/* RST
	.. function:: include(script)

		Includes the script at the given location at the current position in the
		file. The script can begin with 'map:' to include files from the map.

		:type script: :class:`string`
		:arg script: The filename relative to the root of the data directory.
		:returns: :const:`nil`
*/
static int L_include(lua_State * L) {
	const std::string script = luaL_checkstring(L, -1);
	// remove our arguments so that the executed script gets a clear stack
	lua_pop(L, 1);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, "lua_interface");
		LuaInterface * lua = static_cast<LuaInterface *>(lua_touserdata(L, -1));
		lua_pop(L, 1); // pop this userdata
		lua->run_script(script);
	} catch (std::exception & e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
.. function:: get_build_id()

	returns the version string of this widelands executable.  Something like
	"build-16[debug]".
*/
static int L_get_build_id(lua_State * L) {
	lua_pushstring(L, build_id());
	return 1;
}

const static struct luaL_Reg globals [] = {
	{"_", &L__},
	{"get_build_id", &L_get_build_id},
	{"include", &L_include},
	{"ngettext", &L_ngettext},
	{"set_textdomain", &L_set_textdomain},
	{nullptr, nullptr}
};

void luaopen_globals(lua_State * L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);

	// Also add in string.bformat to use boost::format instead, so that we get
	// proper localisation.
	lua_getglobal(L, "string");  // S: string_lib
	lua_pushstring(L, "bformat");  // S: string_lib "bformat"
	lua_pushcfunction(L, &L_string_bformat);  // S: string_lib "bformat" function
	lua_settable(L, -3);  // S: string_lib
	lua_pop(L, 1);
}


}  // namespace LuaGlobals
