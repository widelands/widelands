/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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
#include "interactive_base.h"
#include "map.h"
#include "options.h"
#include "mapview.h"
#include "minimap.h"

/*
==============================================================================

Interactive_Base IMPLEMENTATION

==============================================================================
*/

/*
===============
Interactive_Base::Interactive_Base

Initialize
===============
*/
Interactive_Base::Interactive_Base(Editor_Game_Base* g) :
  UIPanel(0, 0, 0, get_xres(), get_yres())
{
	// Switch to the new graphics system now, if necessary
   Section *s = g_options.pull_section("global");
	
	Sys_InitGraphics(Sys_GetGraphicsSystemFromString(s->get_string("gfxsys", "sw32")),
			get_xres(), get_yres(), s->get_bool("fullscreen", false));

   memset(&m_maprenderinfo, 0, sizeof(m_maprenderinfo));   

   m_fieldsel_freeze = false;
   m_egbase=g;
	m_display_flags = dfDebug;

	m_lastframe = Sys_GetTime();
	m_frametime = 0;
	m_avg_usframetime = 0;

   m_mapview=0;
   m_mm=0;

   set_fieldsel_radius(0);
   unset_fsel_picture(); // set default fsel
}

/*
===============
Interactive_Base::~Interactive_Base

cleanups
===============
*/
Interactive_Base::~Interactive_Base(void)
{
	if (m_maprenderinfo.overlay_basic)
		free(m_maprenderinfo.overlay_basic);
	if (m_maprenderinfo.overlay_roads)
		free(m_maprenderinfo.overlay_roads);
}


/*
===============
Interactive_Base::set_fieldsel

Change the field selection. Does not honour the freeze!
===============
*/
void Interactive_Base::set_fieldsel(Coords c)
{
	m_maprenderinfo.fieldsel = c;
}


/*
===============
Interactive_Base::set_fieldsel_freeze

Field selection is frozen while the field action dialog is visible
===============
*/
void Interactive_Base::set_fieldsel_freeze(bool yes)
{
	m_fieldsel_freeze = yes;
}


/*
===============
Interactive_Base::get_xres [static]
Interactive_Base::get_yres [static]

Retrieve in-game resolution from g_options.
===============
*/
int Interactive_Base::get_xres()
{
	return g_options.pull_section("global")->get_int("xres", 640);
}

int Interactive_Base::get_yres()
{
	return g_options.pull_section("global")->get_int("yres", 480);
}


/*
===============
Interactive_Base::think

Called once per frame by the UI code
===============
*/
void Interactive_Base::think()
{
	// Timing
	uint curframe = Sys_GetTime();

	m_frametime = curframe - m_lastframe;
	m_avg_usframetime = ((m_avg_usframetime * 15) + (m_frametime * 1000)) / 16;
	m_lastframe = curframe;

	// Call game logic here
   // The game advances
	m_egbase->think();

	// The entire screen needs to be redrawn (unit movement, tile animation, etc...)
	g_gr->update_fullscreen();

	// some of the UI windows need to think()
	UIPanel::think();
}


/*
===============
Interactive_Base::draw_overlay

Draw debug overlay when appropriate.
===============
*/
void Interactive_Base::draw_overlay(RenderTarget* dst)
{
	if (get_display_flag(dfDebug))
	{
		// Show fsel coordinates
		char buf[100];
		Coords fsel = get_fieldsel();

		sprintf(buf, "%3i %3i", fsel.x, fsel.y);
		g_font->draw_string(dst, 5, 5, buf);

		// Show FPS
		sprintf(buf, "%4.1f fps (avg: %4.1f fps)",
				1000.0 / m_frametime, 1000.0 / (m_avg_usframetime / 1000));
		g_font->draw_string(dst, 75, 5, buf);
	}
}


