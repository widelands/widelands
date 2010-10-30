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

#ifdef USE_GGZ

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
NetGGZ::NetGGZ():
	tableseats    (1),
	motd          (),
	mapname       (),
	map_w         (-1),
	map_h         (-1),
	win_condition (gametype_endless)
{
m_ggzcore  = new ggz_ggzcore;
m_ggzmod   = new ggz_ggzmod;
m_wlmodule = new ggz_wlmodule;
}

NetGGZ::~NetGGZ()
{
	deinit();
	delete m_wlmodule;
	delete m_ggzmod;
	delete m_ggzcore;
}


/// \returns the _one_ ggzobject. There should be only this one object and it
/// _must_ be used in all cases.
NetGGZ & NetGGZ::ref() {
	if (!ggzobj)
		ggzobj = new NetGGZ();
	return *ggzobj;
}

void NetGGZ::deinit()
{
	ggzmod().disconnect();
	core().deinit();
}


/// \returns the ip of the server, if connected
char const * NetGGZ::ip()
{
	return wlmodule().get_server_ip();;
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
		 //GGZCORE_DBG_XML,
		 //GGZ_SOCKET_DEBUG,
		 "GGZMOD",
		NULL};
	ggz_debug_init(debugstrs, 0);
	//ggz_debug_set_func();
	//ggz_debug_enable();
	
	core().init(metaserver, nick, pwd, registered);
	while
		(core().is_connecting() or (core().logged_in()
		 and not core().is_in_room()))
	{
		core().process();
	}

	return core().logged_in();
}

int NetGGZ::process(int timeout)
{
	fd_set read_fd_set;
	int result;
	struct timeval tv;

	// Do at least on processing step. This may set new filedescriptors.
	if (not ggz_mode())
		core().process();
	ggzmod().process();
	wlmodule().process();

	wlmodule().set_datafd(ggzmod().datafd());

	int i = 100;
	if (timeout > 0) {
		tv.tv_usec = (timeout % 1000) * 1000;
		tv.tv_sec = timeout / 1000;

		FD_ZERO(&read_fd_set);

		core().set_fds(read_fd_set);
		ggzmod().set_fds(read_fd_set);

		result = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);
	}

	// now process all pending data
	while
		((core().data_pending()
		  or ggzmod().data_pending()
		  or wlmodule().data_pending())
		 and i)
	{
		if (not ggz_mode())
			core().process();
		ggzmod().process();
		wlmodule().process();
		//log
		//	("GGZ ## process loop: %i, %i, %i\n",
		//	 core().data_pending(),
		//	 ggzmod().data_pending(),
		//	 wlmodule().data_pending());
		i--;
	} 

	if (i <= 0)
		log("GGZ ## ERROR process loop exited by limit\n");

	if (
		 (core().get_tablestate() == ggz_ggzcore::ggzcoretablestate_launched
		  or core().get_tablestate() == ggz_ggzcore::ggzcoretablestate_joined)
		 and not ggzmod().connected())
	{
		ggzmod().connect();
	}

	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
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
	//
	// I have added experimental support for threadded ggzmod by using
	// boost::threads. With threads enabled it shoulb be possible to host
	// and join games with more than 7 seats -- timowi
	int maxplayers = core().get_max_players();
#ifdef USE_BOOST_THREADS
	return maxplayers;
#else
	return (maxplayers > 7)?7:maxplayers;
#endif
}


/// Tells the metaserver that the game started
void NetGGZ::send_game_playing()
{
	if (ggz_mode() or core().is_in_table()) {
		int fd = ggzmod().datafd();
		if (ggz_write_int(fd, op_state_playing) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


/// Tells the metaserver that the game is done
void NetGGZ::send_game_done()
{
	if (ggz_mode() or core().is_in_table()) {
		int fd = ggzmod().datafd();
		if (ggz_write_int(fd, op_state_done) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


void NetGGZ::send_game_info()
{
	wlmodule().send_game_info
		(mapname, map_w, map_h, win_condition, playerinfo);
}

void NetGGZ::send_game_statistics
	(int32_t gametime,
	 const Widelands::Game::General_Stats_vector & resultvec)
{
// 	wlmodule().send_statistics(gametime, resultvec, playerinfo, playernum);
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
	playerinfo.at(player-1).report_time = gametime;

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
	else if (settings.win_condition.compare("03_territorial_lord") == 0)
		win_condition = gametype_territorial_lord;
	else if (settings.win_condition.compare("03_wood_gnome") == 0)
		win_condition = gametype_wood_gnome;
	else
		win_condition = gametype_endless;
	playernum = settings.playernum;
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
		player.team = pit->team;
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

	if (not core().logged_in()){
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
		core().send_message(to.c_str(), pm.c_str());
		// Add the pm to own message list
		formatedGGZChat(pm, username, false, to);
	} else
		core().send_message(NULL, msg.c_str());
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
	core().launch(tableseats, servername);
}

void NetGGZ::join(char const * tablename)
{
	if (ggz_mode())
		ggzmod().connect();
	else
		core().join(tablename);
}

bool NetGGZ::updateForTables() {
	return core().updateForTables();
}
bool NetGGZ::updateForUsers() {
	return core().updateForUsers();
}

std::vector<Net_Game_Info> const & NetGGZ::tables() {
	return core().tablelist;
}

std::vector<Net_Player>    const & NetGGZ::users() {
	return core().userlist;
}

bool NetGGZ::is_connecting()
{
	return core().is_connecting();
}

bool NetGGZ::logged_in()
{
	return core().logged_in();
}

void NetGGZ::statechange()
{
	
}

bool NetGGZ::set_spectator(bool spec)
{
	if(spec)
		ggzmod().set_spectator();
	else
		ggzmod().set_player();
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
	return ggzmod().playername();
}


#endif //USE_GGZ
