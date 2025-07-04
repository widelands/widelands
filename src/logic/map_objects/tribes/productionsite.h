/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTIONSITE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_PRODUCTIONSITE_H

#include <memory>

#include "base/macros.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/bill_of_materials.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/production_program.h"
#include "logic/map_objects/tribes/program_result.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/lua_table.h"

namespace Widelands {

class FerryFleetYardInterface;
class ShipFleetYardInterface;
class Soldier;

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
	friend struct ProductionProgram;  // To add animations, outputs etc.

	ProductionSiteDescr(const std::string& init_descname,
	                    MapObjectType type,
	                    const LuaTable& t,
	                    const std::vector<std::string>& attribs,
	                    Descriptions& descriptions);
	ProductionSiteDescr(const std::string& init_descname,
	                    const LuaTable& t,
	                    const std::vector<std::string>& attribs,
	                    Descriptions& descriptions);

	[[nodiscard]] Building& create_object() const override;

	// List of wares to register having economy checks. Parsed by the tribes during postload and must
	// be nullptr after loading has finished
	[[nodiscard]] std::set<DescriptionIndex>* ware_demand_checks() const;
	// List of workers to register having economy checks. Parsed by the tribes during postload and
	// must be nullptr after loading has finished
	[[nodiscard]] std::set<DescriptionIndex>* worker_demand_checks() const;
	// Clear ware and worker demand check info
	void clear_demand_checks();

	[[nodiscard]] uint32_t nr_working_positions() const {
		uint32_t result = 0;
		for (const auto& working_pos : working_positions()) {
			result += working_pos.second;
		}
		return result;
	}
	[[nodiscard]] const BillOfMaterials& working_positions() const {
		return working_positions_;
	}
	[[nodiscard]] bool is_output_ware_type(const DescriptionIndex& i) const {
		return output_ware_types_.count(i) != 0u;
	}
	[[nodiscard]] bool is_output_worker_type(const DescriptionIndex& i) const {
		return output_worker_types_.count(i) != 0u;
	}
	[[nodiscard]] const BillOfMaterials& input_wares() const {
		return input_wares_;
	}
	[[nodiscard]] const BillOfMaterials& input_workers() const {
		return input_workers_;
	}
	BillOfMaterials& mutable_input_wares() {
		return input_wares_;
	}
	BillOfMaterials& mutable_input_workers() {
		return input_workers_;
	}
	using Output = std::set<DescriptionIndex>;
	[[nodiscard]] const Output& output_ware_types() const {
		return output_ware_types_;
	}
	[[nodiscard]] const Output& output_worker_types() const {
		return output_worker_types_;
	}
	/// Map objects that this production site needs nearby according to attribute, without removing
	/// them from the map
	[[nodiscard]] const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	needed_attributes() const {
		return needed_attributes_;
	}
	/// Map objects that are collected from the map by this production site according to attribute
	[[nodiscard]] const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	collected_attributes() const {
		return collected_attributes_;
	}
	/// Map objects that are placed on the map by this production site according to attribute
	[[nodiscard]] const std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>>&
	created_attributes() const {
		return created_attributes_;
	}

	/// We only need the attributes during tribes initialization
	void clear_attributes();

	struct CollectedResourceInfo {
		unsigned max_percent;
		unsigned depleted_chance;
	};

