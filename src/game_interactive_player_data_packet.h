/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__GAME_INTERACTIVE_PLAYER_DATA_PACKET_H
#define __S__GAME_INTERACTIVE_PLAYER_DATA_PACKET_H

#include "game_data_packet.h"

/*
 * Informations about the interactive player. Mostly scrollpos,
 * player number and so on
 */
class Game_Interactive_Player_Data_Packet : public Game_Data_Packet {
   public:
      virtual ~Game_Interactive_Player_Data_Packet();

	virtual void Read
		(FileSystem &, Game*, Widelands_Map_Map_Object_Loader * const = 0)
		throw (_wexception);
	virtual void Write
		(FileSystem &, Game*, Widelands_Map_Map_Object_Saver * const = 0)
		throw (_wexception);

   private:
      void Read_Version1(FileRead*, Game*, Widelands_Map_Map_Object_Loader* = 0) throw(_wexception);
};

#endif
