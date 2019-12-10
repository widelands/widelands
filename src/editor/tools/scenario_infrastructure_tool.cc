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

#include "editor/tools/scenario_infrastructure_tool.h"

#include "economy/flag.h"
#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/map_objects/map_object.h"
#include "logic/mapregion.h"
#include "logic/widelands_geometry.h"

int32_t ScenarioInfrastructureTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                      EditorInteractive& eia,
                                                      EditorActionArgs* args,
                                                      Widelands::Map* map) {
	if (args->new_owner < 1 || args->new_owner > map->get_nrplayers() ||
			args->infrastructure_types.empty()) {
		return 0;
	}
	const size_t nr_items = args->infrastructure_types.size();
	Widelands::EditorGameBase& egbase = eia.egbase();
	args->infrastructure_placed = 0;
	if (args->random_index < 0) {
		args->random_index = std::rand() % nr_items;
	}
	const std::pair<Widelands::MapObjectType, Widelands::DescriptionIndex>& item_to_place =
			args->infrastructure_types[args->random_index];
	Widelands::MapObject* mo = nullptr;
	switch (item_to_place.first) {
		case Widelands::MapObjectType::BUILDING: {
			if (args->infrastructure_constructionsite &&
					egbase.tribes().get_building_descr(item_to_place.second)->is_buildable()) {
				mo = &egbase.get_player(args->new_owner)->force_csite(center.node, item_to_place.second);
			} else {
				Widelands::FormerBuildings b;
				b.push_back(std::make_pair(item_to_place.second, ""));
				mo = &egbase.get_player(args->new_owner)->force_building(center.node, b);
			}
			break;
		}
		case Widelands::MapObjectType::FLAG: {
			mo = &egbase.get_player(args->new_owner)->force_flag(map->get_fcoords(center.node));
			break;
		}
		case Widelands::MapObjectType::IMMOVABLE: {
			mo = &egbase.create_immovable(center.node, item_to_place.second,
					Widelands::MapObjectDescr::OwnerType::kTribe, egbase.get_player(args->new_owner));
			break;
		}
		default:
			NEVER_HERE();
	}
	args->infrastructure_placed = mo ? mo->serial() : 0;
	return 1;
}

int32_t ScenarioInfrastructureTool::handle_undo_impl(const Widelands::NodeAndTriangle<>&,
                                                     EditorInteractive& eia,
                                                     EditorActionArgs* args,
                                                     Widelands::Map*) {
	eia.egbase().objects().get_object(args->infrastructure_placed)->remove(eia.egbase());
	return 1;
}

EditorActionArgs ScenarioInfrastructureTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = player_;
	a.infrastructure_constructionsite = construct_;
	a.random_index = -1;
	for (auto& pair : index_) {
		a.infrastructure_types.push_back(pair);
	}
	return a;
}

int32_t ScenarioInfrastructureDeleteTool::handle_click_impl(
                                                       const Widelands::NodeAndTriangle<>& center,
                                                       EditorInteractive& eia,
                                                       EditorActionArgs* args,
                                                       Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		EditorActionArgs::InfrastructureHistory history;
		if (upcast(Widelands::BaseImmovable, imm, mr.location().field->get_immovable())) {
			bool destroy = false;
			if (upcast(Widelands::Flag, flag, imm)) {
				history.type = Widelands::MapObjectType::FLAG;
				history.owner = flag->owner().player_number();
				destroy = true;
			} else if (upcast(Widelands::Immovable, i, imm)) {
				if (i->descr().owner_type() == Widelands::MapObjectDescr::OwnerType::kTribe) {
					history.type = Widelands::MapObjectType::IMMOVABLE;
					history.name = i->descr().name();
					history.owner = i->owner().player_number();
					destroy = true;
				}
			} else if (upcast(Widelands::Building, b, imm)) {
				history.type = Widelands::MapObjectType::BUILDING;
				history.constructionsite = b->descr().type() == Widelands::MapObjectType::CONSTRUCTIONSITE;
				history.name = history.constructionsite ?
						dynamic_cast<Widelands::ConstructionSite*>(b)->building().name() : b->descr().name();
				history.owner = b->owner().player_number();
				history.settings = b->create_building_settings();
				destroy = true;
			}
			if (destroy) {
				imm->remove(eia.egbase());
			}
		}
		args->infrastructure_deleted.push_back(history);
	} while (mr.advance(*map));
	return mr.radius();
}

int32_t ScenarioInfrastructureDeleteTool::handle_undo_impl(
                                                      const Widelands::NodeAndTriangle<>& center,
                                                      EditorInteractive& eia,
                                                      EditorActionArgs* args,
                                                      Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	auto it = args->infrastructure_deleted.begin();
	do {
		switch (it->type) {
			case Widelands::MapObjectType::MAPOBJECT:
				// No item was here
				break;
			case Widelands::MapObjectType::FLAG:
				egbase.get_player(it->owner)->force_flag(mr.location());
				break;
			case Widelands::MapObjectType::IMMOVABLE:
				egbase.create_immovable(mr.location(), egbase.tribes().safe_immovable_index(it->name),
						Widelands::MapObjectDescr::OwnerType::kTribe, egbase.get_player(it->owner));
				break;
			case Widelands::MapObjectType::BUILDING:
				if (it->constructionsite) {
					egbase.get_player(it->owner)->force_csite(mr.location(),
							egbase.tribes().safe_building_index(it->name));
				} else {
					Widelands::FormerBuildings b;
					b.push_back(std::make_pair(egbase.tribes().safe_building_index(it->name), ""));
					egbase.get_player(it->owner)->force_building(mr.location(), b);
				}
				break;
			default:
				NEVER_HERE();
		}
		++it;
	} while (mr.advance(*map));
	args->infrastructure_deleted.clear();
	return mr.radius();
}

EditorActionArgs ScenarioInfrastructureDeleteTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
