/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
#include "wui/edge_overlay_manager.h"
#include "wui/field_overlay_manager.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/logmessage.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"
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
	MiniMap* mm;
	MiniMap::Registry minimap;
	std::unique_ptr<QuickNavigation> quicknavigation;

	explicit InteractiveBaseInternals(QuickNavigation* qnav) : mm(nullptr), quicknavigation(qnav) {
	}
};

InteractiveBase::InteractiveBase(EditorGameBase& the_egbase, Section& global_s)
   : MapView(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres(), *this),
     // Initialize chatoveraly before the toolbar so it is below
     show_workarea_preview_(global_s.get_bool("workareapreview", true)),
     chat_overlay_(new ChatOverlay(this, 10, 25, get_w() / 2, get_h() - 25)),
     toolbar_(this, 0, 0, UI::Box::Horizontal),
     m(new InteractiveBaseInternals(new QuickNavigation(this))),
     field_overlay_manager_(new FieldOverlayManager()),
     edge_overlay_manager_(new EdgeOverlayManager()),
     egbase_(the_egbase),
#ifndef NDEBUG  //  not in releases
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
     workarea_pics_{g_gr->images().get("images/wui/overlays/workarea123.png"),
                    g_gr->images().get("images/wui/overlays/workarea23.png"),
                    g_gr->images().get("images/wui/overlays/workarea3.png"),
                    g_gr->images().get("images/wui/overlays/workarea12.png"),
                    g_gr->images().get("images/wui/overlays/workarea2.png"),
                    g_gr->images().get("images/wui/overlays/workarea1.png")} {

	resize_chat_overlay();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.width, message.height);
		   resize_chat_overlay();
		   adjust_toolbar_position();
		});
	sound_subscriber_ = Notifications::subscribe<NoteSound>([this](const NoteSound& note) {
		if (note.stereo_position != std::numeric_limits<uint32_t>::max()) {
			g_sound_handler.play_fx(note.fx, note.stereo_position, note.priority);
		} else if (note.coords != Widelands::Coords(-1, -1)) {
			g_sound_handler.play_fx(note.fx, stereo_position(note.coords), note.priority);
		}
	});

	toolbar_.set_layout_toplevel(true);
	changeview.connect([this] { mainview_move(); });

	set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
	set_panel_snap_distance(global_s.get_int("panel_snap_distance", 10));
	set_snap_windows_only_when_overlapping(
	   global_s.get_bool("snap_windows_only_when_overlapping", false));
	set_dock_windows_to_edges(global_s.get_bool("dock_windows_to_edges", false));

	//  Having this in the initializer list (before Sys_InitGraphics) will give
	//  funny results.
	sel_.pic = g_gr->images().get("images/ui_basic/fsel.png");

	setDefaultCommand(boost::bind(&InteractiveBase::cmd_lua, this, _1));
	addCommand("mapobject", boost::bind(&InteractiveBase::cmd_map_object, this, _1));
}

InteractiveBase::~InteractiveBase() {
	if (buildroad_) {
		abort_build_road();
	}
	for (auto& registry : registries_) {
		registry.unassign_toggle_button();
	}
}

UniqueWindowHandler& InteractiveBase::unique_windows() {
	return *unique_window_handler_;
}

