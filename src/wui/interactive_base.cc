/*
 * Copyright (C) 2002-2004, 2006-2011, 2015 by the Widelands Development Team
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

#include <memory>

#include <boost/algorithm/string/join.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "base/time_string.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler1.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/maphollowregion.h"
#include "logic/maptriangleregion.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "wlapplication.h"
#include "wui/edge_overlay_manager.h"
#include "wui/field_overlay_manager.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/logmessage.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"
#include "wui/quicknavigation.h"
#include "wui/unique_window_handler.h"

using Widelands::Area;
using Widelands::CoordPath;
using Widelands::Coords;
using Widelands::EditorGameBase;
using Widelands::Game;
using Widelands::Map;
using Widelands::MapObject;
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

InteractiveBase::InteractiveBase(EditorGameBase& the_egbase, Section& global_s)
   : MapView(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres(), *this),
     // Initialize chatoveraly before the toolbar so it is below
     show_workarea_preview_(global_s.get_bool("workareapreview", true)),
     chat_overlay_(new ChatOverlay(this, 10, 25, get_w() / 2, get_h() - 25)),
     toolbar_(this, 0, 0, UI::Box::Horizontal),
     m(new InteractiveBaseInternals(new QuickNavigation(the_egbase, get_w(), get_h()))),
	  field_overlay_manager_(new FieldOverlayManager()),
	  edge_overlay_manager_(new EdgeOverlayManager()),
     egbase_(the_egbase),
#ifndef NDEBUG //  not in releases
     display_flags_(dfDebug),
#else
     display_flags_(0),
#endif
     lastframe_(SDL_GetTicks()),
     frametime_(0),
     avg_usframetime_(0),
     jobid_(0),
     road_buildhelp_overlay_jobid_(0),
     buildroad_(nullptr),
     road_build_player_(0),
     unique_window_handler_(new UniqueWindowHandler()),
     // Start at idx 0 for 2 enhancements, idx 3 for 1, idx 5 if none
     workarea_pics_{g_gr->images().get("pics/workarea123.png"),
                     g_gr->images().get("pics/workarea23.png"),
                     g_gr->images().get("pics/workarea3.png"),
                     g_gr->images().get("pics/workarea12.png"),
                     g_gr->images().get("pics/workarea2.png"),
                     g_gr->images().get("pics/workarea1.png")} {

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.width, message.height);

		   chat_overlay_->set_size(get_w() / 2, get_h() - 25);
		   chat_overlay_->recompute();
		   adjust_toolbar_position();
		});

	toolbar_.set_layout_toplevel(true);
	m->quicknavigation->set_setview
		(boost::bind(&MapView::set_viewpoint, this, _1, true));
	set_changeview
		(boost::bind(&QuickNavigation::view_changed,
		 m->quicknavigation.get(), _1, _2));

	changeview.connect(boost::bind(&InteractiveBase::mainview_move, this, _1, _2));

	set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
	set_panel_snap_distance (global_s.get_int("panel_snap_distance", 10));
	set_snap_windows_only_when_overlapping
		(global_s.get_bool("snap_windows_only_when_overlapping", false));
	set_dock_windows_to_edges
		(global_s.get_bool("dock_windows_to_edges", false));

	//  Having this in the initializer list (before Sys_InitGraphics) will give
	//  funny results.
	sel_.pic = g_gr->images().get("pics/fsel.png");

	setDefaultCommand (boost::bind(&InteractiveBase::cmd_lua, this, _1));
	addCommand
		("mapobject", boost::bind(&InteractiveBase::cmd_map_object, this, _1));
}


InteractiveBase::~InteractiveBase()
{
	if (buildroad_)
		abort_build_road();
}

UniqueWindowHandler& InteractiveBase::unique_windows() {
	return *unique_window_handler_;
}


void InteractiveBase::set_sel_pos(Widelands::NodeAndTriangle<> const center)
{
	Map & map = egbase().map();

	// Remove old sel pointer
	if (sel_.jobid)
		field_overlay_manager_->remove_overlay(sel_.jobid);
	const FieldOverlayManager::OverlayId jobid =
		sel_.jobid = field_overlay_manager_->next_overlay_id();

	sel_.pos = center;

	//  register sel overlay position
	if (sel_.triangles) {
		assert
			(center.triangle.t == TCoords<>::D ||
			 center.triangle.t == TCoords<>::R);
		Widelands::MapTriangleRegion<> mr
			(map, Area<TCoords<> >(center.triangle, sel_.radius));
		do
			field_overlay_manager_->register_overlay
				(mr.location(), sel_.pic, 7, Point::invalid(), jobid);
		while (mr.advance(map));
	} else {
		Widelands::MapRegion<> mr(map, Area<>(center.node, sel_.radius));
		do
			field_overlay_manager_->register_overlay
				(mr.location(), sel_.pic, 7, Point::invalid(), jobid);
		while (mr.advance(map));
		if (upcast(InteractiveGameBase const, igbase, this))
			if
				(upcast
				 	(Widelands::ProductionSite,
				 	 productionsite,
				 	 map[center.node].get_immovable()))
			{
				if (upcast(InteractivePlayer const, iplayer, igbase)) {
					const Widelands::Player & player = iplayer->player();
					if
						(!player.see_all()
						 &&
						  (1
						   >=
						   player.vision
							   (Widelands::Map::get_index
								   (center.node, map.get_width()))
						   ||
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
void InteractiveBase::set_sel_radius(const uint32_t n) {
	if (n != sel_.radius) {
		sel_.radius = n;
		set_sel_pos(get_sel_pos()); //  redraw
	}
}

/*
 * Set/Unset sel picture
 */
