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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/militarysite.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "graphic/style_manager.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/message_queue.h"
#include "logic/player.h"

namespace {
constexpr size_t kNoOfStatisticsStringCases = 4;
}  // namespace

namespace Widelands {

std::vector<Soldier*> MilitarySite::SoldierControl::present_soldiers() const {
	std::vector<Soldier*> soldiers;

	for (Worker* worker : military_site_->get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			if (military_site_->is_present(*soldier)) {
				soldiers.push_back(soldier);
			}
		}
	}
	return soldiers;
}

std::vector<Soldier*> MilitarySite::SoldierControl::stationed_soldiers() const {
	std::vector<Soldier*> soldiers;

	for (Worker* worker : military_site_->get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			soldiers.push_back(soldier);
		}
	}
	return soldiers;
}

Quantity MilitarySite::SoldierControl::min_soldier_capacity() const {
	return 1;
}

Quantity MilitarySite::SoldierControl::max_soldier_capacity() const {
	return military_site_->descr().get_max_number_of_soldiers();
}

Quantity MilitarySite::SoldierControl::soldier_capacity() const {
	return military_site_->capacity_;
}

void MilitarySite::SoldierControl::set_soldier_capacity(uint32_t const capacity) {
	assert(min_soldier_capacity() <= capacity);
	assert(capacity <= max_soldier_capacity());
	if (military_site_->capacity_ != capacity) {
		military_site_->capacity_ = capacity;
		military_site_->update_soldier_request();
	}
}

void MilitarySite::SoldierControl::drop_soldier(Soldier& soldier) {
	Game& game = dynamic_cast<Game&>(military_site_->get_owner()->egbase());

	if (!military_site_->is_present(soldier)) {
		// This can happen when the "drop soldier" player command is delayed
		// by network delay or a client has bugs.
		military_site_->molog(
		   game.get_gametime(), "MilitarySite::drop_soldier(%u): not present\n", soldier.serial());
		return;
	}
	if (present_soldiers().size() <= min_soldier_capacity()) {
		military_site_->molog(game.get_gametime(), "cannot drop last soldier(s)\n");
		return;
	}

	soldier.reset_tasks(game);
	soldier.start_task_leavebuilding(game, true);

	military_site_->update_soldier_request();
}

int MilitarySite::SoldierControl::incorporate_soldier(EditorGameBase& egbase, Soldier& s) {
	if (s.get_location(egbase) != military_site_) {
		s.set_location(military_site_);
	}

	// Soldier upgrade is done once the site is full. In soldier upgrade, we
	// request one new soldier who is better suited than the existing ones.
	// Normally, I kick out one existing soldier as soon as a new guy starts walking
	// towards here. However, since that is done via infrequent polling, a new soldier
	// can sometimes reach the site before such kick-out happens. In those cases, we
	// should either drop one of the existing soldiers or reject the new guy, to
	// avoid overstocking this site.

	if (stationed_soldiers().size() > military_site_->descr().get_max_number_of_soldiers()) {
		return military_site_->incorporate_upgraded_soldier(egbase, s) ? 0 : -1;
	}

	if (!military_site_->didconquer_) {
		military_site_->conquer_area(egbase);
		// Building is now occupied - idle animation should be played
		military_site_->start_animation(
		   egbase, military_site_->descr().get_animation("idle", military_site_));

		if (upcast(Game, game, &egbase)) {
			military_site_->send_message(
			   *game, Message::Type::kEconomySiteOccupied, military_site_->descr().descname(),
			   military_site_->descr().icon_filename(), military_site_->descr().descname(),
			   military_site_->descr().occupied_str_, true);
		}
	}

	if (upcast(Game, game, &egbase)) {
		// Bind the worker into this house, hide him on the map
		s.reset_tasks(*game);
		s.start_task_buildingwork(*game);
	}

	// Make sure the request count is reduced or the request is deleted.
	military_site_->update_soldier_request(true);

	return 0;
}

bool MilitarySite::AttackTarget::can_be_attacked() const {
	return military_site_->didconquer_;
}

