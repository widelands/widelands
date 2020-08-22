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
	if (args->random_index == kRandomIndexNotSet) {
		args->random_index = std::rand() % nr_items;  // NOLINT
	}
	const std::pair<Widelands::MapObjectType, Widelands::DescriptionIndex>& item_to_place =
	   args->infrastructure_types[args->random_index];
	Widelands::Player& player = *egbase.get_player(args->new_owner);
	Widelands::MapObject* mo = nullptr;
	switch (item_to_place.first) {
	case Widelands::MapObjectType::BUILDING: {
		Widelands::FormerBuildings b;
		if (!args->infrastructure_constructionsite) {
			b.push_back(std::make_pair(item_to_place.second, ""));
		}
		if (args->force) {
			if (args->infrastructure_constructionsite &&
			    egbase.tribes().get_building_descr(item_to_place.second)->is_buildable()) {
				mo = &player.force_csite(center.node, item_to_place.second);
			} else {
				mo = &player.force_building(center.node, b);
			}
		} else {
			mo = player.build(
			   center.node, item_to_place.second, args->infrastructure_constructionsite, b);
		}
		break;
	}
	case Widelands::MapObjectType::FLAG: {
		mo = args->force ? &player.force_flag(map->get_fcoords(center.node)) :
		                   player.build_flag(map->get_fcoords(center.node));
		break;
	}
	case Widelands::MapObjectType::IMMOVABLE: {
		if (args->force || !(*map)[center.node].get_immovable()) {
			mo = &egbase.create_immovable(center.node, item_to_place.second,
			                              Widelands::MapObjectDescr::OwnerType::kTribe, &player);
		}
		break;
	}
	default:
		NEVER_HERE();
	}
	args->infrastructure_placed = mo ? mo->serial() : 0;
	return 1;
}

EditorActionArgs ScenarioInfrastructureTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_owner = player_;
	a.force = force_;
	a.infrastructure_constructionsite = construct_;
	a.random_index = kRandomIndexNotSet;
	for (auto& pair : index_) {
		a.infrastructure_types.push_back(pair);
	}
	return a;
}

int32_t
ScenarioInfrastructureDeleteTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorInteractive& eia,
                                                    EditorActionArgs* args,
                                                    Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		if (Widelands::BaseImmovable* imm = mr.location().field->get_immovable()) {
			if (imm->descr().type() != Widelands::MapObjectType::FLAG &&
			    !is_a(Widelands::Building, imm)) {
				upcast(Widelands::Immovable, i, imm);
				if (!i || i->descr().owner_type() != Widelands::MapObjectDescr::OwnerType::kTribe) {
					continue;
				}
			}
			imm->remove(eia.egbase());
		}
	} while (mr.advance(*map));
	return mr.radius();
}

EditorActionArgs ScenarioInfrastructureDeleteTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
