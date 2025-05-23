/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/interactive_base.h"

#include <cstdlib>
#include <memory>

#include <SDL_timer.h>

#include "base/log.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/multithreading.h"
#include "base/string.h"
#include "base/time_string.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/render_queue.h"
#include "graphic/rendertarget.h"
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
#include "logic/playersmanager.h"
#include "logic/widelands_geometry.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "scripting/lua_interface.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/toolbar_setup.h"
#include "wlapplication_options.h"
#include "wui/attack_window.h"
#include "wui/building_statistics_menu.h"
#include "wui/buildingwindow.h"
#include "wui/constructionsitewindow.h"
#include "wui/dismantlesitewindow.h"
#include "wui/economy_options_window.h"
#include "wui/encyclopedia_window.h"
#include "wui/fleet_options_window.h"
#include "wui/game_chat_menu.h"
#include "wui/game_debug_ui.h"
#include "wui/game_diplomacy_menu.h"
#include "wui/game_message_menu.h"
#include "wui/game_objectives_menu.h"
#include "wui/info_panel.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/marketwindow.h"
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
	if ((caps & Widelands::BUILDCAPS_MINE) != 0) {
		return Widelands::Field::Buildhelp_Mine;
	}
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		if ((caps & Widelands::BUILDCAPS_PORT) != 0) {
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
	if ((caps & Widelands::BUILDCAPS_FLAG) != 0) {
		return Widelands::Field::Buildhelp_Flag;
	}
	return Widelands::Field::Buildhelp_None;
}

}  // namespace

InteractiveBase::InteractiveBase(EditorGameBase& the_egbase, Section& global_s, ChatProvider* c)
   : UI::Panel(nullptr,
               UI::PanelStyle::kWui,
               "interactive_base",
               0,
               0,
               g_gr->get_xres(),
               g_gr->get_yres()),
     chat_provider_(c),
     info_panel_(*new InfoPanel(*this)),
     map_view_(this, the_egbase.map(), 0, 0, g_gr->get_xres(), g_gr->get_yres()),
     // Initialize chatoverlay before the toolbar so it is below
     chat_overlay_(new ChatOverlay(this, color_functor(), 10, 25, get_w() / 2, get_h() - 25)),
     toolbar_(*new MainToolbar(info_panel_)),
     mapviewmenu_(toolbar(),
                  "dropdown_menu_mapview",
                  0,
                  0,
                  UI::main_toolbar_button_size(),
                  10,
                  UI::main_toolbar_button_size(),
                  /** TRANSLATORS: Title for the map view menu button in the game */
                  _("Map View"),
                  UI::DropdownType::kPictorialMenu,
                  UI::PanelStyle::kWui,
                  UI::ButtonStyle::kWuiPrimary,
                  [this](MapviewMenuEntry t) { mapview_menu_selected(t); }),
     plugins_dropdown_(toolbar(),
                       "dropdown_menu_plugins",
                       0,
                       0,
                       UI::main_toolbar_button_size(),
                       10,
                       UI::main_toolbar_button_size(),
                       /** TRANSLATORS: Title for the plugins menu button in the game */
                       _("Plugins"),
                       UI::DropdownType::kPictorialMenu,
                       UI::PanelStyle::kWui,
                       UI::ButtonStyle::kWuiPrimary),
     quick_navigation_(&map_view_),
     plugin_actions_(
        this, [this](const std::string& cmd) { egbase().lua().interpret_string(cmd); }),
     minimap_registry_(the_egbase.is_game()),
     workareas_cache_(nullptr),
     egbase_(the_egbase),
     display_flags_(get_config_int("display_flags", kDefaultDisplayFlags)),
     lastframe_(SDL_GetTicks()),
     unique_window_handler_(new UniqueWindowHandler()) {
	if (g_allow_script_console) {
		display_flags_ |= dfDebug;
	} else {
		display_flags_ &= ~dfDebug;
	}

	// Load the buildhelp icons.
	{
		BuildhelpOverlay* buildhelp_overlay = buildhelp_overlays_;
		constexpr const char* filenames[] = {
		   "images/wui/overlays/set_flag.png", "images/wui/overlays/small.png",
		   "images/wui/overlays/medium.png",   "images/wui/overlays/big.png",
		   "images/wui/overlays/mine.png",     "images/wui/overlays/port.png"};
		for (uint8_t scale_index = 0; scale_index < ImageCache::kScalesCount; ++scale_index) {
			const char* const* filename = filenames;

			//  Special case for flag, which has a different formula for hotspot_y.
			buildhelp_overlay->pic = g_image_cache->get(*filename, true, scale_index);
			if (buildhelp_overlay->pic != nullptr) {
				buildhelp_overlay->scale = ImageCache::kScales[scale_index].first;
				buildhelp_overlay->hotspot =
				   Vector2i(buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() - 1);
			}

			const BuildhelpOverlay* const buildhelp_overlays_end =
			   buildhelp_overlay + Widelands::Field::Buildhelp_None;
			for (;;) {  // The other buildhelp overlays.
				++buildhelp_overlay;
				++filename;
				if (buildhelp_overlay == buildhelp_overlays_end) {
					break;
				}

				buildhelp_overlay->pic = g_image_cache->get(*filename, true, scale_index);
				if (buildhelp_overlay->pic != nullptr) {
					buildhelp_overlay->scale = ImageCache::kScales[scale_index].first;
					buildhelp_overlay->hotspot = Vector2i(
					   buildhelp_overlay->pic->width() / 2, buildhelp_overlay->pic->height() / 2);
				}
			}
		}
	}

	resize_chat_overlay();
	info_panel_.move_to_top();

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged& message) {
		   set_size(message.new_width, message.new_height);
		   map_view_.set_size(message.new_width, message.new_height);
		   map_view_.pan_by(Vector2i((message.old_width - message.new_width) / 2,
		                             (message.old_height - message.new_height) / 2),
		                    MapView::Transition::Jump);
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
					   wanted_building_windows_.at(coords.hash())->warp_done = true;
				   }
			   }
		   } break;
		   default:
			   break;
		   }
	   });

	quicknav_registry_.open_window = [this]() {
		new QuickNavigationWindow(*this, quicknav_registry_);
	};

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

