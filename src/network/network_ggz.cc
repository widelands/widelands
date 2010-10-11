/*
 * Copyright (C) 2004-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "network_ggz.h"

#ifdef HAVE_GGZ

#include "log.h"
#include "i18n.h"
#include "warning.h"
#include "wexception.h"
#include "wlapplication.h"
#include "container_iterate.h"
#include "build_info.h"


#include "ggz_ggzcore.h"
#include "ggz_ggzmod.h"
#include "ggz_wlmodule.h"

#include <cstring>

static NetGGZ    * ggzobj    = 0;

/// Constructor of NetGGZ.
/// It is private to avoid the creation of more than one ggz object.
NetGGZ::NetGGZ() :
	tableseats    (1),
	motd          (),
	mapname       (),
	map_w         (-1),
	map_h         (-1),
	win_condition (gametype_endless)
{}


/// \returns the _one_ ggzobject. There should be only this one object and it
/// _must_ be used in all cases.
NetGGZ & NetGGZ::ref() {
	if (!ggzobj)
		ggzobj = new NetGGZ();
	return *ggzobj;
}

void NetGGZ::deinit()
{
	ggz_ggzmod::ref().disconnect();
	ggz_ggzcore::ref().deinit();
}


/// \returns the ip of the server, if connected
char const * NetGGZ::ip()
{
	return ggz_wlmodule::ref().get_server_ip();;
}

#include "ggzmod.h"


/// initializes the local ggz core
/// @return
bool NetGGZ::initcore
	(char const * const metaserver, char const * const nick,
	 char const * const pwd, bool registered)
{
	if (ggz_mode())
		throw wexception("Initcore was called but we are in ggz mode");

	username = nick;

	const char * debugstrs[] =
		{//GGZCORE_DBG_GAME,
		 GGZCORE_DBG_HOOK,
		 GGZCORE_DBG_MODULE,
		 GGZCORE_DBG_NET,
		 GGZCORE_DBG_SERVER,
		 //GGZCORE_DBG_POLL,
		 GGZCORE_DBG_ROOM,
		 GGZCORE_DBG_STATE,
		 GGZCORE_DBG_TABLE,
		 GGZCORE_DBG_XML,
		 //GGZ_SOCKET_DEBUG,
		 "GGZMOD",
		NULL};
	ggz_debug_init(debugstrs, 0);
	//ggz_debug_set_func();
	//ggz_debug_enable();
	
	ggz_ggzcore::ref().init(metaserver, nick, pwd, registered);
	while
		(ggz_ggzcore::ref().is_connecting() or (ggz_ggzcore::ref().logged_in()
		 and not ggz_ggzcore::ref().is_in_room()))
	{
		ggz_ggzcore::ref().process();
	}

	return ggz_ggzcore::ref().logged_in();
}

void NetGGZ::process()
{
	int i = 100;
	do
	{
		if (not ggz_mode())
			ggz_ggzcore::ref().process();
		ggz_ggzmod::ref().process();
		ggz_wlmodule::ref().process();
		//log
		//	("GGZ ## process loop: %i, %i, %i\n",
		//	 ggz_ggzcore::ref().data_pending(),
		//	 ggz_ggzmod::ref().data_pending(),
		//	 ggz_wlmodule::ref().data_pending());
		i--;
	} while
		((ggz_ggzcore::ref().data_pending()
		  or ggz_ggzmod::ref().data_pending()
		  or ggz_wlmodule::ref().data_pending())
		 and i);

	if (i <= 0)
		log("GGZ ## process loop exited by limit\n");

	if (
		 (ggz_ggzcore::ref().get_tablestate() == ggz_ggzcore::ggzcoretablestate_launched
		  or ggz_ggzcore::ref().get_tablestate() == ggz_ggzcore::ggzcoretablestate_joined)
		 and not ggz_ggzmod::ref().connected())
	{
		ggz_ggzmod::ref().init();
		ggz_ggzmod::ref().connect();
	}
}

/// \returns the maximum number of seats in a widelands table (game)
uint32_t NetGGZ::max_players()
{
	//  FIXME problem in ggz - for some reasons only 8 seats are currently
	//  FIXME available. I already posted this problem to the ggz
	//  FIXME mailinglist. -- nasenbaer
	//
	//  FIXME due to a bug in ggz 0.14.1 we may even only support <= 7 seats
	//  FIXME this should be changed once the next official ggz version is
	//  FIXME released and support for ggz 0.14.1 is removed from widelands src
	//return gametype ? 8 : 1;
	//  FIXME it is not a bug in 0.14.1 but a general problem in ggz. Using
	//  FIXME ggzcore and ggzmod from the same thread causes a deadlock between
	//  FIXME the ggz libs. -- timowi
	int maxplayers = ggz_ggzcore::ref().get_max_players();
	return maxplayers;
	//return (maxplayers > 7)?7:maxplayers;
}


/// Tells the metaserver that the game started
void NetGGZ::send_game_playing()
{
	if (ggz_mode() or ggz_ggzcore::ref().is_in_table()) {
		int fd = ggz_ggzmod::ref().datafd();
		if (ggz_write_int(fd, op_state_playing) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


/// Tells the metaserver that the game is done
void NetGGZ::send_game_done()
{
	if (ggz_mode() or ggz_ggzcore::ref().is_in_table()) {
		int fd = ggz_ggzmod::ref().datafd();
		if (ggz_write_int(fd, op_state_done) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


void NetGGZ::send_game_info()
{
	ggz_wlmodule::ref().send_game_info
		(mapname, map_w, map_h, win_condition, playerinfo);
}

void NetGGZ::send_game_statistics
	(int32_t gametime,
	 const Widelands::Game::General_Stats_vector & resultvec)
{
	ggz_wlmodule::ref().send_statistics(gametime, resultvec, playerinfo);
}

void NetGGZ::report_result
	(int32_t player, Widelands::TeamNumber team, int32_t points,
	 bool win, int32_t gametime,
	 const Widelands::Game::General_Stats_vector& resultvec)
{
	log
		("NetGGZ::report_result(%d, %d, %s)\n", player, points,
		 win?"won":"lost");
	//log("NetGGZ::report_result: player %i/%i\n", player, playerinfo.size());

	if(player < 1 or player > static_cast<int32_t>(playerinfo.size()))
	{
		throw wexception
			("NetGGZ::report_result: ERROR: player number out of range\n");
	}
	
	playerinfo.at(player-1).points = points;
	playerinfo.at(player-1).result =
		(win?gamestatresult_winner:gamestatresult_looser);
	playerinfo.at(player-1).team = team;

	bool finished = true;

	for (unsigned int i = 0; i < playerinfo.size(); i++)
	{
		if (playerinfo.at(i).result == gamestatresult_null)
			finished = false;
	}

	//container_iterate(std::vector<Net_Player_Info>, playerinfo, it)
	/*
	std::vector<Net_Player_Info>::iterator pit = playerinfo.begin();
	while (pit != playerinfo.end())
	{
		log("NetGGZ::report_result: no result yet: 1 %s\n", pit->name);
		if (pit->result == gamestatresult_null)
		{
			finished = false;
			log("NetGGZ::report_result: no result yet: %s\n", pit->name);
		}
		pit++;
	}
	*/

	if(finished)
		send_game_statistics(gametime, resultvec);
}


