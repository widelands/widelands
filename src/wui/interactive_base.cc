/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "interactive_base.h"

#include "checkstep.h"
#include "logic/cmd_queue.h"
#include "constants.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "font_handler.h"
#include "logic/game.h"
#include "gamecontroller.h"
#include "graphic/graphic.h"
#include "immovable.h"
#include "interactive_player.h"
#include "maptriangleregion.h"
#include "mapviewpixelconstants.h"
#include "mapviewpixelfunctions.h"
#include "minimap.h"
#include "graphic/overlay_manager.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "upcast.h"
#include "wlapplication.h"

using Widelands::Area;
using Widelands::CoordPath;
using Widelands::Coords;
using Widelands::Editor_Game_Base;
using Widelands::Game;
using Widelands::Map;
using Widelands::Map_Object;
using Widelands::TCoords;

struct InteractiveBaseInternals {
	MiniMap * mm;
	MiniMap::Registry minimap;

	InteractiveBaseInternals() : mm(0) {}
};

Interactive_Base::Interactive_Base
	(Editor_Game_Base & the_egbase, Section & global_s)
	:
	Map_View(0, 0, 0, get_xres(), get_yres(), *this),
	m_show_workarea_preview(global_s.get_bool("workareapreview", false)),
m(new InteractiveBaseInternals),
m_egbase                      (the_egbase),
#ifdef DEBUG //  not in releases
m_display_flags               (dfDebug),
#else
m_display_flags               (0),
#endif
m_lastframe                   (WLApplication::get()->get_time()),
m_frametime                   (0),
m_avg_usframetime             (0),
m_jobid                       (Overlay_Manager::Job_Id::Null()),
m_road_buildhelp_overlay_jobid(Overlay_Manager::Job_Id::Null()),
m_buildroad                   (false),
m_road_build_player           (0),
m_toolbar                     (this, 0, 0, UI::Box::Horizontal),
m_label_speed                 (this, get_w(), 0, std::string(), Align_TopRight)
{
	warpview.set(this, &Interactive_Player::mainview_move);

	set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
	set_panel_snap_distance (global_s.get_int("panel_snap_distance", 10));
	set_snap_windows_only_when_overlapping
		(global_s.get_bool("snap_windows_only_when_overlapping", false));
	set_dock_windows_to_edges
		(global_s.get_bool("dock_windows_to_edges", false));

	//  Switch to the new graphics system now, if necessary.
	WLApplication::get()->init_graphics
		(get_xres(), get_yres(),
		 global_s.get_int("depth", 16),
		 global_s.get_bool("fullscreen", false),
		 global_s.get_bool("hw_improvements", false),
		 global_s.get_bool("double_buffer", false)
#if HAS_OPENGL
		 /**/, global_s.get_bool("opengl", false)
#endif
		 /**/);

	//  Having this in the initializer list (before Sys_InitGraphics) will give
	//  funny results.
	m_sel.pic = g_gr->get_picture(PicMod_Game, "pics/fsel.png");

	m_label_speed.set_visible(false);
}


Interactive_Base::~Interactive_Base()
{
	if (m_buildroad)
		abort_build_road();
}


void Interactive_Base::set_sel_pos(Widelands::Node_and_Triangle<> const center)
{
	Map & map = egbase().map();
	Overlay_Manager & overlay_manager = map.overlay_manager();

	// Remove old sel pointer
	if (m_sel.jobid)
		overlay_manager.remove_overlay(m_sel.jobid);
	const Overlay_Manager::Job_Id jobid =
		m_sel.jobid = overlay_manager.get_a_job_id();

	m_sel.pos = center;

	//  register sel overlay position
	if (m_sel.triangles) {
		assert
			(center.triangle.t == TCoords<>::D or
			 center.triangle.t == TCoords<>::R);
		Widelands::MapTriangleRegion<> mr
			(map, Area<TCoords<> >(center.triangle, m_sel.radius));
		do
			overlay_manager.register_overlay
				(mr.location(), m_sel.pic, 7, Point::invalid(), jobid);
		while (mr.advance(map));
	} else {
		Widelands::MapRegion<> mr(map, Area<>(center.node, m_sel.radius));
		do
			overlay_manager.register_overlay
				(mr.location(), m_sel.pic, 7, Point::invalid(), jobid);
		while (mr.advance(map));
	}

}

/*
 * Set the current sel selection radius.
 */
