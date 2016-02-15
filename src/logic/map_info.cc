/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "base/log.h"
#include "config.h"
#include "graphic/graphic.h"
#include "graphic/image_io.h"
#include "graphic/minimap_renderer.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "map_io/widelands_map_loader.h"

using namespace Widelands;

namespace  {

// Setup the static objects Widelands needs to operate and initializes systems.
void initialize() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		throw wexception("Unable to initialize SDL: %s", SDL_GetError());
	}

	g_fs = new LayeredFileSystem();
	g_fs->add_file_system(&FileSystem::create(INSTALL_DATADIR));

	g_gr = new Graphic();
	g_gr->initialize(Graphic::TraceGl::kNo, 1, 1, false);
}

}  // namespace

int main(int argc, char ** argv)
{
	if (!(2 <= argc && argc <= 3)) {
		log("Usage: %s <map file>\n", argv[0]);
		return 1;
	}

	const std::string map_path = argv[argc - 1];

	try {
		initialize();

		std::string map_dir = FileSystem::fs_dirname(map_path);
		if (map_dir.empty()) {
			map_dir = ".";
		}
		const std::string map_file = FileSystem::fs_filename(map_path.c_str());
		FileSystem* in_out_filesystem = &FileSystem::create(map_dir);
		g_fs->add_file_system(in_out_filesystem);

		Map* map = new Map();
		EditorGameBase egbase(nullptr);
		egbase.set_map(map);
		std::unique_ptr<Widelands::MapLoader> ml(map->get_correct_loader(map_file));

		if (!ml) {
			log("Cannot load map file.\n");
			return 1;
		}

		ml->preload_map(true);
		ml->load_map_complete(egbase, Widelands::MapLoader::LoadType::kScenario);

		std::unique_ptr<Texture> minimap(
		   draw_minimap(egbase, nullptr, Point(0, 0), MiniMapLayer::Terrain));

		// Write minimap
		{
			FileWrite fw;
			save_to_png(minimap.get(), &fw, ColorType::RGBA);
			fw.write(*in_out_filesystem, (map_file + ".png").c_str());
		}

		// Write JSON.
		{
			FileWrite fw;

			const auto write_string = [&fw] (const std::string& s) {
				fw.data(s.c_str(), s.size());
			};
			const auto write_key_value =
			   [&write_string](const std::string& key, const std::string& quoted_value) {
				write_string((boost::format("\"%s\": %s") % key % quoted_value).str());
			};
			const auto write_key_value_string =
			   [&write_key_value](const std::string& key, const std::string& value) {
				std::string quoted_value = value;
				boost::replace_all(quoted_value, "\"", "\\\"");
				write_key_value(key, "\"" + value + "\"");
			};
			const auto write_key_value_int = [&write_key_value] (const std::string& key, const int value) {
				write_key_value(key, boost::lexical_cast<std::string>(value));
			};
			write_string("{\n  ");
			write_key_value_string("name", map->get_name());
			write_string(",\n  ");
			write_key_value_string("author", map->get_author());
			write_string(",\n  ");
			write_key_value_string("description", map->get_description());
			write_string(",\n  ");
			write_key_value_string("hint", map->get_hint());
			write_string(",\n  ");
			write_key_value_int("width", map->get_width());
			write_string(",\n  ");
			write_key_value_int("height", map->get_height());
			write_string(",\n  ");
			write_key_value_int("nr_players", map->get_nrplayers());
			write_string(",\n  ");

			const std::string world_name =
					static_cast<Widelands::WidelandsMapLoader*>(ml.get())->old_world_name();
			write_key_value_string("world_name", world_name);
			write_string(",\n  ");
			write_key_value_string("minimap", map_path + ".png");
			write_string("\n");

			write_string("}\n");
			fw.write(*in_out_filesystem, (map_file + ".json").c_str());
		}
	}
	catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		return 1;
	}
	return 0;
}
