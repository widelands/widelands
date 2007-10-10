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

#ifndef __S__EVENT_ALLOW_BUILDING_H
#define __S__EVENT_ALLOW_BUILDING_H

#include "event.h"

class Editor_Game_Base;

/*
 * Allows/denies the player to build a certain building
 */
struct Event_Allow_Building : public Event {
     Event_Allow_Building();
      ~Event_Allow_Building();

      // one liner functions
	const char * get_id() const {return "allow_building";}

      State run(Game*);
      virtual void reinitialize(Game*);

      // File Functions
	void Write(Section &, const Editor_Game_Base &) const;
      void Read(Section*, Editor_Game_Base*);

	int32_t get_player() const {return m_player;}
	void set_player(int32_t i) {m_player = i;}
      const char* get_building() {return m_building.c_str();}
      void set_building(const char* b) {m_building=b;}
	void set_allow(bool t) {m_allow = t;}
	bool get_allow() {return m_allow;}

private:
      bool m_allow;
      std::string m_building;
      int32_t m_player;
};



#endif
