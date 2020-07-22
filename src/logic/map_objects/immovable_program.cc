/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "logic/map_objects/immovable_program.h"

#include <memory>

#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapfringeregion.h"
#include "logic/player.h"
#include "sound/note_sound.h"

namespace Widelands {

ImmovableProgram::ImmovableProgram(const std::string& init_name, std::unique_ptr<Action> action)
   : MapObjectProgram(init_name) {
	actions_.push_back(std::move(action));
}

/* RST
.. _immovable_programs:

Immovable Programs
==================
Immovables can have programs that will be executed by the game engine. Programs are required to
allow workers to interact with an immovable (e.g. a tree will need a "fall" program to allow
woodcutters to remove the tree).

It is not mandatory for immovables to define programs. If the immovable defines a program named
``program``, this program will be started as the main program on creation. Immovables without such a
program will simply display their main animation indefinitely.

Programs are defined as Lua tables. Each program must be declared as a subtable in the immovable's
Lua table called ``programs`` and have a unique table key. The entries in a program's subtable are
the ``actions`` to execute, like this::

   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 success:5",
         "seed=alder_summer_sapling 180",
      },
      fall = {
         "transform=",
      },
   },

The different command types and the parameters that they take are explained below.

.. highlight:: default

Command Types
^^^^^^^^^^^^^
- `animate`_
- `transform`_
- `grow`_
- `remove`_
- `seed`_
- `construct`_
- `playsound`_
*/

ImmovableProgram::ImmovableProgram(const std::string& init_name,
                                   const std::vector<std::string>& lines,
                                   const ImmovableDescr& immovable)
   : MapObjectProgram(init_name) {
	for (const std::string& line : lines) {
		if (line.empty()) {
			throw GameDataError("Empty line");
		}
		try {
			ProgramParseInput parseinput = parse_program_string(line);

			if (parseinput.name == "animate") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActAnimate(parseinput.arguments, immovable)));
			} else if (parseinput.name == "transform") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActTransform(parseinput.arguments, immovable)));
			} else if (parseinput.name == "grow") {
				// TODO(GunChleoc): Savegame compatibility, remove after v1.0.
				log("WARNING: %s: 'grow' is deprecated, use 'transform' instead.\n",
				    immovable.name().c_str());
				actions_.push_back(
				   std::unique_ptr<Action>(new ActTransform(parseinput.arguments, immovable)));
			} else if (parseinput.name == "remove") {
				// TODO(GunChleoc): Savegame compatibility, remove after v1.0.
				log("WARNING: %s: 'remove' is deprecated, use 'transform=' instead.\n",
				    immovable.name().c_str());
				actions_.push_back(std::unique_ptr<Action>(new ActRemove(parseinput.arguments)));
			} else if (parseinput.name == "seed") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActSeed(parseinput.arguments, immovable)));
			} else if (parseinput.name == "playsound") {
				actions_.push_back(std::unique_ptr<Action>(new ActPlaySound(parseinput.arguments)));
			} else if (parseinput.name == "construct") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActConstruct(parseinput.arguments, immovable)));
			} else {
				throw GameDataError(
				   "Unknown command '%s' in line '%s'", parseinput.name.c_str(), line.c_str());
			}
		} catch (const GameDataError& e) {
			throw GameDataError("Error reading line '%s': %s", line.c_str(), e.what());
		}
	}
	if (actions_.empty()) {
		throw GameDataError("No actions found");
	}
}

ImmovableProgram::Action::~Action() {
}

/* RST

animate
-------
Runs an animation.

Parameter syntax::

  parameters ::= animation duration

Parameter semantics:

``animation``
    The name of an animation (defined in the immovable).
``duration``
    A natural integer specifying the duration in milliseconds.

Starts the specified animation for the immovable. Blocks the execution of the program for the
specified duration. (The duration does not have to equal the length of the animation. It will loop
around. The animation will not be stopped by this command. It will run until another animation is
started.)
*/
ImmovableProgram::ActAnimate::ActAnimate(const std::vector<std::string>& arguments,
                                         const ImmovableDescr& descr) {
	parameters = MapObjectProgram::parse_act_animate(arguments, descr, true);
}

/// Use convolution to make the animation time a random variable with binomial
/// distribution and the configured time as the expected value.
void ImmovableProgram::ActAnimate::execute(Game& game, Immovable& immovable) const {
	immovable.start_animation(game, parameters.animation);
	immovable.program_step(game, parameters.duration ? 1 + game.logic_rand() % parameters.duration +
	                                                      game.logic_rand() % parameters.duration :
	                                                   0);
}

