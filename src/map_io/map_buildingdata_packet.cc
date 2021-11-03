/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/expedition_bootstrap.h"
#include "economy/flag.h"
#include "economy/input_queue.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/warehousesupply.h"
#include "economy/wares_queue.h"
#include "economy/workers_queue.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/production_program.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "map_io/map_packet_versions.h"

namespace Widelands {

// Overall package version
constexpr uint16_t kCurrentPacketVersion = 8;

// Building type package versions
constexpr uint16_t kCurrentPacketVersionDismantlesite = 1;
constexpr uint16_t kCurrentPacketVersionConstructionsite = 5;
constexpr uint16_t kCurrentPacketPFBuilding = 2;
constexpr uint16_t kCurrentPacketVersionMilitarysite = 7;
constexpr uint16_t kCurrentPacketVersionProductionsite = 9;
constexpr uint16_t kCurrentPacketVersionTrainingsite = 7;

void MapBuildingdataPacket::read(FileSystem& fs,
                                 EditorGameBase& egbase,
                                 bool const skip,
                                 MapObjectLoader& mol) {
	if (skip) {
		return;
	}

	FileRead fr;
	try {
		fr.open(fs, "binary/building_data");
	} catch (...) {
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 4) {
			while (!fr.end_of_file()) {
				Serial const serial = fr.unsigned_32();
				try {
					Building& building = mol.get<Building>(serial);

					// Animation. If the animation is no longer known, pick the main animation instead.
					if (fr.unsigned_8()) {
						char const* const animname = fr.c_string();
						if (building.descr().is_animation_known(animname)) {
							building.anim_ = building.descr().get_animation(animname, &building);
						} else {
							log_warn(
							   "Unknown animation '%s' for building '%s', using main animation instead.\n",
							   animname, building.descr().name().c_str());
							building.anim_ = building.descr().main_animation();
						}
					} else {
						building.anim_ = building.descr().main_animation();
					}
					building.animstart_ = Time(fr);

					{
						Building::LeaveQueue& leave_queue = building.leave_queue_;
						leave_queue.resize(fr.unsigned_16());

						for (Building::LeaveQueue::iterator queue_iter = leave_queue.begin();
						     queue_iter != leave_queue.end(); ++queue_iter) {

							if (uint32_t const leaver_serial = fr.unsigned_32()) {
								try {
									//  The check that this worker actually has a
									//  leavebuilding task for this building is in
									//  Building::load_finish, which is called after the
									//  worker (with his stack of tasks) has been fully
									//  loaded.
									*queue_iter = &mol.get<Worker>(leaver_serial);
								} catch (const WException& e) {
									throw GameDataError(
									   "leave queue item #%" PRIuS " (%u): %s",
									   static_cast<size_t>(queue_iter - leave_queue.begin()), leaver_serial,
									   e.what());
								}
							} else {
								*queue_iter = nullptr;
							}
						}
					}

					building.leave_time_ = Time(fr);
					building.worker_evicted_ = packet_version >= 8 ? Time(fr) : Time();

					building.mute_messages_ = packet_version >= 6 && fr.unsigned_8();

					for (size_t i = (packet_version >= 7 ? fr.unsigned_32() : 0); i; --i) {
						const std::string warename(fr.string());
						building.set_priority(
						   wwWARE, egbase.descriptions().ware_index(warename), WarePriority(fr));
					}

					if (uint32_t const leaver_serial = fr.unsigned_32()) {
						try {
							building.leave_allow_ = &mol.get<MapObject>(leaver_serial);
						} catch (const WException& e) {
							throw GameDataError("leave allow item (%u): %s", leaver_serial, e.what());
						}
					} else {
						building.leave_allow_ = nullptr;
					}

					if (packet_version >= 5) {
						while (fr.unsigned_8()) {
							const std::string map_object_name(fr.c_string());
							const std::string type(fr.c_string());
							DescriptionIndex oldidx = INVALID_INDEX;
							// TODO(Nordfriese): `type.empty()` is only allowed for
							// savegame compatibility, disallow after v1.0
							if (type.empty() || type == "building") {
								oldidx = building.owner().tribe().safe_building_index(map_object_name);
							} else if (type == "immovable") {
								oldidx = building.owner().tribe().immovable_index(map_object_name);
								building.was_immovable_ =
								   building.owner().tribe().get_immovable_descr(oldidx);
							} else {
								throw GameDataError(
								   "Invalid FormerBuildings type %s, expected 'building' or 'immovable'",
								   type.c_str());
							}
							assert(oldidx != INVALID_INDEX);
							building.old_buildings_.push_back(std::make_pair(oldidx, type != "immovable"));
						}
					} else {
						while (fr.unsigned_8()) {
							DescriptionIndex oldidx =
							   building.owner().tribe().safe_building_index(fr.c_string());
							building.old_buildings_.push_back(std::make_pair(oldidx, true));
						}
					}
					// Only construction sites may have an empty list
					if (building.old_buildings_.empty() &&
					    building.descr().type() != MapObjectType::CONSTRUCTIONSITE) {
						throw GameDataError("Failed to read %s %u: No former buildings information.\n"
						                    "Your savegame is corrupted",
						                    building.descr().name().c_str(), building.serial());
					}

					if (fr.unsigned_8()) {
						if (upcast(ProductionSite, productionsite, &building)) {
							if (building.descr().type() == MapObjectType::MILITARYSITE) {
								log_warn("Found a stopped %s at (%i, %i) in the "
								         "savegame. Militarysites are not stoppable. "
								         "Ignoring.",
								         building.descr().name().c_str(), building.get_position().x,
								         building.get_position().y);
							} else {
								productionsite->set_stopped(true);
							}
						} else {
							log_warn("Found a stopped %s at (%i, %i) in the "
							         "savegame. Only productionsites are stoppable. "
							         "Ignoring.",
							         building.descr().name().c_str(), building.get_position().x,
							         building.get_position().y);
						}
					}

					//  Set economy now, some stuff below will count on this.
					building.set_economy(building.flag_->get_economy(wwWARE), wwWARE);
					building.set_economy(building.flag_->get_economy(wwWORKER), wwWORKER);

					Game& game = dynamic_cast<Game&>(egbase);

					if (upcast(ConstructionSite, constructionsite, &building)) {
						read_constructionsite(*constructionsite, fr, game, mol);
					} else if (upcast(DismantleSite, dms, &building)) {
						read_dismantlesite(*dms, fr, game, mol);
					} else if (upcast(MilitarySite, militarysite, &building)) {
						read_militarysite(*militarysite, fr, game, mol);
					} else if (upcast(Warehouse, warehouse, &building)) {
						read_warehouse(*warehouse, fr, game, mol);
					} else if (upcast(ProductionSite, productionsite, &building)) {
						if (upcast(TrainingSite, trainingsite, productionsite)) {
							read_trainingsite(*trainingsite, fr, game, mol);
						} else {
							read_productionsite(*productionsite, fr, game, mol);
						}
					} else {
						//  type of building is not one of (or derived from)
						//  {ConstructionSite, Warehouse, ProductionSite}
						NEVER_HERE();
					}
					mol.mark_object_as_loaded(building);
				} catch (const WException& e) {
					throw GameDataError("building %u: %s", serial, e.what());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapBuildingdataPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("buildingdata: %s", e.what());
	}
}

void MapBuildingdataPacket::read_partially_finished_building(PartiallyFinishedBuilding& pfb,
                                                             FileRead& fr,
                                                             Game& game,
                                                             MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketPFBuilding) {
			const TribeDescr& tribe = pfb.owner().tribe();
			pfb.building_ = tribe.get_building_descr(tribe.safe_building_index(fr.c_string()));

			delete pfb.builder_request_;
			if (fr.unsigned_8()) {
				pfb.builder_request_ =
				   new Request(pfb, 0, PartiallyFinishedBuilding::request_builder_callback, wwWORKER);
				pfb.builder_request_->read(fr, game, mol);
			} else {
				pfb.builder_request_ = nullptr;
			}

			if (uint32_t const builder_serial = fr.unsigned_32()) {
				try {
					pfb.builder_ = &mol.get<Worker>(builder_serial);
				} catch (const WException& e) {
					throw GameDataError("builder (%u): %s", builder_serial, e.what());
				}
			} else {
				pfb.builder_ = nullptr;
			}

			try {
				uint16_t size = fr.unsigned_16();
				pfb.consume_wares_.resize(size);
				for (uint16_t i = 0; i < pfb.consume_wares_.size(); ++i) {
					pfb.consume_wares_[i] = new WaresQueue(pfb, INVALID_INDEX, 0);
					pfb.consume_wares_[i]->read(fr, game, mol);
				}

				size = fr.unsigned_16();
				pfb.dropout_wares_.resize(size);
				for (uint16_t i = 0; i < pfb.dropout_wares_.size(); ++i) {
					pfb.dropout_wares_[i] = new WaresQueue(pfb, INVALID_INDEX, 0);
					pfb.dropout_wares_[i]->read(fr, game, mol);
				}
			} catch (const WException& e) {
				throw GameDataError("wares: %s", e.what());
			}

			pfb.working_ = fr.unsigned_8();
			pfb.work_steptime_ = Time(fr);
			pfb.work_completed_ = fr.unsigned_32();
			pfb.work_steps_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Partially Finished Building",
			                            packet_version, kCurrentPacketPFBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("partially_constructed_buildings: %s", e.what());
	}
}

void MapBuildingdataPacket::read_constructionsite(ConstructionSite& constructionsite,
                                                  FileRead& fr,
                                                  Game& game,
                                                  MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 3 && packet_version <= kCurrentPacketVersionConstructionsite) {
			read_partially_finished_building(constructionsite, fr, game, mol);

			for (ConstructionSite::Wares::iterator wares_iter =
			        constructionsite.consume_wares_.begin();
			     wares_iter != constructionsite.consume_wares_.end(); ++wares_iter) {

				(*wares_iter)->set_callback(ConstructionSite::wares_queue_callback, &constructionsite);
			}

			constructionsite.fetchfromflag_ = fr.signed_32();

			if (packet_version >= 4) {
				const uint32_t intermediates = fr.unsigned_32();
				for (uint32_t i = 0; i < intermediates; ++i) {
					constructionsite.info_.intermediates.push_back(
					   game.descriptions().get_building_descr(
					      game.descriptions().safe_building_index(fr.c_string())));
				}
				constructionsite.settings_.reset(
				   BuildingSettings::load(game, constructionsite.owner().tribe(), fr));
			} else {
				constructionsite.init_settings();
			}

			if (packet_version >= 5) {
				for (uint32_t i = fr.unsigned_32(); i; --i) {
					const std::string item = fr.string();
					const uint32_t amount = fr.unsigned_32();
					constructionsite.additional_wares_[game.mutable_descriptions()->load_ware(item)] =
					   amount;
				}
				for (uint32_t i = fr.unsigned_32(); i; --i) {
					constructionsite.additional_workers_.push_back(&mol.get<Worker>(fr.unsigned_32()));
				}
			}
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Constructionsite", packet_version,
			                            kCurrentPacketVersionConstructionsite);
		}
	} catch (const WException& e) {
		throw GameDataError("constructionsite: %s", e.what());
	}
}

