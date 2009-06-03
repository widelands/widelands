// Widelands server for GGZ
// Copyright (C) 2004 Josef Spillner <josef@ggzgamingzone.org>
// Copyright (C) 2009 The Widelands Development Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef WIDELANDS_PROTOCOL_H
#define WIDELANDS_PROTOCOL_H

#define WIDELANDS_PROTOCOL 1

enum NetworkOpcodes
{
	op_greeting = 1,
	op_request_ip = 2,
	op_reply_ip = 3,
	op_broadcast_ip = 4,
	op_state_playing = 5,
	op_state_done = 6
};

#endif

