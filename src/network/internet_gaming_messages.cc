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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "network/internet_gaming_messages.h"

#include <map>

#include "base/i18n.h"

static const std::map<std::string, std::string> igmessages = {
   // Messages from the metaserver (https://github.com/widelands/widelands_metaserver) to the
   // clients.
   {"NO_SUCH_USER", gettext_noop("There is no user with this name logged in.")},
   {"NO_SUCH_GAME", gettext_noop("The game no longer exists, maybe it has just been closed.")},
   {"WRONG_PASSWORD", gettext_noop("Wrong password, please try again.")},
   {"BANNED", gettext_noop("You have been temporarily banned from online gaming.")},
   {"UNSUPPORTED_PROTOCOL", gettext_noop("The protocol version you are using is not supported!")},
   {"ALREADY_LOGGED_IN", gettext_noop("You are already logged in!")},
   {"DEFICIENT_PERMISSION",
    gettext_noop(
       "You got disconnected, as you sent a superuser command without superuser permission. "
       "This incident will be logged and reported to the administrator.")},

   // Messages from the clients to the metaserver
   {"CONNECTION_CLOSED", gettext_noop("Connection was closed by the client normally.")},
   {"CONNECTION_LOST", gettext_noop("The connection to the metaserver was lost.")},
   {"NO_ANSWER", gettext_noop("Metaserver did not answer")},
   {"CLIENT_TIMEOUT",
    gettext_noop(
       "You got disconnected from the metaserver, as you did not answer a PING request in time.")}};

/// Returns a translated message fitting to the message code \arg code
const std::string InternetGamingMessages::get_message(const std::string& code) {
	if (igmessages.find(code) != igmessages.end()) {
		return _(igmessages.at(code));
	}
	// if no message for code was found, just return code
	return code;
}
