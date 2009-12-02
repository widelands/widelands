/*
 * Copyright (C) 2002-2004, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef PRODUCTION_PROGRAM_H
#define PRODUCTION_PROGRAM_H

#include "program_result.h"
#include "tattribute.h"
#include "widelands.h"
#include "writeHTML.h"

#include "io/filewrite.h"
#include "log.h"

#include "container_iterate.h"

#include <cassert>
#include <stdint.h>
#include <cstring>
#include <set>
#include <string>
#include <vector>


struct EncodeData;
struct Profile;

namespace Widelands {

struct Game;
struct ProductionSite_Descr;
struct ProductionSite;
struct Tribe_Descr;

/// Ordered sequence of actions (at least 1). Has a name.
struct ProductionProgram {

	/// Can be executed on a ProductionSite.
	struct Action {
		virtual ~Action();
		virtual void execute(Game &, ProductionSite &) const = 0;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML(::FileWrite &, ProductionSite_Descr const &) const
			= 0;
#endif
	private:
		Action & operator= (Action const &);
	};

	/// Returns from the program.
	///
	/// Parameter syntax:
	///    parameters         ::= return_value [condition_part]
	///    return_value       ::= Failed | Completed | Skipped
	///    Failed             ::= "failed"
	///    Completed          ::= "completed"
	///    Skipped            ::= "skipped"
	///    condition_part     ::= when_condition | unless_conition
	///    when_condition     ::= "when" condition {"and" condition}
	///    unless_condition   ::= "unless" condition {"or" condition}
	///    condition ::= negation | economy_condition | workers_condition
	///    negation           ::= "not" condition
	///    economy_condition  ::= "economy" economy_needs
	///    workers_condition  ::= "workers" workers_need_experience
	///    economy_needs      ::= "needs" ware_type
	///    workers_need_experience ::= "need experience"
	/// Parameter semantics:
	///    return_value:
	///       If return_value is Failed or Completed, the productionsite's
	///       statistics is updated accordingly. If return_value is Skipped, the
	///       statistics are not affected.
	///    condition:
	///       A boolean condition that can be evaluated to true or false.
	///    condition_part:
	///       If omitted, the return is unconditional.
	///    when_condition:
	///       This will cause the program to return when all conditions are
	///       true.
	///    unless_condition:
	///       This will cause the program to return unless some condition is
	///       true.
	///    ware_type:
	///       The name of a ware type (defined in the tribe). A ware type may
	///       only appear once in the command.
	///    economy_needs:
	///       The result of this condition depends on whether the economy that
	///       this productionsite belongs to needs a ware of the specified type.
	///       How this is determined is defined by the economy.
	///
	/// Aborts the execution of the program and sets a return value. Updates the
	/// productionsite's statistics depending on the return value.
	///
	/// Note: If the execution reaches the end of the program. the return value
	/// is implicitly set to Completed.
	struct ActReturn : public Action {
		ActReturn(char * parameters, ProductionSite_Descr const &);
		virtual ~ActReturn();
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif

		struct Condition {
			virtual ~Condition();
			virtual bool evaluate(ProductionSite const &) const = 0;
			virtual std::string description(Tribe_Descr const &) const
				= 0;
#ifdef WRITE_GAME_DATA_AS_HTML
			virtual void writeHTML
				(::FileWrite &, ProductionSite_Descr const &) const
				= 0;
#endif
		};
		static Condition * create_condition
			(char * & parameters, Tribe_Descr const &);
		struct Negation : public Condition {
			Negation
				(char * & parameters, Tribe_Descr const & tribe)
				: operand(create_condition(parameters, tribe))
			{}
			virtual ~Negation();
			virtual bool evaluate(ProductionSite const &) const;
			std::string description(Tribe_Descr const &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
				(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		private:
			Condition * const operand;
		};

		/// Tests whether the economy needs a ware of type ware_type.
		struct Economy_Needs : public Condition {
			Economy_Needs(char * & parameters, Tribe_Descr const &);
			virtual bool evaluate(ProductionSite const &) const;
			std::string description(Tribe_Descr const &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
				(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		private:
			Ware_Index ware_type;
		};

		/// Tests whether any of the workers at the site needs experience to
		/// become upgraded.
		struct Workers_Need_Experience : public Condition {
			virtual bool evaluate(ProductionSite const &) const;
			std::string description(Tribe_Descr const &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		};

		typedef std::vector<Condition *> Conditions;
		Program_Result m_result;
		bool       m_is_when; //  otherwise it is "unless"
		Conditions m_conditions;
	};


	/// Calls a program of the productionsite.
	///
	/// Parameter syntax:
	///    parameters                 ::= program [failure_handling_directive]
	///    failure_handling_directive ::= "on failure" failure_handling_method
	///    failure_handling_method    ::= Fail | Repeat | Ignore
	///    Fail                       ::= "fail"
	///    Repeat                     ::= "repeat"
	///    Ignore                     ::= "ignore"
	/// Parameter semantics:
	///    program:
	///       The name of a program defined in the productionsite.
	///    failure_handling_method:
	///       Specifies how to handle a failure of the called program.
	///       * If failure_handling_method is Fail, the command fails (with the
	///         same effect as executing "return=failed").
	///       * If failure_handling_method is "repeat", the command is repeated.
	///       * If failure_handling_method is "ignore", the failure is ignored
	///         (the program is continued).
	///    failure_handling_directive:
	///       If omitted, the value Ignore is used for failure_handling_method.
	struct ActCall : public Action {
		ActCall(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		ProductionProgram             * m_program;
		Program_Failure_Handling_Method m_failure_handling_method;
	};

	/// Calls a program of the productionsite's main worker.
	///
	/// Parameter syntax:
	///    parameters ::= program
	/// Parameter semantics:
	///    program:
	///       The name of a program defined in the productionsite's main worker.
	struct ActWorker : public Action {
		ActWorker
			(char * parameters, ProductionSite_Descr &,
			 std::string const & production_program_name);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		std::string const & program() const {return m_program;}
	private:
		std::string m_program;
	};

	/// Does nothing.
	///
	/// Parameter syntax:
	///    parameters ::= duration
	/// Parameter semantics:
	///    duration:
	///       A natural integer. If 0, the result from the most recent command
	///       that returned a value is used.
	///
	/// Blocks the execution of the program for the specified duration.
	struct ActSleep : public Action {
		ActSleep(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		Duration m_duration;
	};

	/// Runs an animation.
	///
	/// Parameter syntax:
	///    parameters ::= animation duration
	/// Parameter semantics:
	///    animation:
	///       The name of an animation (defined in the productionsite type).
	///    duration:
	///       A natural integer. If 0, the result from the most recent command
	///       that returned a value is used.
	///
	/// Starts the specified animation for the productionsite. Blocks the
	/// execution of the program for the specified duration. (The duration does
	/// not have to equal the length of the animation. It will loop around. The
	/// animation will not be stopped by this command. It will run until another
	/// animation is started.)
	struct ActAnimate : public Action {
		ActAnimate
			(char * parameters, ProductionSite_Descr &,
			 std::string const & directory, Profile &, EncodeData const *);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		uint32_t m_id;
		Duration m_duration;
	};

	/// Consumes wares from the input storages.
	///
	/// Parameter syntax:
	///    parameters ::= group {group}
	///    group      ::= ware_type{,ware_type}[:count]
	/// Parameter semantics:
	///    ware_type:
	///       The name of a ware type (defined in the tribe).
	///    count:
	///       A positive integer. If omitted, the value 1 is used.
	///
	/// For each group, the number of wares specified in count is consumed. The
	/// consumed wares may be of any type in the group.
	///
	/// If there are not enough wares in the input storages, the command fails
	/// (with the same effect as executing "return=failed"). Then no wares will
	/// be consumed.
	///
	/// Selecting which ware types to consume for a group so that the whole
	/// command succeeds is a constraint satisfaction problem. The
	/// implementation does not implement an exhaustive search for a solution to
	/// it. It is just a greedy algorithm which gives up instead of
	/// backtracking. Therefore The command may fail even if there is a
	/// solution.

	/// However it may be possible to help the algorithm by ordering the groups
	/// carefully. Suppose that the input storage has the wares a:1, b:1 and a
	/// consume command has the parameters "a,b:1 a:1". The algorithm tries to
	/// consume its input wares in order. First it consumes wares of type a. It
	/// starts with the first group and consumes 1 (the group becomes
	/// satisfied). Then it proceeds with the second group, but there are no a's
	/// left to consume. Since there is no other ware type that can satisfy the
	/// group, the command will fail. If the groups are reordered so that the
	/// parameters become "a:1 a,b:1", it will work. When the algorithm consumes
	/// wares of type a, it will consume 1 for the first group. When it proceeds
	/// with the second group, it will not have any a's left. Then it will go on
	/// and consume wares of type b. it will consume 1 for the second group
	/// (which becomes satisfied) and the command succeeds.
	///
	/// \note It is not possible to reorder ware types within a group. "a,b" is
	/// equivalent to "b,a" because in the internal representation the ware
	/// types of a group are sorted. FIXME change this!
	struct ActConsume : public Action {
		ActConsume(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		typedef std::vector<std::pair<std::set<Ware_Index>, uint8_t> > Groups;
		Groups const & groups() const {return m_groups;}
	private:
		Groups m_groups;
	};

	/// Produces wares.
	///
	/// Parameter syntax:
	///    parameters ::= group {group}
	///    group      ::= ware_type[:count]
	/// Parameter semantics:
	///    ware_type:
	///       The name of a ware type (defined in the tribe). A ware type may
	///       only appear once in the command.
	///    count:
	///       A positive integer. If omitted, the value 1 is used.
	///
	/// For each group, the number of wares specified in count is produced. The
	/// produced wares are of the type specified in the group. How the produced
	/// wares are handled is defined by the productionsite.
	struct ActProduce : public Action {
		ActProduce(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
		typedef std::vector<std::pair<Ware_Index, uint8_t> > Items;
		Items const & items() const {return m_items;}
	private:
		Items m_items;
	};

	struct ActMine : public Action {
		ActMine
			(char * parameters, ProductionSite_Descr &,
			 std::string const & production_program_name);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		Resource_Index m_resource;
		uint8_t        m_distance;
		uint8_t        m_max;
		uint8_t        m_chance;
	};

	struct ActCheck_Soldier : public Action {
		ActCheck_Soldier(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		tAttribute attribute;
		uint8_t level;
	};

	struct ActTrain : public Action {
		ActTrain(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		tAttribute attribute;
		uint8_t level;
		uint8_t target_level;
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
		ActPlayFX(char * parameters, ProductionSite_Descr const &);
		virtual void execute(Game &, ProductionSite &) const;
#ifdef WRITE_GAME_DATA_AS_HTML
		virtual void writeHTML
			(::FileWrite &, ProductionSite_Descr const &) const;
#endif
	private:
		std::string name;
		uint8_t     priority;
	};


	ProductionProgram
		(std::string    const & directory,
		 Profile              &,
		 std::string    const & name,
		 char           const * descname,
		 ProductionSite_Descr *,
		 EncodeData     const *);
	~ProductionProgram() {
		container_iterate_const(Actions, m_actions, i)
			delete *i.current;
	}

	std::string const & name() const {return m_name;}
	std::string const & descname() const {return m_descname;}
	int32_t get_size() const {return m_actions.size();}
	Action const & operator[](size_t const idx) const {
		assert(idx < m_actions.size());
		return *m_actions[idx];
	}

	typedef std::vector<Action *> Actions;
	Actions const & actions() const {return m_actions;}

#ifdef WRITE_GAME_DATA_AS_HTML
	void writeHTML(::FileWrite &, ProductionSite_Descr const &) const;
#endif

private:
	std::string m_name;
	std::string m_descname;
	Actions     m_actions;
};

}

#endif
