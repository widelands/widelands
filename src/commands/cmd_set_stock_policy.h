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

#ifndef WL_COMMANDS_CMD_SET_STOCK_POLICY_H
#define WL_COMMANDS_CMD_SET_STOCK_POLICY_H

#include "commands/command.h"
#include "logic/map_objects/tribes/warehouse.h"

namespace Widelands {

/**
 * Command to change the stock policy for a ware or worker in a warehouse.
 */
struct CmdSetStockPolicy : PlayerCommand {
	CmdSetStockPolicy(const Time& time,
	                  PlayerNumber p,
	                  const Building& wh,
	                  bool isworker,
	                  DescriptionIndex ware,
	                  StockPolicy policy);

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetStockPolicy;
	}

	void execute(Game& game) override;

	// Network (de-)serialization
	explicit CmdSetStockPolicy(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	// Savegame functions
	CmdSetStockPolicy();
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial warehouse_;
	bool isworker_;
	DescriptionIndex ware_;
	StockPolicy policy_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_SET_STOCK_POLICY_H
