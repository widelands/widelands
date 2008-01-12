/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_attack_controller_data_packet.h"

#include "attack_controller.h"
#include "battle.h"
#include "editor_game_base.h"
#include "filewrite.h"
#include "widelands_geometry.h"
#include "immovable.h"
#include "militarysite.h"
#include "map.h"
#include "soldier.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "world.h"

#include "upcast.h"

#include <map>
#include <set>

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Attack_Controller_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {
		fr.Open(fs, "binary/attackcontroller");
	} catch (...) {
		// not there, so skip
		return ;
	}
	const uint16_t packet_version = fr.Unsigned16();
	if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
		const Extent extent = egbase->map().extent();
		uint32_t const nrControllers = fr.Unsigned32();
		for (uint32_t i = 0; i < nrControllers; ++i) {
			AttackController* ctrl = egbase->create_attack_controller();

			fr.Unsigned32();
			uint32_t flagFilePos = fr.Unsigned32();

			if (upcast(Flag, flag, ol->get_object_by_file_index(flagFilePos)))
				ctrl->flag = flag;
			else
				throw wexception
					("Map_Attack_Controller_Data_Packet::Read: in "
					 "binary/attackcontroller:%u: object with file index %u is not "
					 "a flag",
					 fr.GetPos() - 4, flagFilePos);
			ctrl->attackingPlayer = fr.Unsigned32();
			ctrl->defendingPlayer = fr.Unsigned32();
			ctrl->totallyLaunched = fr.Unsigned32();
			ctrl->attackedMsEmpty = fr.Unsigned8();

			uint32_t numBs = fr.Unsigned32();

			for (uint32_t j = 0; j < numBs; ++j) {
				upcast(Soldier, soldier, ol->get_object_by_file_index(fr.Unsigned32()));
				assert(soldier); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!

				upcast(MilitarySite, origin, ol->get_object_by_file_index(fr.Unsigned32()));
				assert(origin); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!

				Coords battleGround;
				if (packet_version == 1) {
					const uint32_t x = fr.Unsigned32();
					if (extent.w <= x)
						throw wexception
							("Map_Attack_Controller_Data_Packet::Read: in "
							 "binary/attackcontroller:%u: battleGround has x "
							 "coordinate %i, but the map width is only %u",
							 fr.GetPos() - 4, x, extent.w);
					const uint32_t y = fr.Unsigned32();
					if (extent.h <= y)
						throw wexception
							("Map_Attack_Controller_Data_Packet::Read: in "
							 "binary/attackcontroller:%u: battleGround has y "
							 "coordinate %i, but the map height is only %u",
							 fr.GetPos() - 4, y, extent.h);
					battleGround = Coords(x, y);
				} else {
					try {battleGround = fr.Coords32(extent);}
					catch (const FileRead::Data_Error & e) {
						throw wexception
							("Map_Attack_Controller_Data_Packet::Read: in "
							 "binary/attackcontroller:%u: reading coordinates of "
							 "battleground: %s",
							 fr.GetPos() - 4, e.message().c_str());
					}
				}

				bool attacker = fr.Unsigned8();
				bool arrived = fr.Unsigned8();
				bool fighting = fr.Unsigned8();
				AttackController::BattleSoldier bs = {
					soldier,
					origin,
					battleGround,
					attacker,
					arrived,
					fighting
				};
				ctrl->involvedSoldiers.push_back(bs);
				soldier->set_attack_ctrl(ctrl);
			}

			uint32_t numInMs = fr.Unsigned32();
			for (uint32_t j = 0; j < numInMs; ++j) {
				upcast(MilitarySite, ms, ol->get_object_by_file_index(fr.Unsigned32()));
			assert(ms); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
			ctrl->involvedMilitarySites.insert(ms);
			ms->set_in_battle(true);
			}

		}
		if (fr.Unsigned32() != 0xffffffff)
			throw wexception
				("Error in Attack_Controller_Data_Packet : Couldn't find "
				 "0xffffffff.");
	} else
		throw wexception
			("Unkown version of Map_Battle_Data_Packet : %u", packet_version);
}


void Map_Attack_Controller_Data_Packet::Write
(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Map_Attack_Controller_Data_Packet::Write is obsolete");
}

};