void MilitarySite::AttackTarget::enemy_soldier_approaches(const Soldier& enemy) const {
	Player* owner = military_site_->get_owner();
	Game& game = dynamic_cast<Game&>(owner->egbase());
	const Map& map = game.map();
	if (enemy.get_owner() == owner || (enemy.get_battle() != nullptr) ||
	    military_site_->descr().get_conquers() <=
	       map.calc_distance(enemy.get_position(), military_site_->get_position())) {
		return;
	}

	if (map.find_bobs(game,
	                  Area<FCoords>(map.get_fcoords(military_site_->base_flag().get_position()), 2),
	                  nullptr, FindBobEnemySoldier(owner)) != 0u) {
		return;
	}

	// We're dealing with a soldier that we might want to keep busy
	// Now would be the time to implement some player-definable
	// policy as to how many soldiers are allowed to leave as defenders
	std::vector<Soldier*> present = military_site_->soldier_control_.present_soldiers();

	if (1 < present.size()) {
		for (Soldier* temp_soldier : present) {
			if (!military_site_->has_soldier_job(*temp_soldier)) {
				SoldierJob sj;
				sj.soldier = temp_soldier;
				sj.enemy = &enemy;
				sj.stayhome = false;
				military_site_->soldierjobs_.push_back(sj);
				temp_soldier->update_task_buildingwork(game);
				return;
			}
		}
	}

	// Inform the player, that we are under attack by adding a new entry to the
	// message queue - a sound will automatically be played.
	military_site_->notify_player(game, true);
}

AttackTarget::AttackResult MilitarySite::AttackTarget::attack(Soldier* enemy) const {
	Game& game = dynamic_cast<Game&>(military_site_->get_owner()->egbase());

	std::vector<Soldier*> present = military_site_->soldier_control_.present_soldiers();
	Soldier* defender = nullptr;

	if (!present.empty()) {
		// Find soldier with greatest health
		uint32_t current_max = 0;
		for (Soldier* temp_soldier : present) {
			if (temp_soldier->get_current_health() > current_max) {
				defender = temp_soldier;
				current_max = defender->get_current_health();
			}
		}
	} else {
		// If one of our stationed soldiers is currently walking into the
		// building, give us another chance.
		std::vector<Soldier*> stationed = military_site_->soldier_control_.stationed_soldiers();
		for (Soldier* temp_soldier : stationed) {
			if (temp_soldier->get_position() == military_site_->get_position()) {
				defender = temp_soldier;
				break;
			}
		}
	}

	if (defender != nullptr) {
		military_site_->pop_soldier_job(defender);  // defense overrides all other jobs

		SoldierJob sj;
		sj.soldier = defender;
		sj.enemy = enemy;
		sj.stayhome = true;
		military_site_->soldierjobs_.push_back(sj);

		defender->update_task_buildingwork(game);

		// Inform the player, that we are under attack by adding a new entry to
		// the message queue - a sound will automatically be played.
		military_site_->notify_player(game);

		return AttackTarget::AttackResult::DefenderLaunched;
	}

	// The enemy has defeated our forces, we should inform the player
	const Coords coords = military_site_->get_position();
	{
		military_site_->send_message(game, Message::Type::kWarfareSiteLost,
		                             /** TRANSLATORS: Militarysite lost (taken/destroyed by enemy) */
		                             pgettext("building", "Lost!"),
		                             military_site_->descr().icon_filename(), _("Militarysite lost!"),
		                             military_site_->descr().defeated_enemy_str_, false);
	}

	// Now let's see whether the enemy conquers our militarysite, or whether
	// we still hold the bigger military presence in that area (e.g. if there
	// is a fortress one or two points away from our sentry, the fortress has
	// a higher presence and thus the enemy can just burn down the sentry.
	if (!get_allow_conquer(enemy->owner().player_number()) ||
	    military_site_->military_presence_kept(game)) {
		// Okay we still got the higher military presence, so the attacked
		// militarysite will be destroyed.
		military_site_->set_defeating_player(enemy->owner().player_number());
		military_site_->schedule_destroy(game);
		return AttackTarget::AttackResult::Defenseless;
	}

	// The enemy conquers the building
	// In fact we do not conquer it, but place a new building of same type at
	// the old location. We need to take a copy.
	const FormerBuildings former_buildings = military_site_->old_buildings_;

	// The enemy conquers the building
	// In fact we do not conquer it, but place a new building of same type at
	// the old location.
	Player* enemyplayer = enemy->get_owner();

	// Now we destroy the old building before we place the new one.
	// Waiting for the destroy playercommand causes crashes with the building window, so we need to
	// close it right away.
	military_site_->set_defeating_player(enemyplayer->player_number());
	military_site_->schedule_destroy(game);

	Building* const newbuilding = &enemyplayer->force_building(coords, former_buildings);
	upcast(MilitarySite, newsite, newbuilding);
	newsite->reinit_after_conqueration(game);

	// Of course we should inform the victorious player as well
	newsite->send_message(
	   game, Message::Type::kWarfareSiteDefeated,
	   /** TRANSLATORS: Message title. */
	   /** TRANSLATORS: If you run out of space, you can also translate this as "Success!" */
	   _("Enemy Defeated!"), newsite->descr().icon_filename(), _("Enemy at site defeated!"),
	   newsite->descr().defeated_you_str_, true);

	return AttackTarget::AttackResult::Defenseless;
}

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/militarysites/atlanteans/castle/init.lua
 */