void Interactive_Base::set_sel_radius(const uint32_t n) {
	if (n != m_sel.radius) {
		m_sel.radius = n;
		set_sel_pos(get_sel_pos()); //  redraw
	}
}

/*
 * Set/Unset sel picture
 */
void Interactive_Base::set_sel_picture(const char * const file) {
	m_sel.pic = g_gr->get_picture(PicMod_Game, file);
	set_sel_pos(get_sel_pos()); //  redraw
}
void Interactive_Base::unset_sel_picture() {
	set_sel_picture("pics/fsel.png");
}


/**
 * Retrieves the configured in-game resolution.
 *
 * \note For most purposes, you should use \ref Graphic::get_xres instead.
 */
int32_t Interactive_Base::get_xres()
{
	return g_options.pull_section("global").get_int("xres", 640);
}


/**
 * Retrieves the configured in-game resolution.
 *
 * \note For most purposes, you should use \ref Graphic::get_yres instead.
 */
int32_t Interactive_Base::get_yres()
{
	return g_options.pull_section("global").get_int("yres", 480);
}


/**
 * Called by \ref Game::postload at the end of the game loading
 * sequence.
 *
 * Default implementation does nothing.
 */
void Interactive_Base::postload() {}

static std::string speedString(uint32_t const speed)
{
	if (speed) {
		char buffer[32];
		snprintf
			(buffer, sizeof(buffer), _("%u.%ux"), speed / 1000, speed / 100 % 10);
		return buffer;
	}
	return _("PAUSE");
}

/**
 * Bring the label that display in-game speed uptodate.
 */
void Interactive_Base::update_speedlabel()
{
	if (get_display_flag(dfSpeed)) {
		upcast(Game, game, &m_egbase);
		if (game && game->gameController()) {
			uint32_t const real    = game->gameController()->realSpeed   ();
			uint32_t const desired = game->gameController()->desiredSpeed();
			if (real == desired)
				m_label_speed.set_text
					(real == 1000 ? std::string() : speedString(real));
			else {
				char buffer[128];
				snprintf
					(buffer, sizeof(buffer),
					 _("%s (%s)"),
					 speedString(real).c_str(), speedString(desired).c_str());
				m_label_speed.set_text(buffer);
			}
		} else
			m_label_speed.set_text(_("NO GAME CONTROLLER"));
		m_label_speed.set_visible(true);
	} else
		m_label_speed.set_visible(false);
}


/*
===============
Called once per frame by the UI code
===============
*/
void Interactive_Base::think()
{
	// Timing
	uint32_t curframe = WLApplication::get()->get_time();

	m_frametime = curframe - m_lastframe;
	m_avg_usframetime = ((m_avg_usframetime * 15) + (m_frametime * 1000)) / 16;
	m_lastframe = curframe;

	// If one of the arrow keys is pressed, scroll here
	const uint32_t scrollval = 10;

	if (keyboard_free()) {
		if (get_key_state(SDLK_UP))
			set_rel_viewpoint(Point(0, -scrollval));
		if (get_key_state(SDLK_DOWN))
			set_rel_viewpoint(Point(0,  scrollval));
		if (get_key_state(SDLK_LEFT))
			set_rel_viewpoint(Point(-scrollval, 0));
		if (get_key_state(SDLK_RIGHT))
			set_rel_viewpoint(Point (scrollval, 0));
	}

	egbase().think(); // Call game logic here. The game advances.

	//  Update everything so and so many milliseconds, to make sure that the
	//  whole screen is synced (another user may have done something, and the
	//  screen was not redrawn).
	if (curframe & 1023) // % 1024
		need_complete_redraw();

	//  The entire screen needs to be redrawn (unit movement, tile animation,
	//  etc...)
	g_gr->update_fullscreen();

	update_speedlabel();

	UI::Panel::think();
}


