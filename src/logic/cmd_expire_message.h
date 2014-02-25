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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef CMD_EXPIRE_MESSAGE_H
#define CMD_EXPIRE_MESSAGE_H

#include "logic/cmd_queue.h"
#include "logic/message_queue.h"

namespace Widelands {

/// Expires the player's message.
///
/// \note This is not a GameLogicCommand because it should not be saved.
/// Instead, the commands are recreated separately on load (when both command
/// queue and message queue have been loaded). This is easier than saving the
/// command and then when loading, checking that one exists for each message
/// and if not, warn and recreate it. Such redundancy would also waste space in
/// the savegame.
struct Cmd_ExpireMessage : public Command {
	Cmd_ExpireMessage
		(int32_t const t, Player_Number const p, Message_Id const m)
		: Command(t), player(p), message(m)
	{}

	void execute (Game & game) override;
	virtual uint8_t id() const override {return QUEUE_CMD_EXPIREMESSAGE;}

private:
	Player_Number player;
	Message_Id    message;
};

}

#endif
