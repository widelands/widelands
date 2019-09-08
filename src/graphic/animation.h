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

#ifndef WL_GRAPHIC_ANIMATION_H
#define WL_GRAPHIC_ANIMATION_H

#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "base/macros.h"
#include "base/rect.h"
#include "base/vector.h"
#include "graphic/surface.h"
#include "logic/widelands_geometry.h"

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
	Animation(int representative_frame) : representative_frame_(representative_frame) {
	}
	virtual ~Animation() {
	}

	/// The height of this animation.
	virtual float height() const = 0;

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
	virtual uint32_t frametime() const = 0;

	/// An image of the first frame, blended with the given player color.
	/// The 'clr' is the player color used for blending - the parameter can be
	/// 'nullptr', in which case the neutral image will be returned.
	virtual const Image* representative_image(const RGBColor* clr) const = 0;

	/// Blit the animation frame that should be displayed at the given time index
	/// into the given 'destination_rect'.
	/// 'source_rect' defines the part of the animation that should be blitted.
	/// The 'clr' is the player color used for blitting - the parameter can be 'nullptr',
	/// in which case the neutral image will be blitted. The Surface is the 'target'
	/// for the blit operation and must be non-null.
	virtual void blit(uint32_t time,
	                  const Widelands::Coords& coords,
	                  const Rectf& source_rect,
	                  const Rectf& destination_rect,
	                  const RGBColor* clr,
	                  Surface* target,
	                  float scale) const = 0;

	/// Load animation images into memory for default scale.
	virtual void load_default_scale_and_sounds() const = 0;

protected:
	/// Play the sound effect associated with this animation at the given time.
	/// Any sound effects are played with stereo position according to 'coords'.
	/// If 'coords' == Widelands::Coords::null(), skip playing any sound effects.
	virtual void trigger_sound(uint32_t time, const Widelands::Coords& coords) const = 0;

protected:
	int representative_frame_;

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
	 * The 'basename' is the filename prefix for loading the images, e.g. "idle" or "walk_ne".
	 *
	 * The Lua table must contain a table 'pictures' with image paths and a 'hotspot' table.
	 *
	 * Optional parameters in the Lua table are 'fps' and 'sound_effect'.
	 */
	uint32_t load(const LuaTable& table, const std::string& basename);
	/// Same as above, but this animation will be used for getting a representative image by map
	/// object name
	uint32_t
	load(const std::string& map_object_name, const LuaTable& table, const std::string& basename);

	/// Returns the animation with the given ID or throws an exception if it is
	/// unknown.
	const Animation& get_animation(uint32_t id) const;

	/// Returns the representative image, using the given player color.
	/// If this image has been generated before, it is pulled from the cache using
	/// the clr argument that was used previously.
	const Image* get_representative_image(uint32_t id, const RGBColor* clr = nullptr);
	const Image* get_representative_image(const std::string& map_object_name,
	                                      const RGBColor* clr = nullptr);

private:
	std::vector<std::unique_ptr<Animation>> animations_;
	std::map<std::pair<uint32_t, const RGBColor*>, std::unique_ptr<const Image>>
	   representative_images_;
	std::map<std::string, uint32_t> representative_animations_by_map_object_name_;
};

#endif  // end of include guard: WL_GRAPHIC_ANIMATION_H
