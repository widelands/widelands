/*
 * Copyright (C) 2018-2019 by the Widelands Development Team
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
#include <boost/algorithm/string.hpp>
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
char const* const animation_direction_names[6] = {"_ne", "_e", "_se", "_sw", "_w", "_nw"};

// Find trimmed rect according to transparent pixels.
// Lock texture before you call this function.
// TODO(GunChleoc): Revisit trimming when we have fresh Blender exports, to make sure that we won't
// jump a pixel when zooming
void find_trim_rect(Texture* texture, Recti* rect) {
	const int max_x = texture->width();
	const int max_y = texture->height();

	// Find left margin
	bool found = false;
	for (int x = 0; x < max_x && !found; ++x) {
		for (int y = 0; y < max_y && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				rect->x = std::min(rect->x, x - 1);
				found = true;
			}
		}
	}
	// Find right margin
	found = false;
	for (int x = max_x - 1; x >= 0 && !found; --x) {
		for (int y = 0; y < max_y && !found; ++y) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				rect->w = std::max(max_x, x + 1 - rect->x);
				found = true;
			}
		}
	}
	// Find top margin
	found = false;
	for (int y = 0; y < max_y && !found; ++y) {
		for (int x = 0; x < max_x && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				rect->y = std::min(rect->y, y - 1);
				found = true;
			}
		}
	}
	// Find bottom margin
	found = false;
	for (int y = max_y - 1; y >= 0 && !found; --y) {
		for (int x = 0; x < max_x && !found; ++x) {
			RGBAColor pixel = texture->get_pixel(x, y);
			if (pixel.a != 0) {
				rect->h = std::max(max_y, y + 1 - rect->y);
				found = true;
			}
		}
	}
}

// Finds margins so that we can crop the animation to save space
void find_margins(const std::vector<const Image*>& images, Recti* margins) {
	for (const Image* image : images) {
		std::unique_ptr<Texture> temp_texture(new Texture(image->width(), image->height()));
		Rectf image_dimensions(Vector2f::zero(), image->width(), image->height());
		temp_texture->blit(image_dimensions, *image, image_dimensions, 1., BlendMode::Copy);
		temp_texture->lock();
		find_trim_rect(temp_texture.get(), margins);
	}
}

// Write a spritesheet of the given images into the given filename
void write_spritesheet(std::vector<const Image*> imgs,
                       const std::string& filename,
                       const Recti& rect,
                       int columns,
                       int spritesheet_width,
                       int spritesheet_height,
                       FileSystem* out_filesystem) {
	log("CREATING %d x %d spritesheet with %d columns, %" PRIuS " frames. Image size: %d x %d.\n",
	    spritesheet_width, spritesheet_height, columns, imgs.size(), rect.w, rect.h);
	std::unique_ptr<Texture> spritesheet(new Texture(spritesheet_width, spritesheet_height));
	spritesheet->fill_rect(
	   Rectf(Vector2f::zero(), spritesheet_width, spritesheet_height), RGBAColor(0, 0, 0, 0));
	int row = 0;
	int col = 0;
	for (size_t i = 0; i < imgs.size(); ++i, ++col) {
		if (col == columns) {
			col = 0;
			++row;
		}
		const Image* image = imgs[i];
		const int x = col * rect.w;
		const int y = row * rect.h;
		log("Frame %" PRIuS " at: %d, %d, %d, %d\n", i, x, y, x + rect.w, y + rect.h);
		spritesheet->blit(Rectf(x, y, rect.w, rect.h), *image, Rectf(rect.x, rect.y, rect.w, rect.h),
		                  1., BlendMode::Copy);
	}
	std::unique_ptr<::StreamWrite> sw(out_filesystem->open_stream_write(filename));
	save_to_png(spritesheet.get(), sw.get(), ColorType::RGBA);
	log("Wrote spritesheet to %s/%s\n", out_filesystem->get_basename().c_str(), filename.c_str());
}

// Container for writing spritesheet files
struct SpritesheetData {
	explicit SpritesheetData(const std::string& fb,
	                         const std::vector<const Image*>& im,
	                         const std::vector<const Image*>& pc)
	   : filename_base(fb), images(im), pc_masks(pc) {
	}

	const std::string filename_base;
	const std::vector<const Image*> images;
	const std::vector<const Image*> pc_masks;
};

// Reads animation data from engine and then creates spritesheets and the corresponding lua code.
void write_animation_spritesheets(Widelands::EditorGameBase& egbase,
                                  const std::string& map_object_name,
                                  const std::string& animation_name,
                                  FileSystem* out_filesystem) {
	const Widelands::Tribes& tribes = egbase.tribes();
	const Widelands::World& world = egbase.world();
	log("==========================================\n");

	bool is_fontier_or_flag_animation = false;
	uint32_t frontier_or_flag_animation_id = 0;

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
		// Frontier and flag animations need special treatment
		std::vector<std::string> map_object_name_vector;
		boost::split(map_object_name_vector, map_object_name, boost::is_any_of("_"));
		if (map_object_name_vector.size() == 2) {
			const Widelands::TribeDescr* tribe =
			   tribes.get_tribe_descr(tribes.tribe_index(map_object_name_vector.front()));
			if (map_object_name_vector.back() == "frontier") {
				is_fontier_or_flag_animation = true;
				frontier_or_flag_animation_id = tribe->frontier_animation();
			} else if (map_object_name_vector.back() == "flag") {
				is_fontier_or_flag_animation = true;
				frontier_or_flag_animation_id = tribe->flag_animation();
			}
		}
	}
	if (!is_fontier_or_flag_animation && descr == nullptr) {
		log("ABORTING. Unable to find map object for '%s'!\n", map_object_name.c_str());
		return;
	}
	assert(is_fontier_or_flag_animation || (descr->name() == map_object_name));

	const bool is_directional =
	   !is_fontier_or_flag_animation &&
	   descr->is_animation_known(animation_name + animation_direction_names[0]);

	// Ensure that the animation exists
	if (!is_fontier_or_flag_animation) {
		if (!descr->is_animation_known(animation_name) &&
		    !descr->is_animation_known(animation_name + "_ne")) {
			log("ABORTING. Unknown animation '%s' for '%s'\n", animation_name.c_str(),
			    map_object_name.c_str());
			return;
		}
	}

	// Representative animation for collecting global paramaters for the animation set
	const Animation& representative_animation = g_gr->animations().get_animation(
	   is_fontier_or_flag_animation ?
	      frontier_or_flag_animation_id :
	      descr->get_animation(is_directional ? animation_name + "_ne" : animation_name, nullptr));

	const int nr_frames = representative_animation.nr_frames();

	// Only create spritesheet if animation has more than 1 frame.
	if (nr_frames < 2) {
		log("ABORTING. Animation '%s' for '%s' has less than 2 images and doesn't need a "
		    "spritesheet.\n",
		    animation_name.c_str(), map_object_name.c_str());
		return;
	}

	// Add global paramaters for this animation to Lua
	std::unique_ptr<LuaTree::Element> lua_object(new LuaTree::Element());
	LuaTree::Object* lua_animation = lua_object->add_object(animation_name);
	lua_animation->add_raw("directory", "path.dirname(__file__)");
	lua_animation->add_string("basename", animation_name);

	// We only write FPS if the animation is not a build animation and does not use the default FPS.
	if (animation_name != "build") {
		uint32_t frametime = representative_animation.frametime();
		assert(frametime > 0);
		if (frametime != kFrameLength) {
			assert(nr_frames > 1);
			lua_animation->add_int("fps", 1000 / frametime);
		}
	}

	log("WRITING '%s' animation for '%s'. It has %d pictures and %" PRIuS " scales.\n",
	    animation_name.c_str(), map_object_name.c_str(), nr_frames,
	    representative_animation.available_scales().size());

	const int columns = floor(sqrt(nr_frames));
	int rows = 1;
	while (rows * columns < nr_frames) {
		++rows;
	}

	lua_animation->add_int("frames", nr_frames);
	lua_animation->add_int("rows", rows);
	lua_animation->add_int("columns", columns);

	const int representative_frame = representative_animation.representative_frame();
	if (representative_frame > 0) {
		lua_animation->add_int("representative_frame", representative_frame);
	}

	if (is_directional) {
		lua_animation->add_bool("directional", true);
	}

	// Create image files for each scale and find & write the hotspot
	for (const float scale : representative_animation.available_scales()) {
		// Collect animation data to write
		std::vector<SpritesheetData> spritesheets_to_write;
		if (is_directional) {
			for (int dir = 1; dir <= 6; ++dir) {
				const std::string directional_animname =
				   animation_name + animation_direction_names[dir - 1];
				if (!descr->is_animation_known(directional_animname)) {
					throw wexception(
					   "Missing directional animation '%s\'", directional_animname.c_str());
				}
				const std::string filename_base = (boost::format("%s%s_%d") % animation_name %
				                                   animation_direction_names[dir - 1] % scale)
				                                     .str();
				const Animation& directional_animation = g_gr->animations().get_animation(
				   descr->get_animation(directional_animname, nullptr));
				spritesheets_to_write.push_back(SpritesheetData(filename_base,
				                                                directional_animation.images(scale),
				                                                directional_animation.pc_masks(scale)));
			}

		} else {
			spritesheets_to_write.push_back(SpritesheetData(
			   (boost::format("%s_%d") % animation_name % scale).str(),
			   representative_animation.images(scale), representative_animation.pc_masks(scale)));
		}

		// Find margins for trimming
		std::vector<const Image*> images = spritesheets_to_write.front().images;
		Recti margins(images.front()->width() / 2, images.front()->height() / 2,
		              images.front()->width() / 2, images.front()->height() / 2);
		for (const auto& animation_data : spritesheets_to_write) {
			find_margins(animation_data.images, &margins);
		}

		// Write the spritesheet(s)
		const int spritesheet_width = columns * margins.w;
		const int spritesheet_height = rows * margins.h;

		if (spritesheet_width > kMinimumSizeForTextures ||
		    spritesheet_height > kMinimumSizeForTextures) {
			egbase.cleanup_objects();
			throw wexception("Unable to create spritesheet; either the width (%d) or the height (%d) "
			                 "are bigger than the minimum supported texture size (%d)",
			                 spritesheet_width, spritesheet_height, kMinimumSizeForTextures);
		}

		// Write spritesheets for animation and player colors
		for (const auto& spritesheet_data : spritesheets_to_write) {
			write_spritesheet(spritesheet_data.images, spritesheet_data.filename_base + ".png",
			                  margins, columns, spritesheet_width, spritesheet_height, out_filesystem);
			if (!spritesheet_data.pc_masks.empty()) {
				write_spritesheet(spritesheet_data.pc_masks, spritesheet_data.filename_base + "_pc.png",
				                  margins, columns, spritesheet_width, spritesheet_height,
				                  out_filesystem);
			}
		}

		if (scale == 1.0f) {
			// Adjust and write hostspot
			LuaTree::Object* lua_table = lua_animation->add_object("hotspot");
			const Vector2i& hotspot = representative_animation.hotspot();
			lua_table->add_int("", hotspot.x - margins.x);
			lua_table->add_int("", hotspot.y - margins.y);
		}
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
		write_animation_spritesheets(egbase, map_object_name, animation_name, out_filesystem.get());
		egbase.cleanup_objects();
	} catch (std::exception& e) {
		log("Exception: %s.\n", e.what());
		cleanup();
		return 1;
	}
	cleanup();
	return 0;
}
