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

#include "logic/map_objects/world/critter.h"

#include <cmath>
#include <memory>

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/field.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/critter_program.h"
#include "scripting/lua_table.h"

namespace Widelands {

CritterProgram::CritterProgram(const std::string& program_name, const LuaTable& actions_table)
   : MapObjectProgram(program_name) {
	for (const std::string& line : actions_table.array_entries<std::string>()) {
		try {
			const std::vector<std::string> cmd(split_string(line, " \t\r\n"));
			if (cmd.empty()) {
				continue;
			}

			CritterAction act;
			if (cmd[0] == "remove") {
				if (cmd.size() != 1) {
					throw wexception("Usage: remove");
				}
				act.function = &Critter::run_remove;
			} else {
				throw wexception("unknown command type \"%s\"", cmd[0].c_str());
			}

			actions_.push_back(act);
		} catch (const std::exception& e) {
			throw wexception("Line '%s': %s", line.c_str(), e.what());
		}
	}
}

/*
===================================================

    PROGRAMS

==================================================
*/

/*
==============================

remove

Remove this critter

==============================
*/
bool Critter::run_remove(Game& game, State& state, const CritterAction& /* action */) {
	++state.ivar1;
	// Bye, bye cruel world
	schedule_destroy(game);
	return true;
}

/*
===========================================================================

 CRITTER BOB DESCR

===========================================================================
*/

CritterDescr::CritterDescr(const std::string& init_descname,
                           const LuaTable& table,
                           const std::vector<std::string>& attribs)
   : BobDescr(init_descname, MapObjectType::CRITTER, MapObjectDescr::OwnerType::kWorld, table),
     size_(table.get_int("size")),
     carnivore_(table.has_key("carnivore") && table.get_bool("carnivore")),
     appetite_(0),
     reproduction_rate_(table.get_int("reproduction_rate")) {
	assign_directional_animation(&walk_anims_, "walk");

	add_attributes(attribs);

	if (size_ < 1 || size_ > 10) {
		throw GameDataError(
		   "Critter %s: size_ %u out of range 1..10", name().c_str(), static_cast<unsigned>(size_));
	}
	if (reproduction_rate_ > 100 || reproduction_rate_ < 1) {
		throw GameDataError("Critter %s: reproduction_rate_ %u out of range 1..100", name().c_str(),
		                    static_cast<unsigned>(reproduction_rate_));
	}
	if (table.has_key("herbivore")) {
		for (const std::string& a : table.get_table("herbivore")->array_entries<std::string>()) {
			food_plants_.insert(get_attribute_id(a, true));
		}
		if (food_plants_.empty()) {
			throw GameDataError("Critter %s: 'herbivore' specified but empty", name().c_str());
		}
	}
	if (table.has_key("appetite")) {
		if (!is_carnivore() && !is_herbivore()) {
			throw GameDataError(
			   "Critter %s is neither herbivore but carnivore but has an appetite", name().c_str());
		}
		appetite_ = table.get_int("appetite");
		if (appetite_ > 100) {
			throw GameDataError("Critter %s: appetite %u out of range 0..100", name().c_str(),
			                    static_cast<unsigned>(appetite_));
		}
	} else if (is_carnivore() || is_herbivore()) {
		throw GameDataError(
		   "Critter %s is a herbivore or carnivore but has no appetite", name().c_str());
	}

	std::unique_ptr<LuaTable> programs = table.get_table("programs");
	for (const std::string& program_name : programs->keys<std::string>()) {
		try {
			programs_[program_name] = std::unique_ptr<CritterProgram>(
			   new CritterProgram(program_name, *programs->get_table(program_name)));
		} catch (const std::exception& e) {
			throw wexception("Parse error in program %s: %s", program_name.c_str(), e.what());
		}
	}
}

bool CritterDescr::is_swimming() const {
	const static uint32_t swimming_attribute = get_attribute_id("swimming", true);
	return has_attribute(swimming_attribute);
}

/*
===============
Get a program from the workers description.
===============
*/
CritterProgram const* CritterDescr::get_program(const std::string& program_name) const {
	Programs::const_iterator const it = programs_.find(program_name);
	if (it == programs_.end()) {
		throw wexception("%s has no program '%s'", name().c_str(), program_name.c_str());
	}
	return it->second.get();
}

uint32_t CritterDescr::movecaps() const {
	return is_swimming() ? MOVECAPS_SWIM : MOVECAPS_WALK;
}

/*
==============================================================================

class Critter

==============================================================================
*/

//
// Implementation
//

Critter::Critter(const CritterDescr& critter_descr) : Bob(critter_descr), creation_time_(0) {
}

bool Critter::init(EditorGameBase& egbase) {
	if (egbase.is_game()) {
		// in editor, assume t0 as creation time so bobs don't die of old age right away when the
		// actual game starts
		creation_time_ = egbase.get_gametime();
	}
	return Bob::init(egbase);
}

/*
==============================

PROGRAM task

Follow the steps of a configuration-defined program.
ivar1 is the next action to be performed.
ivar2 is used to store description indices selected by plant
objvar1 is used to store objects found by findobject
coords is used to store target coordinates found by findspace

==============================
*/

Bob::Task const Critter::taskProgram = {
   "program", static_cast<Bob::Ptr>(&Critter::program_update), nullptr, nullptr, true};

void Critter::start_task_program(Game& game, const std::string& programname) {
	push_task(game, taskProgram);
	State& state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}

void Critter::program_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		molog(game.get_gametime(), "[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	for (;;) {
		const CritterProgram& program = dynamic_cast<const CritterProgram&>(*state.program);

		if (state.ivar1 >= program.get_size()) {
			return pop_task(game);
		}

		const CritterAction& action = program[state.ivar1];

		if ((this->*(action.function))(game, state, action)) {
			return;
		}
	}
}

