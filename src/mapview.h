/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#ifndef __S__MAPVIEW_H
#define __S__MAPVIEW_H

#include "map.h"
#include "graphic.h"
#include "ui.h"

/* class Map_View
 *
 * this implements a view of a map. it's used
 * to render a valid map on the screen
 *
 * Depends: class Map
 * 			g_gr
 */
class Map_View : public Panel {
public:
	Map_View(Panel *parent, int x, int y, uint w, uint h, Game *g);
	~Map_View();

	UISignal2<int,int> warpview; // x/y in screen coordinates
	UISignal2<int,int> fieldclicked;

	// Function to set the viewpoint
	void set_viewpoint(int x, int y);
	void set_rel_viewpoint(int x, int y) { set_viewpoint(vpx+x,  vpy+y); }

	inline int get_vpx() const { return vpx; }
	inline int get_vpy() const { return vpy; }

	// Drawing
	void draw(Bitmap *bmp, int ofsx, int ofsy);
	void draw_ground(Bitmap *bmp, int effvpx, int effvpy);

	// Event handling
	bool handle_mouseclick(uint btn, bool down, int x, int y);
	void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);

	void track_fsel(int mx, int my);

private:
   static AutoPic fsel;
   static AutoPic small_building;
   static AutoPic medium_building;
   static AutoPic big_building;
   static AutoPic mine_building;
   static AutoPic setable_flag;

	Game* m_game;
	Map* map;
	int vpx, vpy;
	bool dragging;
	int fselx, fsely; // field the mouse is over

	void draw_field(Bitmap *dst, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
			const int posx, const int rposx, const int posy, const int blposx, const int rblposx, const int blposy);
};


#endif /* __S__MAPVIEW_H */
