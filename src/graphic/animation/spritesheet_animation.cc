/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#include "graphic/animation/spritesheet_animation.h"

#include <cassert>
#include <memory>

#include "graphic/image.h"
#include "graphic/image_cache.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "scripting/lua_table.h"

/*
==============================================================================

SpriteSheetAnimation::MipMapEntry IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::SpriteSheetMipMapEntry::SpriteSheetMipMapEntry(const std::string& file,
                                                                     int init_rows,
                                                                     int init_columns)
   : sheet(nullptr),
     playercolor_mask_sheet(nullptr),
     rows(init_rows),
     columns(init_columns),
     w(0),
     h(0),
     sheet_file(file) {

	assert(g_fs->file_exists(file));

	playercolor_mask_sheet_file = file;
	replace_last(playercolor_mask_sheet_file, ".png", "_pc.png");
	if (g_fs->file_exists(playercolor_mask_sheet_file)) {
		has_playercolor_masks = true;
	} else {
		playercolor_mask_sheet_file = "";
	}
}

void SpriteSheetAnimation::SpriteSheetMipMapEntry::ensure_graphics_are_loaded() const {
	if (sheet == nullptr) {
		const_cast<SpriteSheetMipMapEntry*>(this)->load_graphics();
	}
}

void SpriteSheetAnimation::SpriteSheetMipMapEntry::load_graphics() {
	sheet = g_image_cache->get(sheet_file);

	if (!playercolor_mask_sheet_file.empty()) {
		playercolor_mask_sheet = g_image_cache->get(playercolor_mask_sheet_file);

		if (sheet->width() != playercolor_mask_sheet->width()) {
			throw Widelands::GameDataError("animation sprite sheet has width %d but playercolor mask "
			                               "sheet has width %d. The sheet's image is %s",
			                               sheet->width(), playercolor_mask_sheet->width(),
			                               sheet_file.c_str());
		}
		if (sheet->height() != playercolor_mask_sheet->height()) {
			throw Widelands::GameDataError("animation sprite sheet has height %d but playercolor mask "
			                               "sheet has height %d. The sheet's image is %s",
			                               sheet->height(), playercolor_mask_sheet->height(),
			                               sheet_file.c_str());
		}
	}

	// Frame width and height
	w = sheet->width() / columns;
	h = sheet->height() / rows;

	if ((w * columns) != sheet->width()) {
		throw Widelands::GameDataError(
		   "frame width (%d) x columns (%d) != sheet width (%d). The sheet's image is %s", w, columns,
		   sheet->width(), sheet_file.c_str());
	}
	if ((h * rows) != sheet->height()) {
		throw Widelands::GameDataError(
		   "frame height (%d) x rows (%d) != sheet height (%d). The sheet's image is %s", h, rows,
		   sheet->height(), sheet_file.c_str());
	}
}

void SpriteSheetAnimation::SpriteSheetMipMapEntry::blit(uint32_t idx,
                                                        const Rectf& source_rect,
                                                        const Rectf& destination_rect,
                                                        const RGBColor* clr,
                                                        Surface* target,
                                                        const float opacity) const {
	assert(sheet != nullptr);
	assert(target);
	assert(static_cast<int>(idx) <= columns * rows);

	const int column = idx % columns;
	const int row = idx / columns;

	Rectf frame_rect(source_rect.x + column * width(), source_rect.y + row * height(), source_rect.w,
	                 source_rect.h);

	if (!has_playercolor_masks || clr == nullptr) {
		target->blit(destination_rect, *sheet, frame_rect, opacity, BlendMode::UseAlpha);
	} else {
		assert(playercolor_mask_sheet != nullptr);
		target->blit_blended(destination_rect, *sheet, *playercolor_mask_sheet, frame_rect, *clr);
	}
}

std::vector<std::unique_ptr<const Texture>>
SpriteSheetAnimation::SpriteSheetMipMapEntry::frame_textures(bool return_playercolor_masks) const {
	ensure_graphics_are_loaded();

	std::vector<std::unique_ptr<const Texture>> result;
	const Rectf rect(Vector2f::zero(), width(), height());
	if (!return_playercolor_masks || has_playercolor_masks) {
		const size_t no_of_frames = rows * columns;
		for (size_t i = 0; i < no_of_frames; ++i) {
			std::unique_ptr<Texture> texture(new Texture(width(), height()));

			const int column = i % columns;
			const int row = i / columns;

			texture->fill_rect(rect, RGBAColor(0, 0, 0, 0));
			texture->blit(rect, return_playercolor_masks ? *playercolor_mask_sheet : *sheet,
			              Rectf(column * width(), row * height(), width(), height()), 1.,
			              BlendMode::Copy);
			result.push_back(std::move(texture));
		}
	}
	return result;
}

int SpriteSheetAnimation::SpriteSheetMipMapEntry::width() const {
	return w;
}
int SpriteSheetAnimation::SpriteSheetMipMapEntry::height() const {
	return h;
}

/*
==============================================================================

SpriteSheetAnimation IMPLEMENTATION

==============================================================================
*/

