/*
 * Copyright (C) 2002 by Holger Rapp
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

#include "os.h"
#include "mapview.h"
#include "graphic.h"
#include "input.h"

/* class Map_View
 *
 * this implements a view of a map. it's used
 * to render a valid map on the screen
 *
 * Depends: class Map
 * 			g_gr
 */

/** Map_View::Map_View(Panel *parent, int x, int y, uint w, uint h, Map *m)
 *
 * Init
 *
 * Args: m 	map to use
 */
Map_View::Map_View(Panel *parent, int x, int y, uint w, uint h, Map *m)
	: Panel(parent, x, y, w, h)
{
	vpx = vpy = 0;
	map = m;
	dragging = false;
}

/** Map_View::~Map_View(void)
 *
 * Cleanups
 */
Map_View::~Map_View(void) {

}

/** void Map_View::draw(void)
 *
 * This is the guts!! this function draws the whole
 * map the user can see. we spend a lot of time
 * in this function
 *
 * Args: None
 * Returns: Nothing
 */
void Map_View::draw(Bitmap *bmp, int ofsx, int ofsy)
{
/*	g_gr.set_pixel(0, 0, 0);
	for (int z=0; z<640*480; z++)
		g_gr.set_npixel(0);*/

	// Prepare an improved bitmap which we can draw into without using ofsx/ofsy
	int orig_vpx = vpx;
	int orig_vpy = vpy;
	Bitmap dst;

	if (!dst.make_partof(bmp, ofsx, ofsy, get_w(), get_h(), &ofsx, &ofsy))
		return;
	if (ofsx < 0)
		vpx -= ofsx;
	if (ofsy < 0)
		vpy -= ofsy;

	// Now draw the view
	Field *f;
	static bool xtrans;
	static bool ytrans;

	f=map->get_ffield();
	if( (f->get_rn()->get_xpix()-vpx >=0) && (f->get_bln()->get_xpix()-vpx < (int)g_gr.get_xres()) )
		draw_field(&dst, f);

	for(int i=(map->get_w()*(map->get_h()-1)); --i; )
	{
		f=map->get_nfield();
		// X-check
		if(f->get_rn()->get_xpix()-vpx <0) continue;
		if(f->get_bln()->get_xpix()-vpx >= (int)g_gr.get_xres()) continue;
		// Y-check
		if(f->get_ypix()-vpy >= (int)g_gr.get_yres()) continue;
		if(f->get_bln()->get_ypix()-vpy < 0 &&
		   f->get_brn()->get_ypix()-vpy < 0) continue;
		draw_field(&dst, f);
	}

	if(!xtrans && (uint)vpx> map->get_w()*FIELD_WIDTH-g_gr.get_xres())
	{
		int ovpx=vpx;
		vpx-=map->get_w()*FIELD_WIDTH;
		xtrans=true;
		draw(&dst, 0, 0);
		xtrans=false;
		vpx=ovpx;
	}

	if(!ytrans && (uint)vpy> (((map->get_h()-1)*FIELD_HEIGHT)>>1)-g_gr.get_yres())
	{
		int ovpy=vpy;
		vpy-=(((map->get_h()+1)*FIELD_HEIGHT)>>1);
		ytrans=true;
		draw(&dst, 0, 0);
		ytrans=false;
		vpy=ovpy;
	}

	vpx = orig_vpx;
	vpy = orig_vpy;
}

void Map_View::draw_field(Bitmap *dst, Field* f)
{
	// for plain terrain, this param order will avoid swapping in
	// Graphic::render_triangle
	draw_polygon(dst, f, f->get_bln(), f->get_brn(), f->get_texd());
	draw_polygon(dst, f, f->get_rn(), f->get_brn(), f->get_texr());
}

inline void Map_View::draw_polygon(Bitmap *dst, Field* l, Field* r, Field* m, Pic* pic)
{
	Point p[3];
	p[0] = Point(l->get_xpix()-vpx, l->get_ypix()-vpy);
	p[1] = Point(r->get_xpix()-vpx, r->get_ypix()-vpy);
	p[2] = Point(m->get_xpix()-vpx, m->get_ypix()-vpy);
	Vector n[3];
	n[0] = l->get_normal();
	n[1] = r->get_normal();
	n[2] = m->get_normal();
	render_triangle(dst, p, n, pic);
}

void Map_View::set_viewpoint(uint x,  uint y)
{
	vpx=x; vpy=y;
	while(vpx>FIELD_WIDTH*map->get_w())			vpx-=(FIELD_WIDTH*map->get_w());
	while(vpy>(FIELD_HEIGHT*map->get_h())>>1)	vpy-=(FIELD_HEIGHT*map->get_h())>>1;
	while(vpx< 0)  vpx+=(FIELD_WIDTH*map->get_w());
	while(vpy< 0)  vpy+=(FIELD_HEIGHT*map->get_h())>>1;
}

/** Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Mouseclicks on the map:
 * Right-click: enable/disable dragging
 * Left-click: field action window
 */
void Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
{
	// right-click
	if (btn == 1)
	{
		if (down) {
			dragging = true;
			grab_mouse(true);
		} else if (dragging) {
			grab_mouse(false);
			dragging = false;
		}
	}
}

/** Map_View::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
 *
 * Scroll the view according to mouse movement.
 */
void Map_View::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	if (!(btns & 2))
		dragging = false;
	if (!dragging)
		return;

	set_rel_viewpoint(xdiff, ydiff);
	g_gr.needs_fs_update();
	g_ip.set_mouse_pos(g_ip.get_mplx(), g_ip.get_mply());
}