void InteractiveBase::add_plugin_menu() {
	plugins_dropdown_.set_image(g_image_cache->get("images/plugin.png"));
	toolbar()->add(&plugins_dropdown_);
	plugins_dropdown_.selected.connect(
	   [this] { plugin_actions_.plugin_action(plugins_dropdown_.get_selected(), true); });
}

void InteractiveBase::add_toolbar_plugin(const std::string& action,
                                         const std::string& icon,
                                         const std::string& label,
                                         const std::string& tt,
                                         const std::string& hotkey) {
	plugins_dropdown_.add(label, action, g_image_cache->get(icon), false, tt, hotkey);
	finalize_toolbar();
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
	                 shortcut_string_for(KeyboardShortcut::kCommonMinimap, false));

	if (egbase().is_game()) {
		/** TRANSLATORS: An entry in the game's map view menu */
		mapviewmenu_.add(quicknav_registry_.window != nullptr ? _("Hide Quick Navigation") :
		                                                        _("Show Quick Navigation"),
		                 MapviewMenuEntry::kQuicknav,
		                 g_image_cache->get("images/wui/menus/quicknav.png"), false, "",
		                 shortcut_string_for(KeyboardShortcut::kInGameQuicknavGUI, false));
	}

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Zoom +"), MapviewMenuEntry::kIncreaseZoom,
	                 g_image_cache->get("images/wui/menus/zoom_increase.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomIn, false));

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Reset zoom"), MapviewMenuEntry::kResetZoom,
	                 g_image_cache->get("images/wui/menus/zoom_reset.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomReset, false));

	/** TRANSLATORS: An entry in the game's map view menu */
	mapviewmenu_.add(_("Zoom –"), MapviewMenuEntry::kDecreaseZoom,
	                 g_image_cache->get("images/wui/menus/zoom_decrease.png"), false, "",
	                 shortcut_string_for(KeyboardShortcut::kCommonZoomOut, false));

	mapviewmenu_.select(last_selection);
}

void InteractiveBase::mapview_menu_selected(MapviewMenuEntry entry) {
	switch (entry) {
	case MapviewMenuEntry::kMinimap: {
		toggle_minimap();
		mapviewmenu_.toggle();
	} break;
	case MapviewMenuEntry::kQuicknav: {
		toggle_quicknav();
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
	default:
		NEVER_HERE();
	}
}

const InteractiveBase::BuildhelpOverlay*
InteractiveBase::get_buildhelp_overlay(const Widelands::NodeCaps caps, const float scale) const {
	const int buildhelp_overlay_index = caps_to_buildhelp(caps);
	if (buildhelp_overlay_index >= Widelands::Field::Buildhelp_None) {
		return nullptr;
	}

	// buildhelp_overlays_ is ordered from smallest to biggest scale
	const InteractiveBase::BuildhelpOverlay* result = nullptr;
	for (int s = ImageCache::kScalesCount - 1; s >= 0; --s) {
		const InteractiveBase::BuildhelpOverlay& overlay =
		   buildhelp_overlays_[Widelands::Field::Buildhelp_None * s + buildhelp_overlay_index];
		if (overlay.pic == nullptr) {
			continue;
		}

		if (result == nullptr) {
			result = &overlay;
			continue;
		}

		if (overlay.scale < scale) {
			break;
		}

		result = &overlay;
	}

	return result;
}

bool InteractiveBase::has_workarea_special_coords(const Widelands::Coords& coords) const {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	for (const auto& preview : workarea_previews_) {
		if (preview->special_coords.count(coords) > 0) {
			return true;
		}
	}
	return false;
}

bool InteractiveBase::has_workarea_preview(const Widelands::Coords& coords,
                                           const Widelands::Map* map) const {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	if (map == nullptr) {
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
	if (in_road_building_mode()) {
		append_build_road(sel_.pos.node, true);
	}
}

void InteractiveBase::finalize_toolbar() {
	plugins_dropdown_.set_visible(!plugins_dropdown_.empty());

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

	if ((display_flags & InteractiveBase::dfShowBuildings) != 0u) {
		info_to_draw = info_to_draw | InfoToDraw::kShowBuildings;
	}

	if (!show) {
		return info_to_draw;
	}

	if ((display_flags & InteractiveBase::dfShowCensus) != 0u) {
		info_to_draw = info_to_draw | InfoToDraw::kCensus;
	}
	if ((display_flags & InteractiveBase::dfShowStatistics) != 0u) {
		info_to_draw = info_to_draw | InfoToDraw::kStatistics;
	}
	if ((display_flags & InteractiveBase::dfShowSoldierLevels) != 0u) {
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
	   new UI::Button(&toolbar_.box, name, 0, 0, UI::main_toolbar_button_size(),
	                  UI::main_toolbar_button_size(), UI::ButtonStyle::kWuiPrimary,
	                  g_image_cache->get("images/" + image_basename + ".png"), tooltip_text);
	toolbar_.box.add(button);
	if (window != nullptr) {
		window->opened.connect([button] { button->set_perm_pressed(true); });
		window->closed.connect([button] { button->set_perm_pressed(false); });

		if (bind_default_toggle) {
			button->sigclicked.connect([window]() { window->toggle(); });
		}
	}
	return button;
}

InteractiveBase::RoadBuildingMode::PreviewPathMap
InteractiveBase::road_building_preview_overlays() const {
	if (road_building_mode_) {
		return road_building_mode_->overlay_road_previews;
	}
	return RoadBuildingMode::PreviewPathMap();
}
InteractiveBase::RoadBuildingMode::PreviewPathMap
InteractiveBase::road_building_preview_preview_overlays() const {
	if (road_building_mode_) {
		return road_building_mode_->overlay_roadpreview_previews;
	}
	return RoadBuildingMode::PreviewPathMap();
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
                                    const WorkareaPreview::ExtraDataMap& extra_data,
                                    const std::set<Widelands::Coords>& special_coords) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	workarea_previews_.insert(std::unique_ptr<WorkareaPreview>(
	   new WorkareaPreview(coords, &workarea_info, extra_data, special_coords)));
	workareas_cache_.reset(nullptr);
}

void InteractiveBase::show_workarea(const WorkareaInfo& workarea_info,
                                    Widelands::Coords coords,
                                    const std::set<Widelands::Coords>& special_coords) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	show_workarea(workarea_info, coords, WorkareaPreview::ExtraDataMap(), special_coords);
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
		// Should be impossible, checked by TribeDescr finalizing code
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
		if (it->get()->coords == coords && ((is_additional ^ it->get()->data.empty()) != 0)) {
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

void InteractiveBase::draw_road_building(RenderTarget* dst,
                                         FieldsToDraw::Field& field,
                                         const Time& gametime,
                                         float scale) const {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);
	if (road_building_mode_ == nullptr) {
		return;
	}

	const bool show_extra_info = (SDL_GetModState() & KMOD_CTRL) != 0;

	{
		const RoadBuildingMode::PreviewPathMap rpo1 = road_building_preview_overlays();
		const auto rinfo1 = rpo1.find(field.fcoords);
		std::set<uint8_t> dirs;
		if (rinfo1 != rpo1.end()) {
			dirs.insert(rinfo1->second.begin(), rinfo1->second.end());
		}
		if (show_extra_info) {
			const RoadBuildingMode::PreviewPathMap rpo2 = road_building_preview_preview_overlays();
			const auto rinfo2 = rpo2.find(field.fcoords);
			if (rinfo2 != rpo2.end()) {
				dirs.insert(rinfo2->second.begin(), rinfo2->second.end());
			}
		}
		for (uint8_t dir : dirs) {
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

	/* Check if a flag would be placed here. */
	if (!show_extra_info) {
		return;
	}
	if ((field.fcoords.field->nodecaps() & Widelands::BUILDCAPS_FLAG) == 0) {
		return;
	}

	auto check_draw_flag = [this, scale, &field, dst,
	                        &gametime](const std::vector<Widelands::Coords>& coords) {
		const Widelands::Map& map = egbase().map();
		const int ncoords = coords.size();
		bool last_is_flag = false;
		const bool start_to_end = (SDL_GetModState() & KMOD_SHIFT) != 0;
		for (int i = start_to_end ? 0 : (ncoords - 1); i >= 0 && i < ncoords;
		     i += (start_to_end ? 1 : -1)) {
			if (coords.at(i) == field.fcoords) {
				if ((i == 0 || i == ncoords - 1) || (!last_is_flag && i != 1 && i != ncoords - 2)) {
					constexpr float kOpacity = 0.5f;
					dst->blit_animation(field.rendertarget_pixel, field.fcoords, scale,
					                    field.owner->tribe().flag_animation(), gametime, nullptr,
					                    kOpacity);
				}
				return true;
			}
			last_is_flag =
			   (i == 0 || i == ncoords - 1) ||
			   (!last_is_flag && (map[coords.at(i)].nodecaps() & Widelands::BUILDCAPS_FLAG) != 0);
		}
		return false;
	};
	if (road_building_mode_->preview_path.has_value() &&
	    check_draw_flag(road_building_mode_->preview_path.value().get_coords())) {
		return;
	}
	check_draw_flag(road_building_mode_->path.get_coords());
}

void InteractiveBase::info_panel_fast_forward_message_queue() {
	info_panel_.fast_forward_message_queue();
}

// Called one per logic frame by the logic thread's main loop.
// This is where the actual game logic (and from it, the game
// controller's and the AI's `think()` functions) are called.
// Also updates the stats about the logic FPS and real gamespeed.
void InteractiveBase::game_logic_think() {
	static constexpr uint64_t kFilterTime = 1000;  // milliseconds for averaging

	previous_frame_realtime_ = last_frame_realtime_;
	previous_frame_gametime_ = last_frame_gametime_;
	last_frame_realtime_ = SDL_GetTicks();
	last_frame_gametime_ = egbase().get_gametime();

	if (previous_frame_gametime_ > last_frame_gametime_) {
		assert(!egbase().is_game());  // In the editor, time can run backwards sometimes.
		last_frame_gametime_ = previous_frame_gametime_;
	}

	const uint64_t realtime_step =
	   std::max<uint64_t>(last_frame_realtime_ - previous_frame_realtime_, 1);

	double cur_speed = 1000.0 * (last_frame_gametime_ - previous_frame_gametime_).get();
	cur_speed /= realtime_step;

	uint64_t prev_avg_weight;

	if (realtime_step > kFilterTime) {
		// No need for filtering when updates are slow
		prev_avg_weight = 0;
	} else if (last_frame_realtime_ - gamespeed_last_change_time_ < kFilterTime) {
		// Allow faster convergence after changing speed
		prev_avg_weight =
		   abs(avg_actual_gamespeed_ - cur_speed) > abs(last_target_gamespeed_ - cur_speed) ? 0 : 1;
	} else {
		prev_avg_weight = kFilterTime / realtime_step;
	}

	avg_actual_gamespeed_ =
	   ((avg_actual_gamespeed_ * prev_avg_weight) + (cur_speed * 1000)) / (prev_avg_weight + 1);

	if (egbase().is_game()) {
		const uint64_t new_target = game().game_controller()->real_speed();
		if (last_target_gamespeed_ != new_target) {
			last_target_gamespeed_ = new_target;
			gamespeed_last_change_time_ = last_frame_realtime_;
		}
	}

	egbase().think();  // Call game logic here. The game advances.
}

void InteractiveBase::think() {
	UI::Panel::think();

	if (in_road_building_mode()) {
		const size_t steps = get_build_road_path().get_nsteps();
		if ((SDL_GetModState() & KMOD_CTRL) != 0 && road_building_mode_->preview_path.has_value()) {
			const size_t preview_steps = road_building_mode_->preview_path.value().get_nsteps();
			if (in_road_building_mode(RoadBuildingType::kRoad)) {
				set_tooltip(format(_("Road length: %1$u (%2$u)"), steps, preview_steps));
			} else {
				set_tooltip(format(_("Waterway length: %1$u (%2$u) / %3$u"), steps, preview_steps,
				                   egbase().map().get_waterway_max_length()));
			}
		} else {
			if (in_road_building_mode(RoadBuildingType::kRoad)) {
				set_tooltip(format(_("Road length: %u"), steps));
			} else {
				set_tooltip(format(
				   _("Waterway length: %1$u / %2$u"), steps, egbase().map().get_waterway_max_length()));
			}
		}
	}

	for (auto it = wanted_building_windows_.begin(); it != wanted_building_windows_.end();) {
		if (!it->second->warp_done) {
			++it;
			continue;
		}

		UI::UniqueWindow* building_window = show_building_window(
		   Widelands::Coords::unhash(it->first), true, it->second->show_workarea);

		if (building_window != nullptr) {
			building_window->set_pos(it->second->window_position);
			if (it->second->minimize) {
				building_window->minimize();
			}
			building_window->set_pinned(it->second->pin);
		}

		it = wanted_building_windows_.erase(it);
	}

	plugin_actions_.think();
}

double InteractiveBase::average_fps() const {
	return 1000.0 * 1000.0 / avg_usframetime_;
}

uint64_t InteractiveBase::average_real_gamespeed() const {
	return avg_actual_gamespeed_ / 1000;
}

/*
===============
Draw debug overlay when appropriate.
===============
*/
void InteractiveBase::draw_overlay(RenderTarget& /* rt */) {
	// Timing
	uint32_t curframe = SDL_GetTicks();

	frametime_ = curframe - lastframe_;
	avg_usframetime_ = ((avg_usframetime_ * 15) + (frametime_ * 1000)) / 16;
	lastframe_ = curframe;

	// Node information
	std::string node_text;
	if (!egbase().is_game() || get_display_flag(dfDebug)) {
		// Blit node information in the editor, and in debug mode also for games
		const int32_t height = egbase().map()[sel_.pos.node].get_height();
		node_text = format("(%i, %i, %i)", sel_.pos.node.x, sel_.pos.node.y, height);
	}
	info_panel_.set_coords_string(node_text);

	// In-game clock and FPS
	info_panel_.set_time_string(
	   egbase().is_game() ? gametimestring(egbase().get_gametime().get(), true) : "");
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
	dst->blitrect_scale(
	   pixel_perfect_rect.cast<float>(), image, image->rect(), opacity, BlendMode::UseAlpha);
}

void InteractiveBase::blit_field_overlay(RenderTarget* dst,
                                         const FieldsToDraw::Field& field,
                                         const Image* image,
                                         const Vector2i& hotspot,
                                         float scale,
                                         float opacity) {
	if (field.obscured_by_slope) {
		return;
	}
	blit_overlay(dst, field.rendertarget_pixel.cast<int>(), image, hotspot, scale, opacity);
}

void InteractiveBase::draw_bridges(RenderTarget* dst,
                                   const FieldsToDraw::Field* f,
                                   const Time& gametime,
                                   float scale) const {
	if (f->obscured_by_slope) {
		return;
	}
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
	if (minimap_registry_.window == nullptr) {
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

// Open the quicknav GUI or close it if it's open
void InteractiveBase::toggle_quicknav() {
	quicknav_registry_.toggle();
	rebuild_mapview_menu();
}

/**
 * Hide the minimap if it is currently shown; otherwise, do nothing.
 */
void InteractiveBase::hide_minimap() {
	minimap_registry_.destroy();
}

void InteractiveBase::resize_minimap() const {
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
	return (display_flags_ & flag) != 0u;
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
				const bool pin = fr.unsigned_8() != 0u;
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
				case UI::Panel::SaveType::kDiplomacy:
					w = &GameDiplomacyMenu::load(fr, *this);
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
				case UI::Panel::SaveType::kAttackWindow:
					w = &AttackWindow::load(fr, *this, mol);
					break;
				case UI::Panel::SaveType::kQuicknav:
					w = &QuickNavigationWindow::load(fr, *this);
					break;
				case UI::Panel::SaveType::kFleetOptions:
					w = &FleetOptionsWindow::load(fr, *this, mol);
					break;
				default:
					throw Widelands::GameDataError(
					   "Invalid panel save type %u", static_cast<unsigned>(type));
				}

				if (w != nullptr) {
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
	for (UI::Panel* child = get_first_child(); child != nullptr; child = child->get_next_sibling()) {
		const UI::Panel::SaveType t = child->current_save_type();
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
	for (UI::Panel* child = get_first_child(); child != nullptr; child = child->get_next_sibling()) {
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
		WorkareaPreview::ExtraDataMap wa_data;
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
		show_workarea(
		   *road_building_mode_->work_area, road_start, wa_data, std::set<Widelands::Coords>());
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
		              "Refusing to finish waterway building: length is %" PRIuS " but limit is %u\n",
		              length, egbase().map().get_waterway_max_length());
	} else if (length != 0u) {
		upcast(Game, g, &egbase());

		// Build the path as requested
		if (g != nullptr) {
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

		if (allow_user_input() && ((SDL_GetModState() & KMOD_CTRL) != 0)) {
			//  place flags
			const Map& map = egbase().map();
			const std::vector<Coords>& c_vector = road_building_mode_->path.get_coords();
			std::vector<Coords>::const_iterator const first = c_vector.begin() + 2;
			std::vector<Coords>::const_iterator const last = c_vector.end() - 2;

			auto place_flag = [this, g](const Widelands::FCoords& coords) {
				if (g != nullptr) {
					g->send_player_build_flag(road_building_mode_->player, coords);
				} else {
					egbase().get_player(road_building_mode_->player)->build_flag(coords);
				}
			};

			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
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

std::optional<Widelands::CoordPath>
InteractiveBase::try_append_build_road(const Widelands::Coords field) const {
	assert(road_building_mode_);

	const Map& map = egbase().map();
	const Widelands::Player& player = egbase().player(road_building_mode_->player);
	Widelands::CoordPath result_path = road_building_mode_->path;

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
		if (map.findpath(result_path.get_end(), field, 0, path, cstep, Map::fpBidiCost) < 0) {
			return std::nullopt;  // could not find a path
		}
		result_path.append(map, path);
	}

	{
		//  Fix the road by finding an optimal path through the set of nodes
		//  currently used by the road. This will not claim any new nodes, so it
		//  is guaranteed to not hinder building placement.
		Widelands::Path path;
		{
			Widelands::CheckStepAnd cstep;
			Widelands::CheckStepLimited clim;
			for (const Coords& coord : result_path.get_coords()) {
				clim.add_allowed_location(coord);
			}
			cstep.add(clim);
			if (road_building_mode_->type == RoadBuildingType::kWaterway) {
				// Waterways (unlike roads) are strictly limited by the terrain around the edges
				cstep.add(Widelands::CheckStepFerry(egbase()));
			}
			map.findpath(result_path.get_start(), field, 0, path, cstep, Map::fpBidiCost);
		}
		result_path.truncate(0);
		result_path.append(map, path);
	}

	if (road_building_mode_->type == RoadBuildingType::kWaterway &&
	    result_path.get_nsteps() > map.get_waterway_max_length()) {
		result_path.truncate(map.get_waterway_max_length());
	}

	return std::optional<Widelands::CoordPath>(result_path);
}

/*
===============
If field is on the path, remove tail of path.
Otherwise append if possible or return false.
===============
*/
bool InteractiveBase::append_build_road(Coords const field, bool is_preview) {
	MutexLock m(MutexLock::ID::kIBaseVisualizations);

	road_building_mode_->preview_path = std::nullopt;
	std::optional<Widelands::CoordPath> path = try_append_build_road(field);

	if (!path.has_value()) {
		road_building_remove_overlay();
		road_building_add_overlay();
		return false;
	}

	if (is_preview) {
		if (road_building_mode_->path.get_coords() != path.value().get_coords()) {
			road_building_mode_->preview_path = path;
		}
	} else {
		road_building_mode_->path = path.value();
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

void InteractiveBase::log_message(const std::string& message, const std::string& tooltip) const {
	info_panel_.log_message(message, tooltip);
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
	chat_overlay_->set_size(get_w() / 2, get_h() - 25 - UI::main_toolbar_button_size());
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

	if (road_building_mode_->preview_path.has_value()) {
		road_building_add_overlay(road_building_mode_->preview_path.value(),
		                          road_building_mode_->overlay_roadpreview_previews, false);
	}
	road_building_add_overlay(
	   road_building_mode_->path, road_building_mode_->overlay_road_previews, true);
}

void InteractiveBase::road_building_add_overlay(const Widelands::CoordPath& path,
                                                RoadBuildingMode::PreviewPathMap& target,
                                                bool steepness) {
	assert(target.empty());
	const Map& map = egbase().map();

	// preview of the road
	const CoordPath::StepVector::size_type nr_steps = path.get_nsteps();
	for (CoordPath::StepVector::size_type idx = 0; idx < nr_steps; ++idx) {
		Widelands::Direction dir = (path)[idx];
		Coords c = path.get_coords()[idx];

		if (dir < Widelands::WALK_E || dir > Widelands::WALK_SW) {
			map.get_neighbour(c, dir, &c);
			dir = Widelands::get_reverse_dir(dir);
		}
		target.emplace(c, std::vector<uint8_t>());
		target[c].push_back(dir);
	}

	// build hints
	if (!steepness) {
		return;
	}
	assert(road_building_mode_->overlay_steepness_indicators.empty());
	Widelands::FCoords endpos = map.get_fcoords(path.get_end());

	for (int32_t dir = 1; dir <= 6; ++dir) {
		Widelands::FCoords neighb;
		int32_t caps;

		map.get_neighbour(endpos, dir, &neighb);
		caps = egbase().player(road_building_mode_->player).get_buildcaps(neighb);

		if (road_building_mode_->type == RoadBuildingType::kWaterway) {
			Widelands::CheckStepFerry checkstep(egbase());
			if (!checkstep.reachable_dest(map, neighb) || path.get_index(neighb) >= 0 ||
			    !neighb.field->is_interior(road_building_mode_->player)) {
				continue;
			}

			bool next_to = false;
			Widelands::FCoords nb;
			for (int32_t d = 1; d <= 6; ++d) {
				map.get_neighbour(neighb, d, &nb);
				if (nb != endpos && path.get_index(nb) >= 0 &&
				    checkstep.allowed(map, neighb, nb, d, Widelands::CheckStep::StepId::stepNormal)) {
					next_to = true;
					break;
				}
			}
			if (!next_to && path.get_nsteps() >= map.get_waterway_max_length()) {
				continue;  // exceeds length limit
			}
		} else if ((caps & Widelands::MOVECAPS_WALK) == 0) {
			continue;  // need to be able to walk there
		}

		//  can't build on robusts
		const Widelands::BaseImmovable* imm = map.get_immovable(neighb);
		if ((imm != nullptr) && imm->get_size() >= Widelands::BaseImmovable::SMALL &&
		    ((dynamic_cast<const Widelands::Flag*>(imm) == nullptr) &&
		     ((dynamic_cast<const Widelands::RoadBase*>(imm) == nullptr) ||
		      ((caps & Widelands::BUILDCAPS_FLAG) == 0)))) {
			continue;
		}
		if (path.get_index(neighb) >= 0) {
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
	road_building_mode_->overlay_roadpreview_previews.clear();
	road_building_mode_->overlay_steepness_indicators.clear();
}

void InteractiveBase::add_wanted_building_window(const Widelands::Coords& coords,
                                                 const Vector2i point,
                                                 bool was_minimal,
                                                 bool was_pinned) {
	wanted_building_windows_.insert(std::make_pair(
	   coords.hash(), std::unique_ptr<WantedBuildingWindow>(new WantedBuildingWindow(
	                     point, was_minimal, was_pinned, has_workarea_preview(coords)))));
}

UI::UniqueWindow* InteractiveBase::show_building_window(const Widelands::Coords& coord,
                                                        bool avoid_fastclick,
                                                        bool workarea_preview_wanted) {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::BaseImmovable* immovable = game().map().get_immovable(coord);
	upcast(Widelands::Building, building, immovable);
	if (building == nullptr) {
		return nullptr;  // Race condition
	}
	BuildingWindow::Registry& registry =
	   unique_windows().get_building_window_registry(format("building_%d", building->serial()));

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
	case Widelands::MapObjectType::MARKET:
		registry.open_window = [this, &registry, building, avoid_fastclick, workarea_preview_wanted] {
			new MarketWindow(*this, registry, *dynamic_cast<Widelands::Market*>(building),
			                 avoid_fastclick, workarea_preview_wanted);
		};
		break;
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
	   unique_windows().get_registry(format("ship_%d", ship->serial()));
	registry.open_window = [this, &registry, ship] { new ShipWindow(*this, registry, ship); };
	registry.create();
	return *registry.window;
}

ChatColorForPlayer InteractiveBase::color_functor() const {
	return [this](int player_number) -> const RGBColor* {
		if (player_number > 0 && player_number <= kMaxPlayers) {
			const Widelands::Player* player = egbase().get_player(player_number);
			if (player != nullptr) {
				return &player->get_playercolor();
			}
		}
		return nullptr;
	};
}

void InteractiveBase::broadcast_cheating_message(const std::string& code,
                                                 const std::string& arg2) const {
	if (get_game() == nullptr) {
		return;  // Editor
	}
	if (upcast(GameHost, h, game().game_controller())) {
		if (code == "CHEAT" && player_number() != 0u &&
		    h->get_local_playername() != game().player(player_number()).get_name()) {
			h->send_system_message_code(
			   "CHEAT_OTHER", h->get_local_playername(), game().player(player_number()).get_name());
		} else {
			h->send_system_message_code(code, h->get_local_playername(), arg2);
		}

		if (!g_allow_script_console) {
			// This shouldn't be possible
			h->force_pause();
		}
	} else if (upcast(GameClient, c, game().game_controller())) {
		if (code == "CHEAT" && player_number() != 0u &&
		    c->get_local_playername() != game().player(player_number()).get_name()) {
			c->send_cheating_info("CHEAT_OTHER", game().player(player_number()).get_name());
		} else {
			c->send_cheating_info(code, arg2);
		}

		if (!g_allow_script_console) {
			// This shouldn't be possible
			// TODO(tothxa): Should be handled more nicely, but what can a client do?
			throw wexception("Trying to cheat when the Script Console is disabled.");
		}
	}
}

bool InteractiveBase::handle_key(bool const down, SDL_Keysym const code) {
	if (plugin_actions_.check_keyboard_shortcut_action(code, down)) {
		return true;
	}

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

	if (matches_shortcut(KeyboardShortcut::kCommonChangeMusic, code)) {
		// request soundhandler to change music
		g_sh->change_music();
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
		if (egbase().is_game() && matches_shortcut(KeyboardShortcut::kInGameQuicknavGUI, code)) {
			toggle_quicknav();
			return true;
		}

		if (g_allow_script_console) {
			if (matches_shortcut(KeyboardShortcut::kCommonDebugConsole, code)) {
				GameChatMenu::create_script_console(
				   this, color_functor(), debugconsole_, *DebugConsole::get_chat_provider());
				return true;
			}
			if (matches_shortcut(KeyboardShortcut::kCommonCheatMode, code)) {
				if (cheat_mode_enabled_) {
					cheat_mode_enabled_ = false;
				} else if (!omnipotent()) {
					broadcast_cheating_message();
					cheat_mode_enabled_ = true;
				}
				return true;
			}
		}

		if (code.sym == SDLK_TAB) {
			toolbar()->focus();
			return true;
		}
	}

	if (map_view_.handle_key(down, code)) {
		return true;
	}
	return UI::Panel::handle_key(down, code);
}

void InteractiveBase::cmd_lua(const std::vector<std::string>& args) const {
	const std::string cmd = join(args, " ");

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
	if (!g_allow_script_console) {
		throw wexception("Trying to open map object info when the Script Console is disabled.");
	}

	if (args.size() != 2) {
		DebugConsole::write("usage: mapobject <mapobject serial>");
		return;
	}

	broadcast_cheating_message();

	uint32_t serial = stoul(args[1]);
	MapObject* obj = egbase().objects().get_object(serial);

	if (obj == nullptr) {
		DebugConsole::write(format("No MapObject with serial number %1%", serial));
		return;
	}

	show_mapobject_debug(*this, *obj);
}