MilitarySiteDescr::MilitarySiteDescr(const std::string& init_descname,
                                     const LuaTable& table,
                                     Descriptions& descriptions)
   : BuildingDescr(init_descname, MapObjectType::MILITARYSITE, table, descriptions),
     conquer_radius_(0),
     num_soldiers_(0),
     heal_per_second_(0) {

	conquer_radius_ = table.get_int("conquers");
	num_soldiers_ = table.get_int("max_soldiers");
	heal_per_second_ = table.get_int("heal_per_second");

	if (conquer_radius_ > 0) {
		workarea_info_[conquer_radius_].insert(name() + " conquer");
	}
	prefers_heroes_at_start_ = table.get_bool("prefer_heroes");

	std::unique_ptr<LuaTable> items_table = table.get_table("messages");
	occupied_str_ = items_table->get_string("occupied");
	aggressor_str_ = items_table->get_string("aggressor");
	attack_str_ = items_table->get_string("attack");
	defeated_enemy_str_ = items_table->get_string("defeated_enemy");
	defeated_you_str_ = items_table->get_string("defeated_you");
}

/**
===============
Create a new building of this type
===============
*/
Building& MilitarySiteDescr::create_object() const {
	return *new MilitarySite(*this);
}

/*
=============================

class MilitarySite

=============================
*/

MilitarySite::MilitarySite(const MilitarySiteDescr& ms_descr)
   : Building(ms_descr),
     attack_target_(this),
     soldier_control_(this),
     didconquer_(false),
     capacity_(ms_descr.get_max_number_of_soldiers()),
     nexthealtime_(0),
     soldier_preference_(ms_descr.prefers_heroes_at_start_ ? SoldierPreference::kHeroes :
                                                             SoldierPreference::kRookies),
     next_swap_soldiers_time_(Time(0)),
     soldier_upgrade_try_(false),
     doing_upgrade_request_(false),
     // Initialize vector capacity for statistics string cache
     statistics_string_cache_(kNoOfStatisticsStringCases) {
	set_attack_target(&attack_target_);
	set_soldier_control(&soldier_control_);
}

MilitarySite::~MilitarySite() {
	assert(!normal_soldier_request_);
	assert(!upgrade_soldier_request_);
}

/**
===============
Display number of soldiers.
===============
*/
void MilitarySite::update_statistics_string(std::string* s) {
	s->clear();
	Quantity present = soldier_control_.present_soldiers().size();
	Quantity stationed = soldier_control_.stationed_soldiers().size();

	// Use Lua script to generate the capacity string for the given number of stationed and present
	// soldiers. The index is according to the conditions determined by the caller and ranges [0,
	// kNoOfStatisticsStringCases - 1].
	auto read_capacity_string = [this](Quantity pres, Quantity stat, size_t idx) {
		assert(idx < kNoOfStatisticsStringCases);
		std::tuple<int, int, int> cache_key(pres, stat, capacity_);
		auto it = statistics_string_cache_[idx].find(cache_key);
		if (it != statistics_string_cache_[idx].end()) {
			return it->second;
		}
		const std::string& military_capacity_script = owner().tribe().military_capacity_script();
		// TODO(GunChleoc): API compatibility - require file exists in TribeDescr after v1.0
		if (!military_capacity_script.empty() && g_fs->file_exists(military_capacity_script)) {
			try {
				LuaInterface lua;
				std::unique_ptr<LuaTable> table(lua.run_script(military_capacity_script));
				std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
				cr->push_arg(pres);
				cr->push_arg(stat);
				cr->push_arg(capacity_);
				cr->resume();
				std::string new_string = cr->pop_string();
				statistics_string_cache_[idx].insert(std::make_pair(cache_key, new_string));
				return new_string;
			} catch (LuaError& err) {
				log_err("Failed to read soldier capacity for building '%s': %s", descr().name().c_str(),
				        err.what());
				return std::string();
			}
		}
		return std::string();
	};

	// TODO(GunChleoc): API compatibility - require file exists in TribeDescr after v 1.0 and remove
	// fallbacks to tribe-independent strings
	if (present == stationed) {
		if (capacity_ > stationed) {
			*s = read_capacity_string(present, stationed, 0);
			if (s->empty()) {
				/** TRANSLATORS: %1% is the number of soldiers the plural refers to. %2% is the maximum
				 * number of soldier slots in the building */
				*s = format(ngettext("%1% soldier (+%2%)", "%1% soldiers (+%2%)", stationed), stationed,
				            (capacity_ - stationed));
			}
		} else {
			*s = read_capacity_string(present, stationed, 1);
			if (s->empty()) {
				/** TRANSLATORS: Number of soldiers stationed at a militarysite. */
				*s = format(ngettext("%1% soldier", "%1% soldiers", stationed), stationed);
			}
		}
	} else {
		if (capacity_ > stationed) {
			*s = read_capacity_string(present, stationed, 2);
			if (s->empty()) {
				*s = format(
				   /** TRANSLATORS: %1% is the number of soldiers the plural refers to. %2% are
				      currently open soldier slots in the building. %3% is the maximum number of
				      soldier slots in the building */
				   ngettext("%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", stationed),
				   present, (stationed - present), (capacity_ - stationed));
			}
		} else {
			*s = read_capacity_string(present, stationed, 3);
			if (s->empty()) {
				/** TRANSLATORS: %1% is the number of soldiers the plural refers to. %2% are currently
				 * open soldier slots in the building */
				*s = format(ngettext("%1%(+%2%) soldier", "%1%(+%2%) soldiers", stationed), present,
				            (stationed - present));
			}
		}
	}

	*s = StyleManager::color_tag(
	   // Line break to make Codecheck happy.
	   *s, g_style_manager->building_statistics_style().medium_color());
}

