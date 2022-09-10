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
#include "base/log.h"
#include "base/string.h"

struct MessageString {
	std::string str;
	size_t n_arg;
};
static std::map<std::string, MessageString> ngmessages;

/// Returns a translated message fitting to the message code \arg code
const std::string NetworkGamingMessages::get_message(const std::string& code) {
	if (ngmessages.find(code) != ngmessages.end()) {
		return ngmessages[code].str;
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

	assert(ngmessages[code].n_arg <= 3);

	// for easier handling
	std::vector<std::string> args;
	args.push_back(arg1);
	args.push_back(arg2);
	args.push_back(arg3);

	for (uint8_t i = 0; i < ngmessages[code].n_arg; ++i) {
		if (args[i].empty()) {
			log_warn(
			   "NetworkGamingMessages::get_message: %s: missing argument %d", code.c_str(), i + 1);
		}
	}
	for (uint8_t i = ngmessages[code].n_arg; i < 3; ++i) {
		if (!args[i].empty()) {
			log_warn("NetworkGamingMessages::get_message: %s: unexpected argument %d: %s",
			         code.c_str(), i + 1, args[i].c_str());
		}
	}

	switch (ngmessages[code].n_arg) {
	case 0:
		return ngmessages[code].str;
	case 1:
		return format(ngmessages[code].str, arg1);
	case 2:
		return format(ngmessages[code].str, arg1, arg2);
	case 3:
		return format(ngmessages[code].str, arg1, arg2, arg3);
	default:
		NEVER_HERE();
	}
}

/// Fills the map.
/// This function should be called *before* the first call of "get_message", but only after the
/// locales
/// are loaded.
void NetworkGamingMessages::fill_map() {
	// messages from metaserver to client
	ngmessages["CLIENT_LEFT_GAME"] = {_("Client has left the game."), 0};
	ngmessages["CLIENT_CRASHED"] = {_("Client crashed and performed an emergency save."), 0};
	ngmessages["CONNECTION_LOST"] = {_("Connection was lost."), 0};
	ngmessages["SERVER_LEFT"] = {_("Server has left the game."), 0};
	ngmessages["GAME_STARTED_AT_CONNECT"] = {
	   _("The game has started just after you tried to connect."), 0};
	ngmessages["SERVER_CRASHED"] = {_("Server has crashed and performed an emergency save."), 0};
	ngmessages["CLIENT_DESYNCED"] = {_("Client and host have become desynchronized."), 0};
	ngmessages["KICKED"] = {_("Kicked by the host: %s"), 1};
	ngmessages["MALFORMED_COMMANDS"] = {_("Client sent malformed commands: %s"), 1};
	ngmessages["SOMETHING_WRONG"] = {_("Something went wrong: %s"), 1};
	ngmessages["CLIENT_X_LEFT_GAME"] = {_("%1$s has left the game (%2$s)"), 2};
	ngmessages["CLIENT_X_REPLACED_WITH"] = {_("%1$s has been replaced with %2$s"), 2};
	ngmessages["UNKNOWN_LEFT_GAME"] = {_("Unknown user has left the game (%s)"), 1};
	ngmessages["SYNCREQUEST_WO_GAME"] = {
	   _("Server sent a SYNCREQUEST even though no game is running."), 0};
	ngmessages["PLAYERCMD_WO_GAME"] = {
	   _("Received a PLAYERCOMMAND even though no game is running."), 0};
	ngmessages["UNEXPECTED_LAUNCH"] = {_("Unexpectedly received LAUNCH command from server."), 0};
	ngmessages["PLAYER_UPDATE_FOR_N_E_P"] = {
	   _("Server sent a player update for a player that does not exist."), 0};
	ngmessages["USER_UPDATE_FOR_N_E_U"] = {
	   _("Server sent a user update for a user that does not exist."), 0};
	ngmessages["DIFFERENT_PROTOCOL_VERS"] = {_("Server uses a different protocol version"), 0};
	ngmessages["PROTOCOL_EXCEPTION"] = {_("Received command number %s,"
	                                      " which is not allowed in this state."),
	                                    1};
	ngmessages["BACKWARDS_RUNNING_TIME"] = {
	   _("Client reports time to host that is running backwards."), 0};
	ngmessages["SIMULATING_BEYOND_TIME"] = {
	   _("Client simulates beyond the game time allowed by the host."), 0};
	ngmessages["CLIENT_SYNC_REP_TIMEOUT"] = {_("Client did not submit sync report in time."), 0};
	ngmessages["GAME_ALREADY_STARTED"] = {_("The game has already started."), 0};
	ngmessages["NO_ACCESS_TO_PLAYER"] = {_("Client has no access to other player’s settings."), 0};
	ngmessages["NO_ACCESS_TO_SERVER"] = {_("Client has no access to server settings."), 0};
	ngmessages["TIME_SENT_NOT_READY"] = {
	   _("Client sent TIME command even though game is not running."), 0};
	ngmessages["PLAYERCMD_FOR_OTHER"] = {_("Client sent a PLAYERCOMMAND for another player."), 0};
	ngmessages["UNEXPECTED_SYNC_REP"] = {_("Client sent unexpected synchronization report."), 0};
	ngmessages["REQUEST_OF_N_E_FILE"] = {
	   _("Client requests file although none is available to send."), 0};
	ngmessages["SENT_PLAYER_TO_LOBBY"] = {_("Host sent player %s to the lobby!"), 1};
	ngmessages["DIFFERENT_WL_VERSION"] = {
	   _("WARNING: %1$s uses version: %2$s, while Host uses version: %3$s"), 3};
	ngmessages["CLIENT_HAS_JOINED_GAME"] = {_("%s has joined the game"), 1};
	ngmessages["STARTED_SENDING_FILE"] = {_("Started to send file %1$s to %2$s!"), 2};
	ngmessages["COMPLETED_FILE_TRANSFER"] = {_("Completed transfer of file %1$s to %2$s"), 2};
	ngmessages["PLAYER_DEFEATED"] = {_("The player ‘%s’ was defeated and became a spectator."), 1};
	ngmessages["CLIENT_HUNG"] = {_("Client %1$s did not answer for more than %2$s."), 2};
	ngmessages["CHEAT"] = {_("Client %s is cheating!"), 1};
}
