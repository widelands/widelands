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

#ifndef __S__EVENT_ALLOW_BUILDING_H
#define __S__EVENT_ALLOW_BUILDING_H

#include "event.h"

class Editor_Game_Base;

/*
 * Allows/denies the player to build a certain building 
 */
class Event_Allow_Building : public Event {
   public:
     Event_Allow_Building();
      ~Event_Allow_Building();

      // one liner functions
      uint get_id(void) { return EVENT_ALLOW_BUILDING; }

      void run(Game*);
      virtual void reinitialize(Game*);
      virtual void cleanup(Editor_Game_Base* g);

      // File Functions
      void Write(FileWrite*, Editor_Game_Base*);
      void Read(FileRead*, Editor_Game_Base*, bool);

      inline int get_player(void) { return m_player; }
      inline void set_player(int i) { m_player=i; }
      const char* get_building(void) { return m_building.c_str(); }
      void set_building(const char* b) { m_building=b; }
      inline void set_allow(bool t) { m_allow=t; }
      inline bool get_allow(void) { return m_allow; }

   private:
      bool m_allow;
      std::string m_building;
      int m_player;
};



#endif

