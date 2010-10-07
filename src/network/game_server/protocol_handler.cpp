/*
* Copyright (C) 2010 The Widelands Development Team
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "protocol_handler.h"
#include "protocol.h"
#include "widelands_server.h"
#include "log.h"
#include "protocol_helpers.h"

#include <stdexcept>
#include <cassert>

ProtocolHandler::ProtocolHandler()
{
}

ProtocolHandler::~ProtocolHandler()
{
}

bool ProtocolHandler::send_greeter(Client * const client)
{
	// Send greeter
	int const channel = client->fd;
	ggz_write_int(channel, op_greeting);
	ggz_write_string(channel, "widelands server");
	ggz_write_int(channel, WIDELANDS_PROTOCOL);

	if (client->number != 0) {
		ggz_write_int(channel, op_broadcast_ip);
		ggz_write_string(channel, g_wls->get_host_ip());
	}
}

void ProtocolHandler::process_data(Client * const client)
{
	int opcode;
	int ret;

	WidelandsPlayer * player = g_wls->get_player_by_name(client->name);

	wllog(DL_DEBUG, "data event from %s", client->name.c_str());

	// Read data
	if (ggz_read_int(client->fd, &opcode) < 0)
	{
		wllog
			(DL_ERROR, "dataEvent but read from client \"%s\" failed",
			 client->name.c_str());
			 return;
	}

	switch (opcode) {
	case op_reply_ip:
	{
		// This is not used anymore. Just read a string to prevent desyncing
		char * ip;
		ggz_read_string_alloc(client->fd, &ip);
		wllog(DL_DEBUG, "deprecated reply_ip: %s (unused)", ip);
		ggz_free(ip);
		break;
	}
	case op_state_playing:
		wllog(DL_DEBUG, "GAME: playing!");
		g_wls->set_state_playing();
		break;
	case op_state_done:
		wllog(DL_DUMP, "GAME: done!");
		/* ToDo: Do not switch to state done directly. This exit the widelands
		 * server imediately. Switch to waiting first and wait for statistics.
		 * This should have a timeout
		 */
		g_wls->set_state_done();
		break;
	case op_request_protocol_ext:
		if (not player) {
			player = g_wls->get_player_by_name(client->name, true);
			player->set_ggz_player_number(client->number);
		}
		// This opcode is part of the post build16 protocol.
		player->set_build16_proto(true);
		{
			// Answer this request.
			WLGGZ_writer wr(client->fd);
			wr.type(op_reply_protocol_ext);
			wr << WIDELANDS_PROTOCOL_EXT_MAJOR << WIDELANDS_PROTOCOL_EXT_MINOR;
		}
		// And finally request the protocol version from the client
		wllog(DL_DEBUG, "got ext proto request from %s", client->name.c_str());
		ggz_write_int(client->fd, op_request_protocol_ext);
		ggz_write_int(client->fd, 0);
		break;
	default:
		if (WLGGZ_OLD_OPCODE(opcode) or not SUPPORT_B16_PROTOCOL(player))
		{
			wllog
				(DL_ERROR,
				 "Data error. Unhandled opcode (%i)! not a handled old opcode",
				 opcode);
		}
		else
			process_post_b16_data(opcode, client);
	}
}


void ProtocolHandler::process_post_b16_data(int opcode, Client * const client)
{
	WidelandsPlayer * player = g_wls->get_player_by_name(client->name);
	assert(player);
	assert(SUPPORT_B16_PROTOCOL(player));
	
	switch(opcode) {
		case op_game_statistics:
			wllog(DL_DEBUG, "GAME: read stats!");
			read_game_statistics(client);
			break;
		case op_game_information:
			wllog(DL_DUMP, "GAME: read game info!");
			read_game_information(client);
			break;
		case op_set_debug:
			wlggz_read_parameter_list(client->fd);
			if (client->number == 0)
			{
				wllog
					(DL_DEBUG, "debug request from host: "
					 "enable sending of debug messages");
					 send_debug = client->fd;
			}
			break;
		default:
			//  Discard
			wllog(DL_ERROR, "Data error. Unhandled opcode (%i)!", opcode);
			wlggz_read_parameter_list(client->fd);
			break;
	}
}

