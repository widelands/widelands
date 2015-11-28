/*
 * Copyright (C) 2002-2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_PROGRAM_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_PROGRAM_H

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <stdint.h>

#include "base/log.h"
#include "base/macros.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/program_result.h"
#include "logic/map_objects/tribes/training_attribute.h"
#include "logic/widelands.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Game;
class ImmovableDescr;
class ProductionSiteDescr;
class ProductionSite;
class TribeDescr;
class Worker;
class World;

/// Ordered sequence of actions (at least 1). Has a name.
struct ProductionProgram {

	/// Can be executed on a ProductionSite.
	struct Action {
		Action() = default;
		virtual ~Action();
		virtual void execute(Game &, ProductionSite &) const = 0;

		/**
		 * Called when the given worker is looking for work from the building,
		 * and none of the default actions apply.
		 *
		 * \return \c true iff the worker was assigned something to do,
		 *  \c false iff he should just idle.
		 */
		virtual bool get_building_work(Game &, ProductionSite &, Worker &) const;

		/**
		 * Called when the given worker returns from building work with
		 * a failed status.
		 */
		virtual void building_work_failed(Game &, ProductionSite &, Worker &) const;

	private:
		DISALLOW_COPY_AND_ASSIGN(Action);
	};

	/// A group of ware types with a count.
	using WareTypeGroup = std::pair<std::set<DescriptionIndex>, uint8_t>;

	/// Parse a group of ware types followed by an optional count and terminated
	/// by a space or null. Example: "fish,meat:2".
	static void parse_ware_type_group
		(char            * & parameters,
		 WareTypeGroup   & group,
		 const Tribes& tribes,
		 const BillOfMaterials  & inputs);

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
	///    site_condition     ::= "site" site_has
	///    workers_condition  ::= "workers" workers_need_experience
	///    economy_needs      ::= "needs" (ware_type|worker_type)
	///    site_has           ::= "has" group
	///    group              ::= ware_type{,ware_type}[:count]
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
	///       this productionsite belongs to needs a ware/worker of the
	///       specified type. How this is determined is defined by the economy.
	///
	/// Aborts the execution of the program and sets a return value. Updates the
	/// productionsite's statistics depending on the return value.
	///
	/// Note: If the execution reaches the end of the program. the return value
	/// is implicitly set to Completed.
	struct ActReturn : public Action {
		ActReturn(char* parameters, const ProductionSiteDescr&, const Tribes& tribes);
		virtual ~ActReturn();
		void execute(Game &, ProductionSite &) const override;

		struct Condition {
			virtual ~Condition();
			virtual bool evaluate(const ProductionSite &) const = 0;
			virtual std::string description(const Tribes&) const = 0;
			virtual std::string description_negation(const Tribes&) const = 0;
		};
		static Condition * create_condition
			(char * & parameters, const ProductionSiteDescr&, const Tribes& tribes);
		struct Negation : public Condition {
			Negation
				(char * & parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
				: operand(create_condition(parameters, descr, tribes))
			{}
			virtual ~Negation();
			bool evaluate(const ProductionSite &) const override;
			// Just a dummy to satisfy the superclass interface. Do not use.
			std::string description(const Tribes&) const override;
			// Just a dummy to satisfy the superclass interface. Do not use.
			std::string description_negation(const Tribes&) const override;
		private:
			Condition * const operand;
		};

		/// Tests whether the economy needs a ware of type ware_type.
		struct EconomyNeedsWare : public Condition {
			EconomyNeedsWare(const DescriptionIndex& i) : ware_type(i) {}
			bool evaluate(const ProductionSite &) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;
		private:
			DescriptionIndex ware_type;
		};

		/// Tests whether the economy needs a worker of type worker_type.
		struct EconomyNeedsWorker : public Condition {
			EconomyNeedsWorker(const DescriptionIndex& i) : worker_type(i) {}
			bool evaluate(const ProductionSite &) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;
		private:
			DescriptionIndex worker_type;
		};

		/// Tests whether the site has the specified (or implied) number of
		/// wares, combining from any of the types specified, in its input
		/// queues.
		struct SiteHas : public Condition {
			SiteHas(char* & parameters, const ProductionSiteDescr&, const Tribes& tribes);
			bool evaluate(const ProductionSite &) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;
		private:
			WareTypeGroup group;
		};

		/// Tests whether any of the workers at the site needs experience to
		/// become upgraded.
		struct WorkersNeedExperience : public Condition {
			bool evaluate(const ProductionSite &) const override;
			std::string description(const Tribes&) const override;
			std::string description_negation(const Tribes&) const override;
		};

		using Conditions = std::vector<Condition *>;
		ProgramResult m_result;
		bool       m_is_when; //  otherwise it is "unless"
		Conditions m_conditions;
	};


	/// Calls a program of the productionsite.
	///
	/// Parameter syntax:
	///    parameters         ::= program {handling_directive}
	///    handling_directive ::= "on" Result handling_method
	///    Result             ::= "failure" | "completion" | "skip"
	///    handling_method    ::= Fail | Complete | Skip | Repeat
	///    Fail               ::= "fail"
	///    Ignore             ::= "ignore"
	///    Repeat             ::= "repeat"
	/// Parameter semantics:
	///    program:
	///       The name of a program defined in the productionsite.
	///    handling_directive:
	///       Only 1 handling_directive can be specified for each value of
	///       Result. If no handling_directive is specified for a particular
	///       value of Result, the program is continued when the call has
	///       returned that value.
	///    handling_method:
	///       Specifies how to handle the specified kind of result of the called
	///       program.
	///       * If handling_method for the result of the called program is Fail,
	///         the command fails (with the same effect as executing
	///         "return=failed").
	///       * If handling_method is Complete, the command completes the
	///         calling program (with the same effect as executing
	///         "return=completed").
	///       * If handling_method is Skip, the command skips the calling
	///         program (with the same effect as executing "return=skipped").
	///       * If handling_method is "repeat", the command is repeated.
	struct ActCall : public Action {
		ActCall(char* parameters, const ProductionSiteDescr&);
		void execute(Game &, ProductionSite &) const override;
	private:
		ProductionProgram             * m_program;
		ProgramResultHandlingMethod m_handling_methods[3];
	};

	/// Calls a program of the productionsite's main worker.
	///
	/// Parameter syntax:
	///    parameters ::= program
	/// Parameter semantics:
	///    program:
	///       The name of a program defined in the productionsite's main worker.
	struct ActWorker : public Action {
		ActWorker(char* parameters,
					 const std::string& production_program_name,
					 ProductionSiteDescr*, const Tribes& tribes);
		void execute(Game &, ProductionSite &) const override;
		bool get_building_work(Game &, ProductionSite &, Worker &) const override;
		void building_work_failed(Game &, ProductionSite &, Worker &) const override;
		const std::string & program() const {return m_program;}
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
		ActSleep(char * parameters);
		void execute(Game &, ProductionSite &) const override;
	private:
		Duration m_duration;
	};

	/// Checks whether the map has a certain feature enabled.
	///
	/// Parameter syntax:
	///    parameters ::= feature
	/// Parameter semantics:
	///    feature:
	///       The name of the feature that should be checked. Possible values are:
	///       * Seafaring : to check whether the map has at least two port build spaces
	///
	/// Ends the program if the feature is not enabled.
	struct ActCheckMap : public Action {
		ActCheckMap(char * parameters);
		void execute(Game &, ProductionSite &) const override;
	private:
		 enum {
			 SEAFARING = 1
		 };
		 uint8_t m_feature;
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
		ActAnimate(char* parameters, ProductionSiteDescr*);
		void execute(Game &, ProductionSite &) const override;
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
	/// types of a group are sorted.
	// TODO(unknown): change this!
	struct ActConsume : public Action {
		ActConsume(char* parameters, const ProductionSiteDescr&, const Tribes& tribes);
		void execute(Game &, ProductionSite &) const override;
		using Groups = std::vector<WareTypeGroup>;
		const Groups & groups() const {return m_groups;}
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
	/// For each group, the number of wares specified in count are produced. The
	/// produced wares are of the type specified in the group. How the produced
	/// wares are handled is defined by the productionsite.
	struct ActProduce : public Action {
		ActProduce(char* parameters, const ProductionSiteDescr&, const Tribes& tribes);
		void execute(Game &, ProductionSite &) const override;
		bool get_building_work(Game &, ProductionSite &, Worker &) const override;
		using Items = std::vector<std::pair<DescriptionIndex, uint8_t>>;
		const Items & items() const {return m_items;}
	private:
		Items m_items;
	};

	/// Recruits workers.
	///
	/// Parameter syntax:
	///    parameters ::= group {group}
	///    group      ::= worker_type[:count]
	/// Parameter semantics:
	///    ware_type:
	///       The name of a worker type (defined in the tribe). A worker type
	///       may only appear once in the command.
	///    count:
	///       A positive integer. If omitted, the value 1 is used.
	///
	/// For each group, the number of workers specified in count are recruited.
	/// The recruited workers are of the type specified in the group. How the
	/// recruited workers are handled is defined by the productionsite.
	struct ActRecruit : public Action {
		ActRecruit(char* parameters, const ProductionSiteDescr&, const Tribes& tribes);
		void execute(Game &, ProductionSite &) const override;
		bool get_building_work(Game &, ProductionSite &, Worker &) const override;
		using Items = std::vector<std::pair<DescriptionIndex, uint8_t>>;
		const Items & items() const {return m_items;}
	private:
		Items m_items;
	};

	struct ActMine : public Action {
		ActMine(char* parameters,
		        const World&,
		        const std::string& production_program_name,
		        ProductionSiteDescr*);
		void execute(Game &, ProductionSite &) const override;

	private:
		DescriptionIndex m_resource;
		uint8_t        m_distance; // width/radius of mine
		uint8_t        m_max;  // Can work up to this percent (of total mountain resources)
		uint8_t        m_chance; // odds of finding resources from empty mine
		uint8_t        m_training; // probability of training in _empty_ mines
	};

	struct ActCheckSoldier : public Action {
		ActCheckSoldier(char * parameters);
		void execute(Game &, ProductionSite &) const override;
	private:
		TrainingAttribute attribute;
		uint8_t level;
	};

	struct ActTrain : public Action {
		ActTrain(char * parameters);
		void execute(Game &, ProductionSite &) const override;
	private:
		TrainingAttribute attribute;
		uint8_t level;
		uint8_t target_level;
	};

	/// Plays a soundFX.
	///
	/// Parameter syntax:
	///    parameters ::= soundFX [priority]
	/// Parameter semantics:
	///    directory:
	///       The directory of the sound effect.
	///    soundFX:
	///       The base filename of a soundFX (without path, running numbers or file extension)
	///    priority:
	///       An integer. If omitted, 127 is used.
	///
	/// Plays the specified soundFX with the specified priority. Whether the
	/// soundFX is actually played is determined by the sound handler.
	struct ActPlayFX : public Action {
		ActPlayFX(char * parameters);
		void execute(Game &, ProductionSite &) const override;
	private:
		std::string name;
		uint8_t     priority;
	};

	/// Sends a building worker to construct at an immovable.
	///
	/// Parameter syntax:
	///    parameters ::= object worker-program radius
	/// Parameter semantics:
	///    object
	///       This is the name of the immovable that should be constructed (e.g. 'shipconstruction')
	///    worker-program
	///       This is the name of the program that the worker will use
	///       to act out the construction
	///    radius
	///       Activity radius
	struct ActConstruct : public Action {
		ActConstruct(char* parameters,
		             const std::string& production_program_name,
		             ProductionSiteDescr*);
		void execute(Game &, ProductionSite &) const override;
		bool get_building_work(Game &, ProductionSite &, Worker &) const override;
		void building_work_failed(Game &, ProductionSite &, Worker &) const override;

		const ImmovableDescr& get_construction_descr(const Tribes& tribes) const;

	private:
		std::string objectname;
		std::string workerprogram;
		uint32_t radius;
	};

	ProductionProgram(const std::string& _name,
							const std::string& _descname,
							std::unique_ptr<LuaTable> actions_table,
							const EditorGameBase& egbase,
							ProductionSiteDescr* building);

	~ProductionProgram() {
		for (Action * action : m_actions) {
			delete action;
		}
	}

	const std::string & name() const {return m_name;}
	const std::string & descname() const {return m_descname;}
	int32_t get_size() const {return m_actions.size();}
	const Action & operator[](size_t const idx) const {
		assert(idx < m_actions.size());
		return *m_actions[idx];
	}

	using Actions = std::vector<Action *>;
	const Actions & actions() const {return m_actions;}


private:
	std::string m_name;
	std::string m_descname;
	Actions     m_actions;
};

}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_PROGRAM_H
