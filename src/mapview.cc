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
//	bmp->fill_rect(0, 0, get_w(), get_h(), pack_rgb(255, 0, 0)); // TEST

	// Prepare an improved bitmap which we can draw into without using ofsx/ofsy
	int effvpx = vpx;
	int effvpy = vpy;
	Bitmap dst;

	if (!dst.make_partof(bmp, ofsx, ofsy, get_w(), get_h(), &ofsx, &ofsy))
		return;
	if (ofsx < 0)
		effvpx -= ofsx;
	if (ofsy < 0)
		effvpy -= ofsy;

	draw_ground(&dst, effvpx, effvpy);

	// Draw the fsel
	int x;
	int y;

	Field *f = map->get_field(fselx, fsely);
	map->get_pix(fselx, fsely, f, &x, &y);
	x -= effvpx;
	y -= effvpy;

	if (x < -(int)fsel.get_w())
		x += map->get_w() * FIELD_WIDTH;
	if (y < -(int)fsel.get_h())
		y += map->get_h() * (FIELD_HEIGHT>>1);

	x -= fsel.get_w()>>1;
	y -= fsel.get_h()>>1;
	copy_pic(&dst, &fsel, x, y, 0, 0, fsel.get_w(), fsel.get_h());

	// debug: show fsel coordinates
	char buf[16];
	sprintf(buf, "%i %i", fselx, fsely);
	Pic *p = g_fh.get_string(buf, 0);
	copy_pic(bmp, p, ofsx+5, ofsy+5, 0, 0, p->get_w(), p->get_h());
	delete p;
}

/** Map_View::draw_ground(Bitmap *dst, int effvpx, int effvpy)
 *
 * Draw the ground only (i.e. the shaded triangles)
 *
 * Args: dst	bitmap to draw in
 *       effvpx	viewpoint coordinates
 */
void Map_View::draw_ground(Bitmap *dst, int effvpx, int effvpy)
{
	int minfx, minfy;
	int maxfx, maxfy;

	minfx = (effvpx + (FIELD_WIDTH>>1)) / FIELD_WIDTH - 1; // hack to prevent negative numbers
	minfy = effvpy / (FIELD_HEIGHT>>1);
	maxfx = (effvpx + (FIELD_WIDTH>>1) + dst->get_w()) / FIELD_WIDTH;
	maxfy = (effvpy + dst->get_h()) / (FIELD_HEIGHT>>1);
	maxfy += 10; // necessary because of heights

//	printf("%i %i -> %i %i\n", minfx, minfy, maxfx, maxfy);

	int dx = maxfx - minfx + 1;
	int dy = maxfy - minfy + 1;
	int linear_fy = minfy;

	while(dy--) {
		int linear_fx = minfx;
		int fx, fy;
		int lx, ly;
		Field *f, *fl;
		int posx, posy;
		int blposx, blposy;

		// Use linear (non-wrapped) coordinates to calculate on-screen pos
		map->get_basepix(linear_fx, linear_fy, &posx, &posy);
		posx -= effvpx;
		posy -= effvpy;

		// Get linear bottom-left coordinate
		ly = linear_fy+1;
		lx = linear_fx - (ly&1);

		map->get_basepix(lx, ly, &blposx, &blposy);
		blposx -= effvpx;
		blposy -= effvpy;

		// Calculate safe (bounded) field coordinates and get field pointers
		fx = linear_fx;
		fy = linear_fy;
		map->normalize_coords(&fx, &fy);
		map->normalize_coords(&lx, &ly);

		f = map->get_field(fx, fy);
		fl = map->get_field(lx, ly);

		int count = dx;
		while(count--) {
			Field *rf, *rfl;
			int rposx, rblposx;

			map->get_rn(fx, fy, f, &fx, &fy, &rf);
			rposx = posx + FIELD_WIDTH;

			map->get_rn(lx, ly, fl, &lx, &ly, &rfl);
			rblposx = blposx + FIELD_WIDTH;

			draw_field(dst, f, rf, fl, rfl, posx, rposx, posy, blposx, rblposx, blposy);

			f = rf;
			fl = rfl;
			posx = rposx;
			blposx = rblposx;
		}

		linear_fy++;
	}
}

/* void Map_View::draw_field(Bitmap *dst, Field *f, Field *rf, Field *fl, Field *rfl,
 *                           int posx, int rposx, int posy, int blposx, int rblposx, int blposy)
 *
 * Draw the two triangles associated with one field.
 */
