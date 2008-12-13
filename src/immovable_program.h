/*
 * Copyright (C) 2002-2004, 2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef IMMOVABLE_PROGRAM_H
#define IMMOVABLE_PROGRAM_H

/*
 * Implementation is in immovable.cc
 */

#include "immovable.h"

#include <string>
#include <cstring>

class Profile;

namespace Widelands {

/// Ordered sequence of actions (at least 1). Has a name.
struct ImmovableProgram {

	/// Can be executed on an Immovable.
	struct Action {
		virtual ~Action();
		virtual void execute(Game &, Immovable &) const = 0;
	private:
		Action & operator= (Action const &);
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
			 std::string const & directory, Profile &);
		virtual void execute(Game &, Immovable &) const;
		uint32_t animation() const {return m_id;}
	private:
		uint32_t m_id;
		Duration m_duration;
	};

	struct ActTransform : public Action {
		ActTransform
			(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const;
	private:
		std::string type_name;
		bool        tribe;
		uint8_t     probability;
	};

	struct ActRemove : public Action {
		ActRemove(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const;
	private:
		uint8_t probability;
	};

	struct ActSeed : public Action {
		ActSeed(char * parameters, Immovable_Descr &);
		virtual void execute(Game &, Immovable &) const;
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
	///    soundFX:
	///       The filename of an soundFX (relative to the productionsite's
	///       directory).
	///    priority:
	///       An integer. If omitted, 127 is used.
	///
	/// Plays the specified soundFX with the specified priority. Whether the
	/// soundFX is actually played is determined by the sound handler.
	struct ActPlayFX : public Action {
		ActPlayFX(char * parameters, Immovable_Descr const &);
		virtual void execute(Game &, Immovable &) const;
	private:
		std::string name;
		uint8_t     priority;
	};


	/// Create a program with a single action.
	ImmovableProgram(char const * const _name, Action * const action)
		: m_name(_name)
	{
		m_actions.push_back(action);
	}

	/// Create a program by parsing a conf-file section.
	ImmovableProgram
		(std::string    const & directory,
		 Profile              &,
		 std::string    const & name,
		 Immovable_Descr      &);
	~ImmovableProgram() {
		container_iterate_const(Actions, m_actions, i)
			delete *i.current;
	}

	std::string const & name() const {return m_name;}
	size_t size() const {return m_actions.size();}
	Action const & operator[](size_t const idx) const {
		assert(idx < m_actions.size());
		return *m_actions[idx];
	}

	typedef std::vector<Action *> Actions;
	Actions const & actions() const {return m_actions;}

private:
	std::string m_name;
	Actions     m_actions;
};

};

#endif
