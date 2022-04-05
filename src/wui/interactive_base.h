/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_INTERACTIVE_BASE_H
#define WL_WUI_INTERACTIVE_BASE_H

#include <memory>

#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/path.h"
#include "sound/note_sound.h"
#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/unique_window.h"
#include "wui/chat_overlay.h"
#include "wui/debugconsole.h"
#include "wui/mapview.h"
#include "wui/minimap.h"
#include "wui/quicknavigation.h"

class InfoPanel;
class MainToolbar;
class UniqueWindowHandler;
namespace Widelands {
class MapObjectLoader;
}

struct WorkareaPreview {
	Widelands::Coords coords;
	const WorkareaInfo* info;
	std::map<Widelands::TCoords<>, uint32_t> data;
};

enum class RoadBuildingType { kRoad, kWaterway };

/**
 * This is used to represent the code that InteractivePlayer and
 * EditorInteractive share.
 */
class InteractiveBase : public UI::Panel, public DebugConsole::Handler {
public:
	// Available Display Flags
	// a new flag also needs its corresponding checkbox in options
	enum {
		dfShowCensus = 1,              ///< show census report on buildings
		dfShowStatistics = 2,          ///< show statistics report on buildings
		dfShowSoldierLevels = 4,       ///< show level information above soldiers
		dfShowWorkareaOverlap = 8,     ///< highlight overlapping workareas when placing
		                               //   a constructionsite
		dfDebug = 16,                  ///< general debugging info
		dfShowBuildings = 32,          ///<
		dfShowBuildhelp = 64,          ///< show size of building spaces
		dfShowMaximumBuildhelp = 128,  ///< show max size of building spaces
		dfShowGrid = 256,              ///<
		dfShowImmovables = 512,        ///< show trees, rocks etc.
		dfShowBobs = 1024,             ///< show animals
		dfShowResources = 2048,        ///< show water, coal etc. in editor
	};
	static constexpr int32_t kDefaultDisplayFlags =
	   dfShowSoldierLevels | dfShowBuildings | dfShowWorkareaOverlap;

	/// A build help overlay, i.e. small, big, mine, port ...
	struct BuildhelpOverlay {
		const Image* pic = nullptr;
		Vector2i hotspot = Vector2i::zero();
	};

	// Manages all UniqueWindows.
	UniqueWindowHandler& unique_windows();

	InteractiveBase(Widelands::EditorGameBase&, Section& global_s, ChatProvider*);
	~InteractiveBase() override;

	Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	void show_workarea(const WorkareaInfo& workarea_info, Widelands::Coords coords);
	void show_workarea(const WorkareaInfo& workarea_info,
	                   Widelands::Coords coords,
	                   std::map<Widelands::TCoords<>, uint32_t>& extra_data);
	void hide_workarea(const Widelands::Coords& coords, bool is_additional);

	//  point of view for drawing
	virtual Widelands::Player* get_player() const = 0;

	void game_logic_think() override;
	void think() override;
	double average_fps() const;
	bool handle_key(bool down, SDL_Keysym code) override;
	virtual void postload();

	void load_windows(FileRead&, Widelands::MapObjectLoader&);
	void save_windows(FileWrite&, Widelands::MapObjectSaver&);

	const Widelands::NodeAndTriangle<>& get_sel_pos() const {
		return sel_.pos;
	}

	// Returns true if the buildhelp is currently displayed.
	bool buildhelp() const;

	// Sets if the buildhelp should be displayed and then calls rebuild_showhide_menu
	void show_buildhelp(bool t);

	/**
	 * sel_triangles determines whether the mouse pointer selects triangles.
	 * (False meas that it selects nodes.)
	 */
	bool get_sel_triangles() const {
		return sel_.triangles;
	}
	void set_sel_triangles(const bool yes) {
		sel_.triangles = yes;
	}

	uint32_t get_sel_radius() const {
		return sel_.radius;
	}
	virtual void set_sel_pos(Widelands::NodeAndTriangle<>);
	void set_sel_freeze(const bool yes) {
		sel_.freeze = yes;
	}
	void set_sel_radius(uint32_t);

	//  display flags
	uint32_t get_display_flags() const;
	void set_display_flags(uint32_t flags);
	bool get_display_flag(uint32_t flag) const;
	void set_display_flag(uint32_t flag, bool on);

	//  road building
	bool in_road_building_mode() const {
		return road_building_mode_ != nullptr;
	}
	bool in_road_building_mode(RoadBuildingType t) const {
		return road_building_mode_ && (road_building_mode_->type == t);
	}
	void start_build_road(Widelands::Coords start, Widelands::PlayerNumber player, RoadBuildingType);
	void abort_build_road();
	void finish_build_road();
	bool append_build_road(Widelands::Coords field);
	Widelands::Coords get_build_road_start() const;
	Widelands::Coords get_build_road_end() const;
	Widelands::CoordPath get_build_road_path() const;

