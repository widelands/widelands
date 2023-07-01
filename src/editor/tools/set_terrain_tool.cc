/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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

#include "editor/tools/set_terrain_tool.h"

#include <sstream>

#include "editor/editorinteractive.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/maptriangleregion.h"

using Widelands::TCoords;

int32_t EditorSetTerrainTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                EditorActionArgs* args,
                                                Widelands::Map* map) {
	assert(center.triangle.t == Widelands::TriangleIndex::D ||
	       center.triangle.t == Widelands::TriangleIndex::R);
	uint16_t const radius = args->sel_radius;
	int32_t max = 0;

	if ((get_nr_enabled() != 0) && args->terrain_type.empty()) {
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords>> mr(
		   *map, Widelands::Area<TCoords<Widelands::FCoords>>(
		            TCoords<Widelands::FCoords>(
		               Widelands::FCoords(map->get_fcoords(center.triangle.node)), center.triangle.t),
		            radius));
		do {
			args->original_terrain_type.push_back((mr.location().t == Widelands::TriangleIndex::D) ?
                                                  mr.location().node.field->terrain_d() :
                                                  mr.location().node.field->terrain_r());
			args->terrain_type.push_back(get_random_enabled());
		} while (mr.advance(*map));
	}

	if (!args->terrain_type.empty()) {
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords>> mr(
		   *map, Widelands::Area<TCoords<Widelands::FCoords>>(
		            TCoords<Widelands::FCoords>(
		               Widelands::FCoords(map->get_fcoords(center.triangle.node)), center.triangle.t),
		            radius));
		std::list<Widelands::DescriptionIndex>::iterator i = args->terrain_type.begin();
		auto gap_it = args->selection_gaps.cbegin();
		do {
			if (*gap_it++) {
				continue;
			}

			max = std::max(max, map->change_terrain(parent_.egbase(), mr.location(), *i));
			++i;
		} while (mr.advance(*map));
	}
	return radius + max;
}

int32_t
EditorSetTerrainTool::handle_undo_impl(const Widelands::NodeAndTriangle<Widelands::Coords>& center,
                                       EditorActionArgs* args,
                                       Widelands::Map* map) {
	assert(center.triangle.t == Widelands::TriangleIndex::D ||
	       center.triangle.t == Widelands::TriangleIndex::R);
	uint16_t const radius = args->sel_radius;
	if (!args->terrain_type.empty()) {
		int32_t max = 0;
		Widelands::MapTriangleRegion<TCoords<Widelands::FCoords>> mr(
		   *map, Widelands::Area<TCoords<Widelands::FCoords>>(
		            TCoords<Widelands::FCoords>(
		               Widelands::FCoords(map->get_fcoords(center.triangle.node)), center.triangle.t),
		            radius));

		std::list<Widelands::DescriptionIndex>::iterator i = args->original_terrain_type.begin();
		auto gap_it = args->selection_gaps.cbegin();
		do {
			if (*gap_it++) {
				continue;
			}

			max = std::max(max, map->change_terrain(parent_.egbase(), mr.location(), *i));
			++i;
		} while (mr.advance(*map));
		return radius + max;
	}
	return radius;
}

EditorActionArgs EditorSetTerrainTool::format_args_impl() {
	return EditorTool::format_args_impl();
}

std::string EditorSetTerrainTool::format_conf_description_impl(const ToolConf& conf) {
	const Widelands::Descriptions& descriptions = parent_.egbase().descriptions();
	const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>& terrain_descriptions =
	   descriptions.terrains();

	std::string mapobj_names;

	for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
		if (!mapobj_names.empty()) {
			mapobj_names += " | ";
		}
		mapobj_names += terrain_descriptions.get(idx).descname();
	}

	/** TRANSLATORS: An entry in the tool history list. */
	return format(_("Terrain: %1$s"), mapobj_names);
}

bool EditorSetTerrainTool::save_configuration_impl(ToolConf& conf) {
	if (0 == get_nr_enabled()) {
		return false;
	}

	conf.map_obj_types.insert(get_enabled().begin(), get_enabled().end());

	return true;
}

void EditorSetTerrainTool::load_configuration(const ToolConf& conf) {
	disable_all();
	for (Widelands::DescriptionIndex idx : conf.map_obj_types) {
		enable(idx, true);
	}
}
