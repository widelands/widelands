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
#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "immovable.h"
#include "map.h"
#include "soldier.h"
#include "tribe.h"
#include "world.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "log.h"

#include "upcast.h"

#include <map>


/* VERSION 1: initial release
*/

#define CURRENT_PACKET_VERSION 1

/*
 * Destructor
 */
Widelands_Map_Battle_Data_Packet::~Widelands_Map_Battle_Data_Packet()
{
}

/*
 * Read Function
 */
void Widelands_Map_Battle_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (not skip) {

		FileRead fr;
		try {fr.Open(fs, "binary/battle");} catch (...) {return;}

		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			const uint32_t nr_battles = fr.Unsigned32();
			for (uint32_t i = 0; i < nr_battles; ++i) {
				Battle & battle = *egbase->create_battle ();

				uint32_t const serial = fr.Unsigned32();
				battle.m_next_assault = fr.Unsigned32();
				battle.m_last_try     = fr.Unsigned32();

				uint32_t const soldier_1_serial = fr.Unsigned32();
				Map_Object * const soldier_1_mo =
					ol->get_object_by_file_index(soldier_1_serial);
				if (not soldier_1_mo)
					throw wexception
						("Widelands_Map_Battle_Data_Packet::Read: in "
						 "binary/battle:%u: battle %u: soldier 1 (%u) does not "
						 "exist",
						 fr.GetPos() - 4, serial, soldier_1_serial);
				upcast(Soldier, soldier_1, soldier_1_mo);
				if (not soldier_1)
					throw wexception
						("Widelands_Map_Battle_Data_Packet::Read: in "
						 "binary/battle:%u: battle %u: soldier 1 (%u) is not a "
						 "soldier",
						 fr.GetPos() - 4, serial, soldier_1_serial);

				uint32_t const soldier_2_serial = fr.Unsigned32();
				Map_Object * const soldier_2_mo =
					ol->get_object_by_file_index(soldier_2_serial);
				if (not soldier_2_mo)
					throw wexception
						("Widelands_Map_Battle_Data_Packet::Read: in "
						 "binary/battle:%u: battle %u: soldier 2 (%u) does not "
						 "exist",
						 fr.GetPos() - 4, serial, soldier_2_serial);
				upcast(Soldier, soldier_2, soldier_2_mo);
				if (not soldier_2)
					throw wexception
						("Widelands_Map_Battle_Data_Packet::Read: in "
						 "binary/battle:%u: battle %u: soldier 2 (%u) is not a "
						 "soldier",
						 fr.GetPos() - 4, serial, soldier_2_serial);

				battle.soldiers(soldier_1, soldier_2);

         // and register it with the object loader for further loading
			ol->register_object(egbase, serial, &battle);
		}
      if (fr.Unsigned32() != 0xffffffff)
         throw wexception ("Error in Widelands_Map_Battle_Data_Packet : Couldn't find 0xffffffff.");
		} else
			throw wexception
				("Unkown version of Widelands_Map_Battle_Data_Packet : %d",
				 packet_version);
	}
}

void Widelands_Map_Battle_Data_Packet::Write
(FileSystem &,
 Editor_Game_Base*,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
	throw wexception("Widelands_Map_Battle_Data_Packet::Write is obsolete");
}