void InteractiveBase::set_sel_picture(const char * const file) {
	sel_.pic = g_gr->images().get(file);
	set_sel_pos(get_sel_pos()); //  redraw
}
void InteractiveBase::unset_sel_picture() {
	set_sel_picture("pics/fsel.png");
}

bool InteractiveBase::buildhelp() const {
	return field_overlay_manager_->buildhelp();
}

void InteractiveBase::show_buildhelp(bool t) {
	field_overlay_manager_->show_buildhelp(t);
	on_buildhelp_changed(t);
}

void InteractiveBase::toggle_buildhelp() {
	field_overlay_manager_->show_buildhelp(!field_overlay_manager_->buildhelp());
}

void InteractiveBase::on_buildhelp_changed(bool /* value */) {
}

// Show the given workareas at the given coords and returns the overlay job id associated
FieldOverlayManager::OverlayId InteractiveBase::show_work_area(const WorkareaInfo& workarea_info,
                                                               Widelands::Coords coords) {
	const uint8_t workareas_nrs = workarea_info.size();
	WorkareaInfo::size_type wa_index;
	switch (workareas_nrs) {
		case 0: return 0; // no workarea
		case 1: wa_index = 5; break;
		case 2: wa_index = 3; break;
		case 3: wa_index = 0; break;
		default:
			throw wexception("Encountered unexpected WorkareaInfo size %i", workareas_nrs);
	}
	Widelands::Map & map = egbase_.map();
	FieldOverlayManager::OverlayId overlay_id = field_overlay_manager_->next_overlay_id();

	Widelands::HollowArea<> hollow_area(Widelands::Area<>(coords, 0), 0);

	// Iterate through the work areas, from building to its enhancement
	WorkareaInfo::const_iterator it = workarea_info.begin();
	for (; it != workarea_info.end(); ++it) {
		hollow_area.radius = it->first;
		Widelands::MapHollowRegion<> mr(map, hollow_area);
		do
			field_overlay_manager_->register_overlay
				(mr.location(),
					workarea_pics_[wa_index],
					0,
					Point::invalid(),
					overlay_id);
		while (mr.advance(map));
		wa_index++;
		hollow_area.hole_radius = hollow_area.radius;
	}
	return overlay_id;
}

