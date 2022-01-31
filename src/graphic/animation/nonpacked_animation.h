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

#ifndef WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H

#include <memory>

#include "base/rect.h"
#include "graphic/animation/animation.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/surface.h"
#include "scripting/lua_table.h"

/**
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	~NonPackedAnimation() override = default;
	explicit NonPackedAnimation(const LuaTable& table,
	                            const std::string& basename,
	                            const std::string& animation_directory);

	const Image* representative_image(const RGBColor* clr) const override;

private:
	void add_scale_if_files_present(const std::string& basename,
	                                const std::string& directory,
	                                float scale_as_float,
	                                const std::string& scale_as_string) override;

	struct NonPackedMipMapEntry : Animation::MipMapEntry {
		explicit NonPackedMipMapEntry(std::vector<std::string> files);
		~NonPackedMipMapEntry() override = default;

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

		std::vector<std::unique_ptr<const Texture>>
		frame_textures(bool return_playercolor_masks) const override;

		/// Image files on disk
		std::vector<std::string> image_files;

		/// Loaded images for each frame
		std::vector<const Image*> frames;

		/// Loaded player color mask images for each frame
		std::vector<const Image*> playercolor_mask_frames;

	private:
		/// Player color mask files on disk
		std::vector<std::string> playercolor_mask_image_files;
	};
};
#endif  // end of include guard: WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H
