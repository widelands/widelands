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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTIONSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTIONSITE_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/production_program.h"
#include "logic/map_objects/tribes/program_result.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Request;
class Soldier;
class WorkerDescr;

enum class FailNotificationType { kDefault, kFull };

/**
 * Every building that is part of the economics system is a production site.
 *
 * A production site has a worker.
 * A production site can have one (or more) output wares types (in theory it
 * should be possible to burn wares for some virtual result such as "mana", or
 *  maybe even just for the fun of it, although that's not planned).
 * A production site can have one (or more) input wares types. Every input
 * wares type has an associated store.
 */
class ProductionSiteDescr : public BuildingDescr {
public:
	friend struct ProductionProgram;  // To add animations

	ProductionSiteDescr(const std::string& init_descname,
	                    const std::string& msgctxt,
	                    MapObjectType type,
	                    const LuaTable& t,
	                    const Tribes& tribes,
	                    const World& world);
	ProductionSiteDescr(const std::string& init_descname,
	                    const std::string& msgctxt,
	                    const LuaTable& t,
	                    const Tribes& tribes,
	                    const World& world);

	Building& create_object() const override;

	// List of wares to register having economy checks. Parsed by the tribes during postload and must
	// be nullptr after loading has finished
	std::set<DescriptionIndex>* ware_demand_checks() const;
	// List of workers to register having economy checks. Parsed by the tribes during postload and
	// must be nullptr after loading has finished
	std::set<DescriptionIndex>* worker_demand_checks() const;
	// Clear ware and worker demand check info
	void clear_demand_checks();

	uint32_t nr_working_positions() const {
		uint32_t result = 0;
		for (const auto& working_pos : working_positions()) {
			result += working_pos.second;
		}
		return result;
	}
	const BillOfMaterials& working_positions() const {
		return working_positions_;
	}
	bool is_output_ware_type(const DescriptionIndex& i) const {
		return output_ware_types_.count(i);
	}
	bool is_output_worker_type(const DescriptionIndex& i) const {
		return output_worker_types_.count(i);
	}
	const BillOfMaterials& input_wares() const {
		return input_wares_;
	}
	const BillOfMaterials& input_workers() const {
		return input_workers_;
	}
	using Output = std::set<DescriptionIndex>;
	const Output& output_ware_types() const {
		return output_ware_types_;
	}
	const Output& output_worker_types() const {
		return output_worker_types_;
	}
	const ProductionProgram* get_program(const std::string&) const;
	using Programs = std::map<std::string, std::unique_ptr<ProductionProgram>>;
	const Programs& programs() const {
		return programs_;
	}

	const std::string& out_of_resource_title() const {
		return out_of_resource_title_;
	}

	const std::string& out_of_resource_heading() const {
		return out_of_resource_heading_;
	}

	const std::string& out_of_resource_message() const {
		return out_of_resource_message_;
	}

	const std::string& resource_not_needed_message() const {
		return resource_not_needed_message_;
	}

	uint32_t out_of_resource_productivity_threshold() const {
		return out_of_resource_productivity_threshold_;
	}

	bool highlight_overlapping_workarea_for(const std::string& n, bool* positive) const {
		const auto it = highlight_overlapping_workarea_for_.find(n);
		if (it == highlight_overlapping_workarea_for_.end()) {
			return false;
		} else {
			*positive = it->second;
			return true;
		}
	}

	const std::map<std::string, bool>& get_highlight_overlapping_workarea_for() const {
		return highlight_overlapping_workarea_for_;
	}

private:
	std::unique_ptr<std::set<DescriptionIndex>> ware_demand_checks_;
	std::unique_ptr<std::set<DescriptionIndex>> worker_demand_checks_;
	BillOfMaterials working_positions_;
	BillOfMaterials input_wares_;
	BillOfMaterials input_workers_;
	Output output_ware_types_;
	Output output_worker_types_;
	Programs programs_;
	std::string out_of_resource_title_;
	std::string out_of_resource_heading_;
	std::string out_of_resource_message_;
	std::string resource_not_needed_message_;
	int out_of_resource_productivity_threshold_;
	std::map<std::string, bool> highlight_overlapping_workarea_for_;