/* RST

playsound
---------
Plays a sound effect.

Parameter syntax::

  parameters ::= soundFX [priority]

Parameter semantics:

``filepath``
    The path/base_filename of a soundFX (relative to the data directory).
``priority``
    An integer. If omitted, 127 is used.

Plays the specified soundFX with the specified priority. Whether the soundFX is actually played is
determined by the sound handler.
*/
ImmovableProgram::ActPlaySound::ActPlaySound(const std::vector<std::string>& arguments) {
	parameters = MapObjectProgram::parse_act_play_sound(arguments, kFxPriorityAllowMultiple - 1);
}

/**
 * Send request to the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played is decided by the sound server itself.
 */
void ImmovableProgram::ActPlaySound::execute(Game& game, Immovable& immovable) const {
	Notifications::publish(
	   NoteSound(SoundType::kAmbient, parameters.fx, immovable.get_position(), parameters.priority));
	immovable.program_step(game);
}

/* RST

transform
---------
Replace this immovable with something else or remove it.

Parameter syntax::

  parameters ::= [[bob:]<name>] [remove:<chance>] [success:<chance>]

Parameter semantics:

``[bob:]<name>``
    The name of the immovable to turn into. If the ``bob`` flag is given, the transformation target
    is a bob; otherwise it is an immovable. Currently, only ships are supported as bobs.
``remove:<chance>``
    A natural integer in [1,254] defining the chance that this immovable will be removed from the
    map. If the removal is performed, no other transformation will take place.
``success:<chance>``
    A natural integer in [1,254] defining the chance that the transformation will be performed. The
    game will generate a random number between 0 and 255 and the program step succeeds if and only
    if this number is less than ``chance``. Otherwise, the next program step is triggered. If
    ``success:<chance>`` is omitted, the transformation will calculate the probability from the
    terrain affinity if available; otherwise, it will always succeed.

Deletes this immovable and instantly replaces it with a different immovable or a bob. If no
parameters are given, the immovable is removed and no other transformation will take place. If the
immovable has terrain affinity, or ``success`` is specified, there's a probability that the
transformation will be skipped.
*/
ImmovableProgram::ActTransform::ActTransform(std::vector<std::string>& arguments,
                                             const ImmovableDescr& descr) {
	if (arguments.size() > 3) {
		throw GameDataError("Usage: [[bob:]name] [remove:chance] [success:chance]");
	}
	try {
		bob_ = false;
		removal_wanted_ = false;
		transform_probability_ = 0;
		removal_probability_ = 0;

		if (arguments.empty()) {
			removal_wanted_ = true;
		}

		for (const std::string& argument : arguments) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (!item.second.empty()) {
				if (item.first == "bob") {
					bob_ = true;
					// TODO(GunChleoc): If would be nice to check if target exists, but we can't
					// guarantee the load order. Maybe in postload() one day.
					type_name_ = item.second;
				} else if (item.first == "remove") {
					removal_wanted_ = true;
					removal_probability_ = read_positive(item.second, 254);
				} else if (item.first == "success") {
					transform_probability_ = read_positive(item.second, 254);
				} else {
					throw GameDataError(
					   "Unknown argument '%s'. Usage: [[bob:]name] [remove:chance] [success:chance]",
					   argument.c_str());
				}
			} else if (item.first == "bob") {
				// TODO(GunChleoc): Savegame compatibility, remove this argument option after v1.0
				bob_ = true;
				log("WARNING: %s: Deprecated 'bob' in 'transform' program, use 'bob:<name>' instead.\n",
				    descr.name().c_str());
			} else if (item.first[0] >= '0' && item.first[0] <= '9') {
				// TODO(GunChleoc): Savegame compatibility, remove this argument option after v1.0
				log("WARNING: %s: Deprecated chance in 'transform' program, use 'success:<number>' "
				    "instead.\n",
				    descr.name().c_str());
				transform_probability_ = read_positive(item.first, 254);
			} else {
				// TODO(GunChleoc): If would be nice to check if target exists, but we can't guarantee
				// the load order. Maybe in postload() one day.
				type_name_ = item.first;
			}
		}
		if (type_name_ == descr.name()) {
			throw GameDataError("illegal transformation to the same type");
		}
		if (transform_probability_ > 0 && type_name_.empty()) {
			throw GameDataError("'success' parameter without immovable/bob name");
		}
	} catch (const WException& e) {
		throw GameDataError("transform: %s", e.what());
	}
}

