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

#ifndef WL_COMMANDS_CMD_CALL_ECONOMY_BALANCE_H
#define WL_COMMANDS_CMD_CALL_ECONOMY_BALANCE_H

#include "commands/command.h"
#include "economy/flag.h"

namespace Widelands {
class Economy;

struct CmdCallEconomyBalance : public GameLogicCommand {
	CmdCallEconomyBalance() : GameLogicCommand(Time()) {
	}  ///< for load and save

	CmdCallEconomyBalance(const Time& starttime,
	                      Flag* representative_flag,
	                      WareWorker type,
	                      uint32_t timerid);

	void execute(Game&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kCallEconomyBalance;
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	OPtr<Flag> flag_;
	WareWorker type_{wwWARE};
	uint32_t timerid_{0U};
};
}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_CALL_ECONOMY_BALANCE_H
