/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef TRIGGER_PLAYER_AREA_H
#define TRIGGER_PLAYER_AREA_H

#include "logic/player_area.h"
#include "trigger.h"

struct Trigger_Building_Option_Menu;

namespace Widelands {

/*
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Player_Area : public Trigger {
	friend struct ::Trigger_Building_Option_Menu;
	Trigger_Player_Area(char const * Name, bool set);

	virtual void reorigin(Coords const new_origin, Extent const extent) {
		m_player_area.reorigin(new_origin, extent);
	}

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

	typedef uint16_t Count_Type;

protected:
	Player_Area<Area<FCoords> > m_player_area;
	Count_Type m_count;
};

}

#endif