/*
==============================

ROAM task

Simply roam the map

==============================
*/

constexpr double kPi = 3.1415926535897932384626433832795029;

Bob::Task const Critter::taskRoam = {
   "roam", static_cast<Bob::Ptr>(&Critter::roam_update), nullptr, nullptr, true};

constexpr uint16_t kCritterMaxIdleTime = 2000;
constexpr uint16_t kMealtime = 2500;
constexpr uint32_t kMinReproductionAge = 5 * 60 * 1000;
constexpr uint32_t kMinCritterLifetime = 20 * 60 * 1000;
constexpr uint32_t kMaxCritterLifetime = 10 * 60 * 60 * 1000;

void Critter::roam_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		return pop_task(game);
	}

	// alternately move and idle
	uint32_t idle_time_min = 1000;
	uint32_t idle_time_rnd = kCritterMaxIdleTime;

	if (state.ivar1) {
		state.ivar1 = 0;
		// lots of magic numbers for reasonable weighting of various nearby animals
		uint32_t roaming_dist = 24;
		if (game.map().find_bobs(
		       game, Area<FCoords>(get_position(), 2), nullptr, FindBobByName(descr().name())) > 4) {
			// Too crowded here! Let's go further away
			roaming_dist += 3 * game.map().find_bobs(game, Area<FCoords>(get_position(), 5), nullptr,
			                                         FindBobByName(descr().name()));
		}
		// move only a bit away from other animals of the same class
		roaming_dist += 4 * game.map().find_bobs(game, Area<FCoords>(get_position(), 3), nullptr,
		                                         FindCritterByClass(descr()));
		if (!descr().is_carnivore()) {
			roaming_dist += 6 * game.map().find_bobs(
			                       game, Area<FCoords>(get_position(), 4), nullptr, FindCarnivores());
		}
		roaming_dist /= 12;
		assert(roaming_dist >= 2);
		// the further we want to go, the harder we try to go somewhere
		for (; roaming_dist > 0; --roaming_dist) {
			if (start_task_movepath(game, game.random_location(get_position(), roaming_dist),
			                        roaming_dist, descr().get_walk_anims())) {
				return;
			}
		}
		idle_time_min = 1;
		idle_time_rnd = 1000;
	}
	state.ivar1 = 1;

	const uint32_t age = (game.get_gametime() - creation_time_).get();
	const uint32_t reproduction_rate = descr().get_reproduction_rate();

	{  // chance to die
		const uint32_t nearby_critters1 =
		   game.map().find_bobs(game, Area<FCoords>(get_position(), 2), nullptr, FindCritter());
		const uint32_t nearby_critters2 = game.map().find_bobs(
		   game, Area<FCoords>(get_position(), 7), nullptr, FindBobByName(descr().name()));
		assert(nearby_critters1);  // at least we are here
		assert(nearby_critters2);
		const double r = reproduction_rate * reproduction_rate / 10000000.0;
		const double i1 = r / kMinCritterLifetime;
		const double i2 = (1 - r) / (kMaxCritterLifetime - kMinCritterLifetime);
		assert(i1 * kMaxCritterLifetime <= 1.0);
		assert(i2 >= i1);
		const double d = i1 * age + std::max(0.0, (i2 - i1) * age * reproduction_rate /
		                                             (kMaxCritterLifetime - kMinCritterLifetime));
		assert(d >= 0.0);
		assert(d <= 1.0);
		if (game.logic_rand() % kMinCritterLifetime <
		    d * kMinCritterLifetime * nearby_critters1 * nearby_critters1 * nearby_critters2) {
			// :(
			molog(game.get_gametime(), "Goodbye world :(\n");
			return schedule_destroy(game);
		}
	}

	std::vector<Critter*> candidates_for_eating;
	bool can_eat_immovable = false;
	if (descr().is_herbivore() && get_position().field->get_immovable()) {
		for (uint32_t a : descr().food_plants()) {
			if (get_position().field->get_immovable()->descr().has_attribute(a)) {
				can_eat_immovable = true;
				break;
			}
		}
	}
	bool other_herbivores_on_field = false;
	size_t mating_partners = 0;
	int32_t n_th_bob_on_field = 0;
	bool foundme = false;
	std::vector<Critter*> all_critters_on_field;
	all_critters_on_field.push_back(this);  // not caught by the following loop
	for (Bob* b = get_position().field->get_first_bob(); b; b = b->get_next_bob()) {
		assert(b);
		if (b == this) {
			assert(!foundme);
			foundme = true;
			continue;
		}
		if (!foundme) {
			++n_th_bob_on_field;
		}
		if (descr().name() == b->descr().name()) {
			++mating_partners;
		}
		if (upcast(Critter, c, b)) {
			all_critters_on_field.push_back(c);
			other_herbivores_on_field |= c->descr().is_herbivore();
		}
	}
	assert(foundme);
	if (descr().is_carnivore()) {
		// only hunt other carnivores if there are no herbivores here
		for (Critter* c : all_critters_on_field) {
			if (descr().name() != c->descr().name()) {
				if (!c->descr().is_carnivore() || !other_herbivores_on_field) {
					candidates_for_eating.push_back(c);
				}
			}
		}
	}
	size_t nr_candidates_for_eating = candidates_for_eating.size();
	while (can_eat_immovable || nr_candidates_for_eating > 0) {
		const size_t idx =
		   game.logic_rand() % (nr_candidates_for_eating + (can_eat_immovable ? 1 : 0));
		if (game.logic_rand() % 100 < descr().get_appetite()) {
			// yum yum yum
			bool skipped = false;
			if (idx == nr_candidates_for_eating) {
				// refers to the plant on our field
				assert(can_eat_immovable);
				upcast(Immovable, imm, get_position().field->get_immovable());
				assert(imm);
				molog(game.get_gametime(), "Yummy, I love a %s...\n", imm->descr().name().c_str());
				imm->delay_growth(Duration(descr().get_size() * 2000));
			} else {
				Critter* food = candidates_for_eating[idx];
				molog(game.get_gametime(), "Yummy, I love a %s...\n", food->descr().name().c_str());
				// find hunting partners
				int32_t attacker_strength = 0;
				int32_t defender_strength = 0;
				if (food->descr().is_carnivore()) {
					// 1vs1
					attacker_strength = descr().get_size();
					defender_strength = food->descr().get_size();
				} else {
					// ffa
					for (Critter* c : all_critters_on_field) {
						if (c != this && (c == food || c->descr().is_herbivore())) {
							defender_strength += c->descr().get_size();
						} else if (c->descr().is_carnivore()) {
							attacker_strength += c->descr().get_size();
						}
					}
				}
				assert(attacker_strength > 0);
				assert(defender_strength > 0);
				const int32_t S =
				   attacker_strength - defender_strength;  // weighted combined strength *difference*
				const int32_t N = defender_strength * defender_strength;  // definition of N: the
				                                                          // resulting chance is 1 if
				                                                          // and only if S >= N
				const double weighted_success_chance =
				   S <= -N ? 0.0 :
				   S >= N  ? 1.0 :
                         -(std::log(S * S + 1) - 2 * S * std::atan(S) - kPi * S -
                          std::log(N * N + 1) + N * (2 * std::atan(N) - kPi)) /
				               (2 * N * kPi);
				molog(game.get_gametime(),
				      "    *** [total strength %d] vs [prey %d] *** success chance %lf\n", S,
				      defender_strength, weighted_success_chance);
				assert(weighted_success_chance >= 0.0);
				assert(weighted_success_chance <= 1.0);
				if (game.logic_rand() % (N != 0 ? N : 1) < weighted_success_chance * N) {
					molog(game.get_gametime(), "    SUCCESS :)\n");
					food->remove(game);
				} else {
					molog(game.get_gametime(), "    failed :(\n");
					skipped = true;
				}
			}
			if (!skipped) {
				return start_task_idle(game, descr().get_animation("eating", this), kMealtime);
			}
		}
		if (idx == nr_candidates_for_eating) {
			assert(can_eat_immovable);
			can_eat_immovable = false;
		} else {
			candidates_for_eating.erase(candidates_for_eating.begin() + idx);
			--nr_candidates_for_eating;
		}
	}
	// no food sadly, but perhaps another animal to make cute little fox cubs with…?
	if (age > kMinReproductionAge && game.logic_rand() % 100 < mating_partners * reproduction_rate) {
		// A potential partner is interested in us. Now politely ask the game's birth control system
		// for permission.
		FindBobByName functor(descr().name());
		const size_t population_size_2 =
		   game.map().find_bobs(game, Area<FCoords>(get_position(), 2), nullptr, functor);
		const size_t population_size_5 =
		   game.map().find_bobs(game, Area<FCoords>(get_position(), 5), nullptr, functor);
		const size_t population_size_9 =
		   game.map().find_bobs(game, Area<FCoords>(get_position(), 9), nullptr, functor);
		assert(population_size_2 >=
		       1 + mating_partners);  // at least we and our partners should be there
		assert(population_size_5 >= population_size_2);
		assert(population_size_9 >= population_size_5);
		const size_t weighted_population =
		   (3 * population_size_2 + 2 * population_size_5 + population_size_9) / 3;
		assert(weighted_population >= population_size_2);
		if ((game.logic_rand() % (reproduction_rate * reproduction_rate)) *
		       std::exp2(weighted_population - mating_partners - 1) <
		    reproduction_rate * reproduction_rate * weighted_population) {
			molog(game.get_gametime(), "A cute little %s cub :)\n", descr().name().c_str());
			game.create_critter(get_position(), descr().name());
		}
	}
	return start_task_idle(game, descr().get_animation("idle", this),
	                       idle_time_min + game.logic_rand() % idle_time_rnd,
	                       Vector2i(n_th_bob_on_field * std::cos(n_th_bob_on_field),
	                                n_th_bob_on_field * std::sin(n_th_bob_on_field)));
}

