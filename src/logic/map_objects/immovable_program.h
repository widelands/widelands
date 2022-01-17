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
	class Action {
	public:
		Action() = default;
		virtual ~Action() = default;
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
	class ActAnimate : public Action {
	public:
		ActAnimate(const std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;
		uint32_t animation() const {
			return parameters.animation;
		}

	private:
		AnimationParameters parameters;
	};

	/// Transforms the immovable into another immovable or into a bob.
	/// For parameters, see scripting documentation.
	class ActTransform : public Action {
	public:
		ActTransform(std::vector<std::string>& arguments, ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name_;
		bool bob_;
		unsigned probability_;
	};

	/// Like ActTransform but the probability is determined by the suitability.
	class ActGrow : public Action {
	public:
		ActGrow(std::vector<std::string>& arguments, ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name_;
	};

	class ActRemove : public Action {
	public:
		ActRemove(std::vector<std::string>& arguments, const ImmovableDescr& descr);
		void execute(Game&, Immovable&) const override;

	private:
		unsigned probability_;
	};

	class ActSeed : public Action {
	public:
		ActSeed(std::vector<std::string>& arguments, const ImmovableDescr&);
		void execute(Game&, Immovable&) const override;

	private:
		std::string type_name_;
		unsigned probability_;
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
	class ActPlaySound : public Action {
	public:
		ActPlaySound(const std::vector<std::string>& arguments, const ImmovableDescr& descr);
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
	class ActConstruct : public Action {
	public:
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
	                 ImmovableDescr& immovable);

	~ImmovableProgram() override {
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

	static ImmovableActionData* load(FileRead& fr, const Immovable& imm, const std::string& name);
};

struct ActConstructData : ImmovableActionData {
	const char* name() const override;
	void save(FileWrite& fw, Immovable& imm) const override;
	static ActConstructData* load(FileRead& fr, const Immovable& imm);

	Buildcost delivered;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_IMMOVABLE_PROGRAM_H
