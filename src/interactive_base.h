/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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


#include "ui.h"
#include "editor_game_base.h"

class Map;
class MiniMap;
class MiniMapView;
class Map_View;

/** class Interactive_Base
 *
 * This is the interactive base class. It is used
 * to represent the code that Interactive_Player and
 * Editor_Interactive share.
 */
class Interactive_Base : public Panel {
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
		inline Editor_Game_Base* get_egbase() { return m_egbase; }

		static int get_xres();
		static int get_yres();

      // logic handler func
      void think();

		inline const Coords &get_fieldsel() const { return m_maprenderinfo.fieldsel; }
		inline bool get_fieldsel_freeze() const { return m_fieldsel_freeze; }
		void set_fieldsel(Coords c);
		void set_fieldsel_freeze(bool yes);

		inline const MapRenderInfo* get_maprenderinfo() const { return &m_maprenderinfo; }

      void move_view_to(int fx, int fy);
		void move_view_to_point(Point pos);
		void warp_mouse_to_field(Coords c);

      virtual void recalc_overlay(FCoords fc) = 0;
      virtual void start() = 0;

      void set_fieldsel_radius(int n) { m_maprenderinfo.fieldsel_radius=n; }
      inline int get_fieldsel_radius(void) { return m_maprenderinfo.fieldsel_radius; }

		uint get_display_flags();
		void set_display_flags(uint flags);
		bool get_display_flag(uint flag);
		void set_display_flag(uint flag, bool on);

   private:
      Map_View* m_mapview;
      MiniMap* m_mm;
      Editor_Game_Base* m_egbase;
      bool		         m_fieldsel_freeze; // don't change m_fieldsel even if mouse moves
		uint					m_display_flags;

		uint					m_lastframe;			// system time (milliseconds)
		uint					m_frametime;			// in millseconds
		uint					m_avg_usframetime;	// in microseconds!

      UniqueWindowRegistry m_minimap;

   protected:
      void toggle_minimap(void);

      void mainview_move(int x, int y);
		void minimap_warp(int x, int y);

	   inline void set_mapview(Map_View* w) { m_mapview=w; }
      inline Map_View* get_mapview() { return m_mapview; }

      void set_fsel_picture(const char* file) { m_maprenderinfo.fsel=g_gr->get_picture(PicMod_Game, file, RGBColor(0,0,255)); }
      void unset_fsel_picture(void) { m_maprenderinfo.fsel=g_gr->get_picture(PicMod_Game, "pics/fsel.png", RGBColor(0,0,255)); }

		virtual void draw_overlay(RenderTarget* dst);

      // map rendering stuff. this is still protected since this is
      // used quite often
		MapRenderInfo	   m_maprenderinfo;
};


#endif // __S__INTPLAYER_H
