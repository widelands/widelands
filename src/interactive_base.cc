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

#include "cmd_queue.h"
#include "constants.h"
#include "error.h"
#include "font_handler.h"
#include "game.h"
#include "interactive_base.h"
#include "IntPlayer.h"
#include "map.h"
#include "mapview.h"
#include "minimap.h"
#include "options.h"
#include "overlay_manager.h"
#include "player.h"
#include "system.h"

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

   m_fsd.fieldsel_freeze = false;
   m_egbase=g;
	m_display_flags = dfDebug;

	m_lastframe = Sys_GetTime();
	m_frametime = 0;
	m_avg_usframetime = 0;

   m_mapview=0;
   m_mm=0;

   m_fsd.fieldsel_pos.x=0;
   m_fsd.fieldsel_pos.y=0;
   m_fsd.fieldsel_jobid=0;
   m_fsd.fieldsel_pic=g_gr->get_picture(PicMod_Game, "pics/fsel.png", true);
   m_fsd.fieldsel_radius=0;
   
   m_road_buildhelp_overlay_jobid=0;
   m_jobid=0;
	m_buildroad = false;
   m_road_build_player=0;
}

/*
===============
Interactive_Base::~Interactive_Base

cleanups
===============
*/
Interactive_Base::~Interactive_Base(void)
{
	if (m_buildroad)
		abort_build_road();
}

/*
===============
Change the field selection. Does not honour the freeze!
===============
*/
void Interactive_Base::set_fieldsel_pos(Coords c)
{
   // Remove old fieldsel pointer
   if(m_fsd.fieldsel_jobid)
      get_map()->get_overlay_manager()->remove_overlay(m_fsd.fieldsel_jobid);
	m_fsd.fieldsel_jobid= get_map()->get_overlay_manager()->get_a_job_id();
   m_fsd.fieldsel_pos=c;
   // register fieldsel overlay position
   MapRegion mr(get_map(), c, m_fsd.fieldsel_radius);
   FCoords fc;
   while(mr.next(&fc)) {
      get_map()->get_overlay_manager()->register_overlay(fc, m_fsd.fieldsel_pic, 7, Coords(-1,-1), m_fsd.fieldsel_jobid);
   }
}

/*
 * Set the current fieldsel selection radius.
 */
void Interactive_Base::set_fieldsel_radius(int n) {
   m_fsd.fieldsel_radius=n;
   set_fieldsel_pos(get_fieldsel_pos()); // redraw
}

/*
 *  [ protected functions ]
 *
 * Set/Unset fieldsel picture
 */
void Interactive_Base::set_fieldsel_picture(const char* file) {
   m_fsd.fieldsel_pic=g_gr->get_picture(PicMod_Game, file, true);
   set_fieldsel_pos(get_fieldsel_pos()); // redraw
}
void Interactive_Base::unset_fieldsel_picture(void) {
   set_fieldsel_picture("pics/fsel.png");
}


