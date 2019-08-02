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

#ifndef WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/rect.h"
#include "base/vector.h"
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
	explicit SpriteSheetAnimation(const LuaTable& table, const std::string& basename);

	// Implements Animation.
	const Image* representative_image(const RGBColor* clr) const override;

	std::vector<const Image*> images(float scale) const override;
	std::vector<const Image*> pc_masks(float scale) const override;

private:
	// Load the needed graphics from disk.
	void load_graphics();

	struct SpriteSheetMipMapEntry : Animation::MipMapEntry {
		explicit SpriteSheetMipMapEntry(const std::string& file, int init_rows, int columns);

		// Loads the graphics if they are not yet loaded.
		void ensure_graphics_are_loaded() const override;

		// Load the needed graphics from disk.
		void load_graphics() override;

		void blit(uint32_t idx,
		          const Rectf& source_rect,
		          const Rectf& destination_rect,
		          const RGBColor* clr,
		          Surface* target) const override;

		int width() const override;
		int height() const override;

		// Sprite sheet file name on disk
		const std::string sheet_file;

		// Loaded sprite sheet for all frames
		const Image* sheet;

		// Loaded player color mask sprite sheet for all frames
		const Image* playercolor_mask_sheet;

		const int rows;
		const int columns;
		int w;
		int h;

	private:
		// Player color mask file on disk
		std::string playercolor_mask_sheet_file;
	};

	int rows_;
	int columns_;
};
#endif  // end of include guard: WL_GRAPHIC_ANIMATION_SPRITESHEET_ANIMATION_H
