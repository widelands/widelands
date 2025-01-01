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

#ifndef WL_COMMANDS_CMD_DIPLOMACY_H
#define WL_COMMANDS_CMD_DIPLOMACY_H

#include "commands/command.h"

namespace Widelands {

struct CmdDiplomacy : PlayerCommand {
	CmdDiplomacy(const Time& t, PlayerNumber p, DiplomacyAction a, PlayerNumber o)
	   : PlayerCommand(t, p), action_(a), other_player_(o) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kDiplomacy;
	}

	void execute(Game& game) override;

	explicit CmdDiplomacy(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdDiplomacy() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	DiplomacyAction action_{DiplomacyAction::kResign};
	PlayerNumber other_player_{0U};  // Ignored for kLeaveTeam and kResign
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_DIPLOMACY_H
