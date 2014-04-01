/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/interactive_base.h"

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "constants.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "gamecontroller.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "logic/checkstep.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/immovable.h"
#include "logic/maphollowregion.h"
#include "logic/maptriangleregion.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "profile/profile.h"
#include "scripting/scripting.h"
#include "text_layout.h"
#include "upcast.h"
#include "wlapplication.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"
#include "wui/overlay_manager.h"
#include "wui/quicknavigation.h"

using boost::format;
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
	std::unique_ptr<QuickNavigation> quicknavigation;

	InteractiveBaseInternals(QuickNavigation * qnav)
	:
	mm(nullptr),
	quicknavigation(qnav)
	{}
};

Interactive_Base::Interactive_Base
	(Editor_Game_Base & the_egbase, Section & global_s)
	:
	Map_View(nullptr, 0, 0, global_s.get_int("xres", XRES), global_s.get_int("yres", YRES), *this),
	m_show_workarea_preview(global_s.get_bool("workareapreview", true)),
	m
		(new InteractiveBaseInternals
		 (new QuickNavigation(the_egbase, get_w(), get_h()))),
	m_egbase                      (the_egbase),
#ifndef NDEBUG //  not in releases
	m_display_flags               (dfDebug),
#else
	m_display_flags               (0),
#endif
	m_lastframe                   (WLApplication::get()->get_time()),
	m_frametime                   (0),
	m_avg_usframetime             (0),
	m_jobid                       (Overlay_Manager::Job_Id::Null()),
	m_road_buildhelp_overlay_jobid(Overlay_Manager::Job_Id::Null()),
	m_buildroad                   (nullptr),
	m_road_build_player           (0),
	// Initialize chatoveraly before the toolbar so it is below
	m_chatOverlay                 (new ChatOverlay(this, 10, 25, get_w() / 2, get_h() - 25)),
	m_toolbar                     (this, 0, 0, UI::Box::Horizontal),
	m_label_speed_shadow
		(this, get_w() - 1, 0, std::string(), UI::Align_TopRight),
	m_label_speed
		(this, get_w(), 1, std::string(), UI::Align_TopRight)
{
	m_toolbar.set_layout_toplevel(true);
	m->quicknavigation->set_setview
		(boost::bind(&Map_View::set_viewpoint, this, _1, true));
	set_changeview
		(boost::bind(&QuickNavigation::view_changed,
		 m->quicknavigation.get(), _1, _2));

	changeview.connect(boost::bind(&Interactive_Base::mainview_move, this, _1, _2));

	set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
	set_panel_snap_distance (global_s.get_int("panel_snap_distance", 10));
	set_snap_windows_only_when_overlapping
		(global_s.get_bool("snap_windows_only_when_overlapping", false));
	set_dock_windows_to_edges
		(global_s.get_bool("dock_windows_to_edges", false));

	m_chatOverlay->setLogProvider(m_log_sender);

	//  Switch to the new graphics system now, if necessary.
	WLApplication::get()->refresh_graphics();

	//  Having this in the initializer list (before Sys_InitGraphics) will give
	//  funny results.
	m_sel.pic = g_gr->images().get("pics/fsel.png");

	// Load workarea images.
	// Start at idx 0 for 2 enhancements, idx 3 for 1, idx 5 if none
	workarea_pics[0] = g_gr->images().get("pics/workarea123.png");
	workarea_pics[1] = g_gr->images().get("pics/workarea23.png");
	workarea_pics[2] = g_gr->images().get("pics/workarea3.png");
	workarea_pics[3] = g_gr->images().get("pics/workarea12.png");
	workarea_pics[4] = g_gr->images().get("pics/workarea2.png");
	workarea_pics[5] = g_gr->images().get("pics/workarea1.png");

	m_label_speed.set_visible(false);
	m_label_speed_shadow.set_visible(false);

	UI::TextStyle style_shadow = m_label_speed.get_textstyle();
	style_shadow.fg = RGBColor(0, 0, 0);
	m_label_speed_shadow.set_textstyle(style_shadow);

	setDefaultCommand (boost::bind(&Interactive_Base::cmdLua, this, _1));
	addCommand
		("mapobject", boost::bind(&Interactive_Base::cmdMapObject, this, _1));
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
		if (upcast(Interactive_GameBase const, igbase, this))
			if
				(upcast
				 	(Widelands::ProductionSite,
				 	 productionsite,
				 	 map[center.node].get_immovable()))
			{
				if (upcast(Interactive_Player const, iplayer, igbase)) {
					const Widelands::Player & player = iplayer->player();
					if
						(not player.see_all()
						 and
						  (1
						   >=
						   player.vision
							   (Widelands::Map::get_index
								   (center.node, map.get_width()))
						   or
						   player.is_hostile(*productionsite->get_owner())))
						return set_tooltip("");
				}
				set_tooltip(productionsite->info_string(igbase->building_tooltip_format()));
				return;
			}
	}
	set_tooltip("");
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
	m_sel.pic = g_gr->images().get(file);
	set_sel_pos(get_sel_pos()); //  redraw
}
void Interactive_Base::unset_sel_picture() {
	set_sel_picture("pics/fsel.png");
}