/** Interactive_Base::mainview_move(int x, int y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void Interactive_Base::mainview_move(int x, int y)
{
   if (m_minimap.window) {
      int maxx = MULTIPLY_WITH_FIELD_WIDTH(m_egbase->get_map()->get_width());
      int maxy = MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_egbase->get_map()->get_height());

      x += get_mapview()->get_w()>>1;
      if (x >= maxx) x -= maxx;
      y += get_mapview()->get_h()>>1;
      if (y >= maxy) y -= maxy;


      m_mm->get_minimapview()->set_view_pos(x, y);
   }
}


/*
===============
Interactive_Base::minimap_warp

Called whenever the player clicks on a location on the minimap.
Warps the main mapview position to the clicked location.
===============
*/
void Interactive_Base::minimap_warp(int x, int y)
{
	x -= get_mapview()->get_w()>>1;
	if (x < 0) x += MULTIPLY_WITH_FIELD_WIDTH(m_egbase->get_map()->get_width());
	y -= get_mapview()->get_h()>>1;
	if (y < 0) y += MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_egbase->get_map()->get_height());
	get_mapview()->set_viewpoint(Point(x, y));
}


/*
===============
Interactive_Base::move_view_to

Move the mainview to the given position (in field coordinates)
===============
*/
void Interactive_Base::move_view_to(int fx, int fy)
{
	int x = MULTIPLY_WITH_FIELD_WIDTH(fx);
	int y = MULTIPLY_WITH_HALF_FIELD_HEIGHT(fy);

	if (m_minimap.window)
		m_mm->get_minimapview()->set_view_pos(x, y);

	x -= get_mapview()->get_w()>>1;
	if (x < 0) x += MULTIPLY_WITH_FIELD_WIDTH(m_egbase->get_map()->get_width());
	y -= get_mapview()->get_h()>>1;
	if (y < 0) y += MULTIPLY_WITH_HALF_FIELD_HEIGHT(m_egbase->get_map()->get_height());
	get_mapview()->set_viewpoint(Point(x, y));
}


/*
===============
Interactive_Base::move_view_to_point

Center the mainview on the given position (in pixels)
===============
*/
void Interactive_Base::move_view_to_point(Point pos)
{
	if (m_minimap.window)
		m_mm->get_minimapview()->set_view_pos(pos.x, pos.y);

	get_mapview()->set_viewpoint(pos - Point(get_mapview()->get_w()/2, get_mapview()->get_h()/2));
}


/*
===============
Interactive_Base::warp_mouse_to_field

Move the mouse so that it's directly above the given field
===============
*/
void Interactive_Base::warp_mouse_to_field(Coords c)
{
	get_mapview()->warp_mouse_to_field(c);
}

/*
===========
Interactive_Base::toggle_minimap()

Open the minimap or close it if it's open
===========
*/
void Interactive_Base::toggle_minimap() {
	if (m_minimap.window) {
		delete m_minimap.window;
   }
	else {
		m_mm = new MiniMap(this, &m_minimap);
      m_mm->get_minimapview()->warpview.set(this,
            &Interactive_Base::minimap_warp);

		// make sure the viewpos marker is at the right pos to start with
		Point p = get_mapview()->get_viewpoint();

		mainview_move(p.x, p.y);
	}
}


/*
===============
Interactive_Base::get_display_flags

Return display flags (dfXXX) that modify the view of the map.
===============
*/
uint Interactive_Base::get_display_flags()
{
	return m_display_flags;
}


/*
===============
Interactive_Base::set_display_flags

Change the display flags that modify the view of the map.
===============
*/
void Interactive_Base::set_display_flags(uint flags)
{
	m_display_flags = flags;
}


/*
===============
Interactive_Base::get_display_flag
Interactive_Base::set_display_flag

Get and set one individual flag of the display flags.
===============
*/
bool Interactive_Base::get_display_flag(uint flag)
{
	return m_display_flags & flag;
}

void Interactive_Base::set_display_flag(uint flag, bool on)
{
	m_display_flags &= ~flag;

	if (on)
		m_display_flags |= flag;
}