void Map_View::draw_field(Bitmap *dst, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
           const int posx, const int rposx, const int posy, const int blposx, const int rblposx, const int blposy)
{
	// points are ordered: right, left, bottom-right, bottom-left
	// note that as long as render_triangle messes with the arrays, we need to
	// copy them to a safe place first
	Point p[4];
	int b[4];

	p[0] = Point(rposx, posy - rf->get_height()*HEIGHT_FACTOR);
	p[1] = Point(posx, posy - f->get_height()*HEIGHT_FACTOR);
	p[2] = Point(rblposx, blposy - rfl->get_height()*HEIGHT_FACTOR);
	p[3] = Point(blposx, blposy - fl->get_height()*HEIGHT_FACTOR);

	if ((p[2].y < 0 && p[3].y < 0) ||
	    (p[0].y >= (int)dst->get_h() && p[1].y >= (int)dst->get_h()))
		return;

	b[0] = rf->get_brightness();
	b[1] = f->get_brightness();
	b[2] = rfl->get_brightness();
	b[3] = fl->get_brightness();

/*
	b[0] += 20; // debug override for shading (make field borders visible)
	b[3] -= 20;
*/

	// Render right triangle
	Point ptmp[3];
	int btmp[3];

	memcpy(ptmp, p, sizeof(Point)*3);
	memcpy(btmp, b, sizeof(int)*3);
	render_triangle(dst, ptmp, btmp, f->get_texr());

	// Render bottom triangle
	render_triangle(dst, p+1, b+1, f->get_texd());
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
	while(vpx>(int)(FIELD_WIDTH*map->get_w()))			vpx-=(FIELD_WIDTH*map->get_w());
	while(vpy>(int)((FIELD_HEIGHT*map->get_h())>>1))	vpy-=(FIELD_HEIGHT*map->get_h())>>1;
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
bool Map_View::handle_mouseclick(uint btn, bool down, int x, int y)
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

	return true;
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
	my += vpy;
	fsely = (my + (FIELD_HEIGHT>>2)) / (FIELD_HEIGHT>>1) - 1;

	mx += vpx;
	fselx = mx;
	if (fsely & 1)
		fselx -= FIELD_WIDTH>>1;
	fselx = (fselx + (FIELD_WIDTH>>1)) / FIELD_WIDTH;

	map->normalize_coords(&fselx, &fsely);

	// Now, fselx and fsely point to where we'd be if the field's height
	// was 0.
	// We now recursively move towards the correct field. Because height cannot
	// be negative, we only need to consider the bottom-left or bottom-right neighbour
	int mapheight = map->get_h()*(FIELD_HEIGHT>>1);
	int mapwidth = map->get_w()*FIELD_WIDTH;
	Field *f;
	int fscrx, fscry;

	f = map->get_field(fselx, fsely);
	map->get_pix(fselx, fsely, f, &fscrx, &fscry);

	for(;;) {
		Field *bln, *brn;
		int blx, bly, brx, bry;
		int blscrx, blscry, brscrx, brscry;
		bool movebln, movebrn;
		int fd, blnd, brnd;
		int d2;

		map->get_bln(fselx, fsely, f, &blx, &bly, &bln);
		map->get_brn(fselx, fsely, f, &brx, &bry, &brn);

		map->get_pix(blx, bly, bln, &blscrx, &blscry);
		map->get_pix(brx, bry, brn, &brscrx, &brscry);

		// determine which field the mouse is closer to on the y-axis
		// bit messy because it has to be aware of map wrap-arounds
		fd = my - fscry;
		d2 = my - mapheight - fscry;
		if (abs(d2) < abs(fd))
			fd = d2;

		blnd = blscry - my;
		d2 = blscry - (my - mapheight);
		if (abs(d2) < abs(blnd))
			blnd = d2;
		movebln = blnd < fd;

		brnd = brscry - my;
		d2 = brscry - (my - mapheight);
		if (abs(d2) < abs(brnd))
			brnd = d2;
		movebrn = brnd < fd;

		if (!movebln && !movebrn)
			break;

		// determine which field is closer on the x-axis
		blnd = mx - blscrx;
		d2 = mx - mapwidth - blscrx;
		if (abs(d2) < abs(blnd))
			blnd = d2;

		brnd = brscrx - mx;
		d2 = brscrx - (mx - mapwidth);
		if (abs(d2) < abs(brnd))
			brnd = d2;

		if (brnd < blnd) {
			if (movebrn) {
				f = brn;
				fselx = brx;
				fsely = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else if (movebln) {
				f = bln;
				fselx = blx;
				fsely = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else
				break;
		} else {
			if (movebln)  {
				f = bln;
				fselx = blx;
				fsely = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else if (movebrn) {
				f = brn;
				fselx = brx;
				fsely = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else
				break;
		}
	}
}
