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

#include "commands/cmd_build_road.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** class Cmd_BuildRoad ***/

CmdBuildRoad::CmdBuildRoad(const Time& t, int32_t p, Path& pa)
   : PlayerCommand(t, p),
     path(&pa),
     start(pa.get_start()),
     nsteps(pa.get_nsteps()),
     steps(nullptr) {
}

CmdBuildRoad::CmdBuildRoad(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     // We cannot completely deserialize the path here because we don't have a Map
     path(nullptr),
     start(read_coords_32(&des)),
     nsteps(des.unsigned_16()),
     steps(new uint8_t[nsteps]) {
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		steps[i] = des.unsigned_8();
	}
}

void CmdBuildRoad::execute(Game& game) {
	if (path == nullptr) {
		assert(steps);

		path.reset(new Path(start));
		for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
			path->append(game.map(), steps[i]);
		}
	}

	game.get_player(sender())->build_road(*path);
}

void CmdBuildRoad::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	write_coords_32(&ser, start);
	ser.unsigned_16(nsteps);

	assert(path || steps);

	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		ser.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

constexpr uint16_t kCurrentPacketVersionCmdBuildRoad = 1;

void CmdBuildRoad::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildRoad) {
			PlayerCommand::read(fr, egbase, mol);
			start = read_coords_32(&fr, egbase.map().extent());
			nsteps = fr.unsigned_16();
			path.reset(nullptr);
			steps.reset(new uint8_t[nsteps]);
			for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
				steps[i] = fr.unsigned_8();
			}
		} else {
			throw UnhandledVersionError(
			   "CmdBuildRoad", packet_version, kCurrentPacketVersionCmdBuildRoad);
		}
	} catch (const WException& e) {
		throw GameDataError("build road: %s", e.what());
	}
}
void CmdBuildRoad::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildRoad);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32(&fw, start);
	fw.unsigned_16(nsteps);
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		fw.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

}  // namespace Widelands
