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

#ifndef WL_COMMANDS_CMD_FLEET_TARGETS_H
#define WL_COMMANDS_CMD_FLEET_TARGETS_H

#include "commands/command.h"

namespace Widelands {

struct CmdFleetTargets : PlayerCommand {
	CmdFleetTargets(const Time& t, PlayerNumber p, Serial i, Quantity q)
	   : PlayerCommand(t, p), interface_(i), target_(q) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kFleetTargets;
	}

	void execute(Game& game) override;

	explicit CmdFleetTargets(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdFleetTargets() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial interface_{0U};
	Quantity target_{0U};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_FLEET_TARGETS_H
