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

#ifndef __S__WIDELANDS_MAP_DATA_PACKET_FACTOR_H
#define __S__WIDELANDS_MAP_DATA_PACKET_FACTOR_H


#include "wexception.h"
#include "types.h"

class Game_Data_Packet;

/*
 * This factory creates the valid map_data packet
 * classes. This is usefull to add new Packets without
 * the need to touch any other code than class
 */
class Game_Data_Packet_Factory {
   public:
      Game_Data_Packet* create_correct_packet(ushort id) throw(wexception);
};


#endif
