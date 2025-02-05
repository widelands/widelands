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

#ifndef WL_COMMANDS_CMD_ACT_H
#define WL_COMMANDS_CMD_ACT_H

#include "commands/command.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {

struct CmdAct : public GameLogicCommand {
	CmdAct() : GameLogicCommand(Time()) {
	}  ///< For savegame loading
	CmdAct(const Time& t, MapObject&, int32_t a);

	void execute(Game&) override;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kAct;
	}

private:
	Serial obj_serial{0U};
	int32_t arg{0};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_ACT_H
