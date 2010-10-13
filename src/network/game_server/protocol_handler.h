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

#ifndef __PROTOCOL_HANDLER_H__
#define __PROTOCOL_HANDLER_H__

#include <ggzgameserver.h>
#include "widelands_map.h"

class ProtocolHandler {
	public:
		ProtocolHandler();
		~ProtocolHandler();
		void process_data(Client * const client);
		void process_post_b16_data(int opcode, Client * const client);
		bool send_greeter(Client * const client);

		int m_result_gametime;
		std::string host_version, host_build;
};

#endif //__PROTOCOL_HANDLER_H__
