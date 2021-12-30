/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "logic/map_objects/tribes/trainingsite.h"

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
}  // namespace

namespace Widelands {

const uint32_t TrainingSite::training_state_multiplier_ = 12;

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/trainingsites/atlanteans/dungeon/init.lua
 */
TrainingSiteDescr::TrainingSiteDescr(const std::string& init_descname,
                                     const LuaTable& table,
                                     Descriptions& descriptions)
   : ProductionSiteDescr(init_descname, MapObjectType::TRAININGSITE, table, descriptions),
     num_soldiers_(table.get_int("soldier_capacity")),
     max_stall_(table.get_int("trainer_patience")),

     train_health_(false),
     train_attack_(false),
     train_defense_(false),
     train_evade_(false),
     min_health_(std::numeric_limits<uint32_t>::max()),
     min_attack_(std::numeric_limits<uint32_t>::max()),
     min_defense_(std::numeric_limits<uint32_t>::max()),
     min_evade_(std::numeric_limits<uint32_t>::max()),
     max_health_(0),
     max_attack_(0),
     max_defense_(0),
     max_evade_(0) {
	// Read the range of levels that can update this building
	//  TODO(unknown): This is currently hardcoded to "soldier" but it should search for
	//  sections starting with the name of each soldier type.
	//  These sections also seem redundant. Eliminate them (having the
	//  programs should be enough).
	std::unique_ptr<LuaTable> items_table;
	if (table.has_key("soldier health")) {
		items_table = table.get_table("soldier health");
		// TODO(GunChleoc): Compatibility, remove these after v1.0
		if (items_table->has_key<std::string>("min_level")) {
			log_warn("Trainingsite '%s': Keys 'min_level' and 'max_level' in table 'soldier "
			         "health' are no longer needed\n",
			         name().c_str());
		}
		add_training_inputs(*items_table, &food_health_, &weapons_health_);
	}

	if (table.has_key("soldier attack")) {
		items_table = table.get_table("soldier attack");
		if (items_table->has_key<std::string>("min_level")) {
			log_warn("Trainingsite '%s': Keys 'min_level' and 'max_level' in table 'soldier "
			         "attack' are no longer needed\n",
			         name().c_str());
		}
		add_training_inputs(*items_table, &food_attack_, &weapons_attack_);
	}
	if (table.has_key("soldier defense")) {
		items_table = table.get_table("soldier defense");
		if (items_table->has_key<std::string>("min_level")) {
			log_warn("Trainingsite '%s': Keys 'min_level' and 'max_level' in table 'soldier "
			         "defense' are no longer needed\n",
			         name().c_str());
		}
		add_training_inputs(*items_table, &food_defense_, &weapons_defense_);
	}
	if (table.has_key("soldier evade")) {
		items_table = table.get_table("soldier evade");
		if (items_table->has_key<std::string>("min_level")) {
			log_warn("Trainingsite '%s': Keys 'min_level' and 'max_level' in table 'soldier "
			         "evade' are no longer needed\n",
			         name().c_str());
		}
		add_training_inputs(*items_table, &food_evade_, &weapons_evade_);
	}

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
					   "%d to %d, but the 'checksoldier' action's level must be lower "
					   "than the 'train' action's level in program '%s'",
					   name().c_str(), from_checksoldier.level, checkme.level, program.first.c_str());
				}
				if (from_checksoldier.attribute != checkme.attribute) {
					throw GameDataError(
					   "Trainingsite '%s' is trying to train soldier attribute '%s', but 'checksoldier' "
					   "checked "
					   "for soldier attribute '%s' in program '%s'",
					   name().c_str(), training_attribute_to_string(from_checksoldier.attribute).c_str(),
					   training_attribute_to_string(checkme.attribute).c_str(), program.first.c_str());
				}
				// All clear, let's add the training information
				update_level(from_checksoldier.attribute, from_checksoldier.level);
			}
		}
	}

	if (table.has_key("messages")) {
		items_table = table.get_table("messages");
		no_soldier_to_train_message_ = items_table->get_string("no_soldier");
		no_soldier_for_training_level_message_ = items_table->get_string("no_soldier_for_level");
	} else {
		// TODO(GunChleoc): API compatibility - require this after v1.0
		log_warn("Training site '%s' is lacking its 'messages' table", name().c_str());
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
	case TrainingAttribute::kTotal:
		throw wexception("Unknown attribute value!");
	}
	NEVER_HERE();
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
	case TrainingAttribute::kTotal:
		throw wexception("Unknown attribute value!");
	}
	NEVER_HERE();
}