void MapBuildingdataPacket::read_dismantlesite(DismantleSite& dms,
                                               FileRead& fr,
                                               Game& game,
                                               MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDismantlesite) {
			read_partially_finished_building(dms, fr, game, mol);

			// Nothing to do
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Dismantlesite", packet_version,
			                            kCurrentPacketVersionDismantlesite);
		}
	} catch (const WException& e) {
		throw GameDataError("dismantlesite: %s", e.what());
	}
}

void MapBuildingdataPacket::read_warehouse(Warehouse& warehouse,
                                           FileRead& fr,
                                           Game& game,
                                           MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 6) {
			Player* player = warehouse.get_owner();
			warehouse.init_containers(*player);
			const TribeDescr& tribe = player->tribe();

			while (fr.unsigned_8()) {
				const DescriptionIndex& id = game.mutable_descriptions()->load_ware(fr.c_string());
				Quantity amount = fr.unsigned_32();
				StockPolicy policy = static_cast<StockPolicy>(fr.unsigned_8());

				// TODO(GunChleoc): Change these from vector to map so that we can avoid saveloading
				// unused wares
				if (tribe.has_ware(id)) {
					warehouse.insert_wares(id, amount);
					warehouse.set_ware_policy(id, policy);
				}
			}
			while (fr.unsigned_8()) {
				const DescriptionIndex& id = game.mutable_descriptions()->load_worker(fr.c_string());
				uint32_t amount = fr.unsigned_32();
				StockPolicy policy = static_cast<StockPolicy>(fr.unsigned_8());

				// TODO(GunChleoc): Change these from vector to map so that we can avoid saveloading
				// unused workers
				if (tribe.has_worker(id)) {
					warehouse.insert_workers(id, amount);
					warehouse.set_worker_policy(id, policy);
				}
			}

			// TODO(sirver,trading): Pull out and reuse this for market workers.
			assert(warehouse.incorporated_workers_.empty());
			{
				uint16_t const nrworkers = fr.unsigned_16();
				for (uint16_t i = 0; i < nrworkers; ++i) {
					uint32_t const worker_serial = fr.unsigned_32();

					try {
						Worker& worker = mol.get<Worker>(worker_serial);
						const DescriptionIndex& worker_index = tribe.worker_index(worker.descr().name());
						if (!warehouse.incorporated_workers_.count(worker_index)) {
							warehouse.incorporated_workers_[worker_index] = Warehouse::WorkerList();
						}
						warehouse.incorporated_workers_[worker_index].push_back(&worker);
					} catch (const WException& e) {
						throw GameDataError(
						   "incorporated worker #%u (%u): %s", i, worker_serial, e.what());
					}
				}
			}

			const std::vector<DescriptionIndex>& worker_types_without_cost =
			   tribe.worker_types_without_cost();

			for (;;) {
				const std::string worker_typename = fr.c_string();
				if (worker_typename.empty()) {  //  No more workers found
					break;
				}
				const Time next_spawn(fr);
				DescriptionIndex const worker_index = tribe.safe_worker_index(worker_typename);
				if (!game.descriptions().worker_exists(worker_index)) {
					log_warn("%s %u has a next_spawn time for nonexistent "
					         "worker type \"%s\" set to %u, ignoring\n",
					         warehouse.descr().name().c_str(), warehouse.serial(),
					         worker_typename.c_str(), next_spawn.get());
					continue;
				}
				if (!tribe.get_worker_descr(worker_index)->buildcost().empty()) {
					log_warn("%s %u has a next_spawn time for worker type "
					         "\"%s\", that costs something to build, set to %u, "
					         "ignoring\n",
					         warehouse.descr().name().c_str(), warehouse.serial(),
					         worker_typename.c_str(), next_spawn.get());
					continue;
				}
				for (uint8_t i = 0;; ++i) {
					assert(i < worker_types_without_cost.size());
					if (worker_types_without_cost.at(i) == worker_index) {
						if (warehouse.next_worker_without_cost_spawn_[i].is_valid()) {
							throw GameDataError("%s %u has a next_spawn time for worker type "
							                    "\"%s\" set to %u, but it was previously set "
							                    "to %u\n",
							                    warehouse.descr().name().c_str(), warehouse.serial(),
							                    worker_typename.c_str(), next_spawn.get(),
							                    warehouse.next_worker_without_cost_spawn_[i].get());
						}
						warehouse.next_worker_without_cost_spawn_[i] = next_spawn;
						break;
					}
				}
			}
			//  The checks that the warehouse has a next_spawn time for each
			//  worker type that the player is allowed to spawn, is in
			//  Warehouse::load_finish.

			// Read planned worker data
			// Consistency checks are in Warehouse::load_finish
			uint32_t nr_planned_workers = fr.unsigned_32();
			while (nr_planned_workers--) {
				warehouse.planned_workers_.push_back(Warehouse::PlannedWorkers());
				Warehouse::PlannedWorkers& pw = warehouse.planned_workers_.back();
				pw.index = tribe.safe_worker_index(fr.c_string());
				pw.amount = fr.unsigned_32();

				uint32_t nr_requests = fr.unsigned_32();
				while (nr_requests--) {
					// We have no information regarding the index or WareWorker type yet.
					// Initialize with default values which will be overridden by read().
					// read() will also take care of adding the request to the correct economy.
					pw.requests.push_back(new Request(warehouse, 0, &Warehouse::request_cb, wwWORKER));
					pw.requests.back()->read(fr, game, mol);
				}
			}

			warehouse.next_stock_remove_act_ = Time(fr);

			if (warehouse.descr().get_isport()) {
				if (Serial portdock = fr.unsigned_32()) {
					warehouse.portdock_ = &mol.get<PortDock>(portdock);
					warehouse.portdock_->set_economy(warehouse.get_economy(wwWARE), wwWARE);
					warehouse.portdock_->set_economy(warehouse.get_economy(wwWORKER), wwWORKER);
					// Expedition specific stuff. This is done in this packet
					// because the "new style" loader is not supported and
					// doesn't lend itself to request and other stuff.
					if (warehouse.portdock_->expedition_started()) {
						warehouse.portdock_->expedition_bootstrap()->load(
						   warehouse, fr, game, mol, packet_version);
					}
				}
			}

			const Map& map = game.map();

			if (uint32_t const conquer_radius = warehouse.descr().get_conquers()) {
				//  Add to map of military influence.
				Area<FCoords> a(map.get_fcoords(warehouse.get_position()), conquer_radius);
				const Field& first_map_field = map[0];
				Player::Field* const player_fields = player->fields_.get();
				MapRegion<Area<FCoords>> mr(map, a);
				do {
					player_fields[mr.location().field - &first_map_field].military_influence +=
					   map.calc_influence(mr.location(), Area<>(a, a.radius));
				} while (mr.advance(map));
			}
			warehouse.next_military_act_ = game.get_gametime();
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Warehouse", packet_version,
			                            kCurrentPacketVersionWarehouseAndExpedition);
		}
	} catch (const WException& e) {
		throw GameDataError("warehouse: %s", e.what());
	}
}

