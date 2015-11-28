/*
 * Copyright (C) 2007-2008, 2010 by the Widelands Development Team
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
#include "economy/fleet.h"
#include "economy/portdock.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/battle.h"
#include "logic/critter.h"
#include "logic/editor_game_base.h"
#include "logic/immovable.h"
#include "logic/map.h"
#include "logic/ship.h"
#include "logic/worker.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint8_t kCurrentPacketVersion = 2;

MapObjectPacket::~MapObjectPacket() {
	while (loaders.size()) {
		delete *loaders.begin();
		loaders.erase(loaders.begin());
	}
}


void MapObjectPacket::read
	(FileSystem & fs, EditorGameBase & egbase, MapObjectLoader & mol,
	 const WorldLegacyLookupTable& world_lookup_table,
	 const TribesLegacyLookupTable& tribe_lookup_table)
{
	try {
		FileRead fr;
		fr.open(fs, "binary/mapobjects");

		const uint8_t packet_version = fr.unsigned_8();

		// Some maps contain ware/worker info, so we need compatibility here.
		if (1 <= packet_version && packet_version <= kCurrentPacketVersion) {

		// Initial loading stage
		for (;;)
			switch (uint8_t const header = fr.unsigned_8()) {
			case 0:
				return;
			case MapObject::HeaderImmovable:
				loaders.insert(Immovable::load(egbase, mol, fr, world_lookup_table, tribe_lookup_table));
				break;

			case MapObject::HeaderBattle:
				loaders.insert(Battle::load(egbase, mol, fr));
				break;

			case MapObject::HeaderCritter:
				loaders.insert(Critter::load(egbase, mol, fr, world_lookup_table));
				break;

			case MapObject::HeaderWorker:
				// We can't use the worker's savegame version, because some stuff is loaded before that
				// packet version, and we removed the tribe name.
				loaders.insert(Worker::load(egbase, mol, fr, tribe_lookup_table, packet_version));
				break;

			case MapObject::HeaderWareInstance:
				loaders.insert(WareInstance::load(egbase, mol, fr, tribe_lookup_table));
				break;

			case MapObject::HeaderShip:
				loaders.insert(Ship::load(egbase, mol, fr));
				break;

			case MapObject::HeaderPortDock:
				loaders.insert(PortDock::load(egbase, mol, fr));
				break;

			case MapObject::HeaderFleet:
				loaders.insert(Fleet::load(egbase, mol, fr));
				break;

			default:
				throw GameDataError("unknown object header %u", header);
			}
		} else {
			throw UnhandledVersionError("MapObjectPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception & e) {
		throw GameDataError("map objects: %s", e.what());
	}
}


void MapObjectPacket::load_finish() {
	// load_pointer stage
	for (MapObject::Loader* temp_loader : loaders) {
		try {
			temp_loader->load_pointers();
		} catch (const std::exception & e) {
			throw wexception("load_pointers for %s: %s",
				to_string(temp_loader->get_object()->descr().type()).c_str(),
				e.what());
		}
	}

	// load_finish stage
	for (MapObject::Loader* temp_loader : loaders) {
		try {
			temp_loader->load_finish();
		} catch (const std::exception & e) {
			throw wexception("load_finish for %s: %s",
			                 to_string(temp_loader->get_object()->descr().type()).c_str(),
			                 e.what());
		}
		temp_loader->mol().mark_object_as_loaded(*temp_loader->get_object());
	}
}


void MapObjectPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)
{
	FileWrite fw;

	fw.unsigned_8(kCurrentPacketVersion);

	std::vector<Serial> obj_serials = egbase.objects().all_object_serials_ordered();
	for
		(std::vector<Serial>::iterator cit = obj_serials.begin();
		 cit != obj_serials.end();
		 ++cit)
	{
		MapObject * pobj = egbase.objects().get_object(*cit);
		assert(pobj);
		MapObject & obj = *pobj;

		// These checks can be eliminated and the object saver simplified
		// once all MapObjects are saved using the new system
		if (mos.is_object_known(obj))
			continue;

		if (!obj.has_new_save_support())
			throw GameDataError
				("MO(%u of type %s) without new style save support not saved "
				 "explicitly",
				 obj.serial(), obj.descr().descname().c_str());

		mos.register_object(obj);
		obj.save(egbase, mos, fw);
		mos.mark_object_as_saved(obj);
	}

	fw.unsigned_8(0);

	fw.write(fs, "binary/mapobjects");
}

}