/**
 * Return the maximum level that can be trained, both by school type
 * and resourcing.
 */
int32_t TrainingSite::get_max_unstall_level(const TrainingAttribute at,
                                            const TrainingSiteDescr& tsd) const {
	const int32_t max = tsd.get_max_level(at);
	const int32_t min = tsd.get_min_level(at);
	int32_t lev = min;
	int32_t rtv = min;
	while (lev < max) {
		TypeAndLevel train_tl(at, ++lev);
		TrainFailCount::const_iterator tstep = training_failure_count_.find(train_tl);
		if (max_stall_val_ > tstep->second.first) {
			rtv = lev;
		} else {
			lev = max;
		}
	}

	return rtv;
}

void TrainingSiteDescr::add_training_inputs(const LuaTable& table,
                                            std::vector<std::vector<std::string>>* food,
                                            std::vector<std::string>* weapons) {

	if (table.has_key("food")) {
		std::unique_ptr<LuaTable> food_table = table.get_table("food");
		for (const int key : food_table->keys<int>()) {
			std::vector<std::string> food_vector;
			for (const std::string& food_item :
			     food_table->get_table(key)->array_entries<std::string>()) {
				food_vector.push_back(food_item);
			}
			food->push_back(food_vector);
		}
	}
	if (table.has_key("weapons")) {
		for (const std::string& weapon : table.get_table("weapons")->array_entries<std::string>()) {
			weapons->push_back(weapon);
		}
	}
}