void ImmovableProgram::ActTransform::execute(Game& game, Immovable& immovable) const {
	assert(removal_wanted_ || !type_name_.empty());

	// Check whether we want to remove
	if (removal_wanted_ &&
	    (removal_probability_ == 0 || game.logic_rand() % 256 < removal_probability_)) {
		immovable.remove(game);  //  Now immovable is a dangling reference!
		return;
	}
	// For removal with chance but no transformation to new object
	if (type_name_.empty()) {
		return;
	}

	if (immovable.apply_growth_delay(game)) {
		return;
	}

	// Transform according to success if this was specified, and using any available terrain affinity
	// if not.
	const Map& map = game.map();
	const ImmovableDescr& descr = immovable.descr();
	Player* player = immovable.get_owner();
	FCoords const f = map.get_fcoords(immovable.get_position());
	MapObjectDescr::OwnerType owner_type = immovable.descr().owner_type();

	const bool will_transform =
	   descr.has_terrain_affinity() && transform_probability_ == 0 ?
	      (game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	         probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains()) :
	      transform_probability_ == 0 || game.logic_rand() % 256 < transform_probability_;

	if (will_transform) {
		immovable.remove(game);  //  Now immovable is a dangling reference!
		if (bob_) {
			game.create_ship(f, type_name_, player);
		} else {
			game.create_immovable_with_name(
			   f, type_name_, owner_type, player, nullptr /* former_building_descr */);
		}
	} else {
		immovable.program_step(game);
	}
}

/* RST

grow
----
** DEPRECATED** Use ``transform=<immovable_name>`` instead.
*/

/* RST

remove
------
** DEPRECATED** Use ``transform=remove:[<chance>]`` or ``transform=`` instead.
*/
// TODO(GunChleoc): Savegame compatibility, remove after v1.0
ImmovableProgram::ActRemove::ActRemove(std::vector<std::string>& arguments) {
	if (arguments.size() > 1) {
		throw GameDataError("Usage: remove=[<probability>]");
	}
	probability = arguments.empty() ? 0 : read_positive(arguments.front(), 254);
}

void ImmovableProgram::ActRemove::execute(Game& game, Immovable& immovable) const {
	if (probability == 0 || game.logic_rand() % 256 < probability) {
		immovable.remove(game);  //  Now immovable is a dangling reference!
	} else {
		immovable.program_step(game);
	}
}

/* RST

seed
----
Create a new immovable nearby with a chance depending on terrain affinity.

Parameter syntax::

  parameters ::= name factor

Parameter semantics:

``name``
    The name of the immovable to create.
``factor``
    A natural integer in [1,254]. The radius within which the immovable will seed is not limited and
    is determined by repeatedly generating a random number between 0 and 255 and comparing it with
    ``factor`` until the comparison fails.

Finds a random location nearby and creates a new immovable with the given name there with a chance
depending on *this* immovable's terrain affinity. The chance that such a location will be searched
for in a higher radius can be influenced. Note that this program step will consider only *one*
random location, and it will only seed there if the terrain is well suited. This command may be used
only for immovables with a terrain affinity.
*/
ImmovableProgram::ActSeed::ActSeed(std::vector<std::string>& arguments,
                                   const ImmovableDescr& descr) {
	if (arguments.size() != 2) {
		throw GameDataError("Usage: seed=<immovable name> <radius_range_factor>");
	}
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'seed', but has no terrain_affinity entry.", descr.name().c_str());
	}

	// TODO(GunChleoc): If would be nice to check if target exists, but we can't guarantee the load
	// order. Maybe in postload() one day.
	type_name = arguments.front();
	const int p = std::stoi(arguments[1]);
	if (p <= 0 || p >= 255) {
		throw GameDataError("Immovable %s: Seeding radius range factor %i out of range [1,254]",
		                    descr.name().c_str(), p);
	}
	probability = p;
}

