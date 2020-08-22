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

#include "editor/tools/scenario_worker_tool.h"

#include "editor/editorinteractive.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry.h"

int32_t ScenarioPlaceWorkerTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                   EditorInteractive& eia,
                                                   EditorActionArgs* args,
                                                   Widelands::Map* map) {
	if (args->new_owner < 1 || args->new_owner > map->get_nrplayers() ||
	    args->worker_types.empty()) {
		return 0;
	}
	const size_t nr_items = args->worker_types.size();
	Widelands::EditorGameBase& egbase = eia.egbase();
	args->infrastructure_placed = 0;
	if (args->random_index == kRandomIndexNotSet) {
		args->random_index = std::rand() % nr_items;  // NOLINT
	}
	Widelands::Player* player = egbase.get_player(args->new_owner);
	const Widelands::WorkerDescr* wd = args->worker_types[args->random_index];
	Widelands::Bob* bob = nullptr;
	if (wd) {
		Widelands::Worker& worker = wd->create(egbase, player, nullptr, center.node);
		if (wd->becomes() != Widelands::INVALID_INDEX) {
			worker.set_current_experience(std::max<int32_t>(
			   0, std::min<int32_t>(args->experience, wd->get_needed_experience() - 1)));
		}
		if (args->carried_ware != Widelands::INVALID_INDEX) {
			Widelands::WareInstance* wi = new Widelands::WareInstance(
			   args->carried_ware, egbase.tribes().get_ware_descr(args->carried_ware));
			wi->init(egbase);
			worker.set_carried_ware(egbase, wi);
		}
		bob = &worker;
	} else if ((*map)[center.node].nodecaps() & Widelands::MOVECAPS_SWIM) {
		bob = &egbase.create_ship(center.node, player->tribe().ship(), player);
	}
	args->infrastructure_placed = bob->serial();  // NOLINT
	return 1;
}

EditorActionArgs ScenarioPlaceWorkerTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = player_;
	a.random_index = kRandomIndexNotSet;
	a.experience = experience_;
	a.carried_ware = carried_ware_;
	for (const Widelands::WorkerDescr* d : descr_) {
		a.worker_types.push_back(d);
	}
	return a;
}

int32_t ScenarioDeleteWorkerTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorInteractive& eia,
                                                    EditorActionArgs* args,
                                                    Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		std::list<Widelands::Worker*> workers_to_delete;
		std::list<Widelands::Ship*> ships_to_delete;
		for (Widelands::Bob* b = mr.location().field->get_first_bob(); b; b = b->get_next_bob()) {
			if (upcast(Widelands::Worker, w, b)) {
				if (!w->get_location(egbase)) {
					workers_to_delete.push_back(w);
				}
			} else if (upcast(Widelands::Ship, s, b)) {
				ships_to_delete.push_back(s);
			}
		}
		std::list<EditorActionArgs::WorkerHistory> list_w;
		std::list<std::pair<uint8_t, std::string>> list_s;
		for (Widelands::Worker* w : workers_to_delete) {
			list_w.push_back(EditorActionArgs::WorkerHistory{
			   &w->descr(), w->owner().player_number(),
			   w->needs_experience() ? w->get_current_experience() : 0u,
			   w->get_carried_ware(egbase) ?
			      egbase.tribes().safe_ware_index(w->get_carried_ware(egbase)->descr().name()) :
			      Widelands::INVALID_INDEX});
			w->remove(egbase);
		}
		for (Widelands::Ship* s : ships_to_delete) {
			list_s.push_back(std::make_pair(s->owner().player_number(), s->get_shipname()));
			s->remove(egbase);
		}
		args->workers_deleted.push_back(list_w);
		args->ships_deleted.push_back(list_s);
	} while (mr.advance(*map));
	return mr.radius();
}

EditorActionArgs ScenarioDeleteWorkerTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
