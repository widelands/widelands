/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "base/log.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/time_string.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "graphic/default_resolution.h"
#include "graphic/font_handler.h"
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
#include "logic/widelands_geometry.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "sound/sound_handler.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/logmessage.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"
#include "wui/unique_window_handler.h"

namespace {

using Widelands::Area;
using Widelands::CoordPath;
using Widelands::Coords;
using Widelands::EditorGameBase;
using Widelands::Game;
using Widelands::Map;
using Widelands::MapObject;
using Widelands::TCoords;

int caps_to_buildhelp(const Widelands::NodeCaps caps) {
	if (caps & Widelands::BUILDCAPS_MINE) {
		return Widelands::Field::Buildhelp_Mine;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		if (caps & Widelands::BUILDCAPS_PORT) {
			return Widelands::Field::Buildhelp_Port;
		}
		return Widelands::Field::Buildhelp_Big;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_MEDIUM) {
		return Widelands::Field::Buildhelp_Medium;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_SMALL) {
		return Widelands::Field::Buildhelp_Small;
	}
	if (caps & Widelands::BUILDCAPS_FLAG) {
		return Widelands::Field::Buildhelp_Flag;
	}
	return Widelands::Field::Buildhelp_None;
}

}  // namespace

InteractiveBase::InteractiveBase(EditorGameBase& the_egbase, Section& global_s)
   : UI::Panel(nullptr, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     buildhelp_(false),
     map_view_(this, the_egbase.map(), 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     // Initialize chatoveraly before the toolbar so it is below
     chat_overlay_(new ChatOverlay(this, 10, 25, get_w() / 2, get_h() - 25)),
     toolbar_(this, 0, 0, UI::Box::Horizontal),
     quick_navigation_(&map_view_),
     egbase_(the_egbase),
#ifndef NDEBUG  //  not in releases
     display_flags_(dfDebug),
#else
     display_flags_(0),
#endif
     lastframe_(SDL_GetTicks()),
     frametime_(0),
     avg_usframetime_(0),
     buildroad_(nullptr),
     road_build_player_(0),
     unique_window_handler_(new UniqueWindowHandler()) {

	// Load the buildhelp icons.
	{
		BuildhelpOverlay* buildhelp_overlay = buildhelp_overlays_;
		const char* filenames[] = {
		   "images/wui/overlays/set_flag.png", "images/wui/overlays/small.png",
		   "images/wui/overlays/medium.png",   "images/wui/overlays/big.png",
		   "images/wui/overlays/mine.png",     "images/wui/overlays/port.png"};
		const char* const* filename = filenames;

		//  Special case for flag, which has a different formula for hotspot_y.
		buildhelp_overlay->pic = g_gr->images().get(*filename);
		buildhelp_overlay->hotspot =
		   Vector2i(buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() - 1);

		const BuildhelpOverlay* const buildhelp_overlays_end =
		   buildhelp_overlay + Widelands::Field::Buildhelp_None;
		for (;;) {  // The other buildhelp overlays.
			++buildhelp_overlay;
			++filename;
			if (buildhelp_overlay == buildhelp_overlays_end)
				break;
			buildhelp_overlay->pic = g_gr->images().get(*filename);
			buildhelp_overlay->hotspot =
			   Vector2i(buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() / 2);
		}
	}

	resize_chat_overlay();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.width, message.height);
		   map_view_.set_size(message.width, message.height);
		   resize_chat_overlay();
		   adjust_toolbar_position();
	   });
	sound_subscriber_ = Notifications::subscribe<NoteSound>(
	   [this](const NoteSound& note) { play_sound_effect(note); });
	shipnotes_subscriber_ =
	   Notifications::subscribe<Widelands::NoteShip>([this](const Widelands::NoteShip& note) {
		   if (note.action == Widelands::NoteShip::Action::kWaitingForCommand &&
		       note.ship->get_ship_state() ==
		          Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
			   expedition_port_spaces_.emplace(note.ship, note.ship->exp_port_spaces().front());
		   }
	   });

	toolbar_.set_layout_toplevel(true);
	map_view_.changeview.connect([this] { mainview_move(); });
	map_view()->field_clicked.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		set_sel_pos(node_and_triangle);
	});
	map_view_.track_selection.connect([this](const Widelands::NodeAndTriangle<>& node_and_triangle) {
		if (!sel_.freeze) {
			set_sel_pos(node_and_triangle);
		}
	});

	set_border_snap_distance(global_s.get_int("border_snap_distance", 0));
	set_panel_snap_distance(global_s.get_int("panel_snap_distance", 10));
	set_snap_windows_only_when_overlapping(
	   global_s.get_bool("snap_windows_only_when_overlapping", false));
	set_dock_windows_to_edges(global_s.get_bool("dock_windows_to_edges", false));

	//  Having this in the initializer list (before Sys_InitGraphics) will give
	//  funny results.
	unset_sel_picture();

	setDefaultCommand(boost::bind(&InteractiveBase::cmd_lua, this, _1));
	addCommand("mapobject", boost::bind(&InteractiveBase::cmd_map_object, this, _1));
}