	virtual void cleanup_for_load();

	/**
	 * Log a message to be displayed on screen
	 */
	void log_message(const std::string& message) const;
	void log_message(const char* message) const {
		log_message(std::string(message));
	}

	void toggle_minimap();
	// Toggles the buildhelp and calls rebuild_showhide_menu
	void toggle_buildhelp();

	// Returns the list of landmarks that have been mapped to the keys 0-9
	const QuickNavigation::Landmark* landmarks();

	// Sets the landmark for the keyboard 'key' to 'point'
	void set_landmark(size_t key, const MapView::View& view);

	void add_wanted_building_window(const Widelands::Coords& coords,
	                                const Vector2i point,
	                                bool was_minimal,
	                                bool was_pinned);
	UI::UniqueWindow* show_building_window(const Widelands::Coords& coords,
	                                       bool avoid_fastclick,
	                                       bool workarea_preview_wanted);
	UI::UniqueWindow& show_ship_window(Widelands::Ship* ship);

	MapView* map_view() {
		return &map_view_;
	}

	void info_panel_fast_forward_message_queue();

	// This function should return true only in EditorInteractive
	virtual bool omnipotent() const {
		return cheat_mode_enabled_;
	}
	// These two functions should be overridden only by InteractiveGameBase
	virtual Widelands::Game* get_game() const {
		return nullptr;
	}
	virtual Widelands::Game& game() const {
		NEVER_HERE();
	}
	// These three functions should be overridden only by InteractivePlayer
	virtual bool can_see(Widelands::PlayerNumber) const {
		return true;
	}
	virtual bool can_act(Widelands::PlayerNumber) const {
		return omnipotent();
	}
	virtual Widelands::PlayerNumber player_number() const {
		return 0;
	}

	bool extended_tooltip_accessibility_mode() const override {
		return true;
	}

protected:
	// For referencing the items in mapviewmenu_
	enum class MapviewMenuEntry { kMinimap, kIncreaseZoom, kDecreaseZoom, kResetZoom };

	// Adds the mapviewmenu_ to the toolbar
	void add_mapview_menu(MiniMapType minimap_type);
	// Rebuilds the mapviewmenu_ according to current view settings
	void rebuild_mapview_menu();
	// Takes the appropriate action when an item in the mapviewmenu_ is selected
	void mapview_menu_selected(MapviewMenuEntry entry);

	/// Adds a toolbar button to the toolbar
	/// \param image_basename:      File path for button image starting from 'images' and without
	///                             file extension
	/// \param name:                Internal name of the button
	/// \param tooltip:             The button tooltip
	/// \param window:              The window that's associated with this button.
	/// \param bind_default_toggle: If true, the button will toggle with its 'window'.
	UI::Button* add_toolbar_button(const std::string& image_basename,
	                               const std::string& name,
	                               const std::string& tooltip_text,
	                               UI::UniqueWindow::Registry* window = nullptr,
	                               bool bind_default_toggle = false);

	void hide_minimap();
	void resize_minimap() const;

	void mainview_move();

	void draw_overlay(RenderTarget&) override;
	/**
	 * Will blit the 'image' on the given 'pos', offset by 'hotspot' and scaled according to the
	 * given zoom 'scale'.
	 * */
	void blit_overlay(RenderTarget* dst,
	                  const Vector2i& pos,
	                  const Image* image,
	                  const Vector2i& hotspot,
	                  float scale,
	                  float opacity);
	/**
	 * Will blit the 'image' on the given 'field', offset by 'hotspot' and scaled according to the
	 * given zoom 'scale'.
	 * */
	void blit_field_overlay(RenderTarget* dst,
	                        const FieldsToDraw::Field& field,
	                        const Image* image,
	                        const Vector2i& hotspot,
	                        float scale,
	                        float opacity = 1.f);

	void draw_bridges(RenderTarget* dst,
	                  const FieldsToDraw::Field* f,
	                  const Time& gametime,
	                  float scale) const;
	void draw_road_building(FieldsToDraw::Field&);

	void unset_sel_picture();
	void set_sel_picture(const Image* image);
	const Image* get_sel_picture() {
		return sel_.pic;
	}

	// Sets the toolbar's position to the bottom middle and configures its background images
	void finalize_toolbar();

	ChatOverlay* chat_overlay() {
		return chat_overlay_;
	}

	UI::Box* toolbar();

	// Returns the information which overlay text should currently be drawn.
	// Returns InfoToDraw::kNone if not 'show'
	InfoToDraw get_info_to_draw(bool show) const;

	// Returns the current overlays for the work area previews.
	Workareas get_workarea_overlays(const Widelands::Map& map);
	static WorkareasEntry get_workarea_overlay(const Widelands::Map&, const WorkareaPreview&);

	// Returns the 'BuildhelpOverlay' for 'caps' or nullptr if there is no help
	// to be displayed on this field.
	const BuildhelpOverlay* get_buildhelp_overlay(Widelands::NodeCaps caps) const;

