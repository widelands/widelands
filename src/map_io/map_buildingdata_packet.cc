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

#include "map_io/map_buildingdata_packet.h"

#include <map>
#include <memory>

#include "base/macros.h"
#include "economy/flag.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/warehousesupply.h"
#include "economy/wares_queue.h"
#include "io/fileread.h"
#include "io/filewrite.h"
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
#include "logic/tribes/tribe_descr.h"
#include "logic/warehouse.h"
#include "logic/widelands_geometry_io.h"
#include "logic/worker.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

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


void MapBuildingdataPacket::read
	(FileSystem            &       fs,
	 EditorGameBase      &       egbase,
	 bool                    const skip,
	 MapObjectLoader &       mol)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.open(fs, "binary/building_data");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (1 <= packet_version && packet_version <= CURRENT_PACKET_VERSION) {
			for (;;) {
				if (2 <= packet_version && fr.end_of_file())
					break;
				Serial const serial = fr.unsigned_32();
				if (packet_version < 2 && serial == 0xffffffff) {
					if (!fr.end_of_file())
						throw GameDataError
							("expected end of file after serial 0xffffffff");
					break;
				}
				try {
					Building & building = mol.get<Building>(serial);

					if (fr.unsigned_8()) {
						char const * const animation_name = fr.c_string();
						try {
							building.m_anim =
								building.descr().get_animation(animation_name);
						} catch (const MapObjectDescr::AnimationNonexistent &) {
							log
								("WARNING: %s %s does not have animation \"%s\"; "
								 "using animation \"idle\" instead\n",
								 building.owner().tribe().name().c_str(),
								 building.descr().descname().c_str(),
								 animation_name);
							building.m_anim = building.descr().get_animation("idle");
						}
					} else
						building.m_anim = 0;
					building.m_animstart = fr.unsigned_32();

					{
						Building::LeaveQueue & leave_queue = building.m_leave_queue;
						leave_queue.resize(fr.unsigned_16());

						for (Building::LeaveQueue::iterator queue_iter = leave_queue.begin();
							  queue_iter != leave_queue.end();
							  ++queue_iter) {

							if (uint32_t const leaver_serial = fr.unsigned_32())
								try {
									//  The check that this worker actually has a
									//  leavebuilding task for this building is in
									//  Building::load_finish, which is called after the
									//  worker (with his stack of tasks) has been fully
									//  loaded.
									*queue_iter = &mol.get<Worker>(leaver_serial);
								} catch (const WException & e) {
									throw GameDataError
										("leave queue item #%lu (%u): %s",
										 static_cast<long int>
											(queue_iter - leave_queue.begin()),
										 leaver_serial, e.what());
								}
							else
								*queue_iter = nullptr;
						}
					}

					building.m_leave_time = fr.unsigned_32();

					if (uint32_t const leaver_serial = fr.unsigned_32())
						try {
							building.m_leave_allow =
								&mol.get<MapObject>(leaver_serial);
						} catch (const WException & e) {
							throw GameDataError
								("leave allow item (%u): %s", leaver_serial, e.what());
						}
					else {
						building.m_leave_allow = nullptr;
					}
					if (packet_version >= 3) {
						// For former versions, the former buildings vector
						// will be built after other data are loaded, see below.
						// read_formerbuildings_v2()
						while (fr.unsigned_8()) {
							BuildingIndex oldidx = building.owner().tribe().safe_building_index(fr.c_string());
							building.m_old_buildings.push_back(oldidx);
						}
						// Only construction sites may have an empty list
						if (building.m_old_buildings.empty() && !is_a(ConstructionSite, &building)) {
							throw GameDataError
								("Failed to read %s %u: No former buildings informations.\n"
								"Your savegame is corrupted", building.descr().descname().c_str(), building.serial());
						}
					}
					if (fr.unsigned_8()) {
						if (upcast(ProductionSite, productionsite, &building))
							if (dynamic_cast<MilitarySite const *>(productionsite)) {
								log
									("WARNING: Found a stopped %s at (%i, %i) in the "
									 "savegame. Militarysites are not stoppable. "
									 "Ignoring.",
									 building.descr().descname().c_str(),
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
								 building.descr().descname().c_str(),
								 building.get_position().x,
								 building.get_position().y);
					}

					//  Set economy now, some stuff below will count on this.
					building.set_economy(building.m_flag->get_economy());

					Game& game = dynamic_cast<Game&>(egbase);

					if (upcast(ConstructionSite, constructionsite, &building)) {
						read_constructionsite(*constructionsite, fr, game, mol);
					} else if (upcast(DismantleSite, dms, &building)) {
						read_dismantlesite(*dms, fr, game, mol);
					} else if (upcast(Warehouse, warehouse, &building)) {
						read_warehouse(*warehouse, fr, game, mol);
					} else if (upcast(ProductionSite, productionsite, &building)) {
						if (upcast(MilitarySite, militarysite, productionsite)) {
							read_militarysite(*militarysite, fr, game, mol);
						} else if (upcast(TrainingSite, trainingsite, productionsite)) {
							read_trainingsite(*trainingsite, fr, game, mol);
						} else {
							read_productionsite(*productionsite, fr, game, mol);
						}
					} else {
						//  type of building is not one of (or derived from)
						//  {ConstructionSite, Warehouse, ProductionSite}
						assert(false);
					}
					if (packet_version < 3) {
						read_formerbuildings_v2(building, fr, game, mol);
					}

					mol.mark_object_as_loaded(building);
				} catch (const WException & e) {
					throw GameDataError("building %u: %s", serial, e.what());
				}
			}
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("buildingdata: %s", e.what());
	}
}

void MapBuildingdataPacket::read_formerbuildings_v2
	(Building& b, FileRead&, Game& game, MapObjectLoader&)
{
	BuildingIndex b_idx = game.tribes().building_index(b.descr().name());
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
		BuildingIndex former_idx = b.m_old_buildings.front();
		const BuildingDescr * oldest = game.tribes().get_building_descr(former_idx);
		if (!oldest->is_enhanced()) {
			break;
		}
		b.m_old_buildings.insert(b.m_old_buildings.begin(), oldest->enhanced_from());
	}
}


