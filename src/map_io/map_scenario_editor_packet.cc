/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "map_io/map_scenario_editor_packet.h"

#include "economy/expedition_bootstrap.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "economy/workers_queue.h"
#include "editor/editorinteractive.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapScenarioEditorPacket::read(FileSystem& fs, EditorGameBase& egbase, bool, MapObjectLoader&) {
	upcast(EditorInteractive, eia, egbase.get_ibase());
	assert(eia);
	eia->unfinalize();

	FileRead fr;
	if (!fr.try_open(fs, "binary/scenario")) {
		// Not a scenario
		return;
	}

	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			if (fr.unsigned_8()) {
				// First the scripting data

				eia->finalized_ = true;
				eia->set_display_flag(InteractiveBase::dfShowCensus, fr.unsigned_8());
				eia->init_allowed_buildings_windows_registries();
				eia->new_scripting_saver();

				// The ScriptingLoader constructor will immediately load all ScriptingObjects
				// (triggering both loading phases one after the other)
				ScriptingLoader loader(fr, eia->scripting_saver());

				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->functions_.push_back(&loader.get<FS_LaunchCoroutine>(fr.unsigned_32()));
				}
				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->variables_.push_back(&loader.get<FS_LocalVarDeclOrAssign>(fr.unsigned_32()));
				}
				for (uint32_t n = fr.unsigned_32(); n; --n) {
					eia->includes_.push_back(fr.c_string());
				}

				eia->rebuild_scenario_tool_menu();
				eia->rebuild_showhide_menu();

				// Map objects
				const Widelands::Map& map = egbase.map();
				for (size_t map_index = map.max_index(); map_index; --map_index) {
					const Widelands::FCoords coords = map.get_fcoords(map.coords(map_index - 1));
					if (Widelands::PlayerNumber n = fr.unsigned_8()) {
						egbase.conquer_area_no_building(
						   PlayerArea<Area<FCoords>>(n, Area<FCoords>(coords, 0)));
					}
					if (const uint8_t t = fr.unsigned_8()) {
						Widelands::Field& field = map[map_index - 1];
						const Widelands::MapObjectType type = static_cast<Widelands::MapObjectType>(t);
						switch (type) {
						case Widelands::MapObjectType::FLAG: {
							Widelands::Flag& flag =
							   egbase.get_player(field.get_owned_by())->force_flag(coords);
							for (size_t n = fr.unsigned_32(); n; --n) {
								const Widelands::DescriptionIndex ware = fr.unsigned_32();
								Widelands::WareInstance& wi =
								   *new Widelands::WareInstance(ware, egbase.tribes().get_ware_descr(ware));
								wi.init(egbase);
								flag.add_ware(egbase, wi);
							}
						} break;
						case Widelands::MapObjectType::ROAD: {
							Path path;
							path.load(fr, map);
							Widelands::CoordPath cp(egbase.map(), path);
							Widelands::Road& r = egbase.get_player(field.get_owned_by())->force_road(path);
							r.busy_ = fr.unsigned_8();
							uint8_t i = 0;
							for (const auto& s : r.carrier_slots_) {
								if (fr.unsigned_8()) {
									Widelands::Carrier& c = dynamic_cast<Widelands::Carrier&>(
									   egbase.tribes()
									      .get_worker_descr(s.second_carrier ? r.owner().tribe().carrier2() :
									                                           r.owner().tribe().carrier())
									      ->create(egbase, r.get_owner(), &r,
									               cp.get_coords()[r.get_idle_index()]));
									r.assign_carrier(c, i);
								}
								++i;
							}
						} break;
						case Widelands::MapObjectType::WATERWAY: {
							Path path;
							path.load(fr, map);
							Widelands::Waterway& ww =
							   egbase.get_player(field.get_owned_by())->force_waterway(path);
							if (fr.unsigned_8()) {
								Widelands::Ferry& ferry =
								   dynamic_cast<Widelands::Ferry&>(egbase.create_worker(
								      coords, ww.owner().tribe().ferry(), ww.get_owner()));
								ww.assign_carrier(ferry, 0);
							}
						} break;
						case Widelands::MapObjectType::MILITARYSITE: {
							const Widelands::DescriptionIndex idx = fr.unsigned_32();
							Widelands::FormerBuildings former;
							former.push_back(std::make_pair(idx, ""));
							Widelands::MilitarySite& ms = dynamic_cast<Widelands::MilitarySite&>(
							   egbase.get_player(field.get_owned_by())->force_building(coords, former));
							ms.set_soldier_preference(fr.unsigned_8() ?
							                             Widelands::SoldierPreference::kHeroes :
							                             Widelands::SoldierPreference::kRookies);
							ms.mutable_soldier_control()->set_soldier_capacity(fr.unsigned_32());
							for (size_t n = fr.unsigned_32(); n; --n) {
								Widelands::Soldier& s = dynamic_cast<Widelands::Soldier&>(
								   egbase.tribes()
								      .get_worker_descr(ms.owner().tribe().soldier())
								      ->create(egbase, ms.get_owner(), &ms, ms.get_position()));
								s.set_health_level(fr.unsigned_32());
								s.set_attack_level(fr.unsigned_32());
								s.set_defense_level(fr.unsigned_32());
								s.set_evade_level(fr.unsigned_32());
								s.set_current_health(fr.unsigned_32());
							}
						} break;
						case Widelands::MapObjectType::WAREHOUSE: {
							const Widelands::DescriptionIndex idx = fr.unsigned_32();
							Widelands::FormerBuildings former;
							former.push_back(std::make_pair(idx, ""));
							Widelands::Warehouse& wh = dynamic_cast<Widelands::Warehouse&>(
							   egbase.get_player(field.get_owned_by())->force_building(coords, former));
							for (Widelands::DescriptionIndex di : wh.owner().tribe().wares()) {
								wh.insert_wares(di, fr.unsigned_32());
								wh.set_ware_policy(
								   di, static_cast<Widelands::StockPolicy>(fr.unsigned_8()));
							}
							for (Widelands::DescriptionIndex di : wh.owner().tribe().workers()) {
								wh.insert_workers(di, fr.unsigned_32());
								wh.set_worker_policy(
								   di, static_cast<Widelands::StockPolicy>(fr.unsigned_8()));
							}
							if (fr.unsigned_8()) {
								wh.get_portdock()->start_expedition();
								for (size_t n = fr.unsigned_32(); n; --n) {
									const bool is_worker = fr.unsigned_8();
									const Widelands::DescriptionIndex di = fr.unsigned_32();
									const bool is_additional = fr.unsigned_8();
									wh.get_portdock()
									   ->expedition_bootstrap()
									   ->inputqueue(di, is_worker ? Widelands::wwWORKER : Widelands::wwWARE,
									                is_additional)
									   .set_filled(fr.unsigned_32());
								}
							}
						} break;
						case Widelands::MapObjectType::CONSTRUCTIONSITE:
						case Widelands::MapObjectType::DISMANTLESITE: {
							Widelands::PartiallyFinishedBuilding* pfb = nullptr;
							const Widelands::DescriptionIndex idx = fr.unsigned_32();
							if (type == Widelands::MapObjectType::CONSTRUCTIONSITE) {
								pfb = &dynamic_cast<Widelands::PartiallyFinishedBuilding&>(
								   egbase.get_player(field.get_owned_by())->force_csite(coords, idx));
							} else {
								Widelands::FormerBuildings former;
								former.push_back(std::make_pair(idx, ""));
								Widelands::Building& temp_bld =
								   egbase.get_player(field.get_owned_by())->force_building(coords, former);
								temp_bld.get_owner()->dismantle_building(&temp_bld, true);
								pfb = dynamic_cast<Widelands::PartiallyFinishedBuilding*>(
								   coords.field->get_immovable());
							}
							assert(pfb);
							for (size_t n = fr.unsigned_32(); n; --n) {
								const Widelands::DescriptionIndex di = fr.unsigned_32();
								Widelands::InputQueue& q = pfb->inputqueue(di, Widelands::wwWARE);
								q.set_max_fill(fr.unsigned_32());
								q.set_filled(fr.unsigned_32());
								pfb->set_priority(Widelands::wwWARE, di, fr.signed_32());
							}
							for (Widelands::WaresQueue* q : pfb->dropout_wares_) {
								q->set_max_size(0);
							}
							for (size_t n = fr.unsigned_32(); n; --n) {
								const Widelands::DescriptionIndex di = fr.unsigned_32();
								const Quantity f = fr.unsigned_32();
								try {
									pfb->inputqueue(di, Widelands::wwWARE).set_filled(f);
								} catch (const WException&) {
									// building description must have been changed – ignore
								}
							}
							if (fr.unsigned_8()) {
								pfb->builder_request_ = nullptr;
								pfb->builder_ =
								   &egbase.tribes()
								       .get_worker_descr(pfb->owner().tribe().builder())
								       ->create(egbase, pfb->get_owner(), pfb, pfb->get_position());
							} else {
								pfb->builder_ = nullptr;
								pfb->request_builder(egbase);
							}
							pfb->work_completed_ = fr.unsigned_32();
						} break;
						case Widelands::MapObjectType::TRAININGSITE: {
							const Widelands::DescriptionIndex idx = fr.unsigned_32();
							Widelands::FormerBuildings former;
							former.push_back(std::make_pair(idx, ""));
							Widelands::TrainingSite& ts = dynamic_cast<Widelands::TrainingSite&>(
							   egbase.get_player(field.get_owned_by())->force_building(coords, former));
							ts.mutable_soldier_control()->set_soldier_capacity(fr.unsigned_32());
							for (size_t n = fr.unsigned_32(); n; --n) {
								Widelands::Soldier& s = dynamic_cast<Widelands::Soldier&>(
								   egbase.tribes()
								      .get_worker_descr(ts.owner().tribe().soldier())
								      ->create(egbase, ts.get_owner(), &ts, ts.get_position()));
								s.set_health_level(fr.unsigned_32());
								s.set_attack_level(fr.unsigned_32());
								s.set_defense_level(fr.unsigned_32());
								s.set_evade_level(fr.unsigned_32());
								s.set_current_health(fr.unsigned_32());
							}
						}
							FALLS_THROUGH;
						case Widelands::MapObjectType::PRODUCTIONSITE: {
							Widelands::ProductionSite* ps = nullptr;
							if (type == Widelands::MapObjectType::TRAININGSITE) {
								ps =
								   dynamic_cast<Widelands::ProductionSite*>(coords.field->get_immovable());
							} else {
								const Widelands::DescriptionIndex i = fr.unsigned_32();
								Widelands::FormerBuildings f;
								f.push_back(std::make_pair(i, ""));
								ps = &dynamic_cast<Widelands::ProductionSite&>(
								   egbase.get_player(field.get_owned_by())->force_building(coords, f));
							}
							assert(ps);
							ps->set_stopped(fr.unsigned_8());
							for (size_t n = fr.unsigned_32(); n; --n) {
								const Widelands::WareWorker ww =
								   fr.unsigned_8() ? Widelands::wwWORKER : Widelands::wwWARE;
								const Widelands::DescriptionIndex di = fr.unsigned_32();
								Widelands::InputQueue& q = ps->inputqueue(di, ww);
								q.set_max_fill(fr.unsigned_32());
								q.set_filled(fr.unsigned_32());
								ps->set_priority(ww, di, fr.signed_32());
							}
							for (uint32_t n = ps->descr().nr_working_positions(); n; --n) {
								if (fr.unsigned_8()) {
#ifndef NDEBUG
									const bool success =
#endif
									   ps->warp_worker(egbase,
									                   *egbase.tribes().get_worker_descr(fr.unsigned_32()),
									                   n - 1);
									assert(success);
									ps->working_positions()[n - 1].worker->set_current_experience(
									   fr.signed_32());
								}
							}
						} break;
						default:
							assert(type < Widelands::MapObjectType::BUILDING);
							break;
						}  // switch (type)
					}     // if immovable
					for (size_t n = fr.unsigned_32(); n; --n) {
						const Widelands::PlayerNumber owner = fr.unsigned_8();
						const uint32_t worker = fr.unsigned_32();
						Widelands::Worker& w = egbase.tribes().get_worker_descr(worker)->create(
						   egbase, egbase.get_player(owner), nullptr, coords);
						w.set_current_experience(fr.signed_32());
						const Widelands::DescriptionIndex ware = fr.unsigned_32();
						if (ware != Widelands::INVALID_INDEX) {
							Widelands::WareInstance* wi =
							   new Widelands::WareInstance(ware, egbase.tribes().get_ware_descr(ware));
							wi->init(egbase);
							w.set_carried_ware(egbase, wi);
						}
						if (upcast(Widelands::Soldier, s, &w)) {
							s->set_health_level(fr.unsigned_32());
							s->set_attack_level(fr.unsigned_32());
							s->set_defense_level(fr.unsigned_32());
							s->set_evade_level(fr.unsigned_32());
							s->set_current_health(fr.unsigned_32());
						}
					}  // workers

					for (size_t n = fr.unsigned_32(); n; --n) {
						const Widelands::PlayerNumber owner = fr.unsigned_8();
						Widelands::Ship& ship = dynamic_cast<Widelands::Ship&>(egbase.create_ship(
						   coords, egbase.player(owner).tribe().ship(), egbase.get_player(owner)));
						ship.set_shipname(fr.c_string());
						for (uint32_t nn = fr.unsigned_32(); nn; --nn) {
							const bool is_worker = fr.unsigned_8();
							const Widelands::DescriptionIndex idx = fr.unsigned_32();
							if (is_worker) {
								Widelands::Worker& w = egbase.tribes().get_worker_descr(idx)->create(
								   egbase, ship.get_owner(), nullptr, ship.get_position());
								ship.items_.push_back(Widelands::ShippingItem(w));
							} else {
								Widelands::WareInstance& w =
								   *new Widelands::WareInstance(idx, egbase.tribes().get_ware_descr(idx));
								w.init(egbase);
								w.set_location(egbase, &ship);
								ship.items_.push_back(Widelands::ShippingItem(w));
							}
						}
					}  // ships
				}     // map index iterating

				// Economies
				for (size_t n = fr.unsigned_32(); n; --n) {
					Widelands::Flag& flag = dynamic_cast<Widelands::Flag&>(
					   *map[map.coords(fr.unsigned_32())].get_immovable());
					const bool is_worker = fr.unsigned_8();
					Widelands::Economy& e =
					   *flag.get_economy(is_worker ? Widelands::wwWORKER : Widelands::wwWARE);
					for (Widelands::DescriptionIndex di :
					     (is_worker ? e.owner().tribe().workers() : e.owner().tribe().wares())) {
						e.set_target_quantity(
						   is_worker ? Widelands::wwWORKER : Widelands::wwWARE, di, fr.unsigned_32(), 0);
					}
				}

				// Player relations
				const uint8_t nrplayers = map.get_nrplayers();
				for (Widelands::PlayerNumber p = 1; p <= nrplayers; ++p) {
					Widelands::Player& player = *egbase.get_player(p);
					player.set_team_number(fr.unsigned_8());
					for (Widelands::PlayerNumber p2 = 1; p2 <= nrplayers; ++p2) {
						if (p2 != p) {
							player.set_attack_forbidden(p2, fr.unsigned_8());
						}
					}
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapScenarioEditorPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("scenario editor data: %s", e.what());
	}
}

void MapScenarioEditorPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	upcast(EditorInteractive, eia, egbase.get_ibase());
	if (!eia) {
		throw GameDataError("MapScenarioEditorPacket may be saved only inside the editor");
	}
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	if (!eia->finalized_) {
		fw.unsigned_8(0);
	} else {
		fw.unsigned_8(1);

		// Scripting

		fw.unsigned_8(eia->get_display_flag(InteractiveBase::dfShowCensus) ? 1 : 0);

		eia->scripting_saver().delete_unused(*eia);
		eia->scripting_saver().selftest();
		eia->scripting_saver().save(fw);

		fw.unsigned_32(eia->functions_.size());
		for (const auto& f : eia->functions_) {
			fw.unsigned_32(f->serial());
		}

		fw.unsigned_32(eia->variables_.size());
		for (const auto& v : eia->variables_) {
			fw.unsigned_32(v->serial());
		}

		fw.unsigned_32(eia->includes_.size());
		for (const std::string& s : eia->includes_) {
			fw.c_string(s.c_str());
		}

		// Map objects.
		// This is similar to the MapXXXPackets (only much less detailed) but we do it separately to
		// ensure map compatibility even when we break savegame compatibility. In the far far future
		// it may perhaps be no longer possible to edit scenarios created now with the editor, but it
		// will still be possible to play them because we never break basic map compatibility.
		std::set<Widelands::Serial> saved_mos;
		std::set<const Widelands::Economy*> economies_to_save;
		const Widelands::Map& map = egbase.map();
		for (size_t map_index = map.max_index(); map_index; --map_index) {
			const Widelands::Field& f = map[map_index - 1];
			fw.unsigned_8(f.get_owned_by());
			if (!f.get_immovable()) {
				fw.unsigned_8(0);
			} else {
				bool skip = false;
				if (saved_mos.count(f.get_immovable()->serial())) {
					fw.unsigned_8(0);
					skip = true;
				} else {
					upcast(const Widelands::Building, bld, f.get_immovable());
					if (bld && bld->get_position() != map.coords(map_index - 1)) {
						fw.unsigned_8(0);
						skip = true;
					}
				}
				if (!skip) {
					saved_mos.insert(f.get_immovable()->serial());
					const Widelands::MapObjectType type = f.get_immovable()->descr().type();
					fw.unsigned_8(static_cast<uint8_t>(type));
					switch (type) {
					case Widelands::MapObjectType::FLAG: {
						const Widelands::Flag& flag =
						   dynamic_cast<const Widelands::Flag&>(*f.get_immovable());
						fw.unsigned_32(flag.current_wares());
						for (const WareInstance* w : flag.get_wares()) {
							fw.unsigned_32(w->descr_index());
						}
						if (!economies_to_save.count(flag.get_economy(Widelands::wwWARE))) {
							economies_to_save.insert(flag.get_economy(Widelands::wwWARE));
						}
						if (!economies_to_save.count(flag.get_economy(Widelands::wwWORKER))) {
							economies_to_save.insert(flag.get_economy(Widelands::wwWORKER));
						}
					} break;
					case Widelands::MapObjectType::ROAD: {
						const Widelands::Road& road =
						   dynamic_cast<const Widelands::Road&>(*f.get_immovable());
						road.get_path().save(fw);
						fw.unsigned_8(road.busy_ ? 1 : 0);
						for (const auto& s : road.carrier_slots_) {
							fw.unsigned_8(s.carrier.is_set() ? 1 : 0);
						}
					} break;
					case Widelands::MapObjectType::WATERWAY: {
						const Widelands::Waterway& ww =
						   dynamic_cast<const Widelands::Waterway&>(*f.get_immovable());
						ww.get_path().save(fw);
						fw.unsigned_8(ww.get_ferry() ? 1 : 0);
					} break;
					case Widelands::MapObjectType::CONSTRUCTIONSITE:
					case Widelands::MapObjectType::DISMANTLESITE: {
						const Widelands::PartiallyFinishedBuilding& pfb =
						   dynamic_cast<const Widelands::PartiallyFinishedBuilding&>(*f.get_immovable());
						fw.unsigned_32(egbase.tribes().safe_building_index(pfb.building().name()));
						size_t n = pfb.nr_consume_waresqueues();
						fw.unsigned_32(n);
						for (; n; --n) {
							const WaresQueue& q = *pfb.get_consume_waresqueue(n - 1);
							assert(q.get_type() == Widelands::wwWARE);
							fw.unsigned_32(q.get_index());
							fw.unsigned_32(q.get_max_fill());
							fw.unsigned_32(q.get_filled());
							fw.signed_32(pfb.get_priority(Widelands::wwWARE, q.get_index(), false));
						}
						n = pfb.nr_dropout_waresqueues();
						fw.unsigned_32(n);
						for (; n; --n) {
							const WaresQueue& q = *pfb.get_dropout_waresqueue(n - 1);
							assert(q.get_type() == Widelands::wwWARE);
							fw.unsigned_32(q.get_index());
							fw.unsigned_32(q.get_filled());
						}
						fw.unsigned_8(pfb.builder_request_ ? 0 : 1);  // whether the site has a builder
						fw.unsigned_32(pfb.work_completed_);
					} break;
					case Widelands::MapObjectType::WAREHOUSE: {
						const Widelands::Warehouse& wh =
						   dynamic_cast<const Widelands::Warehouse&>(*f.get_immovable());
						fw.unsigned_32(egbase.tribes().safe_building_index(wh.descr().name()));
						for (Widelands::DescriptionIndex di : wh.owner().tribe().wares()) {
							fw.unsigned_32(wh.get_wares().stock(di));
							fw.unsigned_8(static_cast<uint8_t>(wh.get_ware_policy(di)));
						}
						for (Widelands::DescriptionIndex di : wh.owner().tribe().workers()) {
							fw.unsigned_32(wh.get_workers().stock(di));
							fw.unsigned_8(static_cast<uint8_t>(wh.get_worker_policy(di)));
						}
						if (wh.get_portdock() && wh.get_portdock()->expedition_bootstrap()) {
							fw.unsigned_8(1);
							const std::vector<Widelands::InputQueue*> qs =
							   wh.get_portdock()->expedition_bootstrap()->queues(true);
							fw.unsigned_32(qs.size());
							for (const Widelands::InputQueue* q : qs) {
								fw.unsigned_8(q->get_type() == Widelands::wwWARE ? 0 : 1);
								fw.unsigned_32(q->get_index());
								fw.unsigned_32(q->get_filled());
								fw.unsigned_8(wh.get_portdock()->expedition_bootstrap()->is_additional(*q));
							}
						} else {
							fw.unsigned_8(0);
						}
					} break;
					case Widelands::MapObjectType::MILITARYSITE: {
						const Widelands::MilitarySite& ms =
						   dynamic_cast<const Widelands::MilitarySite&>(*f.get_immovable());
						fw.unsigned_32(egbase.tribes().safe_building_index(ms.descr().name()));
						fw.unsigned_8(
						   ms.get_soldier_preference() == Widelands::SoldierPreference::kRookies ? 0 : 1);
						fw.unsigned_32(ms.soldier_control()->soldier_capacity());
						const std::vector<Widelands::Soldier*> ss =
						   ms.soldier_control()->stationed_soldiers();
						fw.unsigned_32(ss.size());
						for (const Widelands::Soldier* s : ss) {
							fw.unsigned_32(s->get_health_level());
							fw.unsigned_32(s->get_attack_level());
							fw.unsigned_32(s->get_defense_level());
							fw.unsigned_32(s->get_evade_level());
							fw.unsigned_32(s->get_current_health());
						}
					} break;
					case Widelands::MapObjectType::TRAININGSITE: {
						const Widelands::TrainingSite& ts =
						   dynamic_cast<const Widelands::TrainingSite&>(*f.get_immovable());
						fw.unsigned_32(egbase.tribes().safe_building_index(ts.descr().name()));
						fw.unsigned_32(ts.soldier_control()->soldier_capacity());
						const std::vector<Widelands::Soldier*> ss =
						   ts.soldier_control()->stationed_soldiers();
						fw.unsigned_32(ss.size());
						for (const Widelands::Soldier* s : ss) {
							fw.unsigned_32(s->get_health_level());
							fw.unsigned_32(s->get_attack_level());
							fw.unsigned_32(s->get_defense_level());
							fw.unsigned_32(s->get_evade_level());
							fw.unsigned_32(s->get_current_health());
						}
					}
						FALLS_THROUGH;
					case Widelands::MapObjectType::PRODUCTIONSITE: {
						const Widelands::ProductionSite& ps =  // NOLINT
						   dynamic_cast<const Widelands::ProductionSite&>(*f.get_immovable());
						if (type != Widelands::MapObjectType::TRAININGSITE) {
							fw.unsigned_32(egbase.tribes().safe_building_index(ps.descr().name()));
						}
						fw.unsigned_8(ps.is_stopped() ? 1 : 0);
						const std::vector<Widelands::InputQueue*>& qs = ps.inputqueues();
						fw.unsigned_32(qs.size());
						for (const Widelands::InputQueue* q : qs) {
							fw.unsigned_8(q->get_type() == Widelands::wwWARE ? 0 : 1);
							fw.unsigned_32(q->get_index());
							fw.unsigned_32(q->get_max_fill());
							fw.unsigned_32(q->get_filled());
							fw.signed_32(ps.get_priority(Widelands::wwWARE, q->get_index(), false));
						}
						for (uint32_t n = ps.descr().nr_working_positions(); n; --n) {
							const Widelands::ProductionSite::WorkingPosition& w =
							   ps.working_positions()[n - 1];
							if (w.worker) {
								fw.unsigned_8(1);
								fw.unsigned_32(w.worker->descr().worker_index());
								fw.signed_32(w.worker->get_current_experience());
							} else {
								fw.unsigned_8(0);
							}
						}
					} break;
					default:
						assert(type < Widelands::MapObjectType::BUILDING);
						break;
					}  // switch (type)
				}     // if (!skip)
			}        // else (if (!f.get_immovable()))

			std::vector<const Widelands::Worker*> workers;
			std::vector<const Widelands::Ship*> ships;
			for (const Widelands::Bob* bob = f.get_first_bob(); bob; bob = bob->get_next_bob()) {
				if (upcast(const Widelands::Worker, w, bob)) {
					if (!w->get_location(egbase)) {  // not workers in buildings etc
						workers.push_back(w);
					}
				} else if (upcast(const Widelands::Ship, s, bob)) {
					ships.push_back(s);
				}
			}

			fw.unsigned_32(workers.size());
			for (const Widelands::Worker* w : workers) {
				fw.unsigned_8(w->owner().player_number());
				fw.unsigned_32(w->descr().worker_index());
				fw.signed_32(w->get_current_experience());
				const Widelands::WareInstance* ware = w->get_carried_ware(egbase);
				fw.unsigned_32(ware ? ware->descr_index() : Widelands::INVALID_INDEX);
				if (upcast(const Widelands::Soldier, s, w)) {
					fw.unsigned_32(s->get_health_level());
					fw.unsigned_32(s->get_attack_level());
					fw.unsigned_32(s->get_defense_level());
					fw.unsigned_32(s->get_evade_level());
					fw.unsigned_32(s->get_current_health());
				}
			}  // workers

			fw.unsigned_32(ships.size());
			for (const Widelands::Ship* s : ships) {
				fw.unsigned_8(s->owner().player_number());
				fw.c_string(s->get_shipname());
				const uint32_t n = s->get_nritems();
				fw.unsigned_32(n);
				for (uint32_t i = 0; i < n; ++i) {
					const Widelands::ShippingItem& si = s->get_item(i);
					Widelands::WareInstance* wa = nullptr;
					Widelands::Worker* wo = nullptr;
					si.get(egbase, &wa, &wo);
					assert((wa == nullptr) ^ (wo == nullptr));
					fw.unsigned_8(wo ? 1 : 0);
					fw.unsigned_32(wo ? wo->descr().worker_index() : wa->descr_index());
				}
			}  // ships
		}     // for (size_t n = map.max_index(); n; --n)

		// Economies

		fw.unsigned_32(economies_to_save.size());
		for (const Widelands::Economy* e : economies_to_save) {
			fw.unsigned_32(map.get_index(e->get_arbitrary_flag()->get_position()));
			fw.unsigned_8(e->type() == Widelands::wwWARE ? 0 : 1);
			for (Widelands::DescriptionIndex di :
			     (e->type() == Widelands::wwWARE ? e->owner().tribe().wares() :
			                                       e->owner().tribe().workers())) {
				fw.unsigned_32(e->target_quantity(di).permanent);
			}
		}

		// Player relations

		const uint8_t nrplayers = map.get_nrplayers();
		for (Widelands::PlayerNumber p = 1; p <= nrplayers; ++p) {
			const Widelands::Player& player = egbase.player(p);
			fw.unsigned_8(player.team_number());
			for (Widelands::PlayerNumber p2 = 1; p2 <= nrplayers; ++p2) {
				if (p2 != p) {
					fw.unsigned_8(player.is_attack_forbidden(p2) ? 1 : 0);
				}
			}
		}
	}

	fw.write(fs, "binary/scenario");
}
}  // namespace Widelands