void TrainingSiteDescr::update_level(TrainingAttribute attrib, unsigned level) {
	switch (attrib) {
	case TrainingAttribute::kHealth:
		min_health_ = std::min(min_health_, level);
		max_health_ = std::max(max_health_, level);
		train_health_ = true;
		return;
	case TrainingAttribute::kAttack:
		min_attack_ = std::min(min_attack_, level);
		max_attack_ = std::max(max_attack_, level);
		train_attack_ = true;
		return;
	case TrainingAttribute::kDefense:
		min_defense_ = std::min(min_defense_, level);
		max_defense_ = std::max(max_defense_, level);
		train_defense_ = true;
		return;
	case TrainingAttribute::kEvade:
		min_evade_ = std::min(min_evade_, level);
		max_evade_ = std::max(max_evade_, level);
		train_evade_ = true;
		return;
	case TrainingAttribute::kTotal:
		throw wexception("Unknown attribute value!");
	}
	NEVER_HERE();
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

void TrainingSite::SoldierControl::set_soldier_capacity(Quantity const capacity) {
	assert(min_soldier_capacity() <= capacity);
	assert(capacity <= max_soldier_capacity());
	if (training_site_->capacity_ == capacity) {
		return;  // Nothing to do
	}
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
	training_site_->capacity_ = capacity;
	training_site_->update_soldier_request(false);
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
	if (upcast(Game, game, &egbase)) {
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
   : ProductionSite(d),
     soldier_control_(this),
     soldier_request_(nullptr),
     capacity_(descr().get_max_number_of_soldiers()),
     build_heroes_(false),
     result_(ProgramResult::kFailed) {
	set_soldier_control(&soldier_control_);

	// Initialize this in the constructor so that loading code may
	// overwrite priorities.
	calc_upgrades();
	current_upgrade_ = nullptr;
	set_post_timer(Duration(6000));
	training_failure_count_.clear();
	max_stall_val_ = training_state_multiplier_ * d.get_max_stall();
	highest_trainee_level_seen_ = 1;
	latest_trainee_kickout_level_ = 1;
	latest_trainee_was_kickout_ = false;
	requesting_weak_trainees_ = false;
	request_open_since_ = Time(0);
	trainee_general_lower_bound_ = 2;
	repeated_layoff_ctr_ = 0;
	repeated_layoff_inc_ = false;
	recent_capacity_increase_ = false;

	if (d.get_train_health()) {
		init_kick_state(TrainingAttribute::kHealth, d);
	}
	if (d.get_train_attack()) {
		init_kick_state(TrainingAttribute::kAttack, d);
	}
	if (d.get_train_defense()) {
		init_kick_state(TrainingAttribute::kDefense, d);
	}
	if (d.get_train_evade()) {
		init_kick_state(TrainingAttribute::kEvade, d);
	}
}
void TrainingSite::init_kick_state(const TrainingAttribute& art, const TrainingSiteDescr& d) {
	// Now with kick-out state saving implemented, initializing is an overkill
	for (unsigned t = d.get_min_level(art); t <= d.get_max_level(art); t++) {
		training_attempted(art, t);
	}
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

		if (game) {
			soldier->start_task_idle(*game, 0, -1);
		}
	}
	update_soldier_request(false);
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

	if (soldier_request_ && type == soldier_request_->get_type()) {
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

void TrainingSite::switch_heroes() {
	build_heroes_ = !build_heroes_;
	molog(owner().egbase().get_gametime(), "BUILD_HEROES: %s", build_heroes_ ? "TRUE" : "FALSE");
}

void TrainingSite::remove_worker(Worker& w) {
	upcast(Game, game, &get_owner()->egbase());

	if (upcast(Soldier, soldier, &w)) {
		std::vector<Soldier*>::iterator const it =
		   std::find(soldiers_.begin(), soldiers_.end(), soldier);
		if (it != soldiers_.end()) {
			soldiers_.erase(it);

			if (game) {
				schedule_act(*game, Duration(100));
			}
		}
	}

	ProductionSite::remove_worker(w);
}

/**
 * Request soldiers up to capacity, or let go of surplus soldiers.
 *
 * Now, we attempt to intelligently select most suitable soldiers
 * (either already somwhat trained, or if training stalls, less
 * trained ones). If no luck, the criteria is made relaxed until
 * somebody shows up.
 */
void TrainingSite::update_soldier_request(bool did_incorporate) {
	Game* game = get_owner() ? dynamic_cast<Game*>(&(get_owner()->egbase())) : nullptr;
	bool rebuild_request = false;
	bool need_more_soldiers = false;
	Duration dynamic_timeout = acceptance_threshold_timeout;
	uint8_t trainee_general_upper_bound = std::numeric_limits<uint8_t>::max() - 1;
	bool limit_upper_bound = false;

	if (soldiers_.size() < capacity_) {
		// If not full, I need more soldiers.
		need_more_soldiers = true;
	}

	// Usually, we prefer already partially trained soldiers here.
	// In some conditions, this can lead to same soldiers walking back and forth.
	// this tries to break that cycle. The goal is that this code only kicks in
	// in those specific conditions. This if statement is true if we repeatedly
	// incorporate and release soldiers, without training them at all.
	if (kUpperBoundThreshold_ < repeated_layoff_ctr_) {
		if (repeated_layoff_ctr_ > kUpperBoundThreshold_ + highest_trainee_level_seen_) {
			repeated_layoff_ctr_ = 0;
		} else {
			trainee_general_upper_bound =
			   kUpperBoundThreshold_ + highest_trainee_level_seen_ - repeated_layoff_ctr_;
			limit_upper_bound = true;
		}
		if (did_incorporate) {
			rebuild_request = need_more_soldiers;
		}
	}
	// This boolean ensures that kicking out many soldiers in a row does not count as
	// soldiers entering and leaving without training. We need to repeatedly incorporate
	// and release for the last resort to kick in. I need this boolean, to detect that
	// a soldier was incorporated between soldiers leaving.
	if (did_incorporate) {
		repeated_layoff_inc_ = true;
	}

	const Time& timeofgame = game ? game->get_gametime() : Time(0);

	if (did_incorporate && latest_trainee_was_kickout_ != requesting_weak_trainees_) {
		// If type of desired recruits has been changed, the request is rebuild after incorporate
		// even if (wrong/old) type recruits are on the way.
		rebuild_request = need_more_soldiers;
		requesting_weak_trainees_ = latest_trainee_was_kickout_;
	}

	if (did_incorporate) {
		// If we got somebody in, lets become picky again.
		// Request is not regenerated at this point. Should it?
		if (requesting_weak_trainees_) {
			trainee_general_lower_bound_ = latest_trainee_kickout_level_;
		} else {
			trainee_general_lower_bound_ = static_cast<uint8_t>(std::max<unsigned>(
			   1, (std::min<unsigned>(highest_trainee_level_seen_,
			                          (static_cast<unsigned>(trainee_general_lower_bound_) + 1 +
			                           static_cast<unsigned>(highest_trainee_level_seen_)) /
			                             2))));
		}
		request_open_since_ = timeofgame;
	}
	if (soldier_request_ && need_more_soldiers) {
		if ((!requesting_weak_trainees_) && (!limit_upper_bound)) {
			// If requesting strong folks, the acceptance time can sometimes grow unbearable large
			// without this.
			// In request weak mode, resources are typically thin and this harms less, In addition,
			// the starting value tends to be much smaller in request-weak mode.
			dynamic_timeout =
			   acceptance_threshold_timeout /
			   std::max<uint32_t>(1, static_cast<unsigned>(trainee_general_lower_bound_));
			// In the special case of training not working at all, there is no need for this speedup
			// (hence the 2nd check)
		}
		if (0 == soldier_request_->get_num_transfers() &&
		    timeofgame > request_open_since_ + dynamic_timeout) {
			// Timeout: We have been asking for certain type of soldiers, nobody is answering the call.
			// Relaxing the criteria (and thus rebuild the request)
			rebuild_request = need_more_soldiers;
			if (0 < trainee_general_lower_bound_) {
				trainee_general_lower_bound_--;
				dynamic_timeout =
				   acceptance_threshold_timeout /
				   std::max<uint32_t>(1, static_cast<unsigned>(trainee_general_lower_bound_));
			} else if (requesting_weak_trainees_) {
				// If requesting weak trainees, and no people show up:
				// set the state back to request_strong, which will allow everybody in
				// when threshold is zero. Hopefully, you are fine with this misuse
				// of variable names.
				requesting_weak_trainees_ = false;
				latest_trainee_was_kickout_ = false;
			}
			if (kUpperBoundThreshold_ <= repeated_layoff_ctr_ && soldiers_.empty()) {
				// Repeated layoff ctr breaks the cycle when same few soldiers pendle back and forth.
				// If no soldiers are arriving and none are present, this cannot be the case.
				// Trainingsites without soldiers for long confuse players, thus retracting.
				repeated_layoff_ctr_ = 0;
				requesting_weak_trainees_ = false;
				latest_trainee_was_kickout_ = false;
			}
		}
	}

	if (!soldier_request_) {
		rebuild_request = need_more_soldiers;
	}

	if (rebuild_request) {
		// I've changed my acceptance criteria
		if (soldier_request_) {
			delete soldier_request_;
			soldier_request_ = nullptr;
		}

		assert(need_more_soldiers);
		if (recent_capacity_increase_) {
			// See comments in TrainingSite::SoldierControl::set_soldier_capacity() for details
			// In short: If user interacts, I accept anybody regardless of state.
			requesting_weak_trainees_ = false;
			limit_upper_bound = false;
			trainee_general_lower_bound_ = 0;
			recent_capacity_increase_ = false;
		}

		soldier_request_ = new Request(
		   *this, owner().tribe().soldier(), TrainingSite::request_soldier_callback, wwWORKER);

		RequireOr r;

		// set requirements to match this site
		if (descr().get_train_attack()) {
			// In "request weak trainees" mode, we ask for soldiers that are below stalled level
			if (requesting_weak_trainees_) {
				r.add(RequireAttribute(TrainingAttribute::kAttack,
				                       descr().get_min_level(TrainingAttribute::kAttack),
				                       get_max_unstall_level(TrainingAttribute::kAttack, descr())));
			} else {
				r.add(RequireAttribute(TrainingAttribute::kAttack,
				                       descr().get_min_level(TrainingAttribute::kAttack),
				                       descr().get_max_level(TrainingAttribute::kAttack)));
			}
		}
		if (descr().get_train_defense()) {
			if (requesting_weak_trainees_) {
				r.add(RequireAttribute(TrainingAttribute::kDefense,
				                       descr().get_min_level(TrainingAttribute::kDefense),
				                       get_max_unstall_level(TrainingAttribute::kDefense, descr())));
			} else {
				r.add(RequireAttribute(TrainingAttribute::kDefense,
				                       descr().get_min_level(TrainingAttribute::kDefense),
				                       descr().get_max_level(TrainingAttribute::kDefense)));
			}
		}
		if (descr().get_train_evade()) {
			if (requesting_weak_trainees_) {
				r.add(RequireAttribute(TrainingAttribute::kEvade,
				                       descr().get_min_level(TrainingAttribute::kEvade),
				                       get_max_unstall_level(TrainingAttribute::kEvade, descr())));
			} else {
				r.add(RequireAttribute(TrainingAttribute::kEvade,
				                       descr().get_min_level(TrainingAttribute::kEvade),
				                       descr().get_max_level(TrainingAttribute::kEvade)));
			}
		}
		if (descr().get_train_health()) {
			if (requesting_weak_trainees_) {
				r.add(RequireAttribute(TrainingAttribute::kHealth,
				                       descr().get_min_level(TrainingAttribute::kHealth),
				                       get_max_unstall_level(TrainingAttribute::kHealth, descr())));
			} else {
				r.add(RequireAttribute(TrainingAttribute::kHealth,
				                       descr().get_min_level(TrainingAttribute::kHealth),
				                       descr().get_max_level(TrainingAttribute::kHealth)));
			}
		}

		// The above selects everybody that could be trained here. If I am picky, then also exclude
		// those
		// that I could train but do not wish to spend time & resources on.
		if (limit_upper_bound) {
			RequireAnd qr;
			qr.add(RequireAttribute(TrainingAttribute::kTotal, 0, trainee_general_upper_bound));
			qr.add(r);
			soldier_request_->set_requirements(qr);
		} else if (0 < trainee_general_lower_bound_) {
			RequireAnd qr;
			qr.add(RequireAttribute(TrainingAttribute::kTotal, trainee_general_lower_bound_ + 1,
			                        std::numeric_limits<uint8_t>::max() - 1));
			qr.add(r);
			soldier_request_->set_requirements(qr);
			if (game) {
				schedule_act(*game, dynamic_timeout + Duration(1));
			}
		} else {
			soldier_request_->set_requirements(r);
		}
		soldier_request_->set_count(capacity_ - soldiers_.size());
		request_open_since_ = timeofgame;

	} else if (!need_more_soldiers) {
		delete soldier_request_;
		soldier_request_ = nullptr;

		while (soldiers_.size() > capacity_) {
			soldier_control_.drop_soldier(**soldiers_.rbegin());
		}
	} else {
		soldier_request_->set_count(capacity_ - soldiers_.size());
	}
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
                                            DescriptionIndex,
                                            Worker* const w,
                                            PlayerImmovable& target) {
	TrainingSite& tsite = dynamic_cast<TrainingSite&>(target);
	Soldier& s = dynamic_cast<Soldier&>(*w);

	assert(s.get_location(game) == &tsite);
	assert(tsite.soldier_request_ == &rq);

	tsite.soldier_control_.incorporate_soldier(game, s);
}

/**
 * Drop all the soldiers that can not be upgraded further at this building.
 */
void TrainingSite::drop_unupgradable_soldiers(Game&) {
	std::vector<Soldier*> droplist;

	for (Soldier* soldier : soldiers_) {
		std::vector<Upgrade>::iterator it = upgrades_.begin();
		for (; it != upgrades_.end(); ++it) {
			int32_t level = soldier->get_level(it->attribute);
			if (level >= it->min && level <= it->max) {
				break;
			}
		}

		if (it == upgrades_.end()) {
			droplist.push_back(soldier);
		}
	}

	// Drop soldiers only now, so that changes in the soldiers array don't
	// mess things up
	for (Soldier* soldier : droplist) {
		uint8_t level = soldier->get_level(TrainingAttribute::kTotal);
		if (level > highest_trainee_level_seen_) {
			highest_trainee_level_seen_ = level;
		}

		soldier_control_.drop_soldier(*soldier);
		repeated_layoff_ctr_ = 0;  // redundant, but safe (also reset whenever level increases)
		if (latest_trainee_was_kickout_) {
			// If I am calling in weaklings: Stop that. Immediately.
			latest_trainee_was_kickout_ = false;
			update_soldier_request(true);
		}
		repeated_layoff_inc_ = false;
	}
}

/**
 * Drop all the soldiers that can not be upgraded further at this level of resourcing.
 *
 */
void TrainingSite::drop_stalled_soldiers(Game&) {
	Soldier* soldier_to_drop = nullptr;
	uint8_t highest_soldier_level_seen = 0;

	for (Soldier* soldier : soldiers_) {
		uint8_t this_soldier_level = soldier->get_level(TrainingAttribute::kTotal);

		bool this_soldier_is_safe = false;
		if (this_soldier_level <= highest_soldier_level_seen) {
			// Skip the innermost loop for soldiers that would not be kicked out anyway.
			// level-zero soldiers are excepted from kick-out implicitly. This is intentional.
			this_soldier_is_safe = true;
		} else {
			for (const Upgrade& upgrade : upgrades_) {
				if (!this_soldier_is_safe) {
					// Soldier is safe, if he:
					//  - is below maximum, and
					//  - is not in a stalled state
					// Check done separately for each art.
					int32_t level = soldier->get_level(upgrade.attribute);

					// Below maximum -check
					if (level > upgrade.max) {
						continue;
					}

					TypeAndLevel train_tl(upgrade.attribute, level);
					TrainFailCount::iterator tstep = training_failure_count_.find(train_tl);
					if (tstep == training_failure_count_.end()) {
						log_warn("TrainingSite::drop_stalled_soldiers: training step %d,%d "
						         "not found in this school!\n",
						         static_cast<unsigned int>(upgrade.attribute), level);
						break;
					}

					tstep->second.second = 1;  // a soldier is present at this level

					// Stalled state -check
					if (max_stall_val_ > tstep->second.first) {
						this_soldier_is_safe = true;
						break;
					}
				}
			}
		}
		if (!this_soldier_is_safe) {
			// Make this soldier a kick-out candidate
			soldier_to_drop = soldier;
			highest_soldier_level_seen = this_soldier_level;
		}
	}

	// Finally drop the soldier.
	if (nullptr != soldier_to_drop) {
		verb_log_info("TrainingSite::drop_stalled_soldiers: Kicking somebody out.\n");
		uint8_t level = soldier_to_drop->get_level(TrainingAttribute::kTotal);
		if (level > highest_trainee_level_seen_) {
			highest_trainee_level_seen_ = level;
		}
		latest_trainee_kickout_level_ = level;
		soldier_control_.drop_soldier(*soldier_to_drop);
		latest_trainee_was_kickout_ = true;
		// We can enter into state where same soldiers repeatedly enter the site
		// even if they cannot be promited (lack of gold, lack of an equipmentsmith
		// of some kind or so). The repeated_layoff_ctr_ works around that.
		//
		// Only repeated drops with incorporating new soldiers in between causes this to happen!
		if (std::numeric_limits<uint8_t>::max() - 1 > repeated_layoff_ctr_ && repeated_layoff_inc_) {
			repeated_layoff_ctr_++;
			repeated_layoff_inc_ = false;
		}
	}
}

std::unique_ptr<const BuildingSettings> TrainingSite::create_building_settings() const {
	std::unique_ptr<TrainingsiteSettings> settings(
	   new TrainingsiteSettings(descr(), owner().tribe()));
	settings->apply(*ProductionSite::create_building_settings());
	settings->desired_capacity =
	   std::min(settings->max_capacity, soldier_control_.soldier_capacity());
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
	update_soldier_request(false);
}

void TrainingSite::program_end(Game& game, ProgramResult const result) {
	result_ = result;
	ProductionSite::program_end(game, result);
	// For unknown reasons sometimes there is a fully upgraded soldier
	// that failed to be send away, so at the end of this function
	// we test for such soldiers, unless another drop_soldiers
	// function were run
	bool leftover_soldiers_check = true;

	if (current_upgrade_) {
		if (result_ == ProgramResult::kCompleted) {
			drop_unupgradable_soldiers(game);
			leftover_soldiers_check = false;
			current_upgrade_->lastsuccess = true;
			current_upgrade_->failures = 0;

			// I try to already somewhat trained soldiers here, except when
			// no training happens. Now some training has happened, hence zero.
			// read in update_soldier_request
			repeated_layoff_ctr_ = 0;
			repeated_layoff_inc_ = false;
		} else {
			current_upgrade_->failures++;
			drop_stalled_soldiers(game);
			leftover_soldiers_check = false;
		}
		current_upgrade_ = nullptr;
	}

	if (leftover_soldiers_check) {
		drop_unupgradable_soldiers(game);
	}

	training_done();
}

/**
 * Find and start the next training program.
 *
 * Prioritize such that if UpgradeA.prio is twice UpgradeB.prio, then
 * start_upgrade will be called twice as often for UpgradeA.
 * If all priorities are zero, nothing will happen.
 */
void TrainingSite::find_and_start_next_program(Game& game) {
	checked_soldier_training_.level = INVALID_INDEX;
	checked_soldier_training_.attribute = TrainingAttribute::kTotal;
	for (;;) {
		uint32_t maxprio = 0;
		uint32_t maxcredit = 0;

		for (Upgrade& upgrade : upgrades_) {
			if (upgrade.credit >= 10) {
				upgrade.credit -= 10;
				return start_upgrade(game, upgrade);
			}

			if (maxprio < upgrade.prio) {
				maxprio = upgrade.prio;
			}
			if (maxcredit < upgrade.credit) {
				maxcredit = upgrade.credit;
			}
		}

		if (maxprio == 0) {
			return program_start(game, "sleep");
		}

		uint32_t const multiplier = 1 + (10 - maxcredit) / maxprio;

		for (Upgrade& upgrade : upgrades_) {
			upgrade.credit += multiplier * upgrade.prio;
		}
	}
}

/**
 * The prioritizer decided that the given type of upgrade should run.
 * Let's do our worst.
 */
void TrainingSite::start_upgrade(Game& game, Upgrade& upgrade) {
	int32_t minlevel = upgrade.max;
	int32_t maxlevel = upgrade.min;

	for (Soldier* soldier : soldiers_) {
		int32_t const level = soldier->get_level(upgrade.attribute);

		if (level > upgrade.max || level < upgrade.min) {
			continue;
		}
		if (level < minlevel) {
			minlevel = level;
		}
		if (level > maxlevel) {
			maxlevel = level;
		}
	}

	if (minlevel > maxlevel) {
		return program_start(game, "sleep");
	}

	int32_t level;

	if (upgrade.lastsuccess || upgrade.lastattempt < 0) {
		// Start greedily on the first ever attempt, and restart greedily
		// after a sucessful upgrade
		if (build_heroes_) {
			level = maxlevel;
		} else {
			level = minlevel;
		}
	} else {
		// The last attempt wasn't successful;
		// This happens e.g. when lots of low-level soldiers are present,
		// but the prerequisites for improving them aren't.
		if (build_heroes_) {
			level = upgrade.lastattempt - 1;
			if (level < minlevel) {
				level = maxlevel;
			}
		} else {
			level = upgrade.lastattempt + 1;
			if (level > maxlevel) {
				level = minlevel;
			}
		}
	}

	current_upgrade_ = &upgrade;
	upgrade.lastattempt = level;
	upgrade.lastsuccess = false;

	return program_start(game, format("%s%i", upgrade.prefix, level));
}

TrainingSite::Upgrade* TrainingSite::get_upgrade(TrainingAttribute const atr) {
	for (Upgrade& upgrade : upgrades_) {
		if (upgrade.attribute == atr) {
			return &upgrade;
		}
	}
	return nullptr;
}

/**
 * Gets the priority of given attribute
 */
int32_t TrainingSite::get_pri(TrainingAttribute atr) {
	for (const Upgrade& upgrade : upgrades_) {
		if (upgrade.attribute == atr) {
			return upgrade.prio;
		}
	}
	return 0;
}

/**
 * Sets the priority of given attribute
 */
void TrainingSite::set_pri(TrainingAttribute atr, int32_t prio) {
	if (prio < 0) {
		prio = 0;
	}

	for (Upgrade& upgrade : upgrades_) {
		if (upgrade.attribute == atr) {
			upgrade.prio = prio;
			return;
		}
	}
}

/**
 * Only called from \ref calc_upgrades
 */
void TrainingSite::add_upgrade(TrainingAttribute const atr, const std::string& prefix) {
	Upgrade u;
	u.attribute = atr;
	u.prefix = prefix;
	u.min = descr().get_min_level(atr);
	u.max = descr().get_max_level(atr);
	u.prio = 6;
	u.credit = 0;
	u.lastattempt = -1;
	u.lastsuccess = false;
	u.failures = 0;
	upgrades_.push_back(u);
}

/**
 * Called once at initialization to populate \ref upgrades_.
 */
void TrainingSite::calc_upgrades() {
	assert(upgrades_.empty());

	//  TODO(unknown): This is currently hardcoded for "soldier" but it should allow any
	//  soldier type name.
	if (descr().get_train_health()) {
		add_upgrade(TrainingAttribute::kHealth, "upgrade_soldier_health_");
	}
	if (descr().get_train_attack()) {
		add_upgrade(TrainingAttribute::kAttack, "upgrade_soldier_attack_");
	}
	if (descr().get_train_defense()) {
		add_upgrade(TrainingAttribute::kDefense, "upgrade_soldier_defense_");
	}
	if (descr().get_train_evade()) {
		add_upgrade(TrainingAttribute::kEvade, "upgrade_soldier_evade_");
	}
}

void TrainingSite::training_attempted(TrainingAttribute type, uint32_t level) {
	TypeAndLevel key(type, level);
	checked_soldier_training_.level = level;
	checked_soldier_training_.attribute = type;
	if (training_failure_count_.find(key) == training_failure_count_.end()) {
		training_failure_count_[key] = std::make_pair(training_state_multiplier_, 0);
	} else {
		training_failure_count_[key].first += training_state_multiplier_;
	}
}

/**
 * Called whenever it was possible to promote another guy
 */

void TrainingSite::training_successful(TrainingAttribute type, uint32_t level) {
	TypeAndLevel key(type, level);
	// Here I assume that key exists: training has been attempted before it can succeed.
	training_failure_count_[key].first = 0;
}

void TrainingSite::training_done() {
	for (auto& fail_and_presence : training_failure_count_) {
		// If a soldier is present at this training level and site is running, deteoriate
		if (fail_and_presence.second.second && (!is_stopped())) {
			fail_and_presence.second.first++;
			fail_and_presence.second.second = 0;
		} else if (0 < fail_and_presence.second.first) {  // If no soldier, let's become optimistic
			fail_and_presence.second.first--;
		}
	}
}

ProductionProgram::Action::TrainingParameters TrainingSite::checked_soldier_training() const {
	return checked_soldier_training_;
}
}  // namespace Widelands
