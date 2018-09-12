/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#include <cassert>
#include <memory>
#include <vector>

#include <SDL.h>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "graphic/animation/animation.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_io.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/world.h"
#include "logic/widelands.h"
#include "website/lua/lua_tree.h"
#include "website/website_common.h"

namespace {

// Reads animation data from engine and then creates spritesheets and the corresponding lua code.
void write_spritesheet(Widelands::EditorGameBase& egbase,
                     const std::string& map_object_name,
                     const std::string& animation_name,
                     FileSystem* out_filesystem) {
	egbase.mutable_tribes()->postload();  // Make sure that all values have been set.
	const Widelands::Tribes& tribes = egbase.tribes();
	const Widelands::World& world = egbase.world();
	log("==========================================\n");

	// Get the map object
	const Widelands::MapObjectDescr* descr = nullptr;
	if (tribes.building_exists(tribes.building_index(map_object_name))) {
		descr = tribes.get_building_descr(tribes.building_index(map_object_name));
	} else if (tribes.ware_exists(tribes.ware_index(map_object_name))) {
		descr = tribes.get_ware_descr(tribes.ware_index(map_object_name));
	} else if (tribes.worker_exists(tribes.worker_index(map_object_name))) {
		descr = tribes.get_worker_descr(tribes.worker_index(map_object_name));
	} else if (tribes.immovable_exists(tribes.immovable_index(map_object_name))) {
		descr = tribes.get_immovable_descr(tribes.immovable_index(map_object_name));
	} else if (tribes.ship_exists(tribes.ship_index(map_object_name))) {
		descr = tribes.get_ship_descr(tribes.ship_index(map_object_name));
	} else if (world.get_immovable_index(map_object_name) != Widelands::INVALID_INDEX) {
		descr = world.get_immovable_descr(world.get_immovable_index(map_object_name));
	} else if (world.get_critter_descr(map_object_name)) {
		descr = world.get_critter_descr(map_object_name);
	} else {
		log("ABORTING. Unable to find map object for '%s'!\n", map_object_name.c_str());
		return;
	}
	assert(descr->name() == map_object_name);

	// Validate the animation
	if (!descr->is_animation_known(animation_name)) {
		log("ABORTING. Unknown animation '%s' for '%s'\n", animation_name.c_str(),
		    descr->name().c_str());
		return;
	}

	const Animation& animation =
	   g_gr->animations().get_animation(descr->get_animation(animation_name));

	const int nr_frames = animation.nr_frames();

	// Only create spritesheet if animation has more than 1 frame.
	if (nr_frames < 2) {
		log("ABORTING. Animation '%s' for '%s' has less than 2 images and doesn't need a spritesheet.\n", animation_name.c_str(),
			descr->name().c_str());
		return;
	}

	// Write a spritesheet of the given images into the given filename
	const auto write_spritesheet = [out_filesystem](std::vector<const Image*> imgs, const std::string& filename, int img_w, int img_h, int columns, int spritesheet_width, int spritesheet_height) {
		log("CREATING %d x %d spritesheet with %d columns, %" PRIuS " frames. Image size: %d x %d.\n", spritesheet_width, spritesheet_height, columns, imgs.size(), img_w, img_h);
		Texture* spritesheet = new Texture(spritesheet_width, spritesheet_height);
		spritesheet->fill_rect(Rectf(0.f, 0.f, spritesheet_width, spritesheet_height), RGBAColor(0, 0, 0, 0));
		int row = 0;
		int col = 0;
		for (size_t i = 0; i < imgs.size(); ++i, ++col) {
			if (col == columns) {
				col = 0;
				++row;
			}
			const Image* image = imgs[i];
			const int x = col * img_w;
			const int y = row * img_h;
			log("Frame %" PRIuS " at: %d, %d\n", i, x, y);
			spritesheet->blit(Rectf(x, y, img_w, img_h), *image, Rectf(0, 0, img_w, img_h), 1., BlendMode::Copy);
		}
		std::unique_ptr<::StreamWrite> sw(out_filesystem->open_stream_write(filename));
		save_to_png(spritesheet, sw.get(), ColorType::RGBA);
		log("Wrote spritesheet to %s/%s\n", out_filesystem->get_basename().c_str(), filename.c_str());
	};

	// Add global paramaters for this animation to Lua
	std::unique_ptr<LuaTree::Element> lua_object(new LuaTree::Element());
	LuaTree::Object* lua_animation = lua_object->add_object(animation_name);
	lua_animation->add_int("frames", nr_frames);

	LuaTree::Object* lua_table = lua_animation->add_object("hotspot");
	const Vector2i& hotspot = animation.hotspot();
	lua_table->add_int("", hotspot.x);
	lua_table->add_int("", hotspot.y);

	LuaTree::Object* lua_mipmap = lua_animation->add_object("mipmap");

	if (animation.nr_frames() > 1) {
		uint32_t frametime = animation.frametime();
		assert(frametime > 0);
		if (nr_frames > 1 && animation_name != "build" && frametime != kFrameLength) {
			lua_animation->add_int("fps", 1000 / frametime);
		}
	}

	log("WRITING '%s' animation for '%s'. It has %d pictures and %" PRIuS "scales.\n", animation_name.c_str(),
		descr->name().c_str(), nr_frames, animation.available_scales().size());

	// Create image files and add Lua code for each scale
	for (const float scale : animation.available_scales()) {
		std::vector<const Image*> images = animation.images(scale);

		// Get parameters at this scale
		const int w = animation.width() * scale;
		const int h = animation.height() * scale;
		const int columns = floor(sqrt(nr_frames));
		int rows = 1;
		while (rows * columns < nr_frames) {
			++rows;
		}
		const int spritesheet_width = columns * w;
		const int spritesheet_height = rows * h;

		if (spritesheet_width > kMinimumSizeForTextures || spritesheet_height > kMinimumSizeForTextures) {
			egbase.cleanup_objects();
			throw wexception("Unable to create spritesheet; either the width (%d) or the height (%d) are bigger than the minimum supported texture size (%d)", spritesheet_width, spritesheet_height, kMinimumSizeForTextures);
		}

		// Write spritesheet for animation and player colors
		const std::string filename_base = (boost::format("%s_%d") % animation_name % scale).str();
		write_spritesheet(images, filename_base + ".png", w, h, columns, spritesheet_width, spritesheet_height);
		std::vector<const Image*> pc_masks = animation.pc_masks(scale);
		if (!pc_masks.empty()) {
			write_spritesheet(pc_masks, filename_base + "_pc.png", w, h, columns, spritesheet_width, spritesheet_height);
		}

		// Lua parameters at this scale
		lua_table = lua_mipmap->add_object();
		lua_table->add_double("scale", scale);
		lua_table->add_raw("spritesheet", "path.dirname(__file__) .. \"" + filename_base + ".png\"");
		lua_table = lua_table->add_object("dimension");
		lua_table->add_int("", w);
		lua_table->add_int("", h);
	}
	log("LUA CODE:\n%s\n", lua_animation->as_string().c_str());
	log("Done!\n");
}

}  // namespace

/*
 ==========================================================
 MAIN
 ==========================================================
 */

int main(int argc, char** argv) {
	if (argc != 4) {
		log("Usage: %s <mapobject_name> <animation_name> <existing-output-path>\n", argv[0]);
		return 1;
	}

	const std::string map_object_name = argv[1];
	const std::string animation_name = argv[2];
	const std::string output_path = argv[3];

	try {
		initialize();
		std::unique_ptr<FileSystem> out_filesystem(&FileSystem::create(output_path));
		Widelands::EditorGameBase egbase(nullptr);
		write_spritesheet(egbase, map_object_name, animation_name, out_filesystem.get());
		egbase.cleanup_objects();
	} catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		cleanup();
		return 1;
	}
	cleanup();
	return 0;
}
