/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#include "editor/tools/set_port_space_tool.h"

#include <limits>
#include <memory>

#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/mapfringeregion.h"
#include "logic/mapregion.h"

namespace {

Widelands::NodeCaps port_tool_nodecaps(const Widelands::FCoords& c, const Widelands::Map& map) {
	Widelands::NodeCaps const caps = c.field->nodecaps();
	if ((caps & Widelands::BUILDCAPS_SIZEMASK) == Widelands::BUILDCAPS_BIG) {
		if (!map.find_portdock(c, false).empty()) {
			return caps;
		}
	}
	return Widelands::NodeCaps::CAPS_NONE;
}

}  // namespace

EditorSetPortSpaceTool::EditorSetPortSpaceTool(EditorInteractive& parent,
                                               EditorUnsetPortSpaceTool& the_unset_tool)
   : EditorTool(parent, the_unset_tool, the_unset_tool) {
	field_terrain_changed_subscriber_ = Notifications::subscribe<Widelands::NoteFieldTerrainChanged>(
	   [this](const Widelands::NoteFieldTerrainChanged& /*note*/) { workareas_.reset(); });
}

EditorUnsetPortSpaceTool::EditorUnsetPortSpaceTool(EditorInteractive& parent)
   : EditorTool(parent, *this, *this) {
}

int32_t EditorSetPortSpaceTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	auto gap_it = args->selection_gaps.cbegin();
	do {
		if (*gap_it++) {
			continue;
		}
		//  check if field is valid
		if (port_tool_nodecaps(mr.location(), *map) != Widelands::NodeCaps::CAPS_NONE) {
			map->set_port_space(parent_.egbase(), mr.location(), true);
			Widelands::Area<Widelands::FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(parent_.egbase(), a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

Widelands::NodeCaps
EditorSetPortSpaceTool::nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
                                               const Widelands::EditorGameBase& egbase) {
	return port_tool_nodecaps(fcoords, egbase.map());
}

int32_t EditorSetPortSpaceTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorActionArgs* args,
   Widelands::Map* map) {
	return parent_.tools()->unset_port_space.handle_click_impl(center, args, map);
}

int32_t EditorUnsetPortSpaceTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                    EditorActionArgs* args,
                                                    Widelands::Map* map) {
	assert(0 <= center.node.x);
	assert(center.node.x < map->get_width());
	assert(0 <= center.node.y);
	assert(center.node.y < map->get_height());

	uint32_t nr = 0;

	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	auto gap_it = args->selection_gaps.cbegin();
	do {
		if (*gap_it++) {
			continue;
		}
		//  check if field is valid
		if (port_tool_nodecaps(mr.location(), *map) != 0u) {
			map->set_port_space(parent_.egbase(), mr.location(), false);
			Widelands::Area<Widelands::FCoords> a(mr.location(), 0);
			map->recalc_for_field_area(parent_.egbase(), a);
			++nr;
		}
	} while (mr.advance(*map));

	return nr;
}

int32_t EditorUnsetPortSpaceTool::handle_undo_impl(
   const Widelands::NodeAndTriangle<Widelands::Coords>& center,
   EditorActionArgs* args,
   Widelands::Map* map) {
	return parent_.tools()->set_port_space.handle_click_impl(center, args, map);
}

Widelands::NodeCaps
EditorUnsetPortSpaceTool::nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
                                                 const Widelands::EditorGameBase& egbase) {
	return port_tool_nodecaps(fcoords, egbase.map());
}