void NetGGZ::set_players(GameSettings & settings)
{
	log
		("NetGGZ: playernum(%d) usernum(%d) win_condition \"%s\"\n",
		 static_cast<int>(settings.playernum),
		 static_cast<int>(settings.usernum),
		 settings.win_condition.c_str());
 
	if (settings.win_condition.compare("00_endless_game") == 0)
		win_condition = gametype_endless;
	else if (settings.win_condition.compare("01_defeat_all") == 0)
		win_condition = gametype_defeatall;
	else if (settings.win_condition.compare("02_collectors") == 0)
		win_condition = gametype_collectors;
	else if (settings.win_condition.compare("03_tribes_together") == 0)
		win_condition = gametype_tribes_together;
	else
		win_condition = gametype_endless;

	std::vector<PlayerSettings>::iterator pit = settings.players.begin();
	int i = 0;
	playerinfo.clear();
	while (pit != settings.players.end())
	{
		log
			("NetGGZ: PlayerSetting: init_index(%d), name(%s), "
			 "ai(%s), tribe(%s)\n",
			 static_cast<int>(pit->initialization_index),
			 pit->name.c_str(), pit->ai.c_str(), pit->tribe.c_str());

		Net_Player_Info player;
		player.name = pit->name;
		player.tribe = pit->tribe;
		player.playernum = i;
		if (pit->ai.compare("Aggressive") == 0)
			player.type = playertype_ai_aggressive;
		else if (pit->ai.compare("Normal") == 0)
			player.type = playertype_ai_normal;
		else if (pit->ai.compare("Defensive") == 0)
			player.type = playertype_ai_defensive;
		else if (pit->ai.compare("None") == 0)
			player.type = playertype_ai_none;
		else if (pit->ai.empty() and not player.name.empty())
			player.type = playertype_human;
		else {
			log
				("NetGGZ: WARNING: Unknown AI %s. Plaer name %s\n",
				 pit->ai.c_str(), pit->name.c_str());
			player.type = playertype_unknown;
		}
		if (not player.name.empty())
			playerinfo.push_back(player);
		pit++; i++;
	}

	std::vector<UserSettings>::iterator uit = settings.users.begin();
	while (uit != settings.users.end())
	{
		log
			("NetGGZ: UserSetting: position(%d), name(%s)\n",
			 static_cast<int>(uit->position), uit->name.c_str());
		uit++;
	}
}

