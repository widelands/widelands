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

#ifndef __S__WIDELANDS_MAP_DATA_PACKET_H
#define __S__WIDELANDS_MAP_DATA_PACKET_H

#include "map.h"
#include "wexception.h"
#include "widelands_map_map_object_saver.h"
#include "widelands_map_map_object_loader.h"

class FileRead;
class FileWrite;
class Editor_Game_Base;

/*
========================================

This class represents a data packet in a widelands
map file. it is an abstract base class

========================================
*/
class Widelands_Map_Data_Packet {
   public:
      virtual ~Widelands_Map_Data_Packet() { m_skip=false; }

      virtual void Read(FileRead*, Editor_Game_Base*, bool, Widelands_Map_Map_Object_Loader* = 0) throw(wexception) = 0;
      virtual void Write(FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver* = 0) throw(wexception) = 0;

   private:
      bool m_skip;
};

#endif
