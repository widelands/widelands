/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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
#include "ui_panel.h"
#include "ui_unique_window.h"

class CoordPath;
class Map_View;
class MiniMap;

/** class Interactive_Base
 *
 * This is the interactive base class. It is used
 * to represent the code that Interactive_Player and
 * Editor_Interactive share.
 */
class Interactive_Base : public UI::Panel {
   friend class Game_Interactive_Player_Data_Packet;
   friend class Sound_Handler;

   public:
		enum {
			dfShowCensus = 1,				// show census report on buildings
			dfShowStatistics = 2,		// show statistics report on buildings
			dfDebug = 4,					// general debugging info
		};

	public:
		Interactive_Base(Editor_Game_Base* g);
		virtual ~Interactive_Base(void);

      inline Map* get_map() { return m_egbase->get_map(); }
	const Map & map() const {return *m_egbase->get_map();}
	Map & map() {return *m_egbase->get_map();}
	virtual void reference_player_tribe(const int, const void * const) {}
		inline Editor_Game_Base* get_egbase() { return m_egbase; }
      void need_complete_redraw( void );

		static int get_xres();
		static int get_yres();
	bool m_show_workarea_preview;

      // get visibility, for drawing
      virtual std::vector<bool>* get_visibility(void) = 0;

      // logic handler func
      void think();

		inline const Coords &get_fieldsel_pos() const { return m_fsd.fieldsel_pos; }
		inline bool get_fieldsel_freeze() const { return m_fsd.fieldsel_freeze; }
      inline int get_fieldsel_radius(void) { return m_fsd.fieldsel_radius; }
		virtual void set_fieldsel_pos(Coords c);
		void set_fieldsel_freeze(bool yes);
      void set_fieldsel_radius(int n);

	void move_view_to(const Coords);
		void move_view_to_point(Point pos);
		void warp_mouse_to_field(Coords c);

      virtual void start() = 0;

      // Display flags
		uint get_display_flags();
		void set_display_flags(uint flags);
		bool get_display_flag(uint flag);
		void set_display_flag(uint flag, bool on);

      // Road building
		inline bool is_building_road() const { return m_buildroad; }
		inline CoordPath *get_build_road() { return m_buildroad; }
		void start_build_road(Coords start, int player);
		void abort_build_road();
		void finish_build_road();
		bool append_build_road(Coords field);
		const Coords &get_build_road_start();
		const Coords &get_build_road_end();
		int get_build_road_end_dir();

      // for loading
      virtual void cleanup_for_load() { };

   private:
      void roadb_add_overlay();
      void roadb_remove_overlay();

      Map_View* m_mapview;
      MiniMap* m_mm;
      Editor_Game_Base* m_egbase;
      struct FieldSel_Data {
         bool		         fieldsel_freeze; // don't change m_fieldsel even if mouse moves
         Coords            fieldsel_pos;
         int               fieldsel_radius;
         int               fieldsel_pic;
         int               fieldsel_jobid;
      } m_fsd;

		uint					m_display_flags;

		uint					m_lastframe;			// system time (milliseconds)
		uint					m_frametime;			// in millseconds
		uint					m_avg_usframetime;	// in microseconds!

      int      m_jobid;
      int      m_road_buildhelp_overlay_jobid;
      CoordPath		*m_buildroad; // path for the new road
      int      m_road_build_player;

      UI::UniqueWindow::Registry m_minimap;

   protected:
      void toggle_minimap(void);

      void mainview_move(int x, int y);
		void minimap_warp(int x, int y);

	   inline void set_mapview(Map_View* w) { m_mapview=w; }
      inline Map_View* get_mapview() { return m_mapview; }

		virtual void draw_overlay(RenderTarget* dst);

      void unset_fieldsel_picture();
      void set_fieldsel_picture(const char*);
};


#endif // __S__INTPLAYER_H