void MapBuildingdataPacket::read_partially_finished_building
	(PartiallyFinishedBuilding  & pfb,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == CURRENT_PARTIALLYFB_PACKET_VERSION) {
			const TribeDescr & tribe = pfb.owner().tribe();
			pfb.m_building =
				tribe.get_building_descr(tribe.safe_building_index(fr.c_string()));

			delete pfb.m_builder_request;
			if (fr.unsigned_8()) {
				pfb.m_builder_request =
					new Request
					(pfb,
					 0,
					 PartiallyFinishedBuilding::request_builder_callback,
					 wwWORKER);
				pfb.m_builder_request->read(fr, game, mol);
			} else
				pfb.m_builder_request = nullptr;

			if (uint32_t const builder_serial = fr.unsigned_32()) {
				try {
					pfb.m_builder = &mol.get<Worker>(builder_serial);
				} catch (const WException & e) {
					throw GameDataError
						("builder (%u): %s", builder_serial, e.what());
				}
			} else
				pfb.m_builder = nullptr;

			try {
				uint16_t const size = fr.unsigned_16();
				pfb.m_wares.resize(size);
				for (uint16_t i = 0; i < pfb.m_wares.size(); ++i)
				{
					pfb.m_wares[i] =
						new WaresQueue
						(pfb, INVALID_INDEX, 0);
					pfb.m_wares[i]->read(fr, game, mol);
				}
			} catch (const WException & e) {
				throw GameDataError("wares: %s", e.what());
			}

			pfb.m_working        = fr.unsigned_8 ();
			pfb.m_work_steptime  = fr.unsigned_32();
			pfb.m_work_completed = fr.unsigned_32();
			pfb.m_work_steps     = fr.unsigned_32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("partially_constructed_buildings: %s", e.what());
	}
}

void MapBuildingdataPacket::read_constructionsite
	(ConstructionSite      & constructionsite,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == 1)
			return read_constructionsite_v1(constructionsite, fr, game, mol);

		if (packet_version >= 2) {
			read_partially_finished_building(constructionsite, fr, game, mol);

			const TribeDescr & tribe = constructionsite.owner().tribe();

			for (ConstructionSite::Wares::iterator wares_iter = constructionsite.m_wares.begin();
				  wares_iter != constructionsite.m_wares.end();
				  ++wares_iter) {

					(*wares_iter)->set_callback
						(ConstructionSite::wares_queue_callback, &constructionsite);
			}

			if (packet_version <= 2) {
				if (fr.unsigned_8()) {
					BuildingIndex idx = tribe.safe_building_index(fr.c_string());
					constructionsite.m_old_buildings.push_back(idx);
				}
			}

			constructionsite.m_fetchfromflag  = fr.  signed_32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("constructionsite: %s", e.what());
	}
}

void MapBuildingdataPacket::read_constructionsite_v1
	(ConstructionSite      & constructionsite,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	const TribeDescr & tribe = constructionsite.owner().tribe();
	constructionsite.m_building =
		tribe.get_building_descr(tribe.safe_building_index(fr.c_string()));
	if (fr.unsigned_8()) {
		BuildingIndex bidx = tribe.safe_building_index(fr.c_string());
		constructionsite.m_old_buildings.push_back(bidx);
	}

	delete constructionsite.m_builder_request;
	if (fr.unsigned_8()) {
		constructionsite.m_builder_request =
			new Request
			(constructionsite,
			 0,
			 ConstructionSite::request_builder_callback,
			 wwWORKER);
		constructionsite.m_builder_request->read(fr, game, mol);
	} else
		constructionsite.m_builder_request = nullptr;

	if (uint32_t const builder_serial = fr.unsigned_32()) {
		try {
			constructionsite.m_builder = &mol.get<Worker>(builder_serial);
		} catch (const WException & e) {
			throw GameDataError
				("builder (%u): %s", builder_serial, e.what());
		}
	} else
		constructionsite.m_builder = nullptr;

	try {
		uint16_t const size = fr.unsigned_16();
		constructionsite.m_wares.resize(size);
		for (uint16_t i = 0; i < constructionsite.m_wares.size(); ++i)
		{
			constructionsite.m_wares[i] =
				new WaresQueue
				(constructionsite, INVALID_INDEX, 0);
			constructionsite.m_wares[i]->set_callback
				(ConstructionSite::wares_queue_callback, &constructionsite);
			constructionsite.m_wares[i]->read(fr, game, mol);
		}
	} catch (const WException & e) {
		throw GameDataError("wares: %s", e.what());
	}

	constructionsite.m_fetchfromflag  = fr.  signed_32();

	constructionsite.m_working        = fr.unsigned_8 ();
	constructionsite.m_work_steptime  = fr.unsigned_32();
	constructionsite.m_work_completed = fr.unsigned_32();
	constructionsite.m_work_steps     = fr.unsigned_32();
}

void MapBuildingdataPacket::read_dismantlesite
	(DismantleSite         & dms,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == CURRENT_DISMANTLESITE_PACKET_VERSION) {
			read_partially_finished_building(dms, fr, game, mol);

			// Nothing to do
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("dismantlesite: %s", e.what());
	}
}


