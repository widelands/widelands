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

#ifndef WL_COMMANDS_CMD_PROPOSE_TRADE_H
#define WL_COMMANDS_CMD_PROPOSE_TRADE_H

#include "commands/command.h"
#include "logic/game.h"

namespace Widelands {

struct CmdProposeTrade : PlayerCommand {
	CmdProposeTrade(const Time& time, PlayerNumber pn, const TradeInstance& trade);

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kProposeTrade;
	}

	void execute(Game& game) override;

	// Network (de-)serialization
	explicit CmdProposeTrade(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	// Savegame functions
	CmdProposeTrade();
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	TradeInstance trade_;
	Serial initiator_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_PROPOSE_TRADE_H
