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

#include "logic/map_objects/immovable_program.h"

#include <memory>

#include "base/log.h"
#include "base/math.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/terrain_affinity.h"
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
``main``, this program will be started as the main program on creation. Immovables without such a
program will simply display their 'idle' animation indefinitely.

.. note:: The main program used to be called ``program``, which has been deprecated.

Programs are defined as Lua tables. Each program must be declared as a subtable in the immovable's
Lua table called ``programs`` and have a unique table key. The entries in a program's subtable are
the ``actions`` to execute, like this:

.. code-block:: lua

   programs = {
      main = {
         "animate=idle 1550000",
         "transform=deadtree4 chance:5.13%",
         "seed=alder_summer_sapling proximity:70.31%",
      },
      fall = {
         "transform=",
      },
   },

.. highlight:: default

For general information about the format, see :ref:`map_object_programs_syntax`.

Available actions are:

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
                                   ImmovableDescr& immovable)
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
				actions_.push_back(
				   std::unique_ptr<Action>(new ActGrow(parseinput.arguments, immovable)));
			} else if (parseinput.name == "remove") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActRemove(parseinput.arguments, immovable)));
			} else if (parseinput.name == "seed") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActSeed(parseinput.arguments, immovable)));
			} else if (parseinput.name == "playsound") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActPlaySound(parseinput.arguments, immovable)));
			} else if (parseinput.name == "construct") {
				actions_.push_back(
				   std::unique_ptr<Action>(new ActConstruct(parseinput.arguments, immovable)));
			} else {
				throw GameDataError(
				   "Unknown command '%s' in line '%s'", parseinput.name.c_str(), line.c_str());
			}
		} catch (const std::exception& e) {
			throw GameDataError("Error reading line '%s': %s", line.c_str(), e.what());
		}
	}
	if (actions_.empty()) {
		throw GameDataError("No actions found");
	}
}

/* RST

animate
-------
Runs an animation. See :ref:`map_object_programs_animate`.
*/
ImmovableProgram::ActAnimate::ActAnimate(const std::vector<std::string>& arguments,
                                         const ImmovableDescr& descr)
   : parameters(MapObjectProgram::parse_act_animate(arguments, descr, true)) {
}

/// Use convolution to make the animation time a random variable with binomial
/// distribution and the configured time as the expected value.
void ImmovableProgram::ActAnimate::execute(Game& game, Immovable& immovable) const {
	immovable.start_animation(game, parameters.animation);
	immovable.program_step(
	   game, Duration(parameters.duration.get() ? 1 + game.logic_rand() % parameters.duration.get() +
	                                                 game.logic_rand() % parameters.duration.get() :
                                                 0));
}

/* RST

playsound
---------
Plays a sound effect. See :ref:`map_object_programs_playsound`.
*/
ImmovableProgram::ActPlaySound::ActPlaySound(const std::vector<std::string>& arguments,
                                             const ImmovableDescr& descr)
   : parameters(MapObjectProgram::parse_act_play_sound(arguments, descr)) {
}

/**
 * Send request to the g_sound_handler to play a certain sound effect.
 * Whether the effect actually gets played is decided by the sound server itself.
 */
void ImmovableProgram::ActPlaySound::execute(Game& game, Immovable& immovable) const {
	Notifications::publish(NoteSound(SoundType::kAmbient, parameters.fx, immovable.get_position(),
	                                 parameters.priority, parameters.allow_multiple));
	immovable.program_step(game);
}

