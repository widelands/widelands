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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/animation/nonpacked_animation.h"

#include <cassert>
#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "graphic/playercolor.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

/*
==============================================================================

NonPackedAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::NonPackedMipMapEntry::NonPackedMipMapEntry(std::vector<std::string> files)
   : image_files(std::move(files)) {
	if (image_files.empty()) {
		throw Widelands::GameDataError(
		   "Animation without image files. For a scale of 1.0, the template should look similar to "
		   "this:"
		   " 'directory/idle_1_??.png' for 'directory/idle_1_00.png' etc.");
	}

	for (std::string image_file : image_files) {
		replace_last(image_file, ".png", "_pc.png");
		if (g_fs->file_exists(image_file)) {
			has_playercolor_masks = true;
			playercolor_mask_image_files.push_back(image_file);
		} else if (has_playercolor_masks) {
			throw Widelands::GameDataError(
			   "Animation is missing player color file: %s", image_file.c_str());
		}
	}

	assert(!image_files.empty());
}

void NonPackedAnimation::NonPackedMipMapEntry::ensure_graphics_are_loaded() const {
	if (frames.empty()) {
		const_cast<NonPackedMipMapEntry*>(this)->load_graphics();
	}
}

void NonPackedAnimation::NonPackedMipMapEntry::load_graphics() {
	if (image_files.empty()) {
		throw Widelands::GameDataError("animation without image files.");
	}
	if (!playercolor_mask_image_files.empty() &&
	    playercolor_mask_image_files.size() != image_files.size()) {
		throw Widelands::GameDataError(
		   "animation has %" PRIuS " frames but playercolor mask has %" PRIuS
		   " frames. First image is %s",
		   image_files.size(), playercolor_mask_image_files.size(), image_files.front().c_str());
	}

	for (const std::string& filename : image_files) {
		const Image* image = g_image_cache->get(filename);
		if (!frames.empty() && (frames.front()->width() != image->width() ||
		                        frames.front()->height() != image->height())) {
			throw Widelands::GameDataError(
			   "wrong size: (%u, %u) for file %s, should be (%u, %u) like the first frame",
			   image->width(), image->height(), filename.c_str(), frames.front()->width(),
			   frames.front()->height());
		}
		frames.push_back(image);
	}

	for (const std::string& filename : playercolor_mask_image_files) {
		// TODO(unknown): Do not load playercolor mask as opengl texture or use it as
		//     opengl texture.
		const Image* pc_image = g_image_cache->get(filename);
		if (frames.front()->width() != pc_image->width() ||
		    frames.front()->height() != pc_image->height()) {
			throw Widelands::GameDataError("playercolor mask %s has wrong size: (%u, %u), should "
			                               "be (%u, %u) like the animation frame",
			                               filename.c_str(), pc_image->width(), pc_image->height(),
			                               frames.front()->width(), frames.front()->height());
		}
		playercolor_mask_frames.push_back(pc_image);
	}
}

void NonPackedAnimation::NonPackedMipMapEntry::blit(uint32_t idx,
                                                    const Rectf& source_rect,
                                                    const Rectf& destination_rect,
                                                    const RGBColor* clr,
                                                    Surface* target,
                                                    float opacity) const {
	assert(!frames.empty());
	assert(target);
	assert(idx < frames.size());

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *frames.at(idx), source_rect, opacity, BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   destination_rect, *frames.at(idx), *playercolor_mask_frames.at(idx), source_rect, *clr);
	}
}

std::vector<std::unique_ptr<const Texture>>
NonPackedAnimation::NonPackedMipMapEntry::frame_textures(bool return_playercolor_masks) const {
	ensure_graphics_are_loaded();

	std::vector<std::unique_ptr<const Texture>> result;
	const Rectf rect(Vector2f::zero(), width(), height());
	for (const std::string& filename :
	     return_playercolor_masks ? playercolor_mask_image_files : image_files) {
		std::unique_ptr<Texture> texture(new Texture(width(), height()));
		texture->fill_rect(rect, RGBAColor(0, 0, 0, 0));
		texture->blit(rect, *g_image_cache->get(filename), rect, 1., BlendMode::Copy);
		result.push_back(std::move(texture));
	}
	return result;
}

int NonPackedAnimation::NonPackedMipMapEntry::width() const {
	return frames.at(0)->width();
}
int NonPackedAnimation::NonPackedMipMapEntry::height() const {
	return frames.at(0)->height();
}

/*
==============================================================================

NonPackedAnimation IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::NonPackedAnimation(const LuaTable& table,
                                       const std::string& basename,
                                       const std::string& animation_directory)
   : Animation(table) {
	try {
		// Get image files
		if (table.has_key("pictures")) {
			// TODO(GunChleoc): Old code - remove this option once conversion has been completed
			assert(!table.get_table("pictures")->array_entries<std::string>().empty());
			verb_log_dbg("Found deprecated 'pictures' parameter in animation with file\n   %s\n",
			             table.get_table("pictures")->array_entries<std::string>().front().c_str());
			mipmaps_.insert(
			   std::make_pair(1.0f, std::unique_ptr<NonPackedMipMapEntry>(new NonPackedMipMapEntry(
			                           table.get_table("pictures")->array_entries<std::string>()))));
		} else {
			// TODO(GunChleoc): When all animations have been converted, require that
			// animation_directory is not empty.
			add_available_scales(basename, animation_directory.empty() ?
                                           table.get_string("directory") :
                                           animation_directory);
		}

		// Frames
		const NonPackedMipMapEntry& first =
		   dynamic_cast<const NonPackedMipMapEntry&>(*mipmaps_.begin()->second);
		nr_frames_ = first.image_files.size();
		if (table.has_key("fps") && nr_frames_ == 1) {
			throw Widelands::GameDataError(
			   "Animation with one picture %s must not have 'fps'", first.image_files.front().c_str());
		}

		if (representative_frame() < 0 || representative_frame() > nr_frames_ - 1) {
			throw wexception("Animation has %d as its representative frame, but the frame indices "
			                 "available are 0 - %d",
			                 representative_frame(), nr_frames_ - 1);
		}

		// Perform some checks to make sure that the data is complete and consistent
		const bool should_have_playercolor = mipmaps_.begin()->second->has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
			const NonPackedMipMapEntry& nonpacked_mipmap =
			   dynamic_cast<const NonPackedMipMapEntry&>(*mipmap.second);
			if (nonpacked_mipmap.image_files.size() != nr_frames_) {
				throw Widelands::GameDataError(
				   "Mismatched number of images for different scales in animation table: %" PRIuS
				   " vs. %u at scale %.2f",
				   nonpacked_mipmap.image_files.size(), nr_frames_, static_cast<double>(mipmap.first));
			}
			if (nonpacked_mipmap.has_playercolor_masks != should_have_playercolor) {
				throw Widelands::GameDataError(
				   "Mismatched existence of player colors in animation table for scales %.2f and %.2f",
				   static_cast<double>(mipmaps_.begin()->first), static_cast<double>(mipmap.first));
			}
		}
		if (mipmaps_.count(1.0f) != 1) {
			throw Widelands::GameDataError(
			   "All animations must provide images for the neutral scale (1.0)");
		}
	} catch (const LuaError& e) {
		throw Widelands::GameDataError("Error in animation table: %s.", e.what());
	}
}

const Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	const NonPackedMipMapEntry& mipmap =
	   dynamic_cast<const NonPackedMipMapEntry&>(mipmap_entry(1.0f));
	assert(!mipmap.image_files.empty());
	const std::string& image_filename = mipmap.image_files[representative_frame()];
	const Image* image = (mipmap.has_playercolor_masks && (clr != nullptr)) ?
                           playercolor_image(*clr, image_filename) :
                           g_image_cache->get(image_filename);

	const int w = image->width();
	const int h = image->height();

	Texture* rv = new Texture(w, h);
	rv->blit(Rectf(0.f, 0.f, w, h), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

void NonPackedAnimation::add_scale_if_files_present(const std::string& basename,
                                                    const std::string& directory,
                                                    float scale_as_float,
                                                    const std::string& scale_as_string) {
	std::vector<std::string> filenames =
	   g_fs->get_sequential_files(directory, basename + scale_as_string, "png");
	if (!filenames.empty()) {
		mipmaps_.insert(std::make_pair(
		   scale_as_float,
		   std::unique_ptr<NonPackedMipMapEntry>(new NonPackedMipMapEntry(std::move(filenames)))));
	}
}