/*
===============
Draw debug overlay when appropriate.
===============
*/
void Interactive_Base::draw_overlay(RenderTarget & dst) {
	if
		(get_display_flag(dfDebug)
		 or
		 not dynamic_cast<const Game *>(&egbase()))
	{
		//  show sel coordinates
		char buf[100];

		snprintf(buf, sizeof(buf), "%3i %3i", m_sel.pos.node.x, m_sel.pos.node.y);
		g_fh->draw_string
			(dst, UI_FONT_BIG, UI_FONT_BIG_CLR, Point(5, 5), buf, Align_Left);
		assert(m_sel.pos.triangle.t < 2);
		const char * const triangle_string[] = {"down", "right"};
		snprintf
			(buf, sizeof(buf),
			 "%3i %3i %s",
			 m_sel.pos.triangle.x, m_sel.pos.triangle.y,
			 triangle_string[m_sel.pos.triangle.t]);
		g_fh->draw_string
			(dst, UI_FONT_BIG, UI_FONT_BIG_CLR, Point(5, 25), buf, Align_Left);
	}

	if (get_display_flag(dfDebug)) {
		//  show FPS
		char buffer[100];
		snprintf
			(buffer, sizeof(buffer),
			 "%5.1f fps (avg: %5.1f fps)",
			 1000.0 / m_frametime, 1000.0 / (m_avg_usframetime / 1000));
		g_fh->draw_string
			(dst, UI_FONT_BIG, UI_FONT_BIG_CLR, Point(85, 5), buffer, Align_Left);
	}
}


/** Interactive_Base::mainview_move(int32_t x, int32_t y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void Interactive_Base::mainview_move(int32_t x, int32_t y)
{
	if (m->minimap.window) {
		const Map & map = egbase().map();
		const int32_t maxx = MapviewPixelFunctions::get_map_end_screen_x(map);
		const int32_t maxy = MapviewPixelFunctions::get_map_end_screen_y(map);

		x += get_w() >> 1;
		if (x >= maxx)
			x -= maxx;
		y += get_h() >> 1;
		if (y >= maxy)
			y -= maxy;


		m->mm->set_view_pos(x, y);
	}
}


/*
===============
Called whenever the player clicks on a location on the minimap.
Warps the main mapview position to the clicked location.
===============
*/
void Interactive_Base::minimap_warp(int32_t x, int32_t y)
{
	x -= get_w() >> 1;
	y -= get_h() >> 1;
	const Map & map = egbase().map();
	if (x < 0)
		x += map.get_width () * TRIANGLE_WIDTH;
	if (y < 0)
		y += map.get_height() * TRIANGLE_HEIGHT;
	set_viewpoint(Point(x, y));
}


/*
===============
Move the mainview to the given position (in node coordinates)
===============
*/
void Interactive_Base::move_view_to(const Coords c)
{
	assert(0 <= c.x);
	assert     (c.x < egbase().map().get_width ());
	assert(0 <= c.y);
	assert     (c.y < egbase().map().get_height());

	uint32_t const x = c.x * TRIANGLE_WIDTH, y = c.y * TRIANGLE_HEIGHT;
	if (m->minimap.window)
		m->mm->set_view_pos(x, y);
	minimap_warp(x, y);
}


/*
===============
Center the mainview on the given position (in pixels)
===============
*/
void Interactive_Base::move_view_to_point(Point pos)
{
	if (m->minimap.window)
		m->mm->set_view_pos(pos.x, pos.y);

	set_viewpoint(pos - Point(get_w() / 2, get_h() / 2));
}


/*
===========
Interactive_Base::toggle_minimap()

Open the minimap or close it if it's open
===========
*/
void Interactive_Base::toggle_minimap() {
	if (m->minimap.window) {
		delete m->minimap.window;
	}
	else {
		m->mm = new MiniMap(*this, &m->minimap);
		m->mm->warpview.set(this, &Interactive_Base::minimap_warp);

		// make sure the viewpos marker is at the right pos to start with
		const Point p = get_viewpoint();

		mainview_move(p.x, p.y);
	}
}


/**
 * Hide the minimap if it is currently shown; otherwise, do nothing.
 */
void Interactive_Base::hide_minimap()
{
	if (m->minimap.window)
		delete m->minimap.window;
}


/*
===============
Return display flags (dfXXX) that modify the view of the map.
===============
*/
uint32_t Interactive_Base::get_display_flags() const
{
	return m_display_flags;
}


/*
===============
Change the display flags that modify the view of the map.
===============
*/
void Interactive_Base::set_display_flags(uint32_t flags)
{
	m_display_flags = flags;

	update_speedlabel();
}


/*
===============
Get and set one individual flag of the display flags.
===============
*/
bool Interactive_Base::get_display_flag(uint32_t const flag)
{
	return m_display_flags & flag;
}

void Interactive_Base::set_display_flag(uint32_t const flag, bool const on)
{
	m_display_flags &= ~flag;

	if (on)
		m_display_flags |= flag;

	update_speedlabel();
}

