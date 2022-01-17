/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_CMD_DELETE_MESSAGE_H
#define WL_LOGIC_CMD_DELETE_MESSAGE_H

#include "logic/cmd_queue.h"
#include "logic/message_id.h"
#include "logic/widelands.h"

namespace Widelands {

/// Delete the player's message.
///
/// \note This is not a GameLogicCommand because it should not be saved.
/// Instead, the commands are recreated separately on load (when both command
/// queue and message queue have been loaded). This is easier than saving the
/// command and then when loading, checking that one exists for each message
/// and if not, warn and recreate it. Such redundancy would also waste space in
/// the savegame.
struct CmdDeleteMessage : public Command {
	CmdDeleteMessage(const Time& t, PlayerNumber const p, const MessageId& m)
	   : Command(t), player(p), message(m) {
	}

	void execute(Game& game) override;
	QueueCommandTypes id() const override {
		return QueueCommandTypes::kDeleteMessage;
	}

private:
	PlayerNumber player;
	MessageId message;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_CMD_DELETE_MESSAGE_H