void NetGGZ::set_map(std::string name, int w, int h)
{
	mapname = name;
	map_w = w;
	map_h = h;
}

/// Sends a chat message via ggz room chat
void NetGGZ::send(std::string const & msg)
{
	if (ggz_mode())
		throw wexception
			("NetGGZ::send tried to send chat message but we are in ggz mode");

	if (not ggz_ggzcore::ref().logged_in()){
		log("GGZ ## send chat message: not logged in!\n");
		return;
	}
	if (msg.size() && *msg.begin() == '@') {
		// Format a personal message
		std::string::size_type const space = msg.find(' ');
			if (space >= msg.size() - 1)
			return;
		std::string const to = msg.substr(1, space - 1);
		std::string const pm = msg.substr(space + 1);
		ggz_ggzcore::ref().send_message(to.c_str(), pm.c_str());
		// Add the pm to own message list
		formatedGGZChat(pm, username, false, to);
	} else
		ggz_ggzcore::ref().send_message(NULL, msg.c_str());
}


/// Called when a chatmessage was received.
void NetGGZ::recievedGGZChat(void const * const cbdata)
{
	GGZChatEventData const * const msg =
		static_cast<GGZChatEventData const *>(cbdata);
	formatedGGZChat
		(msg->message,
		 msg->sender,
		 msg->type == GGZ_CHAT_ANNOUNCE,
		 msg->type == GGZ_CHAT_PERSONAL ? username : std::string());
}


/// Send a formated message to the chat menu
void NetGGZ::formatedGGZChat
	(std::string const & msg, std::string const & sender,
	 bool system, std::string recipient)
{
	if (ggz_mode())
		throw wexception
			("NetGGZ::formatedGGZChat tried to send "
			 "chat message but we are in ggz mode");
	ChatMessage c;
	c.time = time(0);
	c.sender = !system && sender.empty() ? "<unknown>" : sender;
	c.playern = system ? -1 : recipient.size() ? 3 : 7;
	c.msg = msg;
	c.recipient = recipient;

	receive(c);
}

void NetGGZ::launch()
{
	if (ggz_mode())
		throw wexception
			("NetGGZ::launch tried to launch a game but we are in ggz mode");
	ggz_ggzcore::ref().launch(tableseats, servername);
}

void NetGGZ::join(char const * tablename)
{
	if (ggz_mode())
		ggz_ggzmod::ref().connect();
	else
		ggz_ggzcore::ref().join(tablename);
}

bool NetGGZ::updateForTables() {
	return ggz_ggzcore::ref().updateForTables();
}
bool NetGGZ::updateForUsers() {
	return ggz_ggzcore::ref().updateForUsers();
}

std::vector<Net_Game_Info> const & NetGGZ::tables() {
	return ggz_ggzcore::ref().tablelist;
}

std::vector<Net_Player>    const & NetGGZ::users() {
	return ggz_ggzcore::ref().userlist;
}

bool NetGGZ::is_connecting()
{
	return ggz_ggzcore::ref().is_connecting();
}

bool NetGGZ::logged_in()
{
	return ggz_ggzcore::ref().logged_in();
}

void NetGGZ::ggzcore_statechange()
{

}

void NetGGZ::ggzmod_statechange()
{
	ggz_wlmodule::ref().set_datafd(ggz_ggzmod::ref().datafd());
}

bool NetGGZ::set_spectator(bool spec)
{
	if(spec)
		ggz_ggzmod::ref().set_spectator();
	else
		ggz_ggzmod::ref().set_player();
}

bool NetGGZ::ggz_mode()
{
	// store the result from ggzmod_is_ggz_mode() because it will return
	// also true if we started a table from our ggzcore
	static bool tested = false;
	if (tested)
		return m_started_from_ggzclient;
	tested = true;
	m_started_from_ggzclient = ggzmod_is_ggz_mode();
	return m_started_from_ggzclient;
}

std::string NetGGZ::playername()
{
	return ggz_ggzmod::ref().playername();
}


#endif
