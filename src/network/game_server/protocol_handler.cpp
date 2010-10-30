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
#include "widelands_client.h"
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

	WidelandsClient & wlclient = *g_wls->get_client_by_name(client->name);
	assert(&wlclient);

	if (wlclient.connection_failed)
		return;
	
	if (wlclient.desync) {
		if (not SUPPORT_B16_PROTOCOL((&wlclient))) {
			wlclient.connection_failed = true;
			return;
		}

		int zc = 0; // number of zeros received in a row
		int it = 100; // number of retries
		while (zc < 100 and it > 0)
		{
			char c;
			if ( read(client->fd, &c, 1) < 1) {
				wlclient.connection_failed = true;
				if (client->fd >=0)
					close(client->fd);
			}
			if (c == 0)
				zc++;
			else {
				if (c == SYNCCODE0)
					break;
				zc = 0;
				it--;
			}
		}
		if (it < 1) {
			wlclient.hard_error_count++;
			if (wlclient.hard_error_count > 100) {
				close(client->fd);
				wlclient.connection_failed = true;
			}
			return;
		}
		char c;
		if ( read(client->fd, &c, 1) < 1) {
			wlclient.connection_failed = true;
			if (client->fd >=0)
			close(client->fd);
			return;
		}
		if (c != SYNCCODE1) {
			
			wlclient.hard_error_count++;
			if (wlclient.hard_error_count > 100) {
				close(client->fd);
				wlclient.connection_failed = true;
			}
			return;
		}
		wlclient.soft_error_count = 0;
		wlclient.desync = false;
		wllog(DL_DEBUG, "Resynced client %s", wlclient.name().c_str());
	}

	wllog(DL_DEBUG, "data event from %s", client->name.c_str());

	// Read data
	if (ggz_read_int(client->fd, &opcode) < 0)
	{
		wllog
			(DL_ERROR, "dataEvent but read from client \"%s\" failed",
			 client->name.c_str());
		if (&wlclient)
			wlclient.connection_failed = true;
		if (client->fd >=0)
			close(client->fd);
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
		// This opcode is part of the post build16 protocol.
		wlclient.set_proto(1, 0);
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
		if (WLGGZ_OLD_OPCODE(opcode) or not SUPPORT_B16_PROTOCOL((&wlclient)))
		{
			wllog
				(DL_ERROR,
				 "Data error. Unhandled opcode (%i)! not a handled old opcode",
				 opcode);
		}
		else
			process_post_b16_data(opcode, client);
	}

	if (wlclient.hard_error_count > 100)
	{
		wllog
			(DL_ERROR, "too much fatal errors from %s(%i)",
			 client->name.c_str(), client->number);
		wlclient.connection_failed = true;
		// TODO: kick player from table?
		if (client->fd >=0)
			close(client->fd);
		return;
	}

	if (wlclient.desync or wlclient.soft_error_count > 1000) {
		// we had a problem reading from the client. That probably means the
		// connection is no longer in sync.
		wlclient.desync = true;
		wlclient.soft_error_count = 0;
		if (SUPPORT_B16_PROTOCOL((&wlclient))) {
			ggz_write_int(client->fd, op_desync);
		}
	}
}


void ProtocolHandler::process_post_b16_data(int opcode, Client * const client)
{
	WidelandsClient & wlclient = *g_wls->get_client_by_name(client->name);
	assert(&wlclient);
	assert(SUPPORT_B16_PROTOCOL((&wlclient)));
	std::list<WLGGZParameter> parlist;
	try {
		parlist = wlggz_read_parameter_list(client->fd);
	} catch (_parameterError e) {
		wlclient.hard_error_count++;
		wlclient.desync = true;
		wllog(DL_ERROR, "error whil reading parameters: %s", e.what());
	}

	switch(opcode) {
		case op_game_statistics:
			wllog(DL_DEBUG, "GAME: read stats!");
			if (wlclient.reported_game()) {
				wllog
					(DL_ERROR,
					 "got game_statistics from %s(%i) but already "
					 "have a report from this cilent",
					 client->name.c_str(), client->number);
				return;
			}
			try {
				g_wls->stat_handler().report_game_result(client, parlist);
			}
				catch (_parameterError e) {
					wllog(DL_ERROR, "Catched parameterError %s", e.what());
			}
			break;
		case op_game_information:
			wllog(DL_DUMP, "GAME: read game info!");
			// game is set to playing before sending game info. This is necessary
			// as game is open as long as state is waiting.
			//if (not g_wls->is_playing()) {
			//	wllog(DL_ERROR, "got game info but not in waiting state");
			//	return;
			//}
			try {
				g_wls->stat_handler().report_gameinfo(client, parlist);
			}
			catch (_parameterError e) {
				wllog(DL_ERROR, "Catched parameterError %s", e.what());
			}
			break;
		case op_set_debug:
			if (g_wls->is_host(&wlclient))
			{
				wllog
					(DL_DEBUG, "debug request from host: "
					 "enable sending of debug messages");
					 send_debug = client->fd;
			}
			break;
		case op_reply_protocol_ext:
			{
				if ((parlist).empty() or not (parlist).front().is_integer())
					break;
				int maj = parlist.front().get_integer();
				parlist.pop_front();
				if ((parlist).empty() or not (parlist).front().is_integer())
					break;
				int min = parlist.front().get_integer();
				wlclient.set_proto(maj, min);
			}
			break;
		default:
			//  Discard
			wllog(DL_ERROR, "Data error. Unhandled opcode (%i)!", opcode);
			wlclient.soft_error_count++;
			break;
	}
}


