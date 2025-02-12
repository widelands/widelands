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

#ifndef WL_COMMANDS_CMD_FLAG_ACTION_H
#define WL_COMMANDS_CMD_FLAG_ACTION_H

#include "commands/command.h"
#include "economy/flag.h"

namespace Widelands {

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() = default;  // For savegame loading
	CmdFlagAction(const Time& t, const int32_t p, const Flag& f, FlagJob::Type y)
	   : PlayerCommand(t, p), serial_(f.serial()), type_(y) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kFlagAction;
	}

	explicit CmdFlagAction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	FlagJob::Type type_{FlagJob::Type::kGeologist};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_FLAG_ACTION_H
