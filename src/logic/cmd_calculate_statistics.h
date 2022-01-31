/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_CMD_CALCULATE_STATISTICS_H
#define WL_LOGIC_CMD_CALCULATE_STATISTICS_H

#include "logic/cmd_queue.h"

namespace Widelands {

struct CmdCalculateStatistics : public GameLogicCommand {
	CmdCalculateStatistics() : GameLogicCommand(Time(0)) {
	}  // For savegame loading
	explicit CmdCalculateStatistics(const Time& init_duetime) : GameLogicCommand(init_duetime) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kCalculateStatistics;
	}
	void execute(Game&) override;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_CMD_CALCULATE_STATISTICS_H
