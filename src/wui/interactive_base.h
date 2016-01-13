/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include <memory>

#include <SDL_keycode.h>

#include "graphic/graphic.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "notifications/notifications.h"
#include "profile/profile.h"
#include "ui_basic/box.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/chatoverlay.h"
#include "wui/debugconsole.h"
#include "wui/edge_overlay_manager.h"
#include "wui/field_overlay_manager.h"
#include "wui/mapview.h"

namespace Widelands {struct CoordPath;}

class EdgeOverlayManager;
class UniqueWindowHandler;
struct InteractiveBaseInternals;

/**
 * This is used to represent the code that InteractivePlayer and
 * EditorInteractive share.
 */
class InteractiveBase : public MapView, public DebugConsole::Handler {
public:
	friend class SoundHandler;

	enum {
		dfShowCensus     = 1, ///< show census report on buildings
		dfShowStatistics = 2, ///< show statistics report on buildings
		dfDebug          = 4, ///< general debugging info
	};

	// Manages all UniqueWindows.
	UniqueWindowHandler& unique_windows();

	InteractiveBase(Widelands::EditorGameBase &, Section & global_s);
	virtual ~InteractiveBase();

	Widelands::EditorGameBase & egbase() const {return m_egbase;}
	virtual void reference_player_tribe(Widelands::PlayerNumber, const void * const) {}

	bool m_show_workarea_preview;
	FieldOverlayManager::OverlayId show_work_area(const WorkareaInfo& workarea_info,
	                                              Widelands::Coords coords);
	void hide_work_area(FieldOverlayManager::OverlayId overlay_id);

	//  point of view for drawing
	virtual Widelands::Player * get_player() const = 0;

	void think() override;
	virtual void postload();

	const Widelands::NodeAndTriangle<> & get_sel_pos() const {
		return m_sel.pos;
	}
	bool get_sel_freeze() const {return m_sel.freeze;}

	// Returns true if the buildhelp is currently displayed.
	bool buildhelp() const;

	// Sets if the buildhelp should be displayed. Will also call on_buildhelp_changed().
	void show_buildhelp(bool t);

	/**
	 * sel_triangles determines whether the mouse pointer selects triangles.
	 * (False meas that it selects nodes.)
	 */
	bool get_sel_triangles() const {return m_sel.triangles;}
	void set_sel_triangles(const bool yes) {m_sel.triangles = yes;}

	uint32_t get_sel_radius() const {return m_sel.radius;}
	virtual void set_sel_pos(Widelands::NodeAndTriangle<>);
	void set_sel_freeze(const bool yes) {m_sel.freeze = yes;}
	void set_sel_radius(uint32_t);

	void move_view_to(Widelands::Coords);
	void move_view_to_point(Point pos);

	//  display flags
	uint32_t get_display_flags() const;
	void set_display_flags(uint32_t flags);
	bool get_display_flag(uint32_t flag);
	void set_display_flag(uint32_t flag, bool on);

	//  road building
	bool is_building_road() const {return m_buildroad;}
	Widelands::CoordPath * get_build_road() {return m_buildroad;}
	void start_build_road
		(Widelands::Coords start, Widelands::PlayerNumber player);
		void abort_build_road();
		void finish_build_road();
		bool append_build_road(Widelands::Coords field);
	Widelands::Coords    get_build_road_start  () const;
	Widelands::Coords    get_build_road_end    () const;

	virtual void cleanup_for_load() {}

	/**
	 * Log a message to be displayed on screen
	 */
	void log_message(const std::string& message) const;
	void log_message(const char* message) const {
		log_message(std::string(message));
	}

	const FieldOverlayManager& field_overlay_manager() const {
		return *m_field_overlay_manager;
	}
	FieldOverlayManager* mutable_field_overlay_manager() {
		return m_field_overlay_manager.get();
	}

	const EdgeOverlayManager& edge_overlay_manager() const {
		return *m_edge_overlay_manager;
	}

	void toggle_minimap();

protected:
	// Will be called whenever the buildhelp is changed with the new 'value'.
	virtual void on_buildhelp_changed(bool value);

	void toggle_buildhelp();
	void hide_minimap();

	UI::UniqueWindow::Registry & minimap_registry();

	void mainview_move(int32_t x, int32_t y);
	void minimap_warp(int32_t x, int32_t y);

	void draw_overlay(RenderTarget &) override;
	bool handle_key(bool down, SDL_Keysym) override;

	void unset_sel_picture();
	void set_sel_picture(const char * const);
	void adjust_toolbar_position() {
		m_toolbar.set_pos
			(Point((get_inner_w() - m_toolbar.get_w()) >> 1, get_inner_h() - 34));
	}

	// TODO(sirver): why are these protected?
	ChatOverlay     * m_chatOverlay;
	UI::Box           m_toolbar;

private:
	void roadb_add_overlay   ();
	void roadb_remove_overlay();
	void cmd_map_object(const std::vector<std::string> & args);
	void cmd_lua(const std::vector<std::string> & args);

	struct SelData {
		SelData
			(const bool Freeze = false, const bool Triangles = false,
			 const Widelands::NodeAndTriangle<> Pos       =
				Widelands::NodeAndTriangle<>
			 		(Widelands::Coords(0, 0),
			 		 Widelands::TCoords<>
			 		 	(Widelands::Coords(0, 0), Widelands::TCoords<>::D)),
			 const uint32_t Radius                   = 0,
			 const Image* Pic                     = nullptr,
			 const FieldOverlayManager::OverlayId Jobid = 0)
			:
			freeze(Freeze), triangles(Triangles), pos(Pos), radius(Radius),
			pic(Pic), jobid(Jobid)
		{}
		bool              freeze; // don't change m_sel even if mouse moves
		bool              triangles; //  otherwise nodes
		Widelands::NodeAndTriangle<>     pos;
		uint32_t              radius;
		const Image* pic;
		FieldOverlayManager::OverlayId jobid;
	} m_sel;

	std::unique_ptr<InteractiveBaseInternals> m;

	std::unique_ptr<FieldOverlayManager> m_field_overlay_manager;
	std::unique_ptr<EdgeOverlayManager> m_edge_overlay_manager;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
	Widelands::EditorGameBase & m_egbase;
	uint32_t m_display_flags;
	uint32_t          m_lastframe;         //  system time (milliseconds)
	uint32_t          m_frametime;         //  in millseconds
	uint32_t          m_avg_usframetime;   //  in microseconds!

	EdgeOverlayManager::OverlayId m_jobid;
	FieldOverlayManager::OverlayId m_road_buildhelp_overlay_jobid;
	Widelands::CoordPath  * m_buildroad;         //  path for the new road
	Widelands::PlayerNumber m_road_build_player;

	UI::UniqueWindow::Registry m_debugconsole;
	std::unique_ptr<UniqueWindowHandler> unique_window_handler_;
	std::vector<const Image*> m_workarea_pics;
};

#define PIC2 g_gr->images().get("pics/but2.png")
#define TOOLBAR_BUTTON_COMMON_PARAMETERS(name) \
    &m_toolbar, name, 0, 0, 34U, 34U, PIC2

#endif  // end of include guard: WL_WUI_INTERACTIVE_BASE_H