void ProtocolHandler::read_game_information(Client * const client)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;

	if (ggz_read_int(client->fd, &gameinfo) < 0)
	{
		wllog
			(DL_ERROR, "read_game_information: failed to read int from client %s",
			 client->name.c_str());
		return;
	}

	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(client->fd);

		switch(gameinfo)
		{
		case gameinfo_playerid:
			playernum = parlist.front().get_integer();
			playername.erase();
			player = NULL;
			break;
		case gameinfo_playername:
		{
			parlist.front().get_string();

			if
				(playername.empty() and not
				 parlist.front().get_string().empty() and player == NULL)
			{
				playername = parlist.front().get_string();
				player = g_wls->get_player_by_name(playername, true);
				player->set_ggz_player_number(client->number);
				player->set_wl_player_number(playernum);
			}
			else
				wllog
					(DL_ERROR,
					 "GAMEINFO: error playername \"%s\" %i: %s",
					 playername.c_str(), parlist.front().get_string().size(),
					 parlist.front().get_string().c_str());
		}
			break;
		case gameinfo_tribe:
			if( player->tribe().empty())
				player->set_tribe(parlist.front().get_string());
			if( player->tribe().compare(parlist.front().get_string()))
				wllog
					(DL_WARN,
					 "GAMEINFO: readinfo tribe: "
					 "clients disagree about tribe: %s, %s",
					 player->tribe().c_str(),
					 parlist.front().get_string().c_str());
				 break; 
		case gameinfo_gametype:
			m_map.set_gametype
				(static_cast<WLGGZGameType>(parlist.front().get_integer()));
			break;
		case gameinfo_mapname:
			if(m_map.name().empty())
			{
				std::string mapname = parlist.front().get_string();
				m_map.set_name(mapname);
			}
			break;
		case gameinfo_mapsize:
		{
			int width, height;
			width = parlist.front().get_integer();
			parlist.pop_front();
			height = parlist.front().get_integer();
			m_map.set_size(width, height);
		}
			break;
		case gameinfo_playertype:
			if(player)
				player->set_type
					(static_cast<WLGGZPlayerType>(parlist.front().get_integer()));
			break;
		case gameinfo_version:
		{
			std::string version = parlist.front().get_string();
			parlist.pop_front();
			std::string build = parlist.front().get_string();
			if (client->number == 0)
			{
				host_version = version;
				parlist.pop_front();
				host_build = build;
			}
			player = g_wls->get_player_by_name(client->name);
			if (not player) {
				player = g_wls->get_player_by_name(client->name, true);
				player->set_ggz_player_number(client->number);
			}
			player->set_version(version, build);
			wllog
				(DL_DUMP, "GAMEINFO: Player \"%s\": %s(%s)",
				 client->name.c_str(), version.c_str(), build.c_str());
			break;
		}
		case gameinfo_teamnumber:
			if(player)
				player->set_team(parlist.front().get_integer());
			break;
		default:
			wllog(DL_ERROR, "GAMEINFO: error unknown WLGGZGameInfo!");
		}
		if (ggz_read_int(client->fd, &gameinfo) < 0)
		{
			wllog
				(DL_ERROR, "read_game_information: failed to read int from client %s",
				 client->name.c_str());
			return;
		}
	}


	/*
	int num = playercount(Seat::player);
	wllog(DL_INFO, "GAMEINFO: number of players %i", num);

	for (int i=0; i<num; i++)
	{
		wllog(DL_INFO, "GAMEINFO: seat i: %i");
		if (seat(i))
		{
			if (seat(i)->client)
			{
				wllog
					(DL_INFO, ", s: %i, c: %i, Name: %s", seat(i)->number,
					 seat(i)->client->number, seat(i)->client->name.c_str());
				if(g_wls->m_players.find(seat(i)->client->name) != g_wls->m_players.end())
				{
					 if(g_wls->m_players[seat(i)->client->name]->ggz_player_number()==-1)
						g_wls->m_players[seat(i)->client->name]->
							set_ggz_player_number(seat(i)->client->number);
				}
			} else {
				wllog(DL_INFO, "client* does not exist");
			}
		} else {
			wllog(DL_INFO, "does not exist ...");
		}
	}
	*/
}

