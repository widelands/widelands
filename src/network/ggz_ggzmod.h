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

#ifndef GGZ_GGZMOD_H
#define GGZ_GGZMOD_H

#include <ggzmod.h>
#include <stdint.h>
#include <string>

/**
 * Does the ggzmod part of ggz. Ggzmod is the part of ggz client which
 * implements the actual game part of ggz.
 * @note ggzmod comunicates with the ggzcore object through a socket. This
 *   may cause deadlocks. For example 
 */
class ggz_ggzmod
{
public:
	static ggz_ggzmod & ref();
	void init();
	bool connect();
	bool connected() { return m_connected; }
	void disconnect();
	void process();

	int32_t datafd();

private:
	ggz_ggzmod();
	void process_datacon();
	static void ggzmod_server(GGZMod *, GGZModEvent, void const * cbdata);
	/// filedescriptor to communicate with the widelands_server module
	int32_t m_data_fd;
	int32_t m_server_fd;
	char * server_ip_addr;
	bool m_connected;
};

#endif // GGZ_GGZMOD_H