Workareas EditorSetPortSpaceTool::get_overlays() {
	if (workareas_ == nullptr) {
		// Find all oceans on the map.
		const Widelands::EditorGameBase& egbase = parent_.egbase();
		const Widelands::Map& map = egbase.map();
		const Widelands::CheckStepDefault checkstep(Widelands::MOVECAPS_SWIM);
		const Widelands::FindNodeAlwaysTrue functor;

		// Ocean indices are 1-based, 0 means none
		const size_t nr_fields = map.max_index();
		std::unique_ptr<uint32_t[]> field_to_ocean_index(new uint32_t[nr_fields]);
		unsigned nr_oceans = 0;
		memset(field_to_ocean_index.get(), 0, nr_fields * sizeof(uint32_t));
		static std::vector<uint32_t> kOceanColors;

		for (size_t index = 0; index < nr_fields; ++index) {
			if (field_to_ocean_index[index] != 0) {
				continue;  // Already visited
			}

			Widelands::Field& field = map[index];
			if ((field.nodecaps() & Widelands::MOVECAPS_SWIM) == 0) {
				continue;  // Not an ocean field
			}

			// New ocean found.
			++nr_oceans;

			constexpr uint32_t kAlpha = 0x9f << 24;
			if (kOceanColors.empty()) {
				for (const RGBColor& col : kPlayerColors) {
					kOceanColors.emplace_back(kAlpha | (col.r << 16) | (col.g << 8) | (col.b));
				}
			}
			while (nr_oceans > kOceanColors.size()) {
				kOceanColors.emplace_back(kAlpha | (RNG::static_rand() % 0x1000000));
			}

			std::vector<Widelands::Coords> ocean_fields;
			map.find_reachable_fields(egbase,
			                          Widelands::Area<Widelands::FCoords>(
			                             map.get_fcoords(field), std::numeric_limits<uint16_t>::max()),
			                          &ocean_fields, checkstep, functor);

			// Store and validate the fields.
			for (const Widelands::Coords& coords : ocean_fields) {
				size_t i = map.get_index(coords);
				if (field_to_ocean_index[i] != 0 && field_to_ocean_index[i] != nr_oceans) {
					throw wexception("Field #%" PRIuS " (%dx%d) belongs to two oceans #%d and #%d!", i,
					                 coords.x, coords.y, field_to_ocean_index[i], nr_oceans);
				}
				field_to_ocean_index[i] = nr_oceans;
			}

			if (field_to_ocean_index[index] != nr_oceans) {
				throw wexception("Field #%" PRIuS
				                 " does not belong to its own ocean #%d of size %" PRIuS "!",
				                 index, nr_oceans, ocean_fields.size());
			}
		}

		// Compute the triangles.
		// We do not bother to compute borders because oceans can have very difficult shapes.
		workareas_.reset(new Workareas(nr_oceans));

		for (int y = 0; y < map.get_height(); ++y) {
			for (int x = 0; x < map.get_width(); ++x) {
				Widelands::Coords coords(x, y);
				const uint32_t ocean = field_to_ocean_index[map.get_index(coords)];

				if (ocean == 0) {
					continue;  // Not in an ocean
				}

				std::vector<WorkareaPreviewData>& v_triangles = workareas_->at(ocean - 1).first;
				const uint32_t col = kOceanColors.at(ocean - 1);

				// Take care of all six triangles.
				// R and D are always owned; the others only if their primary owners
				// are not also in this ocean to avoid duplicates.
				// Our L's R triangle is also our BL's TL's R.
				// Our TR's D triangle is also our R's TL's D.
				const bool tr = field_to_ocean_index[map.get_index(map.tr_n(coords))] == ocean;
				const bool tl = field_to_ocean_index[map.get_index(map.tl_n(coords))] == ocean;
				const bool bl = field_to_ocean_index[map.get_index(map.bl_n(coords))] == ocean;
				const bool l = field_to_ocean_index[map.get_index(map.l_n(coords))] == ocean;
				const bool r = field_to_ocean_index[map.get_index(map.r_n(coords))] == ocean;

				v_triangles.emplace_back(
				   Widelands::TCoords<>(coords, Widelands::TriangleIndex::D), 0, col);
				v_triangles.emplace_back(
				   Widelands::TCoords<>(coords, Widelands::TriangleIndex::R), 0, col);

				if (!tl) {
					v_triangles.emplace_back(
					   Widelands::TCoords<>(map.tl_n(coords), Widelands::TriangleIndex::D), 0, col);
					v_triangles.emplace_back(
					   Widelands::TCoords<>(map.tl_n(coords), Widelands::TriangleIndex::R), 0, col);
				}

				if (!tr && !r) {
					v_triangles.emplace_back(
					   Widelands::TCoords<>(map.tr_n(coords), Widelands::TriangleIndex::D), 0, col);
				}
				if (!l && !bl) {
					v_triangles.emplace_back(
					   Widelands::TCoords<>(map.l_n(coords), Widelands::TriangleIndex::R), 0, col);
				}
			}
		}
	}

	return *workareas_;
}
