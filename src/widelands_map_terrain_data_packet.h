/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__WIDELANDS_MAP_TERRAIN_DATA_PACKET_H
#define __S__WIDELANDS_MAP_TERRAIN_DATA_PACKET_H

#include "widelands_map_data_packet.h"

/*
 * The heights data packet contains the heights of all fields
 */
class Widelands_Map_Terrain_Data_Packet : public Widelands_Map_Data_Packet {
   public:
      virtual ~Widelands_Map_Terrain_Data_Packet();

      virtual void Read(FileRead*, Editor_Game_Base*) throw(wexception);
      virtual void Write(FileWrite*, Editor_Game_Base*) throw(wexception);
};


#endif