SpriteSheetAnimation::SpriteSheetAnimation(const LuaTable& table,
                                           const std::string& basename,
                                           const std::string& animation_directory)
   : Animation(table) {
	try {
		// Get image files
		// TODO(GunChleoc): When all animations have been converted, require that animation_directory
		// is not empty.
		const std::string directory =
		   animation_directory.empty() ? table.get_string("directory") : animation_directory;

		// Frames, rows and columns
		nr_frames_ = table.get_int("frames");
		rows_ = table.get_int("rows");
		columns_ = table.get_int("columns");

		add_available_scales(basename, directory);

		// Perform some checks to make sure that the data is complete and consistent
		const SpriteSheetMipMapEntry& first =
		   dynamic_cast<const SpriteSheetMipMapEntry&>(*mipmaps_.begin()->second);
		if (table.has_key("fps") && nr_frames_ == 1) {
			throw Widelands::GameDataError(
			   "'%s' sprite sheet animation with one frame must not have 'fps'", basename.c_str());
		}

		if (representative_frame() < 0 || representative_frame() > nr_frames_ - 1) {
			throw Widelands::GameDataError(
			   "Animation has %d as its representative frame, but the frame indices "
			   "available are 0 - %d",
			   representative_frame(), nr_frames_ - 1);
		}

		if (rows_ * columns_ < nr_frames_) {
			throw Widelands::GameDataError(
			   "Animation has %d frames, which does not fit into %d rows x %d columns", nr_frames_,
			   rows_, columns_);
		}
		if ((rows_ - 1) * columns_ > nr_frames_) {
			throw Widelands::GameDataError(
			   "Animation has %d frames, which is giving us an extra row in %d rows x %d columns",
			   nr_frames_, rows_, columns_);
		}

		const bool should_have_playercolor = first.has_playercolor_masks;
		for (const auto& mipmap : mipmaps_) {
			if (first.has_playercolor_masks != should_have_playercolor) {
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

const Image* SpriteSheetAnimation::representative_image(const RGBColor* clr) const {
	const SpriteSheetMipMapEntry& mipmap =
	   dynamic_cast<const SpriteSheetMipMapEntry&>(mipmap_entry(1.0f));
	const int column = representative_frame() % columns_;
	const int row = representative_frame() / columns_;
	const int w = width();
	const int h = height();

	Texture* rv = new Texture(w, h);
	Rectf rect(Vector2f::zero(), w, h);
	if (mipmap.has_playercolor_masks && clr) {
		rv->fill_rect(rect, RGBAColor(0, 0, 0, 0));
		rv->blit_blended(rect, *mipmap.sheet, *mipmap.playercolor_mask_sheet,
		                 Rectf(column * w, row * h, w, h), *clr);
	} else {
		rv->blit(rect, *mipmap.sheet, Rectf(column * w, row * h, w, h), 1., BlendMode::Copy);
	}
	return rv;
}

void SpriteSheetAnimation::add_scale_if_files_present(const std::string& basename,
                                                      const std::string& directory,
                                                      float scale_as_float,
                                                      const std::string& scale_as_string) {
	const std::string path =
	   directory + FileSystem::file_separator() + basename + scale_as_string + ".png";
	if (g_fs->file_exists(path)) {
		mipmaps_.insert(
		   std::make_pair(scale_as_float, std::unique_ptr<SpriteSheetMipMapEntry>(
		                                     new SpriteSheetMipMapEntry(path, rows_, columns_))));
	}
}