bool MilitarySite::init(EditorGameBase& egbase) {
	Building::init(egbase);

	upcast(Game, game, &egbase);

	for (Worker* worker : get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			soldier->set_location_initially(*this);
			assert(!soldier->get_state());  //  Should be newly created.
			if (game != nullptr) {
				soldier->start_task_buildingwork(*game);
			}
		}
	}
	update_soldier_request();

	//  schedule the first healing
	nexthealtime_ = egbase.get_gametime() + Duration(1000);
	if (game != nullptr) {
		schedule_act(*game, Duration(1000));
	}
	return true;
}

/**
===============
Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy* const e, WareWorker type) {
	Building::set_economy(e, type);

	if (normal_soldier_request_ && (e != nullptr) && type == normal_soldier_request_->get_type()) {
		normal_soldier_request_->set_economy(e);
	}
	if (upgrade_soldier_request_ && (e != nullptr) && type == upgrade_soldier_request_->get_type()) {
		upgrade_soldier_request_->set_economy(e);
	}
}

/**
===============
Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(EditorGameBase& egbase) {
	// unconquer land
	if (didconquer_) {
		egbase.unconquer_area(
		   PlayerArea<Area<FCoords>>(
		      owner().player_number(),
		      Area<FCoords>(egbase.map().get_fcoords(get_position()), descr().get_conquers())),
		   defeating_player_);
	}

	Building::cleanup(egbase);

	// Evict soldiers to get rid of requests
	while (capacity_ > 0) {
		update_soldier_request();
		--capacity_;
	}
	update_soldier_request();

	normal_soldier_request_.reset();
	upgrade_soldier_request_.reset();
}

/*
 * Returns the least wanted soldier -- If player prefers zero-level guys,
 * the most trained soldier is the "weakest guy".
 */

Soldier* MilitarySite::find_least_suited_soldier() {
	const std::vector<Soldier*> present = soldier_control_.present_soldiers();
	const int32_t multiplier = SoldierPreference::kHeroes == soldier_preference_ ? -1 : 1;
	int worst_soldier_level = INT_MIN;
	Soldier* worst_soldier = nullptr;
	for (Soldier* sld : present) {
		int this_soldier_level =
		   multiplier * static_cast<int>(sld->get_level(TrainingAttribute::kTotal));
		if (this_soldier_level > worst_soldier_level) {
			worst_soldier_level = this_soldier_level;
			worst_soldier = sld;
		}
	}
	return worst_soldier;
}

/*
 * Kicks out the least wanted soldier -- If player prefers zero-level guys,
 * the most trained soldier is the "weakest guy".
 *
 * Returns false, if there is only one soldier (won't drop last soldier)
 * or the new guy is not better than the weakest one present, in which case
 * nobody is dropped. If a new guy arrived and nobody was dropped, then
 * caller of this should not allow the new guy to enter.
 *
 * Situations like the above may happen if, for example, when weakest guy
 * that was supposed to be dropped is not present at the moment his replacement
 * arrives.
 */

