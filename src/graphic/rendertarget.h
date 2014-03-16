/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include <vector>

#include "align.h"
#include "graphic/compositemode.h"
#include "graphic/image.h"
#include "rect.h"
#include "rgbcolor.h"

class Surface;

namespace Widelands {
class Player;
};

/**
 * This class represents anything that can be rendered to.
 *
 * It supports windows, which are composed of a clip rectangle and a drawing
 * offset:
 * The drawing offset will be added to all coordinates that are passed to
 * drawing routines. Therefore, the offset is usually negative. Then the
 * coordinates are interpreted as relative to the clip rectangle and the
 * primitives are clipped accordingly.
 * \ref enter_window() can be used to enter a sub-window of the current window.
 * When you're finished drawing in the window, restore the old window by calling
 * \ref set_window() with the values stored in previous and prevofs.
 * \note If the sub-window would be empty/invisible, \ref enter_window() returns
 * false and doesn't change the window state at all.
*/
class RenderTarget {
public:
	RenderTarget(Surface*);
	void set_window(const Rect& rc, const Point& ofs);
	bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	int32_t width() const;
	int32_t height() const;

	void draw_line
		(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const RGBColor& color, uint8_t width = 1);
	void draw_rect(const Rect&, const RGBColor&);
	void fill_rect(const Rect&, const RGBAColor&);
	void brighten_rect(const Rect&, int32_t factor);

	void blit(const Point& dst, const Image* image, Composite cm = CM_Normal, UI::Align = UI::Align_TopLeft);
	void blitrect(const Point& dst, const Image* image, const Rect& src, Composite cm = CM_Normal);
	void tile(const Rect&, const Image* image, const Point& ofs, Composite cm = CM_Normal);

	void drawanim(const Point& dst, uint32_t animation, uint32_t time, const Widelands::Player* = 0);
	void drawanimrect
		(const Point& dst, uint32_t animation, uint32_t time, const Widelands::Player*, const Rect& srcrc);

	void reset();

	Surface* get_surface() const {return m_surface;}
	const Rect& get_rect() const {return m_rect;}
	const Point& get_offset() const {return m_offset;}

protected:
	bool clip(Rect & r) const;
	bool to_surface_geometry(Point* dst, Rect* srcrc) const;

	///The target surface
	Surface* m_surface;
	///The current clip rectangle
	Rect m_rect;
	///Drawing offset
	Point m_offset;
};

#endif
