/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
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

	/// A group of ware types with a count.
	using WareTypeGroup = std::pair<std::set<std::pair<DescriptionIndex, WareWorker>>, uint8_t>;
	using Groups = std::vector<WareTypeGroup>;

	/// Can be executed on a ProductionSite.
	struct Action {
		Action() = default;
		virtual ~Action();
		virtual void execute(Game&, ProductionSite&) const = 0;

		/**
		 * Called when the given worker is looking for work from the building,
		 * and none of the default actions apply.
		 *
		 * \return \c true iff the worker was assigned something to do,
		 *  \c false iff he should just idle.
		 */
		virtual bool get_building_work(Game&, ProductionSite&, Worker&) const;

		/**
		 * Called when the given worker returns from building work with
		 * a failed status.
		 */
		virtual void building_work_failed(Game&, ProductionSite&, Worker&) const;

		const Groups& consumed_wares_workers() const {
			return consumed_wares_workers_;
		}
		const BillOfMaterials& produced_wares() const {
			return produced_wares_;
		}
		const BillOfMaterials& recruited_workers() const {
			return recruited_workers_;
		}

	protected:
		Groups consumed_wares_workers_;
		BillOfMaterials produced_wares_;
		BillOfMaterials recruited_workers_;

	private:
		DISALLOW_COPY_AND_ASSIGN(Action);
	};

	/// Parse a group of ware types followed by an optional count and terminated
	/// by a space or null. Example: "fish,meat:2".
	static void parse_ware_type_group(char*& parameters,
	                                  WareTypeGroup& group,
	                                  const Tribes& tribes,
	                                  const BillOfMaterials& input_wares,
	                                  const BillOfMaterials& input_workers);

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
		void execute(Game&, ProductionSite&) const override;

		struct Condition {
			virtual ~Condition();
			virtual bool evaluate(const ProductionSite&) const = 0;
			virtual std::string description(const Tribes&) const = 0;
			virtual std::string description_negation(const Tribes&) const = 0;
		};
		static Condition*
		create_condition(char*& parameters, const ProductionSiteDescr&, const Tribes& tribes);
		struct Negation : public Condition {
			Negation(char*& parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
			   : operand(create_condition(parameters, descr, tribes)) {
			}
			virtual ~Negation();
			bool evaluate(const ProductionSite&) const override;
			// Just a dummy to satisfy the superclass interface. Do not use.
			std::string description(const Tribes&) const override;
			// Just a dummy to satisfy the superclass interface. Do not use.
			std::string description_negation(const Tribes&) const override;

		private:
			Condition* const operand;
		};

		/// Tests whether the economy needs a ware of type ware_type.
		struct EconomyNeedsWare : public Condition {
			explicit EconomyNeedsWare(const DescriptionIndex& i) : ware_type(i) {
			}
			bool evaluate(const ProductionSite&) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;

		private:
			DescriptionIndex ware_type;
		};

		/// Tests whether the economy needs a worker of type worker_type.
		struct EconomyNeedsWorker : public Condition {
			explicit EconomyNeedsWorker(const DescriptionIndex& i) : worker_type(i) {
			}
			bool evaluate(const ProductionSite&) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;

		private:
			DescriptionIndex worker_type;
		};

		/// Tests whether the site has the specified (or implied) number of
		/// wares, combining from any of the types specified, in its input
		/// queues.
		struct SiteHas : public Condition {
			SiteHas(char*& parameters, const ProductionSiteDescr&, const Tribes& tribes);
			bool evaluate(const ProductionSite&) const override;
			std::string description(const Tribes& tribes) const override;
			std::string description_negation(const Tribes& tribes) const override;

		private:
			WareTypeGroup group;
		};

		/// Tests whether any of the workers at the site needs experience to
		/// become upgraded.
		struct WorkersNeedExperience : public Condition {
			bool evaluate(const ProductionSite&) const override;
			std::string description(const Tribes&) const override;
			std::string description_negation(const Tribes&) const override;
		};

		using Conditions = std::vector<Condition*>;
		ProgramResult result_;
		bool is_when_;  //  otherwise it is "unless"
		Conditions conditions_;
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
		void execute(Game&, ProductionSite&) const override;

	private:
		ProductionProgram* program_;
		ProgramResultHandlingMethod handling_methods_[3];
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
		          ProductionSiteDescr*,
		          const Tribes& tribes);
		void execute(Game&, ProductionSite&) const override;
		bool get_building_work(Game&, ProductionSite&, Worker&) const override;
		void building_work_failed(Game&, ProductionSite&, Worker&) const override;
		const std::string& program() const {
			return program_;
		}

	private:
		std::string program_;
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
		explicit ActSleep(char* parameters);
		void execute(Game&, ProductionSite&) const override;

	private:
		Duration duration_;
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
		explicit ActCheckMap(char* parameters);
		void execute(Game&, ProductionSite&) const override;

	private:
		enum { SEAFARING = 1 };
		uint8_t feature_;
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
		void execute(Game&, ProductionSite&) const override;

	private:
		uint32_t id_;
		Duration duration_;
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
		void execute(Game&, ProductionSite&) const override;
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
		void execute(Game&, ProductionSite&) const override;
		bool get_building_work(Game&, ProductionSite&, Worker&) const override;
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
		void execute(Game&, ProductionSite&) const override;
		bool get_building_work(Game&, ProductionSite&, Worker&) const override;
	};

	struct ActMine : public Action {
		ActMine(char* parameters,
		        const World&,
		        const std::string& production_program_name,
		        ProductionSiteDescr*);
		void execute(Game&, ProductionSite&) const override;

	private:
		DescriptionIndex resource_;
		uint8_t distance_;  // width/radius of mine
		uint8_t max_;       // Can work up to this percent (of total mountain resources)
		uint8_t chance_;    // odds of finding resources from empty mine
		uint8_t training_;  // probability of training in _empty_ mines
	};

	struct ActCheckSoldier : public Action {
		explicit ActCheckSoldier(char* parameters);
		void execute(Game&, ProductionSite&) const override;

	private:
		TrainingAttribute attribute;
		uint8_t level;
	};

	struct ActTrain : public Action {
		explicit ActTrain(char* parameters);
		void execute(Game&, ProductionSite&) const override;

	private:
		TrainingAttribute attribute;
		uint8_t level;
		uint8_t target_level;
	};

	/// Plays a sound effect.
	///
	/// Parameter syntax:
	///    parameters ::= directory sound [priority]
	/// Parameter semantics:
	///    directory:
	///       The directory of the sound files, relative to the datadir.
	///    sound:
	///       The base filename of a sound effect (relative to the directory).
	///    priority:
	///       An integer. If omitted, 127 is used.
	///
	/// Plays the specified sound effect with the specified priority. Whether the
	/// sound effect is actually played is determined by the sound handler.
	struct ActPlaySound : public Action {
		explicit ActPlaySound(char* parameters);
		void execute(Game&, ProductionSite&) const override;

	private:
		std::string name;
		uint8_t priority;
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
		void execute(Game&, ProductionSite&) const override;
		bool get_building_work(Game&, ProductionSite&, Worker&) const override;
		void building_work_failed(Game&, ProductionSite&, Worker&) const override;

		const ImmovableDescr& get_construction_descr(const Tribes& tribes) const;

	private:
		std::string objectname;
		std::string workerprogram;
		uint32_t radius;
	};

	ProductionProgram(const std::string& init_name,
	                  const std::string& init_descname,
	                  std::unique_ptr<LuaTable> actions_table,
	                  const EditorGameBase& egbase,
	                  ProductionSiteDescr* building);

	const std::string& name() const;
	const std::string& descname() const;

	size_t size() const;
	const ProductionProgram::Action& operator[](size_t const idx) const;

	const ProductionProgram::Groups& consumed_wares_workers() const;
	const Buildcost& produced_wares() const;
	const Buildcost& recruited_workers() const;

private:
	std::string name_;
	std::string descname_;
	std::vector<std::unique_ptr<Action>> actions_;
	ProductionProgram::Groups consumed_wares_workers_;
	Buildcost produced_wares_;
	Buildcost recruited_workers_;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTION_PROGRAM_H
