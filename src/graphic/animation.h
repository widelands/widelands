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

#ifndef ANIMATION_H
#define ANIMATION_H

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include "point.h"
#include "rect.h"

class Image;
class Surface;
struct RGBColor;
struct Section;

/**
 * Representation of an Animation in the game. An animation is a looping set of
 * image frames and their corresponding sound effects. This class makes no
 * guarantees on how the graphics are represented in memory - but knows how to
 * render itself at a given time to the given place.
 *
 * The dimensions of an animation is constant and can not change from frame to
 * frame.
 */
class Animation : boost::noncopyable {
public:
	Animation() {}
	virtual ~Animation() {}

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

	// An image frame that shows the first animation frame, colored using the
	// 'player_clr'. This can be used in the UI (e.g. buildingwindow) to
	// represent this image.
	virtual const Image& representative_image(const RGBColor& player_clr) const = 0;

	/// Blit the animation frame that should be displayed at the given time index
	/// so that the given point is at the top left of the frame. Srcrc defines
	/// the part of the animation that should be blitted. The 'clr' is the player
	/// color used for blitting - the parameter can be NULL in which case the
	/// neutral image will be blitted. The Surface is the target for the blit
	/// operation and must be non-null.
	virtual void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const = 0;

	/// Play the sound effect associated with this animation at the given time.
	virtual void trigger_soundfx(uint32_t time, uint32_t stereo_position) const = 0;
};

/**
* The animation manager manages a list of all active animations.
*/
class AnimationManager {
public:
	/**
	 * Loads an animation, graphics sound and everything.
	 *
	 * The animation resides in the given directory and is described by the
	 * given section.
	 *
	 * This function looks for image files as defined by the 'pics' key. If this
	 * is not present, it will try \<sectionname\>_??.png
	 *
	 * \param directory     which directory to look in for image and sound files
	 * \param s             conffile section to search for data on this animation
	*/
	uint32_t load(const std::string & directory, Section& s);

	/// Returns the animation with the given ID or throws an exception if it is
	/// unknown.
	const Animation& get_animation(uint32_t id) const;

private:
	std::vector<Animation*> m_animations;
};

#endif