bool MilitarySite::drop_least_suited_soldier(bool new_soldier_has_arrived, Soldier* newguy) {
	const std::vector<Soldier*> present = soldier_control_.present_soldiers();

	// If I have only one soldier, and the  new guy is not here yet, I can't release.
	if (new_soldier_has_arrived || 1 < present.size()) {
		Soldier* kickoutCandidate = find_least_suited_soldier();

		// If the arriving guy is worse than worst present, I wont't release.
		if (nullptr != newguy && nullptr != kickoutCandidate) {
			int32_t old_level = kickoutCandidate->get_level(TrainingAttribute::kTotal);
			int32_t new_level = newguy->get_level(TrainingAttribute::kTotal);
			if (SoldierPreference::kHeroes == soldier_preference_ && old_level >= new_level) {
				return false;
			}
			if (SoldierPreference::kRookies == soldier_preference_ && old_level <= new_level) {
				return false;
			}
		}

		// Now I know that the new guy is worthy.
		if (nullptr != kickoutCandidate) {
			Game& game = dynamic_cast<Game&>(get_owner()->egbase());
			kickoutCandidate->reset_tasks(game);
			kickoutCandidate->start_task_leavebuilding(game, true);
			return true;
		}
	}
	return false;
}

/*
 * This finds room for a soldier in an already full occupied military building.
 *
 * Returns false if the soldier was not incorporated.
 */
bool MilitarySite::incorporate_upgraded_soldier(EditorGameBase& egbase, Soldier& s) {
	// Call to drop_least routine has side effects: it tries to drop a soldier. Order is important!
	if (soldier_control_.stationed_soldiers().size() < capacity_ ||
	    drop_least_suited_soldier(true, &s)) {
		Game& game = dynamic_cast<Game&>(egbase);
		s.set_location(this);
		s.reset_tasks(game);
		s.start_task_buildingwork(game);
		return true;
	}
	return false;
}

/*
===============
Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback(Game& game,
                                            Request& /* req */,
                                            DescriptionIndex /* index */,
                                            Worker* const w,
                                            PlayerImmovable& target) {
	MilitarySite& msite = dynamic_cast<MilitarySite&>(target);
	Soldier& s = dynamic_cast<Soldier&>(*w);

	msite.soldier_control_.incorporate_soldier(game, s);
}

/**
 * Update the request for soldiers and cause soldiers to be evicted
 * as appropriate.
 */
void MilitarySite::update_normal_soldier_request() {
	std::vector<Soldier*> present = soldier_control_.present_soldiers();
	Quantity const stationed = soldier_control_.stationed_soldiers().size();

	if (stationed < capacity_) {
		if (!normal_soldier_request_) {
			normal_soldier_request_.reset(new Request(
			   *this, owner().tribe().soldier(), MilitarySite::request_soldier_callback, wwWORKER));
			normal_soldier_request_->set_requirements(soldier_requirements_);
		}

		normal_soldier_request_->set_count(capacity_ - stationed);
	} else {
		normal_soldier_request_.reset();
	}

	if (capacity_ < present.size()) {
		Game& game = dynamic_cast<Game&>(get_owner()->egbase());
		for (uint32_t i = 0; i < present.size() - capacity_; ++i) {
			Soldier& soldier = *present[i];
			soldier.reset_tasks(game);
			soldier.start_task_leavebuilding(game, true);
		}
	}
}

/* There are two kinds of soldier requests: "normal", which is used whenever the military site needs
 * more soldiers, and "upgrade" which is used when there is a preference for either heroes or
 * rookies.
 *
 * In case of normal requests, the military site is filled. In case of upgrade requests, only one
 * guy
 * is exchanged at a time.
 *
 * There would be more efficient ways to get well trained soldiers. Now, new buildings appearing in
 * battle
 * field are more vulnerable at the beginning. This is intentional. The purpose of this upgrade
 * thing is
 * to reduce the benefits of site micromanagement. The intention is not to make gameplay easier in
 * other ways.
 */
void MilitarySite::update_upgrade_soldier_request() {
	bool reqch = update_upgrade_requirements();
	if (!soldier_upgrade_try_) {
		return;
	}

	bool do_rebuild_request = reqch;

	if (upgrade_soldier_request_) {
		if (!upgrade_soldier_request_->is_open()) {
			// If a replacement is already walking this way, let's not change our minds.
			do_rebuild_request = false;
		}
		if (0 == upgrade_soldier_request_->get_count()) {
			do_rebuild_request = true;
		}
	} else {
		do_rebuild_request = true;
	}

	if (do_rebuild_request) {
		upgrade_soldier_request_.reset(new Request(
		   *this, owner().tribe().soldier(), MilitarySite::request_soldier_callback, wwWORKER));

		upgrade_soldier_request_->set_requirements(soldier_upgrade_requirements_);
		upgrade_soldier_request_->set_count(1);
	}
}

/*
 * I have update_soldier_request
 *        update_upgrade_soldier_request
 *        update_normal_soldier_request
 *
 * The first one handles state switching between site fill (normal more)
 * and grabbing soldiers with proper training (upgrade mode). The last
 * two actually make the requests.
 *
 * The input parameter incd is true, if we just incorporated a new soldier
 * as a result of an upgrade request. In such cases, we will re-arm the
 * upgrade request.
 */

