/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scripting/lua_globals.h"

#include <map>
#include <memory>
#include <vector>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "build_info.h"
#include "io/fileread.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/addons.h"
#include "logic/game_data_error.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_map.h"
#include "scripting/lua_table.h"
#include "scripting/report_error.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

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

   Not really a global function. But we add a method to ``string`` built-in type in
   Lua that has similar functionality to the built-in ``string.format``, but
   instead uses our own ``format`` function. This allows for better control of the
   formatting as well as reordering of arguments which is needed for proper localisation.

   Use ``bformat()`` whenever your string contains values from variables.

   :returns: The formatted string.

   ``bformat()`` can be used with the global :func:`_(str) function <_>`. Examples:

   .. code-block:: lua

      local a_str    = _("Widelands")
      local a_number = 5
      local a_float  = 3.4

      _("This is a string: %1$s"):bformat(a_str)             -- s = string
      _("This is a number: %1$d"):bformat(a_number)          -- d = number (integer)
      _("This is a number: %1$f"):bformat(a_float)           -- f = number (float)

      -- %1$ refers to the first,
      -- %2$ to the second placeholder and so on:

      local tribe_name = _("Atlanteans")
      _("The %1$s are one of the tribes in %2$s."):bformat(tribe_name, a_str)

      -- Formatting numbers with two digits:
      local hours   = 2
      local minutes = 10
      local seconds = 5

      -- TRANSLATORS: A time string (hh:mm:ss) like "10:02:30"
      _("%1$02d:%2$02d:%3$02d"):bformat(hours, minutes, seconds) -- result: 02:10:05

      -- Formatting floating point numbers with precision:
      local endless = 10 / 3

      _("Precision of 2: %1$.2f"):bformat(endless)           -- result: Precision of 2: 3.33

   If your variable contains a number you should use :func:`ngettext` or
   :func:`npgettext` to allow proper translation of plural strings.

*/
// The 'b' in bformat used to stand for "boost", which we no longer use, but
// renaming the Lua function would break backwards compatibility.
static int L_string_bformat(lua_State* L) {
	try {
		format_impl::ArgsVector fmt_args;
		format_impl::ArgsPair arg;
		const int nargs = lua_gettop(L);

		// Start with argument, the first is already consumed
		for (int i = 2; i <= nargs; ++i) {
			switch (lua_type(L, i)) {
			case LUA_TNIL:
				arg.first = format_impl::AbstractNode::ArgType::kString;
				arg.second.string_val = "nil";
				fmt_args.emplace_back(arg);
				break;

			case LUA_TNUMBER:
				if (lua_isinteger(L, i) != 0) {
					arg.first = format_impl::AbstractNode::ArgType::kSigned;
					arg.second.signed_val = luaL_checkint32(L, i);
					fmt_args.emplace_back(arg);
				} else {
					arg.first = format_impl::AbstractNode::ArgType::kFloat;
					arg.second.float_val = luaL_checknumber(L, i);
					fmt_args.emplace_back(arg);
				}
				break;

			case LUA_TBOOLEAN:
				arg.first = format_impl::AbstractNode::ArgType::kBoolean;
				arg.second.boolean_val = luaL_checkboolean(L, i);
				fmt_args.emplace_back(arg);
				break;

			case LUA_TSTRING:
				arg.first = format_impl::AbstractNode::ArgType::kString;
				arg.second.string_val = luaL_checkstring(L, i);
				fmt_args.emplace_back(arg);
				break;

			case LUA_TTABLE:
			case LUA_TFUNCTION:
			case LUA_TUSERDATA:
			case LUA_TTHREAD:
			case LUA_TLIGHTUSERDATA: {
				const std::string type = lua_typename(L, lua_type(L, i));
				report_error(L, "Cannot format the given type %s at index %i", type.c_str(), i);
				NEVER_HERE();  // as report_error will never return
			}

			default: {
				const std::string type = lua_typename(L, lua_type(L, i));
				throw LuaError("Unexpected type " + type + " is not supported");
			}
			}
		}

		lua_pushstring(L, format(luaL_checkstring(L, 1), fmt_args));
		return 1;
	} catch (const std::exception& err) {
		report_error(L, "Error in bformat: %s", err.what());
	}
}

