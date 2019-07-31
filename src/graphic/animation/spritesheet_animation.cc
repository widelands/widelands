/*
 * Copyright (C) 2019 by the Widelands Development Team
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

#include "graphic/animation/spritesheet_animation.h"

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

SpriteSheetAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::MipMapEntry::MipMapEntry(const std::string& file, int init_rows, int init_columns)
   : has_playercolor_masks(false), sheet_file(file), sheet(nullptr), playercolor_mask_sheet(nullptr), rows(init_rows), columns(init_columns), width(0), height(0), playercolor_mask_sheet_file("") {

	assert(g_fs->file_exists(file));

	playercolor_mask_sheet_file = file;
	boost::replace_last(playercolor_mask_sheet_file, ".png", "_pc.png");
	if (g_fs->file_exists(playercolor_mask_sheet_file)) {
		has_playercolor_masks = true;
	} else {
		playercolor_mask_sheet_file = "";
	}
}


/*
==============================================================================

SpriteSheetAnimation IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::SpriteSheetAnimation(const LuaTable& table, const std::string& basename)
   : Animation(table) {
	assert(table.has_key("columns"));
	try {
		// Get image files
		if (basename.empty() || !table.has_key("directory")) {
			throw Widelands::GameDataError(
			   "Animation did not define both a basename and a directory for its sprite sheet file");
		}
		const std::string directory = table.get_string("directory");

		// Frames, rows and columns
		nr_frames_ = table.get_int("frames");
		rows_ = table.get_int("rows");
		columns_ = table.get_int("columns");

		// Look for a file for the given scale, and if we have it, add a mipmap entry for it.
		auto add_scale = [this, basename, directory](
							float scale_as_float, const std::string& scale_as_string) {
			const std::string path =
			   directory + g_fs->file_separator() + basename + scale_as_string + ".png";
			if (g_fs->file_exists(path)) {
				mipmaps_.insert(std::make_pair(
				   scale_as_float, std::unique_ptr<MipMapEntry>(new MipMapEntry(path, rows_, columns_))));
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
				   "Animation in directory '%s' with basename '%s' has no sprite sheet for mandatory "
				   "scale '1' in mipmap - supported scales are: 0.5, 1, 2, 4",
				   directory.c_str(), basename.c_str());
			}
		}

		// Perform some checks to make sure that the data is complete and consistent
		if (table.has_key("fps") && nr_frames_ == 1) {
				throw Widelands::GameDataError("Animation with one frame in sprite sheet %s must not have 'fps'",
				                               mipmaps_.begin()->second->sheet_file.c_str());
		}

		if (representative_frame() < 0 || representative_frame() > nr_frames_ - 1) {
			throw Widelands::GameDataError("Animation has %d as its representative frame, but the frame indices "
			                 "available are 0 - %d",
			                 representative_frame(), nr_frames_ - 1);
		}

		if (rows_ * columns_ < nr_frames_) {
			throw Widelands::GameDataError("Animation has %d frames, which does not fit into %d rows x %d columns",
										   nr_frames_, rows_, columns_);
		}

		const bool should_have_playercolor = mipmaps_.begin()->second->has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
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
void SpriteSheetAnimation::MipMapEntry::ensure_graphics_are_loaded() const {
	if (sheet == nullptr) {
		const_cast<MipMapEntry*>(this)->load_graphics();
	}
}

// Load the needed graphics from disk.
void SpriteSheetAnimation::MipMapEntry::load_graphics() {

	sheet = g_gr->images().get(sheet_file);

	if (!playercolor_mask_sheet_file.empty()) {
		playercolor_mask_sheet = g_gr->images().get(playercolor_mask_sheet_file);

		if (sheet->width() != playercolor_mask_sheet->width()) {
			throw Widelands::GameDataError(
			   "animation sprite sheet has width %d but playercolor mask sheet has width %d. The sheet's image is %s",
			   sheet->width(), playercolor_mask_sheet->width(), sheet_file.c_str());
		}
		if (sheet->height() != playercolor_mask_sheet->height()) {
			throw Widelands::GameDataError(
			   "animation sprite sheet has height %d but playercolor mask sheet has height %d. The sheet's image is %s",
			   sheet->height(), playercolor_mask_sheet->height(), sheet_file.c_str());
		}
	}

	// Frame width and height
	width = sheet->width() / columns;
	height = sheet->height() / rows;
}

void SpriteSheetAnimation::MipMapEntry::blit(uint32_t idx,
                                           const Rectf& source_rect,
                                           const Rectf& destination_rect,
                                           const RGBColor* clr,
                                           Surface* target) const {
	ensure_graphics_are_loaded();
	assert(sheet != nullptr);
	assert(target);
	assert(static_cast<int>(idx) <= columns * rows);

	// NOCOM reuse calculation for representative image
	const int column = idx % columns;
	const int row = idx / rows;

	Rectf frame_rect(source_rect.x + column * width, source_rect.y + row * height,
					 source_rect.w, source_rect.h);

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *sheet, frame_rect, 1., BlendMode::UseAlpha);
	} else {
		assert(playercolor_mask_sheet != nullptr);
		target->blit_blended(
		   destination_rect, *sheet, *playercolor_mask_sheet, frame_rect, *clr);
	}
}

// NOCOM code duplication
float SpriteSheetAnimation::find_best_scale(float scale) const {
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

float SpriteSheetAnimation::height() const {
	return mipmap_entry(1.0f).height;
}

float SpriteSheetAnimation::width() const {
	return mipmap_entry(1.0f).width;
}

std::vector<const Image*> SpriteSheetAnimation::images(float) const {
	// We only need to implement this if we add compressed spritemaps, or maybe for usage in a test
	NEVER_HERE();
}

std::vector<const Image*> SpriteSheetAnimation::pc_masks(float) const {
	// We only need to implement this if we add compressed spritemaps, or maybe for usage in a test
	NEVER_HERE();
}

// NOCOM code duplication
std::set<float> SpriteSheetAnimation::available_scales() const  {
	std::set<float> result;
	for (float scale : kSupportedScales) {
		if (mipmaps_.count(scale) == 1) {
			result.insert(scale);
		}
	}
	return result;
}

const Image* SpriteSheetAnimation::representative_image(const RGBColor* clr) const {
	const MipMapEntry& mipmap = mipmap_entry(1.0f);
	const int column = representative_frame() % columns_;
	const int row = representative_frame() / rows_;

	// NOCOM inefficient - we should only compose the frame
	const Image* image =
	(mipmap.has_playercolor_masks && clr) ?
	                        playercolor_image(*clr, mipmap.sheet_file) :
	                        g_gr->images().get(mipmap.sheet_file);

	const int w = width();
	const int h = height();

	Texture* rv = new Texture(w, h);
	rv->blit(Rectf(column * w, row * h, w, h), *image, Rectf(0.f, 0.f, w, h), 1., BlendMode::Copy);
	return rv;
}

// NOCOM code duplication
// NOCO Barbarian carriers are holding saws
Rectf SpriteSheetAnimation::source_rectangle(const int percent_from_bottom, float scale) const {
	const MipMapEntry& mipmap = mipmap_entry(find_best_scale(scale));
	const float h = percent_from_bottom * mipmap.height / 100;
	// Using floor for pixel perfect positioning
	return Rectf(0.f, std::floor(mipmap.height - h), mipmap.width, h);
}

// NOCOM code duplication
void SpriteSheetAnimation::blit(uint32_t time,
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

// NOCOM code duplication
void SpriteSheetAnimation::load_default_scale_and_sounds() const {
	mipmaps_.at(1.0f)->ensure_graphics_are_loaded();
	load_sounds();
}

// NOCOM code duplication
const SpriteSheetAnimation::MipMapEntry& SpriteSheetAnimation::mipmap_entry(float scale) const {
	assert(mipmaps_.count(scale) == 1);
	const MipMapEntry& mipmap = *mipmaps_.at(scale);
	mipmap.ensure_graphics_are_loaded();
	return mipmap;
}