void InteractiveBase::hide_work_area(FieldOverlayManager::OverlayId overlay_id) {
	field_overlay_manager_->remove_overlay(overlay_id);
}


/**
 * Called by \ref Game::postload at the end of the game loading
 * sequence.
 *
 * Default implementation does nothing.
 */
void InteractiveBase::postload() {}

/*
===============
Called once per frame by the UI code
===============
*/
void InteractiveBase::think()
{
	// Timing
	uint32_t curframe = SDL_GetTicks();

	frametime_ = curframe - lastframe_;
	avg_usframetime_ = ((avg_usframetime_  * 15) + (frametime_  * 1000)) / 16;
	lastframe_ = curframe;

	// If one of the arrow keys is pressed, scroll here
	const uint32_t scrollval = 10;

	if (keyboard_free() && Panel::allow_user_input()) {
		if (get_key_state(SDL_SCANCODE_UP) ||
			 (get_key_state(SDL_SCANCODE_KP_8) && (SDL_GetModState() ^ KMOD_NUM))) {
			set_rel_viewpoint(Point(0, -scrollval), false);
		}
		if (get_key_state(SDL_SCANCODE_DOWN) ||
			 (get_key_state(SDL_SCANCODE_KP_2) && (SDL_GetModState() ^ KMOD_NUM))) {
			set_rel_viewpoint(Point(0,  scrollval), false);
		}
		if (get_key_state(SDL_SCANCODE_LEFT) ||
			 (get_key_state(SDL_SCANCODE_KP_4) && (SDL_GetModState() ^ KMOD_NUM))) {
			set_rel_viewpoint(Point(-scrollval, 0), false);
		}
		if (get_key_state(SDL_SCANCODE_RIGHT) ||
			 (get_key_state(SDL_SCANCODE_KP_6) && (SDL_GetModState() ^ KMOD_NUM))) {
			set_rel_viewpoint(Point (scrollval, 0), false);
		}
	}

	egbase().think(); // Call game logic here. The game advances.

	//  The entire screen needs to be redrawn (unit movement, tile animation,
	//  etc...)
	g_gr->update();

	UI::Panel::think();
}


/*
===============
Draw debug overlay when appropriate.
===============
*/
void InteractiveBase::draw_overlay(RenderTarget& dst) {

	const Map & map = egbase().map();
	const bool is_game = dynamic_cast<const Game*>(&egbase());

	// Blit node information when in debug mode.
	if (get_display_flag(dfDebug) || !is_game) {
		std::string node_text;
		if (is_game) {
			const std::string gametime(gametimestring(egbase().get_gametime(), true));
			const std::string gametime_text = as_uifont(gametime, UI_FONT_SIZE_SMALL);
			dst.blit(Point(5, 5), UI::g_fh1->render(gametime_text), BlendMode::UseAlpha, UI::Align_TopLeft);

			static boost::format node_format("(%i, %i)");
			node_text = as_uifont
				((node_format % sel_.pos.node.x % sel_.pos.node.y).str(), UI_FONT_SIZE_SMALL);
		} else { //this is an editor
			static boost::format node_format("(%i, %i, %i)");
			const int32_t height = map[sel_.pos.node].get_height();
			node_text = as_uifont
				((node_format % sel_.pos.node.x % sel_.pos.node.y % height).str(), UI_FONT_SIZE_SMALL);
		}

		dst.blit(
			Point(get_w() - 5, get_h() - 5),
			UI::g_fh1->render(node_text),
			BlendMode::UseAlpha,
			UI::Align_BottomRight);
	}

	// Blit FPS when in debug mode.
	if (get_display_flag(dfDebug)) {
		static boost::format fps_format("%5.1f fps (avg: %5.1f fps)");
		const std::string fps_text = as_uifont(
		   (fps_format % (1000.0 / frametime_) % (1000.0 / (avg_usframetime_ / 1000))).str(),
		   UI_FONT_SIZE_SMALL);
		dst.blit(Point(5, (is_game) ? 25 : 5),
		         UI::g_fh1->render(fps_text),
		         BlendMode::UseAlpha,
		         UI::Align_Left);
	}
}

