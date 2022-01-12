/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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

#include "network/internet_gaming_messages.h"

#include <map>

#include "base/i18n.h"

static std::map<std::string, std::string> igmessages;

/// Returns a translated message fitting to the message code \arg code
const std::string& InternetGamingMessages::get_message(const std::string& code) {
	if (igmessages.find(code) != igmessages.end()) {
		return igmessages[code];
	}
	// if no message for code was found, just return code
	return code;
}

/// Fills the map.
/// This function should be called *before* the first call of "get_message", but only after the
/// locales
/// are loaded.
void InternetGamingMessages::fill_map() {
	// Messages from the metaserver (https://github.com/widelands/widelands_metaserver) to the
	// clients.
	igmessages["NO_SUCH_USER"] = _("There is no user with this name logged in.");
	igmessages["NO_SUCH_GAME"] = _("The game no longer exists, maybe it has just been closed.");
	igmessages["WRONG_PASSWORD"] = _("Wrong password, please try again.");
	igmessages["BANNED"] = _("You have been temporarily banned from online gaming.");
	igmessages["UNSUPPORTED_PROTOCOL"] = _("The protocol version you are using is not supported!");
	igmessages["ALREADY_LOGGED_IN"] = _("You are already logged in!");
	igmessages["DEFICIENT_PERMISSION"] =
	   _("You got disconnected, as you sent a superuser command without superuser permission. "
	     "This incident will be logged and reported to the administrator.");
	// Messages from the clients to the metaserver
	igmessages["CONNECTION_CLOSED"] = _("Connection was closed by the client normally.");
	igmessages["CONNECTION_LOST"] = _("The connection to the metaserver was lost.");
	igmessages["NO_ANSWER"] = _("Metaserver did not answer");
	igmessages["CLIENT_TIMEOUT"] =
	   _("You got disconnected from the metaserver, as you did not answer a PING request in time.");
}
