/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef TRIGGER_MESSAGE_IS_READ_OR_ARCHIVED_H
#define TRIGGER_MESSAGE_IS_READ_OR_ARCHIVED_H

#include "trigger.h"
#include "logic/message_id.h"
#include "logic/widelands.h"

namespace Widelands {

struct Trigger_Message_Is_Read_Or_Archived : public Trigger {
	Trigger_Message_Is_Read_Or_Archived(char const * name, bool set = false);

	int32_t option_menu(Editor_Interactive &);

	void check_set_conditions(Game const &);
	void reset_trigger       (Game const &);

	void Read (Section &, Editor_Game_Base       &);
	void Write
		(Section &, Editor_Game_Base const &, Map_Map_Object_Saver const &)
		const;

	void set_message(Player_Number const p, Message_Id const m) {
		player  = p;
		message = m;
	}

private:
	/// Logs a warning when this trigger's message no longer exists when Write
	/// or check_set_conditions is called.
	void warn_when_message_expired() const;

	Player_Number player;
	Message_Id    message;
};

}

#endif
