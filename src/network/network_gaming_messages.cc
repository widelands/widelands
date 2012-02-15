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

static std::map <std::string, std::string> igmessages;

/// Returns a translated message fitting to the message code \arg code
const std::string & NetworkGamingMessages::get_message(const std::string & code) {
	if (igmessages.find(code) != igmessages.end())
		return igmessages[code];
	// if no message for code was found, just return code
	return code;
}

/// Returns a translated message fitting to the message code \arg code and all arguments
const std::string & NetworkGamingMessages::get_message
	(const std::string & code, const std::string & arg1, const std::string & arg2, const std::string & arg3)
{
	if (igmessages.find(code) == igmessages.end()) {
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
	igmessages["CLIENT_LEFT_GAME"]        = _("Client has left the game.");
	igmessages["CLIENT_CRASHED"]          = _("Client crashed and performed an emergency save.");
	igmessages["CONNECTION_LOST"]         = _("Connection was lost.");
	igmessages["SERVER_LEFT"]             = _("Server has left the game.");
	igmessages["GAME_STARTED_AT_CONNECT"] = _("The game has started just after you tried to connect.");
	igmessages["SERVER_CRASHED"]          = _("Server has crashed and performed an emergency save.");
	igmessages["CLIENT_TIMEOUTED"]        = _("Connection to client timeouted: no response for 10 minutes!");
	igmessages["CLIENT_DESYNCED"]         = _("Client and host have become desynchronized.");
	igmessages["KICKED"]                  = _("Kicked by the host: %s");
	igmessages["MALFORMED_COMMANDS"]      = _("Client sent malformed commands: %s");
	igmessages["SOMETHING_WRONG"]         = _("Something went wrong: %s");
	igmessages["CLIENT_X_LEFT_GAME"]      = _("%s has left the game (%s)");
	igmessages["UNKNOWN_LEFT_GAME"]       = _("Unknown user has left the game (%s)");
	igmessages["SYNCREQUEST_WO_GAME"]     = _("Server sent a SYNCREQUEST even though no game is running.");
	igmessages["PLAYERCMD_WO_GAME"]       = _("Received a PLAYERCOMMAND even though no game is running.");
	igmessages["UNEXPECTED_LAUNCH"]       = _("Unexpectedly received LAUNCH command from server.");
	igmessages["PLAYER_UPDATE_FOR_N_E_P"] = _("Server sent a player update for a player that does not exist.");
	igmessages["USER_UPDATE_FOR_N_E_U"]   = _("Server sent an user update for a user that does not exist.");
	igmessages["DIFFERENT_PROTOCOL_VERS"] = _("Server uses a different protocol version");
	igmessages["PROTOCOL_EXCEPTION"]      = _("Received command number %s, that is disallowed in this state.");
	igmessages["BACKWARTS_RUNNING_TIME"]  = _("Client reports time to host that is running backwards.");
	igmessages["SIMULATING_BEYOND_TIME"]  = _("Client simulates beyond the game time allowed by the host.");
	igmessages["CLIENT_SYNC_REP_TIMEOUT"] = _("Client did not submit sync report in time.");
	igmessages["GAME_ALREADY_STARTED"]    = _("The game has already started.");
	igmessages["NO_ACCESS_TO_PLAYER"]     = _("Client has no access to other player's settings.");
	igmessages["NO_ACCESS_TO_SERVER"]     = _("Client has no access to server settings.");
	igmessages["START_SENT_NOT_READY"]    = _("Client sent start command, although server is not yet ready.");
	igmessages["TIME_SENT_NOT_READY"]     = _("Client sent TIME command even though game is not running.");
	igmessages["PLAYERCMD_FOR_OTHER"]     = _("Client sent a playercommand for another player.");
	igmessages["UNEXPECTED_SYNC_REP"]     = _("Client sent unexpected synchronization report.");
	igmessages["REQUEST_OF_N_E_FILE"]     = _("Client requests file although none is available to send.");
	igmessages["REQUEST_OF_N_E_FILEPART"] = _("Client requests file part that does not exist.");
	igmessages["SENT_PLAYER_TO_LOBBY"]    = _("Host sent player %s to the lobby!");
	igmessages["DIFFERENT_WL_VERSION"]    = _("WARNING: %s uses version: %s, while Host uses version: %s");
	igmessages["CLIENT_HAS_JOINED_GAME"]  = _("%s has joined the game");
	igmessages["GAME_SAVED_AS"]           = _("Game was saved as %s.");
	igmessages["STARTED_SENDING_FILE"]    = _("Started to send file %s to %s!");
	igmessages["COMPLETED_FILE_TRANSFER"] = _("Completed transfer of file %s to %s");
	igmessages["SENDING_FILE_PART"]       = _("Sending part %s of file %s to %s");
}