void MapBuildingdataPacket::read_militarysite(MilitarySite& militarysite,
                                              FileRead& fr,
                                              Game& game,
                                              MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 5 && packet_version <= kCurrentPacketVersionMilitarysite) {
			militarysite.normal_soldier_request_.reset();

			if (fr.unsigned_8()) {
				militarysite.normal_soldier_request_.reset(
				   new Request(militarysite, 0, MilitarySite::request_soldier_callback, wwWORKER));
				militarysite.normal_soldier_request_->read(fr, game, mol);
			} else {
				militarysite.normal_soldier_request_.reset();
			}

			if (fr.unsigned_8()) {
				militarysite.upgrade_soldier_request_.reset(new Request(
				   militarysite,
				   (!militarysite.normal_soldier_request_) ? 0 : militarysite.owner().tribe().soldier(),
				   MilitarySite::request_soldier_callback, wwWORKER));
				militarysite.upgrade_soldier_request_->read(fr, game, mol);
			} else {
				militarysite.upgrade_soldier_request_.reset();
			}

			if ((militarysite.didconquer_ = fr.unsigned_8())) {
				//  Add to map of military influence.
				const Map& map = game.map();
				Area<FCoords> a(
				   map.get_fcoords(militarysite.get_position()), militarysite.descr().get_conquers());
				const Field& first_map_field = map[0];
				Player::Field* const player_fields = militarysite.owner().fields_.get();
				MapRegion<Area<FCoords>> mr(map, a);
				do {
					player_fields[mr.location().field - &first_map_field].military_influence +=
					   map.calc_influence(mr.location(), Area<>(a, a.radius));
				} while (mr.advance(map));
			}

			//  capacity (modified by user)
			militarysite.capacity_ = fr.unsigned_8();
			militarysite.nexthealtime_ = Time(fr);

			uint16_t reqmin = fr.unsigned_16();
			uint16_t reqmax = fr.unsigned_16();
			militarysite.soldier_upgrade_requirements_ =
			   RequireAttribute(TrainingAttribute::kTotal, reqmin, reqmax);
			militarysite.soldier_preference_ = static_cast<SoldierPreference>(fr.unsigned_8());
			militarysite.next_swap_soldiers_time_ = Time(fr);
			militarysite.soldier_upgrade_try_ = 0 != fr.unsigned_8();
			militarysite.doing_upgrade_request_ = 0 != fr.unsigned_8();

			// TODO(Nordfriese): Savegame compatibility
			if (packet_version >= 7) {
				for (uint8_t i = fr.unsigned_8(); i; --i) {
					const PlayerNumber p = fr.unsigned_8();
					const bool b = fr.unsigned_8();
					militarysite.attack_target_.allow_conquer_[p] = b;
				}
			}

		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Militarysite", packet_version,
			                            kCurrentPacketVersionMilitarysite);
		}

		//  If the site's capacity is outside the allowed range (can happen if
		//  the site's type's definition has changed), set the variable to the
		//  nearest valid value.
		//
		//  This does not drop excessive soldiers, since they are not loaded into
		//  the site yet. To do that we would have to do this change by adding a
		//  Cmd_ChangeSoldierCapacity to the beginning of the game's command
		//  queue. But that would not work because the command queue is not read
		//  yet and will be cleared before it is read.
		if (militarysite.capacity_ < militarysite.soldier_control()->min_soldier_capacity()) {
			log_warn("militarysite %u of player %u at (%i, %i) has capacity "
			         "set to %u but it must be at least %u. Changing to that value.\n",
			         militarysite.serial(), militarysite.owner().player_number(),
			         militarysite.get_position().x, militarysite.get_position().y,
			         militarysite.capacity_, militarysite.soldier_control()->min_soldier_capacity());
			militarysite.capacity_ = militarysite.soldier_control()->min_soldier_capacity();
		} else if (militarysite.soldier_control()->max_soldier_capacity() < militarysite.capacity_) {
			log_warn("militarysite %u of player %u at (%i, %i) has capacity "
			         "set to %u but it can be at most %u. Changing to that value.\n",
			         militarysite.serial(), militarysite.owner().player_number(),
			         militarysite.get_position().x, militarysite.get_position().y,
			         militarysite.capacity_, militarysite.soldier_control()->max_soldier_capacity());
			militarysite.capacity_ = militarysite.soldier_control()->max_soldier_capacity();
		}
	} catch (const WException& e) {
		throw GameDataError("militarysite: %s", e.what());
	}
}