	DISALLOW_COPY_AND_ASSIGN(ProductionSiteDescr);
};

class ProductionSite : public Building {
	friend class MapBuildingdataPacket;
	friend struct ProductionProgram::ActReturn;
	friend struct ProductionProgram::ActReturn::WorkersNeedExperience;
	friend struct ProductionProgram::ActCall;
	friend struct ProductionProgram::ActCallWorker;
	friend struct ProductionProgram::ActSleep;
	friend struct ProductionProgram::ActCheckMap;
	friend struct ProductionProgram::ActAnimate;
	friend struct ProductionProgram::ActConsume;
	friend struct ProductionProgram::ActProduce;
	friend struct ProductionProgram::ActRecruit;
	friend struct ProductionProgram::ActMine;
	friend struct ProductionProgram::ActCheckSoldier;
	friend struct ProductionProgram::ActTrain;
	friend struct ProductionProgram::ActPlaySound;
	friend struct ProductionProgram::ActConstruct;
	MO_DESCR(ProductionSiteDescr)

public:
	explicit ProductionSite(const ProductionSiteDescr& descr);
	~ProductionSite() override;

	void log_general_info(const EditorGameBase&) const override;

	bool is_stopped() const {
		return is_stopped_;
	}
	void set_stopped(bool);

	struct WorkingPosition {
		WorkingPosition(Request* const wr = nullptr, Worker* const w = nullptr)
		   : worker_request(wr), worker(w) {
		}
		Request* worker_request;
		Worker* worker;
	};

	WorkingPosition const* working_positions() const {
		return working_positions_;
	}

	virtual bool has_workers(DescriptionIndex targetSite, Game& game);
	uint8_t get_statistics_percent() {
		return last_stat_percent_ / 10;
	}

	// receives the duration of the last period and the result (true if something was produced)
	// and sets actual_percent_ to new value
	void update_actual_statistics(uint32_t, bool);

	uint8_t get_actual_statistics() {
		return actual_percent_ / 10;
	}

	const std::string& production_result() const {
		return production_result_;
	}

	// Production and worker programs set this to explain the current
	// state of the production. This string is shown as a tooltip
	// when the mouse hovers over the building.
	void set_production_result(const std::string& text) {
		production_result_ = text;
	}

	InputQueue& inputqueue(DescriptionIndex, WareWorker) override;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;

	void remove_worker(Worker&) override;
	int warp_worker(EditorGameBase&, const WorkerDescr& wd);

	bool fetch_from_flag(Game&) override;
	bool get_building_work(Game&, Worker&, bool success) override;

	void set_economy(Economy*, WareWorker) override;

	using InputQueues = std::vector<InputQueue*>;
	const InputQueues& inputqueues() const {
		return input_queues_;
	}

	const std::vector<Worker*>& workers() const;

	bool can_start_working() const;

	/// sends a message to the player e.g. if the building's resource can't be found
	void notify_player(Game& game,
	                   uint8_t minutes,
	                   FailNotificationType type = FailNotificationType::kDefault);
	void unnotify_player();

	void set_default_anim(std::string);

	const BuildingSettings* create_building_settings() const override;

protected:
	void update_statistics_string(std::string* statistics) override;

	void load_finish(EditorGameBase& egbase) override;

protected:
	struct State {
		const ProductionProgram* program;  ///< currently running program
		size_t ip;                         ///< instruction pointer
		ProgramResult phase;               ///< micro-step index (instruction dependent)
		uint32_t flags;                    ///< pfXXX flags

		/**
		 * Instruction-dependent additional data.
		 */
		/*@{*/
		ObjectPointer objvar;
		Coords coord;
		/*@}*/

