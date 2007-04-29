/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "editor_game_base.h" //TODO: replace, only needed for "Time"
#include "geometry.h"
#include "rgbcolor.h"
#include "types.h"

#include <vector>

class Editor_Game_Base;
class Player;
class Surface;

/**
 * This abstract class represents anything that can be rendered to.
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
	RenderTarget(Surface* bmp);
	~RenderTarget();
	void get_window(Rect* rc, Point* ofs) const;
	void set_window(const Rect& rc, const Point& ofs);
	bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	int get_w() const;
	int get_h() const;

	void draw_line(int x1, int y1, int x2, int y2, RGBColor color);
	void draw_rect(Rect r, const RGBColor clr);
	void fill_rect(Rect r, const RGBColor clr);
	void brighten_rect(Rect r, const int factor);
	void clear();

	void blit(Point dst, const uint picture);
	void blitrect(Point dst, const uint picture, Rect src);
	void tile(Rect r, const uint picture, Point ofs);

	void rendermap (const Editor_Game_Base & egbase,
			const Player * const player, //  Will be 0 when called from the editor.
			Point viewofs, const bool draw_all);

	void renderminimap(const Editor_Game_Base  & egbase,
			   const Player * const player, const Point viewpoint,
			   const uint flags);

	void drawanim(Point dst, const uint animation, const uint time,
		      const Player * const player=0);

	void drawanimrect(Point dst, const uint animation, const uint time,
			  const Player * const player, Rect srcrc);

	void reset();

protected:
	bool clip(Rect & r) const throw ();

	void doblit(Point dst, Surface * const src, Rect srcrc);

	Sint8 node_brightness(const Editor_Game_Base::Time gametime,
			      Editor_Game_Base::Time last_seen,
			      const Vision vision, Sint8 result);

	///Only needed, when this is a mapview
	Surface* m_ground_surface;
	///The target surface
	Surface * m_surface;
	///The current clip rectangle
	Rect m_rect;
	///Drawing offset
	Point m_offset;
};

#endif
