/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_ECONOMY_CMD_CALL_ECONOMY_BALANCE_H
#define WL_ECONOMY_CMD_CALL_ECONOMY_BALANCE_H

#include "economy/flag.h"
#include "logic/cmd_queue.h"
#include "logic/map_objects/map_object.h"

namespace Widelands {
class Economy;
class Game;
class MapObjectLoader;

struct CmdCallEconomyBalance : public GameLogicCommand {
	CmdCallEconomyBalance() : GameLogicCommand(0), timerid_(0) {
	}  ///< for load and save

	CmdCallEconomyBalance(uint32_t starttime, Economy*, uint32_t timerid);

	void execute(Game&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kCallEconomyBalance;
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	OPtr<Flag> flag_;
	uint32_t timerid_;
};
}

#endif  // end of include guard: WL_ECONOMY_CMD_CALL_ECONOMY_BALANCE_H
