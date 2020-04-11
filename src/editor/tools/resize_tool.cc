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

#include "editor/tools/resize_tool.h"

#include "editor/editorinteractive.h"
#include "logic/field.h"
#include "logic/widelands_geometry.h"

int32_t EditorResizeTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                            EditorInteractive& eia,
                                            EditorActionArgs* args,
                                            Widelands::Map* map) {
	args->resized.old_map_size = map->extent();
	args->resized.port_spaces.clear();
	args->resized.starting_positions.clear();
	for (const Widelands::Coords& ps : map->get_port_spaces()) {
		args->resized.port_spaces.insert(Widelands::Coords(ps));
	}
	for (uint8_t i = 1; i <= map->get_nrplayers(); ++i) {
		args->resized.starting_positions.push_back(map->get_starting_pos(i));
	}

	args->resized.deleted_fields =
	   map->resize(eia.egbase(), center.node, args->new_map_size.w, args->new_map_size.h);

	// fix for issue #3754
	Widelands::NodeAndTriangle<> sel = eia.get_sel_pos();
	map->normalize_coords(sel.node);
	map->normalize_coords(sel.triangle.node);
	eia.set_sel_pos(sel);

	map->recalc_whole_map(eia.egbase());
	return 0;
}

int32_t
EditorResizeTool::handle_undo_impl(const Widelands::NodeAndTriangle<Widelands::Coords>& center,
                                   EditorInteractive& eia,
                                   EditorActionArgs* args,
                                   Widelands::Map* map) {
	Widelands::EditorGameBase& egbase = eia.egbase();
	map->resize(egbase, center.node, args->resized.old_map_size.w, args->resized.old_map_size.h);

	// fix for issue #3754
	Widelands::NodeAndTriangle<> sel = eia.get_sel_pos();
	map->normalize_coords(sel.node);
	map->normalize_coords(sel.triangle.node);
	eia.set_sel_pos(sel);

	for (const auto& it : args->resized.deleted_fields) {
		const Widelands::FCoords f = map->get_fcoords(it.first);
		const Widelands::FieldData data = it.second;

		if (Widelands::BaseImmovable* imm = f.field->get_immovable()) {
			imm->remove(egbase);
		}
		for (Widelands::Bob* bob = f.field->get_first_bob(); bob; bob = bob->get_next_bob()) {
			bob->remove(egbase);
		}

		f.field->set_height(data.height);
		f.field->set_terrains(data.terrains);
		map->initialize_resources(f, data.resources, data.resource_amount);

		if (!data.immovable.empty()) {
			egbase.create_immovable_with_name(
			   f, data.immovable, Widelands::MapObjectDescr::OwnerType::kWorld, nullptr, nullptr);
		}
		for (const std::string& bob : data.bobs) {
			egbase.create_critter(f, bob);
		}
	}

	for (const Widelands::Coords& c : args->resized.port_spaces) {
		map->set_port_space(egbase, c, true, true);
	}
	for (uint8_t i = 1; i <= map->get_nrplayers(); ++i) {
		map->set_starting_pos(i, args->resized.starting_positions[i - 1]);
	}

	map->recalc_whole_map(egbase);
	return 0;
}

EditorActionArgs EditorResizeTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.new_map_size = Widelands::Extent(width_, height_);
	return a;
}