/*
===============
Begin building a road
===============
*/
void Interactive_Base::start_build_road
	(Coords _start, Widelands::Player_Number const player)
{
	// create an empty path
	assert(not m_buildroad);
	m_buildroad = new CoordPath(_start);

	m_road_build_player = player;

	//  If we are a game, we obviously build for the Interactive Player.
	assert
		(player
		 ==
		 dynamic_cast<Interactive_Player const &>(*this).player_number());

	roadb_add_overlay();
	need_complete_redraw();
}


/*
===============
Stop building the road
===============
*/
void Interactive_Base::abort_build_road()
{
	assert(m_buildroad);

	roadb_remove_overlay();
	need_complete_redraw();

	m_road_build_player = 0;

	delete m_buildroad;
	m_buildroad = 0;
}


/*
===============
Finally build the road
===============
*/
void Interactive_Base::finish_build_road()
{
	assert(m_buildroad);

	roadb_remove_overlay();
	need_complete_redraw();

	if (m_buildroad->get_nsteps()) {
		// Build the path as requested
		if (upcast(Game, game, &egbase())) {
			game->send_player_build_road
				(m_road_build_player, *new Widelands::Path(*m_buildroad));
			if (get_key_state(SDLK_LCTRL) || get_key_state(SDLK_RCTRL)) {
				// place flags
				Map const & map = game->map();
				std::vector<Coords>         const &       c_vector =
					m_buildroad->get_coords();
				std::vector<Coords>::const_iterator const first    =
					c_vector.begin() + 2;
				std::vector<Coords>::const_iterator const last     =
					c_vector.end  () - 2;

				// start to end
				if (get_key_state(SDLK_LSHIFT) || get_key_state(SDLK_RSHIFT)) {
					for
						(std::vector<Coords>::const_iterator it = first;
						 it <= last;
						 ++it)
						game->send_player_build_flag
							(m_road_build_player, map.get_fcoords(*it));
				} else { //  end to start
					for
						(std::vector<Coords>::const_iterator it = last;
						 first <= it;
						 --it)
						game->send_player_build_flag
							(m_road_build_player, map.get_fcoords(*it));
				}
			}
		}
	}

	delete m_buildroad;
	m_buildroad = 0;
}


/*
===============
If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool Interactive_Base::append_build_road(Coords const field) {
	assert(m_buildroad);

	Map & map = egbase().map();
	Widelands::Player const & player = egbase().player(m_road_build_player);

	{ //  find a path to the clicked-on node
		Widelands::Path path;
		Widelands::CheckStepRoad cstep(player, Widelands::MOVECAPS_WALK);
		if
			(map.findpath
			 	(m_buildroad->get_end(), field, 0, path, cstep, Map::fpBidiCost)
			 <
			 0)
			return false; //  could not find a path
		m_buildroad->append(map, path);
	}

	{
		//  Fix the road by finding an optimal path through the set of nodes
		//  currently used by the road. This will not claim any new nodes, so it
		//  is guaranteed to not hinder building placement.
		Widelands::Path path;
		{
			Widelands::CheckStepLimited cstep;
			{
				std::vector<Coords> const & road_cp = m_buildroad->get_coords();
				container_iterate_const(std::vector<Coords>, road_cp, i)
					cstep.add_allowed_location(*i.current);
			}
			map.findpath
				(m_buildroad->get_start(), field, 0, path, cstep, Map::fpBidiCost);
		}
		m_buildroad->truncate(0);
		m_buildroad->append(map, path);
	}

	roadb_remove_overlay();
	roadb_add_overlay();
	need_complete_redraw();

	return true;
}

/*
===============
Return the current road-building startpoint
===============
*/
Coords Interactive_Base::get_build_road_start() const throw () {
	assert(m_buildroad);

	return m_buildroad->get_start();
}

/*
===============
Return the current road-building endpoint
===============
*/
Coords Interactive_Base::get_build_road_end() const throw () {
	assert(m_buildroad);

	return m_buildroad->get_end();
}

/*
===============
Return the direction of the last step
===============
*/
Widelands::Direction Interactive_Base::get_build_road_end_dir() const throw ()
{
	assert(m_buildroad);

	if (!m_buildroad->get_nsteps())
		return 0;

	return (*m_buildroad)[m_buildroad->get_nsteps() - 1];
}