void InteractiveBase::set_sel_pos(Widelands::NodeAndTriangle<> const center) {
	Map& map = egbase().map();

	// Remove old sel pointer
	if (sel_.jobid)
		field_overlay_manager_->remove_overlay(sel_.jobid);
	const FieldOverlayManager::OverlayId jobid = sel_.jobid =
	   field_overlay_manager_->next_overlay_id();

	sel_.pos = center;

	//  register sel overlay position
	if (sel_.triangles) {
		assert(center.triangle.t == TCoords<>::D || center.triangle.t == TCoords<>::R);
		Widelands::MapTriangleRegion<> mr(map, Area<TCoords<>>(center.triangle, sel_.radius));
		do
			field_overlay_manager_->register_overlay(
			   mr.location(), sel_.pic, 7, Vector2i::invalid(), jobid);
		while (mr.advance(map));
	} else {
		Widelands::MapRegion<> mr(map, Area<>(center.node, sel_.radius));
		do
			field_overlay_manager_->register_overlay(
			   mr.location(), sel_.pic, 7, Vector2i::invalid(), jobid);
		while (mr.advance(map));
		if (upcast(InteractiveGameBase const, igbase, this))
			if (upcast(Widelands::ProductionSite, productionsite, map[center.node].get_immovable())) {
				if (upcast(InteractivePlayer const, iplayer, igbase)) {
					const Widelands::Player& player = iplayer->player();
					if (!player.see_all() &&
					    (1 >= player.vision(Widelands::Map::get_index(center.node, map.get_width())) ||
					     player.is_hostile(*productionsite->get_owner())))
						return set_tooltip("");
				}
				set_tooltip(
				   productionsite->info_string(Widelands::Building::InfoStringFormat::kTooltip));
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
		set_sel_pos(get_sel_pos());  //  redraw
	}
}

/*
 * Set/Unset sel picture
 */
void InteractiveBase::set_sel_picture(const Image* image) {
	sel_.pic = image;
	set_sel_pos(get_sel_pos());  //  redraw
}
void InteractiveBase::unset_sel_picture() {
	set_sel_picture(g_gr->images().get("images/ui_basic/fsel.png"));
}

bool InteractiveBase::buildhelp() const {
	return field_overlay_manager_->buildhelp();
}

void InteractiveBase::show_buildhelp(bool t) {
	field_overlay_manager_->show_buildhelp(t);
	on_buildhelp_changed(t);
}

void InteractiveBase::toggle_buildhelp() {
	show_buildhelp(!field_overlay_manager_->buildhelp());
}

UI::Button* InteractiveBase::add_toolbar_button(const std::string& image_basename,
                                                const std::string& name,
                                                const std::string& tooltip_text,
                                                UI::UniqueWindow::Registry* window,
                                                bool bind_default_toggle) {
	UI::Button* button = new UI::Button(
	   &toolbar_, name, 0, 0, 34U, 34U, g_gr->images().get("images/ui_basic/but2.png"),
	   g_gr->images().get("images/" + image_basename + ".png"), tooltip_text);
	toolbar_.add(button);
	if (window) {
		window->assign_toggle_button(button);
		registries_.push_back(*window);
		if (bind_default_toggle) {
			button->sigclicked.connect(
			   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(*window)));
		}
	}
	return button;
}

void InteractiveBase::on_buildhelp_changed(bool /* value */) {
}

