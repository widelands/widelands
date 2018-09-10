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

#ifndef WL_GRAPHIC_ANIMATION_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_ANIMATION_H

#include <set>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "base/macros.h"
#include "base/rect.h"
#include "base/vector.h"
#include "graphic/color.h"
#include "graphic/image.h"
#include "graphic/surface.h"
#include "scripting/lua_table.h"

/// The default animation speed
constexpr int kFrameLength = 250;

/**
 * Representation of an Animation in the game. An animation is a looping set of
 * image frames and their corresponding sound effects. This class makes no
 * guarantees on how the graphics are represented in memory - but knows how to
 * render itself at a given time to the given place.
 *
 * The dimensions of an animation is constant and can not change from frame to
 * frame.
 */
class Animation {
public:
	enum class Type {
		kUndefined,
		kNonPacked,
		kSpritesheet,
	};

	explicit Animation(const LuaTable& table, Animation::Type type);
	virtual ~Animation() {
	}

	/// Animation type for safety in create_spritemap
	Animation::Type type() const;

	/// The height of this animation.
	virtual float height() const = 0;
	virtual float width() const  = 0;
	const Vector2i& hotspot() const;

	uint32_t current_frame(uint32_t time) const;

	/// The size of the animation source images in pixels. Use 'percent_from_bottom' to crop the
	/// animation.
	virtual Rectf source_rectangle(int percent_from_bottom, float scale) const = 0;

	/// Calculates the destination rectangle for blitting the animation in pixels.
	/// 'position' is where the top left corner of the animation will end up,
	/// 'source_rect' is the rectangle calculated by source_rectangle,
	/// 'scale' is the zoom scale.
	virtual Rectf
	destination_rectangle(const Vector2f& position, const Rectf& source_rect, float scale) const = 0;

	/// The number of animation frames of this animation.
	virtual uint16_t nr_frames() const = 0;

	/// The number of milliseconds each frame will be displayed.
	uint32_t frametime() const;

	/// An image of the first frame, blended with the given player color.
	/// The 'clr' is the player color used for blending - the parameter can be
	/// 'nullptr', in which case the neutral image will be returned.
	virtual const Image* representative_image(const RGBColor* clr) const = 0;
	/// The filename of the image used for the first frame, without player color.
	virtual const std::string& representative_image_filename() const = 0;

	/// Blit the animation frame that should be displayed at the given time index
	/// into the given 'destination_rect'.
	/// 'source_rect' defines the part of the animation that should be blitted.
	/// The 'clr' is the player color used for blitting - the parameter can be 'nullptr',
	/// in which case the neutral image will be blitted. The Surface is the 'target'
	/// for the blit operation and must be non-null.
	virtual void blit(uint32_t time,
	                  const Rectf& source_rect,
	                  const Rectf& destination_rect,
	                  const RGBColor* clr,
	                  Surface* target, float scale) const = 0;

	/// We need to expose these for the packed animation,
	/// so that the create_spritesheet utility can use them
	virtual std::vector<const Image*> images(float scale) const = 0;
	/// We need to expose these for the packed animation,
	/// so that the create_spritemap utility can use them
	virtual std::vector<const Image*> pc_masks(float scale) const = 0;

	/// Play the sound effect associated with this animation at the given time.
	void trigger_sound(uint32_t time, uint32_t stereo_position) const;

protected:
	uint32_t frametime_;

private:
	DISALLOW_COPY_AND_ASSIGN(Animation);

	Animation::Type type_;
	Vector2i hotspot_ = Vector2i::zero();
	const bool play_once_;

	// Name of sound effect that will be played at frame 0.
	std::string sound_effect_;
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_ANIMATION_H
