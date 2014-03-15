/*
 * Copyright (C) 2002-2004, 2006-2011, 2013 by the Widelands Development Team
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

#include "map_io/widelands_map_buildingdata_data_packet.h"

#include <map>

#include <boost/foreach.hpp>

#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/warehousesupply.h"
#include "economy/wares_queue.h"
#include "logic/constructionsite.h"
#include "logic/dismantlesite.h"
#include "logic/editor_game_base.h"
#include "logic/expedition_bootstrap.h"
#include "logic/game.h"
#include "logic/map.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/production_program.h"
#include "logic/productionsite.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "logic/worker.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"

namespace Widelands {

// Versions
// Since V3: m_old_buildings vector
#define CURRENT_PACKET_VERSION 3

// Subversions
#define CURRENT_DISMANTLESITE_PACKET_VERSION    1
// Since V3: m_prev_building not written
#define CURRENT_CONSTRUCTIONSITE_PACKET_VERSION 3
#define CURRENT_PARTIALLYFB_PACKET_VERSION      1
#define CURRENT_WAREHOUSE_PACKET_VERSION        6
#define CURRENT_MILITARYSITE_PACKET_VERSION     4
#define CURRENT_PRODUCTIONSITE_PACKET_VERSION   5
#define CURRENT_TRAININGSITE_PACKET_VERSION     4


void Map_Buildingdata_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &       mol)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/building_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			for (;;) {
				if (2 <= packet_version and fr.EndOfFile())
					break;
				Serial const serial = fr.Unsigned32();
				if (packet_version < 2 and serial == 0xffffffff) {
					if (not fr.EndOfFile())
						throw game_data_error
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Building & building = mol.get<Building>(serial);

					if (fr.Unsigned8()) {
						char const * const animation_name = fr.CString();
						try {
							building.m_anim =
								building.descr().get_animation(animation_name);
						} catch (const Map_Object_Descr::Animation_Nonexistent &) {
							log
								("WARNING: %s %s does not have animation \"%s\"; "
								 "using animation \"idle\" instead\n",
								 building.descr().tribe().name().c_str(),
								 building.descname().c_str(),
								 animation_name);
							building.m_anim = building.descr().get_animation("idle");
						}
					} else
						building.m_anim = 0;
					building.m_animstart = fr.Unsigned32();

					{
						Building::Leave_Queue & leave_queue = building.m_leave_queue;
						leave_queue.resize(fr.Unsigned16());
						container_iterate(Building::Leave_Queue, leave_queue, i)
							if (uint32_t const leaver_serial = fr.Unsigned32())
								try {
									//  The check that this worker actually has a
									//  leavebuilding task for this building is in
									//  Building::load_finish, which is called after the
									//  worker (with his stack of tasks) has been fully
									//  loaded.
									*i.current = &mol.get<Worker>(leaver_serial);
								} catch (const _wexception & e) {
									throw game_data_error
										("leave queue item #%lu (%u): %s",
										 static_cast<long int>
										 	(i.current - leave_queue.begin()),
										 leaver_serial, e.what());
								}
							else
								*i.current = nullptr;
					}

					building.m_leave_time = fr.Unsigned32();

					if (uint32_t const leaver_serial = fr.Unsigned32())
						try {
							building.m_leave_allow =
								&mol.get<Map_Object>(leaver_serial);
						} catch (const _wexception & e) {
							throw game_data_error
								("leave allow item (%u): %s", leaver_serial, e.what());
						}
					else {
						building.m_leave_allow = nullptr;
					}
					if (packet_version >= 3) {
						// For former versions, the former buildings vector
						// will be built after other data are loaded, see below.
						// read_formerbuildings_v2()
						while (fr.Unsigned8()) {
							Building_Index oldidx = building.descr().tribe().safe_building_index(fr.CString());
							building.m_old_buildings.push_back(oldidx);
						}
						// Only construction sites may have an empty list
						if (building.m_old_buildings.empty() && !is_a(ConstructionSite, &building)) {
							throw game_data_error
								("Failed to read %s %u: No former buildings informations.\n"
								"Your savegame is corrupted", building.descr().descname().c_str(), building.serial());
						}
					}
					if (fr.Unsigned8()) {
						if (upcast(ProductionSite, productionsite, &building))
							if (dynamic_cast<MilitarySite const *>(productionsite)) {
								log
									("WARNING: Found a stopped %s at (%i, %i) in the "
									 "savegame. Militarysites are not stoppable. "
									 "Ignoring.",
									 building.descname().c_str(),
									 building.get_position().x,
									 building.get_position().y);
							} else {
								productionsite->set_stopped(true);
							}
						else
							log
								("WARNING: Found a stopped %s at (%i, %i) in the "
								 "savegame. Only productionsites are stoppable. "
								 "Ignoring.",
								 building.descname().c_str(),
								 building.get_position().x,
								 building.get_position().y);
					}

					//  Set economy now, some stuff below will count on this.
					building.set_economy(building.m_flag->get_economy());

					if (upcast(ConstructionSite, constructionsite, &building)) {
						read_constructionsite
							(*constructionsite,
							 fr,
							 ref_cast<Game, Editor_Game_Base>(egbase),
							 mol);
					} else if (upcast(DismantleSite, dms, &building)) {
						read_dismantlesite
							(*dms,
							 fr,
							 ref_cast<Game, Editor_Game_Base>(egbase),
							 mol);
					} else if (upcast(Warehouse, warehouse, &building)) {
						read_warehouse
							(*warehouse,
							 fr,
							 ref_cast<Game, Editor_Game_Base>(egbase),
							 mol);
					} else if (upcast(ProductionSite, productionsite, &building)) {
						if (upcast(MilitarySite, militarysite, productionsite)) {
							read_militarysite
								(*militarysite,
								 fr,
								 ref_cast<Game, Editor_Game_Base>(egbase),
								 mol);
						} else if (upcast(TrainingSite, trainingsite, productionsite)) {
							read_trainingsite
								(*trainingsite,
								 fr,
								 ref_cast<Game, Editor_Game_Base>(egbase),
								 mol);
						} else {
							read_productionsite
								(*productionsite,
								 fr,
								 ref_cast<Game, Editor_Game_Base>(egbase),
								 mol);
						}
					} else {
						//  type of building is not one of (or derived from)
						//  {ConstructionSite, Warehouse, ProductionSite}
						assert(false);
					}
					if (packet_version < 3) {
						read_formerbuildings_v2
							(building, fr, ref_cast<Game, Editor_Game_Base>(egbase), mol);
					}

					mol.mark_object_as_loaded(building);
				} catch (const _wexception & e) {
					throw game_data_error("building %u: %s", serial, e.what());
				}
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("buildingdata: %s", e.what());
	}
}

void Map_Buildingdata_Data_Packet::read_formerbuildings_v2
	(Building& b, FileRead&, Game&, Map_Map_Object_Loader&)
{
	const Tribe_Descr & t = b.descr().tribe();
	Building_Index b_idx = t.building_index(b.descr().name());
	if (is_a(ProductionSite, &b)) {
		assert(b.m_old_buildings.empty());
		b.m_old_buildings.push_back(b_idx);
	} else if (is_a(Warehouse, &b)) {
		assert(b.m_old_buildings.empty());
		b.m_old_buildings.push_back(b_idx);
	} else if (is_a(DismantleSite, &b)) {
		// Former buildings filled with the current one
		// upon building init.
		assert(!b.m_old_buildings.empty());
	} else if (is_a(ConstructionSite, &b)) {
		// Not needed for csite.
		return;
	} else {
		assert(false);
	}

	// iterate through all buildings to find first predecessor
	for (;;) {
		Building_Index former_idx = b.m_old_buildings.front();
		const Building_Descr * oldest = t.get_building_descr(former_idx);
		if (!oldest->is_enhanced()) {
			break;
		}
		for (Building_Index i = Building_Index::First(); i < t.get_nrbuildings(); ++i) {
			Building_Descr const * ob = t.get_building_descr(i);
			if (ob->enhancements().count(former_idx)) {
				b.m_old_buildings.insert(b.m_old_buildings.begin(), i);
				break;
			}
		}
	}
}


void Map_Buildingdata_Data_Packet::read_partially_finished_building
	(Partially_Finished_Building  & pfb,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PARTIALLYFB_PACKET_VERSION) {
			const Tribe_Descr & tribe = pfb.tribe();
			pfb.m_building =
				tribe.get_building_descr(tribe.safe_building_index(fr.CString()));

			delete pfb.m_builder_request;
			if (fr.Unsigned8()) {
				pfb.m_builder_request =
					new Request
					(pfb,
					 Ware_Index::First(),
					 Partially_Finished_Building::request_builder_callback,
					 wwWORKER);
				pfb.m_builder_request->Read(fr, game, mol);
			} else
				pfb.m_builder_request = nullptr;

			if (uint32_t const builder_serial = fr.Unsigned32()) {
				try {
					pfb.m_builder = &mol.get<Worker>(builder_serial);
				} catch (const _wexception & e) {
					throw game_data_error
						("builder (%u): %s", builder_serial, e.what());
				}
			} else
				pfb.m_builder = nullptr;

			try {
				uint16_t const size = fr.Unsigned16();
				pfb.m_wares.resize(size);
				for (uint16_t i = 0; i < pfb.m_wares.size(); ++i)
				{
					pfb.m_wares[i] =
						new WaresQueue
						(pfb, Ware_Index::Null(), 0);
					pfb.m_wares[i]->Read(fr, game, mol);
				}
			} catch (const _wexception & e) {
				throw game_data_error("wares: %s", e.what());
			}

			pfb.m_working        = fr.Unsigned8 ();
			pfb.m_work_steptime  = fr.Unsigned32();
			pfb.m_work_completed = fr.Unsigned32();
			pfb.m_work_steps     = fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("partially_constructed_buildings: %s", e.what());
	}
}

void Map_Buildingdata_Data_Packet::read_constructionsite
	(ConstructionSite      & constructionsite,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == 1)
			return read_constructionsite_v1(constructionsite, fr, game, mol);

		if (packet_version >= 2) {
			read_partially_finished_building(constructionsite, fr, game, mol);

			const Tribe_Descr & tribe = constructionsite.tribe();

			container_iterate
				(ConstructionSite::Wares, constructionsite.m_wares, cur)
					(*cur)->set_callback
						(ConstructionSite::wares_queue_callback, &constructionsite);

			if (packet_version <= 2) {
				if (fr.Unsigned8()) {
					Building_Index idx = tribe.safe_building_index(fr.CString());
					constructionsite.m_old_buildings.push_back(idx);
				}
			}

			constructionsite.m_fetchfromflag  = fr.  Signed32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("constructionsite: %s", e.what());
	}
}

void Map_Buildingdata_Data_Packet::read_constructionsite_v1
	(ConstructionSite      & constructionsite,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	const Tribe_Descr & tribe = constructionsite.tribe();
	constructionsite.m_building =
		tribe.get_building_descr(tribe.safe_building_index(fr.CString()));
	if (fr.Unsigned8()) {
		Building_Index bidx = tribe.safe_building_index(fr.CString());
		constructionsite.m_old_buildings.push_back(bidx);
	}

	delete constructionsite.m_builder_request;
	if (fr.Unsigned8()) {
		constructionsite.m_builder_request =
			new Request
			(constructionsite,
			 Ware_Index::First(),
			 ConstructionSite::request_builder_callback,
			 wwWORKER);
		constructionsite.m_builder_request->Read(fr, game, mol);
	} else
		constructionsite.m_builder_request = nullptr;

	if (uint32_t const builder_serial = fr.Unsigned32()) {
		try {
			constructionsite.m_builder = &mol.get<Worker>(builder_serial);
		} catch (const _wexception & e) {
			throw game_data_error
				("builder (%u): %s", builder_serial, e.what());
		}
	} else
		constructionsite.m_builder = nullptr;

	try {
		uint16_t const size = fr.Unsigned16();
		constructionsite.m_wares.resize(size);
		for (uint16_t i = 0; i < constructionsite.m_wares.size(); ++i)
		{
			constructionsite.m_wares[i] =
				new WaresQueue
				(constructionsite, Ware_Index::Null(), 0);
			constructionsite.m_wares[i]->set_callback
				(ConstructionSite::wares_queue_callback, &constructionsite);
			constructionsite.m_wares[i]->Read(fr, game, mol);
		}
	} catch (const _wexception & e) {
		throw game_data_error("wares: %s", e.what());
	}

	constructionsite.m_fetchfromflag  = fr.  Signed32();

	constructionsite.m_working        = fr.Unsigned8 ();
	constructionsite.m_work_steptime  = fr.Unsigned32();
	constructionsite.m_work_completed = fr.Unsigned32();
	constructionsite.m_work_steps     = fr.Unsigned32();
}

void Map_Buildingdata_Data_Packet::read_dismantlesite
	(DismantleSite         & dms,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_DISMANTLESITE_PACKET_VERSION) {
			read_partially_finished_building(dms, fr, game, mol);

			// Nothing to do
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("dismantlesite: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_warehouse
	(Warehouse             & warehouse,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if
			(1 <= packet_version and
			 packet_version <= CURRENT_WAREHOUSE_PACKET_VERSION)
		{
			Ware_Index const nr_wares   = warehouse.tribe().get_nrwares  ();
			Ware_Index const nr_tribe_workers = warehouse.tribe().get_nrworkers();
			warehouse.m_supply->set_nrwares  (nr_wares);
			warehouse.m_supply->set_nrworkers(nr_tribe_workers);
			warehouse.m_ware_policy.resize(nr_wares.value(), Warehouse::SP_Normal);
			warehouse.m_worker_policy.resize
				(nr_tribe_workers.value(), Warehouse::SP_Normal);
			//log("Reading warehouse stuff for %p\n", &warehouse);
			//  supply
			const Tribe_Descr & tribe = warehouse.tribe();
			while (fr.Unsigned8()) {
				Ware_Index const id = tribe.ware_index(fr.CString());
				if (packet_version >= 5) {
					uint32_t amount = fr.Unsigned32();
					Warehouse::StockPolicy policy =
						static_cast<Warehouse::StockPolicy>(fr.Unsigned8());

					if (id) {
						warehouse.insert_wares(id, amount);
						warehouse.set_ware_policy(id, policy);
					}
				} else {
					uint16_t amount = fr.Unsigned16();

					if (id)
						warehouse.insert_wares(id, amount);
				}
			}
			while (fr.Unsigned8()) {
				Ware_Index const id = tribe.worker_index(fr.CString());
				if (packet_version >= 5) {
					uint32_t amount = fr.Unsigned32();
					Warehouse::StockPolicy policy =
						static_cast<Warehouse::StockPolicy>(fr.Unsigned8());

					if (id) {
						warehouse.insert_workers(id, amount);
						warehouse.set_worker_policy(id, policy);
					}
				} else {
					uint16_t amount = fr.Unsigned16();

					if (id)
						warehouse.insert_workers(id, amount);
				}
			}

			if (packet_version <= 3) {
				// eat the obsolete idle request structures
				uint32_t nrrequests = fr.Unsigned16();
				while (nrrequests--) {
					std::unique_ptr<Request> req
						(new Request
						 	(warehouse,
						 	 Ware_Index::First(),
						 	 &Warehouse::request_cb,
						 	 wwWORKER));
					req->Read(fr, game, mol);
				}
			}

			assert(warehouse.m_incorporated_workers.empty());
			{
				uint16_t const nr_workers = fr.Unsigned16();
				for (uint16_t i = 0; i < nr_workers; ++i) {
					uint32_t const worker_serial = fr.Unsigned32();

					try {
						Worker & worker = mol.get<Worker>(worker_serial);
						if (1 == packet_version) {
							char const * const name = fr.CString();
							if (name != worker.name())
								throw game_data_error
									("expected %s but found \"%s\"",
									 worker.name().c_str(), name);
						}
						Ware_Index worker_index = tribe.worker_index(worker.name().c_str());
						if (!warehouse.m_incorporated_workers.count(worker_index))
							warehouse.m_incorporated_workers[worker_index] = std::vector<Worker *>();
						warehouse.m_incorporated_workers[worker_index].push_back(&worker);
					} catch (const _wexception & e) {
						throw game_data_error
							("incorporated worker #%u (%u): %s",
							 i, worker_serial, e.what());
					}
				}
			}

			const std::vector<Ware_Index> & worker_types_without_cost =
				tribe.worker_types_without_cost();

			if (1 == packet_version) { //  a single next_spawn time for "carrier"
				uint32_t const next_spawn = fr.Unsigned32();
				Ware_Index const worker_index =
					tribe.safe_worker_index("carrier");
				if (not worker_index) {
					log
						("WARNING: %s %u has a next_spawn time for nonexistent "
						 "worker type \"%s\" set to %u, ignoring\n",
						 warehouse.descname().c_str(), warehouse.serial(),
						 "carrier", next_spawn);
				} else if
					(tribe.get_worker_descr(worker_index)->buildcost().size())
				{
					log
						("WARNING: %s %u has a next_spawn time for worker type "
						 "\"%s\", that costs something to build, set to %u, "
						 "ignoring\n",
						 warehouse.descname().c_str(), warehouse.serial(),
						 "carrier", next_spawn);
				} else
					for (uint8_t i = 0;; ++i) {
						assert(i < worker_types_without_cost.size());
						if (worker_types_without_cost.at(i) == worker_index) {
							if
								(warehouse.m_next_worker_without_cost_spawn[i]
								 !=
								 static_cast<uint32_t>(Never()))
							{
								warehouse.molog
									("read_warehouse: "
									 "m_next_worker_without_cost_spawn[%u] = %u\n",
									 i, warehouse.m_next_worker_without_cost_spawn[i]);
							}
							assert
								(warehouse.m_next_worker_without_cost_spawn[i]
								 ==
								 static_cast<uint32_t>(Never()));
							warehouse.m_next_worker_without_cost_spawn[i] =
								next_spawn;
							break;
						}
					}
			} else
				for (;;) {
					char const * const worker_typename = fr.CString   ();
					if (not *worker_typename) //  encountered the terminator ("")
						break;
					uint32_t     const next_spawn      = fr.Unsigned32();
					Ware_Index   const worker_index    =
						tribe.safe_worker_index(worker_typename);
					if (not worker_index) {
						log
							("WARNING: %s %u has a next_spawn time for nonexistent "
							 "worker type \"%s\" set to %u, ignoring\n",
							 warehouse.descname().c_str(), warehouse.serial(),
							 worker_typename, next_spawn);
						continue;
					}
					if (tribe.get_worker_descr(worker_index)->buildcost().size()) {
						log
							("WARNING: %s %u has a next_spawn time for worker type "
							 "\"%s\", that costs something to build, set to %u, "
							 "ignoring\n",
							 warehouse.descname().c_str(), warehouse.serial(),
							 worker_typename, next_spawn);
						continue;
					}
					for (uint8_t i = 0;; ++i) {
						assert(i < worker_types_without_cost.size());
						if (worker_types_without_cost.at(i) == worker_index) {
							if
								(warehouse.m_next_worker_without_cost_spawn[i]
								 !=
								 static_cast<uint32_t>(Never()))
								throw game_data_error
									(
									 "%s %u has a next_spawn time for worker type "
									 "\"%s\" set to %u, but it was previously set "
									 "to %u\n",
									 warehouse.descname().c_str(), warehouse.serial(),
									 worker_typename, next_spawn,
									 warehouse.m_next_worker_without_cost_spawn[i]);
							warehouse.m_next_worker_without_cost_spawn[i] =
								next_spawn;
							break;
						}
					}
				}
				//  The checks that the warehouse has a next_spawn time for each
				//  worker type that the player is allowed to spawn, is in
				//  Warehouse::load_finish.

			if (packet_version >= 3) {
				// Read planned worker data
				// Consistency checks are in Warehouse::load_finish
				uint32_t nr_planned_workers = fr.Unsigned32();
				while (nr_planned_workers--) {
					warehouse.m_planned_workers.push_back
						(Warehouse::PlannedWorkers());
					Warehouse::PlannedWorkers & pw =
						warehouse.m_planned_workers.back();
					pw.index = tribe.worker_index(fr.CString());
					pw.amount = fr.Unsigned32();

					uint32_t nr_requests = fr.Unsigned32();
					while (nr_requests--) {
						pw.requests.push_back
							(new Request
							 	(warehouse,
							 	 Ware_Index::First(),
							 	 &Warehouse::request_cb,
							 	 wwWORKER));
						pw.requests.back()->Read(fr, game, mol);
					}
				}
			}

			if (packet_version >= 5)
				warehouse.m_next_stock_remove_act = fr.Unsigned32();

			if (packet_version >= 6) {
				if (warehouse.descr().get_isport()) {
					if (Serial portdock = fr.Unsigned32()) {
						warehouse.m_portdock = &mol.get<PortDock>(portdock);
						warehouse.m_portdock->set_economy(warehouse.get_economy());
						// Expedition specific stuff. This is done in this packet
						// because the "new style" loader is not supported and
						// doesn't lend itself to request and other stuff.
						if (warehouse.m_portdock->expedition_started()) {
							warehouse.m_portdock->expedition_bootstrap()->load
								(packet_version, warehouse, fr, game, mol);
						}
					}
				}
			}

			if (uint32_t const conquer_radius = warehouse.get_conquers()) {
				//  Add to map of military influence.
				const Map & map = game.map();
				Area<FCoords> a
					(map.get_fcoords(warehouse.get_position()), conquer_radius);
				const Field & first_map_field = map[0];
				Player::Field * const player_fields =
					warehouse.owner().m_fields;
				MapRegion<Area<FCoords> > mr(map, a);
				do
					player_fields[mr.location().field - &first_map_field]
					.military_influence
						+= map.calc_influence(mr.location(), Area<>(a, a.radius));
				while (mr.advance(map));
			}
			warehouse.owner().see_area
				(Area<FCoords>
				 (game.map().get_fcoords(warehouse.get_position()),
				  warehouse.vision_range()));
			warehouse.m_next_military_act = game.get_gametime();
			//log("Read warehouse stuff for %p\n", &warehouse);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("warehouse: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_militarysite
	(MilitarySite          & militarysite,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		bool rel17comp = false;
		if (3 == packet_version and 4 == CURRENT_MILITARYSITE_PACKET_VERSION)
			rel17comp = true;
		if (packet_version == CURRENT_MILITARYSITE_PACKET_VERSION or rel17comp)
		{
			read_productionsite(militarysite, fr, game, mol);

			militarysite.m_normal_soldier_request.reset();

			if (fr.Unsigned8()) {
				militarysite.m_normal_soldier_request.reset
					(new Request
						(militarysite,
						 Ware_Index::First(),
						 MilitarySite::request_soldier_callback,
						 wwWORKER));
				militarysite.m_normal_soldier_request->Read(fr, game, mol);
			}
			else
				militarysite.m_normal_soldier_request.reset();

			if (rel17comp) // compatibility with release 17 savegames
				militarysite.m_upgrade_soldier_request.reset();
			else
			if (fr.Unsigned8())
			{
				militarysite.m_upgrade_soldier_request.reset
					(new Request
						(militarysite,
						 (!militarysite.m_normal_soldier_request) ? Ware_Index::First()
						: militarysite.descr().tribe().safe_worker_index("soldier"),
						MilitarySite::request_soldier_callback,
						wwWORKER));
				militarysite.m_upgrade_soldier_request->Read(fr, game, mol);
			}
			else
				militarysite.m_upgrade_soldier_request.reset();


			if ((militarysite.m_didconquer = fr.Unsigned8())) {
				//  Add to map of military influence.
				const Map & map = game.map();
				Area<FCoords> a
					(map.get_fcoords(militarysite.get_position()),
					 militarysite.get_conquers());
				const Field & first_map_field = map[0];
				Player::Field * const player_fields =
					militarysite.owner().m_fields;
				MapRegion<Area<FCoords> > mr(map, a);
				do
					player_fields[mr.location().field - &first_map_field]
					.military_influence
						+= map.calc_influence(mr.location(), Area<>(a, a.radius));
				while (mr.advance(map));
			}

			//  capacity (modified by user)
			militarysite.m_capacity = fr.Unsigned8();
			militarysite.m_nexthealtime = fr.Signed32();
			if (not (rel17comp)) // compatibility with release 17 savegames
			{

				uint16_t reqmin = fr.Unsigned16();
				uint16_t reqmax = fr.Unsigned16();
				militarysite.m_soldier_upgrade_requirements = RequireAttribute(atrTotal, reqmin, reqmax);
				militarysite.m_soldier_preference = static_cast<MilitarySite::SoldierPreference>(fr.Unsigned8());
				militarysite.m_next_swap_soldiers_time = fr.Signed32();
				militarysite.m_soldier_upgrade_try = 0 != fr.Unsigned8() ? true : false;
				militarysite.m_doing_upgrade_request = 0 != fr.Unsigned8() ? true : false;
			}
			else // Release 17 compatibility branch. Some safe values.
			{
				militarysite.m_soldier_preference = MilitarySite::kPrefersRookies;
				if (2 < militarysite.m_capacity)
					militarysite.m_soldier_preference = MilitarySite::kPrefersHeroes;
				militarysite.m_next_swap_soldiers_time = militarysite.m_nexthealtime;
				militarysite.m_soldier_upgrade_try = false;
				militarysite.m_doing_upgrade_request = false;
			}

		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);

		//  If the site's capacity is outside the allowed range (can happen if
		//  the site's type's definition has changed), set the variable to the
		//  nearest valid value.
		//
		//  This does not drop excessive soldiers, since they are not loaded into
		//  the site yet. To do that we would have to do this change by adding a
		//  Cmd_ChangeSoldierCapacity to the beginning of the game's command
		//  queue. But that would not work because the command queue is not read
		//  yet and will be cleared before it is read.
		if        (militarysite.m_capacity < militarysite.minSoldierCapacity()) {
			log
				("WARNING: militarysite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it must be at least %u. Changing to that value.\n",
				 militarysite.serial(), militarysite.owner().player_number(),
				 militarysite.get_position().x, militarysite.get_position().y,
				 militarysite.m_capacity, militarysite.minSoldierCapacity());
			militarysite.m_capacity = militarysite.minSoldierCapacity();
		} else if (militarysite.maxSoldierCapacity() < militarysite.m_capacity) {
			log
				("WARNING: militarysite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it can be at most %u. Changing to that value.\n",
				 militarysite.serial(), militarysite.owner().player_number(),
				 militarysite.get_position().x, militarysite.get_position().y,
				 militarysite.m_capacity, militarysite.maxSoldierCapacity());
			militarysite.m_capacity = militarysite.maxSoldierCapacity();
		}
	} catch (const _wexception & e) {
		throw game_data_error("militarysite: %s", e.what());
	}
}

void Map_Buildingdata_Data_Packet::read_productionsite
	(ProductionSite        & productionsite,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if
			(1 <= packet_version and
			 packet_version <= CURRENT_PRODUCTIONSITE_PACKET_VERSION)
		{
			ProductionSite::Working_Position & wp_begin =
				*productionsite.m_working_positions;
			const ProductionSite_Descr & pr_descr = productionsite.descr();
			const BillOfMaterials & working_positions = pr_descr.working_positions();

			uint16_t nr_worker_requests = fr.Unsigned16();
			for (uint16_t i = nr_worker_requests; i; --i) {
				Request & req =
					*new Request
						(productionsite,
						 Ware_Index::First(),
						 ProductionSite::request_worker_callback,
						 wwWORKER);
				req.Read(fr, game, mol);
				Ware_Index const worker_index = req.get_index();

				//  Find a working position that matches this request.
				ProductionSite::Working_Position * wp = &wp_begin;
				for
					(wl_const_range<BillOfMaterials>
					 j(working_positions);;
					 ++j)
				{
					if (j.empty())
						throw game_data_error
							("site has request for %s, for which there is no working "
							 "position",
							 productionsite.tribe()
							 .get_worker_descr(req.get_index())->name().c_str());
					uint32_t count = j->second;
					assert(count);
					if (worker_index == j->first) {
						while (wp->worker_request)
							if (--count)
								++wp;
							else
								throw game_data_error
									("request for %s does not match any free working "
									 "position",
									 productionsite.tribe()
									 .get_worker_descr(req.get_index())->name().c_str
									 	());
						break;
					} else
						wp += count;
				}
				wp->worker_request = &req;
			}

			uint16_t nr_workers = fr.Unsigned16();
			for (uint16_t i = nr_workers; i; --i) {
				Worker * worker = &mol.get<Worker>(fr.Unsigned32());

				//  Find a working position that matches this worker.
				const Worker_Descr & worker_descr = worker->descr();
				ProductionSite::Working_Position * wp = &wp_begin;
				for
					(wl_const_range<BillOfMaterials> j(working_positions);;
					 ++j)
				{
					if (j.empty())
						throw game_data_error
							("site has %s, for which there is no free working "
							 "position",
							 worker_descr.name().c_str());
					uint32_t count = j->second;
					assert(count);
					if (worker_descr.can_act_as(j->first)) {
						while (wp->worker or wp->worker_request) {
							++wp;
							if (not --count)
								goto end_working_position;
						}
						break;
					} else
						wp += count;
				end_working_position:;
				}
				wp->worker = worker;
			}

			if (nr_worker_requests + nr_workers < pr_descr.nr_working_positions())
				throw game_data_error
					("number of worker requests and workers are fewer than the "
					 "number of working positions");

			//  items from flags
			productionsite.m_fetchfromflag = fr.Signed32();

			//  skipped programs
			uint32_t const gametime = game.get_gametime();
			for (uint8_t i = 3 <= packet_version ? fr.Unsigned8() : 0; i; --i) {
				char const * const program_name = fr.CString();
				if (pr_descr.programs().count(program_name)) {
					uint32_t const skip_time = fr.Unsigned32();
					if (gametime < skip_time)
						throw game_data_error
							(
							 "program %s was skipped at time %u, but time is only "
							 "%u",
							 program_name, skip_time, gametime);
					productionsite.m_skipped_programs[program_name] = skip_time;
				} else {
					fr.Unsigned32(); // eat skip time
					log
						("WARNING: productionsite has skipped program \"%s\", which "
						 "does not exist\n",
						 program_name);
				}
			}

			//  state
			uint16_t const nr_progs = fr.Unsigned16();
			productionsite.m_stack.resize(nr_progs);
			for (uint16_t i = 0; i < nr_progs; ++i) {
				std::string program_name = fr.CString();
				std::transform
					(program_name.begin(), program_name.end(), program_name.begin(),
					 tolower);

				productionsite.m_stack[i].program =
					productionsite.descr().get_program(program_name);
				productionsite.m_stack[i].ip    = fr.  Signed32();
				productionsite.m_stack[i].phase = fr.  Signed32();
				productionsite.m_stack[i].flags = fr.Unsigned32();

				if (packet_version >= 5) {
					uint32_t serial = fr.Unsigned32();
					if (serial)
						productionsite.m_stack[i].objvar = &mol.get<Map_Object>(serial);
					productionsite.m_stack[i].coord = fr.Coords32_allow_null(game.map().extent());
				}
			}
			productionsite.m_program_timer = fr.Unsigned8();
			productionsite.m_program_time = fr.Signed32();

			uint16_t nr_queues = fr.Unsigned16();
			assert(!productionsite.m_input_queues.size());
			for (uint16_t i = 0; i < nr_queues; ++i) {
				WaresQueue * wq = new WaresQueue(productionsite, Ware_Index::Null(), 0);
				wq->Read(fr, game, mol);

				if (!wq->get_ware()) {
					delete wq;
				} else {
					productionsite.m_input_queues.push_back(wq);
				}
			}

			uint16_t const stats_size = fr.Unsigned16();
			productionsite.m_statistics.resize(stats_size);
			for (uint32_t i = 0; i < productionsite.m_statistics.size(); ++i)
				productionsite.m_statistics[i] = fr.Unsigned8();
			productionsite.m_statistics_changed = fr.Unsigned8();
			if (packet_version == 1) {
				memcpy(productionsite.m_statistics_buffer, fr.Data(40), 40);
				productionsite.m_statistics_buffer[39] = '\0';
			} else {
				{
					char const * const statistics_string        = fr.CString();
					size_t       const statistics_string_length =
						snprintf
							(productionsite.m_statistics_buffer,
							 sizeof(productionsite.m_statistics_buffer),
							 "%s", statistics_string);
					if
						(sizeof(productionsite.m_statistics_buffer)
						 <=
						 statistics_string_length)
						log
							("WARNING: productionsite statistics string can be at "
							 "most %" PRIuS " characters but a loaded building has the "
							 "string \"%s\" of length %" PRIuS "\n",
							 sizeof(productionsite.m_statistics_buffer) - 1,
							 statistics_string, statistics_string_length);
				}
				if (4 <= packet_version) {
					char const * const result_string        = fr.CString();
					size_t       const result_string_length =
						snprintf
							(productionsite.m_result_buffer,
							 sizeof(productionsite.m_result_buffer),
							 "%s", result_string);
					if
						(sizeof(productionsite.m_result_buffer)
						 <=
						 result_string_length)
						log
							("WARNING: productionsite result string can be at "
							 "most %" PRIuS " characters but a loaded building has the "
							 "string \"%s\" of length %" PRIuS "\n",
							 sizeof(productionsite.m_result_buffer) - 1,
							 result_string, result_string_length);
				}
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error
			("productionsite (%s): %s",
			 productionsite.descname().c_str(), e.what());
	}
}


void Map_Buildingdata_Data_Packet::read_trainingsite
	(TrainingSite          & trainingsite,
	 FileRead              & fr,
	 Game                  & game,
	 Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const trainingsite_packet_version = fr.Unsigned16();

		bool rel17comp = false; // compatibility with release 17
		if (4 == CURRENT_TRAININGSITE_PACKET_VERSION && 3 == trainingsite_packet_version)
			rel17comp = true;

		if (trainingsite_packet_version == CURRENT_TRAININGSITE_PACKET_VERSION or rel17comp)
		{
			read_productionsite(trainingsite, fr, game, mol);

			delete trainingsite.m_soldier_request;
			trainingsite.m_soldier_request = nullptr;
			if (fr.Unsigned8()) {
				trainingsite.m_soldier_request =
					new Request
						(trainingsite,
						 Ware_Index::First(),
						 TrainingSite::request_soldier_callback,
						 wwWORKER);
				trainingsite.m_soldier_request->Read(fr, game, mol);
			}

			trainingsite.m_capacity = fr.Unsigned8();
			trainingsite.m_build_heroes = fr.Unsigned8();

			uint8_t const nr_upgrades = fr.Unsigned8();
			for (uint8_t i = 0; i < nr_upgrades; ++i) {
				tAttribute attribute = static_cast<tAttribute>(fr.Unsigned8());
				if
					(TrainingSite::Upgrade * const upgrade =
					 	trainingsite.get_upgrade(attribute))
				{
					upgrade->prio = fr.Unsigned8();
					upgrade->credit = fr.Unsigned8();
					upgrade->lastattempt = fr.Signed32();
					upgrade->lastsuccess = fr.Unsigned8();
				} else {
					fr.Unsigned8();
					fr.Unsigned8();
					fr.Signed32();
					fr.Signed32();
				}
			}
			// load premature kick-out state, was not in release 17..
			if (not rel17comp)
			{
				uint16_t mapsize = fr.Unsigned16();
				while (mapsize)
				{
					uint16_t traintype  = fr.Unsigned16();
					uint16_t trainlevel = fr.Unsigned16();
					uint16_t trainstall = fr.Unsigned16();
					uint16_t spresence  = fr.Unsigned8();
					mapsize--;
					std::pair<uint16_t, uint8_t> t = std::make_pair(trainstall, spresence);
					trainingsite.training_failure_count[std::make_pair(traintype, trainlevel)] = t;
				}
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", trainingsite_packet_version);

		//  If the site's capacity is outside the allowed range (can happen if
		//  the site's type's definition has changed), set the variable to the
		//  nearest valid value.
		//
		//  This does not drop excessive soldiers, since they are not loaded into
		//  the site yet. To do that we would have to do this change by adding a
		//  Cmd_ChangeSoldierCapacity to the beginning of the game's command
		//  queue. But that would not work because the command queue is not read
		//  yet and will be cleared before it is read.
		if        (trainingsite.m_capacity < trainingsite.minSoldierCapacity()) {
			log
				("WARNING: trainingsite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it must be at least %u. Changing to that value.\n",
				 trainingsite.serial(), trainingsite.owner().player_number(),
				 trainingsite.get_position().x, trainingsite.get_position().y,
				 trainingsite.m_capacity, trainingsite.minSoldierCapacity());
			trainingsite.m_capacity = trainingsite.minSoldierCapacity();
		} else if (trainingsite.maxSoldierCapacity() < trainingsite.m_capacity) {
			log
				("WARNING: trainingsite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it can be at most %u. Changing to that value.\n",
				 trainingsite.serial(), trainingsite.owner().player_number(),
				 trainingsite.get_position().x, trainingsite.get_position().y,
				 trainingsite.m_capacity, trainingsite.maxSoldierCapacity());
			trainingsite.m_capacity = trainingsite.maxSoldierCapacity();
		}
	} catch (const _wexception & e) {
		throw game_data_error("trainingsite: %s", e.what());
	}
}


void Map_Buildingdata_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	FileWrite fw;

	// now packet version
	fw.Unsigned16(CURRENT_PACKET_VERSION);

	// Walk the map again
	Map & map = egbase.map();
	const uint32_t mapwidth = map.get_width();
	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i)
		if (upcast(Building const, building, map[i].get_immovable())) {
			assert(mos.is_object_known(*building));

			if (Map::get_index(building->get_position(), mapwidth) != i)
				continue; // This is not this buildings main position.

			fw.Unsigned32(mos.get_object_file_index(*building));

			//  player immovable owner is already in existence packet

			//  write the general stuff
			if (building->m_anim) {
				fw.Unsigned8(1);
				fw.String(building->descr().get_animation_name(building->m_anim));
			} else
				fw.Unsigned8(0);

			fw.Unsigned32(building->m_animstart);

			{
				const Building::Leave_Queue & leave_queue = building->m_leave_queue;
				fw.Unsigned16(leave_queue.size());
				container_iterate_const(Building::Leave_Queue, leave_queue, j) {
					assert(mos.is_object_known(*j.current->get(egbase)));
					fw.Unsigned32
						(mos.get_object_file_index(*j.current->get(egbase)));
				}
			}
			fw.Unsigned32(building->m_leave_time);
			if (Map_Object const * const o = building->m_leave_allow.get(egbase))
			{
				assert(mos.is_object_known(*o));
				fw.Unsigned32(mos.get_object_file_index(*o));
			} else {
				fw.Unsigned32(0);
			}
			{
				const Tribe_Descr& td = building->descr().tribe();
				BOOST_FOREACH(Building_Index b_idx, building->m_old_buildings) {
					const Building_Descr* b_descr = td.get_building_descr(b_idx);
					fw.Unsigned8(1);
					fw.String(b_descr->name());
				}
				fw.Unsigned8(0);
			}
			{
				bool is_stopped = false;
				if (upcast(ProductionSite const, productionsite, building))
					is_stopped = productionsite->is_stopped();
				fw.Unsigned8(is_stopped);
			}

			if (upcast(ConstructionSite const, constructionsite, building))
				write_constructionsite
					(*constructionsite,
					 fw,
					 ref_cast<Game, Editor_Game_Base>(egbase),
					 mos);
			else if (upcast(DismantleSite const, dms, building))
				write_dismantlesite
					(*dms,
					 fw,
					 ref_cast<Game, Editor_Game_Base>(egbase),
					 mos);
			else if (upcast(Warehouse const, warehouse, building))
				write_warehouse
					(*warehouse,
					 fw,
					 ref_cast<Game, Editor_Game_Base>(egbase),
					 mos);
			else if (upcast(ProductionSite const, productionsite, building)) {
				if (upcast(MilitarySite const, militarysite, productionsite))
					write_militarysite
						(*militarysite,
						 fw,
						 ref_cast<Game, Editor_Game_Base>(egbase),
						 mos);
				else if (upcast(TrainingSite const, trainingsite, productionsite))
					write_trainingsite
						(*trainingsite,
						 fw,
						 ref_cast<Game, Editor_Game_Base>(egbase),
						 mos);
				else
					write_productionsite
						(*productionsite,
						 fw,
						 ref_cast<Game, Editor_Game_Base>(egbase),
						 mos);
			} else {
				assert(false);
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
			}

			mos.mark_object_as_saved(*building);
		}

	fw.Write(fs, "binary/building_data");
}

void Map_Buildingdata_Data_Packet::write_partially_finished_building
	(const Partially_Finished_Building & pfb,
	 FileWrite              & fw,
	 Game                   & game,
	 Map_Map_Object_Saver   & mos)
{
	fw.Unsigned16(CURRENT_PARTIALLYFB_PACKET_VERSION);

	//  descriptions
	fw.String(pfb.m_building->name());

	// builder request
	if (pfb.m_builder_request) {
		fw.Unsigned8(1);
		pfb.m_builder_request->Write(fw, game, mos);
	} else
		fw.Unsigned8(0);

	// builder
	if (Worker const * builder = pfb.m_builder.get(game)) {
		assert(mos.is_object_known(*builder));
		fw.Unsigned32(mos.get_object_file_index(*builder));
	} else
		fw.Unsigned32(0);

	const uint16_t wares_size = pfb.m_wares.size();
	fw.Unsigned16(wares_size);
	for (uint16_t i = 0; i < wares_size; ++i)
		pfb.m_wares[i]->Write(fw, game, mos);

	fw.Unsigned8 (pfb.m_working);
	fw.Unsigned32(pfb.m_work_steptime);
	fw.Unsigned32(pfb.m_work_completed);
	fw.Unsigned32(pfb.m_work_steps);
}

void Map_Buildingdata_Data_Packet::write_constructionsite
	(const ConstructionSite & constructionsite,
	 FileWrite              & fw,
	 Game                   & game,
	 Map_Map_Object_Saver   & mos)
{

	fw.Unsigned16(CURRENT_CONSTRUCTIONSITE_PACKET_VERSION);

	write_partially_finished_building(constructionsite, fw, game, mos);

	fw.Signed32(constructionsite.m_fetchfromflag);
}

void Map_Buildingdata_Data_Packet::write_dismantlesite
	(const DismantleSite & dms,
	 FileWrite              & fw,
	 Game                   & game,
	 Map_Map_Object_Saver   & mos)
{

	fw.Unsigned16(CURRENT_DISMANTLESITE_PACKET_VERSION);

	write_partially_finished_building(dms, fw, game, mos);

	// Nothing to Do
}


void Map_Buildingdata_Data_Packet::write_warehouse
	(const Warehouse      & warehouse,
	 FileWrite            & fw,
	 Game                 & game,
	 Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CURRENT_WAREHOUSE_PACKET_VERSION);

	//  supply
	const Tribe_Descr & tribe = warehouse.tribe();
	const WareList & wares = warehouse.m_supply->get_wares();
	for (Ware_Index i = Ware_Index::First(); i < wares.get_nrwareids  (); ++i) {
		fw.Unsigned8(1);
		fw.String(tribe.get_ware_descr(i)->name());
		fw.Unsigned32(wares.stock(i));
		fw.Unsigned8(warehouse.get_ware_policy(i));
	}
	fw.Unsigned8(0);
	const WareList & workers = warehouse.m_supply->get_workers();
	for (Ware_Index i = Ware_Index::First(); i < workers.get_nrwareids(); ++i) {
		fw.Unsigned8(1);
		fw.String(tribe.get_worker_descr(i)->name());
		fw.Unsigned32(workers.stock(i));
		fw.Unsigned8(warehouse.get_worker_policy(i));
	}
	fw.Unsigned8(0);

	//  Incorporated workers, write sorted after file-serial.
	uint32_t nworkers = 0;
	container_iterate_const(Warehouse::IncorporatedWorkers, warehouse.m_incorporated_workers, cwt)
		nworkers += cwt->second.size();

	fw.Unsigned16(nworkers);
	typedef std::map<uint32_t, const Worker *> TWorkerMap;
	TWorkerMap workermap;
	container_iterate_const(Warehouse::IncorporatedWorkers, warehouse.m_incorporated_workers, cwt) {
		container_iterate_const(Warehouse::WorkerList, cwt->second, i) {
			const Worker & w = *(*i);
			assert(mos.is_object_known(w));
			workermap.insert
				(std::pair<uint32_t, const Worker *>
				 (mos.get_object_file_index(w), &w));
		}
	}

	container_iterate_const(TWorkerMap, workermap, i)
	{
		const Worker & obj = *i.current->second;
		assert(mos.is_object_known(obj));
		fw.Unsigned32(mos.get_object_file_index(obj));
	}

	{
		const std::vector<Ware_Index> & worker_types_without_cost =
			tribe.worker_types_without_cost();
		for (uint8_t i = worker_types_without_cost.size(); i;) {
			uint32_t const next_spawn =
				warehouse.m_next_worker_without_cost_spawn[--i];
			if (next_spawn != static_cast<uint32_t>(Never())) {
				fw.String
					(tribe.get_worker_descr(tribe.worker_types_without_cost().at(i))
					 ->name());
				fw.Unsigned32(next_spawn);
			}
		}
	}
	fw.Unsigned8(0); //  terminator for spawn times

	fw.Unsigned32(warehouse.m_planned_workers.size());
	container_iterate_const
		(std::vector<Warehouse::PlannedWorkers>,
		 warehouse.m_planned_workers, pw_it)
	{
		fw.CString(tribe.get_worker_descr(pw_it.current->index)->name());
		fw.Unsigned32(pw_it.current->amount);

		fw.Unsigned32(pw_it.current->requests.size());
		container_iterate_const
			(std::vector<Request *>, pw_it.current->requests, req_it)
			(*req_it.current)->Write(fw, game, mos);
	}

	fw.Unsigned32(warehouse.m_next_stock_remove_act);

	if (warehouse.descr().get_isport()) {
		fw.Unsigned32(mos.get_object_file_index_or_zero(warehouse.m_portdock));

		// Expedition specific stuff. See comment in loader.
		if (warehouse.m_portdock->expedition_started()) {
			warehouse.m_portdock->expedition_bootstrap()->save(fw, game, mos);
		}
	}
}


void Map_Buildingdata_Data_Packet::write_militarysite
	(const MilitarySite   & militarysite,
	 FileWrite            & fw,
	 Game                 & game,
	 Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CURRENT_MILITARYSITE_PACKET_VERSION);
	write_productionsite(militarysite, fw, game, mos);

	if (militarysite.m_normal_soldier_request) {
		fw.Unsigned8(1);
		militarysite.m_normal_soldier_request->Write(fw, game, mos);
	} else {
		fw.Unsigned8(0);
	}

	if (militarysite.m_upgrade_soldier_request)
	{
		fw.Unsigned8(1);
		militarysite.m_upgrade_soldier_request->Write(fw, game, mos);
	}
	else
		fw.Unsigned8(0);


	fw.Unsigned8(militarysite.m_didconquer);
	fw.Unsigned8(militarysite.m_capacity);
	fw.Signed32(militarysite.m_nexthealtime);

	if (militarysite.m_normal_soldier_request)
	{
		if (militarysite.m_upgrade_soldier_request)
			{
				throw game_data_error
				("Internal error in a MilitarySite -- cannot continue. Use previous autosave.");
			}
	}
	fw.Unsigned16(militarysite.m_soldier_upgrade_requirements.getMin());
	fw.Unsigned16(militarysite.m_soldier_upgrade_requirements.getMax());
	fw.Unsigned8(militarysite.m_soldier_preference);
	fw.Signed32(militarysite.m_next_swap_soldiers_time);
	fw.Unsigned8(militarysite.m_soldier_upgrade_try ? 1 : 0);
	fw.Unsigned8(militarysite.m_doing_upgrade_request ? 1 : 0);

}


void Map_Buildingdata_Data_Packet::write_productionsite
	(const ProductionSite & productionsite,
	 FileWrite            & fw,
	 Game                 & game,
	 Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CURRENT_PRODUCTIONSITE_PACKET_VERSION);

	uint32_t const nr_working_positions =
		productionsite.descr().nr_working_positions();
	const ProductionSite::Working_Position & begin =
		productionsite.m_working_positions[0];
	const ProductionSite::Working_Position & end =
		(&begin)[nr_working_positions];
	uint32_t nr_workers = 0;
	for (ProductionSite::Working_Position const * i = &begin; i < &end; ++i)
		nr_workers += i->worker ? 1 : 0;

	//  worker requests
	fw.Unsigned16(nr_working_positions - nr_workers);
	for (ProductionSite::Working_Position const * i = &begin; i < &end; ++i)
		if (Request const * const r = i->worker_request)
			r->Write(fw, game, mos);

	//  workers
	fw.Unsigned16(nr_workers);
	for (ProductionSite::Working_Position const * i = &begin; i < &end; ++i)
		if (Worker const * const w = i->worker) {
			assert(not i->worker_request);
			assert(mos.is_object_known(*w));
			fw.Unsigned32(mos.get_object_file_index(*w));
		}

	fw.Signed32(productionsite.m_fetchfromflag);

	//  skipped programs
	assert
		(productionsite.m_skipped_programs.size()
		 <=
		 std::numeric_limits<uint8_t>::max());
	fw.Unsigned8(productionsite.m_skipped_programs.size());
	container_iterate_const
		(ProductionSite::Skipped_Programs, productionsite.m_skipped_programs, i)
	{
		fw.String    (i.current->first);
		fw.Unsigned32(i.current->second);
	}

	//  state
	uint16_t const program_size = productionsite.m_stack.size();
	fw.Unsigned16(program_size);
	for (uint16_t i = 0; i < program_size; ++i) {
		fw.String    (productionsite.m_stack[i].program->name());
		fw.  Signed32(productionsite.m_stack[i].ip);
		fw.  Signed32(productionsite.m_stack[i].phase);
		fw.Unsigned32(productionsite.m_stack[i].flags);
		fw.Unsigned32(mos.get_object_file_index_or_zero(productionsite.m_stack[i].objvar.get(game)));
		fw.Coords32(productionsite.m_stack[i].coord);
	}
	fw.Unsigned8(productionsite.m_program_timer);
	fw. Signed32(productionsite.m_program_time);

	const uint16_t input_queues_size = productionsite.m_input_queues.size();
	fw.Unsigned16(input_queues_size);
	for (uint16_t i = 0; i < input_queues_size; ++i)
		productionsite.m_input_queues[i]->Write(fw, game, mos);

	const uint16_t statistics_size = productionsite.m_statistics.size();
	fw.Unsigned16(statistics_size);
	for (uint32_t i = 0; i < statistics_size; ++i)
		fw.Unsigned8(productionsite.m_statistics[i]);
	fw.Unsigned8(productionsite.m_statistics_changed);
	fw.String(productionsite.m_statistics_buffer);
	fw.String(productionsite.m_result_buffer);
}

/*
 * write for trainingsite
 */
