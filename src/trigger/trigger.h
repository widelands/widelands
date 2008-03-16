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

#ifndef TRIGGER_H
#define TRIGGER_H

#include "named.h"
#include "referenced.h"

#include <map>
#include <string>
#include <cstring>

struct Editor_Interactive;
struct Section;

namespace Widelands {

struct Editor_Game_Base;
struct Game;

struct Trigger : public Named, public Referenced<Trigger> {
	friend struct Map_Trigger_Data_Packet;

	Trigger(char const * const Name, bool const set)
		: Named(Name), m_is_set(set)
	{}
	virtual ~Trigger() {}

	virtual int32_t option_menu(Editor_Interactive &) = 0;

	virtual void Read (Section &, Editor_Game_Base       &)       = 0;
	virtual void Write(Section &, Editor_Game_Base const &) const = 0;

	virtual void check_set_conditions(Game const &) = 0;

	/// Toggle the triggers state (if it isn't a one timer) and give it a chance
	/// to reinitialize.
	virtual void reset_trigger       (Game const &) {}

	bool is_set() const {return m_is_set;}

protected:
	/// This is only for child classes to toggle the trigger
	void set_trigger(bool t) {m_is_set = t;}

private:
	bool m_is_set;
};

};

#endif
