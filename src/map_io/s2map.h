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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_MAP_IO_S2MAP_H
#define WL_MAP_IO_S2MAP_H

#include "map_io/map_loader.h"

class FileRead;

struct S2MapLoader : public Widelands::MapLoader {
	enum class WorldType {
		kGreenland = 0,
		kBlackland = 1,
		kWinterland = 2,
	};

	S2MapLoader(const std::string& filename, Widelands::Map& M);

	int32_t preload_map(bool, AddOns::AddOnsList*) override;
	int32_t load_map_complete(Widelands::EditorGameBase&, Widelands::MapLoader::LoadType) override;

private:
	const std::string filename_;
	WorldType worldtype_;
	std::set<Widelands::Coords> port_spaces_to_set_;

	void load_s2mf_header(FileRead&);
	void load_s2mf(Widelands::EditorGameBase&);
	void postload_set_port_spaces(const Widelands::EditorGameBase& egbase);
};

#endif  // end of include guard: WL_MAP_IO_S2MAP_H
