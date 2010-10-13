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
#include "protocol_helper_read_list.h"

#include <stdexcept>
#include <cassert>
#include "widelands_player.h"
#include "statistics_handler.h"
#include "wlggz_exception.h"

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

	if (g_wls->get_host_ip()) {
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
			WLGGZ_writer wr(client->fd, op_reply_protocol_ext);
			wr << WIDELANDS_PROTOCOL_EXT_MAJOR << WIDELANDS_PROTOCOL_EXT_MINOR;
		}
		// And finally request the protocol version from the client
		wllog(DL_DEBUG, "got ext proto request from %s", client->name.c_str());
		ggz_write_int(client->fd, op_request_protocol_ext);
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
	std::list<WLGGZParameter> parlist = wlggz_read_parameter_list(client->fd);
	
	switch(opcode) {
		case op_game_statistics:
			wllog(DL_DEBUG, "GAME: read stats!");
			try {
				g_wls->stat_handler().report_game_result(client, parlist);
			}
				catch (parameterError e) {
					wllog(DL_ERROR, "Catched parameterError %s", e.what());
			}
			//read_game_statistics(client);
			break;
		case op_game_information:
			wllog(DL_DUMP, "GAME: read game info!");
			try {
				g_wls->stat_handler().report_gameinfo(client, parlist);
			}
			catch (parameterError e) {
				wllog(DL_ERROR, "Catched parameterError %s", e.what());
			}
			//read_game_information(client);
			break;
		case op_set_debug:
			if (player->is_host())
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
			break;
	}
}


