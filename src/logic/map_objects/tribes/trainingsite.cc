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

#include "logic/map_objects/tribes/trainingsite.h"

#include <algorithm>
#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/request.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/production_program.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace {
std::string training_attribute_to_string(Widelands::TrainingAttribute attribute) {
	switch (attribute) {
	case Widelands::TrainingAttribute::kAttack:
		return "attack";
	case Widelands::TrainingAttribute::kDefense:
		return "defense";
	case Widelands::TrainingAttribute::kEvade:
		return "evade";
	case Widelands::TrainingAttribute::kHealth:
		return "health";
	default:
		return "unknown";
	}
}

inline Widelands::TypeAndLevel upgrade_key(const Widelands::TrainingAttribute attr,
                                           const uint16_t level) {
	// We want them ordered by level first
	return (static_cast<uint32_t>(level) << 8) + static_cast<uint32_t>(attr);
}
inline Widelands::TrainingAttribute type_from_key(const Widelands::TypeAndLevel key) {
	constexpr uint32_t kBase = 1 << 8;
	return static_cast<Widelands::TrainingAttribute>(key % kBase);
}
inline uint16_t level_from_key(const Widelands::TypeAndLevel key) {
	return key >> 8;
}
}  // namespace

namespace Widelands {

const uint32_t TrainingSite::training_state_multiplier_ = 12;

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/trainingsites/atlanteans/dungeon/init.lua
 */
TrainingSiteDescr::TrainingSiteDescr(const std::string& init_descname,
                                     const LuaTable& table,
                                     const std::vector<std::string>& attribs,
                                     Descriptions& descriptions)
   : ProductionSiteDescr(init_descname, MapObjectType::TRAININGSITE, table, attribs, descriptions),
     num_soldiers_(table.get_int("soldier_capacity")),
     max_stall_(table.get_int("trainer_patience")) {
	// Read the range of levels that can update this building
	//  TODO(unknown): This is currently hardcoded to "soldier" but it should search for
	//  sections starting with the name of each soldier type.
	//  These sections also seem redundant. Eliminate them (having the
	//  programs should be enough).
	std::unique_ptr<LuaTable> items_table;

	// Check dependencies between 'checksoldier' & 'train', and set min and max levels
	for (const auto& program : programs()) {
		// The value set by the latest call of 'checksoldier'
		ProductionProgram::Action::TrainingParameters from_checksoldier;
		for (size_t i = 0; i < program.second->size(); ++i) {
			const ProductionProgram::Action& action = (*program.second)[i];
			if (upcast(const ProductionProgram::ActCheckSoldier, checksoldier, &action)) {
				// Get values from 'checksoldier', which is a prerequisite for calling 'train'
				from_checksoldier = checksoldier->training();
			} else if (upcast(const ProductionProgram::ActTrain, train, &action)) {
				// Check 'train' against 'checksoldier' and set min/max levels. Fail on violation.
				const ProductionProgram::Action::TrainingParameters checkme = train->training();
				if (from_checksoldier.level == INVALID_INDEX) {
					throw GameDataError("Trainingsite '%s' is trying to call 'train' action without "
					                    "prior 'checksoldier' action in program '%s'",
					                    name().c_str(), program.first.c_str());
				}
				if (from_checksoldier.level >= checkme.level) {
					throw GameDataError(
					   "Trainingsite '%s' is trying to train a soldier attribute from level "
					   "%u to %u, but the 'checksoldier' action's level must be lower "
					   "than the 'train' action's level in program '%s'",
					   name().c_str(), from_checksoldier.level, checkme.level, program.first.c_str());
				}
				if (from_checksoldier.attribute != checkme.attribute) {
					throw GameDataError(
					   "Trainingsite '%s' is trying to train soldier attribute '%s', but 'checksoldier' "
					   "checked for soldier attribute '%s' in program '%s'",
					   name().c_str(), training_attribute_to_string(from_checksoldier.attribute).c_str(),
					   training_attribute_to_string(checkme.attribute).c_str(), program.first.c_str());
				}
				// All clear, let's add the training information
				update_level(from_checksoldier.attribute, from_checksoldier.level, checkme.level);
				training_costs_.emplace(
				   upgrade_key(from_checksoldier.attribute, from_checksoldier.level),
				   program.second->consumed_wares_workers());
			}
		}
	}

	if (table.has_key("messages")) {
		items_table = table.get_table("messages");
		no_soldier_to_train_message_ = items_table->get_string("no_soldier");
		no_soldier_for_training_level_message_ = items_table->get_string("no_soldier_for_level");
	} else {
		throw GameDataError("Training site '%s' is lacking its 'messages' table", name().c_str());
	}
}

/**
 * Create a new training site
 * \return  the new training site
 */
Building& TrainingSiteDescr::create_object() const {
	return *new TrainingSite(*this);
}

/**
 * \param at the attribute to investigate
 * \return  the minimum level to which this building can downgrade a
 * specified attribute
 */
unsigned TrainingSiteDescr::get_min_level(const TrainingAttribute at) const {
	switch (at) {
	case TrainingAttribute::kHealth:
		return min_health_;
	case TrainingAttribute::kAttack:
		return min_attack_;
	case TrainingAttribute::kDefense:
		return min_defense_;
	case TrainingAttribute::kEvade:
		return min_evade_;
	default:
		throw wexception("Unknown attribute value!");
	}
}

/**
 * Returns the maximum level to which this building can upgrade a
 * specified attribute
 * \param at  the attribute to investigate
 * \return  the maximum level to be attained at this site
 */
unsigned TrainingSiteDescr::get_max_level(const TrainingAttribute at) const {
	switch (at) {
	case TrainingAttribute::kHealth:
		return max_health_;
	case TrainingAttribute::kAttack:
		return max_attack_;
	case TrainingAttribute::kDefense:
		return max_defense_;
	case TrainingAttribute::kEvade:
		return max_evade_;
	default:
		throw wexception("Unknown attribute value!");
	}
}

void TrainingSiteDescr::update_level(TrainingAttribute attrib,
                                     unsigned from_level,
                                     unsigned to_level) {
	trained_attributes_.emplace(attrib);
	switch (attrib) {
	case TrainingAttribute::kHealth:
		min_health_ = std::min(min_health_, from_level);
		max_health_ = std::max(max_health_, to_level);
		train_health_ = true;
		return;
	case TrainingAttribute::kAttack:
		min_attack_ = std::min(min_attack_, from_level);
		max_attack_ = std::max(max_attack_, to_level);
		train_attack_ = true;
		return;
	case TrainingAttribute::kDefense:
		min_defense_ = std::min(min_defense_, from_level);
		max_defense_ = std::max(max_defense_, to_level);
		train_defense_ = true;
		return;
	case TrainingAttribute::kEvade:
		min_evade_ = std::min(min_evade_, from_level);
		max_evade_ = std::max(max_evade_, to_level);
		train_evade_ = true;
		return;
	default:
		throw wexception("Unknown attribute value!");
	}
}

// TODO(sirver): This SoldierControl looks very similar to te one in
// MilitarySite. Pull out a class to reuse code.
std::vector<Soldier*> TrainingSite::SoldierControl::present_soldiers() const {
	return training_site_->soldiers_;
}

std::vector<Soldier*> TrainingSite::SoldierControl::stationed_soldiers() const {
	return training_site_->soldiers_;
}

Quantity TrainingSite::SoldierControl::min_soldier_capacity() const {
	return 0;
}
Quantity TrainingSite::SoldierControl::max_soldier_capacity() const {
	return training_site_->descr().get_max_number_of_soldiers();
}
Quantity TrainingSite::SoldierControl::soldier_capacity() const {
	return training_site_->capacity_;
}

std::vector<Soldier*> TrainingSite::SoldierControl::associated_soldiers() const {
	std::vector<Soldier*> soldiers = stationed_soldiers();
	if (training_site_->soldier_request_ != nullptr) {
		for (const Transfer* t : training_site_->soldier_request_->get_transfers()) {
			Soldier& s = dynamic_cast<Soldier&>(*t->get_worker());
			soldiers.push_back(&s);
		}
	}
	return soldiers;
}

void TrainingSite::SoldierControl::set_soldier_capacity(Quantity const capacity) {
	if (training_site_->capacity_ == capacity) {
		return;  // Nothing to do
	}

	// TODO(tothxa): need to check below behaviour with new implementation
	/*
	// Said in github issue #3869 discussion:
	//
	// > the problem will always be if the capacity of a training site will be
	// > increased AND you don't see soldiers leaving the warehouse while you
	// > know they are there you will be confused. So we should keep this very
	// > short anything more then 5 sec will cause confusion I believe.
	//
	// This piece implements this demand. If we add more control buttons to the
	// UI later, side-effects like this could go away.
	if (capacity > training_site_->capacity_) {
	   // This is the capacity increased part from above.
	   // Splitting a bit futher.
	   if (0 == training_site_->capacity_ && 1 == capacity) {
	      // If the site had a capacity of zero, then the player probably micromanages
	      // and wants a partially trained soldier, if available. Resetting the state.
	      training_site_->repeated_layoff_ctr_ = 0;
	      training_site_->latest_trainee_was_kickout_ = false;
	   } else {
	      // Now the player just wants soldier. Any soldiers.
	      training_site_->recent_capacity_increase_ = true;
	   }
	}
	*/

	training_site_->capacity_ = std::min(capacity, max_soldier_capacity());
	training_site_->update_soldier_request(true);
}

/**
 * Drop a given soldier.
 *
 * 'Dropping' means releasing the soldier from the site. The soldier then
 * becomes available to the economy.
 *
 * \note This is called from player commands, so we need to verify that the
 * soldier is actually stationed here, without breaking anything if he isn't.
 */
void TrainingSite::SoldierControl::drop_soldier(Soldier& soldier) {
	Game& game = dynamic_cast<Game&>(training_site_->get_owner()->egbase());

	std::vector<Soldier*>::iterator it =
	   std::find(training_site_->soldiers_.begin(), training_site_->soldiers_.end(), &soldier);
	if (it == training_site_->soldiers_.end()) {
		training_site_->molog(
		   game.get_gametime(),
		   "TrainingSite::SoldierControl::drop_soldier: soldier not in training site");
		return;
	}

	training_site_->soldiers_.erase(it);

	soldier.reset_tasks(game);
	soldier.start_task_leavebuilding(game, true);

	// Schedule, so that we can call new soldiers on next act()
	training_site_->schedule_act(game, Duration(100));
	Notifications::publish(
	   NoteTrainingSiteSoldierTrained(training_site_, training_site_->get_owner()));
}

int TrainingSite::SoldierControl::incorporate_soldier(EditorGameBase& egbase, Soldier& s) {
	if (s.get_location(egbase) != training_site_) {
		if (stationed_soldiers().size() + 1 > training_site_->descr().get_max_number_of_soldiers()) {
			return -1;
		}

		s.set_location(training_site_);
	}

	// Bind the worker into this house, hide him on the map
	if (upcast(Game, game, &egbase); game != nullptr) {
		s.start_task_idle(*game, 0, -1);
	}

	// Make sure the request count is reduced or the request is deleted.
	training_site_->update_soldier_request(true);

	return 0;
}

/*
=============================

class TrainingSite

=============================
*/

TrainingSite::TrainingSite(const TrainingSiteDescr& d)
   : ProductionSite(d), soldier_control_(this), capacity_(descr().get_max_number_of_soldiers()) {

	set_soldier_control(&soldier_control_);

	// Initialize this in the constructor so that loading code may
	// overwrite priorities.
	init_upgrades();
	current_upgrade_ = upgrades_.end();
	set_post_timer(Duration(6000));
	max_stall_val_ = training_state_multiplier_ * d.get_max_stall();
}

/**
 * Setup the building and request soldiers
 */
bool TrainingSite::init(EditorGameBase& egbase) {
	ProductionSite::init(egbase);

	upcast(Game, game, &egbase);

	for (Soldier* soldier : soldiers_) {
		soldier->set_location_initially(*this);
		assert(!soldier->get_state());  //  Should be newly created.

		if (game != nullptr) {
			soldier->start_task_idle(*game, 0, -1);
		}
	}
	update_soldier_request(true);
	return true;
}

/**
 * Change the economy this site belongs to.
 * \par e  The new economy. Can be 0 (unconnected buildings have no economy).
 * \note the worker (but not the soldiers) is dealt with in the
 * PlayerImmovable code.
 */
void TrainingSite::set_economy(Economy* e, WareWorker type) {
	ProductionSite::set_economy(e, type);

	if ((soldier_request_ != nullptr) && type == soldier_request_->get_type()) {
		soldier_request_->set_economy(e);
	}
}

/**
 * Cleanup after a Training site is removed
 *
 * Cancel all soldier requests and release all soldiers
 */
void TrainingSite::cleanup(EditorGameBase& egbase) {
	delete soldier_request_;
	soldier_request_ = nullptr;

	ProductionSite::cleanup(egbase);
}

void TrainingSite::add_worker(Worker& w) {
	ProductionSite::add_worker(w);

	if (upcast(Soldier, soldier, &w)) {
		// Note that the given Soldier might already be in the array
		// for loadgames.
		if (std::find(soldiers_.begin(), soldiers_.end(), soldier) == soldiers_.end()) {
			soldiers_.push_back(soldier);
		}

		if (upcast(Game, game, &get_owner()->egbase())) {
			schedule_act(*game, Duration(100));
		}
	}
}

void TrainingSite::remove_worker(Worker& w) {
	upcast(Game, game, &get_owner()->egbase());

	if (upcast(Soldier, soldier, &w)) {
		std::vector<Soldier*>::iterator const it =
		   std::find(soldiers_.begin(), soldiers_.end(), soldier);
		if (it != soldiers_.end()) {
			soldiers_.erase(it);

			if (game != nullptr) {
				schedule_act(*game, Duration(100));
			}
		}
	}

	ProductionSite::remove_worker(w);
}

bool TrainingSite::is_present(Worker& worker) const {
	if (worker.descr().type() != MapObjectType::SOLDIER) {
		return Building::is_present(worker);
	}
	/* Unlike other workers or soldiers in militarysites,
	 * soldiers in trainingsites don't have a buildingwork task. */
	return worker.get_location(get_owner()->egbase()) == this &&
	       worker.get_position() == get_position();
}

Soldier*
TrainingSite::get_selected_soldier(Game& game, const TrainingAttribute attr, const unsigned level) {
	Soldier* soldier = selected_soldier_.get(game);
	if (soldier != nullptr && is_present(*soldier) && soldier->get_level(attr) == level) {
		return soldier;
	}

	// The player changed something while we were sleeping, we can't trust the cache, but it's not
	// worth rebuilding either.
	soldier = pick_another_soldier(attr, level);
	// nullptr is OK for both
	selected_soldier_ = soldier;
	return soldier;
}

// Only called when we can't trust the cache. Just make sure that the level matches.
Soldier* TrainingSite::pick_another_soldier(const TrainingAttribute attr, const unsigned level) {
	if (soldiers_.empty()) {
		return nullptr;
	}

	auto upgrade_it = upgrades_.find(upgrade_key(attr, level));
	if (upgrade_it == upgrades_.end()) {
		// TODO(tothxa): throw exception after v1.4
		// May happen after loading old savegame
		return nullptr;
		// throw wexception("Invalid use of pick_soldier(): %s cannot train attribute %s at level %u",
		//                  descr_->descname().c_str(), training_attribute_to_string(attr).c_str(),
		//                  level);
	}

	Soldier* best_soldier = nullptr;
	unsigned best_level = (build_heroes_ == SoldierPreference::kRookies) ?
	                         soldiers_.front()->descr().get_max_total_level() :
	                         0;
	unsigned current_level;

	for (Soldier* soldier : soldiers_) {
		if (soldier->get_level(attr) != level) {
			continue;
		}
		if (build_heroes_ == SoldierPreference::kAny) {
			return soldier;
		}
		current_level = soldier->get_total_level();
		if (compare_levels(best_level, current_level)) {
			best_soldier = soldier;
			best_level = current_level;
		}
	}

	return best_soldier;
}

inline bool TrainingSite::compare_levels(const unsigned first, const unsigned second) const {
	switch (build_heroes_) {
	case SoldierPreference::kAny:
		return false;
	case SoldierPreference::kHeroes:
		return (second > first);
	case SoldierPreference::kRookies:
		return (second < first);
	default:
		NEVER_HERE();
	}
}

/**
 * Request soldiers up to capacity, or let go of surplus soldiers.
 */
void TrainingSite::update_soldier_request(const bool needs_update_statuses) {
	if (soldiers_.size() >= capacity_) {
		if (soldier_request_ != nullptr) {
			delete soldier_request_;
			soldier_request_ = nullptr;
			force_rebuild_soldier_requests_ = true;
		}

		if (needs_update_statuses && soldiers_.size() > capacity_) {
			update_upgrade_statuses(false);
			drop_all_soldiers_from_vector(untrainable_soldiers_);
			if (soldiers_.size() <= capacity_) {
				// Let next run deal with refilling
				return;
			}
		}

		if (soldiers_.size() > capacity_) {
			// TODO(tothxa): For the sake of simplicity, this drops the least preferred soldiers first,
			//               even if they can be trained right away, and keeps the most preferred ones
			//               even if they're stalled.
			//               Should this be reversed? Or use a penalty system?
			std::vector<Soldier*> soldiers_copy(soldiers_);
			if (capacity_ == 0) {
				drop_all_soldiers_from_vector(soldiers_copy);
			} else {
				drop_soldiers_from_vector(soldiers_copy, soldiers_.size() - capacity_);
			}
		}

		return;
	}

	bool shortage = false;

	if (force_rebuild_soldier_requests_ && soldier_request_ != nullptr) {
		shortage = soldier_request_->get_num_transfers() == 0;
		delete soldier_request_;
		soldier_request_ = nullptr;
	}

	if (soldier_request_ == nullptr) {
		if (needs_update_statuses) {
			update_upgrade_statuses(false);
		}

		soldier_request_ =
		   new SoldierRequest(*this, owner().tribe().soldier(),
		                      TrainingSite::request_soldier_callback, wwWORKER, build_heroes_);

		RequireOr r;

		// Default: Request whatever the player set to be trained.
		Upgrade::Status min_status = Upgrade::Status::kNotPossible;

		if (!shortage && max_possible_status_ >= Upgrade::Status::kWait) {
			// We seem to be pretty good, we have free soldiers and supply for at least
			// some of the wares. Let's try to be more specific for quickly trainable
			// soldiers.
			min_status = Upgrade::Status::kWait;
		}

		// We could try to merge consecutive levels, but that's probably more trouble
		// than it's worth. We certainly must allow gaps.
		for (const auto& upgrade : upgrades_) {
			if (upgrade.second.status >= min_status) {
				const TrainingAttribute attr = type_from_key(upgrade.first);
				uint16_t level = level_from_key(upgrade.first);
				r.add(RequireAttribute(attr, level, level));
			}
		}

		soldier_request_->set_requirements(r);

		force_rebuild_soldier_requests_ = false;
	}

	soldier_request_->set_count(capacity_ - soldiers_.size());
}

/**
 * Soldier callback. Since the soldier was already added via add_worker,
 * we only need to update the request structure.
 */
void TrainingSite::request_soldier_callback(Game& game,
#ifndef NDEBUG
                                            Request& rq,
#else
                                            Request&,
#endif
                                            DescriptionIndex /* index */,
                                            Worker* const w,
                                            PlayerImmovable& target) {
	TrainingSite& tsite = dynamic_cast<TrainingSite&>(target);
	Soldier& s = dynamic_cast<Soldier&>(*w);

	assert(s.get_location(game) == &tsite);
	assert(tsite.soldier_request_ == &rq);

	tsite.soldier_control_.incorporate_soldier(game, s);

	// Restart trainer patience to prevent dropping freshly arrived soldiers
	tsite.failures_count_ = 0;
}

// soldiers_ itself must not be passed directly, pass a copy of it if needed
inline void TrainingSite::drop_all_soldiers_from_vector(std::vector<Soldier*>& v) {
	while (!v.empty()) {
		soldier_control_.drop_soldier(*v.back());
		v.pop_back();
	}
}

// soldiers_ itself must not be passed directly, pass a copy of it if needed
void TrainingSite::drop_soldiers_from_vector(std::vector<Soldier*>& v, unsigned number_to_drop) {
	if (number_to_drop >= v.size()) {
		log_warn("TrainingSite::drop_soldiers_from_vector(): Use drop_all_soldiers_from_vector()");
		drop_all_soldiers_from_vector(v);
		return;
	}

	// TODO(tothxa): skip sorting if preference == kAny
	std::sort(v.begin(), v.end(), [this](Soldier* a, Soldier* b) {
		// Sort in order of decreasing desirability, so we can use pop_back()
		return !compare_levels(a->get_total_level(), b->get_total_level());
	});
	for (; number_to_drop > 0; --number_to_drop) {
		soldier_control_.drop_soldier(*v.back());
		v.pop_back();
	}
}

/**
 * Drop all the soldiers that can not be upgraded further at this building.
 */
void TrainingSite::drop_unupgradable_soldiers(Game& /* game */) {
	drop_all_soldiers_from_vector(untrainable_soldiers_);
}

/**
 * Drop all the soldiers that can not be upgraded further at this level of resourcing.
 */
void TrainingSite::drop_stalled_soldiers(Game& /* game */) {
	if (failures_count_ <= max_stall_val_) {
		// Trainer is still patient
		return;
	}

	failures_count_ = 0;

	if (soldier_request_ != nullptr && soldier_request_->get_count() > 0 &&
	    soldier_request_->get_num_transfers() == 0) {
		// Maybe our soldier requirements are outdated?
		force_rebuild_soldier_requests_ = true;
	}

	if (max_possible_status_ <= Upgrade::Status::kNotPossible) {
		// There's a shortage of wares, we can't do anything about it.
		return;
	}

	// First kick out the ones who are waiting for unavailable wares, let the rest wait some more.
	if (!stalled_soldiers_.empty()) {
		drop_all_soldiers_from_vector(stalled_soldiers_);
		return;
	}

	if (max_possible_status_ < Upgrade::Status::kCanStart || waiting_soldiers_.empty()) {
		// No point in replacing trainees if we can't train the new ones either.
		// We just have to wait some more until the needed wares arrive.
		return;
	}

	// We may be able to request replacement soldiers whose next upgrade already has the wares here.
	drop_all_soldiers_from_vector(waiting_soldiers_);

	// Make sure that our soldier requirements are updated too.
	force_rebuild_soldier_requests_ = true;
}

std::unique_ptr<const BuildingSettings> TrainingSite::create_building_settings() const {
	std::unique_ptr<TrainingsiteSettings> settings(
	   new TrainingsiteSettings(descr(), owner().tribe()));
	settings->apply(*ProductionSite::create_building_settings());
	settings->desired_capacity =
	   std::min(settings->max_capacity, soldier_control_.soldier_capacity());
	settings->build_heroes = build_heroes_;
	// Prior to the resolution of a defect report against ISO C++11, local variable 'settings' would
	// have been copied despite being returned by name, due to its not matching the function return
	// type. Call 'std::move' explicitly to avoid copying on older compilers.
	// On modern compilers a simple 'return settings;' would've been fine.
	return std::unique_ptr<const BuildingSettings>(std::move(settings));
}

/**
 * In addition to advancing the program, update soldier status.
 */
void TrainingSite::act(Game& game, uint32_t const data) {
	// unit of gametime is [ms].
	ProductionSite::act(game, data);
	update_soldier_request(true);
}

void TrainingSite::program_end(Game& game, ProgramResult const result) {
	if (result != ProgramResult::kCompleted) {
		++failures_count_;
	} else if (top_state().program->name() != "sleep") {
		failures_count_ = 0;
	}

	update_upgrade_statuses(false);

	drop_unupgradable_soldiers(game);
	drop_stalled_soldiers(game);
	update_soldier_request(false);

	ProductionSite::program_end(game, result);
}

/**
 * Find and start the next training program.
 */
void TrainingSite::find_and_start_next_program(Game& game) {
	if (is_stopped() || capacity_ == 0) {
		program_start(game, "sleep");
		return;
	}

	if (soldiers_.empty()) {
		if (soldier_request_ != nullptr && soldier_request_->get_count() > 0 &&
		    soldier_request_->get_num_transfers() == 0) {
			++failures_count_;
		}
		program_start(game, "sleep");
		return;
	}

	update_upgrade_statuses(true);

	if (max_possible_status_ < Upgrade::Status::kCanStart) {
		++failures_count_;
		program_start(game, "sleep");
		return;
	}

	if (build_heroes_ != SoldierPreference::kAny) {
		// update_upgrade_statuses() picked the soldier and the training step for us,
		// unless there are no soldiers for the possible training steps
		if (selected_soldier_.get(game) == nullptr) {
			++failures_count_;
			program_start(game, "sleep");
			return;
		}
		assert(current_upgrade_ != upgrades_.end());
		program_start(game, current_upgrade_->second.program_name);
		return;
	}

	// We know there are wares for at least one upgrade, and we know there is at least one
	// soldier, but we don't know if they actually match. We also need wrap-around.
	// So we need this to prevent infinite loops.
	const TypeAndLevel last_upgrade =
	   current_upgrade_ != upgrades_.end() ? current_upgrade_->first : upgrades_.rbegin()->first;

	do {
		if (current_upgrade_ != upgrades_.end()) {
			++current_upgrade_;
		}
		if (current_upgrade_ == upgrades_.end()) {
			current_upgrade_ = upgrades_.begin();
		}
	} while (!current_upgrade_->second.has_wares_and_candidate() &&
	         current_upgrade_->first != last_upgrade);

	if (!current_upgrade_->second.has_wares_and_candidate()) {
		++failures_count_;
		program_start(game, "sleep");
		return;
	}

	selected_soldier_ = current_upgrade_->second.candidates.front();
	program_start(game, current_upgrade_->second.program_name);
}

/**
 * Called once at initialization to populate \ref upgrades_.
 */
void TrainingSite::init_upgrades() {
	assert(upgrades_.empty());

	if (descr().get_train_health()) {
		add_upgrades(TrainingAttribute::kHealth);
	}
	if (descr().get_train_attack()) {
		add_upgrades(TrainingAttribute::kAttack);
	}
	if (descr().get_train_defense()) {
		add_upgrades(TrainingAttribute::kDefense);
	}
	if (descr().get_train_evade()) {
		add_upgrades(TrainingAttribute::kEvade);
	}
}

/**
 * Only called from \ref init_upgrades
 */
void TrainingSite::add_upgrades(TrainingAttribute const attr) {
	int32_t min = descr().get_min_level(attr);
	int32_t max = descr().get_max_level(attr);

	for (int32_t level = min; level < max; ++level) {
		const TypeAndLevel key = upgrade_key(attr, level);
		upgrades_.emplace(key, Upgrade(attr, level));
	}
}

TrainingSite::Upgrade::Upgrade(const TrainingAttribute attr, const uint16_t level)
   : key(upgrade_key(attr, level)),
     program_name(format("upgrade_soldier_%s_%d", training_attribute_to_string(attr), level)) {
}

unsigned TrainingSite::current_training_level() const {
	if (current_upgrade_ == upgrades_.end()) {
		// This should only be possible when saving the game
		constexpr unsigned kRandomInvalidLevel = 99;
		return kRandomInvalidLevel;
	}
	return level_from_key(current_upgrade_->first);
}
TrainingAttribute TrainingSite::current_training_attribute() const {
	if (current_upgrade_ == upgrades_.end()) {
		// This should only be possible when saving the game
		return TrainingAttribute::kTotal;
	}
	return type_from_key(current_upgrade_->first);
}

// Only for loading from savegame
void TrainingSite::set_current_training_step(const uint8_t attr, const uint16_t level) {
	// upgrades_.end() is fine if inputs are invalid
	current_upgrade_ = upgrades_.find(upgrade_key(static_cast<TrainingAttribute>(attr), level));
}

void TrainingSite::update_upgrade_statuses(const bool select_next_step) {
	max_possible_status_ = Upgrade::Status::kDisabled;

	for (auto& upgrade_it : upgrades_) {
		Upgrade& upgrade = upgrade_it.second;
		upgrade.candidates.clear();
		const bool was_enabled = upgrade.status != Upgrade::Status::kDisabled;

		upgrade.status = Upgrade::Status::kCanStart;
		const ProductionProgram::Groups& costs = descr().get_training_cost(upgrade.key);

		for (const ProductionProgram::WareTypeGroup& group : costs) {
			uint32_t max = 0;
			uint32_t here = 0;
			uint32_t soon = 0;
			for (const auto& ware : group.first) {
				const InputQueue& iq = inputqueue(ware.first, ware.second, nullptr, 0);
				max += iq.get_max_fill();
				here += iq.get_filled();
				soon += iq.get_max_fill() - iq.get_missing();
			}
			if (max < group.second) {
				upgrade.status = Upgrade::Status::kDisabled;
				break;
			}
			if (here >= group.second) {
				continue;
			}
			if (soon < group.second) {
				upgrade.status = Upgrade::Status::kNotPossible;
			} else if (upgrade.status == Upgrade::Status::kCanStart) {
				upgrade.status = Upgrade::Status::kWait;
			}
		}

		if (upgrade.status > max_possible_status_) {
			max_possible_status_ = upgrade.status;
		}

		const bool is_enabled = upgrade.status != Upgrade::Status::kDisabled;
		if (was_enabled != is_enabled) {
			force_rebuild_soldier_requests_ = true;
		}
	}

	untrainable_soldiers_.clear();
	stalled_soldiers_.clear();
	waiting_soldiers_.clear();

	if (soldiers_.empty()) {
		selected_soldier_ = nullptr;
		return;
	}

	if (select_next_step) {
		selected_soldier_ = nullptr;
	}
	unsigned best_total_level = (build_heroes_ == SoldierPreference::kRookies) ?
	                               soldiers_.front()->descr().get_max_total_level() :
	                               0;

	for (Soldier* soldier : soldiers_) {
		Upgrade::Status best_status = Upgrade::Status::kDisabled;
		std::map<TypeAndLevel, Upgrade>::iterator possible_upgrade = upgrades_.end();
		unsigned lowest_level = 0;

		for (const TrainingAttribute attr : descr().trained_attributes()) {
			const unsigned level = soldier->get_level(attr);
			const auto upgrade_it = upgrades_.find(upgrade_key(attr, level));
			if (upgrade_it != upgrades_.end()) {
				upgrade_it->second.candidates.emplace_back(soldier);
				best_status = std::max(best_status, upgrade_it->second.status);
				if (select_next_step && upgrade_it->second.status == Upgrade::Status::kCanStart) {
					if (lowest_level > level || possible_upgrade == upgrades_.end()) {
						possible_upgrade = upgrade_it;
						lowest_level = level;
					}
				}
			}
		}

		switch (best_status) {
		case Upgrade::Status::kCanStart:
			if (select_next_step && build_heroes_ != SoldierPreference::kAny) {
				const unsigned this_total_level = soldier->get_total_level();
				if (compare_levels(best_total_level, this_total_level) ||
				    selected_soldier_ == nullptr) {
					best_total_level = this_total_level;
					// Already set these here to avoid repeating the iterations in
					// find_and_start_next_program()
					selected_soldier_ = soldier;
					current_upgrade_ = possible_upgrade;
				}
			}
			break;
		case Upgrade::Status::kDisabled:
			untrainable_soldiers_.emplace_back(soldier);
			break;
		case Upgrade::Status::kNotPossible:
			stalled_soldiers_.emplace_back(soldier);
			break;
		case Upgrade::Status::kWait:
			waiting_soldiers_.emplace_back(soldier);
			break;
		default:
			NEVER_HERE();
		}
	}
}

}  // namespace Widelands
