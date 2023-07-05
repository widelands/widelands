/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "editor/tools/place_critter_tool.h"

#include "editor/editorinteractive.h"
#include "logic/editor_game_base.h"
#include "logic/field.h"
#include "logic/map_objects/bob.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/critter.h"
#include "logic/mapregion.h"

/**
 * Choses an object to place randomly from all enabled
 * and places this on the current field
 */
int32_t EditorPlaceCritterTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = parent_.egbase();
	if ((get_nr_enabled() != 0) && args->old_bob_type.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map,
		   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
		do {
			Widelands::Bob* const mbob = mr.location().field->get_first_bob();
			args->old_bob_type.push_back((mbob != nullptr ? &mbob->descr() : nullptr));
			args->new_bob_type.push_back(dynamic_cast<const Widelands::BobDescr*>(
			   egbase.descriptions().get_critter_descr(get_random_enabled())));
		} while (mr.advance(*map));
	}

	if (!args->new_bob_type.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map,
		   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
		std::list<const Widelands::BobDescr*>::iterator i = args->new_bob_type.begin();
		auto gap_it = args->selection_gaps.cbegin();
		do {
			if (*gap_it++) {
				continue;
			}

			const Widelands::BobDescr& descr = *(*i);
			if ((mr.location().field->nodecaps() & descr.movecaps()) != 0u) {
				if (Widelands::Bob* const bob = mr.location().field->get_first_bob()) {
					bob->remove(egbase);  //  There is already a bob. Remove it.
				}
				descr.create(egbase, nullptr, mr.location());
			}
			++i;
		} while (mr.advance(*map));
		return mr.radius() + 2;
	}
	return 0;
}

int32_t EditorPlaceCritterTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorActionArgs* args,
   Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = parent_.egbase();
	if (!args->new_bob_type.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map,
		   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
		std::list<const Widelands::BobDescr*>::iterator i = args->old_bob_type.begin();
		auto gap_it = args->selection_gaps.cbegin();
		do {
			if (*gap_it++) {
				continue;
			}

			if (*i != nullptr) {
				const Widelands::BobDescr& descr = *(*i);
				if ((mr.location().field->nodecaps() & descr.movecaps()) != 0u) {
					if (Widelands::Bob* const bob = mr.location().field->get_first_bob()) {
						bob->remove(egbase);  //  There is already a bob. Remove it.
					}
					descr.create(egbase, nullptr, mr.location());
				}
			} else if (Widelands::Bob* const bob = mr.location().field->get_first_bob()) {
				bob->remove(egbase);
			}
			++i;
		} while (mr.advance(*map));
		return mr.radius() + 2;
	}
	return 0;
}

EditorActionArgs EditorPlaceCritterTool::format_args_impl() {
	return EditorTool::format_args_impl();
}

std::string EditorPlaceCritterTool::format_conf_description_impl(const ToolConf& conf) {
	const Widelands::Descriptions& descriptions = parent_.egbase().descriptions();
	const Widelands::DescriptionMaintainer<Widelands::CritterDescr>& critter_descriptions =
	   descriptions.critters();

	std::string mapobj_names;

	for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
		if (!mapobj_names.empty()) {
			mapobj_names += " | ";
		}
		mapobj_names += critter_descriptions.get(idx).descname();
	}

	/** TRANSLATORS: An entry in the tool history list. */
	return format(_("Critter: %1$s"), mapobj_names);
}

bool EditorPlaceCritterTool::save_configuration_impl(ToolConf& conf) {
	if (0 == get_nr_enabled()) {
		return false;
	}

	conf.map_obj_types.insert(get_enabled().begin(), get_enabled().end());

	return true;
}

void EditorPlaceCritterTool::load_configuration(const ToolConf& conf) {
	disable_all();
	for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
		enable(idx, true);
	}
}
