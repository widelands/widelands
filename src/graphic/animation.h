/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

// NOCOM(#sirver): check includes and decls
class Image;
struct RGBColor;
class Surface;
struct Section;

// NOCOM(#sirver): docu
class Animation : boost::noncopyable {
public:
	Animation() {}
	virtual ~Animation() {}

	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;
	virtual const Image& get_frame(uint32_t time) const = 0;
	virtual const Image& get_frame(uint32_t i, const RGBColor& playercolor) const = 0;
	virtual const Point& hotspot() const = 0;
	virtual uint16_t nr_frames() const = 0;

	// NOCOM(#sirver): should take 'time'
	virtual void trigger_soundfx(uint32_t framenumber, uint32_t stereo_position) const = 0;

	virtual void blit(uint32_t time, const Point&, const Rect& srcrc, const RGBColor* clr, Surface*) const = 0;
};

/**
* The animation manager manages a list of all active animations.
* They are flushed after a game is finished and loaded at gfxload time by the
* Game code.
* get() only works properly before gfxload. This means that all animations must
* be loaded before the game starts.
*
* Note that animation IDs are counted from 1, while the m_animations array is
* counted from 0.
*/
struct AnimationManager {
	uint32_t get
		(const std::string & directory,
		 Section           & s,
		 char       const * picnametempl = 0)
	{
		return get(directory.c_str(), s, picnametempl);
	}
	uint32_t get
		(char       const * directory,
		 Section          &,
		 char       const * picnametempl = 0);

	// for use by the graphics subsystem
	const Animation& get_animation(uint32_t id) const;

private:
	std::vector<Animation*> m_animations;
};

#endif
