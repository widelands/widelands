/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

struct ConstructionSite;
struct FileRead;
struct FileWrite;
struct MilitarySite;
struct TrainingSite;
struct ProductionSite;
struct Warehouse;

/*
 * This cares for the data of buildings
 */
struct Widelands_Map_Buildingdata_Data_Packet : public Widelands_Map_Data_Packet {
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
	virtual void read_constructionsite
		(ConstructionSite &,
		 FileRead &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Loader * const);
	virtual void read_warehouse
		(Warehouse &,
		 FileRead &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Loader * const);
	virtual void read_militarysite
		(MilitarySite &,
		 FileRead &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Loader * const);
	virtual void read_trainingsite
		(TrainingSite &,
		 FileRead &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Loader * const);
	virtual void read_productionsite
		(ProductionSite &,
		 FileRead &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Loader * const);
	virtual void write_constructionsite
		(const ConstructionSite &,
		 FileWrite &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const);
	virtual void write_warehouse
		(const Warehouse &,
		 FileWrite &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const);
	virtual void write_militarysite
		(const MilitarySite &,
		 FileWrite &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const);
	virtual void write_trainingsite
		(const TrainingSite &,
		 FileWrite &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const);
	virtual void write_productionsite
		(const ProductionSite &,
		 FileWrite &,
		 Editor_Game_Base*,
		 Widelands_Map_Map_Object_Saver * const);
};


#endif
