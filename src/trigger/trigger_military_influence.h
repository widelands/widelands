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

#ifndef TRIGGER_MILITARY_INFLUENCE_H
#define TRIGGER_MILITARY_INFLUENCE_H

#include "trigger_player_area.h"

#include <string>
#include <cstring>

struct Trigger_Military_Influence_Option_Menu;

namespace Widelands {

struct Trigger_Military_Influence : public Trigger_Player_Area {
	friend struct ::Trigger_Military_Influence_Option_Menu;
	Trigger_Military_Influence(char const * Name, bool set);

	int32_t option_menu(Editor_Interactive &);

	void check_set_conditions(Game const &);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

private:
	bool require_highest;
};

};

#endif