	/// The resources that this production site needs to collect from the map, the max percent it can
	/// achieve, and the chance when depleted
	[[nodiscard]] const std::map<std::string, CollectedResourceInfo>& collected_resources() const {
		return collected_resources_;
	}
	/// The resources that this production site will place on the map
	[[nodiscard]] const std::set<std::string>& created_resources() const {
		return created_resources_;
	}
	/// The bobs (critters) that this production site needs to collect from the map
	[[nodiscard]] const std::set<std::string>& collected_bobs() const {
		return collected_bobs_;
	}
	/// Set that this production site needs to collect the given bob from the map
	void add_collected_bob(const std::string& bobname) {
		collected_bobs_.insert(bobname);
	}
	/// The bobs (critters or workers) that this production site will place on the map
	[[nodiscard]] const std::set<std::string>& created_bobs() const {
		return created_bobs_;
	}
	/// Set that this production site will place the given bob on the map
	void add_created_bob(const std::string& bobname) {
		created_bobs_.insert(bobname);
	}
	/// The immovables that this production site needs to be nearby
	[[nodiscard]] const std::set<std::string>& needed_immovables() const {
		return needed_immovables_;
	}
	/// The immovables that this production site needs to collect from the map
	[[nodiscard]] const std::set<std::string>& collected_immovables() const {
		return collected_immovables_;
	}
	/// Set that this production site makes use of the given immovable nearby
	void add_needed_immovable(const std::string& immovablename) {
		needed_immovables_.insert(immovablename);
	}
	/// Set that this production site needs to collect the given immovable from the map
	void add_collected_immovable(const std::string& immovablename) {
		collected_immovables_.insert(immovablename);
	}
	/// The immovables that this production site will place on the map
	[[nodiscard]] const std::set<std::string>& created_immovables() const {
		return created_immovables_;
	}
	/// Set that this production site will place the given immovable on the map
	void add_created_immovable(const std::string& immovablename) {
		created_immovables_.insert(immovablename);
	}

	[[nodiscard]] const ProductionProgram* get_program(const std::string&) const;
	using Programs = std::map<std::string, std::unique_ptr<ProductionProgram>>;
	[[nodiscard]] const Programs& programs() const {
		return programs_;
	}
	[[nodiscard]] Programs& mutable_programs() {
		return programs_;
	}

	[[nodiscard]] bool is_infinite_production_useful() const {
		return is_infinite_production_useful_;
	}
	void set_infinite_production_useful(const bool u) {
		is_infinite_production_useful_ = u;
	}

	[[nodiscard]] bool has_ship_fleet_check() const {
		return has_ship_fleet_check_;
	}
	[[nodiscard]] bool has_ferry_fleet_check() const {
		return has_ferry_fleet_check_;
	}

	[[nodiscard]] const std::string& out_of_resource_title() const {
		return out_of_resource_title_;
	}

	[[nodiscard]] const std::string& out_of_resource_heading() const {
		return out_of_resource_heading_;
	}

	[[nodiscard]] const std::string& out_of_resource_message() const {
		return out_of_resource_message_;
	}

	[[nodiscard]] const std::string& resource_not_needed_message() const {
		return resource_not_needed_message_;
	}

	[[nodiscard]] uint32_t out_of_resource_productivity_threshold() const {
		return out_of_resource_productivity_threshold_;
	}

	/// Returns 'true' if an overlap highlighting relationship has been set. Whether an overlap is
	/// wanted or not is written into 'positive'.
	bool highlight_overlapping_workarea_for(const std::string& productionsite, bool* positive) const;
	/// Set that this production site competes with the given 'productionsite' for map resources.
	void add_competing_productionsite(const std::string& productionsite);
	/// Set that this production site creates map resources or objects that the given
	/// 'productionsite' needs.
	void add_supports_productionsite(const std::string& productionsite);
	/// Set that the given 'productionsite' creates map resources or objects that this production
	/// site needs.
	void add_supported_by_productionsite(const std::string& productionsite);
	/// Returns whether this production site competes with the given 'productionsite' for map
	/// resources.
	[[nodiscard]] bool competes_with_productionsite(const std::string& productionsite) const;
	/// Returns whether this production site creates map resources or objects that the given
	/// 'productionsite' needs.
	[[nodiscard]] bool supports_productionsite(const std::string& productionsite) const;
	/// Returns whether the given 'productionsite' creates map resources or objects that this
	/// production site needs.
	[[nodiscard]] bool is_supported_by_productionsite(const std::string& productionsite) const;
	/// Returns the production sites that need a map resource or object that this production site
	/// will place on the map.
	[[nodiscard]] std::set<std::string> supported_productionsites() const {
		return supported_productionsites_;
	}
	/// Returns the production sites that create a map resource or object that this production site
	/// needs.
	[[nodiscard]] std::set<std::string> supported_by_productionsites() const {
		return supported_by_productionsites_;
	}

protected:
	void add_output_ware_type(DescriptionIndex index) {
		output_ware_types_.insert(index);
	}
	void add_output_worker_type(DescriptionIndex index) {
		output_worker_types_.insert(index);
	}

