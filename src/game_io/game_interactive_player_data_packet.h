/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef GAME_INTERACTIVE_PLAYER_DATA_PACKET_H
#define GAME_INTERACTIVE_PLAYER_DATA_PACKET_H

#include "game_io/game_data_packet.h"

namespace Widelands {

/*
 * Information about the interactive player. Mostly scrollpos,
 * player number and so on
 */
struct Game_Interactive_Player_Data_Packet : public Game_Data_Packet {
	void Read (FileSystem &, Game &, Map_Map_Object_Loader * = nullptr) override;
	void Write(FileSystem &, Game &, Map_Map_Object_Saver  * = nullptr) override;
};

}

#endif
