/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#include "editor_game_base.h"
#include "error.h"
#include "fileread.h"
#include "filewrite.h"
#include "immovable.h"
#include "map.h"
#include "wexception.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"


#define CURRENT_PACKET_VERSION 1


Widelands_Map_Object_Packet::~Widelands_Map_Object_Packet()
{
	while(loaders.size()) {
		delete *loaders.begin();
		loaders.erase(loaders.begin());
	}
}


void Widelands_Map_Object_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Loader * const ol)
{
	try {
		FileRead fr;
		fr.Open(fs, "binary/mapobjects");

		const Uint8 packet_version = fr.Unsigned8();
		if (packet_version != CURRENT_PACKET_VERSION)
			throw wexception("Unknown version %u", packet_version);

		// Initial loading stage
		for (;;) {
			Uint8 header = fr.Unsigned8();
			if (!header)
				break;

			switch(header) {
			case Map_Object::header_Immovable:
				loaders.insert(Immovable::load(egbase, ol, fr));
				break;

			default:
				throw wexception("Unknown object header %u", header);
			}
		}
	} catch(const std::exception& e) {
		throw wexception("Loading map objects: %s", e.what());
	} catch(...) {
		throw;
	}
}


void Widelands_Map_Object_Packet::LoadFinish()
{
	// load_pointer stage
	for(LoaderSet::const_iterator cit = loaders.begin(); cit != loaders.end(); ++cit)
		(*cit)->load_pointers();

	// load_finish stage
	for(LoaderSet::const_iterator cit = loaders.begin(); cit != loaders.end(); ++cit)
		(*cit)->load_finish();
}


void Widelands_Map_Object_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const os)
{
	FileWrite fw;

	fw.Unsigned8(CURRENT_PACKET_VERSION);

	const Object_Manager::objmap_t& objs = egbase->objects().get_objects();
	for(Object_Manager::objmap_t::const_iterator cit = objs.begin(); cit != objs.end(); ++cit) {
		Map_Object* obj = cit->second;

		// These checks can be eliminated and the object saver simplified
		// once all Map_Objects are saved using the new system
		if (os->is_object_known(obj))
			continue;

		assert(obj->has_new_save_support());

		os->register_object(obj);
		obj->save(egbase, os, fw);
		os->mark_object_as_saved(obj);
	}

	fw.Unsigned8(0);

	fw.Write(fs, "binary/mapobjects");
}
