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

#ifndef WL_WUI_INTERACTIVE_BASE_H
#define WL_WUI_INTERACTIVE_BASE_H

#include <map>
#include <memory>

#include <SDL_keycode.h>

#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "notifications/notifications.h"
#include "profile/profile.h"
#include "sound/note_sound.h"
#include "ui_basic/box.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/chat_overlay.h"
#include "wui/debugconsole.h"
#include "wui/mapview.h"
#include "wui/minimap.h"
#include "wui/quicknavigation.h"

namespace Widelands {
struct CoordPath;
}

class EdgeOverlayManager;
class UniqueWindowHandler;

/**
 * This is used to represent the code that InteractivePlayer and
 * EditorInteractive share.
 */
class InteractiveBase : public UI::Panel, public DebugConsole::Handler {
public:
	enum {
		dfShowCensus = 1,      ///< show census report on buildings
		dfShowStatistics = 2,  ///< show statistics report on buildings
		dfDebug = 4,           ///< general debugging info
	};

	// Overlays displayed while a road is under construction.
	struct RoadBuildingOverlays {
		// The roads that are displayed while a road is being built. They are not
		// yet logically in the game, but need to be displayed for the user as
		// visual guide. The data type is the same as for Field::road.
		std::map<Widelands::Coords, uint8_t> road_previews;
		std::map<Widelands::Coords, const Image*> steepness_indicators;
	};

	/// A build help overlay, i.e. small, big, mine, port ...
	struct BuildhelpOverlay {
		const Image* pic = nullptr;
		Vector2i hotspot = Vector2i::zero();
	};

	// Manages all UniqueWindows.
	UniqueWindowHandler& unique_windows();

	InteractiveBase(Widelands::EditorGameBase&, Section& global_s);
	~InteractiveBase() override;

	Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	void show_workarea(const WorkareaInfo& workarea_info, Widelands::Coords coords);
	void hide_workarea(const Widelands::Coords& coords);

	bool has_expedition_port_space(const Widelands::Coords&) const;
	std::map<Widelands::Ship*, Widelands::Coords>& get_expedition_port_spaces() {
		return expedition_port_spaces_;
	}

	//  point of view for drawing
	virtual Widelands::Player* get_player() const = 0;

	void think() override;
	bool handle_key(bool down, SDL_Keysym code) override;
	virtual void postload();

	const Widelands::NodeAndTriangle<>& get_sel_pos() const {
		return sel_.pos;
	}

	// Returns true if the buildhelp is currently displayed.
	bool buildhelp() const;

	// Sets if the buildhelp should be displayed. Will also call on_buildhelp_changed().
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
	bool get_display_flag(uint32_t flag);
	void set_display_flag(uint32_t flag, bool on);

	//  road building
	bool is_building_road() const {
		return buildroad_;
	}
	Widelands::CoordPath* get_build_road() {
		return buildroad_;
	}
	void start_build_road(Widelands::Coords start, Widelands::PlayerNumber player);
	void abort_build_road();
	void finish_build_road();
	bool append_build_road(Widelands::Coords field);
	Widelands::Coords get_build_road_start() const;
	Widelands::Coords get_build_road_end() const;

	virtual void cleanup_for_load() {
	}

	/**
	 * Log a message to be displayed on screen
	 */
	void log_message(const std::string& message) const;
	void log_message(const char* message) const {
		log_message(std::string(message));
	}

	void toggle_minimap();
	void toggle_buildhelp();

	// Returns the list of landmarks that have been mapped to the keys 0-9
	const std::vector<QuickNavigation::Landmark>& landmarks();

	// Sets the landmark for the keyboard 'key' to 'point'
	void set_landmark(size_t key, const MapView::View& view);

	MapView* map_view() {
		return &map_view_;
	}

protected:
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

	// Will be called whenever the buildhelp is changed with the new 'value'.
	virtual void on_buildhelp_changed(bool value);

	void hide_minimap();

	MiniMap::Registry& minimap_registry();

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
	                  float scale);
	/**
	 * Will blit the 'image' on the given 'field', offset by 'hotspot' and scaled according to the
	 * given zoom 'scale'.
	 * */
	void blit_field_overlay(RenderTarget* dst,
	                        const FieldsToDraw::Field& field,
	                        const Image* image,
	                        const Vector2i& hotspot,
	                        float scale);

	void unset_sel_picture();
	void set_sel_picture(const Image* image);
	const Image* get_sel_picture() {
		return sel_.pic;
	}
	void adjust_toolbar_position() {
		toolbar_.set_pos(Vector2i((get_inner_w() - toolbar_.get_w()) >> 1, get_inner_h() - 34));
	}

	ChatOverlay* chat_overlay() {
		return chat_overlay_;
	}

	UI::Box* toolbar() {
		return &toolbar_;
	}

	// Returns the information which overlay text should currently be drawn.
	TextToDraw get_text_to_draw() const;

	// Returns the current overlays for the work area previews.
	Workareas get_workarea_overlays(const Widelands::Map& map) const;

	// Returns the 'BuildhelpOverlay' for 'caps' or nullptr if there is no help
	// to be displayed on this field.
	const BuildhelpOverlay* get_buildhelp_overlay(Widelands::NodeCaps caps) const;

	const RoadBuildingOverlays& road_building_overlays() const {
		return road_building_overlays_;
	}

	/// Returns true if there is a workarea preview being shown at the given coordinates.
	/// If 'map' is 0, checks only if the given coords are the center of a workarea;
	/// otherwise checks if the coords are within any workarea.
	bool has_workarea_preview(const Widelands::Coords& coords,
	                          const Widelands::Map* map = nullptr) const;

	/// Returns true if the current player is allowed to hear sounds from map objects on this field
	virtual bool player_hears_field(const Widelands::Coords& coords) const = 0;

private:
	void play_sound_effect(const NoteSound& note) const;
	void resize_chat_overlay();
	void roadb_add_overlay();
	void roadb_remove_overlay();
	void cmd_map_object(const std::vector<std::string>& args);
	void cmd_lua(const std::vector<std::string>& args);

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

	bool buildhelp_;
	MapView map_view_;
	ChatOverlay* chat_overlay_;

	UI::Box toolbar_;
	// No unique_ptr on purpose: 'minimap_' is a UniqueWindow, its parent will
	// delete it.
	MiniMap* minimap_;
	MiniMap::Registry minimap_registry_;
	QuickNavigation quick_navigation_;

	// The currently enabled work area previews. They are keyed by the
	// coordinate that the building that shows the work area is positioned.
	std::map<Widelands::Coords, const WorkareaInfo*> workarea_previews_;

	std::map<Widelands::Ship*, Widelands::Coords> expedition_port_spaces_;

	RoadBuildingOverlays road_building_overlays_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
	std::unique_ptr<Notifications::Subscriber<NoteSound>> sound_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteShip>> shipnotes_subscriber_;
	Widelands::EditorGameBase& egbase_;
	uint32_t display_flags_;
	uint32_t lastframe_;        //  system time (milliseconds)
	uint32_t frametime_;        //  in millseconds
	uint32_t avg_usframetime_;  //  in microseconds!

	Widelands::CoordPath* buildroad_;  //  path for the new road
	Widelands::PlayerNumber road_build_player_;

	UI::UniqueWindow::Registry debugconsole_;
	std::unique_ptr<UniqueWindowHandler> unique_window_handler_;
	BuildhelpOverlay buildhelp_overlays_[Widelands::Field::Buildhelp_None];
};

#endif  // end of include guard: WL_WUI_INTERACTIVE_BASE_H