InteractiveBase::~InteractiveBase() {
	if (buildroad_) {
		abort_build_road();
	}
}

const InteractiveBase::BuildhelpOverlay*
InteractiveBase::get_buildhelp_overlay(const Widelands::NodeCaps caps) const {
	const int buildhelp_overlay_index = caps_to_buildhelp(caps);
	if (buildhelp_overlay_index < Widelands::Field::Buildhelp_None) {
		return &buildhelp_overlays_[buildhelp_overlay_index];
	}
	return nullptr;
}

bool InteractiveBase::has_workarea_preview(const Widelands::Coords& coords,
                                           const Widelands::Map* map) const {
	if (!map) {
		return workarea_previews_.count(coords) == 1;
	}
	for (const auto& pair : workarea_previews_) {
		uint32_t radius = 0;
		for (const auto& p : *pair.second) {
			radius = std::max(radius, p.first);
		}
		if (map->calc_distance(coords, pair.first) <= radius) {
			return true;
		}
	}
	return false;
}

UniqueWindowHandler& InteractiveBase::unique_windows() {
	return *unique_window_handler_;
}

void InteractiveBase::set_sel_pos(Widelands::NodeAndTriangle<> const center) {
	const Map& map = egbase().map();
	sel_.pos = center;

	if (upcast(InteractiveGameBase const, igbase, this))
		if (upcast(Widelands::ProductionSite, productionsite, map[center.node].get_immovable())) {
			if (upcast(InteractivePlayer const, iplayer, igbase)) {
				const Widelands::Player& player = iplayer->player();
				if (!player.see_all() &&
				    (1 >= player.vision(Widelands::Map::get_index(center.node, map.get_width())) ||
				     player.is_hostile(*productionsite->get_owner())))
					return set_tooltip("");
			}
			set_tooltip(productionsite->info_string(Widelands::Building::InfoStringFormat::kTooltip));
			return;
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

TextToDraw InteractiveBase::get_text_to_draw() const {
	TextToDraw text_to_draw = TextToDraw::kNone;
	auto display_flags = get_display_flags();
	if (display_flags & InteractiveBase::dfShowCensus) {
		text_to_draw = text_to_draw | TextToDraw::kCensus;
	}
	if (display_flags & InteractiveBase::dfShowStatistics) {
		text_to_draw = text_to_draw | TextToDraw::kStatistics;
	}
	return text_to_draw;
}

void InteractiveBase::unset_sel_picture() {
	set_sel_picture(g_gr->images().get("images/ui_basic/fsel.png"));
}

bool InteractiveBase::buildhelp() const {
	return buildhelp_;
}

void InteractiveBase::show_buildhelp(bool t) {
	buildhelp_ = t;
	on_buildhelp_changed(t);
}

void InteractiveBase::toggle_buildhelp() {
	show_buildhelp(!buildhelp());
}

UI::Button* InteractiveBase::add_toolbar_button(const std::string& image_basename,
                                                const std::string& name,
                                                const std::string& tooltip_text,
                                                UI::UniqueWindow::Registry* window,
                                                bool bind_default_toggle) {
	UI::Button* button =
	   new UI::Button(&toolbar_, name, 0, 0, 34U, 34U, UI::ButtonStyle::kWuiPrimary,
	                  g_gr->images().get("images/" + image_basename + ".png"), tooltip_text);
	toolbar_.add(button);
	if (window) {
		window->opened.connect([button] { button->set_perm_pressed(true); });
		window->closed.connect([button] { button->set_perm_pressed(false); });

		if (bind_default_toggle) {
			button->sigclicked.connect(
			   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(*window)));
		}
	}
	return button;
}

void InteractiveBase::on_buildhelp_changed(bool /* value */) {
}

bool InteractiveBase::has_expedition_port_space(const Widelands::Coords& coords) const {
	for (const auto& pair : expedition_port_spaces_) {
		if (pair.second == coords) {
			return true;
		}
	}
	return false;
}

// Show the given workareas at the given coords and returns the overlay job id associated
void InteractiveBase::show_workarea(const WorkareaInfo& workarea_info, Widelands::Coords coords) {
	workarea_previews_[coords] = &workarea_info;
}

/* Helper function to get the correct index for graphic/gl/workarea_program.cc::workarea_colors .
 * a, b, c are the indices for the three nodes bordering this triangle.
 * This function returns the biggest workarea type that matches all three corners.
 * The indices stand for:
 * 0 – all three circles
 * 1 – medium and outer circle
 * 2 – outer circle
 * 3 – inner and medium circle
 * 4 – medium circle
 * 5 – inner circle
 * We currently assume that no building will have more than three workarea circles.
 */
static uint8_t workarea_max(uint8_t a, uint8_t b, uint8_t c) {
	// Whether all nodes are part of the inner circle
	bool inner =
	   (a == 0 || a == 3 || a == 5) && (b == 0 || b == 3 || b == 5) && (c == 0 || c == 3 || c == 5);
	// Whether all nodes are part of the medium circle
	bool medium = (a == 0 || a == 1 || a == 3 || a == 4) && (b == 0 || b == 1 || b == 3 || b == 4) &&
	              (c == 0 || c == 1 || c == 3 || c == 4);
	// Whether all nodes are part of the outer circle
	bool outer = a <= 2 && b <= 2 && c <= 2;

	if (medium) {
		if (outer && inner) {
			return 0;
		} else if (inner) {
			return 3;
		} else if (outer) {
			return 1;
		} else {
			return 4;
		}
	} else if (outer) {
		assert(!inner);
		return 2;
	} else {
		assert(inner);
		return 5;
	}
}

Workareas InteractiveBase::get_workarea_overlays(const Widelands::Map& map) const {
	Workareas result_set;
	for (const auto& wa_pair : workarea_previews_) {
		std::map<Coords, uint8_t> intermediate_result;
		const Coords& coords = wa_pair.first;
		const WorkareaInfo* workarea_info = wa_pair.second;
		intermediate_result[coords] = 0;
		WorkareaInfo::size_type wa_index;
		switch (workarea_info->size()) {
		case 0:
			continue;  // no workarea
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
			throw wexception(
			   "Encountered unexpected WorkareaInfo size %i", static_cast<int>(workarea_info->size()));
		}

		Widelands::HollowArea<> hollow_area(Widelands::Area<>(coords, 0), 0);

		// Iterate through the work areas, from building to its enhancement
		WorkareaInfo::const_iterator it = workarea_info->begin();
		for (; it != workarea_info->end(); ++it) {
			hollow_area.radius = it->first;
			Widelands::MapHollowRegion<> mr(map, hollow_area);
			do {
				intermediate_result[mr.location()] = wa_index;
			} while (mr.advance(map));
			wa_index++;
			hollow_area.hole_radius = hollow_area.radius;
		}

		std::map<TCoords<>, uint8_t> result;
		for (const auto& pair : intermediate_result) {
			Coords c;
			map.get_brn(pair.first, &c);
			const auto brn = intermediate_result.find(c);
			if (brn == intermediate_result.end()) {
				continue;
			}
			map.get_bln(pair.first, &c);
			const auto bln = intermediate_result.find(c);
			map.get_rn(pair.first, &c);
			const auto rn = intermediate_result.find(c);
			if (bln != intermediate_result.end()) {
				result[TCoords<>(pair.first, Widelands::TriangleIndex::D)] =
				   workarea_max(pair.second, brn->second, bln->second);
			}
			if (rn != intermediate_result.end()) {
				result[TCoords<>(pair.first, Widelands::TriangleIndex::R)] =
				   workarea_max(pair.second, brn->second, rn->second);
			}
		}
		result_set.emplace(result);
	}
	return result_set;
}

void InteractiveBase::hide_workarea(const Widelands::Coords& coords) {
	workarea_previews_.erase(coords);
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
			map_view_.pan_by(Vector2i(0, -scrollval));
		}
		if (get_key_state(SDL_SCANCODE_DOWN) ||
		    (get_key_state(SDL_SCANCODE_KP_2) && (SDL_GetModState() ^ KMOD_NUM))) {
			map_view_.pan_by(Vector2i(0, scrollval));
		}
		if (get_key_state(SDL_SCANCODE_LEFT) ||
		    (get_key_state(SDL_SCANCODE_KP_4) && (SDL_GetModState() ^ KMOD_NUM))) {
			map_view_.pan_by(Vector2i(-scrollval, 0));
		}
		if (get_key_state(SDL_SCANCODE_RIGHT) ||
		    (get_key_state(SDL_SCANCODE_KP_6) && (SDL_GetModState() ^ KMOD_NUM))) {
			map_view_.pan_by(Vector2i(scrollval, 0));
		}
	}
	egbase().think();  // Call game logic here. The game advances.

	// Cleanup found port spaces if the ship sailed on or was destroyed
	for (auto it = expedition_port_spaces_.begin(); it != expedition_port_spaces_.end(); ++it) {
		if (!egbase().objects().object_still_available(it->first) ||
		    it->first->get_ship_state() != Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
			expedition_port_spaces_.erase(it);
			// If another port space also needs removing, we'll take care of it in the next frame
			return;
		}
	}

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

	Game* game = dynamic_cast<Game*>(&egbase());

	// This portion of code keeps the speed of game so that FPS are kept within
	// range 13 - 15, this is used for training of AI
	if (game != nullptr) {
		if (game->is_auto_speed()) {
			uint32_t cur_fps = 1000000 / avg_usframetime_;
			int32_t speed_diff = 0;
			if (cur_fps < 13) {
				speed_diff = -100;
			}
			if (cur_fps > 15) {
				speed_diff = +100;
			}
			if (speed_diff != 0) {
				if (GameController* const ctrl = game->game_controller()) {
					if ((ctrl->desired_speed() > 950 && ctrl->desired_speed() < 30000) ||
					    (ctrl->desired_speed() < 1000 && speed_diff > 0) ||
					    (ctrl->desired_speed() > 29999 && speed_diff < 0)) {
						ctrl->set_desired_speed(ctrl->desired_speed() + speed_diff);
					}
				}
			}
		}
	}

	// Node information
	std::string node_text("");
	if (game == nullptr) {
		// Always blit node information in the editor
		static boost::format node_format("(%i, %i, %i)");
		const int32_t height = egbase().map()[sel_.pos.node].get_height();
		node_text = (node_format % sel_.pos.node.x % sel_.pos.node.y % height).str();
	} else if (get_display_flag(dfDebug)) {
		// Blit node information for games in debug mode - we're not interested in the height
		static boost::format node_format("(%i, %i)");
		node_text = (node_format % sel_.pos.node.x % sel_.pos.node.y).str();
	}
	if (!node_text.empty()) {
		std::shared_ptr<const UI::RenderedText> rendered_text =
		   UI::g_fh->render(as_condensed(node_text));
		rendered_text->draw(
		   dst, Vector2i(get_w() - 5, get_h() - rendered_text->height() - 5), UI::Align::kRight);
	}

	// In-game clock and FPS
	if (game != nullptr) {
		// Blit in-game clock
		const std::string gametime(gametimestring(egbase().get_gametime(), true));
		std::shared_ptr<const UI::RenderedText> rendered_text =
		   UI::g_fh->render(as_condensed(gametime));
		rendered_text->draw(dst, Vector2i(5, 5));

		// Blit FPS when playing a game in debug mode
		if (get_display_flag(dfDebug)) {
			static boost::format fps_format("%5.1f fps (avg: %5.1f fps)");
			rendered_text = UI::g_fh->render(as_condensed(
			   (fps_format % (1000.0 / frametime_) % (1000.0 / (avg_usframetime_ / 1000))).str()));
			rendered_text->draw(dst, Vector2i((get_w() - rendered_text->width()) / 2, 5));
		}
	}
}

