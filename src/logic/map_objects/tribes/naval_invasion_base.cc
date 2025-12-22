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

#include "logic/map_objects/tribes/naval_invasion_base.h"

#include <memory>

#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

static const Duration kCheckEnemiesInterval(1000);

static NavalInvasionBaseDescr g_naval_invasion_base_descr("naval_invasion_base",
                                                          "Naval Invasion Base");

const NavalInvasionBaseDescr& NavalInvasionBase::descr() const {
	return g_naval_invasion_base_descr;
}

Bob& NavalInvasionBaseDescr::create_object() const {
	return *new NavalInvasionBase();
}

NavalInvasionBase::NavalInvasionBase() : Bob(g_naval_invasion_base_descr) {
}

NavalInvasionBase*
NavalInvasionBase::create(EditorGameBase& egbase, Player* owner, const Coords& pos) {
	NavalInvasionBase* invasion = new NavalInvasionBase();
	invasion->set_owner(owner);
	invasion->set_position(egbase, pos);

	invasion->init(egbase);

	return invasion;
}

void NavalInvasionBase::init_auto_task(Game& game) {
	for (auto it = soldiers_.begin(); it != soldiers_.end();) {
		Soldier* soldier = it->get(game);
		if (soldier == nullptr) {
			molog(game.get_gametime(), "Soldier %u no longer exists", it->serial());
			it = soldiers_.erase(it);
			continue;
		}

		State* state = soldier->get_state(Soldier::taskNavalInvasion);
		if (state == nullptr) {
			molog(game.get_gametime(), "Soldier %u no longer an invasion member", soldier->serial());
			it = soldiers_.erase(it);
			continue;
		}
		if (state->objvar1.serial() != serial()) {
			molog(game.get_gametime(), "Soldier %u at different invasion (%u)!", soldier->serial(),
			      state->objvar1.serial());
			it = soldiers_.erase(it);
			continue;
		}

		++it;
	}

	if (soldiers_.empty()) {
		schedule_destroy(game);
	}

	start_task_idle(game, 0, 1000);
}

void NavalInvasionBase::cleanup(EditorGameBase& egbase) {
	if (did_conquer_) {
		egbase.unconquer_area(PlayerArea<Area<FCoords>>(
		   owner().player_number(), Area<FCoords>(get_position(), Soldier::kPortSpaceRadius)));
	}

	Bob::cleanup(egbase);
}

void NavalInvasionBase::log_general_info(const EditorGameBase& egbase) const {
	Bob::log_general_info(egbase);
	molog(egbase.get_gametime(), "Invasion at %3dx%3d with %" PRIuS " soldiers\n", get_position().x,
	      get_position().y, soldiers_.size());
	for (auto soldier : soldiers_) {
		molog(egbase.get_gametime(), "Soldier %u", soldier.serial());
	}
}

void NavalInvasionBase::add_soldier(Soldier* soldier) {
	assert(soldier != nullptr);
	soldiers_.emplace(soldier);
}

void NavalInvasionBase::remove_soldier(Soldier* soldier) {
	soldiers_.erase(soldier);
}

const std::vector<std::pair<Serial, Coords>>& NavalInvasionBase::get_enemy_buildings(Game& game) {
	if (game.get_gametime() > last_update_ + kCheckEnemiesInterval) {
		check_enemies(game);
	}
	return enemy_buildings_;
}

const std::vector<OPtr<Soldier>>& NavalInvasionBase::get_enemy_soldiers() const {
	// get_enemy_soldiers() should only be called after get_enemy_buildings(), when there are no
	// more buildings left.
	assert(game.get_gametime() <= last_update_ + kCheckEnemiesInterval);
	assert(enemy_buildings_.empty());

	return enemy_soldiers_;
}

