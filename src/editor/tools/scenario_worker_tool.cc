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
	if (args->random_index < 0) {
		args->random_index = std::rand() % nr_items;
	}
	Widelands::Player* player = egbase.get_player(args->new_owner);
	const Widelands::WorkerDescr* wd = args->worker_types[args->random_index];
	Widelands::Bob* bob = nullptr;
	if (wd) {
		Widelands::Worker& worker = wd->create(egbase, player, nullptr, center.node);
		if (wd->becomes() != Widelands::INVALID_INDEX) {
			worker.set_current_experience(
			   std::max<int32_t>(0, std::min<int32_t>(experience_, wd->get_needed_experience() - 1)));
		}
		bob = &worker;
	} else {
		bob = &egbase.create_ship(center.node, player->tribe().ship(), player);
		if (!shipname_.empty()) {
			dynamic_cast<Widelands::Ship*>(bob)->set_shipname(shipname_);
		}
	}
	args->infrastructure_placed = bob->serial();
	return 1;
}

int32_t ScenarioPlaceWorkerTool::handle_undo_impl(const Widelands::NodeAndTriangle<>&,
                                                  EditorInteractive& eia,
                                                  EditorActionArgs* args,
                                                  Widelands::Map*) {
	eia.egbase().objects().get_object(args->infrastructure_placed)->remove(eia.egbase());
	return 1;
}

EditorActionArgs ScenarioPlaceWorkerTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = player_;
	a.random_index = -1;
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
				workers_to_delete.push_back(w);
			} else if (upcast(Widelands::Ship, s, b)) {
				ships_to_delete.push_back(s);
			}
		}
		std::list<std::tuple<const Widelands::WorkerDescr*, uint8_t, uint32_t>> list_w;
		std::list<std::pair<uint8_t, std::string>> list_s;
		for (Widelands::Worker* w : workers_to_delete) {
			list_w.push_back(
			   std::make_tuple(&w->descr(), w->owner().player_number(),
			                   w->needs_experience() ? w->get_current_experience() : 0u));
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

int32_t ScenarioDeleteWorkerTool::handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
                                                   EditorInteractive& eia,
                                                   EditorActionArgs* args,
                                                   Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		for (const auto& tuple : *args->workers_deleted.begin()) {
			Widelands::Player* player = egbase.get_player(std::get<1>(tuple));
			Widelands::Worker& w = std::get<0>(tuple)->create(egbase, player, nullptr, mr.location());
			if (const uint32_t xp = std::get<2>(tuple)) {
				w.set_current_experience(xp);
			}
		}
		for (const auto& pair : *args->ships_deleted.begin()) {
			Widelands::Player* player = egbase.get_player(pair.first);
			dynamic_cast<Widelands::Ship&>(
			   egbase.create_ship(center.node, player->tribe().ship(), player))
			   .set_shipname(pair.second);
		}
		args->workers_deleted.erase(args->workers_deleted.begin());
		args->ships_deleted.erase(args->ships_deleted.begin());
	} while (mr.advance(*map));
	assert(args->workers_deleted.empty());
	assert(args->ships_deleted.empty());
	return mr.radius();
}

EditorActionArgs ScenarioDeleteWorkerTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
