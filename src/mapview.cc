/*
 * Copyright (C) 2002 by The Widelands Development Team
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

AutoPic Map_View::fsel("fsel.bmp", 0, 0, 255);

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

	fselx = fsely = 0;
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

	// Draw the fsel
	f = map->get_field(fselx, fsely);
	int x = f->get_xpix() - vpx;
	int y = f->get_ypix() - vpy;

	if (x < -(int)fsel.get_w())
		x += map->get_w() * FIELD_WIDTH;
	if (y < -(int)fsel.get_h())
		y += map->get_h() * (FIELD_HEIGHT>>1);

	x -= fsel.get_w()>>1;
	y -= fsel.get_h()>>1;
	copy_pic(bmp, &fsel, x+ofsx, y+ofsx, 0, 0, fsel.get_w(), fsel.get_h());
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
	int b[3];
	b[0] = l->get_brightness();
	b[1] = r->get_brightness();
	b[2] = m->get_brightness();
	render_triangle(dst, p, b, pic);
}

/** Map_View::set_viewpoint(int x, int y)
 *
 * Set the viewpoint to the given screen coordinates
 */
void Map_View::set_viewpoint(int x, int y)
{
	if (vpx == x && vpy == y)
		return;

	vpx=x; vpy=y;
	while(vpx>FIELD_WIDTH*map->get_w())			vpx-=(FIELD_WIDTH*map->get_w());
	while(vpy>(FIELD_HEIGHT*map->get_h())>>1)	vpy-=(FIELD_HEIGHT*map->get_h())>>1;
	while(vpx< 0)  vpx+=(FIELD_WIDTH*map->get_w());
	while(vpy< 0)  vpy+=(FIELD_HEIGHT*map->get_h())>>1;

	warpview.call(vpx, vpy);
}

/** Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
 *
 * Mouseclicks on the map:
 * Right-click: enable/disable dragging
 * Left-click: field action window
 */
void Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn == 0)
	{
		if (down) {
			track_fsel(x, y);
			fieldclicked.call(fselx, fsely);
		}
	}
	else if (btn == 1)
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

	if (dragging)
	{
		set_rel_viewpoint(xdiff, ydiff);
		g_ip.set_mouse_pos(g_ip.get_mplx(), g_ip.get_mply());
	}

	track_fsel(x, y);

	g_gr.needs_fs_update();
}

/** Map_View::track_fsel(int mx, int my)
 *
 * Move the fsel to the given mouse position
 *
 * Args: mx	new mouse coordinates relative to panel
 *       my
 */
void Map_View::track_fsel(int mx, int my)
{
	// First of all, get a preliminary field coordinate based on the basic
	// grid (not taking heights into account)
	int fy = my + vpy;
	fsely = (fy + (FIELD_HEIGHT>>2)) / (FIELD_HEIGHT>>1) - 1;
	while(fsely >= map->get_h()) fsely -= map->get_h();
	while(fsely < 0) fsely += map->get_h();

	int fx = mx + vpx;
	fselx = fx;
	if (fsely & 1)
		fselx -= FIELD_WIDTH>>1;
	fselx = (fselx + (FIELD_WIDTH>>1)) / FIELD_WIDTH;
	while(fselx >= map->get_w()) fselx -= map->get_w();
	while(fselx < 0) fselx += map->get_w();

	// Now, fselx and fsely point to where we'd be if the field's height
	// was 0.
	// We now recursively move towards the correct field. Because height cannot
	// be negative, we can only move to the bottom-left or bottom-right neighbour
	Field *f = map->get_field(fselx, fsely);
	int mapheight = map->get_h()*(FIELD_HEIGHT>>1);
	//printf("%i %i\n", fx, fy);

	for(;;) {
		Field *bln = f->get_bln();
		Field *brn = f->get_brn();
		bool movebln, movebrn;
		int fd, blnd, brnd;
		int d2;

		// determine which field the mouse is closer to on the y-axis
		// bit messy because it has to be aware of map wrap-arounds
		fd = fy - f->get_ypix();
		d2 = fy - mapheight - f->get_ypix();
		if (abs(d2) < abs(fd))
			fd = d2;

		blnd = bln->get_ypix() - fy;
		d2 = bln->get_ypix() - (fy - mapheight);
		if (abs(d2) < abs(blnd))
			blnd = d2;
		movebln = blnd < fd;

		brnd = brn->get_ypix() - fy;
		d2 = brn->get_ypix() - (fy - mapheight);
		if (abs(d2) < abs(brnd))
			brnd = d2;
		movebrn = brnd < fd;

		if (!movebln && !movebrn)
			break;

		//printf("  %i %i (%s %s)\n", f->get_xpix(), f->get_ypix(), movebln?"left":"", movebrn?"right":"");
		//printf("   %i <> %i\n", bln->get_xpix(), brn->get_xpix());

		// descend one field
		if (brn->get_xpix()-fx < fx-bln->get_xpix()) {
			if (movebrn)
				f = brn;
			else if (movebln)
				f = bln;
			else
				break;
		} else {
			if (movebln)
				f = bln;
			else if (movebrn)
				f = brn;
			else
				break;
		}
	}

	//printf("  %i %i\n", f->get_xpix(), f->get_ypix());

	// Store the final field coordinates
	fselx = f->get_xpos();
	fsely = f->get_ypos();
}
