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

#include "trigger_player_area.h"

#include <string>

struct Trigger_Building_Option_Menu;

namespace Widelands {

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Building : public Trigger_Player_Area {
	friend struct ::Trigger_Building_Option_Menu;
	Trigger_Building(char const * Name, bool set);

	int32_t option_menu(Editor_Interactive &);

	void check_set_conditions(Game const &);

	void Read (Section &, Editor_Game_Base &);
	void Write(Section &) const;

	const char* get_building() const {return m_building.c_str();}
      void set_building(const char* b) {m_building=b;}

private:
      std::string m_building;
};

};

#endif
