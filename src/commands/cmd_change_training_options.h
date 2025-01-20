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

#ifndef WL_COMMANDS_CMD_CHANGE_TRAINING_OPTIONS_H
#define WL_COMMANDS_CMD_CHANGE_TRAINING_OPTIONS_H

#include "commands/command.h"
#include "logic/map_objects/tribes/trainingsite.h"

namespace Widelands {

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions() = default;  // For savegame loading
	CmdChangeTrainingOptions(const Time& t,
	                         const PlayerNumber p,
	                         TrainingSite& ts,
	                         const TrainingAttribute at,
	                         const int32_t val)
	   : PlayerCommand(t, p), serial(ts.serial()), attribute(at), value(val) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kChangeTrainingOptions;
	}

	explicit CmdChangeTrainingOptions(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	TrainingAttribute attribute{TrainingAttribute::kHealth};
	int32_t value{0};
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_CHANGE_TRAINING_OPTIONS_H