void InteractiveBase::blit_overlay(RenderTarget* dst,
                                   const Vector2i& position,
                                   const Image* image,
                                   const Vector2i& hotspot,
                                   float scale) {
	const Recti pixel_perfect_rect =
	   Recti(position - hotspot * scale, image->width() * scale, image->height() * scale);
	dst->blitrect_scale(pixel_perfect_rect.cast<float>(), image,
	                    Recti(0, 0, image->width(), image->height()), 1.f, BlendMode::UseAlpha);
}

void InteractiveBase::blit_field_overlay(RenderTarget* dst,
                                         const FieldsToDraw::Field& field,
                                         const Image* image,
                                         const Vector2i& hotspot,
                                         float scale) {
	blit_overlay(dst, field.rendertarget_pixel.cast<int>(), image, hotspot, scale);
}

void InteractiveBase::mainview_move() {
	if (minimap_registry_.window) {
		minimap_->set_view(map_view_.view_area().rect());
	}
}

// Open the minimap or close it if it's open
void InteractiveBase::toggle_minimap() {
	if (minimap_registry_.window) {
		delete minimap_registry_.window;
	} else {
		minimap_ = new MiniMap(*this, &minimap_registry_);
		minimap_->warpview.connect([this](const Vector2f& map_pixel) {
			map_view_.scroll_to_map_pixel(map_pixel, MapView::Transition::Smooth);
		});
		mainview_move();
	}
}

