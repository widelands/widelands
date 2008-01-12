/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "player_area.h"
#include "trigger.h"

#include <string>

struct Trigger_Building_Option_Menu;

namespace Widelands {

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Building : public Trigger {
	friend struct ::Trigger_Building_Option_Menu;
      Trigger_Building();
      ~Trigger_Building();

      // one liner functions
	const char * get_id() const {return "building";}

      void check_set_conditions(Game*);
      void reset_trigger(Game*);

      // File Functions
	void Write(Section &) const;
      void Read(Section*, Editor_Game_Base*);

	const char* get_building() const {return m_building.c_str();}
      void set_building(const char* b) {m_building=b;}
      void set_building_count(int32_t n) {m_count=n;}
	int32_t get_building_count() const {return m_count;}

	typedef uint8_t Count_Type;

private:
	Player_Area<Area<FCoords> > m_player_area;
      std::string m_building;
	Count_Type m_count;
};

};

#endif
