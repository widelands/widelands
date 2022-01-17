/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "logic/cmd_incorporate.h"

#include "base/i18n.h"
#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void CmdIncorporate::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			GameLogicCommand::read(fr, egbase, mol);
			uint32_t const worker_serial = fr.unsigned_32();
			try {
				worker = &mol.get<Worker>(worker_serial);
			} catch (const WException& e) {
				throw wexception("worker %u: %s", worker_serial, e.what());
			}
		} else {
			throw UnhandledVersionError("CmdIncorporate", packet_version, kCurrentPacketVersion);
		}

	} catch (const WException& e) {
		throw wexception("incorporate: %s", e.what());
	}
}

void CmdIncorporate::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersion);

	// Write base classes
	GameLogicCommand::write(fw, egbase, mos);

	// Now serial
	assert(mos.is_object_known(*worker));
	fw.unsigned_32(mos.get_object_file_index(*worker));
}
}  // namespace Widelands
