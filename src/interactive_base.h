/*
 * Copyright (C) 2002 by The Widelands Development Team
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

/** class Interactive_Base
 *
 * This is the interactive base class. It is used 
 * to represent the code that Interactive_Player and
 * Editor_Interactive share.
 */
class Interactive_Base : public Panel {
	public:
		Interactive_Base(Editor_Game_Base* g);
		virtual ~Interactive_Base(void);

      inline Map* get_map() { assert(m_egbase); return m_egbase->get_map(); }
      
		static int get_xres();
		static int get_yres();
	   
      // logic handler func
      void think();

		inline const Coords &get_fieldsel() const { return m_maprenderinfo.fieldsel; }
		inline bool get_fieldsel_freeze() const { return m_fieldsel_freeze; }
		void set_fieldsel(Coords c);
		void set_fieldsel_freeze(bool yes);
		
		inline const MapRenderInfo* get_maprenderinfo() const { return &m_maprenderinfo; }
		
      virtual void recalc_overlay(FCoords fc) = 0;
      virtual void start() = 0;

   protected:
		bool		         m_fieldsel_freeze; // don't change m_fieldsel even if mouse moves
		MapRenderInfo	   m_maprenderinfo;
      
      Editor_Game_Base* m_egbase;
};


#endif // __S__INTPLAYER_H
