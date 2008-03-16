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

#ifndef TRIGGER_TIME_H
#define TRIGGER_TIME_H

#include "trigger.h"

namespace Widelands {

/**
 * For documentation see the description in editor or trigger_factory.cc
 * or see trigger.h
 */
struct Trigger_Time : public Trigger {
	Trigger_Time(char const * Name, bool set);

	int32_t option_menu(Editor_Interactive &);

	void check_set_conditions(Game const &);
	void reset_trigger       (Game const &);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

	void set_wait_time      (int32_t i) {m_wait_time       = i;}
	void set_last_start_time(int32_t i) {m_last_start_time = i;}
	int32_t get_wait_time() const {return m_wait_time;}

private:
	uint32_t m_wait_time; // in seconds
	uint32_t m_last_start_time;
};

};

#endif