/*
===============
Interactive_Base::set_fieldsel_freeze

Field selection is frozen while the field action dialog is visible
===============
*/
void Interactive_Base::set_fieldsel_freeze(bool yes)
{
	m_fsd.fieldsel_freeze = yes;
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
   if(get_display_flag(dfDebug) || !get_egbase()->is_game()) {
      // Show fsel coordinates
      char buf[100];
      Coords fsel = get_fieldsel_pos();

      sprintf(buf, "%3i %3i", fsel.x, fsel.y);
      g_fh->draw_string(dst, UI_FONT_BIG, UI_FONT_BIG_CLR,  5, 5, buf);
   }

   if (get_display_flag(dfDebug))
   {
      // Show FPS
      char buf[100];
		sprintf(buf, "%5.1f fps (avg: %5.1f fps)",
				1000.0 / m_frametime, 1000.0 / (m_avg_usframetime / 1000));
		g_fh->draw_string(dst, UI_FONT_BIG, UI_FONT_BIG_CLR,  75, 5, buf);
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

/*
===============
Interactive_Base::start_build_road

Begin building a road
===============
*/
void Interactive_Base::start_build_road(Coords start, int player)
{
	// create an empty path
	m_buildroad = new CoordPath(m_egbase->get_map(), start);

   m_road_build_player=player;

   // If we are a game, we obviously build for the Interactive Player
   assert(!m_egbase->is_game() || (player==static_cast<Interactive_Player*>(this)->get_player_number()));

	roadb_add_overlay();
}


/*
===============
Interactive_Base::abort_build_road

Stop building the road
===============
*/
void Interactive_Base::abort_build_road()
{
	assert(m_buildroad);

	roadb_remove_overlay();

   m_road_build_player=0;

	delete m_buildroad;
	m_buildroad = 0;
}


/*
===============
Interactive_Base::finish_build_road

Finally build the road
===============
*/
void Interactive_Base::finish_build_road()
{
	assert(m_buildroad);

	roadb_remove_overlay();

	if (m_buildroad->get_nsteps()) {
		// awkward... path changes ownership
		Path *path = new Path(*m_buildroad);
      if(m_egbase->is_game()) {
         // Build the path as requested 
         static_cast<Game*>(m_egbase)->send_player_command(m_road_build_player, CMD_BUILD_ROAD, (int)path, 0, 0);
      } else {
         get_egbase()->get_player(m_road_build_player)->build_road(path);
         delete path;
      }
	}

	delete m_buildroad;
	m_buildroad = 0;
}


/*
===============
Interactive_Base::append_build_road

If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool Interactive_Base::append_build_road(Coords field)
{
	assert(m_buildroad);

	int idx = m_buildroad->get_index(field);

	if (idx >= 0) {
		roadb_remove_overlay();
		m_buildroad->truncate(idx);
		roadb_add_overlay();

		return true;
	}

	// Find a path to the clicked-on field
	Map *map = m_egbase->get_map();
	Path path;
	CheckStepRoad cstep(m_egbase->get_player(m_road_build_player), MOVECAPS_WALK, &m_buildroad->get_coords());

	if (map->findpath(m_buildroad->get_end(), field, 0, &path, &cstep, Map::fpBidiCost) < 0)
		return false; // couldn't find a path

	roadb_remove_overlay();
	m_buildroad->append(path);
	roadb_add_overlay();

	return true;
}

/*
===============
Interactive_Base::get_build_road_start

Return the current road-building startpoint
===============
*/
const Coords &Interactive_Base::get_build_road_start()
{
	assert(m_buildroad);

	return m_buildroad->get_start();
}

/*
===============
Interactive_Base::get_build_road_end

Return the current road-building endpoint
===============
*/
const Coords &Interactive_Base::get_build_road_end()
{
	assert(m_buildroad);

	return m_buildroad->get_end();
}

/*
===============
Interactive_Base::get_build_road_end_dir

Return the direction of the last step
===============
*/
int Interactive_Base::get_build_road_end_dir()
{
	assert(m_buildroad);

	if (!m_buildroad->get_nsteps())
		return 0;

	return m_buildroad->get_step(m_buildroad->get_nsteps()-1);
}

/*
===============
Interactive_Base::roadb_add_overlay

Add road building data to the road overlay
===============
*/
void Interactive_Base::roadb_add_overlay()
{
	assert(m_buildroad);

	//log("Add overlay\n");

	Map* map = m_egbase->get_map();

	// preview of the road
   assert(!m_jobid);
   m_jobid=get_map()->get_overlay_manager()->get_a_job_id();
	for(int idx = 0; idx < m_buildroad->get_nsteps(); idx++)	{
		uchar dir = m_buildroad->get_step(idx);
		Coords c = m_buildroad->get_coords()[idx];

		if (dir < Map_Object::WALK_E || dir > Map_Object::WALK_SW) {
			map->get_neighbour(c, dir, &c);
			dir = get_reverse_dir(dir);
		}

		int shift = 2*(dir - Map_Object::WALK_E);

      uchar set_to= get_map()->get_overlay_manager()->get_road_overlay(c);
      set_to|=  Road_Normal << shift;
      get_map()->get_overlay_manager()->register_road_overlay(c, set_to, m_jobid);
	}

	// build hints
	FCoords endpos = map->get_fcoords(m_buildroad->get_end());

   assert(!m_road_buildhelp_overlay_jobid);
   m_road_buildhelp_overlay_jobid= get_map()->get_overlay_manager()->get_a_job_id();
	for(int dir = 1; dir <= 6; dir++) {
		FCoords neighb;
		int caps;

		map->get_neighbour(endpos, dir, &neighb);
		caps = m_egbase->get_player(m_road_build_player)->get_buildcaps(neighb);

		if (!(caps & MOVECAPS_WALK))
			continue; // need to be able to walk there

		BaseImmovable *imm = map->get_immovable(neighb); // can't build on robusts
		if (imm && imm->get_size() >= BaseImmovable::SMALL) {
			if (!(imm->get_type() == Map_Object::FLAG ||
					(imm->get_type() == Map_Object::ROAD && caps & BUILDCAPS_FLAG)))
				continue;
		}

		if (m_buildroad->get_index(neighb) >= 0)
			continue; // the road can't cross itself

		int slope = abs(endpos.field->get_height() - neighb.field->get_height());
		int icon;

		if (slope < 2)
			icon = 1;
		else if (slope < 4)
			icon = 2;
		else
			icon = 3;

      std::string name="";
      switch(icon) {
         case 1: name="pics/roadb_green.png"; break;
         case 2: name="pics/roadb_yellow.png"; break;
         case 3: name="pics/roadb_red.png"; break;
      };

      assert(name!="");

      get_map()->get_overlay_manager()->register_overlay(neighb,  g_gr->get_picture(PicMod_Game, name.c_str(), true),7, Coords(-1,-1), m_road_buildhelp_overlay_jobid);
	}
}

/*
===============
Interactive_Base::roadb_remove_overlay

Remove road building data from road overlay
===============
*/
void Interactive_Base::roadb_remove_overlay()
{
	assert(m_buildroad);

	//log("Remove overlay\n");

   // preview of the road
   if(m_jobid)
       get_map()->get_overlay_manager()->remove_road_overlay(m_jobid);
   m_jobid=0;

	// build hints
   if(m_road_buildhelp_overlay_jobid)
      get_map()->get_overlay_manager()->remove_overlay(m_road_buildhelp_overlay_jobid);
   m_road_buildhelp_overlay_jobid=0;
}