	/// Set that this production site needs to be placed near map objects with the given attribute
	void
	add_needed_attribute(std::pair<MapObjectType, MapObjectDescr::AttributeIndex> attribute_info) {
		needed_attributes_.insert(attribute_info);
	}

	/// Set that this production site needs to collect map objects with the given attribute from the
	/// map
	void add_collected_attribute(
	   std::pair<MapObjectType, MapObjectDescr::AttributeIndex> attribute_info) {
		collected_attributes_.insert(attribute_info);
	}
	/// Set that this production site will place map objects with the given attribute on the map
	void
	add_created_attribute(std::pair<MapObjectType, MapObjectDescr::AttributeIndex> attribute_info) {
		created_attributes_.insert(attribute_info);
	}
	/// Set that this production site needs to collect the given resource from the map
	void add_collected_resource(const std::string& resource,
	                            unsigned max_percent,
	                            unsigned depleted_chance) {
		collected_resources_.insert(
		   std::make_pair(resource, CollectedResourceInfo{max_percent, depleted_chance}));
	}
	/// Set that this production site will place the given resource on the map
	void add_created_resource(const std::string& resource) {
		created_resources_.insert(resource);
	}

private:
	std::unique_ptr<std::set<DescriptionIndex>> ware_demand_checks_;
	std::unique_ptr<std::set<DescriptionIndex>> worker_demand_checks_;
	BillOfMaterials working_positions_;
	BillOfMaterials input_wares_;
	BillOfMaterials input_workers_;
	Output output_ware_types_;
	Output output_worker_types_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> needed_attributes_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> collected_attributes_;
	std::set<std::pair<MapObjectType, MapObjectDescr::AttributeIndex>> created_attributes_;
	std::map<std::string, CollectedResourceInfo> collected_resources_;
	std::set<std::string> created_resources_;
	std::set<std::string> collected_bobs_;
	std::set<std::string> created_bobs_;
	std::set<std::string> needed_immovables_;
	std::set<std::string> collected_immovables_;
	std::set<std::string> created_immovables_;
	Programs programs_;
	bool is_infinite_production_useful_;
	std::string out_of_resource_title_;
	std::string out_of_resource_heading_;
	std::string out_of_resource_message_;
	std::string resource_not_needed_message_;
	int out_of_resource_productivity_threshold_;
	std::set<std::string> competing_productionsites_;
	std::set<std::string> supported_productionsites_;
	std::set<std::string> supported_by_productionsites_;
	bool has_ship_fleet_check_{false};
	bool has_ferry_fleet_check_{false};

	DISALLOW_COPY_AND_ASSIGN(ProductionSiteDescr);
};

class ProductionSite : public Building {
	friend class MapBuildingdataPacket;
	// TODO(Nordfriese): Refactor this. Many friend classes are bad style.
	friend struct ProductionProgram::ActReturn;
	friend struct ProductionProgram::ActReturn::WorkersNeedExperience;
	friend struct ProductionProgram::ActCall;
	friend struct ProductionProgram::ActCallWorker;
	friend struct ProductionProgram::ActSleep;
	friend struct ProductionProgram::ActAnimate;
	friend struct ProductionProgram::ActConsume;
	friend struct ProductionProgram::ActProduce;
	friend struct ProductionProgram::ActRecruit;
	friend struct ProductionProgram::ActMine;
	friend struct ProductionProgram::ActCheckSoldier;
	friend struct ProductionProgram::ActTrain;
	friend struct ProductionProgram::ActPlaySound;
	friend struct ProductionProgram::ActRunScript;
	friend struct ProductionProgram::ActConstruct;
	MO_DESCR(ProductionSiteDescr)

public:
	explicit ProductionSite(const ProductionSiteDescr& descr);

