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

#include "widelands_map_waredata_data_packet.h"

#include "bob.h"
#include "editor_game_base.h"
#include "game.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "tribe.h"
#include "widelands_fileread.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Waredata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/ware_data");} catch (...) {return;}

	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) for (;;) {
		uint32_t reg = fr.Unsigned32();
		if (reg == 0xffffffff)
			break; // end of wares
		if (upcast(WareInstance, ware, ol->get_object_by_file_index(reg))) {
			reg = fr.Unsigned32();
			if (Map_Object * const location = ol->get_object_by_file_index(reg)) {
				const uint32_t ware_index_from_file = fr.Unsigned32();
				ware->m_descr_index =
					static_cast<Ware_Index::value_t>(ware_index_from_file);
				if (upcast(PlayerImmovable, player_immovable, location)) {
					if
						(dynamic_cast<const Building *>(player_immovable)
						 or
						 dynamic_cast<const Flag     *>(player_immovable))
					{
						//  We didn't know what kind of ware we were till now, so no
						//  economy might have a clue of us.
						ware->m_economy = 0;

						const Tribe_Descr & tribe = player_immovable->owner().tribe();
						if (tribe.get_nrwares() <= static_cast<int32_t>(ware_index_from_file))
							throw wexception
								("Map_Waredata_Data_Packet: ware index out of range: "
								 "%i",
								 ware_index_from_file);
						ware->m_descr = tribe.get_ware_descr(ware->descr_index());
						ware->set_economy(player_immovable->get_economy());
					} else
						throw wexception
							("Map_Waredata_Data_Packet: location is PlayerImmovable "
							 "but not Building or Flag");
				} else if (upcast(Worker, worker, location)) {
					const Tribe_Descr & tribe = *worker->get_tribe();
					if (tribe.get_nrwares() <= static_cast<int32_t>(ware_index_from_file))
						throw wexception
							("Map_Waredata_Data_Packet: ware index out of range: %i",
							 ware_index_from_file);
					ware->m_descr = tribe.get_ware_descr(ware->descr_index());
					ware->m_economy = 0; //  The worker sets our economy.
				} else
					throw wexception
						("Map_Waredata_Data_Packet: location is not PlayerImmovable "
						 "or Worker");
				//  Do not touch supply or transfer.

				if (uint32_t const nextstep = fr.Unsigned32()) {
					assert(ol->is_object_known(reg)); //  FIXME NEVER USE assert TO VALIDATE INPUT!!!
					ware->m_transfer_nextstep=ol->get_object_by_file_index(nextstep);
				} else
					ware->m_transfer_nextstep = static_cast<Map_Object *>(0);

				//  Do some kind of init.
				if (upcast(Game, game, egbase))
					ware->set_location(game, location);
				ol->mark_object_as_loaded(ware);
			} else
				throw wexception
					("Map_Waredata_Data_Packet: location with serial number %u is "
					 "not known",
					 reg);
		} else
			throw wexception
				("Map_Waredata_Data_Packet: ware with serial number %u is not "
				 "known",
				 reg);
	} else
		throw wexception
			("Unknown version %u in Map_Waredata_Data_Packet!", packet_version);
}


void Map_Waredata_Data_Packet::Write
(FileSystem & fs, Editor_Game_Base * egbase, Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  We transverse the map and whenever we find a suitable object, we check
	//  if it has wares of some kind.
	std::vector<uint32_t> ids;
	Map   const & map        = egbase->map();
	Field const & fields_end = map[map.max_index()];
	for (Field const * field = &map[0]; field < &fields_end; ++field) {
		// First, check for Flags
		if (upcast(Flag const, fl, field->get_immovable()))
			for (int32_t i = 0; i < fl->m_item_filled; ++i) {
				assert(os->is_object_known(fl->m_items[i].item));
				write_ware(&fw, egbase, os, fl->m_items[i].item);
			}

		//  Now, check for workers.
		for (Bob const * b = field->get_first_bob(); b; b = b->get_next_bob())
			if (upcast(Worker const, worker, b))
				if
					(WareInstance const * const ware =
					 worker->get_carried_item(egbase))
				{
					assert(os->is_object_known(ware));
					write_ware(&fw, egbase, os, ware);
				}
	}
	fw.Unsigned32(0xffffffff); // End of wares

	fw.Write(fs, "binary/ware_data");
}

/*
 * Write this ware instances data to disk
 */
void Map_Waredata_Data_Packet::write_ware
	(FileWrite            * fw,
	 Editor_Game_Base     * egbase,
	 Map_Map_Object_Saver * os,
	 WareInstance   const * ware)
{
	fw->Unsigned32(os->get_object_file_index(ware));

	if (Map_Object const * const obj = ware->m_location.get(egbase)) {
		assert(os->is_object_known(obj));
		fw->Unsigned32(os->get_object_file_index(obj));
	} else
		fw->Unsigned32(0);

	// Economy is set by set_location()

	fw->Signed32(ware->descr_index());

	if (Map_Object const * const obj = ware->m_transfer_nextstep.get(egbase)) {
		assert(os->is_object_known(obj));
		fw->Unsigned32(os->get_object_file_index(obj));
	} else
		fw->Unsigned32(0);

	os->mark_object_as_saved(ware);
}

};
