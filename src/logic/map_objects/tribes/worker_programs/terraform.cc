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

bool Worker::is_port_construction(const FCoords& coord, Game& game) {
	Coords ports_space = game.map().find_portspace_for_dockpoint(coord);
	if (ports_space == Coords::null()) {
		return false;
	}

	BaseImmovable* ps_imm = game.map().get_fcoords(ports_space).field->get_immovable();
	if (upcast(Widelands::ConstructionSite const, constructionsite, ps_imm)) {
		return constructionsite->building().type() == MapObjectType::WAREHOUSE;
	}
	return false;
}

bool Worker::run_terraform(Game& game, State& state, const Action& a) {
	const Descriptions& descriptions = game.descriptions();
	std::map<TCoords<FCoords>, DescriptionIndex> triangles;
	const FCoords f = get_position();
	DescriptionIndex di;
	FCoords tln;
	FCoords ln;
	FCoords trn;
	FCoords bln;
	FCoords rn;
	FCoords brn;
	game.map().get_tln(f, &tln);
	game.map().get_trn(f, &trn);
	game.map().get_ln(f, &ln);
	game.map().get_bln(f, &bln);
	game.map().get_brn(f, &brn);
	game.map().get_rn(f, &rn);

	BaseImmovable* imm_tln = tln.field->get_immovable();
	BaseImmovable* imm_bln = bln.field->get_immovable();
	BaseImmovable* imm_ln = ln.field->get_immovable();
	BaseImmovable* imm_trn = trn.field->get_immovable();
	BaseImmovable* imm_brn = brn.field->get_immovable();
	BaseImmovable* imm_rn = rn.field->get_immovable();

	if ((imm_brn == nullptr || imm_brn->descr().type() != MapObjectType::PORTDOCK) &&
	    !is_port_construction(brn, game)) {
		di = descriptions.terrain_index(
		   descriptions.get_terrain_descr(f.field->terrain_r())->enhancement(a.sparam1));
		if (di != INVALID_INDEX &&
		    (imm_rn == nullptr || imm_rn->descr().type() != MapObjectType::PORTDOCK) &&
		    !is_port_construction(rn, game)) {
			triangles.emplace(TCoords<FCoords>(f, TriangleIndex::R), di);
		}
		di = descriptions.terrain_index(
		   descriptions.get_terrain_descr(f.field->terrain_d())->enhancement(a.sparam1));
		if (di != INVALID_INDEX &&
		    (imm_bln == nullptr || imm_bln->descr().type() != MapObjectType::PORTDOCK) &&
		    !is_port_construction(bln, game)) {
			triangles.emplace(TCoords<FCoords>(f, TriangleIndex::D), di);
		}
	}
	if ((imm_tln == nullptr || imm_tln->descr().type() != MapObjectType::PORTDOCK) &&
	    !is_port_construction(tln, game)) {
		di = descriptions.terrain_index(
		   descriptions.get_terrain_descr(tln.field->terrain_r())->enhancement(a.sparam1));
		if (di != INVALID_INDEX &&
		    (imm_trn == nullptr || imm_trn->descr().type() != MapObjectType::PORTDOCK) &&
		    !is_port_construction(trn, game)) {
			triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::R), di);
		}
		di = descriptions.terrain_index(
		   descriptions.get_terrain_descr(tln.field->terrain_d())->enhancement(a.sparam1));
		if (di != INVALID_INDEX &&
		    (imm_ln == nullptr || imm_ln->descr().type() != MapObjectType::PORTDOCK) &&
		    !is_port_construction(ln, game)) {
			triangles.emplace(TCoords<FCoords>(tln, TriangleIndex::D), di);
		}
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(ln.field->terrain_r())->enhancement(a.sparam1));
	if (di != INVALID_INDEX &&
	    (imm_ln == nullptr || imm_ln->descr().type() != MapObjectType::PORTDOCK) &&
	    (imm_bln == nullptr || imm_bln->descr().type() != MapObjectType::PORTDOCK) &&
	    !is_port_construction(bln, game) && !is_port_construction(ln, game)) {
		triangles.emplace(TCoords<FCoords>(ln, TriangleIndex::R), di);
	}
	di = descriptions.terrain_index(
	   descriptions.get_terrain_descr(trn.field->terrain_d())->enhancement(a.sparam1));
	if (di != INVALID_INDEX &&
	    (imm_trn == nullptr || imm_trn->descr().type() != MapObjectType::PORTDOCK) &&
	    (imm_rn == nullptr || imm_rn->descr().type() != MapObjectType::PORTDOCK) &&
	    !is_port_construction(rn, game) && !is_port_construction(trn, game)) {
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
	game.mutable_map()->change_terrain(game, it->first, it->second);
	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