	void log_general_info(const EditorGameBase&) const override;

	bool is_stopped() const {
		return is_stopped_;
	}
	void set_stopped(bool);

	struct WorkingPosition {
		explicit WorkingPosition(Request* const wr = nullptr, Worker* const w = nullptr)
		   : worker_request(wr), worker(w) {
		}
		Request* worker_request;
		OPtr<Worker> worker;
	};

	[[nodiscard]] const std::vector<WorkingPosition>* working_positions() const {
		return &working_positions_;
	}

	virtual bool has_workers(DescriptionIndex targetSite, Game& game);
	[[nodiscard]] uint8_t get_statistics_percent() const {
		return last_stat_percent_ / 10;
	}

	// receives the duration of the last period and the result (true if something was produced)
	// and sets actual_percent_ to new value
	void update_actual_statistics(Duration, bool);

	[[nodiscard]] uint8_t get_actual_statistics() const {
		return actual_percent_ / 10;
	}

	[[nodiscard]] const std::string& production_result() const {
		return production_result_;
	}

	// Production and worker programs set this to explain the current
	// state of the production. This string is shown as a tooltip
	// when the mouse hovers over the building.
	void set_production_result(const std::string& text) {
		production_result_ = text;
	}

	InputQueue&
	inputqueue(DescriptionIndex, WareWorker, const Request*, uint32_t disambiguator_id) override;

	bool init(EditorGameBase&) override;
	void cleanup(EditorGameBase&) override;
	void act(Game&, uint32_t data) override;

	std::pair<int32_t, int32_t> find_worker(OPtr<Worker>);
	void try_replace_worker(const Game*, DescriptionIndex, WorkingPosition*);
	void remove_worker(Worker&) override;
	bool warp_worker(EditorGameBase&, const WorkerDescr& wd, int32_t slot = -1);

	bool fetch_from_flag(Game&) override;
	bool get_building_work(Game&, Worker&, bool success) override;

	void set_economy(Economy*, WareWorker) override;

	using InputQueues = std::vector<InputQueue*>;
	[[nodiscard]] const InputQueues& inputqueues() const {
		return input_queues_;
	}

	[[nodiscard]] const std::vector<Worker*>& workers() const;

	[[nodiscard]] bool can_start_working() const;

	/// sends a message to the player e.g. if the building's resource can't be found
	void notify_player(Game& game,
	                   uint8_t minutes,
	                   FailNotificationType type = FailNotificationType::kDefault);
	void unnotify_player();

	void set_default_anim(const std::string&);

	[[nodiscard]] std::unique_ptr<const BuildingSettings> create_building_settings() const override;

	// This function forces the productionsite to interrupt whatever it is doing ASAP,
	// and start the specified program immediately afterwards. If that program expects
	// some extra data, this data needs to be provided as the third parameter.
	void set_next_program_override(Game&, const std::string&, MapObject* extra_data);
	// Returns `true` if `set_next_program_override()` has been called recently
	// and the force-started program has not terminated yet.
	[[nodiscard]] bool has_forced_state() const;

	[[nodiscard]] bool infinite_production() const {
		return infinite_production_;
	}
	void set_infinite_production(bool);

	[[nodiscard]] const std::vector<ShipFleetYardInterface*>& get_ship_fleet_interfaces() const {
		return ship_fleet_interfaces_;
	}
	[[nodiscard]] const std::vector<FerryFleetYardInterface*>& get_ferry_fleet_interfaces() const {
		return ferry_fleet_interfaces_;
	}
	void remove_fleet_interface(EditorGameBase& egbase, const ShipFleetYardInterface* i);
	void remove_fleet_interface(EditorGameBase& egbase, const FerryFleetYardInterface* i);

protected:
	void update_statistics_string(std::string* statistics) override;

	void load_finish(EditorGameBase& egbase) override;
	void postload(EditorGameBase& egbase) override;

