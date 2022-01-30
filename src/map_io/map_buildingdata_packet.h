/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_MAP_IO_MAP_BUILDINGDATA_PACKET_H
#define WL_MAP_IO_MAP_BUILDINGDATA_PACKET_H

#include "map_io/map_data_packet.h"

class FileRead;
class FileWrite;

namespace Widelands {

class ConstructionSite;
class PartiallyFinishedBuilding;
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
class MapBuildingdataPacket {
public:
	void read(FileSystem&, EditorGameBase&, bool, MapObjectLoader&);
	void write(FileSystem&, EditorGameBase&, MapObjectSaver&);

private:
	void read_constructionsite(ConstructionSite&, FileRead&, Game&, MapObjectLoader&);
	void read_dismantlesite(DismantleSite&, FileRead&, Game&, MapObjectLoader&);
	void
	read_partially_finished_building(PartiallyFinishedBuilding&, FileRead&, Game&, MapObjectLoader&);
	void read_warehouse(Warehouse&, FileRead&, Game&, MapObjectLoader&);
	void read_militarysite(MilitarySite&, FileRead&, Game&, MapObjectLoader&);
	void read_trainingsite(TrainingSite&, FileRead&, Game&, MapObjectLoader&);
	void read_productionsite(ProductionSite&, FileRead&, Game&, MapObjectLoader&);

	void write_constructionsite(const ConstructionSite&, FileWrite&, Game&, MapObjectSaver&);
	void write_dismantlesite(const DismantleSite&, FileWrite&, Game&, MapObjectSaver&);
	void write_partially_finished_building(const PartiallyFinishedBuilding&,
	                                       FileWrite&,
	                                       Game&,
	                                       MapObjectSaver&);
	void write_warehouse(const Warehouse&, FileWrite&, Game&, MapObjectSaver&);
	void write_militarysite(const MilitarySite&, FileWrite&, Game&, MapObjectSaver&);
	void write_trainingsite(const TrainingSite&, FileWrite&, Game&, MapObjectSaver&);
	void write_productionsite(const ProductionSite&, FileWrite&, Game&, MapObjectSaver&);
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_BUILDINGDATA_PACKET_H
