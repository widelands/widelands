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

#include "widelands_map_buildingdata_data_packet.h"

#include "constructionsite.h"
#include "editor_game_base.h"
#include "game.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "production_program.h"
#include "request.h"
#include "soldier.h"
#include "trainingsite.h"
#include "transport.h"
#include "tribe.h"
#include "warehouse.h"
#include "warehousesupply.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"
#include "worker.h"

#include "upcast.h"

#include <map>

namespace Widelands {

// Versions
#define CURRENT_PACKET_VERSION 1

// Subversions
#define CURRENT_CONSTRUCTIONSITE_PACKET_VERSION 1
#define CURRENT_WAREHOUSE_PACKET_VERSION        1
#define CURRENT_MILITARYSITE_PACKET_VERSION     2
#define CURRENT_PRODUCTIONSITE_PACKET_VERSION   1
#define CURRENT_TRAININGSITE_PACKET_VERSION     1


void Map_Buildingdata_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;

	FileRead fr;
	try {fr.Open(fs, "binary/building_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			for (;;) {
				Serial const serial = fr.Unsigned32();
				//  FIXME Just test EndOfFile instead in the next packet version.
				if (serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw wexception
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Building & building = ol->get<Building>(serial);

					building.m_anim =
						fr.Unsigned8() ?
						building.descr().get_animation(fr.CString()) : 0;
					building.m_animstart = fr.Unsigned32();

					{
						Building::Leave_Queue & leave_queue = building.m_leave_queue;
						leave_queue.resize(fr.Unsigned16());
						Building::Leave_Queue::const_iterator const leave_queue_end =
							leave_queue.end();
						for
							(Building::Leave_Queue::iterator it = leave_queue.begin();
							 it != leave_queue_end;
							 ++it)
							if (uint32_t const leaver_serial = fr.Unsigned32())
								try {
									*it = &ol->get<Map_Object>(leaver_serial);
								} catch (_wexception const & e) {
									throw wexception
										("leave queue item #%u (%u): %s",
										 it - leave_queue.begin(), leaver_serial,
										 e.what());
								}
						else
							*it = 0;
					}

					building.m_leave_time = fr.Unsigned32();

					if (uint32_t const leaver_serial = fr.Unsigned32())
						try {
							building.m_leave_allow =
								&ol->get<Map_Object>(leaver_serial);
						} catch (_wexception const & e) {
							throw wexception
								("leave allow item (%u): %s", leaver_serial, e.what());
						}
					else
						building.m_leave_allow = 0;

					building.m_stop = fr.Unsigned8();

					//  Set economy now, some stuff below will count on this.
					building.set_economy(building.m_flag->get_economy());

					if (upcast(ConstructionSite, constructionsite, &building))
						read_constructionsite(*constructionsite, fr, egbase, ol);
					else if (upcast(Warehouse, warehouse, &building))
						read_warehouse(*warehouse, fr, egbase, ol);
					else if (upcast(ProductionSite, productionsite, &building)) {
						if (upcast(MilitarySite, militarysite, productionsite))
							read_militarysite(*militarysite, fr, egbase, ol);
						else if (upcast(TrainingSite, trainingsite, productionsite))
							read_trainingsite(*trainingsite, fr, egbase, ol);
						else read_productionsite(*productionsite, fr, egbase, ol);
					} else
						//  type of building is not one of (or derived from)
						//  {ConstructionSite, Warehouse, ProductionSite}
						assert(false);


					ol->mark_object_as_loaded(&building);
				} catch (_wexception const & e) {
					throw wexception("building %u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("buildingdata: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_constructionsite
(ConstructionSite & constructionsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Map_Map_Object_Loader * const ol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_CONSTRUCTIONSITE_PACKET_VERSION) {
			Tribe_Descr const & tribe = constructionsite.owner().tribe();
			constructionsite.m_building =
				tribe.get_building_descr(tribe.get_safe_building_index(fr.CString()));
			if (fr.Unsigned8()) {
				constructionsite.m_prev_building =
					tribe.get_building_descr
					(tribe.get_safe_building_index(fr.CString()));
			} else
				constructionsite.m_prev_building = 0;

			delete constructionsite.m_builder_request;
			if (fr.Unsigned8()) {
				constructionsite.m_builder_request = new Request
					(&constructionsite,
					 0,
					 ConstructionSite::request_builder_callback,
					 &constructionsite,
					 Request::WORKER);
				constructionsite.m_builder_request->Read(&fr, egbase, ol);
			} else
				constructionsite.m_builder_request = 0;

			if (uint32_t const builder_serial = fr.Unsigned32()) {
				try {
					constructionsite.m_builder = &ol->get<Worker>(builder_serial);
				} catch (_wexception const & e) {
					throw wexception("builder (%u): %s", builder_serial, e.what());
				}
			} else
				constructionsite.m_builder = 0;

			try {
				uint16_t const size = fr.Unsigned16();
				if (constructionsite.m_wares.size() < size)
					throw wexception("constructionsite.m_wares.size() < size");
				upcast(Game, game, egbase);
				for (uint16_t i = size; i < constructionsite.m_wares.size(); ++i) {
					if (game)
						constructionsite.m_wares[i]->cleanup();
					delete constructionsite.m_wares[i];
				}
				constructionsite.m_wares.resize(size);
				for (uint16_t i = 0; i < constructionsite.m_wares.size(); ++i)
					constructionsite.m_wares[i]->Read(&fr, egbase, ol);
			} catch (_wexception const & e) {
				throw wexception("wares: %s", e.what());
			}

			constructionsite.m_fetchfromflag  = fr.  Signed32();

			constructionsite.m_working        = fr.Unsigned8 ();
			constructionsite.m_work_steptime  = fr.Unsigned32();
			constructionsite.m_work_completed = fr.Unsigned32();
			constructionsite.m_work_steps     = fr.Unsigned32();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("constructionsite: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_warehouse
(Warehouse & warehouse,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Map_Map_Object_Loader * const ol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_WAREHOUSE_PACKET_VERSION) {
			log("Reading warehouse stuff for %p\n", &warehouse);
			//  supply
			Tribe_Descr const & tribe = warehouse.owner().tribe();
			while (fr.Unsigned8()) {
				int32_t const id = tribe.get_safe_ware_index(fr.CString());
				warehouse.remove_wares(id, warehouse.m_supply->stock_wares(id));
				warehouse.insert_wares(id, fr.Unsigned16());
			}
			while (fr.Unsigned8()) {
				int32_t const id = tribe.get_safe_worker_index(fr.CString());
				warehouse.remove_workers
					(id, warehouse.m_supply->stock_workers(id));
				warehouse.insert_workers(id, fr.Unsigned16());
			}

			for (uint32_t i = 0; i < warehouse.m_requests.size(); ++i)
				delete warehouse.m_requests[i];
			warehouse.m_requests.resize(fr.Unsigned16());
			for (uint32_t i = 0; i < warehouse.m_requests.size(); ++i) {
				Request & req = *new Request
					(&warehouse,
					 0,
					 Warehouse::idle_request_cb,
					 &warehouse,
					 Request::WORKER);
				req.Read(&fr, egbase, ol);
				warehouse.m_requests[i] = &req;
			}

			assert(warehouse.m_incorporated_workers.empty());
			{
				uint16_t const nr_workers = fr.Unsigned16();
				for (uint16_t i = 0; i < nr_workers; ++i) {
					uint32_t const worker_serial = fr.Unsigned32();
					try {
						//  FIXME Is this really needed?
						char const * const name = fr.CString();
						//  Worker might not yet be loaded so that get ware would not
						//  work but make sure that such a worker type exists.
						if (tribe.get_worker_index(name) == -1)
							throw wexception("unknown worker type \"%s\"", name);

						warehouse.sort_worker_in
							(egbase, name, &ol->get<Worker>(worker_serial));
					} catch (_wexception const & e) {
						throw wexception
							("incorporated worker #%u (%u): %s",
							 i, worker_serial, e.what());
					}
				}
			}

			warehouse.m_next_carrier_spawn = fr.Unsigned32();

			log("Read warehouse stuff for %p\n", &warehouse);
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("warehouse: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_militarysite
(MilitarySite & militarysite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Map_Map_Object_Loader * const ol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_MILITARYSITE_PACKET_VERSION) {
			read_productionsite(militarysite, fr, egbase, ol);

			//  FIXME The reason for this code is probably that the constructor of
			//  FIXME MilitarySite requests soldiers. That makes sense when a
			//  FIXME MilitarySite is created in the game, but definitely not when
			//  FIXME only allocating a MilitarySite to later fill it with
			//  FIXME information from a savegame. Therefore this code here undoes
			//  FIXME what the constructor just did. There should really be
			//  FIXME different constructors for those cases.
			for (uint16_t i = 0; i < militarysite.m_soldier_requests.size(); ++i)
				delete militarysite.m_soldier_requests[i];

			{
				uint16_t const nr_requests = fr.Unsigned16();
				militarysite.m_soldier_requests.resize(nr_requests);
				for (uint16_t i = 0; i < nr_requests; ++i) {
					Request & req =
						*new Request
						(&militarysite,
						 0,
						 MilitarySite::request_soldier_callback,
						 &militarysite,
						 Request::WORKER);
					req.Read(&fr, egbase, ol);
					militarysite.m_soldier_requests[i] = &req;
				}
			}

			assert(militarysite.m_soldiers.empty());
			{
				uint16_t const nr_soldiers = fr.Unsigned16();
				militarysite.m_soldiers.resize(nr_soldiers);
				for (uint16_t i = 0; i < nr_soldiers; ++i) {
					uint32_t const soldier_serial = fr.Unsigned32();
					try {
						militarysite.m_soldiers[i] =
							&ol->get<Soldier>(soldier_serial);
					} catch (_wexception const & e) {
						throw wexception
							("soldier #%u (%u): %s", i, soldier_serial, e.what());
					}
				}
			}

			militarysite.m_didconquer = fr.Unsigned8();

			//  capacity (modified by user)
			militarysite.m_capacity = fr.Unsigned8();
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("militarysite: %s", e.what());
	}
}

void Map_Buildingdata_Data_Packet::read_productionsite
(ProductionSite & productionsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Map_Map_Object_Loader * const ol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			//  FIXME The reason for this code is probably that the constructor of
			//  FIXME ProductionSite requests workers. That makes sense when a
			//  FIXME ProductionSite is created in the game, but definitely not
			//  FIXME when only allocating a ProductionSite to later fill it with
			//  FIXME information from a savegame. Therefore this code here undoes
			//  FIXME what the constructor just did. There should really be
			//  FIXME different constructors for those cases.
			for (uint16_t i = 0; i < productionsite.m_worker_requests.size(); ++i)
				delete productionsite.m_worker_requests[i];

			{
				uint16_t const nr_requests = fr.Unsigned16();
				productionsite.m_worker_requests.resize(nr_requests);
				for (uint16_t i = 0; i < nr_requests; ++i) {
					Request & req =
						*new Request
						(&productionsite,
						 0,
						 ProductionSite::request_worker_callback,
						 &productionsite,
						 Request::WORKER);
					req.Read(&fr, egbase, ol);
					productionsite.m_worker_requests[i] = &req;
				}
			}

			assert(productionsite.m_workers.empty());
			{
				uint16_t const nr_workers = fr.Unsigned16();
				productionsite.m_workers.resize(nr_workers);
				for (uint16_t i = 0; i < nr_workers; ++i) {
					uint32_t const worker_serial = fr.Unsigned32();
					try {
						productionsite.m_workers[i] =
							&ol->get<Worker>(worker_serial);
					} catch (_wexception const & e) {
						throw wexception
							("worker #%u (%u): %s", i, worker_serial, e.what());
					}
				}
			}

			//  items from flags
			productionsite.m_fetchfromflag = fr.Signed32();

			//  state
			uint16_t const nr_progs = fr.Unsigned16();
			productionsite.m_program.resize(nr_progs);
			for (uint16_t i = 0; i < nr_progs; ++i) {
				std::string prog = fr.CString();
				productionsite.m_program[i].program =
					productionsite.descr().get_program(prog.c_str());
				productionsite.m_program[i].ip      = fr.Signed32();
				productionsite.m_program[i].phase   = fr.Signed32();
				productionsite.m_program[i].flags   = fr.Unsigned32();
			}
			productionsite.m_program_timer = fr.Unsigned8();
			productionsite.m_program_time = fr.Signed32();

			uint16_t const nr_queues = fr.Unsigned16();
			if (nr_queues != productionsite.m_input_queues.size())
				throw wexception("wrong number of input queues");
			for (uint16_t i = 0; i < productionsite.m_input_queues.size(); ++i)
				productionsite.m_input_queues[i]->Read(&fr, egbase, ol);

			uint16_t const stats_size = fr.Unsigned16();
			productionsite.m_statistics.resize(stats_size);
			for (uint32_t i = 0; i < productionsite.m_statistics.size(); ++i)
				productionsite.m_statistics[i] = fr.Unsigned8();
			productionsite.m_statistics_changed = fr.Unsigned8();
			memcpy
				(productionsite.m_statistics_buf,
				 fr.Data(sizeof(productionsite.m_statistics_buf)),
				 sizeof(productionsite.m_statistics_buf));
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("militarysite: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_trainingsite
(TrainingSite & trainingsite,
 FileRead & fr,
 Editor_Game_Base* egbase,
 Map_Map_Object_Loader * const ol)
{
	try {
		uint16_t const trainingsite_packet_version = fr.Unsigned16();
		if (trainingsite_packet_version == CURRENT_TRAININGSITE_PACKET_VERSION) {
			read_productionsite(trainingsite, fr, egbase, ol);

			//  FIXME The reason for this code is probably that the constructor of
			//  FIXME TrainingSite requests soldiers. That makes sense when a
			//  FIXME TrainingSite is created in the game, but definitely not when
			//  FIXME only allocating a TrainingSite to later fill it with
			//  FIXME information from a savegame. Therefore this code here undoes
			//  FIXME what the constructor just did. There should really be
			//  FIXME different constructors for those cases.
			for (uint16_t i = 0; i < trainingsite.m_soldier_requests.size(); ++i)
				delete trainingsite.m_soldier_requests[i];

			{
				uint16_t const nr_requests = fr.Unsigned16();
				trainingsite.m_soldier_requests.resize(nr_requests);
				for (uint32_t i = 0; i < nr_requests; ++i) {
					Request & req = *new Request
						(&trainingsite,
						 0,
						 TrainingSite::request_soldier_callback,
						 &trainingsite,
						 Request::WORKER);
					req.Read(&fr, egbase, ol);
					trainingsite.m_soldier_requests[i] = &req;
				}
			}

			assert(trainingsite.m_soldiers.empty());
			{
				uint16_t const nr_soldiers = fr.Unsigned16();
				trainingsite.m_soldiers.resize(nr_soldiers);
				for (uint16_t i = 0; i < nr_soldiers; ++i) {
					uint32_t const soldier_serial = fr.Unsigned32();
					try {
						trainingsite.m_soldiers[i] =
							&ol->get<Soldier>(soldier_serial);
					} catch (_wexception const & e) {
						throw wexception
							("soldier #%u (%u): %s", i, soldier_serial, e.what());
					}
				}
			}

			// Do not save m_list_upgrades (remake at load).

			//  Building heros?
			trainingsite.m_build_heros = fr.Unsigned8();

			//  priority upgrades
			trainingsite.m_pri_hp      = fr.Unsigned16();
			trainingsite.m_pri_attack  = fr.Unsigned16();
			trainingsite.m_pri_defense = fr.Unsigned16();
			trainingsite.m_pri_evade   = fr.Unsigned16();

			//  priority modificators
			trainingsite.m_pri_hp_mod      = fr.Unsigned16();
			trainingsite.m_pri_attack_mod  = fr.Unsigned16();
			trainingsite.m_pri_defense_mod = fr.Unsigned16();
			trainingsite.m_pri_evade_mod   = fr.Unsigned16();

			//  capacity (modified by user)
			trainingsite.m_capacity = fr.Unsigned8();

			//  Need to read the m_prog_name as string !!
			std::string prog = fr.CString();
			trainingsite.m_prog_name = prog;

			//  m_total_soldiers is just a convenience variable and not saved.
			//  Recalculate it.
			trainingsite.m_total_soldiers =
				trainingsite.m_soldiers.size()
				+
				trainingsite.m_soldier_requests.size();
		} else
			throw wexception
				("unknown/unhandled version %u", trainingsite_packet_version);
	} catch (_wexception const & e) {
		throw wexception("trainingsite: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	// now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Walk the map again
	Map & map = egbase->map();
	const uint32_t mapwidth = map.get_width();
	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i)
		if (upcast(Building const, building, map[i].get_immovable())) {
			assert(os->is_object_known(building));

			if (Map::get_index(building->get_position(), mapwidth) != i)
				continue; // This is not this buildings main position.

			fw.Unsigned32(os->get_object_file_index(building));

			//  player immovable owner is already in existance packet

			//  write the general stuff
			if (building->m_anim) {
				fw.Unsigned8(1);
				fw.String(building->descr().get_animation_name(building->m_anim));
			} else
				fw.Unsigned8(0);

			fw.Unsigned32(building->m_animstart);

			{
				const Building::Leave_Queue & leave_queue = building->m_leave_queue;
				const Building::Leave_Queue::const_iterator leave_queue_end =
					leave_queue.end();
				fw.Unsigned16(leave_queue.size());
				for
					(Building::Leave_Queue::const_iterator jt = leave_queue.begin();
					 jt != leave_queue_end;
					 ++jt)
				{
					assert(os->is_object_known(jt->get(egbase)));
					fw.Unsigned32(os->get_object_file_index(jt->get(egbase)));
				}
			}
			fw.Unsigned32(building->m_leave_time);
			if (building->m_leave_allow.get(egbase)) {
				assert(os->is_object_known(building->m_leave_allow.get(egbase)));
				fw.Unsigned32(os->get_object_file_index(building->m_leave_allow.get(egbase)));
			} else
				fw.Unsigned32(0);
			fw.Unsigned8(building->m_stop);

			if (upcast(ConstructionSite const, constructionsite, building))
				write_constructionsite(*constructionsite, fw, egbase, os);
			else if (upcast(Warehouse const, warehouse, building))
				write_warehouse(*warehouse, fw, egbase, os);
			else if (upcast(ProductionSite const, productionsite, building)) {
				if (upcast(MilitarySite const, militarysite, productionsite))
					write_militarysite(*militarysite, fw, egbase, os);
				else if (upcast(TrainingSite const, trainingsite, productionsite))
					write_trainingsite(*trainingsite, fw, egbase, os);
				else write_productionsite(*productionsite, fw, egbase, os);
			} else {
				assert(false);
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
			}

			os->mark_object_as_saved(building);
		}

	//  FIXME Remove this in the next packet version. End of file is enough.
	fw.Unsigned32(0xffffffff);

	fw.Write(fs, "binary/building_data");
}


void Map_Buildingdata_Data_Packet::write_constructionsite
(const ConstructionSite & constructionsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
{

	fw.Unsigned16(CURRENT_CONSTRUCTIONSITE_PACKET_VERSION);

	//  descriptions
	fw.String(constructionsite.m_building->name());
	//  FIXME Just write the string without the 1 first:
	//  FIXME   fw.CString(constructionsite.m_prev_building ? constructionsite.m_prev_building->name().c_str() : "");
	//  FIXME When reading, the empty string should mean no prev_building.
	if (constructionsite.m_prev_building) {
		fw.Unsigned8(1);
		fw.String(constructionsite.m_prev_building->name());
	} else
		fw.Unsigned8(0);

	// builder request
	if (constructionsite.m_builder_request) {
		fw.Unsigned8(1);
		constructionsite.m_builder_request->Write(&fw, egbase, os);
	} else
		fw.Unsigned8(0);

	// builder
	const Worker* builder = constructionsite.m_builder.get(egbase);
	if (builder) {
		assert(os->is_object_known(builder));
		fw.Unsigned32(os->get_object_file_index(builder));
	} else
		fw.Unsigned32(0);

	const uint16_t wares_size = constructionsite.m_wares.size();
	fw.Unsigned16(wares_size);
	for (uint16_t i = 0; i < wares_size; ++i)
		constructionsite.m_wares[i]->Write(&fw, egbase, os);

	fw.  Signed32(constructionsite.m_fetchfromflag);

	fw.Unsigned8 (constructionsite.m_working);
	fw.Unsigned32(constructionsite.m_work_steptime);
	fw.Unsigned32(constructionsite.m_work_completed);
	fw.Unsigned32(constructionsite.m_work_steps);
}


void Map_Buildingdata_Data_Packet::write_warehouse
(const Warehouse & warehouse,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
{
	fw.Unsigned16(CURRENT_WAREHOUSE_PACKET_VERSION);

	//  supply
	Tribe_Descr const & tribe = warehouse.owner().tribe();
	WareList const & wares = warehouse.m_supply->get_wares();
	for (Ware_Index::value_t i = 0; i < wares.get_nrwareids(); ++i) {
		fw.Unsigned8(1);
		fw.String(tribe.get_ware_descr(i)->name());
		fw.Unsigned16(wares.stock(i));
	}
	fw.Unsigned8(0);
	WareList const & workers = warehouse.m_supply->get_workers();
	for (Ware_Index::value_t i = 0; i < workers.get_nrwareids(); ++i) {
		fw.Unsigned8(1);
		fw.String(tribe.get_worker_descr(i)->name());
		fw.Unsigned16(workers.stock(i));
	}
	fw.Unsigned8(0);

	const uint16_t requests_size = warehouse.m_requests.size();
	fw.Unsigned16(requests_size);
	for (uint16_t i = 0; i < warehouse.m_requests.size(); ++i)
		warehouse.m_requests[i]->Write(&fw, egbase, os);

	//  Incorporated workers, write sorted after file-serial.
	fw.Unsigned16(warehouse.m_incorporated_workers.size());
	std::map<uint32_t, const Worker *> workermap;
	const std::vector<Object_Ptr>::const_iterator iw_end =
		warehouse.m_incorporated_workers.end();
	for
		(std::vector<Object_Ptr>::const_iterator it =
		 warehouse.m_incorporated_workers.begin();
		 it != iw_end;
		 ++it)
	{
		assert(os->is_object_known(it->get(egbase)));
		workermap.insert
			(std::pair<uint32_t, const Worker *>
			 (os->get_object_file_index(it->get(egbase)),
			  static_cast<const Worker *>(it->get(egbase))));
	}

	for
		(std::map<uint32_t, const Worker *>::const_iterator it =
		 workermap.begin();
		 it != workermap.end();
		 ++it)
	{
		assert(os->is_object_known(it->second));
		fw.Unsigned32(os->get_object_file_index(it->second));
		fw.String(it->second->name()); //  FIXME is this really needed? If not, remove in the next packet version.
	}

	fw.Unsigned32(warehouse.m_next_carrier_spawn);
}


void Map_Buildingdata_Data_Packet::write_militarysite
(const MilitarySite & militarysite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
{
	fw.Unsigned16(CURRENT_MILITARYSITE_PACKET_VERSION);
	write_productionsite(militarysite, fw, egbase, os);

	const uint16_t soldier_requests_size =
		militarysite.m_soldier_requests.size();
	fw.Unsigned16(soldier_requests_size);
	for (uint16_t i = 0; i < soldier_requests_size; ++i)
		militarysite.m_soldier_requests[i]->Write(&fw, egbase, os);

	//  soldier
	const uint16_t soldiers_size = militarysite.m_soldiers.size();
	fw.Unsigned16(soldiers_size);
	for (uint32_t i = 0; i < soldiers_size; ++i) {
		assert(os->is_object_known(militarysite.m_soldiers[i]));
		fw.Unsigned32(os->get_object_file_index(militarysite.m_soldiers[i]));
	}

	fw.Unsigned8(militarysite.m_didconquer);
	fw.Unsigned8(militarysite.m_capacity);
}


void Map_Buildingdata_Data_Packet::write_productionsite
(const ProductionSite & productionsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
{
	fw.Unsigned16(CURRENT_PRODUCTIONSITE_PACKET_VERSION);

	const uint16_t worker_requests_size =
		productionsite.m_worker_requests.size();
	fw.Unsigned16(worker_requests_size);
	for (uint16_t i = 0; i < productionsite.m_worker_requests.size(); ++i)
		productionsite.m_worker_requests[i]->Write(&fw, egbase, os);

	//  workers
	const uint16_t workers_size = productionsite.m_workers.size();
	fw.Unsigned16(workers_size);
	for (uint16_t i = 0; i < workers_size; ++i) {
		assert(os->is_object_known(productionsite.m_workers[i]));
		fw.Unsigned32(os->get_object_file_index(productionsite.m_workers[i]));
	}

	fw.Signed32(productionsite.m_fetchfromflag);

	//  state
	const uint16_t program_size = productionsite.m_program.size();
	fw.Unsigned16(program_size);
	for (uint16_t i = 0; i < program_size; ++i) {
		fw.String(productionsite.m_program[i].program->get_name());
		fw.  Signed32(productionsite.m_program[i].ip);
		fw.  Signed32(productionsite.m_program[i].phase);
		fw.Unsigned32(productionsite.m_program[i].flags);
	}
	fw.Unsigned8(productionsite.m_program_timer);
	fw. Signed32(productionsite.m_program_time);

	const uint16_t input_queues_size = productionsite.m_input_queues.size();
	fw.Unsigned16(input_queues_size);
	for (uint16_t i = 0; i < input_queues_size; ++i)
		productionsite.m_input_queues[i]->Write(&fw, egbase, os);

	const uint16_t statistics_size = productionsite.m_statistics.size();
	fw.Unsigned16(statistics_size);
	for (uint32_t i = 0; i < statistics_size; ++i)
		fw.Unsigned8(productionsite.m_statistics[i]);
	fw.Unsigned8(productionsite.m_statistics_changed);
	fw.Data
		(productionsite.m_statistics_buf,
		 sizeof(productionsite.m_statistics_buf),
		 FileWrite::Pos::Null());
}

/*
 * write for trainingsite
 */
void Map_Buildingdata_Data_Packet::write_trainingsite
(const TrainingSite & trainingsite,
 FileWrite & fw,
 Editor_Game_Base* egbase,
 Map_Map_Object_Saver * const os)
{
	fw.Unsigned16(CURRENT_TRAININGSITE_PACKET_VERSION);

	write_productionsite(trainingsite, fw, egbase, os);

	//  requests
	const uint16_t soldier_requests_size =
		trainingsite.m_soldier_requests.size();
	fw.Unsigned16(soldier_requests_size);
	for (uint32_t i = 0; i < soldier_requests_size; ++i)
		trainingsite.m_soldier_requests[i]->Write(&fw, egbase, os);


	//  soldiers
	const uint16_t soldiers_size = trainingsite.m_soldiers.size();
	fw.Unsigned16(soldiers_size);
	for (uint32_t i = 0; i < trainingsite.m_soldiers.size(); ++i) {
		assert(os->is_object_known(trainingsite.m_soldiers[i]));
		fw.Unsigned32(os->get_object_file_index(trainingsite.m_soldiers[i]));
	}

	// Don't save m_list_upgrades (remake at load)

	fw.Unsigned8(trainingsite.m_build_heros);

	//  priority upgrades
	fw.Unsigned16(trainingsite.m_pri_hp);
	fw.Unsigned16(trainingsite.m_pri_attack);
	fw.Unsigned16(trainingsite.m_pri_defense);
	fw.Unsigned16(trainingsite.m_pri_evade);

	//  priority modificators
	fw.Unsigned16(trainingsite.m_pri_hp_mod);
	fw.Unsigned16(trainingsite.m_pri_attack_mod);
	fw.Unsigned16(trainingsite.m_pri_defense_mod);
	fw.Unsigned16(trainingsite.m_pri_evade_mod);

	// capacity (modified by user)
	fw.Unsigned8(trainingsite.m_capacity);

	// Need to read the m_prog_name as string !!
	fw.String(trainingsite.m_prog_name);

	// DONE
}

};
