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

#ifndef WIDELANDS_MAP_OBJECT_PACKET_H
#define WIDELANDS_MAP_OBJECT_PACKET_H

#include <set>

#include "logic/instances.h"

class FileSystem;

namespace Widelands {

class Editor_Game_Base;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;

/**
 * This data packet contains all \ref Map_Object and derived instances.
 *
 * \note Right now, only those Map_Objects not covered by other objects
 * are in this packet.
 */
struct Map_Object_Packet {
	struct loader_sorter {
		bool operator()
			(Map_Object::Loader * const a, Map_Object::Loader * const b) const
		{
			assert(a->get_object()->serial() != b->get_object()->serial());
			return a->get_object()->serial() < b->get_object()->serial();
		}
	};

	typedef std::set<Map_Object::Loader *, loader_sorter> LoaderSet;
	LoaderSet loaders;

	~Map_Object_Packet();

	void Read (FileSystem &, Editor_Game_Base &, Map_Map_Object_Loader &);

	void LoadFinish();

	void Write(FileSystem &, Editor_Game_Base &, Map_Map_Object_Saver  &);
};

}

#endif
