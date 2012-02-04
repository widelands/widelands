/*
 * Copyright (C) 2004-2006, 2008-2009, 2012 by the Widelands Development Team
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


#include "internet_gaming.h"
#include "internet_gaming_messages.h"

InternetGaming::InternetGaming() :
	m_state       (OFFLINE),
	m_maxclients  (1),
	clientupdate  (false),
	tableupdate   (false)
{
	// Fill the list of possible messages from the server
	InternetGamingMessages::fill_map();
}

static InternetGaming * ig = 0;

InternetGaming & InternetGaming::ref() {
	if (not ig)
		ig = new InternetGaming();
	return * ig;
}


/// Login to metaserver
bool InternetGaming::login
	(std::string const & nick, std::string const & pwd, bool registered,
	 std::string const & metaserver, uint32_t port)
{return false;}


/// logout of the metaserver
void InternetGaming::logout() {}


/// handles all communication between the metaserver and the client
void InternetGaming::handle_metaserver_communication() {}


/// \returns the ip of the game the client is on or wants to join (or the client is hosting)
///          or 0, if no ip available.
char const * InternetGaming::ip() {return 0;}


/// called by a client to join the game \arg gamename
void InternetGaming::join_game(const std::string & gamename) {}


/// called by a client to open a new game with name \arg gamename
void InternetGaming::open_game() {}


/// called by a client that is host of a game to inform the metaserver, that the game started
void InternetGaming::set_game_playing() {}


/// called by a client that is host of a game to inform the metaserver, that the game was ended.
void InternetGaming::set_game_done() {}


/// \returns whether the local gamelist was updated
bool InternetGaming::updateForGames() {
	bool temp = tableupdate;
	tableupdate = false;
	return temp;
}

/// \returns the tables in the room
std::vector<Net_Game_Info> const & InternetGaming::games() {
	return gamelist;
}

/// \returns whether the local clientlist was updated
bool InternetGaming::updateForClients() {
	bool temp = clientupdate;
	clientupdate = false;
	return temp;
}

/// \returns the players in the room
std::vector<Net_Client>    const & InternetGaming::clients() {
	return clientlist;
}


/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(std::string const & msg) {}