/* RST

transform
---------

.. function:: transform=[bob:]\<name\> [chance:\<percent\>]

   :arg string \<name\>: The name of the map object to turn into.
      If the ``bob:<name>`` flag is given, the transformation target is a bob;
      otherwise it is an immovable. Currently, only ships are supported as bobs.

   :arg percent chance: The :ref:`map_object_programs_datatypes_percent` chance that the
      transformation will be performed. The game will generate a random number and the
      transformation will be performed if and only if this number is less than ``chance``.
      If ``chance:<percent>`` is omitted, the transformation will always be performed.

   Deletes this immovable and instantly replaces it with a different immovable or a bob. If
   ``chance`` is specified, there's a probability that the transformation will be skipped.
   When the transformation succeeds, no further program steps will be executed, because this object
   will be gone. Example:

.. code-block:: lua

     main = {
         "animate=idle duration:25m50s",
         "transform=deadtree3 chance:9.37%",
         -- This line will be skipped if the removal succeeds
         "seed=spruce_summer_sapling proximity:78.12%",
      },
*/
ImmovableProgram::ActTransform::ActTransform(std::vector<std::string>& arguments,
                                             ImmovableDescr& descr) {
	if (arguments.empty() || arguments.size() > 2) {
		throw GameDataError("Usage: [bob:]name [chance:<percent>]");
	}
	try {
		bob_ = false;
		probability_ = 0;

		for (const std::string& argument : arguments) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (!item.second.empty()) {
				if (item.first == "bob") {
					bob_ = true;
					type_name_ = item.second;
				} else if (item.first == "chance") {
					probability_ = math::read_percent_to_int(item.second);
				} else {
					throw GameDataError(
					   "Unknown argument '%s'. Usage: [bob:]name [chance:<percent>]", argument.c_str());
				}
			} else if (item.first == "bob") {
				// TODO(GunChleoc): Savegame compatibility, remove this argument option after v1.0
				bob_ = true;
				log_warn("%s: Deprecated 'bob' in 'transform' program, use 'bob:<name>' instead.\n",
				         descr.name().c_str());
			} else if (item.first[0] >= '0' && item.first[0] <= '9') {
				// TODO(GunChleoc): Savegame compatibility, remove this argument option after v1.0
				log_warn("%s: Deprecated chance in 'transform' program, use 'chance:<percent>' "
				         "instead.\n",
				         descr.name().c_str());
				probability_ = (read_positive(item.first, 254) * math::k100PercentAsInt) / 256;
			} else {
				type_name_ = argument;
			}
		}
		if (type_name_ == descr.name()) {
			throw GameDataError("illegal transformation to the same type");
		}

		// This ensures that the object we're transforming to is loaded and known. It does not
		// ensure that it's an appropriate immovable/bob.
		Notifications::publish(
		   NoteMapObjectDescription(type_name_, NoteMapObjectDescription::LoadType::kObject));

		// Register target at ImmovableDescr
		descr.becomes_.insert(
		   std::make_pair(bob_ ? MapObjectType::BOB : MapObjectType::IMMOVABLE, type_name_));
		if (!bob_) {
			descr.register_immovable_relation(descr.name(), type_name_);
		}
	} catch (const std::exception& e) {
		throw GameDataError("transform: %s", e.what());
	}
}

void ImmovableProgram::ActTransform::execute(Game& game, Immovable& immovable) const {
	if (immovable.apply_growth_delay(game)) {
		return;
	}
	if (probability_ == 0 || game.logic_rand() % math::k100PercentAsInt < probability_) {
		Player* player = immovable.get_owner();
		Coords const c = immovable.get_position();
		std::set<PlayerNumber> mfr = immovable.get_marked_for_removal();

		immovable.remove(game);  //  Now immovable is a dangling reference!

		if (bob_) {
			game.create_ship(c, type_name_, player);
		} else {
			Immovable& i = game.create_immovable_with_name(
			   c, type_name_, player, nullptr /* former_building_descr */);
			for (const PlayerNumber& p : mfr) {
				i.set_marked_for_removal(p, true);
			}
		}
	} else {
		immovable.program_step(game);
	}
}

