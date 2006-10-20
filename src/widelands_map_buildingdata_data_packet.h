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

#ifndef __S__WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H
#define __S__WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H

#include "widelands_map_data_packet.h"

class Building;

/*
 * This cares for the data of buildings
 */
class Widelands_Map_Buildingdata_Data_Packet : public Widelands_Map_Data_Packet {
   public:
      virtual ~Widelands_Map_Buildingdata_Data_Packet();

	virtual void Read
		(FileSystem &,
		 Editor_Game_Base*,
		 const bool,
		 Widelands_Map_Map_Object_Loader * const = 0)
		throw (_wexception);
	virtual void Write
		(FileSystem &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const  = 0)
		throw (_wexception);

   private:
      virtual void read_constructionsite(Building*, FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
      virtual void read_warehouse(Building*, FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
      virtual void read_militarysite(Building*, FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
		virtual void read_trainingsite(Building*, FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
		virtual void read_productionsite(Building*, FileRead*, Editor_Game_Base*, Widelands_Map_Map_Object_Loader*);
      virtual void write_constructionsite(Building*, FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
      virtual void write_warehouse(Building*, FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
      virtual void write_militarysite(Building*, FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void write_trainingsite(Building*, FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
		virtual void write_productionsite(Building*, FileWrite*, Editor_Game_Base*, Widelands_Map_Map_Object_Saver*);
};


#endif
