/*
 * Copyright (C) 2012 by the Widelands Development Team
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

#include "internet_gaming_messages.h"

#include "i18n.h"

#include <map>

static std::map <std::string, std::string> igmessages;

/// Returns a translated message fitting to the message code \arg code
const std::string & InternetGamingMessages::get_message(const std::string & code) {
	if (igmessages.find(code) != igmessages.end())
		return igmessages[code];
	// if no message for code was found, just return code
	return code;
}


/// Fills the map.
/// This function should be called *before* the first call of "get_message", but only after the locales
/// are loaded.
void InternetGamingMessages::fill_map() {
	igmessages["WRONG_PASSWORD"]       = _("The send password was incorrect!");
	igmessages["ALREADY_LOGGED_IN"]    = _("You are already logged in!");
	igmessages["DEFICIENT_PERMISSION"] =
		_
		 ("You got disconnected, as you send a superuser command without superuser permission."
		  "This incident will be logged and reported to the administrator.");
	igmessages["RESTARTING"]           = _("You got disconnected, as the metaserver is currently restarting");
}
