/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H

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
 * Implements the Animation interface for an animation that is unpacked on disk, that
 * is every frame and every pc color frame is an singular file on disk.
 */
class NonPackedAnimation : public Animation {
public:
	~NonPackedAnimation() override {
	}
	explicit NonPackedAnimation(const LuaTable& table);

	// Implements Animation.
	float height() const override;
	float width() const override;
	Rectf source_rectangle(int percent_from_bottom, float scale) const override;
	Rectf destination_rectangle(const Vector2f& position,
	                            const Rectf& source_rect,
	                            float scale) const override;
	uint16_t nr_frames() const override;

	const Image* representative_image(const RGBColor* clr) const override;
	const std::string& representative_image_filename() const override;
	virtual void blit(uint32_t time,
	                  const Rectf& source_rect,
	                  const Rectf& destination_rect,
	                  const RGBColor* clr,
	                  Surface* target, float scale) const override;

	std::vector<const Image*> images(float scale) const override;
	std::vector<const Image*> pc_masks(float scale) const override;

private:
	float find_best_scale(float scale) const;

	// Loads the graphics if they are not yet loaded.
	void ensure_graphics_are_loaded() const;

	// Load the needed graphics from disk.
	void load_graphics();

	struct MipMapEntry {
		explicit MipMapEntry(float scale, const LuaTable& table);

		bool hasplrclrs;
		std::vector<std::string> image_files;
		std::vector<std::string> pc_mask_image_files;

		std::vector<const Image*> frames;
		std::vector<const Image*> pcmasks;
	};

	struct MipMapCompare {
	  bool operator() (const float lhs, const float rhs) const
	  {return lhs > rhs;}
	};
	std::map<float, std::unique_ptr<MipMapEntry>, MipMapCompare> mipmaps_;

	uint16_t nr_frames_;
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_NONPACKED_ANIMATION_H