void ProtocolHandler::read_game_statistics(Client * const client)
{
	int gameinfo, playernum=-1;
	std::string playername="";
	WidelandsPlayer * player = NULL;
	
	if (ggz_read_int(client->fd, &gameinfo) < 0)
	{
		wllog
			(DL_ERROR, "read_game_statistics: failed to read int from client");
			 return;
	}

	while(gameinfo)
	{
		std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(client->fd);

		switch(gameinfo)
		{
		case gamestat_playernumber:
		{
			if ( not parlist.size() or not parlist.front().is_integer())
			{
				wllog
					(DL_ERROR, "GAMESTATISTICS: ERROR: "
					 "got playernumber but parameter error");
				break;
			}
			playernum = parlist.front().get_integer();
			player = g_wls->get_player_by_wlid(playernum);
			if(not player)
				wllog
					(DL_ERROR, "GAMESTATISTICS: ERROR: "
					 "got playernumber but could no find the player %i",
					 parlist.front().get_integer());
			break;
		}
		case gamestat_result:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.result=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: got result but catched a error");
				break;
		case gamestat_points:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.points=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: got points but catched a error");
				break;
		case gamestat_land:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.land=parlist.front().get_integer();
			else
				wllog(DL_ERROR, "GAMESTATISTICS: got land but catched a error");
			break;
		case gamestat_buildings:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.buildings=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got buildings but catched a error");
			break;
		case gamestat_milbuildingslost:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.milbuildingslost=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got milbuildingslost but catched a error");
			break;
		case gamestat_civbuildingslost:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.civbuildingslost=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got civbuildingslost but catched a error");
			break;
		case gamestat_buildingsdefeat:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.buildingsdefeat=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got buildingsdefeat but catched a error");
			break;
		case gamestat_milbuildingsconq:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.milbuildingsconq=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got milbuildingsconq but catched a error");
			break;
		case gamestat_economystrength:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.economystrength=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got economystrength but catched a error");
			break;
		case gamestat_militarystrength:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.militarystrength=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: "
					 "got militarystrength but catched a error");
			break;
		case gamestat_workers:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.workers=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: got workers but catched a error");
			break;
		case gamestat_wares:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.wares=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: got wares but catched a error");
			break;
		case gamestat_productivity:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.productivity=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR,
					 "GAMESTATISTICS: got productivity but catched a error");
			break;
		case gamestat_casualties:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.casualties=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR,
					 "GAMESTATISTICS: got casulties but catched a error");
			break;
		case gamestat_kills:
			if(player and parlist.size() and parlist.front().is_integer())
				player->last_stats.kills=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR, "GAMESTATISTICS: got kills but catched a error");
			break;
		case gamestat_gametime:
			if (parlist.size() and parlist.front().is_integer())
				m_result_gametime=parlist.front().get_integer();
			else
				wllog
					(DL_ERROR,
					 "GAMESTATISTICS: got gametime but catched a error");
			break;
		default:
			wllog(DL_WARN,  "GAMESTATISTICS: Warning unknown WLGGZGameStats!");
		}
		if (ggz_read_int(client->fd, &gameinfo) < 0)
		{
			wllog
				(DL_ERROR, "read_game_statistics: failed to read int from client");
			return;
		}
	}
}