/* RST
grow
----
.. function:: grow=\<immovable_name\>

   :arg string \<immovable_name\>: The name of the immovable to turn into.

   Deletes the immovable (preventing subsequent program steps from being called) and replaces it
   with an immovable of the given name. The chance that this program step succeeds depends on how
   well this immovable's terrain affinity matches the terrains it is growing on. If the growth
   fails, the next program step is triggered. This command may be used only for immovables with a
   terrain affinity. Example:

.. code-block:: lua

      main = {
         "animate=idle duration:57s500ms",
         "remove=chance:8.2%",
         "grow=alder_summer_pole",
      },
*/
ImmovableProgram::ActGrow::ActGrow(std::vector<std::string>& arguments, ImmovableDescr& descr) {
	if (arguments.size() != 1) {
		throw GameDataError("Usage: grow=<immovable name>");
	}
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'grow', but has no terrain_affinity entry.", descr.name().c_str());
	}
	if (type_name_ == descr.name()) {
		throw GameDataError("illegal growth to the same type");
	}

	type_name_ = arguments.front();
	// This ensures that the object we're transforming to is loaded and known. It does not ensure
	// that it's an appropriate immovable.
	Notifications::publish(
	   NoteMapObjectDescription(type_name_, NoteMapObjectDescription::LoadType::kObject));

	// Register target at ImmovableDescr
	descr.becomes_.insert(std::make_pair(MapObjectType::IMMOVABLE, type_name_));
	descr.register_immovable_relation(descr.name(), type_name_);
}

