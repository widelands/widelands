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

#include "network_gaming_messages.h"

#include "i18n.h"
#include "log.h"

#include <map>
#include <boost/format.hpp>
#include <stdint.h>

static std::map <std::string, std::string> ngmessages;

/// Returns a translated message fitting to the message code \arg code
const std::string & NetworkGamingMessages::get_message(const std::string & code) {
	if (ngmessages.find(code) != ngmessages.end())
		return ngmessages[code];
	// if no message for code was found, just return code
	return code;
}

/// Returns a translated message fitting to the message code \arg code and all arguments
const std::string & NetworkGamingMessages::get_message
	(const std::string & code, const std::string & arg1, const std::string & arg2, const std::string & arg3)
{
	if (ngmessages.find(code) == ngmessages.end()) {
		std::string & rv = * new std::string
			((boost::format("%s, %s, %s, %s") % code % get_message(arg1) % get_message(arg2) % get_message(arg3))
			.str());
		return rv;
	}

	// push code and all arguments - if existing - in a vector for easier handling
	std::vector<std::string> strings;
	strings.push_back(get_message(code));
	if (arg1.size() > 0) {
		strings.push_back(get_message(arg1));
		if (arg2.size() > 0) {
			strings.push_back(get_message(arg2));
			if (arg3.size() > 0)
				strings.push_back(get_message(arg3));
		}
	}

	uint8_t last = strings.size() + 1;
	// Try merging the strings
	while (strings.size() > 1 && last > strings.size()) {
		last = strings.size();

		// try to merge the strings from back to front
		try {
			// try to merge last two strings
			std::string temp =
				(boost::format(strings.at(strings.size() - 2)) % strings.at(strings.size() - 1)).str();
			strings.resize(strings.size() - 2);
			strings.push_back(temp);
		} catch (...) {
			if (last < 3)
				break; // no way to merge the two strings;
			try {
				// try to merge last three strings
				std::string temp =
					(boost::format(strings.at(strings.size() - 3))
					 % strings.at(strings.size() - 2) % strings.at(strings.size() - 1))
					.str();
				strings.resize(strings.size() - 3);
				strings.push_back(temp);
			} catch (...)  {
				if (last < 4)
					break; // no way to merge the three strings;
				try {
					// try to merge all four strings
					std::string temp =
						(boost::format(strings.at(strings.size() - 4))
						 % strings.at(strings.size() - 3) % strings.at(strings.size() - 2)
						 % strings.at(strings.size() - 1))
						.str();
					strings.resize(strings.size() - 4);
					strings.push_back(temp);
				} catch (...)  {
					break; // no way to merge all four strings
				}
			}
		}
	}

	// Check if merging succeded
	if (strings.size() == 1)
		return * new std::string(strings.at(0));

	// No, it did not
	std::string & rv = * new std::string
		((boost::format("%s, %s, %s, %s")
		  %  get_message(code) % get_message(arg1) % get_message(arg2) % get_message(arg3))
		.str());
	return rv;

}


/// Fills the map.
/// This function should be called *before* the first call of "get_message", but only after the locales
/// are loaded.
void NetworkGamingMessages::fill_map() {
	// messages from metaserver to client
	ngmessages["CLIENT_LEFT_GAME"]        = _("Client has left the game.");
	ngmessages["CLIENT_CRASHED"]          = _("Client crashed and performed an emergency save.");
	ngmessages["CONNECTION_LOST"]         = _("Connection was lost.");
	ngmessages["SERVER_LEFT"]             = _("Server has left the game.");
	ngmessages["GAME_STARTED_AT_CONNECT"] = _("The game has started just after you tried to connect.");
	ngmessages["SERVER_CRASHED"]          = _("Server has crashed and performed an emergency save.");
	ngmessages["CLIENT_TIMEOUTED"]        = _("Connection to client timeouted: no response for 10 minutes!");
	ngmessages["CLIENT_DESYNCED"]         = _("Client and host have become desynchronized.");
	ngmessages["KICKED"]                  = _("Kicked by the host: %s");
	ngmessages["MALFORMED_COMMANDS"]      = _("Client sent malformed commands: %s");
	ngmessages["SOMETHING_WRONG"]         = _("Something went wrong: %s");
	ngmessages["CLIENT_X_LEFT_GAME"]      = _("%s has left the game (%s)");
	ngmessages["UNKNOWN_LEFT_GAME"]       = _("Unknown user has left the game (%s)");
	ngmessages["SYNCREQUEST_WO_GAME"]     = _("Server sent a SYNCREQUEST even though no game is running.");
	ngmessages["PLAYERCMD_WO_GAME"]       = _("Received a PLAYERCOMMAND even though no game is running.");
	ngmessages["UNEXPECTED_LAUNCH"]       = _("Unexpectedly received LAUNCH command from server.");
	ngmessages["PLAYER_UPDATE_FOR_N_E_P"] = _("Server sent a player update for a player that does not exist.");
	ngmessages["USER_UPDATE_FOR_N_E_U"]   = _("Server sent an user update for a user that does not exist.");
	ngmessages["DIFFERENT_PROTOCOL_VERS"] = _("Server uses a different protocol version");
	ngmessages["PROTOCOL_EXCEPTION"]      = _("Received command number %s, that is disallowed in this state.");
	ngmessages["BACKWARTS_RUNNING_TIME"]  = _("Client reports time to host that is running backwards.");
	ngmessages["SIMULATING_BEYOND_TIME"]  = _("Client simulates beyond the game time allowed by the host.");
	ngmessages["CLIENT_SYNC_REP_TIMEOUT"] = _("Client did not submit sync report in time.");
	ngmessages["GAME_ALREADY_STARTED"]    = _("The game has already started.");
	ngmessages["NO_ACCESS_TO_PLAYER"]     = _("Client has no access to other player's settings.");
	ngmessages["NO_ACCESS_TO_SERVER"]     = _("Client has no access to server settings.");
	ngmessages["START_SENT_NOT_READY"]    = _("Client sent start command, although server is not yet ready.");
	ngmessages["TIME_SENT_NOT_READY"]     = _("Client sent TIME command even though game is not running.");
	ngmessages["PLAYERCMD_FOR_OTHER"]     = _("Client sent a playercommand for another player.");
	ngmessages["UNEXPECTED_SYNC_REP"]     = _("Client sent unexpected synchronization report.");
	ngmessages["REQUEST_OF_N_E_FILE"]     = _("Client requests file although none is available to send.");
	ngmessages["REQUEST_OF_N_E_FILEPART"] = _("Client requests file part that does not exist.");
	ngmessages["SENT_PLAYER_TO_LOBBY"]    = _("Host sent player %s to the lobby!");
	ngmessages["DIFFERENT_WL_VERSION"]    = _("WARNING: %s uses version: %s, while Host uses version: %s");
	ngmessages["CLIENT_HAS_JOINED_GAME"]  = _("%s has joined the game");
	ngmessages["GAME_SAVED_AS"]           = _("Game was saved as %s.");
	ngmessages["STARTED_SENDING_FILE"]    = _("Started to send file %s to %s!");
	ngmessages["COMPLETED_FILE_TRANSFER"] = _("Completed transfer of file %s to %s");
	ngmessages["SENDING_FILE_PART"]       = _("Sending part %s of file %s to %s");
	ngmessages["PLAYER_DEFEATED"]         = _("The player \"%s\" was defeated and became spectator.");
}
