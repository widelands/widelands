/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__TRIGGER_BUILDING_H
#define __S__TRIGGER_BUILDING_H

#include <string>
#include "trigger.h"
#include "geometry.h"

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
class Trigger_Building : public Trigger {
   public:
      Trigger_Building();
      ~Trigger_Building();

      // one liner functions
      const char* get_id(void) { return "building"; }

      void check_set_conditions(Game*);
      void reset_trigger(Game*);

      // File Functions
      void Write(Section*);
      void Read(Section*, Editor_Game_Base*);

      inline void set_coords(Coords pt) { m_pt=pt; }
      inline Coords get_coords(void) { return m_pt; }
      inline int get_player(void) { return m_player; }
      inline void set_player(int i) { m_player=i; }
      inline int get_area(void) { return m_area; }
      inline void set_area(int i) { m_area=i; }
      const wchar_t* get_building(void) { return m_building.c_str(); }
      void set_building(const wchar_t* b) { m_building=b; }
      void set_building_count(int n) { m_count=n; }
      int get_building_count(void) { return m_count; }
 
   private:
      Coords m_pt;
      std::wstring m_building;
      int m_player;
      int m_area;
      int m_count;
};

#endif