/** InteractiveBase::mainview_move(int32_t x, int32_t y)
 *
 * Signal handler for the main view's warpview updates the mini map's
 * viewpos marker position
 */
void InteractiveBase::mainview_move(int32_t x, int32_t y)
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
void InteractiveBase::minimap_warp(int32_t x, int32_t y)
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
void InteractiveBase::move_view_to(const Coords c)
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
void InteractiveBase::move_view_to_point(Point pos)
{
	if (m->minimap.window)
		m->mm->set_view_pos(pos.x, pos.y);

	set_viewpoint(pos - Point(get_w() / 2, get_h() / 2), true);
}


// Open the minimap or close it if it's open
void InteractiveBase::toggle_minimap() {
	if (m->minimap.window) {
		delete m->minimap.window;
	}
	else {
		m->mm = new MiniMap(*this, &m->minimap);
		m->mm->warpview.connect(boost::bind(&InteractiveBase::minimap_warp, this, _1, _2));

		// make sure the viewpos marker is at the right pos to start with
		const Point p = get_viewpoint();

		mainview_move(p.x, p.y);
	}
}

/**
 * Hide the minimap if it is currently shown; otherwise, do nothing.
 */
void InteractiveBase::hide_minimap()
{
	delete m->minimap.window;
}

/**
===========
InteractiveBase::minimap_registry()

Exposes the Registry object of the minimap to derived classes
===========
*/
UI::UniqueWindow::Registry & InteractiveBase::minimap_registry() {
	return m->minimap;
}

/*
===============
Return display flags (dfXXX) that modify the view of the map.
===============
*/
uint32_t InteractiveBase::get_display_flags() const
{
	return display_flags_;
}


/*
===============
Change the display flags that modify the view of the map.
===============
*/
void InteractiveBase::set_display_flags(uint32_t flags)
{
	display_flags_ = flags;
}

/*
===============
Get and set one individual flag of the display flags.
===============
*/
bool InteractiveBase::get_display_flag(uint32_t const flag)
{
	return display_flags_& flag;
}

void InteractiveBase::set_display_flag(uint32_t const flag, bool const on)
{
	display_flags_ &= ~flag;

	if (on)
		display_flags_ |= flag;
}

/*
===============
Begin building a road
===============
*/
void InteractiveBase::start_build_road
	(Coords road_start, Widelands::PlayerNumber const player)
{
	// create an empty path
	assert(!buildroad_);
	buildroad_ = new CoordPath(road_start);

	road_build_player_ = player;

	roadb_add_overlay();
}


/*
===============
Stop building the road
===============
*/
void InteractiveBase::abort_build_road()
{
	assert(buildroad_);

	roadb_remove_overlay();

	road_build_player_ = 0;

	delete buildroad_;
	buildroad_ = nullptr;
}