void MilitarySite::update_soldier_request(bool incd) {
	const uint32_t capacity = soldier_control_.soldier_capacity();
	const uint32_t stationed = soldier_control_.stationed_soldiers().size();

	if (doing_upgrade_request_) {
		if (incd && upgrade_soldier_request_)  // update requests always ask for one soldier at time!
		{
			upgrade_soldier_request_.reset();
		}
		if (capacity > stationed) {
			// Somebody is killing my soldiers in the middle of upgrade
			// or I have kicked out his predecessor already.
			if (upgrade_soldier_request_ &&
			    (upgrade_soldier_request_->is_open() || 0 == upgrade_soldier_request_->get_count())) {

				// Economy was not able to find the soldiers I need.
				// I can safely drop the upgrade request and go to fill mode.
				upgrade_soldier_request_.reset();
			}
			if (!upgrade_soldier_request_) {
				// phoo -- I can safely request new soldiers.
				doing_upgrade_request_ = false;
				update_normal_soldier_request();
			}
			// else -- ohno please help me! Player is in trouble -- evil grin
		} else                        // military site is full or overfull
		   if (capacity < stationed)  // player is reducing capacity
		{
			drop_least_suited_soldier(false, nullptr);
		} else  // capacity == stationed size
		{
			if (upgrade_soldier_request_ && (!(upgrade_soldier_request_->is_open())) &&
			    1 == upgrade_soldier_request_->get_count() && (!incd)) {
				drop_least_suited_soldier(false, nullptr);
			} else {
				update_upgrade_soldier_request();
			}
		}
	} else  // not doing upgrade request
	{
		if ((capacity != stationed) || (normal_soldier_request_)) {
			update_normal_soldier_request();
		}

		if ((capacity == stationed) && (!normal_soldier_request_)) {
			if (soldier_control_.present_soldiers().size() == capacity) {
				doing_upgrade_request_ = true;
				update_upgrade_soldier_request();
			}
			// Note -- if there are non-present stationed soldiers, nothing gets
			// called. Therefore, I revisit this routine periodically without apparent
			// reason, hoping that all stationed soldiers would be present.
		}
	}
}

/*
===============
Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game& game, uint32_t const data) {
	// TODO(unknown): do all kinds of stuff, but if you do nothing, let
	// Building::act() handle all this. Also note, that some Building
	// commands rely, that Building::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.

	Building::act(game, data);

	const Time& timeofgame = game.get_gametime();
	if (normal_soldier_request_ && upgrade_soldier_request_) {
		throw wexception(
		   "MilitarySite::act: Two soldier requests are ongoing -- should never happen!\n");
	}

	// I do not get a callback when stationed, non-present soldier returns --
	// Therefore I must poll in some occasions. Let's do that rather infrequently,
	// to keep the game lightweight.

	// TODO(unknown): I would need two new callbacks, to get rid ot this polling.
	if (timeofgame > next_swap_soldiers_time_) {
		next_swap_soldiers_time_ = timeofgame + Duration(soldier_upgrade_try_ ? 20000 : 100000);
		update_soldier_request();
	}

	// Heal soldiers
	if (nexthealtime_ <= timeofgame) {
		const unsigned total_heal = descr().get_heal_per_second();
		uint32_t max_total_level = 0;
		float max_health = 0;
		Soldier* soldier_to_heal = nullptr;

		for (Soldier* soldier : soldier_control_.stationed_soldiers()) {
			if (soldier->get_current_health() < soldier->get_max_health()) {
				if (is_present(*soldier)) {
					// The healing algorithm for present soldiers is:
					// * heal soldier with highest total level
					// * heal healthiest if multiple of same total level exist
					if (soldier_to_heal == nullptr || soldier->get_total_level() > max_total_level ||
					    (soldier->get_total_level() == max_total_level &&
					     soldier->get_current_health() / static_cast<float>(soldier->get_max_health()) >
					        max_health)) {
						max_total_level = soldier->get_total_level();
						max_health =
						   soldier->get_current_health() / static_cast<float>(soldier->get_max_health());
						soldier_to_heal = soldier;
					}
				} else if ((soldier->get_battle() == nullptr ||
				            soldier->get_battle()->opponent(*soldier) == nullptr) &&
				           !get_economy(WareWorker::wwWORKER)->warehouses().empty()) {
					// Somewhat heal soldiers in the field that are not currently engaged in fighting an
					// opponent, but only if there is a warehouse connected.
					const PlayerNumber field_owner = soldier->get_position().field->get_owned_by();
					if (owner().player_number() == field_owner) {
						const unsigned int air_distance =
						   game.map().calc_distance(get_position(), soldier->get_position());
						const unsigned int heal_with_factor =
						   total_heal * descr().get_conquers() / std::max(air_distance * 4U, 1U);
						soldier->heal(std::min(total_heal, heal_with_factor));
					}
				}
			}
		}

		if (soldier_to_heal != nullptr) {
			soldier_to_heal->heal(total_heal);
		}

		nexthealtime_ = timeofgame + Duration(1000);
		schedule_act(game, Duration(1000));
	}
}

/**
 * The worker is about to be removed.
 *
 * After the removal of the worker, check whether we need to request
 * new soldiers.
 */