using TextdomainInfo = std::pair<std::string, bool /* addon */>;
static std::map<const lua_State*, std::vector<TextdomainInfo>> textdomains;

/* RST
.. function:: push_textdomain(domain [, addon=false])

   Sets the textdomain for all further calls to :func:`_` until it is reset
   to the previous value using :func:`pop_textdomain`.

   If your script is part of an add-on, the second parameter needs to be `true`.

   :arg domain: The textdomain
   :type domain: :class:`string`
   :returns: :const:`nil`
*/
static int L_push_textdomain(lua_State* L) {
	textdomains[L].emplace_back(
	   luaL_checkstring(L, 1), lua_gettop(L) > 1 && luaL_checkboolean(L, 2));
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

static const TextdomainInfo* current_textdomain(const lua_State* L) {
	const auto it = textdomains.find(L);
	return it == textdomains.end() || it->second.empty() ? nullptr : &it->second.back();
}

constexpr uint16_t kCurrentPacketVersion = 1;
void read_textdomain_stack(FileRead& fr, const lua_State* L) {
	{
		const auto it = textdomains.find(L);
		if (it != textdomains.end()) {
			it->second.clear();
		}
	}

	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			for (size_t i = fr.unsigned_32(); i > 0u; --i) {
				const std::string str = fr.string();
				const bool a = fr.unsigned_8() != 0u;
				textdomains[L].emplace_back(str, a);
			}
		} else {
			throw Widelands::UnhandledVersionError(
			   "read_textdomain_stack", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("LuaGlobals::read_textdomain_stack: %s", e.what());
	}
}
void write_textdomain_stack(FileWrite& fw, const lua_State* L) {
	fw.unsigned_16(kCurrentPacketVersion);

	const auto it = textdomains.find(L);
	if (it == textdomains.end()) {
		fw.unsigned_32(0);
	} else {
		fw.unsigned_32(it->second.size());
		for (const auto& pair : it->second) {
			fw.string(pair.first);
			fw.unsigned_8(pair.second ? 1 : 0);
		}
	}
}

/* RST
.. function:: _(str)

   This peculiar function is used to translate texts in your scenario into
   another language. The function takes a single string, grabs the
   textdomain of your map (which is usually the maps name) and returns the
   translated string. Make sure that you separate translatable and untranslatable
   stuff:

   .. code-block:: lua

      s = "<p><br>" .. _("Only this should be translated") .. "<br></p>"

   :arg str: text to translate.
   :type str: :class:`string`
   :returns: The translated string.
*/
CLANG_DIAG_RESERVED_IDENTIFIER_OFF
// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
static int L__(lua_State* L) {
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	if (const TextdomainInfo* td = current_textdomain(L)) {
		if (td->second) {
			std::unique_ptr<i18n::GenericTextdomain> dom(
			   AddOns::create_textdomain_for_addon(td->first));
			lua_pushstring(L, i18n::translate(luaL_checkstring(L, 1)));
		} else {
			i18n::Textdomain dom(td->first);
			lua_pushstring(L, i18n::translate(luaL_checkstring(L, 1)));
		}
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

   Example to show how it works:

   .. code-block:: lua

      local count = _get_items()                -- count can be 0 or more
      local text  = ngettext("You have %1$d item", "You have %1$d items", count):bformat(count)

   .. note::
      The singular and plural forms should be identical except for the plural form itself.
      If you want to special-case the number ``1`` (or any other number),
      or if you don't want to include the ``count`` variable in the string,
      do *not* use ``ngettext``, but rather an ``if``-``else``-construct. Example:

   .. code-block:: lua

      local count = _get_items()  -- count can be 0 or more
      local text

      if count == 0 then
         text = _("You have no items.")
      elseif count == 1 then
         text = _("You have only one item.")
      else
         text = _("You have a lot of items.")
      end

      -- Note the _() function for translation.
      -- The arguments to ngettext do not require an additional _() call.
*/
static int L_ngettext(lua_State* L) {
	//  S: msgid msgid_plural n
	const char* msgid = luaL_checkstring(L, 1);
	const char* msgid_plural = luaL_checkstring(L, 2);
	const int32_t n = luaL_checkint32(L, 3);
	if (n < 0) {
		report_error(L, "Call to ngettext with negative number %d", n);
	}

	if (const TextdomainInfo* td = current_textdomain(L)) {
		if (td->second) {
			std::unique_ptr<i18n::GenericTextdomain> dom(
			   AddOns::create_textdomain_for_addon(td->first));
			lua_pushstring(L, ngettext(msgid, msgid_plural, n));
		} else {
			i18n::Textdomain dom(td->first);
			lua_pushstring(L, ngettext(msgid, msgid_plural, n));
		}
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
static int L_pgettext(lua_State* L) {
	//  S: msgctxt msgid
	const char* msgctxt = luaL_checkstring(L, 1);
	const char* msgid = luaL_checkstring(L, 2);

	if (const TextdomainInfo* td = current_textdomain(L)) {
		if (td->second) {
			std::unique_ptr<i18n::GenericTextdomain> dom(
			   AddOns::create_textdomain_for_addon(td->first));
			lua_pushstring(L, pgettext(msgctxt, msgid));
		} else {
			i18n::Textdomain dom(td->first);
			lua_pushstring(L, pgettext(msgctxt, msgid));
		}
	} else {
		lua_pushstring(L, pgettext(msgctxt, msgid));
	}
	return 1;
}

/* RST
.. function:: npgettext(msgctxt, msgid, msgid_plural, n)

   A wrapper for the npgettext() function, needed for allowing multiple translations of the same
   plural string according to context.

   :arg msgctxt: a named context for this string for disambiguation
   :type msgctxt: :class:`string`
   :arg msgid: text to translate
   :type msgid: :class:`string`
   :arg msgid_plural: text to translate (plural)
   :type msgid_plural: :class:`string`
   :arg n: The number of elements.
   :type n: An unsigned integer.

   :returns: The translated string.
*/
static int L_npgettext(lua_State* L) {
	//  S: msgctxt msgid
	const char* msgctxt = luaL_checkstring(L, 1);
	const char* msgid = luaL_checkstring(L, 2);
	const char* msgid_plural = luaL_checkstring(L, 3);
	const int32_t n = luaL_checkint32(L, 4);
	if (n < 0) {
		report_error(L, "Call to npgettext with negative number %d", n);
	}

	if (const TextdomainInfo* td = current_textdomain(L)) {
		if (td->second) {
			std::unique_ptr<i18n::GenericTextdomain> dom(
			   AddOns::create_textdomain_for_addon(td->first));
			lua_pushstring(L, npgettext(msgctxt, msgid, msgid_plural, n));
		} else {
			i18n::Textdomain dom(td->first);
			lua_pushstring(L, npgettext(msgctxt, msgid, msgid_plural, n));
		}
	} else {
		lua_pushstring(L, npgettext(msgctxt, msgid, msgid_plural, n));
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
   "1.1" (for a release),
   "1.2~git26354 (4ba897c@master)" (development for 1.2) or
   "build-16[debug]" (old, before version 1.0).
*/
static int L_get_build_id(lua_State* L) {
	lua_pushstring(L, build_id());
	return 1;
}

/* Use our own logging function as a replacement for Lua's built-in print(). */
static int L_print(lua_State* L) {
	std::string message;

	for (int i = 1; i <= lua_gettop(L); ++i) {
		if (i > 1) {
			message += ' ';
		}

		switch (lua_type(L, i)) {
		case LUA_TNIL:
			message += "(nil)";
			break;
		case LUA_TBOOLEAN:
			message += luaL_checkboolean(L, i) ? "true" : "false";
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			message += luaL_checkstring(L, i);
			break;
		default:
			message += as_string(format("[%s @ %p]", luaL_typename(L, i), lua_topointer(L, i)));
			break;
		}
	}

	do_log(LogType::kLua, Time(), "%s", message.c_str());
	return 0;
}

/* RST
.. function:: play_sound(file[, priority = 100, allow_multiple = true, field = nil])

   .. versionadded:: 1.3

   Play a sound effect.

   See :ref:`the playsound program <map_object_programs_playsound>` for information
   on how the file has to be provided and the meaning of the optional arguments.
   Only ``.ogg`` files are supported.

   If a field is provided, the sound is played in stereo,
   and only if the player can hear sounds on the given field.

   The volume of the sound and whether the sound will be played at all are determined
   by the user's settings for ambient sounds.

   :arg file: The path and basename of the sound effect to play
      (without the .ogg filename extension and the optional ``_??`` numbering).
   :type file: :class:`string`
   :arg priority: The priority of the sound in percent.
   :type priority: :class:`number`
   :arg allow_multiple: Whether the sound may be played
      even when another instance of it is already playing.
   :type allow_multiple: :class:`boolean`
   :arg field: The map position of the sound, if any.
   :type field: :class:`wl.map.Field`
*/
static int L_play_sound(lua_State* L) {
	const int nargs = lua_gettop(L);
	if (nargs < 1 || nargs > 4) {
		report_error(L, "Wrong number of arguments");
	}

	const FxId fx = SoundHandler::register_fx(SoundType::kAmbient, luaL_checkstring(L, 1));

	const int32_t priority =
	   nargs < 2 ? kFxMaximumPriority : static_cast<int32_t>(luaL_checknumber(L, 2) * 100);
	if (priority < kFxPriorityLowest || priority > kFxMaximumPriority) {
		report_error(L, "Priority %f%% is out of bounds %f..%f", priority / 100.0,
		             kFxPriorityLowest / 100.0, kFxMaximumPriority / 100.0);
	}

	const bool allow_multiple = nargs < 3 || luaL_checkboolean(L, 3);

	if (nargs < 4 || lua_isnil(L, 4)) {
		g_sh->play_fx(SoundType::kAmbient, fx, priority, allow_multiple);
	} else {
		LuaMaps::LuaField* coords = *get_user_class<LuaMaps::LuaField>(L, 4);
		Notifications::publish(
		   NoteSound(SoundType::kAmbient, fx, coords->coords(), priority, allow_multiple));
	}

	return 0;
}

static int L_deleted_function(lua_State*) {
	throw LuaError("call to a removed function");
}

/* Always append new globals to the end, and never remove any list entries.
 * Otherwise you break savegame compatibility.
 */
const static struct luaL_Reg globals[] = {{"_", &L__},
                                          {"get_build_id", &L_get_build_id},
                                          {"include", &L_include},
                                          {"ngettext", &L_ngettext},
                                          {"pgettext", &L_pgettext},
                                          {"npgettext", &L_npgettext},
                                          {"set_textdomain", &L_deleted_function},
                                          {"push_textdomain", &L_push_textdomain},
                                          {"pop_textdomain", &L_pop_textdomain},
                                          {"ticks", &L_ticks},
                                          {"play_sound", &L_play_sound},
                                          {"print", &L_print},
                                          {nullptr, nullptr}};

void luaopen_globals(lua_State* L) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
	luaL_setfuncs(L, globals, 0);
	lua_pop(L, 1);

	// Also add in string.bformat to use format instead, so that we get
	// proper localisation.
	lua_getglobal(L, "string");               // S: string_lib
	lua_pushstring(L, "bformat");             // S: string_lib "bformat"
	lua_pushcfunction(L, &L_string_bformat);  // S: string_lib "bformat" function
	lua_settable(L, -3);                      // S: string_lib
	lua_pop(L, 1);
}

}  // namespace LuaGlobals
