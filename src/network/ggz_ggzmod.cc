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

#include "ggz_ggzmod.h"
#include "network_ggz.h"
#include "log.h"
#include "gamesettings.h"
#include "wexception.h"

#include <stdint.h>
#include <string>
#include <vector>

#include <limits>

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#endif
#include <warning.h>



static GGZMod    * mod       = 0;
ggz_ggzmod       * ggzmodobj = 0;

ggz_ggzmod & ggz_ggzmod::ref() {
	if (!ggzmodobj)
		ggzmodobj = new ggz_ggzmod();
	return *ggzmodobj;
}

ggz_ggzmod::ggz_ggzmod():
	m_connected(false),
	server_ip_addr(0)
{

}

void ggz_ggzmod::init()
{
#warning TODO
}


/**
 * Connect to GGZCORE. This does not connect to the metaserver but to our
 * ggzcore instance.
 */
bool ggz_ggzmod::connect()
{
	if (m_connected)
		return false;
	
	log("GGZMOD ## connect to GGZCORE\n");
	mod = ggzmod_new(GGZMOD_GAME);
	
	// Set handler for ggzmod events:
	ggzmod_set_handler(mod, GGZMOD_EVENT_SERVER, &ggz_ggzmod::ggzmod_server);
	ggzmod_set_handler(mod, GGZMOD_EVENT_ERROR, &ggz_ggzmod::ggzmod_server);
	// not handled / not used events of the GGZMOD Server:
	// * GGZMOD_EVENT_STATE
	// * GGZMOD_EVENT_PLAYER
	// * GGZMOD_EVENT_SEAT
	// * GGZMOD_EVENT_SPECTATOR_SEAT
	// * GGZMOD_EVENT_CHAT
	// * GGZMOD_EVENT_STATS
	// * GGZMOD_EVENT_INFO
	// * GGZMOD_EVENT_RANKINGS

	if (ggzmod_connect(mod)) {
		log("GGZMOD ## connection failed\n");
		return false;
	}

	m_connected = true;

	m_server_fd = ggzmod_get_fd(mod);
	log("GGZMOD ## connection fd %i\n", m_server_fd);

/*
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 999 * 1000;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);
	while (ggzmod_get_state(mod) != GGZMOD_STATE_PLAYING) {
		select(fd + 1, &fdset, 0, 0, &timeout);
		ggzmod_dispatch(mod);
		//log("GGZ ## timeout!\n");
		if (usedcore())
			datacore();
	}
*/
	return true;
}

/// handles the events of the ggzmod server
void ggz_ggzmod::ggzmod_server
	(GGZMod * const cbmod, GGZModEvent const e, void const * const cbdata)
{
	log("GGZMOD ## ggzmod_server\n");
	if (e == GGZMOD_EVENT_SERVER) {
		int32_t const fd = *static_cast<int32_t const *>(cbdata);
		ggzmodobj->m_data_fd = fd;
		log("GGZMOD ## got fd: %i\n", fd);
		ggzmod_set_state(cbmod, GGZMOD_STATE_PLAYING);
	} else if (e == GGZMOD_EVENT_ERROR) {
		const char * msg = static_cast<const char * >(cbdata);
		log("GGZMOD ## ERROR: %s\n", msg);
	} else
		log("GGZMOD ## HANDLE ERROR: %i\n", e);
}

int32_t ggz_ggzmod::datafd()
{
	if (m_data_fd < 0)
		wexception("GGZMOD ## Tried to get datafd but it is not valid\n");
	return m_data_fd;
}

void ggz_ggzmod::disconnect()
{
	if (m_data_fd > 0)
		close(m_data_fd);
	ggzmod_disconnect(mod);
	ggzmod_free(mod);
	m_connected = false;
}

void ggz_ggzmod::process_datacon() {
	int32_t op;
	char * ipstring;
	char * greeter;
	int32_t greeterversion;
	char ipaddress[17];
	int32_t fd = datafd();

	if (not m_connected)
		return;
	
	if (fd < 0 or fd > FD_SETSIZE)
		return;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	fd_set fdset;
	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	{
		int32_t const ret = select(fd + 1, &fdset, 0, 0, &timeout);
		if (ret <= 0)
			return;
		log("GGZMOD/WLDATA/process ## select() returns: %i for fd %i\n", ret, fd);
	}
	
	{
		int32_t const ret = ggz_read_int(fd, &op);
		log("GGZMOD/WLDATA/process ## received opcode: %i (%i)\n", op, ret);
		if (ret < 0) {
			ggz_ggzmod::ref().disconnect();
#warning TODO Handle read error from ggzmod datafd
//				use_ggz = false;
			return;
		}
	}
		
	switch (op) {
	case op_greeting:
		ggz_read_string_alloc(fd, &greeter);
		ggz_read_int(fd, &greeterversion);
		log("GGZMOD/WLDATA/process ## server is: '%s' '%i'\n", greeter, greeterversion);
		ggz_free(greeter);
		break;
	case op_request_ip:
		// This it not used (anymore?). Return 255.255.255.255 to the server
		log("GGZMOD/WLDATA/process ## ip request!\n");
		snprintf(ipaddress, sizeof(ipaddress), "%i.%i.%i.%i", 255, 255, 255, 255);
		ggz_write_int(fd, op_reply_ip);
		ggz_write_string(fd, ipaddress);
		break;
	case op_broadcast_ip:
		ggz_read_string_alloc(fd, &ipstring);
		log("GGZMOD/WLDATA/process ## got ip broadcast: '%s'\n", ipstring);
		server_ip_addr = ggz_strdup(ipstring);
		ggz_free(ipstring);
		break;
	case op_unreachable:
		NetGGZ::ref().deinit();
		throw warning
					(_("Connection problem"), "%s",
					 _
					  ("Your Server was not reachable from the Internet.\n"
					   "Please try to solve the problem - Reading the notes\n"
					   "at http://wl.widelands.org/wiki/InternetGaming can\n"
					   "be advantageous."));
	default: log("GGZMOD/WLDATA/process ## opcode unknown!\n");
	}
}

void ggz_ggzmod::process()
{
	if (not m_connected)
		return;
	ggzmod_dispatch(mod);
	process_datacon();
}

bool ggz_ggzmod::data_pending()
{
	fd_set read_fd_set;
	int result;
	struct timeval tv;

	if (not m_connected)
		return false;

	if (m_server_fd < 0 or m_server_fd > FD_SETSIZE)
		return false;

	FD_ZERO(&read_fd_set);
	if (m_server_fd > 0 and m_server_fd <= FD_SETSIZE)
		FD_SET(m_server_fd, &read_fd_set);
	if (m_data_fd > 0 and m_data_fd <= FD_SETSIZE)
		FD_SET(m_data_fd, &read_fd_set);
	tv.tv_sec = tv.tv_usec = 0;
	result = select(FD_SETSIZE, &read_fd_set, NULL, NULL, &tv);
	if (result > 0)
		return true;
	return false;
}

