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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/map_object_packet.h"

#include "base/wexception.h"
#include "economy/ferry_fleet.h"
#include "economy/portdock.h"
#include "economy/ship_fleet.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/critter.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "map_io/map_packet_versions.h"

namespace Widelands {

MapObjectPacket::~MapObjectPacket() {
	while (!loaders.empty()) {
		delete *loaders.begin();
		loaders.erase(loaders.begin());
	}
}

void MapObjectPacket::read(FileSystem& fs, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		FileRead fr;
		fr.open(fs, "binary/mapobjects");

		// Packet version is checked by individual loaders as necessary
		const uint8_t packet_version = fr.unsigned_8();

		// Initial loading stage
		for (;;) {
			switch (uint8_t const header = fr.unsigned_8()) {
			case 0:
				return;
			case MapObject::HeaderImmovable:
				loaders.insert(Immovable::load(egbase, mol, fr));
				break;

			case MapObject::HeaderBattle:
				loaders.insert(Battle::load(egbase, mol, fr));
				break;

			case MapObject::HeaderCritter:
				loaders.insert(Critter::load(egbase, mol, fr));
				break;

			case MapObject::HeaderWorker:
				// We can't use the worker's savegame version, because some stuff is loaded before
				// that
				// packet version, and we removed the tribe name.
				loaders.insert(Worker::load(egbase, mol, fr, packet_version));
				break;

			case MapObject::HeaderWareInstance:
				loaders.insert(WareInstance::load(egbase, mol, fr));
				break;

			case MapObject::HeaderShip:
				loaders.insert(Ship::load(egbase, mol, fr));
				break;

			case MapObject::HeaderPortDock:
				loaders.insert(PortDock::load(egbase, mol, fr));
				break;

			case MapObject::HeaderShipFleet:
				loaders.insert(ShipFleet::load(egbase, mol, fr));
				break;

			case MapObject::HeaderFerryFleet:
				loaders.insert(FerryFleet::load(egbase, mol, fr));
				break;

			default:
				throw GameDataError("unknown object header %u", header);
			}
		}
	} catch (const std::exception& e) {
		throw GameDataError("map objects: %s", e.what());
	}
}

void MapObjectPacket::load_finish() {
	// load_pointer stage
	for (MapObject::Loader* temp_loader : loaders) {
		try {
			temp_loader->load_pointers();
		} catch (const std::exception& e) {
			throw wexception("load_pointers for %s: %s",
			                 to_string(temp_loader->get_object()->descr().type()).c_str(), e.what());
		}
	}

	// load_finish stage
	for (MapObject::Loader* temp_loader : loaders) {
		try {
			temp_loader->load_finish();
		} catch (const std::exception& e) {
			throw wexception("load_finish for %s: %s",
			                 to_string(temp_loader->get_object()->descr().type()).c_str(), e.what());
		}
		temp_loader->mol().mark_object_as_loaded(*temp_loader->get_object());
	}
}

void MapObjectPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	fw.unsigned_8(kCurrentPacketVersionMapObject);

	for (const Serial ser : egbase.objects().all_object_serials_ordered()) {
		MapObject* pobj = egbase.objects().get_object(ser);
		assert(pobj);
		MapObject& obj = *pobj;

		// These checks can be eliminated and the object saver simplified
		// once all MapObjects are saved using the new system
		if (mos.is_object_known(obj)) {
			continue;
		}

		if (!obj.has_new_save_support()) {
			throw GameDataError("MO(%u of type %s) without new style save support not saved "
			                    "explicitly",
			                    obj.serial(), obj.descr().name().c_str());
		}

		mos.register_object(obj);
		obj.save(egbase, mos, fw);
		mos.mark_object_as_saved(obj);
	}

	fw.unsigned_8(0);

	fw.write(fs, "binary/mapobjects");
}
}  // namespace Widelands
