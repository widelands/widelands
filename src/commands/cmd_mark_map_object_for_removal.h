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

#ifndef WL_COMMANDS_CMD_MARK_MAP_OBJECT_FOR_REMOVAL_H
#define WL_COMMANDS_CMD_MARK_MAP_OBJECT_FOR_REMOVAL_H

#include "commands/command.h"
#include "logic/map_objects/immovable.h"

namespace Widelands {

struct CmdMarkMapObjectForRemoval : PlayerCommand {
	CmdMarkMapObjectForRemoval(const Time& t, PlayerNumber p, const Immovable& mo, bool m)
	   : PlayerCommand(t, p), object_(mo.serial()), mark_(m) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kMarkMapObjectForRemoval;
	}

	void execute(Game& game) override;

	explicit CmdMarkMapObjectForRemoval(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdMarkMapObjectForRemoval() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial object_{kInvalidSerial};
	bool mark_{false};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_MARK_MAP_OBJECT_FOR_REMOVAL_H
