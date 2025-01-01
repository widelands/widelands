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

#ifndef WL_COMMANDS_CMD_TOGGLE_MUTE_MESSAGES_H
#define WL_COMMANDS_CMD_TOGGLE_MUTE_MESSAGES_H

#include "commands/command.h"
#include "logic/map_objects/tribes/building.h"

namespace Widelands {

struct CmdToggleMuteMessages : PlayerCommand {
	CmdToggleMuteMessages(const Time& t, PlayerNumber p, const Building& b, bool a)
	   : PlayerCommand(t, p), building_(b.serial()), all_(a) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kToggleMuteMessages;
	}

	void execute(Game& game) override;

	explicit CmdToggleMuteMessages(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdToggleMuteMessages() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial building_{kInvalidSerial};
	bool all_{false};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_TOGGLE_MUTE_MESSAGES_H