void Interactive_Base::toggle_buildhelp() {
	egbase().map().overlay_manager().toggle_buildhelp();
}
bool Interactive_Base::buildhelp() {
	return egbase().map().overlay_manager().buildhelp();
}
void Interactive_Base::show_buildhelp(bool t) {
	egbase().map().overlay_manager().show_buildhelp(t);
}

// Show the given workareas at the given coords and returns the overlay job id associated
Overlay_Manager::Job_Id Interactive_Base::show_work_area
	(const Workarea_Info & workarea_info, Widelands::Coords coords)
{
	uint8_t workareas_nrs = workarea_info.size();
	Workarea_Info::size_type wa_index;
	switch (workareas_nrs) {
		case 0: return Overlay_Manager::Job_Id::Null(); break; // no workarea
		case 1: wa_index = 5; break;
		case 2: wa_index = 3; break;
		case 3: wa_index = 0; break;
		default: assert(false); break;
	}
	Widelands::Map & map = m_egbase.map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	Overlay_Manager::Job_Id job_id = overlay_manager.get_a_job_id();

	Widelands::HollowArea<> hollow_area(Widelands::Area<>(coords, 0), 0);

	// Iterate through the work areas, from building to its enhancement
	Workarea_Info::const_iterator it = workarea_info.begin();
	for (; it != workarea_info.end(); ++it) {
		assert(wa_index < NUMBER_OF_WORKAREA_PICS);
		hollow_area.radius = it->first;
		Widelands::MapHollowRegion<> mr(map, hollow_area);
		do
			overlay_manager.register_overlay
				(mr.location(),
					workarea_pics[wa_index],
					0,
					Point::invalid(),
					job_id);
		while (mr.advance(map));
		wa_index++;
		hollow_area.hole_radius = hollow_area.radius;
	}
	return job_id;
#if 0
		//  This is debug output.
		//  Improvement suggestion: add to sign explanation window instead.
		container_iterate_const(Workarea_Info, workarea_info, i) {
			log("Radius: %i\n", i.current->first);
			container_iterate_const(std::set<std::string>, i.current->second, j)
				log("        %s\n", j.current->c_str());
		}
#endif
}

