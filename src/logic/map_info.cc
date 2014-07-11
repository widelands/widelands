/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include <memory>

#include <SDL.h>

#include "base/log.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/scripting.h"

using namespace Widelands;

int main(int /* argc */, char ** /* argv */)
{
	try {
		SDL_Init(SDL_INIT_VIDEO);

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		g_fs = new LayeredFileSystem();
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		g_fs->AddFileSystem(&FileSystem::Create("/Users/sirver/Desktop/Programming/cpp/widelands/bzr_repo/"));
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		g_gr = new Graphic();
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		g_gr->initialize(640, 480, false, false);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

		// NOCOM(#sirver): what
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		Editor_Game_Base egbase(nullptr);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		egbase.mutable_world();
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

		Map* map = new Map();
		egbase.set_map(map);

		std::unique_ptr<Widelands::Map_Loader> ml(map->get_correct_loader("maps/Atoll.wmf"));
		assert(ml != nullptr);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		ml->preload_map(true);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		ml->load_map_complete(egbase, true);
		log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	}
	catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		return 1;
	}
	return 0;
}