void MapBuildingdataPacket::read_productionsite(ProductionSite& productionsite,
                                                FileRead& fr,
                                                Game& game,
                                                MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionProductionsite) {
			const auto& wp_begin = productionsite.working_positions_.begin();
			const ProductionSiteDescr& pr_descr = productionsite.descr();
			const BillOfMaterials& working_positions = pr_descr.working_positions();

			uint16_t nr_worker_requests = fr.unsigned_16();
			for (uint16_t i = nr_worker_requests; i; --i) {
				Request& req =
				   *new Request(productionsite, 0, ProductionSite::request_worker_callback, wwWORKER);
				req.read(fr, game, mol);
				const DescriptionIndex& worker_index = req.get_index();

				//  Find a working position that matches this request.
				auto wp = wp_begin;
				bool found_working_position = false;
				for (const auto& working_position : working_positions) {
					uint32_t count = working_position.second;
					assert(count);
					if (worker_index == working_position.first) {
						while (wp->worker_request) {
							if (--count) {
								++wp;
							} else {
								throw GameDataError("request for %s does not match any free working "
								                    "position",
								                    productionsite.owner()
								                       .tribe()
								                       .get_worker_descr(req.get_index())
								                       ->name()
								                       .c_str());
							}
						}
						found_working_position = true;
						break;
					} else {
						wp += count;
					}
				}

				if (!found_working_position) {
					throw GameDataError(
					   "site has request for %s, for which there is no working "
					   "position",
					   productionsite.owner().tribe().get_worker_descr(req.get_index())->name().c_str());
				}

				wp->worker_request = &req;
			}

			uint16_t nr_workers = fr.unsigned_16();
			for (uint16_t i = nr_workers; i; --i) {
				Worker* worker = &mol.get<Worker>(fr.unsigned_32());

				//  Find a working position that matches this worker.
				const WorkerDescr& worker_descr = worker->descr();
				auto wp = wp_begin;
				bool found_working_position = false;
				for (const auto& working_position : working_positions) {
					uint32_t count = working_position.second;
					assert(count);

					if (worker_descr.can_act_as(working_position.first)) {
						while (wp->worker.get(game) || wp->worker_request) {
							++wp;
							if (!--count) {
								goto end_working_position;
							}
						}
						found_working_position = true;
						break;
					} else {
						wp += count;
					}
				end_working_position:;
				}

				if (!found_working_position) {
					throw GameDataError("site has %s, for which there is no free working "
					                    "position",
					                    worker_descr.name().c_str());
				}
				wp->worker = worker;
			}

			if (nr_worker_requests + nr_workers < pr_descr.nr_working_positions()) {
				throw GameDataError("number of worker requests and workers are fewer than the "
				                    "number of working positions");
			}

			//  items from flags
			productionsite.fetchfromflag_ = fr.signed_32();

			//  skipped programs
			const Time& gametime = game.get_gametime();
			for (uint8_t i = fr.unsigned_8(); i; --i) {
				char const* const program_name = fr.c_string();
				if (pr_descr.programs().count(program_name)) {
					const Time skip_time(fr);
					if (gametime < skip_time) {
						throw GameDataError("program %s failed/was skipped at time %u, but time is only "
						                    "%u",
						                    program_name, skip_time.get(), gametime.get());
					}
					productionsite.failed_skipped_programs_[program_name] = skip_time;
				} else {
					const Time skip_time(fr);  // eat skip time
					log_warn("productionsite has failed/skipped program \"%s\", which "
					         "does not exist\n",
					         program_name);
				}
			}

			//  state
			uint16_t const nr_progs = fr.unsigned_16();
			productionsite.stack_.resize(nr_progs);
			for (uint16_t i = 0; i < nr_progs; ++i) {
				std::string program_name = fr.c_string();
				program_name = to_lower(program_name);
				if (!pr_descr.programs().count(program_name)) {
					log_warn("productionsite has unknown program \"%s\", replacing it with "
					         "\"main\"\n",
					         program_name.c_str());
					program_name = MapObjectProgram::kMainProgram;
				}

				productionsite.stack_[i].program = productionsite.descr().get_program(program_name);
				productionsite.stack_[i].ip = fr.signed_32();
				productionsite.stack_[i].phase = static_cast<ProgramResult>(fr.signed_32());
				productionsite.stack_[i].flags = fr.unsigned_32();

				uint32_t serial = fr.unsigned_32();
				if (serial) {
					productionsite.stack_[i].objvar = &mol.get<MapObject>(serial);
				}
				productionsite.stack_[i].coord = read_coords_32_allow_null(&fr, game.map().extent());
			}
			productionsite.program_timer_ = fr.unsigned_8();
			productionsite.program_time_ = Time(fr);

			uint16_t nr_queues = fr.unsigned_16();
			assert(productionsite.input_queues_.empty());
			for (uint16_t i = 0; i < nr_queues; ++i) {
				WaresQueue* wq = new WaresQueue(productionsite, INVALID_INDEX, 0);
				wq->read(fr, game, mol);

				if (!game.descriptions().ware_exists(wq->get_index())) {
					delete wq;
				} else {
					productionsite.input_queues_.push_back(wq);
				}
			}

			nr_queues = fr.unsigned_16();
			for (uint16_t i = 0; i < nr_queues; ++i) {
				WorkersQueue* wq = new WorkersQueue(productionsite, INVALID_INDEX, 0);
				wq->read(fr, game, mol);

				if (!game.descriptions().worker_exists(wq->get_index())) {
					delete wq;
				} else {
					productionsite.input_queues_.push_back(wq);
				}
			}

			productionsite.actual_percent_ = fr.unsigned_32();
			productionsite.statistics_string_on_changed_statistics_ = fr.c_string();
			productionsite.production_result_ = fr.c_string();
			productionsite.main_worker_ = -1;

			if (fr.unsigned_8()) {
				const Worker& worker = mol.get<Worker>(fr.unsigned_32());
				int32_t i = 0;
				// Determine main worker's index as this may change during saveloading (#3891)
				for (const ProductionSite::WorkingPosition& wp : *productionsite.working_positions()) {
					if (wp.worker.get(game) == &worker) {
						productionsite.main_worker_ = i;
						break;
					}
					++i;
				}
			}
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Productionsite", packet_version,
			                            kCurrentPacketVersionProductionsite);
		}
	} catch (const WException& e) {
		throw GameDataError(
		   "productionsite (%s): %s", productionsite.descr().name().c_str(), e.what());
	}
}

