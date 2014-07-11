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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_MAP_IO_WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H
#define WL_MAP_IO_WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H

#include "map_io/widelands_map_data_packet.h"

class FileRead;
class FileWrite;

namespace Widelands {

class ConstructionSite;
class Partially_Finished_Building;
class DismantleSite;
class Game;
class MilitarySite;
class TrainingSite;
class ProductionSite;
class Warehouse;
class Building;

/*
 * This cares for the data of buildings
 */
class Map_Buildingdata_Data_Packet {
public:
	void Read(FileSystem&, Editor_Game_Base&, bool, Map_Map_Object_Loader&);
	void Write(FileSystem&, Editor_Game_Base&, Map_Map_Object_Saver&);

private:
	void read_constructionsite
		(ConstructionSite       &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_dismantlesite
		(DismantleSite       &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_partially_finished_building
		(Partially_Finished_Building   &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_constructionsite_v1
		(ConstructionSite       &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_warehouse
		(Warehouse              &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_militarysite
		(MilitarySite           &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_trainingsite
		(TrainingSite           &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_productionsite
		(ProductionSite         &, FileRead  &, Game &, Map_Map_Object_Loader &);
	void read_formerbuildings_v2
		(Building               &, FileRead  &, Game &, Map_Map_Object_Loader &);

	void write_constructionsite
		(const ConstructionSite &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_dismantlesite
		(const DismantleSite &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_partially_finished_building
		(const Partially_Finished_Building &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_warehouse
		(const Warehouse        &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_militarysite
		(const MilitarySite     &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_trainingsite
		(const TrainingSite     &, FileWrite &, Game &, Map_Map_Object_Saver  &);
	void write_productionsite
		(const ProductionSite   &, FileWrite &, Game &, Map_Map_Object_Saver  &);
};

}

#endif  // end of include guard: WL_MAP_IO_WIDELANDS_MAP_BUILDINGDATA_DATA_PACKET_H
