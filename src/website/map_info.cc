/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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
#include "website/json/json.h"
#include "website/website_common.h"

int main(int argc, char** argv) {
#ifdef _WIN32
	set_logging_dir();
#endif
	if (!(2 <= argc && argc <= 3)) {
		log_err("Usage: %s <map file>\n", argv[0]);
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

		Widelands::EditorGameBase egbase(nullptr);
		auto* map = egbase.mutable_map();
		std::unique_ptr<Widelands::MapLoader> ml(map->get_correct_loader(map_file));

		if (!ml) {
			log_err("Cannot load map file.\n");
			return 1;
		}

		ml->preload_map(true, nullptr);
		if (!map->required_addons().empty()) {
			log_err("This map depends on add-ons!\n");
			return 1;
		}
		ml->load_map_for_render(egbase, nullptr);

		std::unique_ptr<Texture> minimap(
		   draw_minimap(egbase, nullptr, Rectf(), MiniMapType::kStaticMap, MiniMapLayer::Terrain));

		// Write minimap
		{
			FileWrite fw;
			save_to_png(minimap.get(), &fw, ColorType::RGBA);
			fw.write(*in_out_filesystem, map_file + ".png");
		}

		// Write JSON.
		{
			std::unique_ptr<JSON::Object> json(new JSON::Object());
			json->add_string("name", map->get_name());
			json->add_string("author", map->get_author());
			json->add_string("description", map->get_description());
			json->add_string("hint", map->get_hint());
			json->add_int("width", map->get_width());
			json->add_int("height", map->get_height());
			json->add_int("nr_players", map->get_nrplayers());
			json->add_string(
			   "minimum_required_widelands_version", map->minimum_required_widelands_version());

			const std::string world_name =
			   dynamic_cast<Widelands::WidelandsMapLoader*>(ml.get())->old_world_name();
			json->add_string("world_name", world_name);
			json->add_string("minimap", map_path + ".png");
			json->write_to_file(*in_out_filesystem, map_file + ".json");
		}
		egbase.cleanup_objects();
	} catch (std::exception& e) {
		log_err("Exception: %s.\n", e.what());
		cleanup();
		return 1;
	}
	cleanup();
	return 0;
}
