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

#include "widelands.h"
#include "graphic.h"
#include "map.h"
#include "field.h"
#include "mapview.h"
#include "game.h"
#include "player.h"
#include "IntPlayer.h"


/*
===============
Map_View::Map_View

Initialize
===============
*/
Map_View::Map_View(Panel *parent, int x, int y, uint w, uint h, Interactive_Base *player)
	: Panel(parent, x, y, w, h)
{
	m_intbase = player;
	
	vpx = vpy = 0;
	dragging = false;
}


/*
===============
Map_View::~Map_View

Cleanups
===============
*/
Map_View::~Map_View(void)
{
}


/*
===============
Map_View::warp_mouse_to_field

Moves the mouse cursor so that it is directly above the given field
===============
*/
void Map_View::warp_mouse_to_field(Coords c)
{
	int x, y;

	m_intbase->get_map()->get_pix(c, &x, &y);
	x -= vpx;
	y -= vpy;
	
	if (x >= 0 && x < get_w() && y >= 0 && y < get_h())
		set_mouse_pos(x, y);
}


/*
===============
Map_View::draw
 
This is the guts!! this function draws the whole
map the user can see. we spend a lot of time
in this function
===============
*/
void Map_View::draw(RenderTarget* dst)
{
   // TEMP DEBUG TODO fps counter
   static long start_fps_counter=Sys_GetTime();
   static long cur_fps_counter=start_fps_counter;
   static long framecount=0;
	static float fps = 0;
   static float fps_average=0;
   static int fps_av_count=0;
	// TEMP
   
	dst->rendermap(m_intbase->get_maprenderinfo(), Point(vpx, vpy));

	// debug: show fsel coordinates
	Coords fsel = m_intbase->get_fieldsel();
	char buf[100];
	sprintf(buf, "%3i %3i", fsel.x, fsel.y);
	g_font->draw_string(dst, 5, 5, buf);

   // debug show fps
   ++framecount;
   cur_fps_counter=Sys_GetTime();
   if(cur_fps_counter-start_fps_counter > 1000) {
      fps = (((float)framecount*1000)/(float)(cur_fps_counter-start_fps_counter));
      fps_av_count++;
      fps_average+=fps;
         
      // one second has passed
      framecount=0;
      start_fps_counter=cur_fps_counter;
   }
   sprintf(buf, "%4f fps (av: %4f fps)", fps, fps_average/(float)fps_av_count );
	g_font->draw_string(dst, 75, 5, buf);
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
	while(vpx>(int)(MULTIPLY_WITH_FIELD_WIDTH(m_intbase->get_map()->get_width())))			vpx-=(MULTIPLY_WITH_FIELD_WIDTH(m_intbase->get_map()->get_width()));
	while(vpy>(int)(MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_intbase->get_map()->get_height())))	vpy-=MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_intbase->get_map()->get_height());
	while(vpx< 0)  vpx+=MULTIPLY_WITH_FIELD_WIDTH(m_intbase->get_map()->get_width());
	while(vpy< 0)  vpy+=MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_intbase->get_map()->get_height());

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
	if (btn == MOUSE_LEFT)
	{
		if (down) {
         track_fsel(x, y);

			fieldclicked.call();
		}
	}
	else if (btn == MOUSE_RIGHT)
	{
		if (down) {
			dragging = true;
			grab_mouse(true);
			Sys_MouseLock(true);
		} else if (dragging) {
			Sys_MouseLock(false);
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
   if (!(btns & (1<<MOUSE_RIGHT)))
		dragging = false;
   
	if (dragging)
	{
		set_rel_viewpoint(xdiff, ydiff);
	}

	if (!m_intbase->get_fieldsel_freeze())
		track_fsel(x, y);

	g_gr->update_fullscreen();
}

/*
===============
Map_View::track_fsel(int mx, int my)

Move the fsel to the given mouse position.
Does not honour fieldsel freeze.
===============
*/
void Map_View::track_fsel(int mx, int my)
{
	Coords fsel;
	
	// First of all, get a preliminary field coordinate based on the basic
	// grid (not taking heights into account)
	my += vpy;
	fsel.y = (my + (FIELD_HEIGHT>>2)) / (FIELD_HEIGHT>>1) - 1;

	mx += vpx;
	fsel.x = mx;
	if (fsel.y & 1)
		fsel.x -= FIELD_WIDTH>>1;
	fsel.x = (fsel.x + (FIELD_WIDTH>>1)) / FIELD_WIDTH;

	m_intbase->get_map()->normalize_coords(&fsel.x, &fsel.y);

	// Now, fsel point to where we'd be if the field's height was 0.
	// We now recursively move towards the correct field. Because height cannot
	// be negative, we only need to consider the bottom-left or bottom-right neighbour
	int mapheight = MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_intbase->get_map()->get_height());
	int mapwidth = MULTIPLY_WITH_FIELD_WIDTH(m_intbase->get_map()->get_width());
	Field *f;
	int fscrx, fscry;

	f = m_intbase->get_map()->get_field(fsel);
	m_intbase->get_map()->get_pix(fsel.x, fsel.y, f, &fscrx, &fscry);

	for(;;) {
		Field *bln, *brn;
		int blx, bly, brx, bry;
		int blscrx, blscry, brscrx, brscry;
		bool movebln, movebrn;
		int fd, blnd, brnd;
		int d2;

		m_intbase->get_map()->get_bln(fsel.x, fsel.y, f, &blx, &bly, &bln);
		m_intbase->get_map()->get_brn(fsel.x, fsel.y, f, &brx, &bry, &brn);

		m_intbase->get_map()->get_pix(blx, bly, bln, &blscrx, &blscry);
		m_intbase->get_map()->get_pix(brx, bry, brn, &brscrx, &brscry);

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
				fsel.x = brx;
				fsel.y = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else if (movebln) {
				f = bln;
				fsel.x = blx;
				fsel.y = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else
				break;
		} else {
			if (movebln)  {
				f = bln;
				fsel.x = blx;
				fsel.y = bly;
				fscrx = blscrx;
				fscry = blscry;
			} else if (movebrn) {
				f = brn;
				fsel.x = brx;
				fsel.y = bry;
				fscrx = brscrx;
				fscry = brscry;
			} else
				break;
		}
	}
	
	// Apply the new fieldsel
	m_intbase->set_fieldsel(fsel);
}