void MilitarySite::remove_worker(Worker& w) {
	Building::remove_worker(w);

	if (upcast(Soldier, soldier, &w)) {
		pop_soldier_job(soldier);
	}

	update_soldier_request();
}

/**
 * Called by soldiers in the building.
 */
bool MilitarySite::get_building_work(Game& game, Worker& worker, bool /*success*/) {
	if (upcast(Soldier, soldier, &worker)) {
		// Evict soldiers that have returned home if the capacity is too low
		if (capacity_ < soldier_control_.present_soldiers().size()) {
			worker.reset_tasks(game);
			worker.start_task_leavebuilding(game, true);
			return true;
		}

		bool stayhome;
		if (MapObject* const enemy = pop_soldier_job(soldier, &stayhome)) {
			if (upcast(Building, building, enemy)) {
				soldier->start_task_attack(game, *building);
				return true;
			}
			if (upcast(Soldier, opponent, enemy)) {
				if (opponent->get_battle() == nullptr) {
					soldier->start_task_defense(game, stayhome);
					if (stayhome) {
						opponent->send_signal(game, "sleep");
					}
					return true;
				}
			} else {
				throw wexception("MilitarySite::get_building_work: bad SoldierJob");
			}
		}
	}

	return false;
}

/**
 * \return \c true if the soldier is currently present and idle in the building.
 */
bool MilitarySite::is_present(Soldier& soldier) const {
	return soldier.get_location(get_owner()->egbase()) == this &&
	       soldier.get_state() == soldier.get_state(Worker::taskBuildingwork) &&
	       soldier.get_position() == get_position();
}

void MilitarySite::conquer_area(EditorGameBase& egbase) {
	assert(!didconquer_);
	egbase.conquer_area(PlayerArea<Area<FCoords>>(
	   owner().player_number(),
	   Area<FCoords>(egbase.map().get_fcoords(get_position()), descr().get_conquers())));
	didconquer_ = true;
}

/// Initialises the militarysite after it was "conquered" (the old was replaced)
void MilitarySite::reinit_after_conqueration(Game& game) {
	clear_requirements();
	conquer_area(game);
	update_soldier_request();
	start_animation(game, descr().get_animation("idle", this));

	// feature request 1247384 in launchpad bugs: Conquered buildings tend to
	// be in a hostile area; typically players want heroes there.
	set_soldier_preference(SoldierPreference::kHeroes);
}

/// Calculates whether the military presence is still kept and \returns true if.
bool MilitarySite::military_presence_kept(Game& game) {
	// collect information about immovables in the area
	std::vector<ImmovableFound> immovables;

	// Search in a radius of 3 (needed for big militarysites)
	FCoords const fc = game.map().get_fcoords(get_position());
	game.map().find_immovables(game, Area<FCoords>(fc, 3), &immovables);

	for (const ImmovableFound& imm : immovables) {
		if (upcast(MilitarySite const, militarysite, imm.object)) {
			if (this != militarysite && &owner() == &militarysite->owner() &&
			    get_size() <= militarysite->get_size() && militarysite->didconquer_) {
				return true;
			}
		}
	}
	return false;
}

/// Informs the player about an attack of his opponent.
void MilitarySite::notify_player(Game& game, bool const discovered) {
	// Add a message as long as no previous message was send from a point with
	// radius <= 5 near the current location in the last 60 seconds
	send_message(game, Message::Type::kWarfareUnderAttack,
	             /** TRANSLATORS: Militarysite is being attacked */
	             pgettext("building", "Attack!"), descr().icon_filename(), _("You are under attack"),
	             discovered ? descr().aggressor_str_ : descr().attack_str_, false,
	             Duration(60 * 1000), 5);
}

/*
   MilitarySite::set_requirements

   Easy to use, overwrite with given requirements.
*/
void MilitarySite::set_requirements(const Requirements& r) {
	soldier_requirements_ = r;
}

/*
   MilitarySite::clear_requirements

   This should cancel any requirement pushed at this house
*/
void MilitarySite::clear_requirements() {
	soldier_requirements_ = Requirements();
}