const std::vector<QuickNavigation::Landmark>& InteractiveBase::landmarks() {
	return quick_navigation_.landmarks();
}

void InteractiveBase::set_landmark(size_t key, const MapView::View& landmark_view) {
	quick_navigation_.set_landmark(key, landmark_view);
}

/**
 * Hide the minimap if it is currently shown; otherwise, do nothing.
 */
void InteractiveBase::hide_minimap() {
	minimap_registry_.destroy();
}

/**
===========
InteractiveBase::minimap_registry()

Exposes the Registry object of the minimap to derived classes
===========
*/
MiniMap::Registry& InteractiveBase::minimap_registry() {
	return minimap_registry_;
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

	set_sel_picture(g_gr->images().get("images/ui_basic/fsel_roadbuilding.png"));
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

	unset_sel_picture();
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

	unset_sel_picture();
}

/*
===============
If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool InteractiveBase::append_build_road(Coords const field) {
	assert(buildroad_);

	const Map& map = egbase().map();
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

/**
 * Plays a sound effect positioned according to the map coordinates in the note.
 */
void InteractiveBase::play_sound_effect(const NoteSound& note) const {
	if (!g_sh->is_sound_enabled(note.type)) {
		return;
	}

	if (note.coords != Widelands::Coords::null() && player_hears_field(note.coords)) {
		constexpr int kSoundMaxDistance = 255;
		constexpr float kSoundDistanceDivisor = 4.f;

		// Viewpoint is the point of the map in pixel which is shown in the upper
		// left corner of window or fullscreen
		const MapView::ViewArea area = map_view_.view_area();
		const Vector2f position_pix = area.find_pixel_for_coordinates(note.coords);
		const int stereo_pos =
		   static_cast<int>((position_pix.x - area.rect().x) * kStereoRight / area.rect().w);

		int distance = MapviewPixelFunctions::calc_pix_distance(
		                  egbase().map(), area.rect().center(), position_pix) /
		               kSoundDistanceDivisor;

		distance = (note.priority == kFxPriorityAlwaysPlay) ?
		              (math::clamp(distance, 0, kSoundMaxDistance) / 2) :
		              distance;

		if (distance < kSoundMaxDistance) {
			g_sh->play_fx(note.type, note.fx, note.priority,
			              math::clamp(stereo_pos, kStereoLeft, kStereoRight), distance);
		}
	}
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
	assert(road_building_overlays_.road_previews.empty());
	assert(road_building_overlays_.steepness_indicators.empty());

	const Map& map = egbase().map();

	// preview of the road
	const CoordPath::StepVector::size_type nr_steps = buildroad_->get_nsteps();
	for (CoordPath::StepVector::size_type idx = 0; idx < nr_steps; ++idx) {
		Widelands::Direction dir = (*buildroad_)[idx];
		Coords c = buildroad_->get_coords()[idx];

		if (dir < Widelands::WALK_E || dir > Widelands::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}
		int32_t const shift = 2 * (dir - Widelands::WALK_E);
		road_building_overlays_.road_previews[c] |= (Widelands::RoadType::kNormal << shift);
	}

	// build hints
	Widelands::FCoords endpos = map.get_fcoords(buildroad_->get_end());

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
		road_building_overlays_.steepness_indicators[neighb] = g_gr->images().get(name);
	}
}

/*
===============
Remove road building data from road overlay
===============
*/
void InteractiveBase::roadb_remove_overlay() {
	assert(buildroad_);
	road_building_overlays_.road_previews.clear();
	road_building_overlays_.steepness_indicators.clear();
}

bool InteractiveBase::handle_key(bool const down, SDL_Keysym const code) {
	if (quick_navigation_.handle_key(down, code))
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

	return map_view_.handle_key(down, code);
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