	// Overlays displayed while a road or waterway is under construction.
	struct RoadBuildingMode {
		RoadBuildingMode(Widelands::PlayerNumber p, Widelands::Coords s, RoadBuildingType t)
		   : player(p), path(s), type(t), work_area(nullptr) {
		}
		const Widelands::PlayerNumber player;
		Widelands::CoordPath path;
		const RoadBuildingType type;
		std::unique_ptr<WorkareaInfo> work_area;
		std::map<Widelands::Coords, std::vector<uint8_t>> overlay_road_previews;
		std::map<Widelands::Coords, const Image*> overlay_steepness_indicators;
	};
	std::map<Widelands::Coords, std::vector<uint8_t>> road_building_preview_overlays() const;
	std::map<Widelands::Coords, const Image*> road_building_steepness_overlays() const;

	/// Returns true if there is a workarea preview being shown at the given coordinates.
	/// If 'map' is 0, checks only if the given coords are the center of a workarea;
	/// otherwise checks if the coords are within any workarea.
	bool has_workarea_preview(const Widelands::Coords& coords,
	                          const Widelands::Map* map = nullptr) const;

	/// Returns true if the current player is allowed to hear sounds from map objects on this field
	virtual bool player_hears_field(const Widelands::Coords& coords) const = 0;

	void set_toolbar_imageset(const ToolbarImageset& imageset);

	ChatProvider* chat_provider_;

	void broadcast_cheating_message() const;

#ifndef NDEBUG  //  only in debug builds
	UI::UniqueWindow::Registry debugconsole_;
#endif

	InfoPanel& info_panel_;

private:
	void play_sound_effect(const NoteSound& note) const;
	void resize_chat_overlay();
	void road_building_add_overlay();
	void road_building_remove_overlay();
	void cmd_map_object(const std::vector<std::string>& args);
	void cmd_lua(const std::vector<std::string>& args);

	// Rebuilds the subclass' showhidemenu_ according to current map settings
	virtual void rebuild_showhide_menu() = 0;

	struct SelData {
		SelData(const bool Freeze = false,
		        const bool Triangles = false,
		        const Widelands::NodeAndTriangle<>& Pos =
		           Widelands::NodeAndTriangle<>{
		              Widelands::Coords(0, 0),
		              Widelands::TCoords<>(Widelands::Coords(0, 0), Widelands::TriangleIndex::D)},
		        const uint32_t Radius = 0,
		        const Image* Pic = nullptr)
		   : freeze(Freeze), triangles(Triangles), pos(Pos), radius(Radius), pic(Pic) {
		}
		bool freeze;     // don't change sel, even if mouse moves
		bool triangles;  //  otherwise nodes
		Widelands::NodeAndTriangle<> pos;
		uint32_t radius;
		const Image* pic;
	} sel_;

	MapView map_view_;
	ChatOverlay* chat_overlay_;

	struct WantedBuildingWindow {
		explicit WantedBuildingWindow(const Vector2i& pos,
		                              bool was_minimized,
		                              bool was_pinned,
		                              bool was_showing_workarea)
		   : window_position(pos),
		     minimize(was_minimized),
		     pin(was_pinned),
		     show_workarea(was_showing_workarea) {
		}
		const Vector2i window_position;
		const bool minimize;
		const bool pin;
		const bool show_workarea;
	};

	// Building coordinates, window position, whether the window was minimized
	std::map<uint32_t, std::unique_ptr<const WantedBuildingWindow>> wanted_building_windows_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteBuilding>> buildingnotes_subscriber_;

	MainToolbar& toolbar_;

	// Map View menu on the toolbar
	UI::Dropdown<MapviewMenuEntry> mapviewmenu_;
	QuickNavigation quick_navigation_;

public:
	MiniMap::Registry minimap_registry_;

private:
	// The currently enabled work area previews
	std::unordered_set<std::unique_ptr<WorkareaPreview>> workarea_previews_;
	std::unique_ptr<Workareas> workareas_cache_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
	std::unique_ptr<Notifications::Subscriber<NoteSound>> sound_subscriber_;
	Widelands::EditorGameBase& egbase_;
	uint32_t display_flags_;
	uint32_t lastframe_;        //  system time (milliseconds)
	uint32_t frametime_;        //  in millseconds
	uint32_t avg_usframetime_;  //  in microseconds!

	// For measuring actual game speed and how smoothly the game logic runs
	uint32_t last_frame_realtime_, previous_frame_realtime_;
	Time last_frame_gametime_, previous_frame_gametime_;

	std::unique_ptr<RoadBuildingMode> road_building_mode_;

	std::unique_ptr<UniqueWindowHandler> unique_window_handler_;
	BuildhelpOverlay buildhelp_overlays_[Widelands::Field::Buildhelp_None];

	bool cheat_mode_enabled_;
	bool screenshot_failed_;
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_BASE_H
