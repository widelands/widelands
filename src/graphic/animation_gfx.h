/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef ANIMATION_GFX_H
#define ANIMATION_GFX_H

#include <cstddef>
#include <vector>

#include "point.h"

class Image;
class ImageCache;
struct AnimationData;
struct RGBColor;

struct AnimationGfx { /// The graphics belonging to an animation.
	AnimationGfx(const AnimationData& data, ImageCache*);

	size_t nr_frames() const {return m_frames.size();}
	const Point& hotspot() const throw () {return m_hotspot;}
	const Image& get_frame(size_t i, const RGBColor& playercolor);
	const Image& get_frame(size_t i) const;

private:
	std::vector<const Image*> m_frames;
	std::vector<const Image*> m_pcmasks;
	Point m_hotspot;
	bool m_hasplrclrs;
};

#endif
