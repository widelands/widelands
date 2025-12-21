/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#include "logic/map_objects/tribes/worker.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {

bool Worker::terraform_prevented(const FCoords& coord, Game& game) {
	if ((coord.field->nodecaps() & Widelands::MOVECAPS_WALK) != 0) {
		// fields that are on the shore or inland can't hold a portdock
		return false;
	}
	BaseImmovable* imm = coord.field->get_immovable();
	if (imm == nullptr) {
		// there is no immovable ergo no portdock
		Coords ports_space = game.map().find_portspace_for_dockpoint(coord);
		if (ports_space == Coords::null()) {
			// no portspace is available from this field
			return false;
		}
		BaseImmovable* ps_imm = game.map().get_fcoords(ports_space).field->get_immovable();
		if (ps_imm != nullptr && ps_imm->descr().type() == MapObjectType::CONSTRUCTIONSITE) {
			upcast(Widelands::ConstructionSite const, constructionsite, ps_imm);
			// only true if a port construction is going on at the field
			return constructionsite->building().get_isport();
		}
		// no constructionsite on portspace
		return false;
	}
	if (imm->descr().type() == MapObjectType::PORTDOCK) {
		// there is a portdock
		return true;
	}
	return false;
}

bool Worker::run_terraform(Game& game, State& state, const Action& a) {
	const Descriptions& descriptions = game.descriptions();
	std::map<TCoords<FCoords>, DescriptionIndex> triangles;
	const FCoords f = get_position();
	FCoords tln;
	FCoords ln;
	FCoords trn;
	game.map().get_tln(f, &tln);
	game.map().get_trn(f, &trn);
	game.map().get_ln(f, &ln);

	DescriptionIndex di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(f.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(f, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(f.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(f, TriangleIndex::D), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(tln.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::D), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(ln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(ln, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(trn.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX) {
		triangles.emplace(TCoords<FCoords>(trn, TriangleIndex::D), di);
	}

	if (triangles.empty()) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}
	assert(game.mutable_map());
	auto it = triangles.begin();
	for (size_t rand = game.logic_rand() % triangles.size(); rand > 0; --rand) {
		++it;
	}
	FCoords second_triangle_point;
	FCoords third_triangle_point;
	if (it->first.t == TriangleIndex::D) {
		game.map().get_brn(it->first.node, &second_triangle_point);
		game.map().get_bln(it->first.node, &third_triangle_point);
	} else if (it->first.t == TriangleIndex::R) {
		game.map().get_rn(it->first.node, &second_triangle_point);
		game.map().get_brn(it->first.node, &third_triangle_point);
	}
	if (terraform_prevented(it->first.node, game) ||
	    terraform_prevented(second_triangle_point, game) ||
	    terraform_prevented(third_triangle_point, game)) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}
	game.mutable_map()->change_terrain(game, it->first, it->second);
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