/*
===============
Finally build the road
===============
*/
void InteractiveBase::finish_build_road()
{
	assert(buildroad_);

	roadb_remove_overlay();

	if (buildroad_->get_nsteps()) {
		upcast(Game, game, &egbase());

		// Build the path as requested
		if (game)
			game->send_player_build_road
				(road_build_player_, *new Widelands::Path(*buildroad_));
		else
			egbase().get_player(road_build_player_)->build_road
				(*new Widelands::Path(*buildroad_));

		if
			(allow_user_input() &&
			 (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)))
		{
			//  place flags
			const Map & map = egbase().map();
			const std::vector<Coords>         &       c_vector =
				buildroad_->get_coords();
			std::vector<Coords>::const_iterator const first    =
				c_vector.begin() + 2;
			std::vector<Coords>::const_iterator const last     =
				c_vector.end  () - 2;

			if (get_key_state(SDL_SCANCODE_LSHIFT) || get_key_state(SDL_SCANCODE_RSHIFT)) {
				for //  start to end
					(std::vector<Coords>::const_iterator it = first;
					 it <= last;
					 ++it)
						if (game)
							game->send_player_build_flag
								(road_build_player_, map.get_fcoords(*it));
						else
							egbase().get_player(road_build_player_)->build_flag
								(map.get_fcoords(*it));

			} else {
				for //  end to start
					(std::vector<Coords>::const_iterator it = last;
					 first <= it;
					 --it)
						if (game)
							game->send_player_build_flag
								(road_build_player_, map.get_fcoords(*it));
						else
							egbase().get_player(road_build_player_)->build_flag
								(map.get_fcoords(*it));
			}
		}
	}

	delete buildroad_;
	buildroad_ = nullptr;
}


/*
===============
If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool InteractiveBase::append_build_road(Coords const field) {
	assert(buildroad_);

	Map & map = egbase().map();
	const Widelands::Player & player = egbase().player(road_build_player_);

	{ //  find a path to the clicked-on node
		Widelands::Path path;
		Widelands::CheckStepRoad cstep(player, Widelands::MOVECAPS_WALK);
		if
			(map.findpath
			 	(buildroad_->get_end(), field, 0, path, cstep, Map::fpBidiCost)
			 <
			 0)
			return false; //  could not find a path
		buildroad_->append(map, path);
	}

	{
		//  Fix the road by finding an optimal path through the set of nodes
		//  currently used by the road. This will not claim any new nodes, so it
		//  is guaranteed to not hinder building placement.
		Widelands::Path path;
		{
			Widelands::CheckStepLimited cstep;
			{
				for (const Coords& coord : buildroad_->get_coords()) {
					cstep.add_allowed_location(coord);
				}
			}
			map.findpath
				(buildroad_->get_start(), field, 0, path, cstep, Map::fpBidiCost);
		}
		buildroad_->truncate(0);
		buildroad_->append(map, path);
	}

	roadb_remove_overlay();
	roadb_add_overlay();

	return true;
}

/*
===============
Return the current road-building startpoint
===============
*/
Coords InteractiveBase::get_build_road_start() const {
	assert(buildroad_);

	return buildroad_->get_start();
}

/*
===============
Return the current road-building endpoint
===============
*/
Coords InteractiveBase::get_build_road_end() const {
	assert(buildroad_);

	return buildroad_->get_end();
}

void InteractiveBase::log_message(const std::string& message) const
{
	// Send to linked receivers
	LogMessage lm;
	lm.msg = message;
	lm.time = time(nullptr);
	Notifications::publish(lm);
}