void MapBuildingdataPacket::read_trainingsite(TrainingSite& trainingsite,
                                              FileRead& fr,
                                              Game& game,
                                              MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 6 && packet_version <= kCurrentPacketVersionTrainingsite) {

			read_productionsite(trainingsite, fr, game, mol);

			delete trainingsite.soldier_request_;
			trainingsite.soldier_request_ = nullptr;
			if (fr.unsigned_8()) {
				trainingsite.soldier_request_ =
				   new Request(trainingsite, 0, TrainingSite::request_soldier_callback, wwWORKER);
				trainingsite.soldier_request_->read(fr, game, mol);
			}

			trainingsite.capacity_ = fr.unsigned_8();
			trainingsite.build_heroes_ = fr.unsigned_8();

			uint8_t const nr_upgrades = fr.unsigned_8();
			for (uint8_t i = 0; i < nr_upgrades; ++i) {
				TrainingAttribute attribute = static_cast<TrainingAttribute>(fr.unsigned_8());
				if (TrainingSite::Upgrade* const upgrade = trainingsite.get_upgrade(attribute)) {
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

			uint16_t mapsize = fr.unsigned_16();  // map of training levels (not _the_ map)
			while (mapsize) {
				// Get the training attribute and check if it is a valid enum member
				// We use a temp value, because the static_cast to the enum might be undefined.
				uint8_t temp_traintype = fr.unsigned_8();
				switch (temp_traintype) {
				case static_cast<uint8_t>(TrainingAttribute::kHealth):
				case static_cast<uint8_t>(TrainingAttribute::kAttack):
				case static_cast<uint8_t>(TrainingAttribute::kDefense):
				case static_cast<uint8_t>(TrainingAttribute::kEvade):
				case static_cast<uint8_t>(TrainingAttribute::kTotal):
					break;
				default:
					throw GameDataError("expected kHealth (%u), kAttack (%u), kDefense (%u), kEvade "
					                    "(%u) or kTotal (%u) but found unknown attribute value (%u)",
					                    static_cast<unsigned int>(TrainingAttribute::kHealth),
					                    static_cast<unsigned int>(TrainingAttribute::kAttack),
					                    static_cast<unsigned int>(TrainingAttribute::kDefense),
					                    static_cast<unsigned int>(TrainingAttribute::kEvade),
					                    static_cast<unsigned int>(TrainingAttribute::kTotal),
					                    temp_traintype);
				}
				TrainingAttribute traintype = static_cast<TrainingAttribute>(temp_traintype);
				uint16_t trainlevel = fr.unsigned_16();
				uint16_t trainstall = fr.unsigned_16();
				uint16_t spresence = fr.unsigned_8();
				mapsize--;
				trainingsite.training_failure_count_[std::make_pair(traintype, trainlevel)] =
				   std::make_pair(trainstall, spresence);
			}

			trainingsite.highest_trainee_level_seen_ = fr.unsigned_8();
			trainingsite.latest_trainee_kickout_level_ = fr.unsigned_8();
			trainingsite.trainee_general_lower_bound_ = fr.unsigned_8();
			uint8_t somebits = fr.unsigned_8();
			trainingsite.latest_trainee_was_kickout_ = 0 < (somebits & 1);
			trainingsite.requesting_weak_trainees_ = 0 < (somebits & 2);
			trainingsite.repeated_layoff_inc_ = 0 < (somebits & 4);
			trainingsite.recent_capacity_increase_ = 0 < (somebits & 8);
			assert(16 > somebits);
			trainingsite.repeated_layoff_ctr_ = fr.unsigned_8();
			trainingsite.request_open_since_ = Time(fr);

			// TODO(Niektory): Savegame compatibility
			if (packet_version >= 7) {
				trainingsite.checked_soldier_training_.attribute =
				   static_cast<TrainingAttribute>(fr.unsigned_8());
				trainingsite.checked_soldier_training_.level = fr.unsigned_8();
			}
		} else {
			throw UnhandledVersionError("MapBuildingdataPacket - Trainingsite", packet_version,
			                            kCurrentPacketVersionTrainingsite);
		}

		//  If the site's capacity is outside the allowed range (can happen if
		//  the site's type's definition has changed), set the variable to the
		//  nearest valid value.
		//
		//  This does not drop excessive soldiers, since they are not loaded into
		//  the site yet. To do that we would have to do this change by adding a
		//  Cmd_ChangeSoldierCapacity to the beginning of the game's command
		//  queue. But that would not work because the command queue is not read
		//  yet and will be cleared before it is read.
		if (trainingsite.capacity_ < trainingsite.soldier_control()->min_soldier_capacity()) {
			log_warn("trainingsite %u of player %u at (%i, %i) has capacity "
			         "set to %u but it must be at least %u. Changing to that value.\n",
			         trainingsite.serial(), trainingsite.owner().player_number(),
			         trainingsite.get_position().x, trainingsite.get_position().y,
			         trainingsite.capacity_, trainingsite.soldier_control()->min_soldier_capacity());
			trainingsite.capacity_ = trainingsite.soldier_control()->min_soldier_capacity();
		} else if (trainingsite.soldier_control()->max_soldier_capacity() < trainingsite.capacity_) {
			log_warn("trainingsite %u of player %u at (%i, %i) has capacity "
			         "set to %u but it can be at most %u. Changing to that value.\n",
			         trainingsite.serial(), trainingsite.owner().player_number(),
			         trainingsite.get_position().x, trainingsite.get_position().y,
			         trainingsite.capacity_, trainingsite.soldier_control()->max_soldier_capacity());
			trainingsite.capacity_ = trainingsite.soldier_control()->max_soldier_capacity();
		}
	} catch (const WException& e) {
		throw GameDataError("trainingsite: %s", e.what());
	}
}

void MapBuildingdataPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver& mos) {
	FileWrite fw;

	// now packet version
	fw.unsigned_16(kCurrentPacketVersion);

	// Walk the map again
	const Map& map = egbase.map();
	const uint32_t mapwidth = map.get_width();
	MapIndex const max_index = map.max_index();
	for (MapIndex i = 0; i < max_index; ++i) {
		if (upcast(Building const, building, map[i].get_immovable())) {
			assert(mos.is_object_known(*building));

			if (Map::get_index(building->get_position(), mapwidth) != i) {
				continue;  // This is not this buildings main position.
			}

			fw.unsigned_32(mos.get_object_file_index(*building));

			//  player immovable owner is already in existence packet

			//  write the general stuff
			if (building->anim_) {
				fw.unsigned_8(1);
				fw.string(building->descr().get_animation_name(building->anim_));
			} else {
				fw.unsigned_8(0);
			}

			building->animstart_.save(fw);

			{
				const Building::LeaveQueue& leave_queue = building->leave_queue_;
				fw.unsigned_16(leave_queue.size());
				for (const OPtr<Worker>& temp_queue : leave_queue) {
					assert(mos.is_object_known(*temp_queue.get(egbase)));
					fw.unsigned_32(mos.get_object_file_index(*temp_queue.get(egbase)));
				}
			}
			building->leave_time_.save(fw);
			building->worker_evicted_.save(fw);
			fw.unsigned_8(building->mute_messages_ ? 1 : 0);

			fw.unsigned_32(building->ware_priorities_.size());
			for (const auto& pair : building->ware_priorities_) {
				fw.string(egbase.descriptions().get_ware_descr(pair.first)->name());
				pair.second.write(fw);
			}

			if (MapObject const* const o = building->leave_allow_.get(egbase)) {
				assert(mos.is_object_known(*o));
				fw.unsigned_32(mos.get_object_file_index(*o));
			} else {
				fw.unsigned_32(0);
			}
			{
				const TribeDescr& td = building->owner().tribe();
				for (const auto& pair : building->old_buildings_) {
					const MapObjectDescr* b_descr = nullptr;
					if (pair.second) {
						b_descr = td.get_building_descr(pair.first);
					} else {
						b_descr = td.get_immovable_descr(pair.first);
					}
					assert(b_descr);
					fw.unsigned_8(1);
					fw.string(b_descr->name());
					fw.string(pair.second ? "building" : "immovable");
				}
				fw.unsigned_8(0);
			}
			{
				bool is_stopped = false;
				if (upcast(ProductionSite const, productionsite, building)) {
					is_stopped = productionsite->is_stopped();
				}
				fw.unsigned_8(is_stopped);
			}

			Game& game = dynamic_cast<Game&>(egbase);

			if (upcast(ConstructionSite const, constructionsite, building)) {
				write_constructionsite(*constructionsite, fw, game, mos);
			} else if (upcast(DismantleSite const, dms, building)) {
				write_dismantlesite(*dms, fw, game, mos);
			} else if (upcast(MilitarySite const, militarysite, building)) {
				write_militarysite(*militarysite, fw, game, mos);
			} else if (upcast(Warehouse const, warehouse, building)) {
				write_warehouse(*warehouse, fw, game, mos);
			} else if (upcast(ProductionSite const, productionsite, building)) {
				if (upcast(TrainingSite const, trainingsite, productionsite)) {
					write_trainingsite(*trainingsite, fw, game, mos);
				} else {
					write_productionsite(*productionsite, fw, game, mos);
				}
			} else {
				NEVER_HERE();
				//  type of building is not one of (or derived from)
				//  {ConstructionSite, Warehouse, ProductionSite}
			}
			mos.mark_object_as_saved(*building);
		}
	}
	fw.write(fs, "binary/building_data");
}

void MapBuildingdataPacket::write_partially_finished_building(const PartiallyFinishedBuilding& pfb,
                                                              FileWrite& fw,
                                                              Game& game,
                                                              MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketPFBuilding);

	//  descriptions
	fw.string(pfb.building_->name());

	// builder request
	if (pfb.builder_request_) {
		fw.unsigned_8(1);
		pfb.builder_request_->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	// builder
	if (Worker const* builder = pfb.builder_.get(game)) {
		assert(mos.is_object_known(*builder));
		fw.unsigned_32(mos.get_object_file_index(*builder));
	} else {
		fw.unsigned_32(0);
	}

	uint16_t wares_size = pfb.consume_wares_.size();
	fw.unsigned_16(wares_size);
	for (uint16_t i = 0; i < wares_size; ++i) {
		pfb.consume_wares_[i]->write(fw, game, mos);
	}
	wares_size = pfb.dropout_wares_.size();
	fw.unsigned_16(wares_size);
	for (uint16_t i = 0; i < wares_size; ++i) {
		pfb.dropout_wares_[i]->write(fw, game, mos);
	}

	fw.unsigned_8(pfb.working_);
	pfb.work_steptime_.save(fw);
	fw.unsigned_32(pfb.work_completed_);
	fw.unsigned_32(pfb.work_steps_);
}

