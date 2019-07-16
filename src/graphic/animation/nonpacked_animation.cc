/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "graphic/animation/nonpacked_animation.h"

#include <cassert>
#include <cstdio>
#include <limits>
#include <memory>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/playercolor.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

/*
==============================================================================

NonPackedAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::MipMapEntry::MipMapEntry(std::vector<std::string> files)
   : has_playercolor_masks(false), image_files(files) {
	if (image_files.empty()) {
		throw Widelands::GameDataError(
		   "Animation without image files. For a scale of 1.0, the template should look similar to "
		   "this:"
		   " 'directory/idle_1_??.png' for 'directory/idle_1_00.png' etc.");
	}

	for (std::string image_file : image_files) {
		boost::replace_last(image_file, ".png", "_pc.png");
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


/*
==============================================================================

NonPackedAnimation IMPLEMENTATION

==============================================================================
*/

NonPackedAnimation::NonPackedAnimation(const LuaTable& table, const std::string& basename)
   : Animation(table) {
	try {
		// Get image files
		if (table.has_key("pictures")) {
			// TODO(GunChleoc): Old code - remove this option once conversion has been completed
			mipmaps_.insert(std::make_pair(
			   1.0f, std::unique_ptr<MipMapEntry>(
			            new MipMapEntry(table.get_table("pictures")->array_entries<std::string>()))));
		} else {
			if (basename.empty() || !table.has_key("directory")) {
				throw Widelands::GameDataError(
				   "Animation did not define both a basename and a directory for its image files");
			}
			const std::string directory = table.get_string("directory");

			// List files for the given scale, and if we have any, add a mipmap entry for them.
			auto add_scale = [this, basename, directory](
			                    float scale_as_float, const std::string& scale_as_string) {
				std::vector<std::string> filenames =
				   g_fs->get_sequential_files(directory, basename + scale_as_string, "png");
				if (!filenames.empty()) {
					mipmaps_.insert(std::make_pair(
					   scale_as_float, std::unique_ptr<MipMapEntry>(new MipMapEntry(filenames))));
				}
			};
			add_scale(0.5f, "_0.5");
			add_scale(1.0f, "_1");
			add_scale(2.0f, "_2");
			add_scale(4.0f, "_4");

			if (mipmaps_.count(1.0f) == 0) {
				// There might be only 1 scale
				add_scale(1.0f, "");
				if (mipmaps_.count(1.0f) == 0) {
					// No files found at all
					throw Widelands::GameDataError(
					   "Animation in directory '%s' with basename '%s' has no images for mandatory "
					   "scale '1' in mipmap - supported scales are: 0.5, 1, 2, 4",
					   directory.c_str(), basename.c_str());
				}
			}
		}

		// Frames
		nr_frames_ = mipmaps_.begin()->second->image_files.size();
		if (table.has_key("fps") && nr_frames_ == 1) {
				throw Widelands::GameDataError("Animation with one picture %s must not have 'fps'",
				                               mipmaps_.begin()->second->image_files.front().c_str());
		}

		if (representative_frame() < 0 || representative_frame() > nr_frames_ - 1) {
			throw wexception("Animation has %d as its representative frame, but the frame indices "
			                 "available are 0 - %d",
			                 representative_frame(), nr_frames_ - 1);
		}

		// Perform some checks to make sure that the data is complete and consistent
		const bool should_have_playercolor = mipmaps_.begin()->second->has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
			if (mipmap.second->image_files.size() != nr_frames_) {
				throw Widelands::GameDataError(
				   "Mismatched number of images for different scales in animation table: %" PRIuS
				   " vs. %u at scale %.2f",
				   mipmap.second->image_files.size(), nr_frames_, static_cast<double>(mipmap.first));
			}
			if (mipmap.second->has_playercolor_masks != should_have_playercolor) {
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
		throw Widelands::GameDataError("Error in animation table: %s", e.what());
	}
}

// Loads the graphics if they are not yet loaded.
void NonPackedAnimation::MipMapEntry::ensure_graphics_are_loaded() const {
	if (frames.empty()) {
		const_cast<MipMapEntry*>(this)->load_graphics();
	}
}

// Load the needed graphics from disk.
void NonPackedAnimation::MipMapEntry::load_graphics() {
	if (image_files.empty()) {
		throw Widelands::GameDataError("animation without image files.");
	}
	if (playercolor_mask_image_files.size() &&
	    playercolor_mask_image_files.size() != image_files.size()) {
		throw Widelands::GameDataError(
		   "animation has %" PRIuS " frames but playercolor mask has %" PRIuS
		   " frames. First image is %s",
		   image_files.size(), playercolor_mask_image_files.size(), image_files.front().c_str());
	}

	for (const std::string& filename : image_files) {
		const Image* image = g_gr->images().get(filename);
		if (frames.size() && (frames.front()->width() != image->width() ||
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
		const Image* pc_image = g_gr->images().get(filename);
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

void NonPackedAnimation::MipMapEntry::blit(uint32_t idx,
                                           const Rectf& source_rect,
                                           const Rectf& destination_rect,
                                           const RGBColor* clr,
                                           Surface* target) const {
	assert(!frames.empty());
	assert(target);
	assert(idx < frames.size());

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *frames.at(idx), source_rect, 1., BlendMode::UseAlpha);
	} else {
		target->blit_blended(
		   destination_rect, *frames.at(idx), *playercolor_mask_frames.at(idx), source_rect, *clr);
	}
}

float NonPackedAnimation::find_best_scale(float scale) const {
	assert(!mipmaps_.empty());
	float result = mipmaps_.begin()->first;
	for (const auto& mipmap : mipmaps_) {
		// The map is reverse sorted, so we can break as soon as we are lower than the wanted scale
		if (mipmap.first < scale) {
			break;
		}
		result = mipmap.first;
	}
	return result;
}

float NonPackedAnimation::height() const {
	return mipmap_entry(1.0f).frames.front()->height();
}

float NonPackedAnimation::width() const {
	return mipmap_entry(1.0f).frames.front()->width();
}

std::vector<const Image*> NonPackedAnimation::images(float scale) const {
	return mipmap_entry(scale).frames;
}

std::vector<const Image*> NonPackedAnimation::pc_masks(float scale) const {
	return mipmap_entry(scale).playercolor_mask_frames;
}

std::set<float> NonPackedAnimation::available_scales() const  {
	std::set<float> result;
	for (float scale : kSupportedScales) {
		if (mipmaps_.count(scale) == 1) {
			result.insert(scale);
		}
	}
	return result;
}

const Image* NonPackedAnimation::representative_image(const RGBColor* clr) const {
	const MipMapEntry& mipmap = mipmap_entry(1.0f);
	std::vector<std::string> images = mipmap.image_files;
	assert(!images.empty());
	const Image* image = (mipmap.has_playercolor_masks && clr) ?
	                        playercolor_image(*clr, images[representative_frame()]) :
	                        g_gr->images().get(images[representative_frame()]);

	const int w = image->width();
	const int h = image->height();

	Texture* rv = new Texture(w, h);
	rv->blit(Rectf(0.f, 0.f, w, h), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

Rectf NonPackedAnimation::source_rectangle(const int percent_from_bottom, float scale) const {
	const Image* first_frame = mipmap_entry(find_best_scale(scale)).frames.front();
	const float h = percent_from_bottom * first_frame->height() / 100;
	// Using floor for pixel perfect positioning
	return Rectf(0.f, std::floor(first_frame->height() - h), first_frame->width(), h);
}

Rectf NonPackedAnimation::destination_rectangle(const Vector2f& position,
                                                const Rectf& source_rect,
                                                const float scale) const {
	const float best_scale = find_best_scale(scale);
	// Using floor + ceil for pixel perfect positioning
	return Rectf(std::floor(position.x - hotspot().x * scale - source_rect.x),
	             std::floor(position.y - hotspot().y * scale - source_rect.y),
	             std::ceil(source_rect.w * scale / best_scale), std::ceil(source_rect.h * scale / best_scale));
}

void NonPackedAnimation::blit(uint32_t time,
                              const Widelands::Coords& coords,
                              const Rectf& source_rect,
                              const Rectf& destination_rect,
                              const RGBColor* clr,
                              Surface* target,
                              float scale) const {
	mipmap_entry(find_best_scale(scale))
	   .blit(current_frame(time), source_rect, destination_rect, clr, target);
	trigger_sound(time, coords);
}

void NonPackedAnimation::load_default_scale_and_sounds() const {
	mipmaps_.at(1.0f)->ensure_graphics_are_loaded();
	load_sounds();
}

const NonPackedAnimation::MipMapEntry& NonPackedAnimation::mipmap_entry(float scale) const {
	assert(mipmaps_.count(scale) == 1);
	const MipMapEntry& mipmap = *mipmaps_.at(scale);
	mipmap.ensure_graphics_are_loaded();
	return mipmap;
}
