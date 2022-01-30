/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_OBJECT_PACKET_H
#define WL_MAP_IO_MAP_OBJECT_PACKET_H

#include "logic/map_objects/map_object.h"

class FileSystem;

namespace Widelands {

class EditorGameBase;
class MapObjectLoader;
struct MapObjectSaver;

/**
 * This data packet contains all \ref MapObject and derived instances.
 *
 * \note Right now, only those MapObjects not covered by other objects
 * are in this packet.
 */
struct MapObjectPacket {
	struct LoaderSorter {
		bool operator()(MapObject::Loader* const a, MapObject::Loader* const b) const {
			assert(a->get_object()->serial() != b->get_object()->serial());
			return a->get_object()->serial() < b->get_object()->serial();
		}
	};

	using LoaderSet = std::set<MapObject::Loader*, LoaderSorter>;
	LoaderSet loaders;

	~MapObjectPacket();

	void read(FileSystem&, EditorGameBase&, MapObjectLoader&);

	void load_finish();

	void write(FileSystem&, EditorGameBase&, MapObjectSaver&);
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_OBJECT_PACKET_H