void Map_Buildingdata_Data_Packet::write_trainingsite
	(const TrainingSite   & trainingsite,
	 FileWrite            & fw,
	 Game                 & game,
	 Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(CURRENT_TRAININGSITE_PACKET_VERSION);

	write_productionsite(trainingsite, fw, game, mos);

	//  requests

	if (trainingsite.m_soldier_request) {
		fw.Unsigned8(1);
		trainingsite.m_soldier_request->Write(fw, game, mos);
	} else {
		fw.Unsigned8(0);
	}

	fw.Unsigned8(trainingsite.m_capacity);
	fw.Unsigned8(trainingsite.m_build_heroes);

	// upgrades
	fw.Unsigned8(trainingsite.m_upgrades.size());
	for (uint8_t i = 0; i < trainingsite.m_upgrades.size(); ++i) {
		const TrainingSite::Upgrade & upgrade = trainingsite.m_upgrades[i];
		fw.Unsigned8(upgrade.attribute);
		fw.Unsigned8(upgrade.prio);
		fw.Unsigned8(upgrade.credit);
		fw.Signed32(upgrade.lastattempt);
		fw.Signed8(upgrade.lastsuccess);
	}
	if (255 < trainingsite.training_failure_count.size())
		log
			("Save TrainingSite: Failure counter has ridiculously many entries! (%u)\n",
			 static_cast<uint16_t>(trainingsite.training_failure_count.size()));
	fw.Unsigned16(static_cast<uint16_t> (trainingsite.training_failure_count.size()));
	for
		(TrainingSite::TrainFailCount_t::const_iterator i = trainingsite.training_failure_count.begin();
		 i != trainingsite.training_failure_count.end(); i++)
	{
		fw.Unsigned16(i->first.first);
		fw.Unsigned16(i->first.second);
		fw.Unsigned16(i->second.first);
		fw.Unsigned8(i->second.second);
	}

	// DONE
}
}
