/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
                                                  EditorInteractive& eia,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
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
		do {
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
   EditorInteractive& eia,
   EditorActionArgs* args,
   Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
	if (!args->new_bob_type.empty()) {
		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   *map,
		   Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
		std::list<const Widelands::BobDescr*>::iterator i = args->old_bob_type.begin();
		do {
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

EditorActionArgs EditorPlaceCritterTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}


std::string EditorPlaceCritterTool::format_conf_string_impl(EditorInteractive& parent, const ToolConf& conf) {
        const Widelands::Descriptions& descriptions = parent.egbase().descriptions();
        const Widelands::DescriptionMaintainer<Widelands::CritterDescr>& critter_descriptions = descriptions.critters();

	std::string buf;
	constexpr int max_string_size = 100;
	int j = get_nr_enabled();
	for (int i = 0; j && buf.size() < max_string_size; ++i) {
		if (is_enabled(i)) {
			if (j < get_nr_enabled()) {
				buf += " | ";
			}
			buf += critter_descriptions.get(i).descname();
			--j;
		}
	}

        return format(_("Place critter: %s; size: %d"), buf, conf.sel_radius);
}

bool EditorPlaceCritterTool::save_configuration_impl(ToolConf& conf, EditorInteractive&) {
	int j = get_nr_enabled();

        if (j == 0) {
                return false;
        }

	for (int i = 0; j; ++i) {
                if (is_enabled(i)) {
                        conf.critter_types.push_back(i);
                        --j;
                }
        }

        return true;
}


void EditorPlaceCritterTool::load_configuration(const ToolConf& conf) {
        disable_all();
        std::list<Widelands::DescriptionIndex>::const_iterator p = conf.critter_types.begin();
        while (p != conf.critter_types.end()) {
                enable(*p, true);
                ++p;
        }
}