void MapBuildingdataPacket::write_constructionsite(const ConstructionSite& constructionsite,
                                                   FileWrite& fw,
                                                   Game& game,
                                                   MapObjectSaver& mos) {

	fw.unsigned_16(kCurrentPacketVersionConstructionsite);

	write_partially_finished_building(constructionsite, fw, game, mos);

	fw.signed_32(constructionsite.fetchfromflag_);

	fw.unsigned_32(constructionsite.info_.intermediates.size());
	for (const BuildingDescr* d : constructionsite.info_.intermediates) {
		fw.c_string(d->name().c_str());
	}

	assert(constructionsite.settings_);
	constructionsite.settings_->save(game, fw);

	fw.unsigned_32(constructionsite.additional_wares_.size());
	for (const auto& pair : constructionsite.additional_wares_) {
		fw.string(game.descriptions().get_ware_descr(pair.first)->name());
		fw.unsigned_32(pair.second);
	}
	fw.unsigned_32(constructionsite.additional_workers_.size());
	for (const Worker* w : constructionsite.additional_workers_) {
		fw.unsigned_32(mos.get_object_file_index(*w));
	}
}

void MapBuildingdataPacket::write_dismantlesite(const DismantleSite& dms,
                                                FileWrite& fw,
                                                Game& game,
                                                MapObjectSaver& mos) {

	fw.unsigned_16(kCurrentPacketVersionDismantlesite);

	write_partially_finished_building(dms, fw, game, mos);

	// Nothing to Do
}

void MapBuildingdataPacket::write_warehouse(const Warehouse& warehouse,
                                            FileWrite& fw,
                                            Game& game,
                                            MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionWarehouseAndExpedition);

	//  supply
	const TribeDescr& tribe = warehouse.owner().tribe();
	const WareList& wares = warehouse.supply_->get_wares();
	for (DescriptionIndex i = 0; i < wares.get_nrwareids(); ++i) {
		fw.unsigned_8(1);
		fw.string(tribe.get_ware_descr(i)->name());
		fw.unsigned_32(wares.stock(i));
		fw.unsigned_8(static_cast<uint8_t>(warehouse.get_ware_policy(i)));
	}
	fw.unsigned_8(0);
	const WareList& workers = warehouse.supply_->get_workers();
	for (DescriptionIndex i = 0; i < workers.get_nrwareids(); ++i) {
		fw.unsigned_8(1);
		fw.string(tribe.get_worker_descr(i)->name());
		fw.unsigned_32(workers.stock(i));
		fw.unsigned_8(static_cast<uint8_t>(warehouse.get_worker_policy(i)));
	}
	fw.unsigned_8(0);

	//  Incorporated workers, write sorted after file-serial.
	uint32_t nworkers = 0;
	for (const auto& cwt : warehouse.incorporated_workers_) {
		nworkers += cwt.second.size();
	}

	fw.unsigned_16(nworkers);
	using TWorkerMap = std::map<uint32_t, const Worker*>;
	TWorkerMap workermap;
	for (const auto& cwt : warehouse.incorporated_workers_) {
		for (OPtr<Worker> temp_worker : cwt.second) {
			const Worker& w = *temp_worker.get(game);
			assert(mos.is_object_known(w));
			workermap.insert(std::make_pair(mos.get_object_file_index(w), &w));
		}
	}

	for (const auto& temp_worker : workermap) {
		const Worker& obj = *temp_worker.second;
		assert(mos.is_object_known(obj));
		fw.unsigned_32(mos.get_object_file_index(obj));
	}

	{
		const std::vector<DescriptionIndex>& worker_types_without_cost =
		   tribe.worker_types_without_cost();
		assert(worker_types_without_cost.size() == warehouse.next_worker_without_cost_spawn_.size());
		for (uint8_t i = worker_types_without_cost.size(); i;) {
			const Time& next_spawn = warehouse.next_worker_without_cost_spawn_[--i];
			if (next_spawn.is_valid()) {
				fw.string(tribe.get_worker_descr(tribe.worker_types_without_cost().at(i))->name());
				next_spawn.save(fw);
			}
		}
	}
	fw.unsigned_8(0);  //  terminator for spawn times

	fw.unsigned_32(warehouse.planned_workers_.size());
	for (const Warehouse::PlannedWorkers& temp_worker : warehouse.planned_workers_) {
		fw.c_string(tribe.get_worker_descr(temp_worker.index)->name());
		fw.unsigned_32(temp_worker.amount);

		fw.unsigned_32(temp_worker.requests.size());

		for (Request* temp_request : temp_worker.requests) {
			temp_request->write(fw, game, mos);
		}
	}

	warehouse.next_stock_remove_act_.save(fw);

	if (warehouse.descr().get_isport()) {
		fw.unsigned_32(mos.get_object_file_index_or_zero(warehouse.portdock_));

		// Expedition specific stuff. See comment in loader.
		if (warehouse.portdock_->expedition_started()) {
			warehouse.portdock_->expedition_bootstrap()->save(fw, game, mos);
		}
	}
}

