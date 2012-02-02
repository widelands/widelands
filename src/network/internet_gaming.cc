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
#include "network_ggz.h"

//static InternetGaming * ig = 0;

InternetGaming & InternetGaming::ref() {
	return NetGGZ::ref();
#if 0
	if (not ig)
		ig = new InternetGaming();
	return * ig;
#endif
}

InternetGaming::InternetGaming() :
	maxclients    (1)
{
	// Fill the list of possible messages from the server
	InternetGamingMessages::fill_map();
}

/// \returns the tables in the room
std::vector<Net_Game_Info> const & InternetGaming::games() {
	return gamelist;
}

/// \returns the players in the room
std::vector<Net_Client>    const & InternetGaming::clients() {
	return clientlist;
}

#if 0 // to be implemented
/// ChatProvider: sends a message via the metaserver.
void InternetGaming::send(std::string const & msg) {};
#endif
