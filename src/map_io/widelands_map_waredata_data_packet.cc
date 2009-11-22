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

#include "logic/bob.h"
#include "economy/flag.h"
#include "economy/ware_instance.h"
#include "logic/legacy.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "upcast.h"
#include "logic/widelands_fileread.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "logic/worker.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Waredata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/ware_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 == packet_version) {
			for (;;) {
				Serial const serial = fr.Unsigned32();
				if (serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw game_data_error
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					WareInstance & ware = ol->get<WareInstance>(serial);
					ware.m_economy = 0;
					uint32_t     const location_serial = fr.Unsigned32();
					uint32_t     const index           = fr.Unsigned32();
					try {
						Map_Object & location = ol->get<Map_Object>(location_serial);

						if (upcast(PlayerImmovable, player_immovable, &location)) {
							if
								(dynamic_cast<Building const *>(player_immovable)
								 or
								 dynamic_cast<Flag     const *>(player_immovable))
							{
								//  We didn't know what kind of ware we were till now,
								//  so no economy might have a clue of us.
								Tribe_Descr const & tribe =
									player_immovable->owner().tribe();
								ware.m_descr =
									tribe.get_ware_descr
										(ware.m_descr_index =
										 	Legacy::ware_index
										 		(tribe,
										 		 player_immovable->descr().descname(),
										 		 "has",
										 		 index));

								ware.set_economy(player_immovable->get_economy());
							} else
								throw game_data_error
									("is PlayerImmovable but not Building or Flag");
						} else if (upcast(Worker, worker, &location)) {
							Tribe_Descr const & tribe = *worker->get_tribe();
							ware.m_descr =
								tribe.get_ware_descr
									(ware.m_descr_index =
									 	Legacy::ware_index
									 		(tribe,
									 		 worker->descr().descname(),
									 		 "has",
									 		 index));

							//  The worker sets our economy.
						} else
							throw game_data_error("is not PlayerImmovable or Worker");
						//  Do not touch supply or transfer.

						if (uint32_t const nextstep_serial = fr.Unsigned32())
							try {
								ware.m_transfer_nextstep =
									&ol->get<PlayerImmovable>(nextstep_serial);
							} catch (_wexception const & e) {
								throw game_data_error
									("nextstep %u: %s", nextstep_serial, e.what());
							}
						else
							ware.m_transfer_nextstep =
								static_cast<PlayerImmovable *>(0);

						//  Do some kind of init.
						ware.set_location
							(ref_cast<Game, Editor_Game_Base>(egbase), &location);
					} catch (_wexception const & e) {
						throw game_data_error
							("location %u: %s", location_serial, e.what());
					}
					ol->mark_object_as_loaded(&ware);
				} catch (_wexception const & e) {
					throw game_data_error(_("item %u: %s"), serial, e.what());
				}
			}
		} else if
			(2 <= packet_version and packet_version <= CURRENT_PACKET_VERSION)
		{
			for (; not fr.EndOfFile();) {
				Serial const serial = fr.Unsigned32();
				try {
					WareInstance & ware = ol->get<WareInstance>(serial);
					ware.m_economy = 0;
					uint32_t     const location_serial = fr.Unsigned32();
					char const * const type_name       = fr.CString   ();
					try {
						Map_Object & location = ol->get<Map_Object>(location_serial);

						if (upcast(PlayerImmovable, player_immovable, &location)) {
							if
								(dynamic_cast<Building const *>(player_immovable)
								 or
								 dynamic_cast<Flag     const *>(player_immovable))
							{
								Tribe_Descr const & tribe =
									player_immovable->owner().tribe();
								ware.m_descr =
									tribe.get_ware_descr
										(ware.m_descr_index =
										 	tribe.safe_ware_index(type_name));
								ware.set_economy(player_immovable->get_economy());
							} else
								throw game_data_error
									("is PlayerImmovable but not Building or Flag");
						} else if (upcast(Worker, worker, &location)) {
							Tribe_Descr const & tribe = *worker->get_tribe();
							ware.m_descr =
								tribe.get_ware_descr
									(ware.m_descr_index =
									 	tribe.safe_ware_index(type_name));
							//  The worker sets our economy.
						} else
							throw game_data_error("is not PlayerImmovable or Worker");
						//  Do not touch supply or transfer.

						if (uint32_t const nextstep_serial = fr.Unsigned32())
							try {
								ware.m_transfer_nextstep =
									&ol->get<PlayerImmovable>(nextstep_serial);
							} catch (_wexception const & e) {
								throw game_data_error
									("nextstep %u: %s", nextstep_serial, e.what());
							}
						else
							ware.m_transfer_nextstep =
								static_cast<PlayerImmovable *>(0);

						//  Do some kind of init.
						ware.set_location
							(ref_cast<Game, Editor_Game_Base>(egbase), &location);
					} catch (_wexception const & e) {
						throw game_data_error
							("location %u: %s", location_serial, e.what());
					}
					ol->mark_object_as_loaded(&ware);
				} catch (_wexception const & e) {
					throw game_data_error(_("item %u: %s"), serial, e.what());
				}
			}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("waredata: %s"), e.what());
	}
}


void Map_Waredata_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  We transverse the map and whenever we find a suitable object, we check
	//  if it has wares of some kind.
	std::vector<uint32_t> ids;
	Map   const & map        = egbase.map();
	Field const & fields_end = map[map.max_index()];
	for (Field const * field = &map[0]; field < &fields_end; ++field) {
		// First, check for Flags
		if (upcast(Flag const, fl, field->get_immovable()))
			for (int32_t i = 0; i < fl->m_item_filled; ++i) {
				assert(os->is_object_known(*fl->m_items[i].item));
				write_ware(fw, egbase, os, *fl->m_items[i].item);
			}

		//  Now, check for workers.
		for (Bob const * b = field->get_first_bob(); b; b = b->get_next_bob())
			if (upcast(Worker const, worker, b))
				if
					(WareInstance const * const ware =
					 	worker->get_carried_item(egbase))
				{
					assert(os->is_object_known(*ware));
					write_ware(fw, egbase, os, *ware);
				}
	}

	fw.Write(fs, "binary/ware_data");
}

/*
 * Write this ware instances data to disk
 */
void Map_Waredata_Data_Packet::write_ware
	(FileWrite            & fw,
	 Editor_Game_Base     & egbase,
	 Map_Map_Object_Saver * os,
	 WareInstance   const & ware)
{
	fw.Unsigned32(os->get_object_file_index(ware));

	if (Map_Object const * const obj = ware.m_location.get(egbase)) {
		assert(os->is_object_known(*obj));
		fw.Unsigned32(os->get_object_file_index(*obj));
	} else
		fw.Unsigned32(0);

	fw.CString(ware.descr().name());

	if (Map_Object const * const obj = ware.m_transfer_nextstep.get(egbase)) {
		assert(os->is_object_known(*obj));
		fw.Unsigned32(os->get_object_file_index(*obj));
	} else
		fw.Unsigned32(0);

	os->mark_object_as_saved(ware);
}

}