// Show the given workareas at the given coords and returns the overlay job id associated
FieldOverlayManager::OverlayId InteractiveBase::show_work_area(const WorkareaInfo& workarea_info,
                                                               Widelands::Coords coords) {
	const uint8_t workareas_nrs = workarea_info.size();
	WorkareaInfo::size_type wa_index;
	switch (workareas_nrs) {
	case 0:
		return 0;  // no workarea
	case 1:
		wa_index = 5;
		break;
	case 2:
		wa_index = 3;
		break;
	case 3:
		wa_index = 0;
		break;
	default:
		throw wexception("Encountered unexpected WorkareaInfo size %i", workareas_nrs);
	}
	Widelands::Map& map = egbase_.map();
	FieldOverlayManager::OverlayId overlay_id = field_overlay_manager_->next_overlay_id();

	Widelands::HollowArea<> hollow_area(Widelands::Area<>(coords, 0), 0);

	// Iterate through the work areas, from building to its enhancement
	WorkareaInfo::const_iterator it = workarea_info.begin();
	for (; it != workarea_info.end(); ++it) {
		hollow_area.radius = it->first;
		Widelands::MapHollowRegion<> mr(map, hollow_area);
		do
			field_overlay_manager_->register_overlay(
			   mr.location(), workarea_pics_[wa_index], 0, Vector2i::invalid(), overlay_id);
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
void InteractiveBase::postload() {
}

/*
===============
Called once per frame by the UI code
===============
*/
void InteractiveBase::think() {
	// If one of the arrow keys is pressed, scroll here
	const uint32_t scrollval = 10;

	if (keyboard_free() && Panel::allow_user_input()) {
		if (get_key_state(SDL_SCANCODE_UP) ||
		    (get_key_state(SDL_SCANCODE_KP_8) && (SDL_GetModState() ^ KMOD_NUM))) {
			pan_by(Vector2i(0, -scrollval));
		}
		if (get_key_state(SDL_SCANCODE_DOWN) ||
		    (get_key_state(SDL_SCANCODE_KP_2) && (SDL_GetModState() ^ KMOD_NUM))) {
			pan_by(Vector2i(0, scrollval));
		}
		if (get_key_state(SDL_SCANCODE_LEFT) ||
		    (get_key_state(SDL_SCANCODE_KP_4) && (SDL_GetModState() ^ KMOD_NUM))) {
			pan_by(Vector2i(-scrollval, 0));
		}
		if (get_key_state(SDL_SCANCODE_RIGHT) ||
		    (get_key_state(SDL_SCANCODE_KP_6) && (SDL_GetModState() ^ KMOD_NUM))) {
			pan_by(Vector2i(scrollval, 0));
		}
	}
	egbase().think();  // Call game logic here. The game advances.

	UI::Panel::think();
}

/*
===============
Draw debug overlay when appropriate.
===============
*/
void InteractiveBase::draw_overlay(RenderTarget& dst) {
	// Timing
	uint32_t curframe = SDL_GetTicks();

	frametime_ = curframe - lastframe_;
	avg_usframetime_ = ((avg_usframetime_ * 15) + (frametime_ * 1000)) / 16;
	lastframe_ = curframe;

	const Map& map = egbase().map();
	const bool is_game = dynamic_cast<const Game*>(&egbase());

	// Blit node information when in debug mode.
	if (get_display_flag(dfDebug) || !is_game) {
		std::string node_text;
		if (is_game) {
			const std::string gametime(gametimestring(egbase().get_gametime(), true));
			std::shared_ptr<const UI::RenderedText> rendered_text =
			   UI::g_fh1->render(as_condensed(gametime));
			rendered_text->draw(dst, Vector2i(5, 5));

			static boost::format node_format("(%i, %i)");
			node_text = as_condensed((node_format % sel_.pos.node.x % sel_.pos.node.y).str());
		} else {  // This is an editor
			static boost::format node_format("(%i, %i, %i)");
			const int32_t height = map[sel_.pos.node].get_height();
			node_text = as_condensed((node_format % sel_.pos.node.x % sel_.pos.node.y % height).str());
		}
		std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh1->render(node_text);
		rendered_text->draw(
		   dst, Vector2i(get_w() - 5, get_h() - rendered_text->height() - 5), UI::Align::kRight);
	}

	// Blit FPS when playing a game in debug mode.
	if (get_display_flag(dfDebug) && is_game) {
		static boost::format fps_format("%5.1f fps (avg: %5.1f fps)");
		std::shared_ptr<const UI::RenderedText> rendered_text = UI::g_fh1->render(as_condensed(
		   (fps_format % (1000.0 / frametime_) % (1000.0 / (avg_usframetime_ / 1000))).str()));
		rendered_text->draw(dst, Vector2i((get_w() - rendered_text->width()) / 2, 5));
	}
}

void InteractiveBase::mainview_move() {
	if (m->minimap.window) {
		m->mm->set_view(view_area().rect());
	}
}

// Open the minimap or close it if it's open
void InteractiveBase::toggle_minimap() {
	if (m->minimap.window) {
		delete m->minimap.window;
	} else {
		m->mm = new MiniMap(*this, &m->minimap);
		m->mm->warpview.connect(
		   [this](const Vector2f& map_pixel) { scroll_to_map_pixel(map_pixel, Transition::Smooth); });
		mainview_move();
	}
}

const std::vector<QuickNavigation::Landmark>& InteractiveBase::landmarks() {
	return m->quicknavigation->landmarks();
}

void InteractiveBase::set_landmark(size_t key, const MapView::View& landmark_view) {
	m->quicknavigation->set_landmark(key, landmark_view);
}

/**
 * Hide the minimap if it is currently shown; otherwise, do nothing.
 */
void InteractiveBase::hide_minimap() {
	m->minimap.destroy();
}

/**
===========
InteractiveBase::minimap_registry()

Exposes the Registry object of the minimap to derived classes
===========
*/
MiniMap::Registry& InteractiveBase::minimap_registry() {
	return m->minimap;
}

/*
===============
Return display flags (dfXXX) that modify the view of the map.
===============
*/
uint32_t InteractiveBase::get_display_flags() const {
	return display_flags_;
}

/*
===============
Change the display flags that modify the view of the map.
===============
*/
void InteractiveBase::set_display_flags(uint32_t flags) {
	display_flags_ = flags;
}

/*
===============
Get and set one individual flag of the display flags.
===============
*/
bool InteractiveBase::get_display_flag(uint32_t const flag) {
	return display_flags_ & flag;
}

void InteractiveBase::set_display_flag(uint32_t const flag, bool const on) {
	display_flags_ &= ~flag;

	if (on)
		display_flags_ |= flag;
}

/*
===============
Begin building a road
===============
*/
void InteractiveBase::start_build_road(Coords road_start, Widelands::PlayerNumber const player) {
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
void InteractiveBase::abort_build_road() {
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
void InteractiveBase::finish_build_road() {
	assert(buildroad_);

	roadb_remove_overlay();

	if (buildroad_->get_nsteps()) {
		upcast(Game, game, &egbase());

		// Build the path as requested
		if (game)
			game->send_player_build_road(road_build_player_, *new Widelands::Path(*buildroad_));
		else
			egbase().get_player(road_build_player_)->build_road(*new Widelands::Path(*buildroad_));

		if (allow_user_input() && (SDL_GetModState() & KMOD_CTRL)) {
			//  place flags
			const Map& map = egbase().map();
			const std::vector<Coords>& c_vector = buildroad_->get_coords();
			std::vector<Coords>::const_iterator const first = c_vector.begin() + 2;
			std::vector<Coords>::const_iterator const last = c_vector.end() - 2;

			if (SDL_GetModState() & KMOD_SHIFT) {
				for //  start to end
					(std::vector<Coords>::const_iterator it = first;
					 it <= last;
					 ++it)
					if (game)
						game->send_player_build_flag(road_build_player_, map.get_fcoords(*it));
					else
						egbase().get_player(road_build_player_)->build_flag(map.get_fcoords(*it));

			} else {
				for //  end to start
					(std::vector<Coords>::const_iterator it = last;
					 first <= it;
					 --it)
					if (game)
						game->send_player_build_flag(road_build_player_, map.get_fcoords(*it));
					else
						egbase().get_player(road_build_player_)->build_flag(map.get_fcoords(*it));
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

	Map& map = egbase().map();
	const Widelands::Player& player = egbase().player(road_build_player_);

	{  //  find a path to the clicked-on node
		Widelands::Path path;
		Widelands::CheckStepRoad cstep(player, Widelands::MOVECAPS_WALK);
		if (map.findpath(buildroad_->get_end(), field, 0, path, cstep, Map::fpBidiCost) < 0)
			return false;  //  could not find a path
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
			map.findpath(buildroad_->get_start(), field, 0, path, cstep, Map::fpBidiCost);
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

void InteractiveBase::log_message(const std::string& message) const {
	// Send to linked receivers
	LogMessage lm;
	lm.msg = message;
	lm.time = time(nullptr);
	Notifications::publish(lm);
}

/** Calculate  the position of an effect in relation to the visible part of the
 * screen.
 * \param position  where the event happened (map coordinates)
 * \return position in widelands' game window: left=0, right=254, not in
 * viewport = -1
 * \note This function can also be used to check whether a logical coordinate is
 * visible at all
*/
int32_t InteractiveBase::stereo_position(Widelands::Coords const position_map) {
	assert(position_map);

	// Viewpoint is the point of the map in pixel which is shown in the upper
	// left corner of window or fullscreen
	const MapView::ViewArea area = view_area();
	if (!area.contains(position_map)) {
		return -1;
	}
	const Vector2f position_pix = area.move_inside(position_map);
	return static_cast<int>((position_pix.x - area.rect().x) * 254 / area.rect().w);
}

// Repositions the chat overlay
void InteractiveBase::resize_chat_overlay() {
	// 34 is the button height of the bottom menu
	chat_overlay_->set_size(get_w() / 2, get_h() - 25 - 34);
	chat_overlay_->recompute();
}

/*
===============
Add road building data to the road overlay
===============
*/
void InteractiveBase::roadb_add_overlay() {
	assert(buildroad_);

	Map& map = egbase().map();

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
		set_to |= Widelands::RoadType::kNormal << shift;
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
			continue;  // need to be able to walk there

		//  can't build on robusts
		Widelands::BaseImmovable* const imm = map.get_immovable(neighb);
		if (imm && imm->get_size() >= Widelands::BaseImmovable::SMALL) {
			if (!(dynamic_cast<const Widelands::Flag*>(imm) ||
			      (dynamic_cast<const Widelands::Road*>(imm) && (caps & Widelands::BUILDCAPS_FLAG))))
				continue;
		}

		if (buildroad_->get_index(neighb) >= 0)
			continue;  // the road can't cross itself

		int32_t slope;

		if (Widelands::WALK_E == dir || Widelands::WALK_NE == dir || Widelands::WALK_SE == dir)
			slope = neighb.field->get_height() - endpos.field->get_height();
		else
			slope = endpos.field->get_height() - neighb.field->get_height();

		const char* name = nullptr;

		if (slope <= -4)
			name = "images/wui/overlays/roadb_reddown.png";
		else if (slope <= -2)
			name = "images/wui/overlays/roadb_yellowdown.png";
		else if (slope < 2)
			name = "images/wui/overlays/roadb_green.png";
		else if (slope < 4)
			name = "images/wui/overlays/roadb_yellow.png";
		else
			name = "images/wui/overlays/roadb_red.png";

		field_overlay_manager_->register_overlay(
		   neighb, g_gr->images().get(name), 7, Vector2i::invalid(), road_buildhelp_overlay_jobid_);
	}
}

/*
===============
Remove road building data from road overlay
===============
*/
void InteractiveBase::roadb_remove_overlay() {
	assert(buildroad_);

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

bool InteractiveBase::handle_key(bool const down, SDL_Keysym const code) {
	if (m->quicknavigation->handle_key(down, code))
		return true;

	if (down) {
		switch (code.sym) {
		case SDLK_KP_9:
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
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
			if (code.mod & KMOD_NUM) {
				break;
			}
			FALLS_THROUGH;
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

void InteractiveBase::cmd_lua(const std::vector<std::string>& args) {
	const std::string cmd = boost::algorithm::join(args, " ");

	DebugConsole::write("Starting Lua interpretation!");
	try {
		egbase().lua().interpret_string(cmd);
	} catch (LuaError& e) {
		DebugConsole::write(e.what());
	}

	DebugConsole::write("Ending Lua interpretation!");
}

/**
 * Show a map object's debug window
 */
void InteractiveBase::cmd_map_object(const std::vector<std::string>& args) {
	if (args.size() != 2) {
		DebugConsole::write("usage: mapobject <mapobject serial>");
		return;
	}

	uint32_t serial = atoi(args[1].c_str());
	MapObject* obj = egbase().objects().get_object(serial);

	if (!obj) {
		DebugConsole::write(str(boost::format("No MapObject with serial number %1%") % serial));
		return;
	}

	show_mapobject_debug(*this, *obj);
}