void NavalInvasionBase::check_enemies(Game& game) {
	enemy_buildings_.clear();
	enemy_soldiers_.clear();

	const Map& map = game.map();
	CheckStepWalkOn checkstep(MOVECAPS_WALK, false);
	const FCoords portspace_fcoords = map.get_fcoords(get_position());
	const int32_t port_radius =
	   game.descriptions().get_building_descr(owner().tribe().port())->get_conquers();
	const int32_t max_radius = game.descriptions().get_largest_workarea();

	std::vector<ImmovableFound> results;
	map.find_reachable_immovables(game, Area<FCoords>(portspace_fcoords, port_radius + max_radius),
	                              &results, checkstep, FindImmovableAttackTarget());

	for (const ImmovableFound& result : results) {
		Building& bld = dynamic_cast<Building&>(*result.object);
		if (!owner().is_hostile(bld.owner()) || bld.attack_target() == nullptr ||
		    !bld.attack_target()->can_be_attacked() ||
		    bld.descr().get_conquers() + port_radius <
		       map.calc_distance(bld.get_position(), get_position())) {
			continue;
		}
		enemy_buildings_.emplace_back(result.object->serial(), result.coords);
	}

	if (enemy_buildings_.empty()) {
		std::vector<Bob*> hostile_soldiers;
		map.find_reachable_bobs(game, Area<FCoords>(portspace_fcoords, port_radius),
		                        &hostile_soldiers, checkstep, FindBobEnemySoldier(get_owner()));
		for (Bob* bob : hostile_soldiers) {
			upcast(Soldier, soldier, bob);
			assert(soldier != nullptr);
			enemy_soldiers_.emplace_back(soldier);
		}
	}

	// As long as there are soldiers, they should make sure this function is run regularly,
	// so we can do the conquering here.
	// TODO(tothxa): We should not conquer if a friend owns (some of) the area and still has
	//               influence.
	if (!did_conquer_ && enemy_buildings_.empty() && enemy_soldiers_.empty() && !soldiers_.empty()) {
		// Only conquer when at least one soldier got back to the base
		for (const OPtr<Soldier>& soldier : soldiers_) {
			if (map.calc_distance(soldier.get(game)->get_position(), get_position()) <=
			    kPortSpaceGeneralAreaRadius) {
				game.conquer_area(PlayerArea<Area<FCoords>>(
				   owner().player_number(), Area<FCoords>(get_position(), Soldier::kPortSpaceRadius)));
				did_conquer_ = true;
				break;
			}
		}
	}

	last_update_ = game.get_gametime();
}

constexpr uint8_t kCurrentPacketVersionInvasion = 1;

void NavalInvasionBase::Loader::load(FileRead& fr) {
	Bob::Loader::load(fr);
	for (size_t i = fr.unsigned_32(); i > 0; --i) {
		soldiers_.insert(fr.unsigned_32());
	}
}

void NavalInvasionBase::Loader::load_pointers() {
	Bob::Loader::load_pointers();

	NavalInvasionBase& invasion = get<NavalInvasionBase>();
	for (Serial s : soldiers_) {
		invasion.add_soldier(&mol().get<Soldier>(s));
	}
}

// TODO(tothxa): Save and load did_conquer_ to avoid double conquering.
//               but compatibility with v1.3 will be messy...

Bob::Loader* NavalInvasionBase::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionInvasion) {
			loader->init(egbase, mol, *new NavalInvasionBase);
			loader->load(fr);
		} else {
			throw UnhandledVersionError(
			   "NavalInvasionBase", packet_version, kCurrentPacketVersionInvasion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading naval invasion base: %s", e.what());
	}

	return loader.release();
}

void NavalInvasionBase::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderNavalInvasionBase);
	fw.unsigned_8(kCurrentPacketVersionInvasion);

	Bob::save(egbase, mos, fw);

	fw.unsigned_32(soldiers_.size());
	for (const auto& soldier : soldiers_) {
		fw.unsigned_32(mos.get_object_file_index(*soldier.get(egbase)));
	}
}

}  // namespace Widelands