		State()
		   : program(nullptr), ip(0), phase(ProgramResult::kNone), flags(0), coord(Coords::null()) {
		}
	};

	Request& request_worker(DescriptionIndex);
	static void
	request_worker_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	/**
	 * Determine the next program to be run when the last program has finished.
	 * The default implementation starts program "work".
	 */
	virtual void find_and_start_next_program(Game&);

	State& top_state() {
		assert(stack_.size());
		return *stack_.rbegin();
	}
	State* get_state() {
		return stack_.size() ? &*stack_.rbegin() : nullptr;
	}
	void program_act(Game&);

	/// \param phase can be used to pass a value on to the next step in the
	/// program. For example if one step is a mine command, it can calculate
	/// how long it should take to mine, given the particular circumstances,
	/// and pass the result to the following animation command, to set the
	/// duration.
	void program_step(Game&, uint32_t delay = 10, ProgramResult phase = ProgramResult::kNone);

	void program_start(Game&, const std::string& program_name);
	virtual void program_end(Game&, ProgramResult);
	virtual void train_workers(Game&);

	void format_statistics_string();
	void try_start_working(Game&);
	void set_post_timer(int32_t const t) {
		post_timer_ = t;
	}

protected:  // TrainingSite must have access to this stuff
	WorkingPosition* working_positions_;

	int32_t fetchfromflag_;  ///< Number of wares to fetch from flag

	/// If a program has ended with the result Failed or Skipped, that program may not
	/// start again until a certain time has passed. This is a map from program
	/// name to game time. When a program ends with the result Failed or Skipped, its name
	/// is added to this map, with the current game time. (When the program ends
	/// with any other result, its name is removed from the map.)
	using FailedSkippedPrograms = std::map<std::string, Time>;
	FailedSkippedPrograms failed_skipped_programs_;

	using Stack = std::vector<State>;
	Stack stack_;           ///<  program stack
	bool program_timer_;    ///< execute next instruction based on pointer
	int32_t program_time_;  ///< timer time
	int32_t post_timer_;    ///< Time to schedule after ends

	BillOfMaterials produced_wares_;
	BillOfMaterials recruited_workers_;
	InputQueues input_queues_;  ///< input queues for all inputs
	uint16_t last_stat_percent_;
	// integer 0-10000000, to be divided by 10000 to get a percent, to avoid float (target range:
	// 0-100)
	uint32_t actual_percent_;  // basically this is percent * 10 to avoid floats
	uint32_t last_program_end_time;
	bool is_stopped_;
	std::string default_anim_;  // normally "idle", "empty", if empty mine.

private:
	enum class Trend { kUnchanged, kRising, kFalling };
	Trend trend_;
	std::string statistics_string_on_changed_statistics_;
	std::string production_result_;  // hover tooltip text

	int32_t main_worker_;

	DISALLOW_COPY_AND_ASSIGN(ProductionSite);
};

/**
 * Describes, how many wares of a certain ware can be stored in a house.
 *
 * This class will be extended to support ordering of certain wares directly or
 * releasing some wares out of a building
 */
struct Input {
	Input(const DescriptionIndex& Ware, uint8_t const Max) : ware_(Ware), max_(Max) {
	}
	~Input() {
	}

	DescriptionIndex ware() const {
		return ware_;
	}
	uint8_t max() const {
		return max_;
	}

private:
	DescriptionIndex ware_;
	uint8_t max_;
};

/**
 * Note to be published when a production site is out of resources
 */
// A note we're using to notify the AI
struct NoteProductionSiteOutOfResources {
	CAN_BE_SENT_AS_NOTE(NoteId::ProductionSiteOutOfResources)

	// The production site that is out of resources.
	ProductionSite* ps;

	// The player that owns the production site.
	Player* player;

	NoteProductionSiteOutOfResources(ProductionSite* const init_ps, Player* init_player)
	   : ps(init_ps), player(init_player) {
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTIONSITE_H
