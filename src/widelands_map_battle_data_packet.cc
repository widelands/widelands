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

#include "widelands_map_battle_data_packet.h"

#include "battle.h"
#include "editor_game_base.h"
#include "immovable.h"
#include "map.h"
#include "soldier.h"
#include "tribe.h"
#include "world.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "log.h"

#include "upcast.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Battle_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/battle");} catch (...) {return;}

	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			const uint32_t nr_battles = fr.Unsigned32();
			for (uint32_t i = 0; i < nr_battles; ++i) {
				Serial const serial = fr.Unsigned32();
				try {
					Battle & battle =
						ol->register_object<Battle>
						(serial, *egbase->create_battle());
					battle.m_next_assault = fr.Unsigned32();
					battle.m_last_try     = fr.Unsigned32();

					Soldier * soldier_1, * soldier_2;
					{
						Serial const soldier_1_serial = fr.Unsigned32();
						try {
							soldier_1 = &ol->get<Soldier>(soldier_1_serial);
						} catch (_wexception const & e) {
							throw wexception
								("soldier 1 (%u): %s", soldier_1_serial, e.what());
						}
					}
					{
						Serial const soldier_2_serial = fr.Unsigned32();
						try {
							soldier_2 = &ol->get<Soldier>(soldier_2_serial);
						} catch (_wexception const & e) {
							throw wexception
								("soldier 2 (%u): %s", soldier_2_serial, e.what());
						}
					}
					battle.soldiers(soldier_1, soldier_2);
				} catch (_wexception const & e) {
					throw wexception("battle %u: %s", serial, e.what());
				}
			}
			if (fr.Unsigned32() != 0xffffffff)
				throw wexception
					("Error in Map_Battle_Data_Packet : Couldn't find 0xffffffff.");
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("battle: %s", e.what());
	}
}

void Map_Battle_Data_Packet::Write
(FileSystem &, Editor_Game_Base *, Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Map_Battle_Data_Packet::Write is obsolete");
}

};
