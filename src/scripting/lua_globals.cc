/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#include <map>
#include <memory>
#include <vector>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "build_info.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "scripting/report_error.h"

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
static int L_string_bformat(lua_State* L) {
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
				if (lua_isinteger(L, i)) {
					fmt % luaL_checkint32(L, i);
				} else {
					fmt % luaL_checknumber(L, i);
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
				NEVER_HERE();  // as report_error will never return

			default: {
				const std::string type = lua_typename(L, i);
				throw LuaError("Unexpected type " + type + " is not supported");
			}
			}
		}

		lua_pushstring(L, fmt.str());
		return 1;
	} catch (const boost::io::format_error& err) {
		report_error(L, "Error in bformat: %s", err.what());
	}
}

static std::map<const lua_State*, std::vector<std::string>> textdomains;
/* RST
   .. function:: push_textdomain(domain)

      Sets the textdomain for all further calls to :func:`_` until it is reset
      to the previous value using :func:`pop_textdomain`.

      :arg domain: The textdomain
      :type domain: :class:`string`
      :returns: :const:`nil`
*/
static int L_push_textdomain(lua_State* L) {
	textdomains[L].push_back(luaL_checkstring(L, -1));
	return 0;
}
/* RST
   .. function:: pop_textdomain()

      Resets the textdomain for calls to :func:`_` to the value it had
      before the last call to :func:`push_textdomain`.

      :returns: :const:`nil`
*/
static int L_pop_textdomain(lua_State* L) {
	textdomains.at(L).pop_back();
	return 0;
}
/* RST
   .. function:: set_textdomain(domain)

      DEPRECATED. Use `push_textdomain(domain)` instead.
*/
// TODO(Nordfriese): Delete after v1.0
static int L_set_textdomain(lua_State* L) {
	return L_push_textdomain(L);
}

static std::string current_textdomain(const lua_State* L) {
	const auto it = textdomains.find(L);
	return it == textdomains.end() || it->second.empty() ? "" : it->second.back();
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
static int L__(lua_State* L) {
	const std::string td = current_textdomain(L);

	if (!td.empty()) {
		i18n::Textdomain dom(td);
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
   :type msgid_plural: :class:`string`
   :arg n: The number of elements.
   :type n: An unsigned integer.

   :returns: The translated string.
*/
static int L_ngettext(lua_State* L) {
	//  S: msgid msgid_plural n
	const char* msgid = luaL_checkstring(L, 1);
	const char* msgid_plural = luaL_checkstring(L, 2);
	const int32_t n = luaL_checkint32(L, 3);
	if (n < 0) {
		report_error(L, "Call to ngettext with negative number %d", n);
	}

	const std::string td = current_textdomain(L);
	if (td.empty()) {
		lua_pushstring(L, dngettext(td.c_str(), msgid, msgid_plural, n));
	} else {
		lua_pushstring(L, ngettext(msgid, msgid_plural, n));
	}
	return 1;
}

/* RST
.. function:: pgettext(msgctxt, msgid)

   A wrapper for the pgettext() function, needed for allowing multiple translations of the same
   string according to context.

   :arg msgctxt: a named context for this string for disambiguation
   :type msgctxt: :class:`string`
   :arg msgid: text to translate
   :type msgid: :class:`string`

   :returns: The translated string.
*/
// UNTESTED
static int L_pgettext(lua_State* L) {
	//  S: msgctxt msgid
	const char* msgctxt = luaL_checkstring(L, 1);
	const char* msgid = luaL_checkstring(L, 2);

	const std::string td = current_textdomain(L);
	if (!td.empty()) {
		lua_pushstring(L, dpgettext_expr(td.c_str(), msgctxt, msgid));
	} else {
		lua_pushstring(L, pgettext_expr(msgctxt, msgid));
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
static int L_include(lua_State* L) {
	const std::string script = luaL_checkstring(L, -1);
	// remove our arguments so that the executed script gets a clear stack
	lua_pop(L, 1);

	try {
		lua_getfield(L, LUA_REGISTRYINDEX, "lua_interface");
		LuaInterface* lua = static_cast<LuaInterface*>(lua_touserdata(L, -1));
		lua_pop(L, 1);  // pop this userdata
		std::unique_ptr<LuaTable> table(lua->run_script(script));
		table->do_not_warn_about_unaccessed_keys();
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
.. function:: ticks()

   Returns an integer value representing the number of milliseconds since the SDL library
   initialized.
*/
static int L_ticks(lua_State* L) {
	lua_pushinteger(L, SDL_GetTicks());
	return 1;
}

/* RST
.. function:: get_build_id()

   returns the version string of this widelands executable.  Something like
   "build-16[debug]".
*/
static int L_get_build_id(lua_State* L) {
	lua_pushstring(L, build_id());
	return 1;
}

const static struct luaL_Reg globals[] = {{"_", &L__},
                                          {"get_build_id", &L_get_build_id},
                                          {"include", &L_include},
                                          {"ngettext", &L_ngettext},
                                          {"pgettext", &L_pgettext},
                                          {"set_textdomain", &L_set_textdomain},
                                          {"push_textdomain", &L_push_textdomain},
                                          {"pop_textdomain", &L_pop_textdomain},
                                          {"ticks", &L_ticks},
                                          {nullptr, nullptr}};

void luaopen_globals(lua_State* L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);

	// Also add in string.bformat to use boost::format instead, so that we get
	// proper localisation.
	lua_getglobal(L, "string");               // S: string_lib
	lua_pushstring(L, "bformat");             // S: string_lib "bformat"
	lua_pushcfunction(L, &L_string_bformat);  // S: string_lib "bformat" function
	lua_settable(L, -3);                      // S: string_lib
	lua_pop(L, 1);
}

}  // namespace LuaGlobals
