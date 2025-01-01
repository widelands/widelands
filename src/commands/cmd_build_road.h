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

#ifndef WL_COMMANDS_CMD_BUILD_ROAD_H
#define WL_COMMANDS_CMD_BUILD_ROAD_H

#include <memory>

#include "commands/command.h"
#include "logic/path.h"

namespace Widelands {

struct CmdBuildRoad : public PlayerCommand {
	CmdBuildRoad() = default;  // For savegame loading
	CmdBuildRoad(const Time&, int32_t, Path&);
	explicit CmdBuildRoad(StreamRead&);

	~CmdBuildRoad() override = default;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildRoad;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	std::unique_ptr<Path> path;
	Coords start;
	Path::StepVector::size_type nsteps{0U};
	std::unique_ptr<uint8_t[]> steps;
};

}  // namespace Widelands

#endif  // end of include guard: WL_COMMANDS_CMD_BUILD_ROAD_H