void Critter::init_auto_task(Game& game) {
	push_task(game, taskRoam);
	top_state().ivar1 = 0;
}

Bob& CritterDescr::create_object() const {
	return *new Critter(*this);
}

/*
==============================

Load / Save implementation

==============================
*/

// We need to bump this packet version every time we rename a critter, so that the world legacy
// lookup table will work.
constexpr uint8_t kCurrentPacketVersion = 4;

const Bob::Task* Critter::Loader::get_task(const std::string& name) {
	if (name == "roam") {
		return &taskRoam;
	}
	if (name == "program") {
		return &taskProgram;
	}
	return Bob::Loader::get_task(name);
}

const MapObjectProgram* Critter::Loader::get_program(const std::string& name) {
	const Critter& critter = get<Critter>();
	return critter.descr().get_program(name);
}

MapObject::Loader* Critter::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const packet_version = fr.unsigned_8();
		// Supporting older versions for map loading
		if (1 <= packet_version && packet_version <= kCurrentPacketVersion) {
			if (packet_version < 4) {
				fr.c_string();  // Consume obsolete owner type (world/tribes)
			}

			const CritterDescr* descr = egbase.descriptions().get_critter_descr(
			   egbase.mutable_descriptions()->load_critter(fr.c_string()));

			Critter& critter = dynamic_cast<Critter&>(descr->create_object());
			critter.creation_time_ = packet_version >= 3 ? Time(fr) : Time(0);
			loader->init(egbase, mol, critter);
			loader->load(fr);
		} else {
			throw UnhandledVersionError("Critter", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading critter: %s", e.what());
	}

	return loader.release();
}

void Critter::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderCritter);
	fw.unsigned_8(kCurrentPacketVersion);

	fw.c_string(descr().name());
	creation_time_.save(fw);

	Bob::save(egbase, mos, fw);
}
}  // namespace Widelands