/*
===============
Add road building data to the road overlay
===============
*/
void InteractiveBase::roadb_add_overlay()
{
	assert(buildroad_);

	//log("Add overlay\n");

	Map & map = egbase().map();

	// preview of the road
	assert(!jobid_);
	jobid_ = field_overlay_manager_->next_overlay_id();
	const CoordPath::StepVector::size_type nr_steps = buildroad_->get_nsteps();
	for (CoordPath::StepVector::size_type idx = 0; idx < nr_steps; ++idx) {
		Widelands::Direction dir = (*buildroad_)[idx];
		Coords c = buildroad_->get_coords()[idx];

		if (dir < Widelands::WALK_E || dir > Widelands::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}

		int32_t const shift = 2 * (dir - Widelands::WALK_E);

		uint8_t set_to = edge_overlay_manager_->get_overlay(c);
		set_to |=  Widelands::RoadType::kNormal << shift;
		edge_overlay_manager_->register_overlay(c, set_to, jobid_);
	}

	// build hints
	Widelands::FCoords endpos = map.get_fcoords(buildroad_->get_end());

	assert(!road_buildhelp_overlay_jobid_);
	road_buildhelp_overlay_jobid_ = field_overlay_manager_->next_overlay_id();
	for (int32_t dir = 1; dir <= 6; ++dir) {
		Widelands::FCoords neighb;
		int32_t caps;

		map.get_neighbour(endpos, dir, &neighb);
		caps = egbase().player(road_build_player_).get_buildcaps(neighb);

		if (!(caps & Widelands::MOVECAPS_WALK))
			continue; // need to be able to walk there

		//  can't build on robusts
		Widelands::BaseImmovable * const imm = map.get_immovable(neighb);
		if (imm && imm->get_size() >= Widelands::BaseImmovable::SMALL) {
			if
				(!(
					dynamic_cast<const Widelands::Flag *>(imm)
					||
					(
						dynamic_cast<const Widelands::Road *>(imm)
						&&
						(caps & Widelands::BUILDCAPS_FLAG)
					)
				))
				continue;
		}

		if (buildroad_->get_index(neighb) >= 0)
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

		field_overlay_manager_->register_overlay
			(neighb,
			 g_gr->images().get(name),
			 7,
			 Point::invalid(),
			 road_buildhelp_overlay_jobid_);
	}
}

/*
===============
Remove road building data from road overlay
===============
*/
void InteractiveBase::roadb_remove_overlay()
{
	assert(buildroad_);

	//log("Remove overlay\n");

	//  preview of the road
	if (jobid_) {
		edge_overlay_manager_->remove_overlay(jobid_);
	}
	jobid_ = 0;

	// build hints
	if (road_buildhelp_overlay_jobid_) {
		field_overlay_manager_->remove_overlay(road_buildhelp_overlay_jobid_);
	}
	road_buildhelp_overlay_jobid_ = 0;
}


bool InteractiveBase::handle_key(bool const down, SDL_Keysym const code)
{
	if (m->quicknavigation->handle_key(down, code))
		return true;

	if (down) {
		switch (code.sym) {
		case SDLK_KP_9:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		case SDLK_PAGEUP:
			if (upcast(Game, game, &egbase_)) {
				if (GameController* const ctrl = game->game_controller()) {
					ctrl->set_desired_speed(ctrl->desired_speed() + 1000);
				}
			}
			return true;

		case SDLK_PAUSE:
			if (upcast(Game, game, &egbase_)) {
				if (GameController* const ctrl = game->game_controller()) {
					ctrl->toggle_paused();
				}
			}
			return true;

		case SDLK_KP_3:
			if (code.mod & KMOD_NUM)
				break;
		/* no break */
		case SDLK_PAGEDOWN:
			if (upcast(Widelands::Game, game, &egbase_)) {
				if (GameController* const ctrl = game->game_controller()) {
					uint32_t const speed = ctrl->desired_speed();
					ctrl->set_desired_speed(1000 < speed ? speed - 1000 : 0);
				}
			}
			return true;
#ifndef NDEBUG  //  only in debug builds
		case SDLK_F6:
			GameChatMenu::create_script_console(
				this, debugconsole_, *DebugConsole::get_chat_provider());
			return true;
#endif
		default:
			break;
		}
	}

	return MapView::handle_key(down, code);
}

void InteractiveBase::cmd_lua(const std::vector<std::string> & args)
{
	const std::string cmd = boost::algorithm::join(args, " ");

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
void InteractiveBase::cmd_map_object(const std::vector<std::string>& args)
{
	if (args.size() != 2) {
		DebugConsole::write("usage: mapobject <mapobject serial>");
		return;
	}

	uint32_t serial = atoi(args[1].c_str());
	MapObject * obj = egbase().objects().get_object(serial);

	if (!obj) {
		DebugConsole::write
			(str(boost::format("No MapObject with serial number %1%") % serial));
		return;
	}

	show_mapobject_debug(*this, *obj);
}
