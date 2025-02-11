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
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

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
NavalInvasionBase::create(EditorGameBase& egbase, Soldier& soldier, const Coords& pos) {
	NavalInvasionBase* invasion = new NavalInvasionBase();
	invasion->set_owner(soldier.get_owner());
	invasion->set_position(egbase, pos);

	invasion->add_soldier(egbase, &soldier);
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
		if (state->coords != get_position()) {
			molog(game.get_gametime(), "Soldier %u at different invasion location %dx%d!",
			      soldier->serial(), state->coords.x, state->coords.y);
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
	egbase.unconquer_area(PlayerArea<Area<FCoords>>(
	   owner().player_number(), Area<FCoords>(get_position(), Soldier::kPortSpaceRadius)));

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

void NavalInvasionBase::add_soldier(EditorGameBase& egbase, Soldier* soldier) {
	assert(soldier != nullptr);

	if (soldiers_.empty()) {
		egbase.conquer_area(PlayerArea<Area<FCoords>>(
		   owner().player_number(), Area<FCoords>(get_position(), Soldier::kPortSpaceRadius)));
	}

	soldiers_.insert(soldier);
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
		invasion.add_soldier(egbase(), &mol().get<Soldier>(s));
	}
}

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
