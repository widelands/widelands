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

#ifndef WL_LOGIC_MAP_OBJECTS_IMMOVABLE_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_IMMOVABLE_PROGRAM_H

#include <memory>

#include "logic/map_objects/immovable.h"
#include "logic/map_objects/map_object_program.h"

namespace Widelands {

/// Ordered sequence of actions (at least 1). Has a name.
struct ImmovableProgram : public MapObjectProgram {

	/// Can be executed on an Immovable.
	struct Action {
		Action() = default;
		virtual ~Action();
		virtual void execute(Game&, Immovable&) const = 0;

	private:
		DISALLOW_COPY_AND_ASSIGN(Action);
	};

	/// Runs an animation.
	///
	/// Parameter syntax:
	///    parameters ::= animation duration
	/// Parameter semantics:
	///    animation:
	///       The name of an animation (defined in the immovable type).
	///    duration:
	///       A natural integer.
	///
	/// Starts the specified animation for the immovable. Blocks the execution
	/// of the program for the specified duration. (The duration does not have
	/// to equal the length of the animation. It will loop around. The animation
	/// will not be stopped by this command. It will run until another animation
	/// is started.)
	struct ActAnimate : public Action {
		ActAnimate(const std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;
		uint32_t animation() const {
			return parameters.animation;
		}

	private:
		AnimationParameters parameters;
	};

	/// Transforms the immovable into another immovable or into a bob
	///
	/// Parameter syntax
	///    parameters ::= {probability} {bob|immovable} world|tribe:name
	/// Parameter semantics:
	///    probability: (defaults to 0 -- i.e. always)
	///       The probability (out of 255) for replacing the immovable with
	///       a new one; if the probability is 0 (i.e. the default), then the
	///       transformation always happens
	///    bob|immovable: (defaults to immovable)
	///       whether we'll be replaced by a bob or by an immovable
	///    world|tribe:
	///       whether the other object is taken from the world or from
	///       the owner's tribe
	///    name:
	///       name of the replacement object
	struct ActTransform : public Action {
		ActTransform(std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name;
		bool bob;
		uint8_t probability;
	};

	/// Like ActTransform but the probability is determined by the suitability.
	struct ActGrow : public Action {
		ActGrow(std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name;
	};

	struct ActRemove : public Action {
		ActRemove(std::vector<std::string>& arguments);
		void execute(Game&, Immovable&) const override;

	private:
		uint8_t probability;
	};

	struct ActSeed : public Action {
		ActSeed(std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name;
		uint8_t probability;
	};

	/// Plays a sound effect.
	///
	/// Parameter syntax:
	///    parameters ::= directory sound [priority]
	/// Parameter semantics:
	///    path:
	///       The directory of the sound files, relative to the datadir, followed
	///       by the base filename of a sound effect (relative to the directory).
	///    priority:
	///       An integer. If omitted, 127 is used.
	///
	/// Plays the specified sound effect with the specified priority. Whether the
	/// sound effect is actually played is determined by the sound handler.
	struct ActPlaySound : public Action {
		ActPlaySound(const std::vector<std::string>& arguments);
		void execute(Game&, Immovable&) const override;

	private:
		PlaySoundParameters parameters;
	};

	/**
	 * Puts the immovable into construction mode.
	 *
	 * Parameter syntax:
	 *    parameters ::= animation build-time decay-time
	 * Parameter semantics:
	 *    animation:
	 *       The basic animation to be displayed during construction.
	 *    build-time:
	 *       Time for a single building step.
	 *    decay-time:
	 *       Time until construction decays one step if no progress has been made.
	 */
	struct ActConstruct : public Action {
		ActConstruct(std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

		Duration buildtime() const {
			return buildtime_;
		}
		Duration decaytime() const {
			return decaytime_;
		}

	private:
		std::string animation_name_;
		Duration buildtime_;
		Duration decaytime_;
	};

	/// Create a program with a single action.
	ImmovableProgram(const std::string& init_name, std::unique_ptr<Action> action);

	/// Create an immovable program from a number of lines.
	ImmovableProgram(const std::string& init_name,
	                 const std::vector<std::string>& lines,
	                 const ImmovableDescr& immovable);

	~ImmovableProgram() {
	}

	size_t size() const {
		return actions_.size();
	}
	const Action& operator[](size_t const idx) const {
		assert(idx < actions_.size());
		return *actions_[idx];
	}

private:
	std::vector<std::unique_ptr<Action>> actions_;
};

struct ImmovableActionData {
	ImmovableActionData() {
	}
	virtual ~ImmovableActionData() {
	}

	virtual const char* name() const = 0;
	virtual void save(FileWrite& fw, Immovable& imm) const = 0;

	static ImmovableActionData* load(FileRead& fr, Immovable& imm, const std::string& name);
};

struct ActConstructData : ImmovableActionData {
	const char* name() const override;
	void save(FileWrite& fw, Immovable& imm) const override;
	static ActConstructData* load(FileRead& fr, Immovable& imm);

	Buildcost delivered;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_IMMOVABLE_PROGRAM_H
