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
	try {fr.Open(fs, "binary/attackcontroller");} catch (...) {return;}
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			Extent const extent = egbase->map().extent();
			uint32_t const nrControllers = fr.Unsigned32();
			for (uint32_t i = 0; i < nrControllers; ++i) {
				AttackController & ctrl = *egbase->create_attack_controller();

				fr.Unsigned32();
				uint32_t const flagFilePos = fr.Unsigned32();
				try {
					ctrl.flag = &ol->get<Flag>(flagFilePos);
				} catch (_wexception const & e) {
					throw wexception("flag (%u): %s", flagFilePos, e.what());
				}
				ctrl.attackingPlayer = fr.Unsigned32();
				ctrl.defendingPlayer = fr.Unsigned32();
				ctrl.totallyLaunched = fr.Unsigned32();
				ctrl.attackedMsEmpty = fr.Unsigned8();

				{
					uint32_t const numBs = fr.Unsigned32();
					for (uint32_t j = 0; j < numBs; ++j) {
						uint32_t const soldier_serial = fr.Unsigned32();
						try {
							Soldier & soldier = ol->get<Soldier>(soldier_serial);

							uint32_t const origin_serial = fr.Unsigned32();
							MilitarySite * origin;
							try {
								origin = &ol->get<MilitarySite>(origin_serial);
							} catch (_wexception const & e) {
								throw wexception
									("origin (%u): %s", origin_serial, e.what());
							}

							Coords battleGround;
							if (packet_version == 1) {
								uint32_t const x = fr.Unsigned32();
								if (extent.w <= x)
									throw wexception
									("Map_Attack_Controller_Data_Packet::Read: in "
									 "binary/attackcontroller:%u: battleGround has x "
									 "coordinate %i, but the map width is only %u",
									 fr.GetPos() - 4, x, extent.w);
								uint32_t const y = fr.Unsigned32();
								if (extent.h <= y)
									throw wexception
									("Map_Attack_Controller_Data_Packet::Read: in "
									 "binary/attackcontroller:%u: battleGround has y "
									 "coordinate %i, but the map height is only %u",
									 fr.GetPos() - 4, y, extent.h);
								battleGround = Coords(x, y);
							} else {
								try {battleGround = fr.Coords32(extent);}
								catch (_wexception const & e) {
									throw wexception
										("Map_Attack_Controller_Data_Packet::Read: in "
										 "binary/attackcontroller:%u: reading coordinates of "
										 "battleground: %s",
										 fr.GetPos() - 4, e.what());
								}
							}

							{
								bool const is_attacker = fr.Unsigned8();
								bool const has_arrived = fr.Unsigned8();
								bool const is_fighting = fr.Unsigned8();
								AttackController::BattleSoldier bs = {
									&soldier,
									origin,
									battleGround,
									is_attacker,
									has_arrived,
									is_fighting
								};
								ctrl.involvedSoldiers.push_back(bs);
							}
							soldier.set_attack_ctrl(&ctrl);
						} catch (_wexception const & e) {
							throw wexception
								("soldier #%u (%u): %s", i, soldier_serial, e.what());
						}
					}
				}

				uint32_t const numInMs = fr.Unsigned32();
				for (uint32_t j = numInMs; j; --j) {
					uint32_t const ms_serial = fr.Unsigned32();
					try {
						MilitarySite & ms = ol->get<MilitarySite>(ms_serial);
						ctrl.involvedMilitarySites.insert(&ms);
						ms.set_in_battle(true);
					} catch (_wexception const & e) {
						throw wexception
							("involved militarysite #%u (%u): %s",
							 numInMs - j, ms_serial, e.what());
					}
				}

			}
			if (fr.Unsigned32() != 0xffffffff)
				throw wexception
					("Error in Attack_Controller_Data_Packet : Couldn't find "
					 "0xffffffff.");
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("attack controller: %s", e.what());
	}
}


void Map_Attack_Controller_Data_Packet::Write
(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Map_Attack_Controller_Data_Packet::Write is obsolete");
}

};
