/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "widelands_map_object_packet.h"

#include "battle.h"
#include "editor_game_base.h"
#include "immovable.h"
#include "legacy.h"
#include "map.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


Map_Object_Packet::~Map_Object_Packet() {
	while (loaders.size()) {
		delete *loaders.begin();
		loaders.erase(loaders.begin());
	}
}


void Map_Object_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      *       egbase,
	 Map_Map_Object_Loader * const ol)
{
	try {
		FileRead fr;
		fr.Open(fs, "binary/mapobjects");

		const uint8_t packet_version = fr.Unsigned8();
		if (packet_version != CURRENT_PACKET_VERSION)
			throw wexception("unknown/unhandled version %u", packet_version);

		// Initial loading stage
		for (;;)
			switch (uint8_t const header = fr.Unsigned8()) {
			case 0:
				return;
			case Map_Object::header_Immovable:
				loaders.insert(Immovable::load(egbase, ol, fr));
				break;

			case Map_Object::header_Legacy_AttackController:
				loaders.insert(Legacy::loadAttackController(egbase, ol, fr));
				break;

			case Map_Object::header_Legacy_Battle:
				loaders.insert(Legacy::loadBattle(egbase, ol, fr));
				break;

			case Map_Object::header_Battle:
				loaders.insert(Battle::load(egbase, ol, fr));
				break;

			default:
				throw wexception("unknown object header %u", header);
			}
	} catch (const std::exception & e) {
		throw wexception("map objects: %s", e.what());
	} catch (...) {
		throw;
	}
}


struct loader_sorter {
	bool operator()
		(Map_Object::Loader * const a, Map_Object::Loader * const b) const
	{
		assert
			(a->get_object()->get_serial() != b->get_object()->get_serial());
		return a->get_object()->get_serial() < b->get_object()->get_serial();
	}
};
void Map_Object_Packet::LoadFinish() {
	typedef std::set<Map_Object::Loader *, loader_sorter> LoaderSetSorted;
	LoaderSetSorted loaders_sorted;
	// load_pointer stage
	for
		(LoaderSet::const_iterator cit = loaders.begin();
		 cit != loaders.end();
		 ++cit)
	{
		(*cit)->load_pointers();
		loaders_sorted.insert(*cit);
	}

	// load_finish stage
	LoaderSetSorted::const_iterator const loaders_sorted_end =
		loaders_sorted.end();
	for
		(LoaderSetSorted::const_iterator cit = loaders_sorted.begin();
		 cit != loaders_sorted_end;
		 ++cit)
	{
		(*cit)->load_finish();
		(*cit)->mol().mark_object_as_loaded((*cit)->get_object());
	}
}


void Map_Object_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     *       egbase,
	 Map_Map_Object_Saver * const os)
{
	FileWrite fw;

	fw.Unsigned8(CURRENT_PACKET_VERSION);

	const Object_Manager::objmap_t& objs = egbase->objects().get_objects();
	for
		(Object_Manager::objmap_t::const_iterator cit = objs.begin();
		 cit != objs.end();
		 ++cit)
	{
		Map_Object* obj = cit->second;

		// These checks can be eliminated and the object saver simplified
		// once all Map_Objects are saved using the new system
		if (os->is_object_known(obj))
			continue;

		if (!obj->has_new_save_support())
			throw wexception
				("MO(%u of type %s) without new style save support not saved "
				 "explicitly",
				 obj->get_serial(), obj->descr().descname().c_str());

		os->register_object(obj);
		obj->save(egbase, os, fw);
		os->mark_object_as_saved(obj);
	}

	fw.Unsigned8(0);

	fw.Write(fs, "binary/mapobjects");
}

};
