/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "picture.h"
#include "rect.h"
#include "rgbcolor.h"

#include <vector>

//#ifndef PICTURE_ID
//#define PICTURE_ID
//struct Picture;
//typedef boost::shared_ptr<Picture> PictureID;
//#endif

namespace Widelands {
struct Editor_Game_Base;
struct Player;
};
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
struct RenderTarget {
	RenderTarget(Surface *);
	void set_window(Rect const & rc, Point const & ofs);
	bool enter_window(Rect const & rc, Rect * previous, Point * prevofs);

	int32_t get_w() const;
	int32_t get_h() const;

	void draw_line
		(int32_t x1,
		 int32_t y1,
		 int32_t x2,
		 int32_t y2,
		 RGBColor color);
	void draw_rect(Rect, RGBColor);
	void fill_rect(Rect, RGBColor);
	void brighten_rect(Rect, int32_t factor);
	void clear();

	void blit(Point dst, PictureID picture);
	void blitrect(Point dst, PictureID picture, Rect src);
	void tile(Rect, PictureID picture, Point ofs);

	/**
	 * Renders the map from a player's point of view into the current drawing
	 * window.
	 *
	 * Will call the function below when player.see_all().
	 *
	 * viewofs is the offset of the upper left corner of the window into the map,
	 * in pixels.
	 */
	void rendermap
		(Widelands::Editor_Game_Base const &       egbase,
		 Widelands::Player           const &       player,
		 Point                                     viewofs);

	/**
	 * Same as above but not from a player's point of view. Used in game when
	 * rendering for a player that sees all and the editor.
	 */
	void rendermap
		(Widelands::Editor_Game_Base const & egbase,
		 Point                               viewofs);

	/**
	 * Render the minimap. If player is not 0, it renders from that player's
	 * point of view.
	 */
	void renderminimap
		(Widelands::Editor_Game_Base const & egbase,
		 Widelands::Player           const * player,
		 Point                               viewpoint,
		 uint32_t                            flags);

	void drawanim
		(Point                     dst,
		 uint32_t                  animation,
		 uint32_t                  time,
		 Widelands::Player const * = 0);

	void drawanimrect
		(Point                     dst,
		 uint32_t                  animation,
		 uint32_t                  time,
		 Widelands::Player const *,
		 Rect                      srcrc);

	void reset();

protected:
	bool clip(Rect & r) const throw ();

	void doblit(Point dst, Surface * const src, Rect srcrc);

	///The target surface
	Surface * m_surface;
	///The current clip rectangle
	Rect m_rect;
	///Drawing offset
	Point m_offset;

};

#endif
