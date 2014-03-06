/*
 * Copyright (C) 2002-2004, 2008-2011 by the Widelands Development Team
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

#ifndef IMMOVABLE_PROGRAM_H
#define IMMOVABLE_PROGRAM_H

#include <cstring>
#include <string>

#include <boost/noncopyable.hpp>

/*
 * Implementation is in immovable.cc
 */

#include "logic/buildcost.h"
#include "logic/immovable.h"

struct Profile;

namespace Widelands {

/// Ordered sequence of actions (at least 1). Has a name.
struct ImmovableProgram {

	/// Can be executed on an Immovable.
	struct Action : boost::noncopyable {
		virtual ~Action();
		virtual void execute(Game &, Immovable &) const = 0;
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
		ActAnimate
			(char * parameters, Immovable_Descr &,
			 const std::string & directory, Profile &);
		virtual void execute(Game &, Immovable &) const override;
		uint32_t animation() const {return m_id;}
	private:
		uint32_t m_id;
		Duration m_duration;
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
		ActTransform
			(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const override;
	private:
		std::string type_name;
		bool        bob;
		bool        tribe;
		uint8_t     probability;
	};

	/// Like ActTransform but the probability is determined by the suitability.
	struct ActGrow : public Action {
		ActGrow
			(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const override;
	private:
		std::string type_name;
		bool        tribe;
	};

	struct ActRemove : public Action {
		ActRemove(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const override;
	private:
		uint8_t probability;
	};

	struct ActSeed : public Action {
		ActSeed(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const override;
	private:
		std::string type_name;
		bool        tribe;
		uint8_t probability;
	};

	/// Plays a soundFX.
	///
	/// Parameter syntax:
	///    parameters ::= soundFX [priority]
	/// Parameter semantics:
	///    directory:
	///       The directory of the productionsite.
	///    soundFX:
	///       The filename of an soundFX (relative to the productionsite's
	///       directory).
	///    priority:
	///       An integer. If omitted, 127 is used.
	///
	/// Plays the specified soundFX with the specified priority. Whether the
	/// soundFX is actually played is determined by the sound handler.
	struct ActPlayFX : public Action {
		ActPlayFX(const std::string & directory, char * parameters, const Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const override;
	private:
		std::string name;
		uint8_t     priority;
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
	struct ActConstruction : public Action {
		ActConstruction(char * parameters, Immovable_Descr &, const std::string & directory, Profile &);
		virtual void execute(Game &, Immovable &) const override;

		Duration buildtime() const {return m_buildtime;}
		Duration decaytime() const {return m_decaytime;}

	private:
		uint32_t m_animid;
		Duration m_buildtime;
		Duration m_decaytime;
	};

	/// Create a program with a single action.
	ImmovableProgram(char const * const _name, Action * const action)
		: m_name(_name)
	{
		m_actions.push_back(action);
	}

	/// Create a program by parsing a conf-file section.
	ImmovableProgram
		(const std::string    & directory,
		 Profile              &,
		 const std::string    & name,
		 Immovable_Descr      &);
	~ImmovableProgram() {
		container_iterate_const(Actions, m_actions, i)
			delete *i.current;
	}

	const std::string & name() const {return m_name;}
	size_t size() const {return m_actions.size();}
	const Action & operator[](size_t const idx) const {
		assert(idx < m_actions.size());
		return *m_actions[idx];
	}

	typedef std::vector<Action *> Actions;
	const Actions & actions() const {return m_actions;}

private:
	std::string m_name;
	Actions     m_actions;
};

struct ImmovableActionData {
	ImmovableActionData() {}
	virtual ~ImmovableActionData() {}

	virtual const char * name() const = 0;
	virtual void save(FileWrite & fw, Immovable & imm) = 0;

	static ImmovableActionData * load(FileRead & fr, Immovable & imm, const std::string & name);
};

}

#endif