void Interactive_Base::hide_work_area(Overlay_Manager::Job_Id job_id) {
	Widelands::Map & map = m_egbase.map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	overlay_manager.remove_overlay(job_id);
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
		snprintf(buffer, sizeof(buffer), ("%u.%ux"), speed / 1000, speed / 100 % 10);
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
				m_label_speed.set_text(
					(format
						 /** TRANSLATORS: actual_speed (desired_speed) */
						(_("%1$s (%2$s)"))
						% speedString(real).c_str()
						% speedString(desired).c_str()
					).str().c_str()
				);
			}
		} else
			m_label_speed.set_text(_("NO GAME CONTROLLER"));
		m_label_speed.set_visible(true);
	} else
		m_label_speed.set_visible(false);

	m_label_speed_shadow.set_text(m_label_speed.get_text());
	m_label_speed_shadow.set_visible(m_label_speed.is_visible());

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

	if (keyboard_free() && Panel::allow_user_input()) {
		if (get_key_state(SDLK_UP) || (get_key_state(SDLK_KP8) && (SDL_GetModState() ^ KMOD_NUM)))
			set_rel_viewpoint(Point(0, -scrollval), false);
		if (get_key_state(SDLK_DOWN) || (get_key_state(SDLK_KP2) && (SDL_GetModState() ^ KMOD_NUM)))
			set_rel_viewpoint(Point(0,  scrollval), false);
		if (get_key_state(SDLK_LEFT) || (get_key_state(SDLK_KP4) && (SDL_GetModState() ^ KMOD_NUM)))
			set_rel_viewpoint(Point(-scrollval, 0), false);
		if (get_key_state(SDLK_RIGHT) || (get_key_state(SDLK_KP6) && (SDL_GetModState() ^ KMOD_NUM)))
			set_rel_viewpoint(Point (scrollval, 0), false);
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
void Interactive_Base::draw_overlay(RenderTarget& dst) {
	// Blit node information when in debug mode.
	if (get_display_flag(dfDebug) or not dynamic_cast<const Game*>(&egbase())) {
		static format node_format("%3i %3i");
		const std::string node_text = as_uifont
			((node_format % m_sel.pos.node.x % m_sel.pos.node.y).str(), UI_FONT_SIZE_BIG);
		dst.blit(Point(5, 5), UI::g_fh1->render(node_text), CM_Normal, UI::Align_Left);
	}

	// Blit FPS when in debug mode.
	if (get_display_flag(dfDebug)) {
		static format fps_format("%5.1f fps (avg: %5.1f fps)");
		const std::string fps_text = as_uifont
			((fps_format %
			  (1000.0 / m_frametime) % (1000.0 / (m_avg_usframetime / 1000)))
			 .str(), UI_FONT_SIZE_BIG);
		dst.blit(Point(90, 5), UI::g_fh1->render(fps_text), CM_Normal, UI::Align_Left);
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
	set_viewpoint(Point(x, y), true);
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

	const Map & map = egbase().map();
	uint32_t const x = (c.x + (c.y & 1) * 0.5) * TRIANGLE_WIDTH;
	uint32_t const y = c.y * TRIANGLE_HEIGHT - map[c].get_height() * HEIGHT_FACTOR;
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

	set_viewpoint(pos - Point(get_w() / 2, get_h() / 2), true);
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
		m->mm->warpview.connect(boost::bind(&Interactive_Base::minimap_warp, this, _1, _2));

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
	delete m->minimap.window;
}

/**
===========
Interactive_Base::minimap_registry()

Exposes the Registry object of the minimap to derived classes
===========
*/
UI::UniqueWindow::Registry & Interactive_Base::minimap_registry() {
	return m->minimap;
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
	m_buildroad = nullptr;
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
		upcast(Game, game, &egbase());

		// Build the path as requested
		if (game)
			game->send_player_build_road
				(m_road_build_player, *new Widelands::Path(*m_buildroad));
		else
			egbase().get_player(m_road_build_player)->build_road
				(*new Widelands::Path(*m_buildroad));

		if
			(allow_user_input() and
			 (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)))
		{
			//  place flags
			const Map & map = egbase().map();
			const std::vector<Coords>         &       c_vector =
				m_buildroad->get_coords();
			std::vector<Coords>::const_iterator const first    =
				c_vector.begin() + 2;
			std::vector<Coords>::const_iterator const last     =
				c_vector.end  () - 2;

			if (get_key_state(SDLK_LSHIFT) || get_key_state(SDLK_RSHIFT)) {
				for //  start to end
					(std::vector<Coords>::const_iterator it = first;
					 it <= last;
					 ++it)
						if (game)
							game->send_player_build_flag
								(m_road_build_player, map.get_fcoords(*it));
						else
							egbase().get_player(m_road_build_player)->build_flag
								(map.get_fcoords(*it));

			} else {
				for //  end to start
					(std::vector<Coords>::const_iterator it = last;
					 first <= it;
					 --it)
						if (game)
							game->send_player_build_flag
								(m_road_build_player, map.get_fcoords(*it));
						else
							egbase().get_player(m_road_build_player)->build_flag
								(map.get_fcoords(*it));
			}
		}
	}

	delete m_buildroad;
	m_buildroad = nullptr;
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
	const Widelands::Player & player = egbase().player(m_road_build_player);

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
				const std::vector<Coords> & road_cp = m_buildroad->get_coords();
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
Coords Interactive_Base::get_build_road_start() const {
	assert(m_buildroad);

	return m_buildroad->get_start();
}

/*
===============
Return the current road-building endpoint
===============
*/
Coords Interactive_Base::get_build_road_end() const {
	assert(m_buildroad);

	return m_buildroad->get_end();
}

void Interactive_Base::log_message(const std::string& message) const
{
	// Send to linked receivers
	LogMessage lm;
	lm.msg = message;
	lm.time = time(nullptr);
	m_log_sender.send(lm);
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

		if (dir < Widelands::WALK_E || dir > Widelands::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}

		int32_t const shift = 2 * (dir - Widelands::WALK_E);

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
				   (caps & Widelands::BUILDCAPS_FLAG))))
				continue;
		}

		if (m_buildroad->get_index(neighb) >= 0)
			continue; // the road can't cross itself

		int32_t slope;

		if
			(Widelands::WALK_E == dir
			 || Widelands::WALK_NE == dir
			 || Widelands::WALK_SE == dir)
			slope = neighb.field->get_height() - endpos.field->get_height();
		else
			slope = endpos.field->get_height() - neighb.field->get_height();

		const char * name = nullptr;

		if (slope <= -4)
			name = "pics/roadb_reddown.png";
		else if (slope <= -2)
			name = "pics/roadb_yellowdown.png";
		else if (slope < 2)
			name = "pics/roadb_green.png";
		else if (slope < 4)
			name = "pics/roadb_yellow.png";
		else
			name = "pics/roadb_red.png";

		egbase().map().overlay_manager().register_overlay
			(neighb,
			 g_gr->images().get(name),
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


bool Interactive_Base::handle_key(bool const down, SDL_keysym const code)
{
	if (m->quicknavigation->handle_key(down, code))
		return true;

	switch (code.sym) {
	case SDLK_KP9:
		if (code.mod & KMOD_NUM)
			break;
		/* no break */
	case SDLK_PAGEUP:
		if (!get_display_flag(dfSpeed))
			break;

		if (down)
			if (upcast(Game, game, &m_egbase))
				if (GameController * const ctrl = game->gameController())
					ctrl->setDesiredSpeed(ctrl->desiredSpeed() + 1000);
		return true;

	case SDLK_PAUSE:
		if (down)
			if (upcast(Game, game, &m_egbase))
				if (GameController * const ctrl = game->gameController())
					ctrl->togglePaused();
		return true;

	case SDLK_KP3:
		if (code.mod & KMOD_NUM)
			break;
		/* no break */
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
#ifndef NDEBUG //  only in debug builds
		case SDLK_F6:
			if (get_display_flag(dfDebug)) {
				new GameChatMenu
					(this, m_debugconsole, *DebugConsole::getChatProvider());
			}
			return true;
#endif
	default:
		break;
	}

	return Map_View::handle_key(down, code);
}

void Interactive_Base::cmdLua(const std::vector<std::string> & args)
{
	std::string cmd;

	// Drop lua, start with the second word
	for
		(wl_const_range<std::vector<std::string> >
		 i(args.begin(), args.end());;)
	{
		cmd += i.front();
		if (i.advance().empty())
			break;
		cmd += ' ';
	}

	DebugConsole::write("Starting Lua interpretation!");
	try {
		egbase().lua().interpret_string(cmd);
	} catch (LuaError & e) {
		DebugConsole::write(e.what());
	}

	DebugConsole::write("Ending Lua interpretation!");
}

/**
 * Show a map object's debug window
 */
void Interactive_Base::cmdMapObject(const std::vector<std::string>& args)
{
	if (args.size() != 2) {
		DebugConsole::write("usage: mapobject <mapobject serial>");
		return;
	}

	uint32_t serial = atoi(args[1].c_str());
	Map_Object * obj = egbase().objects().get_object(serial);

	if (!obj) {
		DebugConsole::write
			(str(format("No Map_Object with serial number %1%") % serial));
		return;
	}

	show_mapobject_debug(*this, *obj);
}
