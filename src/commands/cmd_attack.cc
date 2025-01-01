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

#include "commands/cmd_attack.h"

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

/*** Cmd_Attack ***/

CmdAttack::CmdAttack(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	const uint32_t number = des.unsigned_32();
	soldiers_.clear();
	for (uint32_t i = 0; i < number; ++i) {
		soldiers_.push_back(des.unsigned_32());
	}
	allow_conquer_ = (des.unsigned_8() != 0u);
}

void CmdAttack::execute(Game& game) {
	Player* player = game.get_player(sender());

	if (upcast(Flag, flag, game.objects().get_object(serial_))) {
		verb_log_info_time(game.get_gametime(),
		                   "Cmd_Attack::execute player(%u): flag->owner(%d) "
		                   "number=%" PRIuS "\n",
		                   player->player_number(), flag->owner().player_number(), soldiers_.size());

		if (const Building* const building = flag->get_building()) {
			if (player->is_hostile(flag->owner())) {
				for (Widelands::Coords& coords : building->get_positions(game)) {
					if (player->is_seeing(Map::get_index(coords, game.map().get_width()))) {
						std::vector<Soldier*> result;
						for (Serial s : soldiers_) {
							if (Soldier* soldier = dynamic_cast<Soldier*>(game.objects().get_object(s))) {
								result.push_back(soldier);
							}
						}
						player->attack(*flag, sender(), result, allow_conquer_);
						return;
					}
				}
			}
			log_warn_time(game.get_gametime(),
			              "Cmd_Attack::execute: wrong player target not seen or not hostile.\n");
		}
	}
}

void CmdAttack::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_32(soldiers_.size());
	for (Serial s : soldiers_) {
		ser.unsigned_32(s);
	}
	ser.unsigned_8(allow_conquer_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdAttack = 5;  // since v1.0

void CmdAttack::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersionCmdAttack && packet_version >= 5) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);

			soldiers_.clear();
			const uint32_t number = fr.unsigned_32();
			for (uint32_t i = 0; i < number; ++i) {
				soldiers_.push_back(mol.get<Soldier>(fr.unsigned_32()).serial());
			}
			allow_conquer_ = (fr.unsigned_8() != 0u);
		} else {
			throw UnhandledVersionError("CmdAttack", packet_version, kCurrentPacketVersionCmdAttack);
		}
	} catch (const WException& e) {
		throw GameDataError("enemy flag action: %s", e.what());
	}
}

void CmdAttack::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdAttack);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));

	// Now param
	fw.unsigned_32(soldiers_.size());
	for (Serial s : soldiers_) {
		fw.unsigned_32(mos.get_object_file_index(*egbase.objects().get_object(s)));
	}

	fw.unsigned_8(allow_conquer_ ? 1 : 0);
}

}  // namespace Widelands