void MapBuildingdataPacket::read_warehouse
	(Warehouse             & warehouse,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if
			(1 <= packet_version &&
			 packet_version <= CURRENT_WAREHOUSE_PACKET_VERSION)
		{
			Player& player = warehouse.owner();
			warehouse.init_containers(player);
			const TribeDescr& tribe = player.tribe();

			while (fr.unsigned_8()) {
				const WareIndex& id = tribe.ware_index(fr.c_string());
				if (packet_version >= 5) {
					uint32_t amount = fr.unsigned_32();
					Warehouse::StockPolicy policy =
						static_cast<Warehouse::StockPolicy>(fr.unsigned_8());

					if (game.tribes().ware_exists(id)) {
						warehouse.insert_wares(id, amount);
						warehouse.set_ware_policy(id, policy);
					}
				} else {
					uint16_t amount = fr.unsigned_16();

					if (game.tribes().ware_exists(id))
						warehouse.insert_wares(id, amount);
				}
			}
			while (fr.unsigned_8()) {
				const WareIndex& id = tribe.worker_index(fr.c_string());
				if (packet_version >= 5) {
					uint32_t amount = fr.unsigned_32();
					Warehouse::StockPolicy policy =
						static_cast<Warehouse::StockPolicy>(fr.unsigned_8());

					if (game.tribes().worker_exists(id)) {
						warehouse.insert_workers(id, amount);
						warehouse.set_worker_policy(id, policy);
					}
				} else {
					uint16_t amount = fr.unsigned_16();

					if (game.tribes().worker_exists(id))
						warehouse.insert_workers(id, amount);
				}
			}

			if (packet_version <= 3) {
				// eat the obsolete idle request structures
				uint32_t nrrequests = fr.unsigned_16();
				while (nrrequests--) {
					std::unique_ptr<Request> req
						(new Request
						 	(warehouse,
						 	 0,
						 	 &Warehouse::request_cb,
						 	 wwWORKER));
					req->read(fr, game, mol);
				}
			}

			assert(warehouse.m_incorporated_workers.empty());
			{
				uint16_t const nrworkers = fr.unsigned_16();
				for (uint16_t i = 0; i < nrworkers; ++i) {
					uint32_t const worker_serial = fr.unsigned_32();

					try {
						Worker & worker = mol.get<Worker>(worker_serial);
						if (1 == packet_version) {
							char const * const name = fr.c_string();
							if (name != worker.descr().name())
								throw GameDataError
									("expected %s but found \"%s\"",
									 worker.descr().name().c_str(), name);
						}
						const WareIndex& worker_index = tribe.worker_index(worker.descr().name().c_str());
						if (!warehouse.m_incorporated_workers.count(worker_index))
							warehouse.m_incorporated_workers[worker_index] = std::vector<Worker *>();
						warehouse.m_incorporated_workers[worker_index].push_back(&worker);
					} catch (const WException & e) {
						throw GameDataError
							("incorporated worker #%u (%u): %s",
							 i, worker_serial, e.what());
					}
				}
			}

			const std::vector<WareIndex>& worker_types_without_cost = tribe.worker_types_without_cost();

			if (1 == packet_version) { //  a single next_spawn time for "carrier"
				const Time& next_spawn = static_cast<Time>(fr.unsigned_32());
				const WareIndex& worker_index = tribe.carrier();
				if (!game.tribes().worker_exists(worker_index)) {
					log
						("WARNING: %s %u has a next_spawn time for nonexistent "
						 "worker type \"%s\" set to %u, ignoring\n",
						 warehouse.descr().descname().c_str(), warehouse.serial(),
						 "carrier", next_spawn);
				} else if
					(tribe.get_worker_descr(worker_index)->buildcost().size())
				{
					log
						("WARNING: %s %u has a next_spawn time for worker type "
						 "\"%s\", that costs something to build, set to %u, "
						 "ignoring\n",
						 warehouse.descr().descname().c_str(), warehouse.serial(),
						 "carrier", next_spawn);
				} else
					assert(worker_types_without_cost.size() ==
							 warehouse.m_next_worker_without_cost_spawn.size());
					for (uint8_t i = 0;; ++i) {
						assert(i < worker_types_without_cost.size());
						if (worker_types_without_cost.at(i) == worker_index) {
							if (warehouse.m_next_worker_without_cost_spawn[i] != never()) {
								warehouse.molog
									("read_warehouse: "
									 "m_next_worker_without_cost_spawn[%u] = %u\n",
									 i, warehouse.m_next_worker_without_cost_spawn[i]);
							}
							assert(warehouse.m_next_worker_without_cost_spawn[i] == never());
							warehouse.m_next_worker_without_cost_spawn[i] = next_spawn;
							break;
						}
					}
			} else {
				for (;;) {
					char const * const worker_typename = fr.c_string   ();
					if (!*worker_typename) { //  encountered the terminator ("")
						break;
					}
					const Time& next_spawn = static_cast<Time>(fr.unsigned_32());
					const WareIndex& worker_index = tribe.safe_worker_index(worker_typename);
					if (!game.tribes().worker_exists(worker_index)) {
						log
							("WARNING: %s %u has a next_spawn time for nonexistent "
							 "worker type \"%s\" set to %u, ignoring\n",
							 warehouse.descr().descname().c_str(), warehouse.serial(),
							 worker_typename, next_spawn);
						continue;
					}
					if (tribe.get_worker_descr(worker_index)->buildcost().size()) {
						log
							("WARNING: %s %u has a next_spawn time for worker type "
							 "\"%s\", that costs something to build, set to %u, "
							 "ignoring\n",
							 warehouse.descr().descname().c_str(), warehouse.serial(),
							 worker_typename, next_spawn);
						continue;
					}

					assert(worker_types_without_cost.size() ==
							 warehouse.m_next_worker_without_cost_spawn.size());
					for (uint8_t i = 0;; ++i) {
						assert(i < worker_types_without_cost.size());
						if (worker_types_without_cost.at(i) == worker_index) {
							if (warehouse.m_next_worker_without_cost_spawn[i] != never()) {
								throw GameDataError
									(
									 "%s %u has a next_spawn time for worker type "
									 "\"%s\" set to %u, but it was previously set "
									 "to %u\n",
									 warehouse.descr().descname().c_str(), warehouse.serial(),
									 worker_typename, next_spawn,
									 warehouse.m_next_worker_without_cost_spawn[i]);
							}
							warehouse.m_next_worker_without_cost_spawn[i] = next_spawn;
							break;
						}
					}
				}
			}
			//  The checks that the warehouse has a next_spawn time for each
			//  worker type that the player is allowed to spawn, is in
			//  Warehouse::load_finish.

			if (packet_version >= 3) {
				// Read planned worker data
				// Consistency checks are in Warehouse::load_finish
				uint32_t nr_planned_workers = fr.unsigned_32();
				while (nr_planned_workers--) {
					warehouse.m_planned_workers.push_back
						(Warehouse::PlannedWorkers());
					Warehouse::PlannedWorkers & pw =
						warehouse.m_planned_workers.back();
					pw.index = tribe.worker_index(fr.c_string());
					pw.amount = fr.unsigned_32();

					uint32_t nr_requests = fr.unsigned_32();
					while (nr_requests--) {
						pw.requests.push_back
							(new Request
							 	(warehouse,
							 	 0,
							 	 &Warehouse::request_cb,
							 	 wwWORKER));
						pw.requests.back()->read(fr, game, mol);
					}
				}
			}

			if (packet_version >= 5) {
				warehouse.m_next_stock_remove_act = fr.unsigned_32();
			}

			if (packet_version >= 6) {
				if (warehouse.descr().get_isport()) {
					if (Serial portdock = fr.unsigned_32()) {
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

			if (uint32_t const conquer_radius = warehouse.descr().get_conquers()) {
				//  Add to map of military influence.
				const Map & map = game.map();
				Area<FCoords> a
					(map.get_fcoords(warehouse.get_position()), conquer_radius);
				const Field & first_map_field = map[0];
				Player::Field * const player_fields = player.m_fields;
				MapRegion<Area<FCoords> > mr(map, a);
				do
					player_fields[mr.location().field - &first_map_field]
					.military_influence
						+= map.calc_influence(mr.location(), Area<>(a, a.radius));
				while (mr.advance(map));
			}
			player.see_area
				(Area<FCoords>
				 (game.map().get_fcoords(warehouse.get_position()),
				  warehouse.descr().vision_range()));
			warehouse.m_next_military_act = game.get_gametime();
			//log("Read warehouse stuff for %p\n", &warehouse);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("warehouse: %s", e.what());
	}
}


void MapBuildingdataPacket::read_militarysite
	(MilitarySite          & militarysite,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == CURRENT_MILITARYSITE_PACKET_VERSION)
		{
			read_productionsite(militarysite, fr, game, mol);

			militarysite.m_normal_soldier_request.reset();

			if (fr.unsigned_8()) {
				militarysite.m_normal_soldier_request.reset
					(new Request
						(militarysite,
						 0,
						 MilitarySite::request_soldier_callback,
						 wwWORKER));
				militarysite.m_normal_soldier_request->read(fr, game, mol);
			}
			else
				militarysite.m_normal_soldier_request.reset();

			if (fr.unsigned_8())
			{
				militarysite.m_upgrade_soldier_request.reset
					(new Request
						(militarysite,
						 (!militarysite.m_normal_soldier_request) ? 0
						: militarysite.owner().tribe().soldier(),
						MilitarySite::request_soldier_callback,
						wwWORKER));
				militarysite.m_upgrade_soldier_request->read(fr, game, mol);
			}
			else
				militarysite.m_upgrade_soldier_request.reset();


			if ((militarysite.m_didconquer = fr.unsigned_8())) {
				//  Add to map of military influence.
				const Map & map = game.map();
				Area<FCoords> a
					(map.get_fcoords(militarysite.get_position()),
					 militarysite.descr().get_conquers());
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
			militarysite.m_capacity = fr.unsigned_8();
			militarysite.m_nexthealtime = fr.signed_32();

			uint16_t reqmin = fr.unsigned_16();
			uint16_t reqmax = fr.unsigned_16();
			militarysite.m_soldier_upgrade_requirements = RequireAttribute(atrTotal, reqmin, reqmax);
			militarysite.m_soldier_preference = static_cast<MilitarySite::SoldierPreference>(fr.unsigned_8());
			militarysite.m_next_swap_soldiers_time = fr.signed_32();
			militarysite.m_soldier_upgrade_try = 0 != fr.unsigned_8() ? true : false;
			militarysite.m_doing_upgrade_request = 0 != fr.unsigned_8() ? true : false;

		} else
			throw GameDataError
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
		if        (militarysite.m_capacity < militarysite.min_soldier_capacity()) {
			log
				("WARNING: militarysite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it must be at least %u. Changing to that value.\n",
				 militarysite.serial(), militarysite.owner().player_number(),
				 militarysite.get_position().x, militarysite.get_position().y,
				 militarysite.m_capacity, militarysite.min_soldier_capacity());
			militarysite.m_capacity = militarysite.min_soldier_capacity();
		} else if (militarysite.max_soldier_capacity() < militarysite.m_capacity) {
			log
				("WARNING: militarysite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it can be at most %u. Changing to that value.\n",
				 militarysite.serial(), militarysite.owner().player_number(),
				 militarysite.get_position().x, militarysite.get_position().y,
				 militarysite.m_capacity, militarysite.max_soldier_capacity());
			militarysite.m_capacity = militarysite.max_soldier_capacity();
		}
	} catch (const WException & e) {
		throw GameDataError("militarysite: %s", e.what());
	}
}

void MapBuildingdataPacket::read_productionsite
	(ProductionSite        & productionsite,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if
			(1 <= packet_version &&
			 packet_version <= CURRENT_PRODUCTIONSITE_PACKET_VERSION)
		{
			ProductionSite::WorkingPosition & wp_begin =
				*productionsite.m_working_positions;
			const ProductionSiteDescr & pr_descr = productionsite.descr();
			const BillOfMaterials & working_positions = pr_descr.working_positions();

			uint16_t nr_worker_requests = fr.unsigned_16();
			for (uint16_t i = nr_worker_requests; i; --i) {
				Request & req =
					*new Request
						(productionsite,
						 0,
						 ProductionSite::request_worker_callback,
						 wwWORKER);
				req.read(fr, game, mol);
				const WareIndex& worker_index = req.get_index();

				//  Find a working position that matches this request.
				ProductionSite::WorkingPosition * wp = &wp_begin;
				bool found_working_position = false;
				for (const WareAmount& working_position : working_positions) {
					uint32_t count = working_position.second;
					assert(count);
					if (worker_index == working_position.first) {
						while (wp->worker_request) {
							if (--count) {
								++wp;
							} else {
								throw GameDataError
									("request for %s does not match any free working "
									 "position",
									 productionsite.owner().tribe()
									 .get_worker_descr(req.get_index())->name().c_str
									 	());
							}
						}
						found_working_position = true;
						break;
					} else
						wp += count;
				}

				if (!found_working_position)
					throw GameDataError(
					   "site has request for %s, for which there is no working "
					   "position",
						productionsite.owner().tribe().get_worker_descr(req.get_index())->name().c_str());

				wp->worker_request = &req;
			}

			uint16_t nr_workers = fr.unsigned_16();
			for (uint16_t i = nr_workers; i; --i) {
				Worker * worker = &mol.get<Worker>(fr.unsigned_32());

				//  Find a working position that matches this worker.
				const WorkerDescr & worker_descr = worker->descr();
				ProductionSite::WorkingPosition * wp = &wp_begin;
				bool found_working_position = false;
				for (const WareAmount& working_position : working_positions) {
					uint32_t count = working_position.second;
					assert(count);

					if (worker_descr.can_act_as(working_position.first)) {
						while (wp->worker || wp->worker_request) {
							++wp;
							if (!--count)
								goto end_working_position;
						}
						found_working_position = true;
						break;
					} else
						wp += count;
				end_working_position:;
				}

				if (!found_working_position)
					throw GameDataError
						("site has %s, for which there is no free working "
						 "position",
						 worker_descr.name().c_str());
				wp->worker = worker;
			}

			if (nr_worker_requests + nr_workers < pr_descr.nr_working_positions())
				throw GameDataError
					("number of worker requests and workers are fewer than the "
					 "number of working positions");

			//  items from flags
			productionsite.m_fetchfromflag = fr.signed_32();

			//  skipped programs
			uint32_t const gametime = game.get_gametime();
			for (uint8_t i = 3 <= packet_version ? fr.unsigned_8() : 0; i; --i) {
				char const * const program_name = fr.c_string();
				if (pr_descr.programs().count(program_name)) {
					uint32_t const skip_time = fr.unsigned_32();
					if (gametime < skip_time)
						throw GameDataError
							(
							 "program %s was skipped at time %u, but time is only "
							 "%u",
							 program_name, skip_time, gametime);
					productionsite.m_skipped_programs[program_name] = skip_time;
				} else {
					fr.unsigned_32(); // eat skip time
					log
						("WARNING: productionsite has skipped program \"%s\", which "
						 "does not exist\n",
						 program_name);
				}
			}

			//  state
			uint16_t const nr_progs = fr.unsigned_16();
			productionsite.m_stack.resize(nr_progs);
			for (uint16_t i = 0; i < nr_progs; ++i) {
				std::string program_name = fr.c_string();
				std::transform
					(program_name.begin(), program_name.end(), program_name.begin(),
					 tolower);

				productionsite.m_stack[i].program =
					productionsite.descr().get_program(program_name);
				productionsite.m_stack[i].ip    = fr.  signed_32();
				productionsite.m_stack[i].phase = fr.  signed_32();
				productionsite.m_stack[i].flags = fr.unsigned_32();

				if (packet_version >= 5) {
					uint32_t serial = fr.unsigned_32();
					if (serial)
						productionsite.m_stack[i].objvar = &mol.get<MapObject>(serial);
					productionsite.m_stack[i].coord = read_coords_32_allow_null(&fr, game.map().extent());
				}
			}
			productionsite.m_program_timer = fr.unsigned_8();
			productionsite.m_program_time = fr.signed_32();

			uint16_t nr_queues = fr.unsigned_16();
			assert(!productionsite.m_input_queues.size());
			for (uint16_t i = 0; i < nr_queues; ++i) {
				WaresQueue * wq = new WaresQueue(productionsite, INVALID_INDEX, 0);
				wq->read(fr, game, mol);

				if (!game.tribes().ware_exists(wq->get_ware())) {
					delete wq;
				} else {
					productionsite.m_input_queues.push_back(wq);
				}
			}

			uint16_t const stats_size = fr.unsigned_16();
			productionsite.m_statistics.resize(stats_size);
			for (uint32_t i = 0; i < productionsite.m_statistics.size(); ++i)
				productionsite.m_statistics[i] = fr.unsigned_8();
			productionsite.m_statistics_string_on_changed_statistics = fr.c_string();
			productionsite.m_production_result = fr.c_string();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError
			("productionsite (%s): %s",
			 productionsite.descr().descname().c_str(), e.what());
	}
}


void MapBuildingdataPacket::read_trainingsite
	(TrainingSite          & trainingsite,
	 FileRead              & fr,
	 Game                  & game,
	 MapObjectLoader & mol)
{
	try {
		uint16_t const trainingsite_packet_version = fr.unsigned_16();

		if (trainingsite_packet_version == CURRENT_TRAININGSITE_PACKET_VERSION)
		{
			read_productionsite(trainingsite, fr, game, mol);

			delete trainingsite.m_soldier_request;
			trainingsite.m_soldier_request = nullptr;
			if (fr.unsigned_8()) {
				trainingsite.m_soldier_request =
					new Request
						(trainingsite,
						 0,
						 TrainingSite::request_soldier_callback,
						 wwWORKER);
				trainingsite.m_soldier_request->read(fr, game, mol);
			}

			trainingsite.m_capacity = fr.unsigned_8();
			trainingsite.m_build_heroes = fr.unsigned_8();

			uint8_t const nr_upgrades = fr.unsigned_8();
			for (uint8_t i = 0; i < nr_upgrades; ++i) {
				TrainingAttribute attribute = static_cast<TrainingAttribute>(fr.unsigned_8());
				if
					(TrainingSite::Upgrade * const upgrade =
					 	trainingsite.get_upgrade(attribute))
				{
					upgrade->prio = fr.unsigned_8();
					upgrade->credit = fr.unsigned_8();
					upgrade->lastattempt = fr.signed_32();
					upgrade->lastsuccess = fr.unsigned_8();
				} else {
					fr.unsigned_8();
					fr.unsigned_8();
					fr.signed_32();
					fr.signed_32();
				}
			}

			uint16_t mapsize = fr.unsigned_16(); // map of training levels (not _the_ map)
			while (mapsize)
			{
				uint16_t traintype  = fr.unsigned_16();
				uint16_t trainlevel = fr.unsigned_16();
				uint16_t trainstall = fr.unsigned_16();
				uint16_t spresence  = fr.unsigned_8();
				mapsize--;
				std::pair<uint16_t, uint8_t> t = std::make_pair(trainstall, spresence);
				trainingsite.training_failure_count[std::make_pair(traintype, trainlevel)] = t;
			}
		} else
			throw GameDataError
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
		if        (trainingsite.m_capacity < trainingsite.min_soldier_capacity()) {
			log
				("WARNING: trainingsite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it must be at least %u. Changing to that value.\n",
				 trainingsite.serial(), trainingsite.owner().player_number(),
				 trainingsite.get_position().x, trainingsite.get_position().y,
				 trainingsite.m_capacity, trainingsite.min_soldier_capacity());
			trainingsite.m_capacity = trainingsite.min_soldier_capacity();
		} else if (trainingsite.max_soldier_capacity() < trainingsite.m_capacity) {
			log
				("WARNING: trainingsite %u of player %u at (%i, %i) has capacity "
				 "set to %u but it can be at most %u. Changing to that value.\n",
				 trainingsite.serial(), trainingsite.owner().player_number(),
				 trainingsite.get_position().x, trainingsite.get_position().y,
				 trainingsite.m_capacity, trainingsite.max_soldier_capacity());
			trainingsite.m_capacity = trainingsite.max_soldier_capacity();
		}
	} catch (const WException & e) {
		throw GameDataError("trainingsite: %s", e.what());
	}
}


void MapBuildingdataPacket::write
	(FileSystem & fs, EditorGameBase & egbase, MapObjectSaver & mos)
{
	FileWrite fw;

	// now packet version
	fw.unsigned_16(CURRENT_PACKET_VERSION);

	// Walk the map again
	Map & map = egbase.map();
	const uint32_t mapwidth = map.get_width();
	MapIndex const max_index = map.max_index();
	for (MapIndex i = 0; i < max_index; ++i) {
		if (upcast(Building const, building, map[i].get_immovable())) {
			assert(mos.is_object_known(*building));

			if (Map::get_index(building->get_position(), mapwidth) != i)
				continue; // This is not this buildings main position.

			fw.unsigned_32(mos.get_object_file_index(*building));

			//  player immovable owner is already in existence packet

			//  write the general stuff
			if (building->m_anim) {
				fw.unsigned_8(1);
				fw.string(building->descr().get_animation_name(building->m_anim));
			} else
				fw.unsigned_8(0);

			fw.unsigned_32(building->m_animstart);

			{
				const Building::LeaveQueue & leave_queue = building->m_leave_queue;
				fw.unsigned_16(leave_queue.size());
				for (const OPtr<Worker >& temp_queue: leave_queue) {
					assert(mos.is_object_known(*temp_queue.get(egbase)));
					fw.unsigned_32
						(mos.get_object_file_index(*temp_queue.get(egbase)));
				}
			}
			fw.unsigned_32(building->m_leave_time);
			if (MapObject const * const o = building->m_leave_allow.get(egbase))
			{
				assert(mos.is_object_known(*o));
				fw.unsigned_32(mos.get_object_file_index(*o));
			} else {
				fw.unsigned_32(0);
			}
			{
				const TribeDescr& td = building->owner().tribe();
				for (BuildingIndex b_idx : building->m_old_buildings) {
					const BuildingDescr* b_descr = td.get_building_descr(b_idx);
					fw.unsigned_8(1);
					fw.string(b_descr->name());
				}
				fw.unsigned_8(0);
			}
			{
				bool is_stopped = false;
				if (upcast(ProductionSite const, productionsite, building))
					is_stopped = productionsite->is_stopped();
				fw.unsigned_8(is_stopped);
			}

			Game& game = dynamic_cast<Game&>(egbase);

			if (upcast(ConstructionSite const, constructionsite, building)) {
				write_constructionsite(*constructionsite, fw, game, mos);
			} else if (upcast(DismantleSite const, dms, building)) {
				write_dismantlesite(*dms, fw, game, mos);
			} else if (upcast(Warehouse const, warehouse, building)) {
				write_warehouse (*warehouse, fw, game, mos);
			} else if (upcast(ProductionSite const, productionsite, building)) {
				if (upcast(MilitarySite const, militarysite, productionsite)) {
					write_militarysite(*militarysite, fw, game, mos);
				}
				else if (upcast(TrainingSite const, trainingsite, productionsite)) {
					write_trainingsite(*trainingsite, fw, game, mos);
				}
				else {
					write_productionsite(*productionsite, fw, game, mos);
				}
			} else {
				assert(false);
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
			}
			mos.mark_object_as_saved(*building);
		}
	}
	fw.write(fs, "binary/building_data");
}

void MapBuildingdataPacket::write_partially_finished_building
	(const PartiallyFinishedBuilding & pfb,
	 FileWrite              & fw,
	 Game                   & game,
	 MapObjectSaver   & mos)
{
	fw.unsigned_16(CURRENT_PARTIALLYFB_PACKET_VERSION);

	//  descriptions
	fw.string(pfb.m_building->name());

	// builder request
	if (pfb.m_builder_request) {
		fw.unsigned_8(1);
		pfb.m_builder_request->write(fw, game, mos);
	} else
		fw.unsigned_8(0);

	// builder
	if (Worker const * builder = pfb.m_builder.get(game)) {
		assert(mos.is_object_known(*builder));
		fw.unsigned_32(mos.get_object_file_index(*builder));
	} else
		fw.unsigned_32(0);

	const uint16_t wares_size = pfb.m_wares.size();
	fw.unsigned_16(wares_size);
	for (uint16_t i = 0; i < wares_size; ++i)
		pfb.m_wares[i]->write(fw, game, mos);

	fw.unsigned_8 (pfb.m_working);
	fw.unsigned_32(pfb.m_work_steptime);
	fw.unsigned_32(pfb.m_work_completed);
	fw.unsigned_32(pfb.m_work_steps);
}

void MapBuildingdataPacket::write_constructionsite
	(const ConstructionSite & constructionsite,
	 FileWrite              & fw,
	 Game                   & game,
	 MapObjectSaver   & mos)
{

	fw.unsigned_16(CURRENT_CONSTRUCTIONSITE_PACKET_VERSION);

	write_partially_finished_building(constructionsite, fw, game, mos);

	fw.signed_32(constructionsite.m_fetchfromflag);
}

void MapBuildingdataPacket::write_dismantlesite
	(const DismantleSite & dms,
	 FileWrite              & fw,
	 Game                   & game,
	 MapObjectSaver   & mos)
{

	fw.unsigned_16(CURRENT_DISMANTLESITE_PACKET_VERSION);

	write_partially_finished_building(dms, fw, game, mos);

	// Nothing to Do
}


void MapBuildingdataPacket::write_warehouse
	(const Warehouse      & warehouse,
	 FileWrite            & fw,
	 Game                 & game,
	 MapObjectSaver & mos)
{
	fw.unsigned_16(CURRENT_WAREHOUSE_PACKET_VERSION);

	//  supply
	const TribeDescr & tribe = warehouse.owner().tribe();
	const WareList & wares = warehouse.m_supply->get_wares();
	for (WareIndex i = 0; i < wares.get_nrwareids  (); ++i) {
		fw.unsigned_8(1);
		fw.string(tribe.get_ware_descr(i)->name());
		fw.unsigned_32(wares.stock(i));
		fw.unsigned_8(warehouse.get_ware_policy(i));
	}
	fw.unsigned_8(0);
	const WareList & workers = warehouse.m_supply->get_workers();
	for (WareIndex i = 0; i < workers.get_nrwareids(); ++i) {
		fw.unsigned_8(1);
		fw.string(tribe.get_worker_descr(i)->name());
		fw.unsigned_32(workers.stock(i));
		fw.unsigned_8(warehouse.get_worker_policy(i));
	}
	fw.unsigned_8(0);

	//  Incorporated workers, write sorted after file-serial.
	uint32_t nworkers = 0;
	for (const std::pair<WareIndex, Warehouse::WorkerList>& cwt: warehouse.m_incorporated_workers) {
		nworkers += cwt.second.size();
	}

	fw.unsigned_16(nworkers);
	using TWorkerMap = std::map<uint32_t, const Worker *>;
	TWorkerMap workermap;
	for (const std::pair<WareIndex, Warehouse::WorkerList>& cwt : warehouse.m_incorporated_workers) {
		for (Worker * temp_worker : cwt.second) {
			const Worker & w = *temp_worker;
			assert(mos.is_object_known(w));
			workermap.insert
				(std::pair<uint32_t, const Worker *>
				 (mos.get_object_file_index(w), &w));
		}
	}

	for (const std::pair<uint32_t, const Worker *>& temp_worker : workermap) {
		const Worker & obj = *temp_worker.second;
		assert(mos.is_object_known(obj));
		fw.unsigned_32(mos.get_object_file_index(obj));
	}

	{
		const std::vector<WareIndex> & worker_types_without_cost =
			tribe.worker_types_without_cost();
		assert(worker_types_without_cost.size() ==
				 warehouse.m_next_worker_without_cost_spawn.size());
		for (uint8_t i = worker_types_without_cost.size(); i;) {
			const Time& next_spawn = warehouse.m_next_worker_without_cost_spawn[--i];
			if (next_spawn != never()) {
				fw.string
					(tribe.get_worker_descr(tribe.worker_types_without_cost().at(i))
					 ->name());
				fw.unsigned_32(next_spawn);
			}
		}
	}
	fw.unsigned_8(0); //  terminator for spawn times

	fw.unsigned_32(warehouse.m_planned_workers.size());
	for (const Warehouse::PlannedWorkers& temp_worker : warehouse.m_planned_workers) {
		fw.c_string(tribe.get_worker_descr(temp_worker.index)->name());
		fw.unsigned_32(temp_worker.amount);

		fw.unsigned_32(temp_worker.requests.size());

		for (Request * temp_request : temp_worker.requests) {
			temp_request->write(fw, game, mos);
		}
	}

	fw.unsigned_32(warehouse.m_next_stock_remove_act);

	if (warehouse.descr().get_isport()) {
		fw.unsigned_32(mos.get_object_file_index_or_zero(warehouse.m_portdock));

		// Expedition specific stuff. See comment in loader.
		if (warehouse.m_portdock->expedition_started()) {
			warehouse.m_portdock->expedition_bootstrap()->save(fw, game, mos);
		}
	}
}


void MapBuildingdataPacket::write_militarysite
	(const MilitarySite   & militarysite,
	 FileWrite            & fw,
	 Game                 & game,
	 MapObjectSaver & mos)
{
	fw.unsigned_16(CURRENT_MILITARYSITE_PACKET_VERSION);
	write_productionsite(militarysite, fw, game, mos);

	if (militarysite.m_normal_soldier_request) {
		fw.unsigned_8(1);
		militarysite.m_normal_soldier_request->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	if (militarysite.m_upgrade_soldier_request)
	{
		fw.unsigned_8(1);
		militarysite.m_upgrade_soldier_request->write(fw, game, mos);
	}
	else
		fw.unsigned_8(0);


	fw.unsigned_8(militarysite.m_didconquer);
	fw.unsigned_8(militarysite.m_capacity);
	fw.signed_32(militarysite.m_nexthealtime);

	if (militarysite.m_normal_soldier_request)
	{
		if (militarysite.m_upgrade_soldier_request)
			{
				throw GameDataError
				("Internal error in a MilitarySite -- cannot continue. Use previous autosave.");
			}
	}
	fw.unsigned_16(militarysite.m_soldier_upgrade_requirements.get_min());
	fw.unsigned_16(militarysite.m_soldier_upgrade_requirements.get_max());
	fw.unsigned_8(militarysite.m_soldier_preference);
	fw.signed_32(militarysite.m_next_swap_soldiers_time);
	fw.unsigned_8(militarysite.m_soldier_upgrade_try ? 1 : 0);
	fw.unsigned_8(militarysite.m_doing_upgrade_request ? 1 : 0);

}


void MapBuildingdataPacket::write_productionsite
	(const ProductionSite & productionsite,
	 FileWrite            & fw,
	 Game                 & game,
	 MapObjectSaver & mos)
{
	fw.unsigned_16(CURRENT_PRODUCTIONSITE_PACKET_VERSION);

	uint32_t const nr_working_positions =
		productionsite.descr().nr_working_positions();
	const ProductionSite::WorkingPosition & begin =
		productionsite.m_working_positions[0];
	const ProductionSite::WorkingPosition & end =
		(&begin)[nr_working_positions];
	uint32_t nr_workers = 0;
	for (ProductionSite::WorkingPosition const * i = &begin; i < &end; ++i)
		nr_workers += i->worker ? 1 : 0;

	//  worker requests
	fw.unsigned_16(nr_working_positions - nr_workers);
	for (ProductionSite::WorkingPosition const * i = &begin; i < &end; ++i)
		if (Request const * const r = i->worker_request)
			r->write(fw, game, mos);

	//  workers
	fw.unsigned_16(nr_workers);
	for (ProductionSite::WorkingPosition const * i = &begin; i < &end; ++i)
		if (Worker const * const w = i->worker) {
			assert(!i->worker_request);
			assert(mos.is_object_known(*w));
			fw.unsigned_32(mos.get_object_file_index(*w));
		}

	fw.signed_32(productionsite.m_fetchfromflag);

	//  skipped programs
	assert
		(productionsite.m_skipped_programs.size()
		 <=
		 std::numeric_limits<uint8_t>::max());
	fw.unsigned_8(productionsite.m_skipped_programs.size());

	for (const std::pair<std::string, Time>& temp_program : productionsite.m_skipped_programs) {
		fw.string    (temp_program.first);
		fw.unsigned_32(temp_program.second);
	}

	//  state
	uint16_t const program_size = productionsite.m_stack.size();
	fw.unsigned_16(program_size);
	for (uint16_t i = 0; i < program_size; ++i) {
		fw.string    (productionsite.m_stack[i].program->name());
		fw.  signed_32(productionsite.m_stack[i].ip);
		fw.  signed_32(productionsite.m_stack[i].phase);
		fw.unsigned_32(productionsite.m_stack[i].flags);
		fw.unsigned_32(mos.get_object_file_index_or_zero(productionsite.m_stack[i].objvar.get(game)));
		write_coords_32(&fw, productionsite.m_stack[i].coord);
	}
	fw.unsigned_8(productionsite.m_program_timer);
	fw. signed_32(productionsite.m_program_time);

	const uint16_t input_queues_size = productionsite.m_input_queues.size();
	fw.unsigned_16(input_queues_size);
	for (uint16_t i = 0; i < input_queues_size; ++i)
		productionsite.m_input_queues[i]->write(fw, game, mos);

	const uint16_t statistics_size = productionsite.m_statistics.size();
	fw.unsigned_16(statistics_size);
	for (uint32_t i = 0; i < statistics_size; ++i)
		fw.unsigned_8(productionsite.m_statistics[i]);
	fw.string(productionsite.m_statistics_string_on_changed_statistics);
	fw.string(productionsite.production_result());
}

/*
 * write for trainingsite
 */
void MapBuildingdataPacket::write_trainingsite
	(const TrainingSite   & trainingsite,
	 FileWrite            & fw,
	 Game                 & game,
	 MapObjectSaver & mos)
{
	fw.unsigned_16(CURRENT_TRAININGSITE_PACKET_VERSION);

	write_productionsite(trainingsite, fw, game, mos);

	//  requests

	if (trainingsite.m_soldier_request) {
		fw.unsigned_8(1);
		trainingsite.m_soldier_request->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	fw.unsigned_8(trainingsite.m_capacity);
	fw.unsigned_8(trainingsite.m_build_heroes);

	// upgrades
	fw.unsigned_8(trainingsite.m_upgrades.size());
	for (uint8_t i = 0; i < trainingsite.m_upgrades.size(); ++i) {
		const TrainingSite::Upgrade & upgrade = trainingsite.m_upgrades[i];
		fw.unsigned_8(upgrade.attribute);
		fw.unsigned_8(upgrade.prio);
		fw.unsigned_8(upgrade.credit);
		fw.signed_32(upgrade.lastattempt);
		fw.signed_8(upgrade.lastsuccess);
	}
	if (255 < trainingsite.training_failure_count.size())
		log
			("Save TrainingSite: Failure counter has ridiculously many entries! (%u)\n",
			 static_cast<uint16_t>(trainingsite.training_failure_count.size()));
	fw.unsigned_16(static_cast<uint16_t> (trainingsite.training_failure_count.size()));
	for
		(TrainingSite::TrainFailCount::const_iterator i = trainingsite.training_failure_count.begin();
		 i != trainingsite.training_failure_count.end(); i++)
	{
		fw.unsigned_16(i->first.first);
		fw.unsigned_16(i->first.second);
		fw.unsigned_16(i->second.first);
		fw.unsigned_8(i->second.second);
	}

	// DONE
}
}