void ImmovableProgram::ActGrow::execute(Game& game, Immovable& immovable) const {
	if (immovable.apply_growth_delay(game)) {
		return;
	}

	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if ((game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	    probability_to_grow(descr.terrain_affinity(), f, map, game.descriptions().terrains())) {
		Player* owner = immovable.get_owner();
		std::set<PlayerNumber> mfr = immovable.get_marked_for_removal();

		immovable.remove(game);  //  Now immovable is a dangling reference!
		Immovable& i =
		   game.create_immovable_with_name(f, type_name_, owner, nullptr /* former_building_descr */);
		for (const PlayerNumber& p : mfr) {
			i.set_marked_for_removal(p, true);
		}
	} else {
		immovable.program_step(game);
	}
}

/* RST

remove
------

.. function:: remove=[chance:\<percent\>]

   :arg percent chance: The :ref:`map_object_programs_datatypes_percent` chance that the immovable
      will be removed. The game will generate a random number and the immovable will be removed if
      and only if this number is less than ``chance``. If ``chance:<percent>`` is omitted, the
      immovable will always be removed.

   Remove this immovable. If ``chance`` is specified, there's a probability that the removal will be
   skipped. When the removal succeeds, no further program steps will be executed, because this
   object will be gone. Examples:

.. code-block:: lua

      main = {
         "animate=idle duration:55s",
         "remove=chance:16.41%",
         "grow=spruce_summer_pole", -- This line will be skipped if the removal succeeds
      },
      fall = {
         "remove=", -- This object will always be removed when 'fall' is called
      },
*/
ImmovableProgram::ActRemove::ActRemove(std::vector<std::string>& arguments,
                                       const ImmovableDescr& descr) {
	if (arguments.size() > 1) {
		throw GameDataError("Usage: remove=[chance:<percent>]");
	}
	if (arguments.empty()) {
		probability_ = 0;
	} else {
		const std::pair<std::string, std::string> item = read_key_value_pair(arguments.front(), ':');
		if (item.first == "chance") {
			probability_ = math::read_percent_to_int(item.second);
		} else if (item.first[0] >= '0' && item.first[0] <= '9') {
			// TODO(GunChleoc): Savegame compatibility, remove this argument option after v1.0
			log_warn("%s: Deprecated chance in 'remove' program, use 'chance:<percent>' instead.\n",
			         descr.name().c_str());
			probability_ = (read_positive(item.first, 254) * math::k100PercentAsInt) / 256;
		} else {
			throw GameDataError(
			   "Unknown argument '%s'. Usage: [chance:<percent>]", arguments.front().c_str());
		}
	}
}

void ImmovableProgram::ActRemove::execute(Game& game, Immovable& immovable) const {
	if (probability_ == 0 || game.logic_rand() % math::k100PercentAsInt < probability_) {
		immovable.remove(game);  //  Now immovable is a dangling reference!
	} else {
		immovable.program_step(game);
	}
}

/* RST
seed
----

.. function:: seed=\<immovable_name\> proximity:\<percent\>

   :arg string \<immovable_name\>: The name of the immovable to create.

   :arg percent proximity: The radius within which the immovable will seed is not limited and
      is determined by repeatedly generating a random number and comparing it with the proximity
      :ref:`map_object_programs_datatypes_percent` chance until the comparison fails. The higher
      this number, the closer the new immovable will be seeded.

   Finds a random location nearby and creates a new immovable with the given name there with a
   chance depending on *this* immovable's terrain affinity. The chance that such a location will be
   searched for in a higher radius is influenced by the ``proximity`` parameter. Note that this
   program step will consider only *one* random location, and it will only seed there if the terrain
   is well suited. This command may be used only for immovables with a terrain affinity. Example:

.. code-block:: lua

     main = {
         "animate=idle duration:20s",
         "remove=chance:11.72%",
         -- Select a location with a chance of 19.53% in the base radius,
         -- then expand the radius and try again with a chance of 19.53%.
         -- Repeat until a location has been selected, then plant an
         -- 'umbrella_red_wasteland_sapling' if the terrain affinity check
         -- for this immovable succeeds at the selected location.
         "seed=umbrella_red_wasteland_sapling proximity:19.53%",
         "animate=idle duration:20s",
         "remove=chance:7.81%",
         "grow=umbrella_red_wasteland_old",
     },
*/
ImmovableProgram::ActSeed::ActSeed(std::vector<std::string>& arguments,
                                   const ImmovableDescr& descr) {
	if (arguments.size() != 2) {
		throw GameDataError("Usage: seed=<immovable_name> proximity:<percent>");
	}
	if (!descr.has_terrain_affinity()) {
		throw GameDataError(
		   "Immovable %s can 'seed', but has no terrain_affinity entry.", descr.name().c_str());
	}

	if (read_key_value_pair(arguments.at(1), ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this argument option after v1.0
		log_warn("'seed' program without parameter names is deprecated, please use "
		         "'seed=<immovable_name> proximity:<percent>' in %s\n",
		         descr.name().c_str());
		type_name_ = arguments.front();
		probability_ = (read_positive(arguments.at(1), 254) * math::k100PercentAsInt) / 256;
	} else {
		for (const std::string& argument : arguments) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
			if (item.first == "proximity") {
				probability_ = math::read_percent_to_int(item.second);
			} else if (item.second.empty()) {
				// TODO(GunChleoc): It would be nice to check if target exists, but we can't guarantee
				// the load order. Maybe in postload() one day.
				type_name_ = item.first;
			} else {
				throw GameDataError(
				   "Unknown parameter '%s'. Usage: seed=<immovable_name> proximity:<percent>",
				   item.first.c_str());
			}
		}
	}

	// This ensures that the object we're transforming to is loaded and known. It does not ensure
	// that it's an appropriate immovable.
	Notifications::publish(
	   NoteMapObjectDescription(type_name_, NoteMapObjectDescription::LoadType::kObject));
}

void ImmovableProgram::ActSeed::execute(Game& game, Immovable& immovable) const {
	const Map& map = game.map();
	FCoords const f = map.get_fcoords(immovable.get_position());
	const ImmovableDescr& descr = immovable.descr();

	if ((game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
	    probability_to_grow(descr.terrain_affinity(), f, map, game.descriptions().terrains())) {
		// Seed a new tree.
		MapFringeRegion<> mr(map, Area<>(f, 0));
		uint32_t fringe_size = 0;
		do {
			mr.extend(map);
			fringe_size += 6;
		} while (game.logic_rand() % math::k100PercentAsInt < probability_);

		for (uint32_t n = game.logic_rand() % fringe_size; n; --n) {
			mr.advance(map);
		}

		const FCoords new_location = map.get_fcoords(mr.location());
		if (!new_location.field->get_immovable() &&
		    (new_location.field->nodecaps() & MOVECAPS_WALK) &&
		    (game.logic_rand() % TerrainAffinity::kPrecisionFactor) <
		       probability_to_grow(
		          descr.terrain_affinity(), new_location, map, game.descriptions().terrains())) {
			game.create_immovable_with_name(
			   mr.location(), type_name_, nullptr /* owner */, nullptr /* former_building_descr */);
		}
	}

	immovable.program_step(game);
}

/* RST

construct
---------
.. function:: construct=\<animation_name\> duration:\<duration\> decay_after:\<duration\>

   :arg string animation_name: The animation to display while the immovable is being constructed.
   :arg duration duration: The :ref:`map_object_programs_datatypes_duration` of each construction
      step for visualising the construction progress. Used only in drawing code.
   :arg duration decay_after: When no construction material has been delivered for this
      :ref:`map_object_programs_datatypes_duration`, the construction progress starts to gradually
      reverse.

   Blocks execution of subsequent programs until enough wares have been delivered to this immovable
   by a worker. The wares to deliver are specified in the immovable's ``buildcost`` table which is
   mandatory for immovables using the ``construct`` command. If no wares are being delivered for a
   while, the progress gradually starts to reverse, increasing the number of wares left to deliver.
   If the immovable keeps decaying, it will eventually be removed. Example:

.. code-block:: lua

      main = {
         "construct=idle duration:5s decay_after:3m30s",
         "transform=bob:frisians_ship",
      }
*/
ImmovableProgram::ActConstruct::ActConstruct(std::vector<std::string>& arguments,
                                             const ImmovableDescr& descr) {
	if (arguments.size() != 3) {
		throw GameDataError(
		   "Usage: construct=<animation_name> duration:<duration> decay_after:<duration>");
	}
	if (read_key_value_pair(arguments[1], ':').second.empty()) {
		// TODO(GunChleoc): Compatibility, remove this argument option after v1.0
		log_warn("Old-style syntax found for 'construct' program in %s, use "
		         "construct=<animation_name> duration:<duration> decay_after:<duration> instead.\n",
		         descr.name().c_str());
		animation_name_ = arguments[0];

		buildtime_ = Duration(read_positive(arguments[1]));
		decaytime_ = Duration(read_positive(arguments[2]));
	} else {
		for (const std::string& argument : arguments) {
			const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');

			if (item.first == "duration") {
				buildtime_ = read_duration(item.second, descr);
			} else if (item.first == "decay_after") {
				decaytime_ = read_duration(item.second, descr);
			} else if (item.second.empty()) {
				animation_name_ = item.first;
			} else {
				throw GameDataError("Unknown predicate '%s'. Usage: construct=<animation_name> "
				                    "duration:<duration> decay_after:<duration>.",
				                    argument.c_str());
			}
		}
	}

	if (!descr.is_animation_known(animation_name_)) {
		throw GameDataError("Unknown animation '%s'", animation_name_.c_str());
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

ActConstructData* ActConstructData::load(FileRead& fr, const Immovable& imm) {
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
		for (auto& item : d->delivered) {
			if (randdecay < item.second) {
				item.second--;
				break;
			}

			randdecay -= item.second;
		}

		imm.anim_construction_done_ = d->delivered.total();
	}

	imm.program_step_ = imm.schedule_act(g, decaytime_);
}

ImmovableActionData*
ImmovableActionData::load(FileRead& fr, const Immovable& imm, const std::string& name) {
	if (name == "construct") {
		return ActConstructData::load(fr, imm);
	}
	log_err("ImmovableActionData::load: type %s not known", name.c_str());
	return nullptr;
}
}  // namespace Widelands
