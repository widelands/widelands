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

#ifndef WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H

#include <memory>

#include "base/rect.h"
#include "graphic/animation/animation.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/surface.h"
#include "scripting/lua_table.h"

/**
 * Implements the Animation interface for an animation where the images are in a spritesheet.
 */
class SpriteSheetAnimation : public Animation {
public:
	~SpriteSheetAnimation() override = default;
	explicit SpriteSheetAnimation(const LuaTable& table,
	                              const std::string& basename,
	                              const std::string& animation_directory);

	const Image* representative_image(const RGBColor* clr) const override;

private:
	void add_scale_if_files_present(const std::string& basename,
	                                const std::string& directory,
	                                float scale_as_float,
	                                const std::string& scale_as_string) override;

	struct SpriteSheetMipMapEntry : Animation::MipMapEntry {
		explicit SpriteSheetMipMapEntry(const std::string& file, int init_rows, int columns);

		void ensure_graphics_are_loaded() const override;
		void load_graphics() override;

		void blit(uint32_t idx,
		          const Rectf& source_rect,
		          const Rectf& destination_rect,
		          const RGBColor* clr,
		          Surface* target,
		          float opacity) const override;

		int width() const override;
		int height() const override;

		/// Loaded sprite sheet for all frames
		const Image* sheet;

		/// Loaded player color mask sprite sheet for all frames
		const Image* playercolor_mask_sheet;

		/// Number of rows for the spritesheets
		const int rows;
		/// Number of columns for the spritesheets
		const int columns;
		/// Texture width
		int w;
		/// Texture height
		int h;

		std::vector<std::unique_ptr<const Texture>>
		frame_textures(bool return_playercolor_masks) const override;

	private:
		/// Sprite sheet file name on disk
		const std::string sheet_file;

		/// Player color mask file on disk
		std::string playercolor_mask_sheet_file;
	};

	/// Number of rows for the spritesheets
	int rows_;
	/// Number of columns for the spritesheets
	int columns_;
};
#endif  // end of include guard: WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H
