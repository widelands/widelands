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

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "geometry.h"
#include "rgbcolor.h"
#include "types.h"

struct MapRenderInfo;

/*
class RenderTarget

This abstract class represents anything that can be rendered to.

It supports windows, which are composed of a clip rectangle and a drawing
offset:
The drawing offset will be added to all coordinates that are passed to drawing
routines. Therefore, the offset is usually negative. Then the coordinates are
interpreted as relative to the clip rectangle and the primitives are clipped
accordingly.
enter_window() can be used to enter a sub-window of the current window. When
you're finished drawing in the window, restore the old window by calling
set_window() with the values stored in previous and prevofs.
Note: If the sub-window would be empty/invisible, enter_window() returns false
and doesn't change the window state at all.
*/
class RenderTarget {
public:
	virtual void get_window(Rect* rc, Point* ofs) const = 0;
	virtual void set_window(const Rect& rc, const Point& ofs) = 0;
	virtual bool enter_window(const Rect& rc, Rect* previous, Point* prevofs) = 0;

	virtual int get_w() const = 0;
	virtual int get_h() const = 0;

	virtual void draw_rect(int x, int y, int w, int h, RGBColor clr) = 0;
	virtual void fill_rect(int x, int y, int w, int h, RGBColor clr) = 0;
	virtual void brighten_rect(int x, int y, int w, int h, int factor) = 0;
	virtual void clear() = 0;

	virtual void blit(int dstx, int dsty, uint picture) = 0;
	virtual void blitrect(int dstx, int dsty, uint picture,
	                      int srcx, int srcy, int w, int h) = 0;
	virtual void tile(int x, int y, int w, int h, uint picture, int ofsx, int ofsy) = 0;

	virtual void rendermap(const MapRenderInfo* mri, Point viewofs) = 0;
	virtual void renderminimap(const MapRenderInfo* mri, Coords viewpoint, uint flags) = 0;

	virtual void drawanim(int dstx, int dsty, uint animation, uint time, const RGBColor* plrclrs) = 0;
	virtual void drawanimrect(int dstx, int dsty, uint animation, uint time,
				const RGBColor* plrclrs, int srcx, int srcy, int w, int h) = 0;
};

#endif /* RENDERTARGET_H */