/*
===============
Add road building data to the road overlay
===============
*/
void Interactive_Base::roadb_add_overlay()
{
	assert(m_buildroad);

	//log("Add overlay\n");

	Map & map = egbase().map();
	Overlay_Manager & overlay_manager = map.overlay_manager();

	// preview of the road
	assert(not m_jobid);
	m_jobid = overlay_manager.get_a_job_id();
	const CoordPath::Step_Vector::size_type nr_steps = m_buildroad->get_nsteps();
	for (CoordPath::Step_Vector::size_type idx = 0; idx < nr_steps; ++idx) {
		Widelands::Direction dir = (*m_buildroad)[idx];
		Coords c = m_buildroad->get_coords()[idx];

		if (dir < Map_Object::WALK_E || dir > Map_Object::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}

		int32_t const shift = 2 * (dir - Map_Object::WALK_E);

		uint8_t set_to = overlay_manager.get_road_overlay(c);
		set_to |=  Widelands::Road_Normal << shift;
		overlay_manager.register_road_overlay(c, set_to, m_jobid);
	}

	// build hints
	Widelands::FCoords endpos = map.get_fcoords(m_buildroad->get_end());

	assert(not m_road_buildhelp_overlay_jobid);
	m_road_buildhelp_overlay_jobid = overlay_manager.get_a_job_id();
	for (int32_t dir = 1; dir <= 6; ++dir) {
		Widelands::FCoords neighb;
		int32_t caps;

		map.get_neighbour(endpos, dir, &neighb);
		caps = egbase().player(m_road_build_player).get_buildcaps(neighb);

		if (!(caps & Widelands::MOVECAPS_WALK))
			continue; // need to be able to walk there

		//  can't build on robusts
		Widelands::BaseImmovable * const imm = map.get_immovable(neighb);
		if (imm && imm->get_size() >= Widelands::BaseImmovable::SMALL) {
			if
				(not
				 (dynamic_cast<const Widelands::Flag *>(imm)
				  or
				  (dynamic_cast<const Widelands::Road *>(imm)
				   and
				   caps & Widelands::BUILDCAPS_FLAG)))
				continue;
		}

		if (m_buildroad->get_index(neighb) >= 0)
			continue; // the road can't cross itself

		int32_t const slope =
			abs(endpos.field->get_height() - neighb.field->get_height());
		int32_t icon;

		if (slope < 2)
			icon = 1;
		else if (slope < 4)
			icon = 2;
		else
			icon = 3;

		char const * name;
		switch (icon) {
		case 1: name = "pics/roadb_green.png";  break;
		case 2: name = "pics/roadb_yellow.png"; break;
		case 3: name = "pics/roadb_red.png";    break;
		default:
			assert(false);
		}

		egbase().map().overlay_manager().register_overlay
			(neighb,
			 g_gr->get_picture(PicMod_Game, name),
			 7,
			 Point::invalid(),
			 m_road_buildhelp_overlay_jobid);
	}
}

/*
===============
Remove road building data from road overlay
===============
*/
void Interactive_Base::roadb_remove_overlay()
{
	assert(m_buildroad);

	//log("Remove overlay\n");

	//  preview of the road
	Overlay_Manager & overlay_manager = egbase().map().overlay_manager();
	if (m_jobid)
		overlay_manager.remove_road_overlay(m_jobid);
	m_jobid = Overlay_Manager::Job_Id::Null();

	// build hints
	if (m_road_buildhelp_overlay_jobid)
		overlay_manager.remove_overlay(m_road_buildhelp_overlay_jobid);
	m_road_buildhelp_overlay_jobid = Overlay_Manager::Job_Id::Null();
}


bool Interactive_Base::handle_key(bool down, SDL_keysym code)
{
	switch (code.sym) {
	case SDLK_PAGEUP:
		if (!get_display_flag(dfSpeed))
			break;

		if (down)
			if (upcast(Game, game, &m_egbase))
				if (GameController * const ctrl = game->gameController())
					ctrl->setDesiredSpeed(ctrl->desiredSpeed() + 1000);
		return true;

	case SDLK_PAGEDOWN:
		if (!get_display_flag(dfSpeed))
			break;

		if (down)
			if (upcast(Widelands::Game, game, &m_egbase))
				if (GameController * const ctrl = game->gameController()) {
					uint32_t const speed = ctrl->desiredSpeed();
					ctrl->setDesiredSpeed(1000 < speed ? speed - 1000 : 0);
				}
		return true;

	default:
		break;
	}

	return Map_View::handle_key(down, code);
}