	struct State {
		const ProductionProgram* program{nullptr};  ///< currently running program
		size_t ip{0};                               ///< instruction pointer
		ProgramResult phase{ProgramResult::kNone};  ///< micro-step index (instruction dependent)
		enum StateFlags : uint32_t { kStateFlagIgnoreStopped = 1, kStateFlagHasExtraData = 2 };
		uint32_t flags{0};  ///< pfXXX flags

		/**
		 * Instruction-dependent additional data.
		 */
		/*@{*/
		ObjectPointer objvar;
		Coords coord;
		/*@}*/

		State() : coord(Coords::null()) {
		}
	};

	Request& request_worker(DescriptionIndex);
	static void
	request_worker_callback(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);

	/**
	 * Determine the next program to be run when the last program has finished.
	 * The default implementation starts program "main".
	 */
	virtual void find_and_start_next_program(Game&);

	[[nodiscard]] State& top_state() {
		assert(!stack_.empty());
		return *stack_.rbegin();
	}
	[[nodiscard]] State* get_state() {
		return !stack_.empty() ? &*stack_.rbegin() : nullptr;
	}
	void program_act(Game&);

	/// \param phase can be used to pass a value on to the next step in the
	/// program. For example if one step is a mine command, it can calculate
	/// how long it should take to mine, given the particular circumstances,
	/// and pass the result to the following animation command, to set the
	/// duration.
	void program_step(Game&,
	                  const Duration& delay = Duration(10),
	                  ProgramResult phase = ProgramResult::kNone);

	void program_start(Game&,
	                   const std::string& program_name,
	                   bool force = false,
	                   MapObject* extra_data = nullptr);
	virtual void program_end(Game&, ProgramResult);
	virtual void train_workers(Game&);
	void init_yard_interfaces(EditorGameBase& egbase);

	void format_statistics_string();
	void try_start_working(Game&);
	void set_post_timer(const Duration& t) {
		post_timer_ = t;
	}

	// TrainingSite must have access to this stuff
	std::vector<WorkingPosition> working_positions_;

	int32_t fetchfromflag_{0};  ///< Number of wares to fetch from flag

	/// If a program has ended with the result Failed or Skipped, that program may not
	/// start again until a certain time has passed. This is a map from program
	/// name to game time. When a program ends with the result Failed or Skipped, its name
	/// is added to this map, with the current game time. (When the program ends
	/// with any other result, its name is removed from the map.)
	using FailedSkippedPrograms = std::map<std::string, Time>;
	FailedSkippedPrograms failed_skipped_programs_;

	using Stack = std::vector<State>;
	Stack stack_;                ///<  program stack
	bool program_timer_{false};  ///< execute next instruction based on pointer
	Time program_time_{0U};      ///< timer time
	Duration post_timer_{50U};   ///< Time to schedule after ends

	BillOfMaterials produced_wares_;
	BillOfMaterials recruited_workers_;
	InputQueues input_queues_;  ///< input queues for all inputs
	uint16_t last_stat_percent_{0U};
	// integer 0-10000000, to be divided by 10000 to get a percent, to avoid float (target range:
	// 0-100)
	uint32_t actual_percent_{0U};  // basically this is percent * 10 to avoid floats
	Time last_program_end_time{0U};
	bool is_stopped_{false};
	bool infinite_production_{false};
	std::string default_anim_{"idle"};  // normally "idle", "empty", if empty mine.

	std::vector<ShipFleetYardInterface*> ship_fleet_interfaces_;
	std::vector<FerryFleetYardInterface*> ferry_fleet_interfaces_;
	std::unique_ptr<Notifications::Subscriber<NoteFieldTerrainChanged>>
	   field_terrain_changed_subscriber_;

private:
	enum class Trend { kUnchanged, kRising, kFalling };
	Trend trend_;
	std::string statistics_string_on_changed_statistics_;
	std::string production_result_;  // hover tooltip text

	int32_t main_worker_{-1};

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
	~Input() = default;

	[[nodiscard]] DescriptionIndex ware() const {
		return ware_;
	}
	[[nodiscard]] uint8_t max() const {
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