void MilitarySite::send_attacker(Soldier& soldier, Building& target) {
	if (!is_present(soldier)) {
		// The soldier may not be present anymore due to having been kicked out. Most of the time
		// the function calling us will notice this, but there are cornercase where it might not,
		// e.g. when a soldier was ordered to leave but did not physically quit the building yet.
		log_warn_time(
		   owner().egbase().get_gametime(),
		   "MilitarySite(%3dx%3d)::send_attacker: Not sending soldier %u because he left the "
		   "building\n",
		   get_position().x, get_position().y, soldier.serial());
		return;
	}

	if (has_soldier_job(soldier)) {
		return;
	}

	SoldierJob sj;
	sj.soldier = &soldier;
	sj.enemy = &target;
	sj.stayhome = false;
	soldierjobs_.push_back(sj);

	soldier.update_task_buildingwork(dynamic_cast<Game&>(get_owner()->egbase()));
}

bool MilitarySite::has_soldier_job(Soldier& soldier) {
	for (const SoldierJob& temp_job : soldierjobs_) {
		if (temp_job.soldier == &soldier) {
			return true;
		}
	}
	return false;
}

/**
 * \return the enemy, if any, that the given soldier was scheduled
 * to attack, and remove the job.
 */
MapObject* MilitarySite::pop_soldier_job(Soldier* const soldier, bool* const stayhome) {
	for (std::vector<SoldierJob>::iterator job_iter = soldierjobs_.begin();
	     job_iter != soldierjobs_.end(); ++job_iter) {
		if (job_iter->soldier == soldier) {
			MapObject* const enemy = job_iter->enemy.get(owner().egbase());
			if (stayhome != nullptr) {
				*stayhome = job_iter->stayhome;
			}
			soldierjobs_.erase(job_iter);
			return enemy;
		}
	}
	return nullptr;
}

/*
 * When upgrading soldiers, we do not ask for just any soldiers, but soldiers
 * that are better than what we already have. This routine sets the requirements
 * used by the request.
 *
 * The routine returns true if upgrade request thresholds have changed. This information could be
 * used to decide whether the soldier-Request should be upgraded.
 */
bool MilitarySite::update_upgrade_requirements() {
	int32_t soldier_upgrade_required_min = soldier_upgrade_requirements_.get_min();
	int32_t soldier_upgrade_required_max = soldier_upgrade_requirements_.get_max();

	// Find the level of the soldier that is currently least-suited.
	Soldier* worst_guy = find_least_suited_soldier();
	if (worst_guy == nullptr) {
		// There could be no soldier in the militarysite right now. No reason to freak out.
		return false;
	}
	int32_t wg_level = worst_guy->get_level(TrainingAttribute::kTotal);

	// Micro-optimization: I assume that the majority of military sites have only level-zero
	// soldiers and prefer rookies. Handle them separately.
	soldier_upgrade_try_ = true;
	if (SoldierPreference::kRookies == soldier_preference_) {
		if (0 == wg_level) {
			soldier_upgrade_try_ = false;
			return false;
		}
	}

	// Now I actually build the new requirements.
	int32_t reqmin = SoldierPreference::kHeroes == soldier_preference_ ? 1 + wg_level : 0;
	int32_t reqmax = SoldierPreference::kHeroes == soldier_preference_ ? SHRT_MAX : wg_level - 1;

	bool maxchanged = reqmax != soldier_upgrade_required_max;
	bool minchanged = reqmin != soldier_upgrade_required_min;

	if (maxchanged || minchanged) {
		if (upgrade_soldier_request_ && (upgrade_soldier_request_->is_open())) {
			upgrade_soldier_request_.reset();
		}
		soldier_upgrade_requirements_ = RequireAttribute(TrainingAttribute::kTotal, reqmin, reqmax);

		return true;
	}

	return false;
}

std::unique_ptr<const BuildingSettings> MilitarySite::create_building_settings() const {
	std::unique_ptr<MilitarysiteSettings> settings(
	   new MilitarysiteSettings(descr(), owner().tribe()));
	settings->desired_capacity =
	   std::min(settings->max_capacity, soldier_control_.soldier_capacity());
	settings->prefer_heroes = soldier_preference_ == SoldierPreference::kHeroes;
	// Prior to the resolution of a defect report against ISO C++11, local variable 'settings' would
	// have been copied despite being returned by name, due to its not matching the function return
	// type. Call 'std::move' explicitly to avoid copying on older compilers.
	// On modern compilers a simple 'return settings;' would've been fine.
	return std::unique_ptr<const BuildingSettings>(std::move(settings));
}

// setters

void MilitarySite::set_soldier_preference(SoldierPreference p) {
	soldier_preference_ = p;
	next_swap_soldiers_time_ = Time(0);
}
}  // namespace Widelands
