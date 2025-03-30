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

#ifndef WL_COMMANDS_CMD_TRADE_ACTION_H
#define WL_COMMANDS_CMD_TRADE_ACTION_H

#include "commands/command.h"
#include "logic/game.h"

namespace Widelands {

struct CmdTradeAction : PlayerCommand {
	CmdTradeAction(const Time& time,
	               PlayerNumber pn,
	               TradeID trade_id,
	               TradeAction action,
	               Serial accepter,
	               Serial source);

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kTradeAction;
	}

	void execute(Game& game) override;

	// Network (de-)serialization
	explicit CmdTradeAction(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	// Savegame functions
	CmdTradeAction();
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	TradeID trade_id_{0U};
	TradeAction action_{TradeAction::kCancel};
	Serial accepter_{0U};
	Serial source_{0U};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_TRADE_ACTION_H
