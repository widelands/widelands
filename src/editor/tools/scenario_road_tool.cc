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

#include "editor/tools/scenario_road_tool.h"

#include "economy/road.h"
#include "economy/waterway.h"
#include "editor/editorinteractive.h"
#include "logic/map_objects/checkstep.h"

static bool create_road(Widelands::EditorGameBase& egbase,
                        Widelands::Path path,
                        EditorActionArgs::RoadMode mode,
                        bool primary_carrier,
                        bool secondary_carrier,
                        bool force) {
	if (path.get_nsteps() < 2) {
		return false;
	}
	secondary_carrier &= mode == EditorActionArgs::RoadMode::kBusy;
	upcast(Widelands::Flag, flag, egbase.map()[path.get_start()].get_immovable());
	assert(flag);
	Widelands::Player& player = *flag->get_owner();
	// Create the end flag if it doesn't exist yet
	if (force) {
		player.force_flag(egbase.map().get_fcoords(path.get_end()));
	} else {
		player.build_flag(path.get_end());
	}
	Widelands::CoordPath cp(egbase.map(), path);
	const auto& cv = cp.get_coords();
	const auto first = cv.begin() + 1;
	const auto last = cv.end() - 1;
	if (mode == EditorActionArgs::RoadMode::kWaterway) {
		Widelands::Waterway* ww = force ? &player.force_waterway(path) : player.build_waterway(path);
		if (!ww) {
			return false;
		}
		// Build flags along the way whereever possible
		if (SDL_GetModState() & KMOD_CTRL) {
			if (SDL_GetModState() & KMOD_SHIFT) {
				for (auto it = first; it <= last; ++it) {
					player.build_flag(egbase.map().get_fcoords(*it));
				}
			} else {
				for (auto it = last; first <= it; --it) {
					player.build_flag(egbase.map().get_fcoords(*it));
				}
			}
		}
		// Create ferries if desired
		if (primary_carrier) {
			std::set<Widelands::Waterway*> wws;
			for (auto it = first; it <= last; ++it) {
				if (upcast(Widelands::Waterway, w, egbase.map()[*it].get_immovable()))
					if (!wws.count(w))
						wws.insert(w);
			}
			assert(!wws.empty());
			for (Widelands::Waterway* w : wws) {
				Widelands::Ferry& ferry = dynamic_cast<Widelands::Ferry&>(
				   egbase.create_ferry(cv[ww->get_idle_index()], &player));
				w->assign_carrier(ferry, 0);
				ferry.set_location(w);
			}
		}
		return true;
	} else {
		Widelands::Road* road = force ? &player.force_road(path) : player.build_road(path);
		if (!road) {
			return false;
		}
		// Build flags along the road whereever possible
		if (SDL_GetModState() & KMOD_CTRL) {
			if (SDL_GetModState() & KMOD_SHIFT) {
				for (auto it = first; it <= last; ++it) {
					player.build_flag(egbase.map().get_fcoords(*it));
				}
			} else {
				for (auto it = last; first <= it; --it) {
					player.build_flag(egbase.map().get_fcoords(*it));
				}
			}
		}
		// Create carriers if desired
		if (primary_carrier || secondary_carrier) {
			std::set<Widelands::Road*> roads;
			for (auto it = first; it <= last; ++it) {
				if (upcast(Widelands::Road, r, egbase.map()[*it].get_immovable()))
					if (!roads.count(r))
						roads.insert(r);
			}
			assert(!roads.empty());
			for (Widelands::Road* r : roads) {
				// r->busy_ = mode == EditorActionArgs::RoadMode::kBusy; // NOCOM
				if (primary_carrier) {
					Widelands::Carrier& c = dynamic_cast<Widelands::Carrier&>(
					   egbase.tribes()
					      .get_worker_descr(player.tribe().carrier())
					      ->create(egbase, &player, r, cv[road->get_idle_index()]));
					r->assign_carrier(c, 0);
				}
				if (secondary_carrier) {
					Widelands::Carrier& c = dynamic_cast<Widelands::Carrier&>(
					   egbase.tribes()
					      .get_worker_descr(player.tribe().carrier2())
					      ->create(egbase, &player, r, cv[road->get_idle_index()]));
					r->assign_carrier(c, 1);
				}
			}
		}
		return true;
	}
}

int32_t ScenarioPlaceRoadTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                 EditorInteractive& eia,
                                                 EditorActionArgs* args,
                                                 Widelands::Map* map) {
	upcast(Widelands::Flag, flag, (*map)[center.node].get_immovable());
	if (args->road_mode == EditorActionArgs::RoadMode::kWaterway) {
		assert(!eia.is_building_road());
		if (eia.is_building_waterway()) {
			if (eia.get_build_waterway_end() == center.node) {
				Widelands::CoordPath p = eia.get_build_waterway_path();
				if (eia.get_build_waterway_start() == eia.get_build_waterway_end() ||
				    create_road(eia.egbase(), Widelands::Path(p), args->road_mode,
				                args->create_primary_worker, args->create_secondary_worker,
				                args->force)) {
					eia.abort_build_waterway();
				}
			} else {
				eia.append_build_waterway(center.node);
				Widelands::CoordPath p = eia.get_build_waterway_path();
				if (flag && create_road(eia.egbase(), Widelands::Path(p), args->road_mode,
				                        args->create_primary_worker, args->create_secondary_worker,
				                        args->force)) {
					eia.abort_build_waterway();
				}
			}
		} else {
			if (!flag) {
				return 0;
			}
			eia.start_build_waterway(center.node, flag->owner().player_number());
		}
	} else {
		assert(!eia.is_building_waterway());
		if (eia.is_building_road()) {
			if (eia.get_build_road_end() == center.node) {
				Widelands::CoordPath p = eia.get_build_road_path();
				if (eia.get_build_road_start() == eia.get_build_road_end() ||
				    create_road(eia.egbase(), Widelands::Path(p), args->road_mode,
				                args->create_primary_worker, args->create_secondary_worker,
				                args->force)) {
					eia.abort_build_road();
				}
			} else {
				eia.append_build_road(center.node);
				Widelands::CoordPath p = eia.get_build_road_path();
				if (flag && create_road(eia.egbase(), Widelands::Path(p), args->road_mode,
				                        args->create_primary_worker, args->create_secondary_worker,
				                        args->force)) {
					eia.abort_build_road();
				}
			}
		} else {
			if (!flag) {
				return 0;
			}
			eia.start_build_road(center.node, flag->owner().player_number());
		}
	}
	return 1;
}

EditorActionArgs ScenarioPlaceRoadTool::format_args_impl(EditorInteractive& parent) {
	EditorActionArgs a(parent);
	a.road_mode = mode_;
	a.create_primary_worker = create_primary_worker_;
	a.create_secondary_worker = create_secondary_worker_;
	a.force = force_;
	return a;
}

int32_t ScenarioDeleteRoadTool::handle_click_impl(const Widelands::NodeAndTriangle<>& center,
                                                  EditorInteractive& eia,
                                                  EditorActionArgs* args,
                                                  Widelands::Map* map) {
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   *map, Widelands::Area<Widelands::FCoords>(map->get_fcoords(center.node), args->sel_radius));
	do {
		if (upcast(Widelands::RoadBase, r, mr.location().field->get_immovable())) {
			r->remove(eia.egbase());
		}
	} while (mr.advance(*map));
	return mr.radius();
}

EditorActionArgs ScenarioDeleteRoadTool::format_args_impl(EditorInteractive& parent) {
	return EditorTool::format_args_impl(parent);
}
