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

#ifndef WL_COMMANDS_CMD_BUILDING_NAME_H
#define WL_COMMANDS_CMD_BUILDING_NAME_H

#include "commands/command.h"

namespace Widelands {

struct CmdBuildingName : PlayerCommand {
	CmdBuildingName(const Time& t, PlayerNumber p, Serial s, const std::string& n)
	   : PlayerCommand(t, p), serial_(s), name_(n) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildingName;
	}

	void execute(Game& game) override;

	explicit CmdBuildingName(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdBuildingName() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial serial_{0U};
	std::string name_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_BUILDING_NAME_H
