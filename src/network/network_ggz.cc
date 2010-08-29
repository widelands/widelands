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
#include "game_server/protocol_helpers.h"
#include "container_iterate.h"
#include "build_info.h"


#include "ggz_ggzcore.h"
#include "ggz_ggzmod.h"

#include <cstring>

static NetGGZ    * ggzobj    = 0;

/// Constructor of NetGGZ.
/// It is private to avoid the creation of more than one ggz object.
NetGGZ::NetGGZ() :
	use_ggz       (false),
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

/// \returns true, if ggz is used
bool NetGGZ::used()
{
	return use_ggz;
}

void NetGGZ::deinit()
{
#warning to implement
}


/// \returns the ip of the server, if connected
char const * NetGGZ::ip()
{
	#warning implement this!
	return "";
}


/// initializes the local ggz core
/// @return
bool NetGGZ::initcore
	(char const * const metaserver, char const * const nick,
	 char const * const pwd, bool registered)
{
	username = nick;
	return use_ggz = ggz_ggzcore::ref().init(metaserver, nick, pwd, registered);
}

void NetGGZ::process()
{
	if (!used())
		return;

	while 
		(ggz_ggzcore::ref().data_pending()
		 or ggz_ggzmod::ref().data_pending())
	{
		log("GGZ ## process data ...\n");
		ggz_ggzcore::ref().process();
		ggz_ggzmod::ref().process();
	}

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
	if (!ggz_ggzcore::ref().is_in_room()) {
		log("GGZ ## !ggz_ggzcore::ref().is_in_room()\n");
		return 1;
	}

	//GGZGameType * const gametype = ggzcore_room_get_gametype(room);
	//  FIXME problem in ggz - for some reasons only 8 seats are currently
	//  FIXME available. I already posted this problem to the ggz
	//  FIXME mailinglist. -- nasenbaer
	//return gametype ? ggzcore_gametype_get_max_players(gametype) : 1;
	//  FIXME due to a bug in ggz 0.14.1 we may even only support <= 7 seats
	//  FIXME this should be changed once the next official ggz version is
	//  FIXME released and support for ggz 0.14.1 is removed from widelands src
	//return gametype ? 8 : 1;
	//  FIXME it is not a bug in 0.14.1 but a general problem in ggz. Using
	//  FIXME ggzcore and ggzmod from the same thread causes a deadlock between
	//  FIXME the ggz libs. -- timowi
	return 12;

}


