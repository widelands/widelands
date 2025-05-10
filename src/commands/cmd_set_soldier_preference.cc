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

#include "commands/cmd_set_soldier_preference.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_SetSoldierPreference ***/

CmdSetSoldierPreference::CmdSetSoldierPreference(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	preference = static_cast<Widelands::SoldierPreference>(des.unsigned_8());
}

void CmdSetSoldierPreference::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(preference));
}

void CmdSetSoldierPreference::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		if (upcast(MilitarysiteSettings, ms, cs->get_settings())) {
			ms->soldier_preference = preference;
		} else if (upcast(WarehouseSettings, wh, cs->get_settings())) {
			wh->soldier_preference = preference;
		}
	} else if (upcast(MilitarySite, ms, mo)) {
		ms->set_soldier_preference(preference);
	} else if (upcast(Warehouse, wh, mo)) {
		wh->set_soldier_preference(preference);
	} else if (upcast(Ship, ship, mo)) {
		ship->set_soldier_preference(preference);
	}
}

constexpr uint16_t kCurrentPacketVersionSoldierPreference = 1;

void CmdSetSoldierPreference::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionSoldierPreference);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_8(static_cast<uint8_t>(preference));

	// Now serial.
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

void CmdSetSoldierPreference::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSoldierPreference) {
			PlayerCommand::read(fr, egbase, mol);
			preference = static_cast<Widelands::SoldierPreference>(fr.unsigned_8());
			serial = get_object_serial_or_zero<MapObject>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdSetSoldierPreference", packet_version, kCurrentPacketVersionSoldierPreference);
		}
	} catch (const WException& e) {
		throw GameDataError("cmd soldier preference: %s", e.what());
	}
}

}  // namespace Widelands