void MapBuildingdataPacket::write_militarysite(const MilitarySite& militarysite,
                                               FileWrite& fw,
                                               Game& game,
                                               MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionMilitarysite);

	if (militarysite.normal_soldier_request_) {
		fw.unsigned_8(1);
		militarysite.normal_soldier_request_->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	if (militarysite.upgrade_soldier_request_) {
		fw.unsigned_8(1);
		militarysite.upgrade_soldier_request_->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	fw.unsigned_8(militarysite.didconquer_);
	fw.unsigned_8(militarysite.capacity_);
	militarysite.nexthealtime_.save(fw);

	if (militarysite.normal_soldier_request_) {
		if (militarysite.upgrade_soldier_request_) {
			throw GameDataError(
			   "Internal error in a MilitarySite -- cannot continue. Use previous autosave.");
		}
	}
	fw.unsigned_16(militarysite.soldier_upgrade_requirements_.get_min());
	fw.unsigned_16(militarysite.soldier_upgrade_requirements_.get_max());
	fw.unsigned_8(static_cast<uint8_t>(militarysite.soldier_preference_));
	militarysite.next_swap_soldiers_time_.save(fw);
	fw.unsigned_8(militarysite.soldier_upgrade_try_ ? 1 : 0);
	fw.unsigned_8(militarysite.doing_upgrade_request_ ? 1 : 0);

	fw.unsigned_8(militarysite.attack_target_.allow_conquer_.size());
	for (const auto& pair : militarysite.attack_target_.allow_conquer_) {
		fw.unsigned_8(pair.first);
		fw.unsigned_8(pair.second ? 1 : 0);
	}
}

void MapBuildingdataPacket::write_productionsite(const ProductionSite& productionsite,
                                                 FileWrite& fw,
                                                 Game& game,
                                                 MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionProductionsite);

	uint32_t const nr_working_positions = productionsite.descr().nr_working_positions();
	const auto& begin = productionsite.working_positions_.begin();
	const auto& end = std::next(begin, nr_working_positions);
	uint32_t nr_workers = 0;
	for (auto i = begin; i != end; ++i) {
		nr_workers += i->worker.get(game) ? 1 : 0;
	}

	//  worker requests
	fw.unsigned_16(nr_working_positions - nr_workers);
	for (auto i = begin; i != end; ++i) {
		if (Request const* const r = i->worker_request) {
			r->write(fw, game, mos);
		}
	}

	//  workers
	fw.unsigned_16(nr_workers);
	for (auto i = begin; i != end; ++i) {
		if (Worker const* const w = i->worker.get(game)) {
			assert(!i->worker_request);
			assert(mos.is_object_known(*w));
			fw.unsigned_32(mos.get_object_file_index(*w));
		}
	}

	fw.signed_32(productionsite.fetchfromflag_);

	//  skipped programs
	assert(productionsite.failed_skipped_programs_.size() <= std::numeric_limits<uint8_t>::max());
	fw.unsigned_8(productionsite.failed_skipped_programs_.size());

	for (const auto& temp_program : productionsite.failed_skipped_programs_) {
		fw.string(temp_program.first);
		temp_program.second.save(fw);
	}

	//  state
	uint16_t const program_size = productionsite.stack_.size();
	fw.unsigned_16(program_size);
	for (uint16_t i = 0; i < program_size; ++i) {
		fw.string(productionsite.stack_[i].program->name());
		fw.signed_32(productionsite.stack_[i].ip);
		// TODO(GunChleoc): If we ever change this packet, we can have an uint8 here.
		fw.signed_32(static_cast<int>(productionsite.stack_[i].phase));
		fw.unsigned_32(productionsite.stack_[i].flags);
		fw.unsigned_32(mos.get_object_file_index_or_zero(productionsite.stack_[i].objvar.get(game)));
		write_coords_32(&fw, productionsite.stack_[i].coord);
	}
	fw.unsigned_8(productionsite.program_timer_);
	productionsite.program_time_.save(fw);

	// Get number of ware queues. Not very pretty but avoids changing the save file format
	uint16_t input_ware_queues_size = 0;
	for (InputQueue* iq : productionsite.inputqueues()) {
		if (iq->get_type() == wwWARE) {
			input_ware_queues_size++;
		}
	}
	// Write count of ware queues
	fw.unsigned_16(input_ware_queues_size);
	for (InputQueue* iq : productionsite.inputqueues()) {
		if (iq->get_type() == wwWARE) {
			iq->write(fw, game, mos);
		}
	}

	// Same for worker queues
	fw.unsigned_16(productionsite.input_queues_.size() - input_ware_queues_size);
	for (InputQueue* iq : productionsite.inputqueues()) {
		if (iq->get_type() == wwWORKER) {
			iq->write(fw, game, mos);
		}
	}

	fw.unsigned_32(productionsite.actual_percent_);
	fw.string(productionsite.statistics_string_on_changed_statistics_);
	fw.string(productionsite.production_result());

	if (productionsite.main_worker_ < 0) {
		fw.unsigned_8(0);
	} else {
		fw.unsigned_8(1);
		fw.unsigned_32(mos.get_object_file_index(
		   *productionsite.working_positions_.at(productionsite.main_worker_).worker.get(game)));
	}
}

/*
 * write for trainingsite
 */
void MapBuildingdataPacket::write_trainingsite(const TrainingSite& trainingsite,
                                               FileWrite& fw,
                                               Game& game,
                                               MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionTrainingsite);

	write_productionsite(trainingsite, fw, game, mos);

	//  requests

	if (trainingsite.soldier_request_) {
		fw.unsigned_8(1);
		trainingsite.soldier_request_->write(fw, game, mos);
	} else {
		fw.unsigned_8(0);
	}

	fw.unsigned_8(trainingsite.capacity_);
	fw.unsigned_8(trainingsite.build_heroes_);

	// upgrades
	fw.unsigned_8(trainingsite.upgrades_.size());
	for (const TrainingSite::Upgrade& upgrade : trainingsite.upgrades_) {
		fw.unsigned_8(static_cast<uint8_t>(upgrade.attribute));
		fw.unsigned_8(upgrade.prio);
		fw.unsigned_8(upgrade.credit);
		fw.signed_32(upgrade.lastattempt);
		fw.signed_8(upgrade.lastsuccess);
	}
	if (255 < trainingsite.training_failure_count_.size()) {
		log_warn_time(game.get_gametime(),
		              "Save TrainingSite: Failure counter has ridiculously many entries! (%u)\n",
		              static_cast<uint16_t>(trainingsite.training_failure_count_.size()));
	}
	fw.unsigned_16(static_cast<uint16_t>(trainingsite.training_failure_count_.size()));
	for (const auto& fail_and_presence : trainingsite.training_failure_count_) {
		fw.unsigned_8(static_cast<uint8_t>(fail_and_presence.first.first));
		fw.unsigned_16(fail_and_presence.first.second);
		fw.unsigned_16(fail_and_presence.second.first);
		fw.unsigned_8(fail_and_presence.second.second);
	}
	fw.unsigned_8(trainingsite.highest_trainee_level_seen_);
	fw.unsigned_8(trainingsite.latest_trainee_kickout_level_);
	fw.unsigned_8(trainingsite.trainee_general_lower_bound_);
	uint8_t somebits = 0;
	if (trainingsite.latest_trainee_was_kickout_) {
		somebits++;
	}
	if (trainingsite.requesting_weak_trainees_) {
		somebits += 2;
	}
	if (trainingsite.repeated_layoff_inc_) {
		somebits += 4;
	}
	if (trainingsite.recent_capacity_increase_) {
		somebits += 8;
	}
	fw.unsigned_8(somebits);
	fw.unsigned_8(trainingsite.repeated_layoff_ctr_);
	trainingsite.request_open_since_.save(fw);

	fw.unsigned_8(static_cast<uint8_t>(trainingsite.checked_soldier_training_.attribute));
	fw.unsigned_8(trainingsite.checked_soldier_training_.level);

	// DONE
}
}  // namespace Widelands