/// Tells the metaserver that the game started
void NetGGZ::send_game_playing()
{
	if (used()) {
		int fd = ggz_ggzmod::ref().datafd();
		if (ggz_write_int(fd, op_state_playing) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


/// Tells the metaserver that the game is done
void NetGGZ::send_game_done()
{
	if (used()) {
		int fd = ggz_ggzmod::ref().datafd();
		if (ggz_write_int(fd, op_state_done) < 0)
			log("GGZMOD ERROR: Game state could not be send!\n");
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
}


void NetGGZ::send_game_info()
{
	log("GGZMOD NetGGZ::send_game_info()\n");
	if (used()) {
		int fd = ggz_ggzmod::ref().datafd();
		if (ggz_write_int(fd, op_game_information) < 0)
			log("GGZMOD ERROR: Game information could not be send!\n");

		WLGGZ_writer w = WLGGZ_writer(fd);

		w.type(gameinfo_mapname);
		w << mapname;

		w.type(gameinfo_mapsize);
		w << map_w << map_h;

		w.type(gameinfo_gametype);
		w << static_cast<int>(win_condition);

		w.type(gameinfo_version);
		w << build_id() << build_type();

		log("Iterate Players\n");
		std::vector<Net_Player_Info>::iterator pit = playerinfo.begin();
		while (pit != playerinfo.end())
		{
			w.type(gameinfo_playerid);
			w << pit->playernum;
			w.type(gameinfo_playername);
			w << pit->name;
			w.type(gameinfo_tribe);
			w << pit->tribe;
			w.type(gameinfo_playertype);
			w << static_cast<int>(pit->type);
			w.type(gameinfo_teamnumber);
			w << pit->team;
			pit++;
		}
		log("Player Iterate finished\n");
		w.flush();
		ggz_write_int(fd, 0);
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
	log("GGZMOD NetGGZ::send_game_info(): ende\n");
}

void NetGGZ::send_game_statistics
	(int32_t gametime,
	 const Widelands::Game::General_Stats_vector & resultvec)
{
	if (used()) {
		int fd = ggz_ggzmod::ref().datafd();
		log("NetGGZ::send_game_statistics: send statistics to metaserver now!\n");
		ggz_write_int(fd, op_game_statistics);

		WLGGZ_writer w = WLGGZ_writer(fd);
		
		w.type(gamestat_gametime);
		w << gametime;

		log
			("resultvec size: %d, playerinfo size: %d\n",
			 resultvec.size(), playerinfo.size());

		for (unsigned int i = 0; i < playerinfo.size(); i++)
		{
			w.type(gamestat_playernumber);
			w << static_cast<int>(i);

			w.type(gamestat_result);
			w << playerinfo.at(i).result;

			w.type(gamestat_points);
			w << playerinfo.at(i).points;

			w.type(gamestat_land);
			if (resultvec.at(i).land_size.size())
				w << static_cast<int>(resultvec.at(i).land_size.back());
			else
				w << 0;

			w.type(gamestat_buildings);
			if (resultvec.at(i).nr_buildings.size())
				w << static_cast<int>(resultvec.at(i).nr_buildings.back());
			else
				w << 0;

			w.type(gamestat_militarystrength);
			if (resultvec.at(i).miltary_strength.size())
				w << static_cast<int>(resultvec.at(i).miltary_strength.back());
			else
				w << 0;

			w.type(gamestat_casualties);
			if (resultvec.at(i).nr_casualties.size())
				w << static_cast<int>(resultvec.at(i).nr_casualties.back());
			else
				w << 0;

			w.type(gamestat_land);
			w << static_cast<int>(resultvec[i].nr_civil_blds_defeated.back());

			w.type(gamestat_civbuildingslost);
			if (resultvec.at(i).nr_civil_blds_lost.size())
				w << static_cast<int>(resultvec.at(i).nr_civil_blds_lost.back());
			else
				w << 0;

			w.type(gamestat_kills);
			if (resultvec.at(i).nr_kills.size())
				w << static_cast<int>(resultvec.at(i).nr_kills.back());
			else
				w << 0;

			w.type(gamestat_buildingsdefeat);
			w << static_cast<int>(resultvec[i].nr_msites_defeated.back());

			w.type(gamestat_milbuildingslost);
			if (resultvec.at(i).nr_msites_lost.size())
				w << static_cast<int>(resultvec.at(i).nr_msites_lost.back());
			else
				w << 0;

			w.type(gamestat_wares);
			if (resultvec.at(i).nr_wares.size())
				w << static_cast<int>(resultvec.at(i).nr_wares.back());
			else
				w << (0);

			w.type(gamestat_workers);
			if (resultvec.at(i).nr_workers.size())
				w << static_cast<int>(resultvec.at(i).nr_workers.back());
			else
				w << (0);

			w.type(gamestat_productivity);
			if (resultvec.at(i).productivity.size())
				w << static_cast<int>(resultvec.at(i).productivity.back());
			else
				w << (0);
		}

		/*
		- gamestat_playernumber
		gamestat_result
		gamestat_points
		- gamestat_land
		- gamestat_buildings
		- gamestat_milbuildingslost
		- gamestat_civbuildingslost
		gamestat_buildingsdefeat
		gamestat_milbuildingsconq
		gamestat_economystrength
		gamestat_militarystrength
		- gamestat_workers
		- gamestat_wares
		- gamestat_productivity
		- gamestat_casualties
		- gamestat_kills */
		w.flush();
		ggz_write_int(fd, 0);
	} else
		log("GGZMOD ERROR: GGZ not used!\n");
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

	if(player < 1 or player > playerinfo.size())
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
	ggz_ggzcore::ref().launch(tableseats, servername);
}

void NetGGZ::join(char const * tablename)
{
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

#endif