void ImmovableProgram::ActSeed::execute(Game& game, Immovable& immovable) const {
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if ((game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	    probability_to_grow(descr.terrain_affinity(), f, map, game.world().terrains())) {
		// Seed a new tree.
		MapFringeRegion<> mr(map, Area<>(f, 0));
		uint32_t fringe_size = 0;
		do {
			mr.extend(map);
			fringe_size += 6;
		} while (game.logic_rand() % std::numeric_limits<uint8_t>::max() < probability);

		for (uint32_t n = game.logic_rand() % fringe_size; n; --n) {
			mr.advance(map);
		}

		const FCoords new_location = map.get_fcoords(mr.location());
		if (!new_location.field->get_immovable() &&
		    (new_location.field->nodecaps() & MOVECAPS_WALK) &&
		    (game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
		       probability_to_grow(
		          descr.terrain_affinity(), new_location, map, game.world().terrains())) {
			game.create_immovable_with_name(mr.location(), type_name, descr.owner_type(),
			                                nullptr /* owner */, nullptr /* former_building_descr */);
		}
	}

	immovable.program_step(game);
}

/* RST

construct
---------
Blocks execution until enough wares have been delivered to this immovable by a worker.

Parameter syntax::

  parameters ::= animation build decay

Parameter semantics:

``animation``
    The animation to display while the immovable is being constructed.
``build``
    The duration of each construction step in milliseconds for visualising the construction
    progress. Used only in drawing code.
``decay``
    When no construction material has been delivered for this many milliseconds, the construction
    progress starts to gradually reverse.

Blocks execution of subsequent programs until enough wares have been delivered to this immovable by
a worker. The wares to deliver are specified in the immovable's ``buildcost`` table which is
mandatory for immovables using the ``construct`` command. If no wares are being delivered for a
while, the progress gradually starts to reverse, increasing the number of wares left to deliver. If
the immovable keeps decaying, it will eventually be removed.
*/
ImmovableProgram::ActConstruct::ActConstruct(std::vector<std::string>& arguments,
                                             const ImmovableDescr& descr) {
	if (arguments.size() != 3) {
		throw GameDataError("Usage: construct=<animation> <build duration> <decay duration>");
	}
	try {
		animation_name_ = arguments[0];
		if (!descr.is_animation_known(animation_name_)) {
			throw GameDataError("Unknown animation '%s' in immovable program for immovable '%s'",
			                    animation_name_.c_str(), descr.name().c_str());
		}

		buildtime_ = read_positive(arguments[1]);
		decaytime_ = read_positive(arguments[2]);
	} catch (const WException& e) {
		throw GameDataError("construct: %s", e.what());
	}
}

constexpr uint8_t kCurrentPacketVersionConstructionData = 1;

const char* ActConstructData::name() const {
	return "construct";
}
void ActConstructData::save(FileWrite& fw, Immovable& imm) const {
	fw.unsigned_8(kCurrentPacketVersionConstructionData);
	delivered.save(fw, imm.get_owner()->tribe());
}

ActConstructData* ActConstructData::load(FileRead& fr, Immovable& imm) {
	ActConstructData* d = new ActConstructData;

	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionConstructionData) {
			d->delivered.load(fr, imm.get_owner()->tribe());
		} else {
			throw UnhandledVersionError(
			   "ActConstructData", packet_version, kCurrentPacketVersionConstructionData);
		}
	} catch (const WException& e) {
		delete d;
		d = nullptr;
		throw GameDataError("ActConstructData: %s", e.what());
	}

	return d;
}

void ImmovableProgram::ActConstruct::execute(Game& g, Immovable& imm) const {
	ActConstructData* d = imm.get_action_data<ActConstructData>();
	if (!d) {
		// First execution
		d = new ActConstructData;
		imm.set_action_data(d);

		imm.start_animation(g, imm.descr().get_animation(animation_name_, &imm));
		imm.anim_construction_total_ = imm.descr().buildcost().total();
	} else {
		// Perhaps we are called due to the construction timeout of the last construction step
		Buildcost remaining;
		imm.construct_remaining_buildcost(g, &remaining);
		if (remaining.empty()) {
			imm.program_step(g);
			return;
		}

		// Otherwise, this is a decay timeout
		uint32_t totaldelivered = 0;
		for (const auto& addme : d->delivered) {
			totaldelivered += addme.second;
		}

		if (!totaldelivered) {
			imm.remove(g);
			return;
		}

		uint32_t randdecay = g.logic_rand() % totaldelivered;
		for (Buildcost::iterator it = d->delivered.begin(); it != d->delivered.end(); ++it) {
			if (randdecay < it->second) {
				it->second--;
				break;
			}

			randdecay -= it->second;
		}

		imm.anim_construction_done_ = d->delivered.total();
	}

	imm.program_step_ = imm.schedule_act(g, decaytime_);
}

ImmovableActionData*
ImmovableActionData::load(FileRead& fr, Immovable& imm, const std::string& name) {
	if (name == "construct") {
		return ActConstructData::load(fr, imm);
	} else {
		log("ImmovableActionData::load: type %s not known", name.c_str());
		return nullptr;
	}
}
}  // namespace Widelands
