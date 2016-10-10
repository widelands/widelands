/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_H

#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "base/macros.h"
#include "base/point.h"
#include "base/rect.h"

class Image;
class LuaTable;
class Surface;
struct RGBColor;

/// FRAME_LENGTH is the default animation speed
constexpr int FRAME_LENGTH = 250;

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
	Animation() {
	}
	virtual ~Animation() {
	}

	/// The dimensions of this animation.
	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;

	/// The number of animation frames of this animation.
	virtual uint16_t nr_frames() const = 0;

	/// The number of milliseconds each frame will be displayed.
	virtual uint32_t frametime() const = 0;

	/// The hotspot of this animation. Note that this is ignored when blitting,
	/// so the caller has to adjust for the hotspot himself.
	virtual const Point& hotspot() const = 0;

	/// An image of the first frame, blended with the given player color.
	/// The 'clr' is the player color used for blending - the parameter can be
	/// 'nullptr', in which case the neutral image will be returned.
	virtual Image* representative_image(const RGBColor* clr) const = 0;
	/// The filename of the image used for the first frame, without player color.
	virtual const std::string& representative_image_filename() const = 0;

	/// Blit the animation frame that should be displayed at the given time index
	/// so that the given point is at the top left of the frame. Srcrc defines
	/// the part of the animation that should be blitted. The 'clr' is the player
	/// color used for blitting - the parameter can be 'nullptr', in which case the
	/// neutral image will be blitted. The Surface is the target for the blit
	/// operation and must be non-null.
	virtual void blit(
	   uint32_t time, const Rect& dstrc, const Rect& srcrc, const RGBColor* clr, Surface*) const = 0;

	/// Play the sound effect associated with this animation at the given time.
	virtual void trigger_sound(uint32_t time, uint32_t stereo_position) const = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(Animation);
};

/**
* The animation manager manages a list of all active animations.
*/
class AnimationManager {
public:
	/**
	 * Loads an animation, graphics sound and everything from a Lua table.
	 *
	 * The Lua table must contain a table 'pictures' with image paths and a 'hotspot' table.
	 *
	 * Optional parameters in the Lua table are 'fps' and 'sound_effect'.
	*/
	uint32_t load(const LuaTable& table);

	/// Returns the animation with the given ID or throws an exception if it is
	/// unknown.
	const Animation& get_animation(uint32_t id) const;

	/// Returns the representative image, using the given player color.
	/// If this image has been generated before, it is pulled from the cache using
	/// the clr argument that was used previously.
	const Image* get_representative_image(uint32_t id, const RGBColor* clr = nullptr);

private:
	std::vector<std::unique_ptr<Animation>> animations_;
	std::map<uint32_t, std::unique_ptr<Image>> representative_images_;
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_H
