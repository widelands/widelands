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
#include "graphic/image_io.h"
#include "graphic/render/minimaprenderer.h"
#include "graphic/surface.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/scripting.h"

using namespace Widelands;

int main(int /* argc */, char ** argv)
{
	try {
		SDL_Init(SDL_INIT_VIDEO);

		g_fs = new LayeredFileSystem();
		g_fs->AddFileSystem(&FileSystem::Create("/Users/sirver/Desktop/Programming/cpp/widelands/bzr_repo/"));

		g_gr = new Graphic();
		g_gr->initialize(640, 480, false, false);

		// NOCOM(#sirver): what
		Editor_Game_Base egbase(nullptr);
		Map* map = new Map();
		egbase.set_map(map);

		std::unique_ptr<Widelands::Map_Loader> ml(map->get_correct_loader(argv[1]));
		assert(ml != nullptr);

		ml->preload_map(true);
		ml->load_map_complete(egbase, true);

		std::unique_ptr<Surface> minimap(draw_minimap(egbase, nullptr, Point(0, 0), MiniMapLayers::Terrains));

		FileWrite fw;
		save_surface_to_png(minimap.get(), &fw);
		std::unique_ptr<FileSystem> fs(&FileSystem::Create("."));
		fw.Write(*fs, "minimap.png");
	}
	catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		return 1;
	}
	return 0;
}
