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

#ifndef INTERACTIVE_BASE_H
#define INTERACTIVE_BASE_H

#include <SDL_keysym.h>

#include "debugconsole.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/notification.h"
#include "logmessage.h"
#include "wui/chatoverlay.h"
#include "wui/mapview.h"
#include "wui/overlay_manager.h"
#include "ui_basic/box.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

namespace Widelands {struct CoordPath;}

struct InteractiveBaseInternals;

/**
 * This is used to represent the code that Interactive_Player and
 * Editor_Interactive share.
 */
struct Interactive_Base : public Map_View, public DebugConsole::Handler {

	friend class Sound_Handler;

	enum {
		dfShowCensus     = 1, ///< show census report on buildings
		dfShowStatistics = 2, ///< show statistics report on buildings
		dfDebug          = 4, ///< general debugging info
		dfSpeed          = 8, ///< show game speed and speed controls
	};

	Interactive_Base(Widelands::Editor_Game_Base &, Section & global_s);
	virtual ~Interactive_Base();

	Widelands::Editor_Game_Base & egbase() const {return m_egbase;}
	virtual void reference_player_tribe(Widelands::Player_Number, const void * const) {}

	bool m_show_workarea_preview;
	Overlay_Manager::Job_Id show_work_area(const Workarea_Info & workarea_info, Widelands::Coords coords);
	void hide_work_area(Overlay_Manager::Job_Id job_id);

	//  point of view for drawing
	virtual Widelands::Player * get_player() const = 0;

	void think() override;
	virtual void postload();

	const Widelands::Node_and_Triangle<> & get_sel_pos() const {
		return m_sel.pos;
	}
	bool get_sel_freeze() const {return m_sel.freeze;}

	bool buildhelp();
	void show_buildhelp(bool t);
	void toggle_buildhelp ();

	/**
	 * sel_triangles determines whether the mouse pointer selects triangles.
	 * (False meas that it selects nodes.)
	 */
	bool get_sel_triangles() const {return m_sel.triangles;}
	void set_sel_triangles(const bool yes) {m_sel.triangles = yes;}

	uint32_t get_sel_radius() const {return m_sel.radius;}
	virtual void set_sel_pos(Widelands::Node_and_Triangle<>);
	void set_sel_freeze(const bool yes) {m_sel.freeze = yes;}
	void set_sel_radius(uint32_t);

	void move_view_to(Widelands::Coords);
	void move_view_to_point(Point pos);

	virtual void start() = 0;

	//  display flags
	uint32_t get_display_flags() const;
	void set_display_flags(uint32_t flags);
	bool get_display_flag(uint32_t flag);
	void set_display_flag(uint32_t flag, bool on);

	//  road building
	bool is_building_road() const {return m_buildroad;}
	Widelands::CoordPath * get_build_road() {return m_buildroad;}
	void start_build_road
		(Widelands::Coords start, Widelands::Player_Number player);
		void abort_build_road();
		void finish_build_road();
		bool append_build_road(Widelands::Coords field);
	Widelands::Coords    get_build_road_start  () const;
	Widelands::Coords    get_build_road_end    () const;

	virtual void cleanup_for_load() {};

	/**
	 * Log a message to be displayed on screen
	 */
	void log_message(const std::string& message) const;
	void log_message(const char* message) const {
		log_message(std::string(message));
	}
private:
	void roadb_add_overlay   ();
	void roadb_remove_overlay();

	std::unique_ptr<InteractiveBaseInternals> m;
	Widelands::Editor_Game_Base & m_egbase;
	struct Sel_Data {
		Sel_Data
			(const bool Freeze = false, const bool Triangles = false,
			 const Widelands::Node_and_Triangle<> Pos       =
			 	Widelands::Node_and_Triangle<>
			 		(Widelands::Coords(0, 0),
			 		 Widelands::TCoords<>
			 		 	(Widelands::Coords(0, 0), Widelands::TCoords<>::D)),
			 const uint32_t Radius                   = 0,
			 const Image* Pic                     = nullptr,
			 const Overlay_Manager::Job_Id Jobid = Overlay_Manager::Job_Id::Null())
			:
			freeze(Freeze), triangles(Triangles), pos(Pos), radius(Radius),
			pic(Pic), jobid(Jobid)
		{}
		bool              freeze; // don't change m_sel even if mouse moves
		bool              triangles; //  otherwise nodes
		Widelands::Node_and_Triangle<>     pos;
		uint32_t              radius;
		const Image* pic;
		Overlay_Manager::Job_Id jobid;
	} m_sel;

	uint32_t m_display_flags;

	uint32_t          m_lastframe;         //  system time (milliseconds)
	uint32_t          m_frametime;         //  in millseconds
	uint32_t          m_avg_usframetime;   //  in microseconds!

	Overlay_Manager::Job_Id m_jobid;
	Overlay_Manager::Job_Id m_road_buildhelp_overlay_jobid;
	Widelands::CoordPath  * m_buildroad;         //  path for the new road
	Widelands::Player_Number m_road_build_player;
	const Image* workarea_pics[NUMBER_OF_WORKAREA_PICS];

protected:
	void toggle_minimap();
	void hide_minimap();
	UI::UniqueWindow::Registry & minimap_registry();

	void mainview_move(int32_t x, int32_t y);
	void minimap_warp(int32_t x, int32_t y);

	virtual void draw_overlay(RenderTarget &) override;
	bool handle_key(bool down, SDL_keysym) override;

	void unset_sel_picture();
	void set_sel_picture(const char * const);
	void adjust_toolbar_position() {
		m_toolbar.set_pos
			(Point((get_inner_w() - m_toolbar.get_w()) >> 1, get_inner_h() - 34));
	}
	ChatOverlay     * m_chatOverlay;
	UI::Box           m_toolbar;


private:
	void cmdMapObject(const std::vector<std::string> & args);
	void cmdLua(const std::vector<std::string> & args);
	void update_speedlabel();

	UI::Textarea m_label_speed_shadow;
	UI::Textarea m_label_speed;

	UI::UniqueWindow::Registry m_debugconsole;
	Widelands::NoteSender<LogMessage> m_log_sender;
};

#define PIC2 g_gr->images().get("pics/but2.png")
#define TOOLBAR_BUTTON_COMMON_PARAMETERS(name) \
    &m_toolbar, name, 0, 0, 34U, 34U, PIC2

#endif
