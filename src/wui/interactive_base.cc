/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <SDL_timer.h>
#include <boost/algorithm/string.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/multithreading.h"
#include "base/time_string.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
#include "logic/cmd_queue.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/maptriangleregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "scripting/lua_interface.h"
#include "sound/sound_handler.h"
#include "wlapplication_options.h"
#include "wui/building_statistics_menu.h"
#include "wui/constructionsitewindow.h"
#include "wui/dismantlesitewindow.h"
#include "wui/economy_options_window.h"
#include "wui/encyclopedia_window.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/info_panel.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/militarysitewindow.h"
#include "wui/minimap.h"
#include "wui/seafaring_statistics_menu.h"
#include "wui/shipwindow.h"
#include "wui/soldier_statistics_menu.h"
#include "wui/stock_menu.h"
#include "wui/toolbar.h"
#include "wui/trainingsitewindow.h"
#include "wui/unique_window_handler.h"
#include "wui/ware_statistics_menu.h"
#include "wui/warehousewindow.h"
#include "wui/watchwindow.h"

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

InteractiveBase::InteractiveBase(EditorGameBase& the_egbase, Section& global_s, ChatProvider* c)
   : UI::Panel(nullptr, UI::PanelStyle::kWui, 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     chat_provider_(c),
     info_panel_(*new InfoPanel(*this)),
     map_view_(this, the_egbase.map(), 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     // Initialize chatoverlay before the toolbar so it is below
     chat_overlay_(new ChatOverlay(this, 10, 25, get_w() / 2, get_h() - 25)),
     toolbar_(*new MainToolbar(info_panel_)),
     mapviewmenu_(toolbar(),
                  "dropdown_menu_mapview",
                  0,
                  0,
                  MainToolbar::kButtonSize,
                  10,
                  MainToolbar::kButtonSize,
                  /** TRANSLATORS: Title for the map view menu button in the game */
                  _("Map View"),
                  UI::DropdownType::kPictorialMenu,
                  UI::PanelStyle::kWui,
                  UI::ButtonStyle::kWuiPrimary),
     quick_navigation_(&map_view_),
     workareas_cache_(nullptr),
     egbase_(the_egbase),
#ifndef NDEBUG  //  not in releases
     display_flags_(dfDebug | get_config_int("display_flags", kDefaultDisplayFlags)),
#else
     display_flags_(get_config_int("display_flags", kDefaultDisplayFlags)),
#endif
     lastframe_(SDL_GetTicks()),
     frametime_(0),
     avg_usframetime_(0),
     last_frame_realtime_(0),
     previous_frame_realtime_(0),
     last_frame_gametime_(0),
     previous_frame_gametime_(0),
     road_building_mode_(nullptr),
     unique_window_handler_(new UniqueWindowHandler()),
     cheat_mode_enabled_(false),
     screenshot_failed_(false) {

	// Load the buildhelp icons.
	{
		BuildhelpOverlay* buildhelp_overlay = buildhelp_overlays_;
		const char* filenames[] = {
		   "images/wui/overlays/set_flag.png", "images/wui/overlays/small.png",
		   "images/wui/overlays/medium.png",   "images/wui/overlays/big.png",
		   "images/wui/overlays/mine.png",     "images/wui/overlays/port.png"};
		const char* const* filename = filenames;

		//  Special case for flag, which has a different formula for hotspot_y.
		buildhelp_overlay->pic = g_image_cache->get(*filename);
		buildhelp_overlay->hotspot =
		   Vector2i(buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() - 1);

		const BuildhelpOverlay* const buildhelp_overlays_end =
		   buildhelp_overlay + Widelands::Field::Buildhelp_None;
		for (;;) {  // The other buildhelp overlays.
			++buildhelp_overlay;
			++filename;
			if (buildhelp_overlay == buildhelp_overlays_end) {
				break;
			}
			buildhelp_overlay->pic = g_image_cache->get(*filename);
			buildhelp_overlay->hotspot =
			   Vector2i(buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() / 2);
		}
	}

	resize_chat_overlay();
	info_panel_.move_to_top();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
		   map_view_.set_size(message.new_width, message.new_height);
		   resize_chat_overlay();
		   finalize_toolbar();
		   info_panel_.layout();
		   mainview_move();
	   });
	sound_subscriber_ = Notifications::subscribe<NoteSound>(
	   [this](const NoteSound& note) { play_sound_effect(note); });
	buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   switch (note.action) {
		   case Widelands::NoteBuilding::Action::kFinishWarp: {
			   if (upcast(
			          Widelands::Building const, building, game().objects().get_object(note.serial))) {
				   const Widelands::Coords coords = building->get_position();
				   // Check whether the window is wanted
				   if (wanted_building_windows_.count(coords.hash()) == 1) {
					   const WantedBuildingWindow& wanted_building_window =
					      *wanted_building_windows_.at(coords.hash());
					   UI::UniqueWindow* building_window =
					      show_building_window(coords, true, wanted_building_window.show_workarea);
					   building_window->set_pos(wanted_building_window.window_position);
					   if (wanted_building_window.minimize) {
						   building_window->minimize();
					   }
					   building_window->set_pinned(wanted_building_window.pin);
					   wanted_building_windows_.erase(coords.hash());
				   }
			   }
		   } break;
		   default:
			   break;
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

	setDefaultCommand([this](const std::vector<std::string>& str) { cmd_lua(str); });
	addCommand("mapobject", [this](const std::vector<std::string>& str) { cmd_map_object(str); });

	// Inform panel code that we have logic-related code
	set_logic_think();
}

InteractiveBase::~InteractiveBase() {
	if (road_building_mode_) {
		abort_build_road();
	}
	RenderQueue::instance().clear();  // Cleanup spurious drawing commands
}

UI::Box* InteractiveBase::toolbar() {
	return &toolbar_.box;
}

void InteractiveBase::add_mapview_menu(MiniMapType minimap_type) {
	mapviewmenu_.set_image(g_image_cache->get("images/wui/menus/toggle_minimap.png"));
	toolbar()->add(&mapviewmenu_);

	minimap_registry_.open_window = [this] { toggle_minimap(); };
	minimap_registry_.minimap_type = minimap_type;
	minimap_registry_.closed.connect([this] { rebuild_mapview_menu(); });

	rebuild_mapview_menu();
	mapviewmenu_.selected.connect([this] { mapview_menu_selected(mapviewmenu_.get_selected()); });
}

void InteractiveBase::rebuild_mapview_menu() {
	const MapviewMenuEntry last_selection =
	   mapviewmenu_.has_selection() ? mapviewmenu_.get_selected() : MapviewMenuEntry::kMinimap;

	mapviewmenu_.clear();

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(minimap_registry_.window != nullptr ? _("Hide Minimap") : _("Show Minimap"),
	                 MapviewMenuEntry::kMinimap,
	                 g_image_cache->get("images/wui/menus/toggle_minimap.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonMinimap));

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Zoom +"), MapviewMenuEntry::kIncreaseZoom,
	                 g_image_cache->get("images/wui/menus/zoom_increase.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomIn));

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Reset zoom"), MapviewMenuEntry::kResetZoom,
	                 g_image_cache->get("images/wui/menus/zoom_reset.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomReset));

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Zoom –"), MapviewMenuEntry::kDecreaseZoom,
	                 g_image_cache->get("images/wui/menus/zoom_decrease.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomOut));

	mapviewmenu_.select(last_selection);
}

void InteractiveBase::mapview_menu_selected(MapviewMenuEntry entry) {
	switch (entry) {
	case MapviewMenuEntry::kMinimap: {
		toggle_minimap();
		mapviewmenu_.toggle();
	} break;
	case MapviewMenuEntry::kDecreaseZoom: {
		map_view()->decrease_zoom();
		mapviewmenu_.toggle();
	} break;
	case MapviewMenuEntry::kIncreaseZoom: {
		map_view()->increase_zoom();
		mapviewmenu_.toggle();
	} break;

	case MapviewMenuEntry::kResetZoom: {
		map_view()->reset_zoom();
		mapviewmenu_.toggle();
	} break;
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
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	if (!map) {
		for (const auto& preview : workarea_previews_) {
			if (preview->coords == coords) {
				return true;
			}
		}
		return false;
	}
	for (const auto& preview : workarea_previews_) {
		uint32_t radius = 0;
		for (const auto& wa : *preview->info) {
			radius = std::max(radius, wa.first);
		}
		if (map->calc_distance(coords, preview->coords) <= radius) {
			return true;
		}
	}
	return false;
}

void InteractiveBase::set_toolbar_imageset(const ToolbarImageset& imageset) {
	toolbar_.change_imageset(imageset);
}

UniqueWindowHandler& InteractiveBase::unique_windows() {
	return *unique_window_handler_;
}

void InteractiveBase::set_sel_pos(Widelands::NodeAndTriangle<> const center) {
	sel_.pos = center;
}

void InteractiveBase::finalize_toolbar() {
	toolbar_.finalize();
	// prevent toolbar dropdowns from grabbing the Space button
	focus();
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

InfoToDraw InteractiveBase::get_info_to_draw(bool show) const {
	const auto display_flags = get_display_flags();
	InfoToDraw info_to_draw = InfoToDraw::kNone;

	if (display_flags & InteractiveBase::dfShowBuildings) {
		info_to_draw = info_to_draw | InfoToDraw::kShowBuildings;
	}

	if (!show) {
		return info_to_draw;
	}

	if (display_flags & InteractiveBase::dfShowCensus) {
		info_to_draw = info_to_draw | InfoToDraw::kCensus;
	}
	if (display_flags & InteractiveBase::dfShowStatistics) {
		info_to_draw = info_to_draw | InfoToDraw::kStatistics;
	}
	if (display_flags & InteractiveBase::dfShowSoldierLevels) {
		info_to_draw = info_to_draw | InfoToDraw::kSoldierLevels;
	}

	return info_to_draw;
}

void InteractiveBase::unset_sel_picture() {
	set_sel_picture(g_image_cache->get("images/ui_basic/fsel.png"));
	set_tooltip("");
}

bool InteractiveBase::buildhelp() const {
	return get_display_flag(dfShowBuildhelp);
}

void InteractiveBase::show_buildhelp(bool t) {
	set_display_flag(dfShowBuildhelp, t);
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
	   new UI::Button(&toolbar_.box, name, 0, 0, MainToolbar::kButtonSize, MainToolbar::kButtonSize,
	                  UI::ButtonStyle::kWuiPrimary,
	                  g_image_cache->get("images/" + image_basename + ".png"), tooltip_text);
	toolbar_.box.add(button);
	if (window) {
		window->opened.connect([button] { button->set_perm_pressed(true); });
		window->closed.connect([button] { button->set_perm_pressed(false); });

		if (bind_default_toggle) {
			button->sigclicked.connect([window]() { window->toggle(); });
		}
	}
	return button;
}

std::map<Widelands::Coords, std::vector<uint8_t>>
InteractiveBase::road_building_preview_overlays() const {
	if (road_building_mode_) {
		return road_building_mode_->overlay_road_previews;
	}
	return std::map<Widelands::Coords, std::vector<uint8_t>>();
}
std::map<Widelands::Coords, const Image*>
InteractiveBase::road_building_steepness_overlays() const {
	if (road_building_mode_) {
		return road_building_mode_->overlay_steepness_indicators;
	}
	return std::map<Widelands::Coords, const Image*>();
}

// Show the given workareas at the given coords
void InteractiveBase::show_workarea(const WorkareaInfo& workarea_info,
                                    Widelands::Coords coords,
                                    std::map<Widelands::TCoords<>, uint32_t>& extra_data) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	workarea_previews_.insert(
	   std::unique_ptr<WorkareaPreview>(new WorkareaPreview{coords, &workarea_info, extra_data}));
	workareas_cache_.reset(nullptr);
}

void InteractiveBase::show_workarea(const WorkareaInfo& workarea_info, Widelands::Coords coords) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	std::map<Widelands::TCoords<>, uint32_t> empty;
	show_workarea(workarea_info, coords, empty);
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
		if (outer) {
			if (inner) {
				return 0;
			}
			return 1;
		}
		if (inner) {
			return 3;
		}
		return 4;
	}
	if (outer) {
		assert(!inner);
		return 2;
	}
	assert(inner);
	return 5;
}

Workareas InteractiveBase::get_workarea_overlays(const Widelands::Map& map) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	if (!workareas_cache_) {
		workareas_cache_.reset(new Workareas());
		for (const auto& preview : workarea_previews_) {
			workareas_cache_->push_back(get_workarea_overlay(map, *preview));
		}
	}

	return Workareas(*workareas_cache_);
}

// static
WorkareasEntry InteractiveBase::get_workarea_overlay(const Widelands::Map& map,
                                                     const WorkareaPreview& workarea) {
	std::map<Coords, uint8_t> intermediate_result;
	const Coords& coords = workarea.coords;
	const WorkareaInfo* workarea_info = workarea.info;
	intermediate_result[coords] = 0;
	WorkareaInfo::size_type wa_index;
	const size_t workarea_size = workarea_info->size();
	switch (workarea_size) {
	case 0:
		return WorkareasEntry();  // no workarea
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

	WorkareasEntry result;
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
			TCoords<> tc(pair.first, Widelands::TriangleIndex::D);
			WorkareaPreviewData wd(tc, workarea_max(pair.second, brn->second, bln->second));
			for (const auto& p : workarea.data) {
				if (p.first == tc) {
					wd = WorkareaPreviewData(tc, wd.index, p.second);
					break;
				}
			}
			result.first.push_back(wd);
		}
		if (rn != intermediate_result.end()) {
			TCoords<> tc(pair.first, Widelands::TriangleIndex::R);
			WorkareaPreviewData wd(tc, workarea_max(pair.second, brn->second, rn->second));
			for (const auto& p : workarea.data) {
				if (p.first == tc) {
					wd = WorkareaPreviewData(tc, wd.index, p.second);
					break;
				}
			}
			result.first.push_back(wd);
		}
	}
	for (const auto& pair : *workarea_info) {
		std::vector<Coords> border;
		Coords c = coords;
		for (uint32_t i = pair.first; i > 0; --i) {
			map.get_tln(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_rn(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_brn(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_bln(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_ln(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_tln(c, &c);
		}
		for (uint32_t i = pair.first; i > 0; --i) {
			border.push_back(c);
			map.get_trn(c, &c);
		}
		result.second.push_back(border);
	}
	return result;
}

void InteractiveBase::hide_workarea(const Widelands::Coords& coords, bool is_additional) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	for (auto it = workarea_previews_.begin(); it != workarea_previews_.end(); ++it) {
		if (it->get()->coords == coords && (is_additional ^ it->get()->data.empty())) {
			workarea_previews_.erase(it);
			workareas_cache_.reset(nullptr);
			return;
		}
	}
}

/**
 * Called by \ref Game::postload at the end of the game loading
 * sequence.
 *
 * Default implementation does nothing.
 */
void InteractiveBase::postload() {
}

void InteractiveBase::draw_road_building(FieldsToDraw::Field& field) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	const auto rpo = road_building_preview_overlays();
	const auto rinfo = rpo.find(field.fcoords);
	if (rinfo != rpo.end()) {
		for (uint8_t dir : rinfo->second) {
			switch (dir) {
			case Widelands::WALK_E:
				field.road_e = in_road_building_mode(RoadBuildingType::kRoad) ?
                              Widelands::RoadSegment::kNormal :
                              Widelands::RoadSegment::kWaterway;
				break;
			case Widelands::WALK_SE:
				field.road_se = in_road_building_mode(RoadBuildingType::kRoad) ?
                               Widelands::RoadSegment::kNormal :
                               Widelands::RoadSegment::kWaterway;
				break;
			case Widelands::WALK_SW:
				field.road_sw = in_road_building_mode(RoadBuildingType::kRoad) ?
                               Widelands::RoadSegment::kNormal :
                               Widelands::RoadSegment::kWaterway;
				break;
			default:
				throw wexception("Attempt to set road-building overlay for invalid direction %i", dir);
			}
		}
	}
}

void InteractiveBase::info_panel_fast_forward_message_queue() {
	info_panel_.fast_forward_message_queue();
}

// Called one per logic frame by the logic thread's main loop.
// This is where the actual game logic (and from it, the game
// controller's and the AI's `think()` functions) are called.
// Also updates the stats about the logic FPS and real gamespeed.
void InteractiveBase::game_logic_think() {
	previous_frame_realtime_ = last_frame_realtime_;
	previous_frame_gametime_ = last_frame_gametime_;
	last_frame_realtime_ = SDL_GetTicks();
	last_frame_gametime_ = egbase().get_gametime();

	egbase().think();  // Call game logic here. The game advances.
}

void InteractiveBase::think() {
	UI::Panel::think();
	if (in_road_building_mode()) {
		if (in_road_building_mode(RoadBuildingType::kRoad)) {
			set_tooltip(
			   (boost::format(_("Road length: %u")) % get_build_road_path().get_nsteps()).str());
		} else {
			set_tooltip((boost::format(_("Waterway length: %1$u/%2$u")) %
			             get_build_road_path().get_nsteps() % egbase().map().get_waterway_max_length())
			               .str());
		}
	}
}

double InteractiveBase::average_fps() const {
	return 1000.0 * 1000.0 / avg_usframetime_;
}

/*
===============
Draw debug overlay when appropriate.
===============
*/
void InteractiveBase::draw_overlay(RenderTarget&) {
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
			const uint32_t cur_fps = average_fps();
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
	std::string node_text;
	if (game == nullptr || get_display_flag(dfDebug)) {
		// Blit node information in the editor, and in debug mode also for games
		boost::format node_format("(%i, %i, %i)");
		const int32_t height = egbase().map()[sel_.pos.node].get_height();
		node_text = (node_format % sel_.pos.node.x % sel_.pos.node.y % height).str();
	}
	info_panel_.set_coords_string(node_text);

	// In-game clock and FPS
	info_panel_.set_time_string(game ? gametimestring(egbase().get_gametime().get(), true) : "");
	info_panel_.set_fps_string(
	   get_display_flag(dfDebug), cheat_mode_enabled_, 1000.0 / frametime_, average_fps());
}

void InteractiveBase::blit_overlay(RenderTarget* dst,
                                   const Vector2i& position,
                                   const Image* image,
                                   const Vector2i& hotspot,
                                   float scale,
                                   float opacity) {
	const Recti pixel_perfect_rect =
	   Recti(position - hotspot * scale, image->width() * scale, image->height() * scale);
	dst->blitrect_scale(pixel_perfect_rect.cast<float>(), image,
	                    Recti(0, 0, image->width(), image->height()), opacity, BlendMode::UseAlpha);
}

void InteractiveBase::blit_field_overlay(RenderTarget* dst,
                                         const FieldsToDraw::Field& field,
                                         const Image* image,
                                         const Vector2i& hotspot,
                                         float scale,
                                         float opacity) {
	blit_overlay(dst, field.rendertarget_pixel.cast<int>(), image, hotspot, scale, opacity);
}

void InteractiveBase::draw_bridges(RenderTarget* dst,
                                   const FieldsToDraw::Field* f,
                                   const Time& gametime,
                                   float scale) const {
	if (Widelands::is_bridge_segment(f->road_e)) {
		dst->blit_animation(f->rendertarget_pixel, f->fcoords, scale,
		                    f->owner->tribe().bridge_animation(
		                       Widelands::WALK_E, f->road_e == Widelands::RoadSegment::kBridgeBusy),
		                    gametime, &f->owner->get_playercolor());
	}
	if (Widelands::is_bridge_segment(f->road_sw)) {
		dst->blit_animation(f->rendertarget_pixel, f->fcoords, scale,
		                    f->owner->tribe().bridge_animation(
		                       Widelands::WALK_SW, f->road_sw == Widelands::RoadSegment::kBridgeBusy),
		                    gametime, &f->owner->get_playercolor());
	}
	if (Widelands::is_bridge_segment(f->road_se)) {
		dst->blit_animation(f->rendertarget_pixel, f->fcoords, scale,
		                    f->owner->tribe().bridge_animation(
		                       Widelands::WALK_SE, f->road_se == Widelands::RoadSegment::kBridgeBusy),
		                    gametime, &f->owner->get_playercolor());
	}
}

void InteractiveBase::mainview_move() {
	if (MiniMap* const minimap = minimap_registry_.get_window()) {
		minimap->set_view(map_view_.view_area().rect());
	}
}

// Open the minimap or close it if it's open
void InteractiveBase::toggle_minimap() {
	if (!minimap_registry_.window) {
		// Don't store the MiniMap pointer, we can access it via 'minimap_registry_.get_window()'.
		// A MiniMap is a UniqueWindow, its parent will delete it.
		new MiniMap(*this, &minimap_registry_);
		minimap_registry_.get_window()->warpview.connect([this](const Vector2f& map_pixel) {
			map_view_.scroll_to_map_pixel(map_pixel, MapView::Transition::Smooth);
		});
		mainview_move();
	} else {
		delete minimap_registry_.window;
	}
	rebuild_mapview_menu();
}

const QuickNavigation::Landmark* InteractiveBase::landmarks() {
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

void InteractiveBase::resize_minimap() {
	if (MiniMap* const minimap = minimap_registry_.get_window()) {
		minimap->check_boundaries();
	}
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
	const uint32_t old_value = display_flags_;
	display_flags_ = flags;
	if (old_value != display_flags_) {
		rebuild_showhide_menu();
	}
}

/*
===============
Get and set one individual flag of the display flags.
===============
*/
bool InteractiveBase::get_display_flag(uint32_t const flag) const {
	return display_flags_ & flag;
}

void InteractiveBase::set_display_flag(uint32_t const flag, bool const on) {
	const uint32_t old_value = display_flags_;
	display_flags_ &= ~flag;

	if (on) {
		display_flags_ |= flag;
	}
	if (old_value != display_flags_) {
		NoteThreadSafeFunction::instantiate([this]() { rebuild_showhide_menu(); }, false);
	}
}

/*
===============
Saveloading support for open unique windows.
===============
*/
constexpr uint16_t kCurrentPacketVersionUniqueWindows = 1;

void InteractiveBase::load_windows(FileRead& fr, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionUniqueWindows) {
			for (;;) {
				const UI::Panel::SaveType type = static_cast<UI::Panel::SaveType>(fr.unsigned_8());
				if (type == UI::Panel::SaveType::kNone) {
					break;
				}

				const int32_t x = fr.signed_32();
				const int32_t y = fr.signed_32();
				const bool pin = fr.unsigned_8();
				UI::Window* w = nullptr;

				switch (type) {
				case UI::Panel::SaveType::kBuildingWindow:
					w = &BuildingWindow::load(fr, *this);
					break;
				case UI::Panel::SaveType::kShipWindow:
					w = &ShipWindow::load(fr, *this, mol);
					break;
				case UI::Panel::SaveType::kWatchWindow:
					w = &WatchWindow::load(fr, *this, mol);
					break;
				case UI::Panel::SaveType::kStockMenu:
					w = &StockMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kGeneralStats:
					w = &GeneralStatisticsMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kSoldierStats:
					w = &SoldierStatisticsMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kWareStats:
					w = &WareStatisticsMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kSeafaringStats:
					w = &SeafaringStatisticsMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kBuildingStats:
					w = &BuildingStatisticsMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kObjectives:
					w = &GameObjectivesMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kMessages:
					w = &GameMessageMenu::load(fr, *this);
					break;
				case UI::Panel::SaveType::kMinimap:
					w = &MiniMap::load(fr, *this);
					break;
				case UI::Panel::SaveType::kEncyclopedia:
					w = &UI::EncyclopediaWindow::load(fr, *this);
					break;
				case UI::Panel::SaveType::kConfigureEconomy:
					w = EconomyOptionsWindow::load(fr, *this, mol);
					break;
				default:
					throw Widelands::GameDataError(
					   "Invalid panel save type %u", static_cast<unsigned>(type));
				}

				if (w) {
					w->set_pinned(pin);
					w->set_pos(Vector2i(x, y));
					w->move_inside_parent();  // In case the game was loaded at a smaller resolution.
				}
			}
		} else {
			throw Widelands::UnhandledVersionError(
			   "Unique Windows", packet_version, kCurrentPacketVersionUniqueWindows);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("unique windows: %s", e.what());
	}
}

void InteractiveBase::save_windows(FileWrite& fw, Widelands::MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionUniqueWindows);
	for (UI::Panel* child = get_first_child(); child; child = child->get_next_sibling()) {
		const UI::Panel::SaveType t = child->save_type();
		if (t != UI::Panel::SaveType::kNone) {
			fw.unsigned_8(static_cast<uint8_t>(t));
			fw.signed_32(child->get_x());
			fw.signed_32(child->get_y());
			fw.unsigned_8(dynamic_cast<UI::Window&>(*child).is_pinned() ? 1 : 0);
			child->save(fw, mos);
		}
	}
	fw.unsigned_8(0);
}

void InteractiveBase::cleanup_for_load() {
	std::set<UI::Panel*> panels_to_kill;
	for (UI::Panel* child = get_first_child(); child; child = child->get_next_sibling()) {
		if (dynamic_cast<UI::Window*>(child) != nullptr) {
			panels_to_kill.insert(child);
		}
	}
	for (UI::Panel* p : panels_to_kill) {
		delete p;
	}
}

/*
===============
Begin building a road
===============
*/
void InteractiveBase::start_build_road(Coords road_start,
                                       Widelands::PlayerNumber const player,
                                       RoadBuildingType t) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	assert(!road_building_mode_);
	road_building_mode_.reset(new RoadBuildingMode(player, road_start, t));

	road_building_add_overlay();
	set_sel_picture(g_image_cache->get(t == RoadBuildingType::kWaterway ?
                                         "images/ui_basic/fsel_waterwaybuilding.png" :
                                         "images/ui_basic/fsel_roadbuilding.png"));

	if (t == RoadBuildingType::kWaterway) {
		// Show workarea to visualise length limit
		const Widelands::Map& map = egbase().map();
		const uint32_t len = map.get_waterway_max_length();
		assert(len > 1);
		road_building_mode_->work_area.reset(new WorkareaInfo());
		road_building_mode_->work_area->insert(std::make_pair(len, std::set<std::string>()));

		std::map<Widelands::FCoords, bool> reachable_nodes;
		Widelands::CheckStepFerry cstep(egbase());
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   map, Widelands::Area<Widelands::FCoords>(map.get_fcoords(road_start), len));
		do {
			reachable_nodes.insert(
			   std::make_pair(mr.location(), mr.location().field->get_owned_by() == player &&
			                                    !mr.location().field->is_border() &&
			                                    cstep.reachable_dest(map, mr.location())));
		} while (mr.advance(map));
		std::map<Widelands::TCoords<>, uint32_t> wa_data;
		for (const auto& pair : reachable_nodes) {
			const auto br = reachable_nodes.find(map.br_n(pair.first));
			if (br == reachable_nodes.end()) {
				continue;
			}
			auto it = reachable_nodes.find(map.bl_n(pair.first));
			if (it != reachable_nodes.end()) {
				wa_data.insert(
				   std::make_pair(Widelands::TCoords<>(pair.first, Widelands::TriangleIndex::D),
				                  pair.second && br->second && it->second ? 5 : 6));
			}
			it = reachable_nodes.find(map.r_n(pair.first));
			if (it != reachable_nodes.end()) {
				wa_data.insert(
				   std::make_pair(Widelands::TCoords<>(pair.first, Widelands::TriangleIndex::R),
				                  pair.second && br->second && it->second ? 5 : 6));
			}
		}
		show_workarea(*road_building_mode_->work_area, road_start, wa_data);
	}
}

/*
===============
Stop building the road
===============
*/
void InteractiveBase::abort_build_road() {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	assert(road_building_mode_);
	if (road_building_mode_->type == RoadBuildingType::kWaterway) {
		assert(road_building_mode_->work_area);
		hide_workarea(road_building_mode_->path.get_start(), true);
	}
#ifndef NDEBUG
	else {
		assert(!road_building_mode_->work_area);
	}
#endif

	road_building_remove_overlay();
	road_building_mode_.reset(nullptr);
	unset_sel_picture();
}

/*
===============
Finally build the road
===============
*/
void InteractiveBase::finish_build_road() {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	assert(road_building_mode_);

	if (road_building_mode_->type == RoadBuildingType::kWaterway) {
		assert(road_building_mode_->work_area);
		hide_workarea(road_building_mode_->path.get_start(), true);
	}
#ifndef NDEBUG
	else {
		assert(!road_building_mode_->work_area);
	}
#endif

	road_building_remove_overlay();

	const size_t length = road_building_mode_->path.get_nsteps();
	if (road_building_mode_->type == RoadBuildingType::kWaterway &&
	    length > egbase().map().get_waterway_max_length()) {
		log_warn_time(egbase().get_gametime(),
		              "Refusing to finish waterway building: length is %" PRIuS " but limit is %d\n",
		              length, egbase().map().get_waterway_max_length());
	} else if (length) {
		upcast(Game, g, &egbase());

		// Build the path as requested
		if (g) {
			if (road_building_mode_->type == RoadBuildingType::kWaterway) {
				g->send_player_build_waterway(
				   road_building_mode_->player, *new Widelands::Path(road_building_mode_->path));
			} else {
				g->send_player_build_road(
				   road_building_mode_->player, *new Widelands::Path(road_building_mode_->path));
			}
		} else {
			if (road_building_mode_->type == RoadBuildingType::kWaterway) {
				egbase()
				   .get_player(road_building_mode_->player)
				   ->build_waterway(*new Widelands::Path(road_building_mode_->path));
			} else {
				egbase()
				   .get_player(road_building_mode_->player)
				   ->build_road(*new Widelands::Path(road_building_mode_->path));
			}
		}

		if (allow_user_input() && (SDL_GetModState() & KMOD_CTRL)) {
			//  place flags
			const Map& map = egbase().map();
			const std::vector<Coords>& c_vector = road_building_mode_->path.get_coords();
			std::vector<Coords>::const_iterator const first = c_vector.begin() + 2;
			std::vector<Coords>::const_iterator const last = c_vector.end() - 2;

			auto place_flag = [this, g](const Widelands::FCoords& coords) {
				if (g) {
					g->send_player_build_flag(road_building_mode_->player, coords);
				} else {
					egbase().get_player(road_building_mode_->player)->build_flag(coords);
				}
			};

			if (SDL_GetModState() & KMOD_SHIFT) {
				//  start to end
				for (std::vector<Coords>::const_iterator it = first; it <= last; ++it) {
					place_flag(map.get_fcoords(*it));
				}
			} else {
				//  end to start
				for (std::vector<Coords>::const_iterator it = last; first <= it; --it) {
					place_flag(map.get_fcoords(*it));
				}
			}
		}
	}

	road_building_mode_.reset(nullptr);
	unset_sel_picture();
}

/*
===============
If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool InteractiveBase::append_build_road(Coords const field) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	assert(road_building_mode_);

	const Map& map = egbase().map();
	const Widelands::Player& player = egbase().player(road_building_mode_->player);

	{  //  find a path to the clicked-on node
		Widelands::Path path;
		Widelands::CheckStepAnd cstep;
		if (road_building_mode_->type == RoadBuildingType::kWaterway) {
			cstep.add(Widelands::CheckStepFerry(egbase()));
			cstep.add(
			   Widelands::CheckStepRoad(player, Widelands::MOVECAPS_SWIM | Widelands::MOVECAPS_WALK));
		} else {
			cstep.add(Widelands::CheckStepRoad(player, Widelands::MOVECAPS_WALK));
		}
		if (map.findpath(
		       road_building_mode_->path.get_end(), field, 0, path, cstep, Map::fpBidiCost) < 0) {
			return false;  // could not find a path
		}
		road_building_mode_->path.append(map, path);
	}

	{
		//  Fix the road by finding an optimal path through the set of nodes
		//  currently used by the road. This will not claim any new nodes, so it
		//  is guaranteed to not hinder building placement.
		Widelands::Path path;
		{
			Widelands::CheckStepAnd cstep;
			Widelands::CheckStepLimited clim;
			for (const Coords& coord : road_building_mode_->path.get_coords()) {
				clim.add_allowed_location(coord);
			}
			cstep.add(clim);
			if (road_building_mode_->type == RoadBuildingType::kWaterway) {
				// Waterways (unlike roads) are strictly limited by the terrain around the edges
				cstep.add(Widelands::CheckStepFerry(egbase()));
			}
			map.findpath(
			   road_building_mode_->path.get_start(), field, 0, path, cstep, Map::fpBidiCost);
		}
		road_building_mode_->path.truncate(0);
		road_building_mode_->path.append(map, path);
	}

	if (road_building_mode_->type == RoadBuildingType::kWaterway &&
	    road_building_mode_->path.get_nsteps() > map.get_waterway_max_length()) {
		road_building_mode_->path.truncate(map.get_waterway_max_length());
	}

	road_building_remove_overlay();
	road_building_add_overlay();

	return true;
}

/*
===============
Return the current road-building startpoint
===============
*/
Coords InteractiveBase::get_build_road_start() const {
	assert(road_building_mode_);
	return road_building_mode_->path.get_start();
}

/*
===============
Return the current road-building endpoint
===============
*/
Coords InteractiveBase::get_build_road_end() const {
	assert(road_building_mode_);
	return road_building_mode_->path.get_end();
}

Widelands::CoordPath InteractiveBase::get_build_road_path() const {
	assert(road_building_mode_);
	return road_building_mode_->path;
}

void InteractiveBase::log_message(const std::string& message) const {
	info_panel_.log_message(message);
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

		distance = (note.priority == kFxMaximumPriority) ?
                    (math::clamp(distance, 0, kSoundMaxDistance) / 2) :
                    distance;

		if (distance < kSoundMaxDistance) {
			g_sh->play_fx(note.type, note.fx, note.priority, note.allow_multiple,
			              math::clamp(stereo_pos, kStereoLeft, kStereoRight), distance);
		}
	}
}

// Repositions the chat overlay
void InteractiveBase::resize_chat_overlay() {
	chat_overlay_->set_size(get_w() / 2, get_h() - 25 - MainToolbar::kButtonSize);
	chat_overlay_->recompute();
}

/*
===============
Add road building data to the road overlay
===============
*/
void InteractiveBase::road_building_add_overlay() {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	assert(road_building_mode_);
	assert(road_building_mode_->overlay_road_previews.empty());
	assert(road_building_mode_->overlay_steepness_indicators.empty());

	const Map& map = egbase().map();

	// preview of the road
	const CoordPath::StepVector::size_type nr_steps = road_building_mode_->path.get_nsteps();
	for (CoordPath::StepVector::size_type idx = 0; idx < nr_steps; ++idx) {
		Widelands::Direction dir = (road_building_mode_->path)[idx];
		Coords c = road_building_mode_->path.get_coords()[idx];

		if (dir < Widelands::WALK_E || dir > Widelands::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}
		road_building_mode_->overlay_road_previews.emplace(c, std::vector<uint8_t>());
		road_building_mode_->overlay_road_previews[c].push_back(dir);
	}

	// build hints
	Widelands::FCoords endpos = map.get_fcoords(road_building_mode_->path.get_end());

	for (int32_t dir = 1; dir <= 6; ++dir) {
		Widelands::FCoords neighb;
		int32_t caps;

		map.get_neighbour(endpos, dir, &neighb);
		caps = egbase().player(road_building_mode_->player).get_buildcaps(neighb);

		if (road_building_mode_->type == RoadBuildingType::kWaterway) {
			Widelands::CheckStepFerry checkstep(egbase());
			if (!checkstep.reachable_dest(map, neighb) ||
			    road_building_mode_->path.get_index(neighb) >= 0 ||
			    !neighb.field->is_interior(road_building_mode_->player)) {
				continue;
			}

			bool next_to = false;
			Widelands::FCoords nb;
			for (int32_t d = 1; d <= 6; ++d) {
				map.get_neighbour(neighb, d, &nb);
				if (nb != endpos && road_building_mode_->path.get_index(nb) >= 0 &&
				    checkstep.allowed(map, neighb, nb, d, Widelands::CheckStep::StepId::stepNormal)) {
					next_to = true;
					break;
				}
			}
			if (!next_to && road_building_mode_->path.get_nsteps() >= map.get_waterway_max_length()) {
				continue;  // exceeds length limit
			}
		} else if (!(caps & Widelands::MOVECAPS_WALK)) {
			continue;  // need to be able to walk there
		}

		//  can't build on robusts
		const Widelands::BaseImmovable* imm = map.get_immovable(neighb);
		if (imm && imm->get_size() >= Widelands::BaseImmovable::SMALL &&
		    (!(dynamic_cast<const Widelands::Flag*>(imm) ||
		       (dynamic_cast<const Widelands::RoadBase*>(imm) &&
		        (caps & Widelands::BUILDCAPS_FLAG))))) {
			continue;
		}
		if (road_building_mode_->path.get_index(neighb) >= 0) {
			continue;  // the road can't cross itself
		}

		int32_t slope;

		if (Widelands::WALK_E == dir || Widelands::WALK_NE == dir || Widelands::WALK_SE == dir) {
			slope = neighb.field->get_height() - endpos.field->get_height();
		} else {
			slope = endpos.field->get_height() - neighb.field->get_height();
		}

		const char* name = nullptr;

		if (road_building_mode_->type == RoadBuildingType::kWaterway) {
			if (slope <= -4) {
				name = "images/wui/overlays/waterway_building_steepdown.png";
			} else if (slope >= 4) {
				name = "images/wui/overlays/waterway_building_steepup.png";
			} else if (slope <= -2) {
				name = "images/wui/overlays/waterway_building_down.png";
			} else if (slope >= 2) {
				name = "images/wui/overlays/waterway_building_up.png";
			} else {
				name = "images/wui/overlays/waterway_building_even.png";
			}
		} else {
			if (slope <= -4) {
				name = "images/wui/overlays/road_building_reddown.png";
			} else if (slope <= -2) {
				name = "images/wui/overlays/road_building_yellowdown.png";
			} else if (slope < 2) {
				name = "images/wui/overlays/road_building_green.png";
			} else if (slope < 4) {
				name = "images/wui/overlays/road_building_yellow.png";
			} else {
				name = "images/wui/overlays/road_building_red.png";
			}
		}
		road_building_mode_->overlay_steepness_indicators[neighb] = g_image_cache->get(name);
	}
}

/*
===============
Remove road building data from road overlay
===============
*/
void InteractiveBase::road_building_remove_overlay() {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	assert(road_building_mode_);
	road_building_mode_->overlay_road_previews.clear();
	road_building_mode_->overlay_steepness_indicators.clear();
}

void InteractiveBase::add_wanted_building_window(const Widelands::Coords& coords,
                                                 const Vector2i point,
                                                 bool was_minimal,
                                                 bool was_pinned) {
	wanted_building_windows_.insert(std::make_pair(
	   coords.hash(), std::unique_ptr<const WantedBuildingWindow>(new WantedBuildingWindow(
	                     point, was_minimal, was_pinned, has_workarea_preview(coords)))));
}

UI::UniqueWindow* InteractiveBase::show_building_window(const Widelands::Coords& coord,
                                                        bool avoid_fastclick,
                                                        bool workarea_preview_wanted) {
	Widelands::BaseImmovable* immovable = game().map().get_immovable(coord);
	upcast(Widelands::Building, building, immovable);
	assert(building);
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("building_%d") % building->serial()).str());

	switch (building->descr().type()) {
	case Widelands::MapObjectType::CONSTRUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ConstructionSiteWindow(*this, registry,
			                           *dynamic_cast<Widelands::ConstructionSite*>(building),
			                           avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::DISMANTLESITE:
		registry.open_window = [this, &registry, building, avoid_fastclick] {
			new DismantleSiteWindow(
			   *this, registry, *dynamic_cast<Widelands::DismantleSite*>(building), avoid_fastclick);
		};
		break;
	case Widelands::MapObjectType::MILITARYSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new MilitarySiteWindow(*this, registry, *dynamic_cast<Widelands::MilitarySite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::PRODUCTIONSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new ProductionSiteWindow(*this, registry,
			                         *dynamic_cast<Widelands::ProductionSite*>(building),
			                         avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::TRAININGSITE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new TrainingSiteWindow(*this, registry, *dynamic_cast<Widelands::TrainingSite*>(building),
			                       avoid_fastclick, workarea_preview_wanted);
		};
		break;
	case Widelands::MapObjectType::WAREHOUSE:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new WarehouseWindow(*this, registry, *dynamic_cast<Widelands::Warehouse*>(building),
			                    avoid_fastclick, workarea_preview_wanted);
		};
		break;
	// TODO(sirver,trading): Add UI for market.
	default:
		log_err_time(egbase().get_gametime(), "Unable to show window for building '%s', type '%s'.\n",
		             building->descr().name().c_str(), to_string(building->descr().type()).c_str());
		NEVER_HERE();
	}
	NoteThreadSafeFunction::instantiate([&registry]() { registry.create(); }, true);
	return registry.window;
}

UI::UniqueWindow& InteractiveBase::show_ship_window(Widelands::Ship* ship) {
	UI::UniqueWindow::Registry& registry =
	   unique_windows().get_registry((boost::format("ship_%d") % ship->serial()).str());
	registry.open_window = [this, &registry, ship] { new ShipWindow(*this, registry, ship); };
	registry.create();
	return *registry.window;
}

void InteractiveBase::broadcast_cheating_message() {
	if (!get_game()) {
		return;  // Editor
	}
	if (upcast(GameHost, h, game().game_controller())) {
		h->send_system_message_code(
		   "CHEAT", player_number() ? game().player(player_number()).get_name() : "");
	} else if (upcast(GameClient, c, game().game_controller())) {
		c->send_cheating_info();
	}
}

bool InteractiveBase::handle_key(bool const down, SDL_Keysym const code) {
	if (quick_navigation_.handle_key(down, code)) {
		return true;
	}

	if (matches_shortcut(KeyboardShortcut::kCommonScreenshot, code)) {
		// Screenshot taken by topmost handler, just show a notification
		if (down) {
			log_message(_("Failed saving screenshot!"));
			screenshot_failed_ = true;
		} else {
			if (!screenshot_failed_) {
				log_message(_("Screenshot saved"));
			}
			screenshot_failed_ = false;
		}
		return true;
	}

	if (down) {
		if (matches_shortcut(KeyboardShortcut::kCommonBuildhelp, code)) {
			toggle_buildhelp();
			return true;
		}
		if (matches_shortcut(KeyboardShortcut::kCommonMinimap, code)) {
			toggle_minimap();
			return true;
		}

		switch (code.sym) {
#ifndef NDEBUG  //  only in debug builds
		case SDLK_SPACE:
			if ((code.mod & KMOD_CTRL) && (code.mod & KMOD_SHIFT)) {
				GameChatMenu::create_script_console(
				   this, debugconsole_, *DebugConsole::get_chat_provider());
				return true;
			}
			break;
		case SDLK_BACKSPACE:
			if ((code.mod & KMOD_CTRL) && (code.mod & KMOD_SHIFT)) {
				if (cheat_mode_enabled_) {
					cheat_mode_enabled_ = false;
				} else if (!omnipotent()) {
					broadcast_cheating_message();
					cheat_mode_enabled_ = true;
				}
				return true;
			}
			break;
#endif
		case SDLK_TAB:
			toolbar()->focus();
			return true;
		default:
			break;
		}
	}

	if (map_view_.handle_key(down, code)) {
		return true;
	}
	return UI::Panel::handle_key(down, code);
}

void InteractiveBase::cmd_lua(const std::vector<std::string>& args) {
	const std::string cmd = boost::algorithm::join(args, " ");

	broadcast_cheating_message();

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

	uint32_t serial = boost::lexical_cast<uint32_t>(args[1]);
	MapObject* obj = egbase().objects().get_object(serial);

	if (!obj) {
		DebugConsole::write(str(boost::format("No MapObject with serial number %1%") % serial));
		return;
	}

	show_mapobject_debug(*this, *obj);
}
