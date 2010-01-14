/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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

#ifndef TRIGGER_DEFEATED_H
#define TRIGGER_DEFEATED_H

#include "trigger.h"

#include <string>
#include <cstring>

namespace Widelands {

/**
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Defeated : public Trigger {
	Trigger_Defeated(char const *, bool);

	bool has_option_menu() const {return false;}
	int32_t option_menu(Editor_Interactive &) __attribute__ ((noreturn)) {
		throw;
	}

	void check_set_conditions(Game const &);

	void Read (Section &, Editor_Game_Base       &);
	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

private:
	int32_t m_player;

};

}

#endif
