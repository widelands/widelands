/*
 * Copyright (C) 2002-2004 by the Wide Lands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef RGBCOLOR_H
#define RGBCOLOR_H

#include <SDL.h>
#include "types.h"

/*
==============================================================================

COLOR HANDLING

==============================================================================
*/

/*
class RGBColor
*/
class RGBColor {
   SDL_Color m_color;

public:
	inline RGBColor() { }
	inline RGBColor(uchar r, uchar g, uchar b) {
		m_color.r = r;
      m_color.g = g;
      m_color.b = b;
	}

	inline void set(uchar r, uchar g, uchar b) {
		m_color.r = r;
      m_color.g = g;
      m_color.b = b;
	}

	inline uchar r() const { return m_color.r; }
	inline uchar g() const { return m_color.g; }
	inline uchar b() const { return m_color.b; }

	inline ulong map(SDL_PixelFormat* fmt) const {
         return SDL_MapRGB(fmt, m_color.r, m_color.g, m_color.b);
   }
	inline void set(SDL_PixelFormat* fmt, ulong clr) {
		SDL_GetRGB(clr, fmt, &m_color.r, &m_color.g, &m_color.b);
	}

   inline bool operator==(const RGBColor& colin) const {
      return ( m_color.r  == colin.m_color.r &&
            m_color.g == colin.m_color.g &&
            m_color.b == colin.m_color.b );
   }
};

#endif /* RGBCOLOR_H */
