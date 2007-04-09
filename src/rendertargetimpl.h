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

#ifndef RENDERTARGETIMPL_H
#define RENDERTARGETIMPL_H

#include "editor_game_base.h"
#include "rendertarget.h"
#include "surface.h"

/**
 * Wrapper around a Bitmap that can be rendered into
 *
 * The 16-bit software renderer implementation of the RenderTarget interface
*/

class RenderTargetImpl : public RenderTarget
{
	Surface* m_ground_surface; // only needed, when this is a mapview
	Surface * m_surface;        //  the target surface
	Rect      m_rect;           //  the current clip rectangle
	Point     m_offset;         //  drawing offset

public:
	RenderTargetImpl(Surface* bmp);
	virtual ~RenderTargetImpl();

	void reset();

	virtual void get_window(Rect* rc, Point* ofs) const;
	virtual void set_window(const Rect& rc, const Point& ofs);
	virtual bool enter_window(const Rect& rc, Rect* previous, Point* prevofs);

	virtual int get_w() const;
	virtual int get_h() const;

	virtual void draw_line(int x1, int y1, int x2, int y2, RGBColor color);
	virtual void draw_rect(const Rect, const RGBColor);
	virtual void fill_rect(const Rect, const RGBColor);
	virtual void brighten_rect(const Rect, const int factor);
	virtual void clear();

	virtual void blit(const Point, uint picture);
	virtual void blitrect(const Point dst, uint picture, const Rect src);
	virtual void tile(Rect r, uint picture, Point ofs);

	static Sint8 node_brightness
	(const Editor_Game_Base::Time gametime,
	 Editor_Game_Base::Time last_seen,
	 const Vision vision,
	 Sint8 result);

	virtual void rendermap
	(const Editor_Game_Base &,
	 const Player * const,
	 Point viewofs,
	 const bool draw_all);

	/**
	 * Renders a minimap into the current window. The field at viewpoint will be
	 * in the top-left corner of the window. Flags specifies what information to
	 * display (see Minimap_XXX enums).
	 *
	 * Calculate the field at the top-left corner of the clipping rect
	 * The entire clipping rect will be used for drawing.
	 */
	virtual void renderminimap
	(const Editor_Game_Base  & egbase,
	 const Player * const     player,
	 const Point               viewpoint,
	 const uint                flags)
	{
		m_surface->draw_minimap
		(egbase, player, m_rect, viewpoint - m_offset, flags);
	}

	virtual void drawanim
	(Point dst,
	 const uint animation,
	 const uint time,
	 const Player * const plrclrs = 0);
	virtual void drawanimrect
	(const Point dst,
	 const uint animation,
	 const uint time,
	 const Player * const plrclrs,
	 Rect);
private:
	bool clip(Rect &) const throw ();
	void doblit(Point, Surface * const, Rect);
};

#endif
