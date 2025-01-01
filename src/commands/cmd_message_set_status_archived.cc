/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "commands/cmd_message_set_status_archived.h"

#include "logic/game.h"
#include "logic/player.h"

namespace Widelands {

/*** struct Cmd_MessageSetStatusArchived ***/

void CmdMessageSetStatusArchived::execute(Game& game) {
	game.get_player(sender())->get_messages()->set_message_status(
	   message_id(), Message::Status::kArchived);
}

void CmdMessageSetStatusArchived::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(message_id().value());
}

}  // namespace Widelands
