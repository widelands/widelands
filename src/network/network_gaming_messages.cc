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

#include "network/network_gaming_messages.h"

#include <cassert>
#include <map>

#include "base/i18n.h"
#include "base/string.h"

// Table assigning printable messages to message codes
static const std::map<std::string, std::string> ngmessages = {
   // Messages from metaserver to client
   {"CLIENT_LEFT_GAME", gettext_noop("Client has left the game.")},
   {"CLIENT_CRASHED", gettext_noop("Client crashed and performed an emergency save.")},
   {"CONNECTION_LOST", gettext_noop("Connection was lost.")},
   {"SERVER_LEFT", gettext_noop("Server has left the game.")},
   {"GAME_STARTED_AT_CONNECT",
    gettext_noop("The game has started just after you tried to connect.")},
   {"SERVER_CRASHED", gettext_noop("Server has crashed and performed an emergency save.")},
   {"CLIENT_DESYNCED", gettext_noop("Client and host have become desynchronized.")},
   {"KICKED", gettext_noop("Kicked by the host: %s")},
   {"MALFORMED_COMMANDS", gettext_noop("Client sent malformed commands: %s")},
   {"SOMETHING_WRONG", gettext_noop("Something went wrong: %s")},
   {"CLIENT_X_LEFT_GAME", gettext_noop("%1$s has left the game (%2$s)")},
   {"CLIENT_X_REPLACED_WITH", gettext_noop("%1$s has been replaced with %2$s")},
   {"UNKNOWN_LEFT_GAME", gettext_noop("Unknown user has left the game (%s)")},
   {"SYNCREQUEST_WO_GAME",
    gettext_noop("Server sent a SYNCREQUEST even though no game is running.")},
   {"PLAYERCMD_WO_GAME", gettext_noop("Received a PLAYERCOMMAND even though no game is running.")},
   {"UNEXPECTED_LAUNCH", gettext_noop("Unexpectedly received LAUNCH command from server.")},
   {"PLAYER_UPDATE_FOR_N_E_P",
    gettext_noop("Server sent a player update for a player that does not exist.")},
   {"USER_UPDATE_FOR_N_E_U",
    gettext_noop("Server sent a user update for a user that does not exist.")},
   {"DIFFERENT_PROTOCOL_VERS", gettext_noop("Server uses a different protocol version")},
   {"PROTOCOL_EXCEPTION",
    gettext_noop("Received command number %s, which is not allowed in this state.")},
   {"BACKWARDS_RUNNING_TIME",
    gettext_noop("Client reports time to host that is running backwards.")},
   {"SIMULATING_BEYOND_TIME",
    gettext_noop("Client simulates beyond the game time allowed by the host.")},
   {"CLIENT_SYNC_REP_TIMEOUT", gettext_noop("Client did not submit sync report in time.")},
   {"GAME_ALREADY_STARTED", gettext_noop("The game has already started.")},
   {"NO_ACCESS_TO_PLAYER", gettext_noop("Client has no access to other player’s settings.")},
   {"NO_ACCESS_TO_SERVER", gettext_noop("Client has no access to server settings.")},
   {"TIME_SENT_NOT_READY",
    gettext_noop("Client sent TIME command even though game is not running.")},
   {"PLAYERCMD_FOR_OTHER", gettext_noop("Client sent a PLAYERCOMMAND for another player.")},
   {"UNEXPECTED_SYNC_REP", gettext_noop("Client sent unexpected synchronization report.")},
   {"REQUEST_OF_N_E_FILE",
    gettext_noop("Client requests file although none is available to send.")},
   {"SENT_PLAYER_TO_LOBBY", gettext_noop("Host sent player %s to the lobby!")},
   {"DIFFERENT_WL_VERSION",
    gettext_noop("WARNING: %1$s uses version: %2$s, while Host uses version: %3$s")},
   {"CLIENT_HAS_JOINED_GAME", gettext_noop("%s has joined the game")},
   {"STARTED_SENDING_FILE", gettext_noop("Started to send file %1$s to %2$s!")},
   {"COMPLETED_FILE_TRANSFER", gettext_noop("Completed transfer of file %1$s to %2$s")},
   {"PLAYER_DEFEATED", gettext_noop("The player ‘%s’ was defeated and became a spectator.")},
   {"CLIENT_HUNG", gettext_noop("Client %1$s did not answer for more than %2$s.")},
   {"CHEAT", gettext_noop("Client %s is cheating!")}};

/// Returns a translated message fitting to the message code \arg code
const std::string NetworkGamingMessages::get_message(const std::string& code) {
	if (ngmessages.find(code) != ngmessages.end()) {
		return _(ngmessages.at(code));
	}
	// if no message for code was found, just return code
	return code;
}

/// Returns a translated message fitting to the message code \arg code and all arguments
const std::string NetworkGamingMessages::get_message(const std::string& code,
                                                     const std::string& arg1,
                                                     const std::string& arg2,
                                                     const std::string& arg3) {

	if (ngmessages.find(code) == ngmessages.end()) {
		return (
		   format("%s, %s, %s, %s", code, get_message(arg1), get_message(arg2), get_message(arg3)));
	}

	// clang-tidy says this shouldn't be const because it may be returned directly in case 0 below
	std::string msg_translated = _(ngmessages.at(code));

	const size_t n_fmt_arg = format_arguments_count(msg_translated);
	assert(n_fmt_arg <= 3);

	switch (n_fmt_arg) {
	case 0:
		return msg_translated;
	case 1:
		return format(msg_translated, arg1);
	case 2:
		return format(msg_translated, arg1, arg2);
	case 3:
		return format(msg_translated, arg1, arg2, arg3);
	default:
		NEVER_HERE();
	}
}
