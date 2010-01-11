/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H
#define WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H

#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "widelands_map_data_packet.h"

namespace Widelands {

struct ConstructionSite;
struct Game;
struct MilitarySite;
struct TrainingSite;
struct ProductionSite;
struct Warehouse;

/*
 * This cares for the data of buildings
 */
struct Map_Buildingdata_Data_Packet : public Map_Data_Packet {
	void Read
		(FileSystem &, Editor_Game_Base &, bool, Map_Map_Object_Loader * = 0)
		throw (_wexception);
	void Write(FileSystem &, Editor_Game_Base &, Map_Map_Object_Saver * = 0)
		throw (_wexception);

private:
	virtual void read_constructionsite
		(ConstructionSite       &, FileRead  &, Game &, Map_Map_Object_Loader *);
	virtual void read_warehouse
		(Warehouse              &, FileRead  &, Game &, Map_Map_Object_Loader *);
	virtual void read_militarysite
		(MilitarySite           &, FileRead  &, Game &, Map_Map_Object_Loader *);
	virtual void read_trainingsite
		(TrainingSite           &, FileRead  &, Game &, Map_Map_Object_Loader *);
	virtual void read_productionsite
		(ProductionSite         &, FileRead  &, Game &, Map_Map_Object_Loader *);
	virtual void write_constructionsite
		(ConstructionSite const &, FileWrite &, Game &, Map_Map_Object_Saver  *);
	virtual void write_warehouse
		(Warehouse        const &, FileWrite &, Game &, Map_Map_Object_Saver  *);
	virtual void write_militarysite
		(MilitarySite     const &, FileWrite &, Game &, Map_Map_Object_Saver  *);
	virtual void write_trainingsite
		(TrainingSite     const &, FileWrite &, Game &, Map_Map_Object_Saver  *);
	virtual void write_productionsite
		(ProductionSite   const &, FileWrite &, Game &, Map_Map_Object_Saver  *);
};

}

#endif
