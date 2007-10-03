/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __S__INTBASE_H
#define __S__INTBASE_H

#include "editor_game_base.h"
#include "geometry.h"
#include "map.h"
#include "mapview.h"
#include "overlay_manager.h"

#include <stdint.h>
#include "ui_panel.h"
#include "ui_unique_window.h"

class CoordPath;
class MiniMap;

/**
 * This is used to represent the code that Interactive_Player and
 * Editor_Interactive share.
 */
struct Interactive_Base : public Map_View {
   friend class Sound_Handler;

		enum {
			dfShowCensus     = 1, //  show census report on buildings
			dfShowStatistics = 2, //  show statistics report on buildings
			dfDebug          = 4, //  general debugging info
		};

	Interactive_Base(Editor_Game_Base &);
		virtual ~Interactive_Base();

	const Editor_Game_Base & egbase() const throw () {return m_egbase;}
	Editor_Game_Base       & egbase()       throw () {return m_egbase;}
	virtual void reference_player_tribe(const int32_t, const void * const) {}

		static int32_t get_xres();
		static int32_t get_yres();
	bool m_show_workarea_preview;

	//  point of view for drawing
	virtual Player * get_player() const throw () = 0;

      // logic handler func
      void think();
	virtual void postload();

	const Node_and_Triangle<> & get_sel_pos() const {return m_sel.pos;}
	bool get_sel_freeze() const {return m_sel.freeze;}

	/**
	 * sel_triangles determines whether the mouse pointer selects triangles.
	 * (False meas that it selects nodes.)
	 */
	bool get_sel_triangles() const throw () {return m_sel.triangles;}
	void set_sel_triangles(const bool yes) throw () {m_sel.triangles = yes;}

	uint32_t get_sel_radius() const throw () {return m_sel.radius;}
	virtual void set_sel_pos(const Node_and_Triangle<>);
	void set_sel_freeze(const bool yes) throw () {m_sel.freeze = yes;}
	void set_sel_radius(const uint32_t n);

	void move_view_to(const Coords);
		void move_view_to_point(Point pos);

      virtual void start() = 0;

      // Display flags
		uint32_t get_display_flags();
		void set_display_flags(uint32_t flags);
		bool get_display_flag(uint32_t flag);
		void set_display_flag(uint32_t flag, bool on);

      // Road building
		inline bool is_building_road() const {return m_buildroad;}
		inline CoordPath *get_build_road() {return m_buildroad;}
		void start_build_road(Coords start, int32_t player);
		void abort_build_road();
		void finish_build_road();
		bool append_build_road(Coords field);
	Coords    get_build_road_start  () const throw ();
	Coords    get_build_road_end    () const throw ();
	Direction get_build_road_end_dir() const throw ();

      // for loading
      virtual void cleanup_for_load() {};

private:
      void roadb_add_overlay();
      void roadb_remove_overlay();

      MiniMap* m_mm;
	Editor_Game_Base & m_egbase;
	struct Sel_Data {
		Sel_Data
			(const bool Freeze = false, const bool Triangles = false,
			 const Node_and_Triangle<> Pos       =
			 Node_and_Triangle<>
			 (Coords(0, 0), TCoords<>(Coords(0, 0), TCoords<>::D)),
			 const uint32_t Radius                   = 0,
			 const int32_t Pic                       = 0,
			 const Overlay_Manager::Job_Id Jobid = Overlay_Manager::Job_Id::Null())
			:
			freeze(Freeze), triangles(Triangles), pos(Pos), radius(Radius),
			pic(Pic), jobid(Jobid)
		{}
		bool              freeze; // don't change m_sel even if mouse moves
		bool              triangles; //  otherwise nodes
		Node_and_Triangle<>     pos;
		uint32_t              radius;
		int32_t               pic;
		Overlay_Manager::Job_Id jobid;
	} m_sel;

	uint32_t m_display_flags;

	uint32_t                    m_lastframe;         //  system time (milliseconds)
	uint32_t                    m_frametime;         //  in millseconds
	uint32_t                    m_avg_usframetime;   //  in microseconds!

	Overlay_Manager::Job_Id m_jobid;
	Overlay_Manager::Job_Id m_road_buildhelp_overlay_jobid;
	CoordPath             * m_buildroad;         //  path for the new road
      int32_t      m_road_build_player;

      UI::UniqueWindow::Registry m_minimap;

protected:
	void toggle_minimap();
	void hide_minimap();

      void mainview_move(int32_t x, int32_t y);
		void minimap_warp(int32_t x, int32_t y);

	virtual void draw_overlay(RenderTarget &);

      void unset_sel_picture();
      void set_sel_picture(const char * const);
};


#endif // __S__INTPLAYER_H
