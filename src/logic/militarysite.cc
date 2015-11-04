/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "logic/militarysite.h"

#include <clocale>
#include <cstdio>
#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "logic/battle.h"
#include "logic/editor_game_base.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/worker.h"

namespace Widelands {

MilitarySiteDescr::MilitarySiteDescr(const std::string& init_descname,
												 const LuaTable& table,
												 const EditorGameBase& egbase)
	:
	ProductionSiteDescr
		(init_descname, "", MapObjectType::MILITARYSITE, table, egbase),
	m_conquer_radius     (0),
	m_num_soldiers       (0),
	m_heal_per_second    (0)
{
	i18n::Textdomain td("tribes");

	m_conquer_radius      = table.get_int("conquers");
	m_num_soldiers        = table.get_int("max_soldiers");
	m_heal_per_second     = table.get_int("heal_per_second");

	if (m_conquer_radius > 0)
		m_workarea_info[m_conquer_radius].insert(descname() + " conquer");
	m_prefers_heroes_at_start = table.get_bool("prefer_heroes");

	std::unique_ptr<LuaTable> items_table = table.get_table("messages");
	m_occupied_str = _(items_table->get_string("occupied"));
	m_aggressor_str = _(items_table->get_string("aggressor"));
	m_attack_str = _(items_table->get_string("attack"));
	m_defeated_enemy_str = _(items_table->get_string("defeated_enemy"));
	m_defeated_you_str = _(items_table->get_string("defeated_you"));
}

/**
===============
Create a new building of this type
===============
*/
Building & MilitarySiteDescr::create_object() const {
	return *new MilitarySite(*this);
}


/*
=============================

class MilitarySite

=============================
*/

MilitarySite::MilitarySite(const MilitarySiteDescr & ms_descr) :
ProductionSite(ms_descr),
m_didconquer  (false),
m_capacity    (ms_descr.get_max_number_of_soldiers()),
m_nexthealtime(0),
m_soldier_preference(ms_descr.m_prefers_heroes_at_start ? kPrefersHeroes : kPrefersRookies),
m_soldier_upgrade_try(false),
m_doing_upgrade_request(false)
{
	m_next_swap_soldiers_time = 0;
}


MilitarySite::~MilitarySite()
{
	assert(!m_normal_soldier_request);
	assert(!m_upgrade_soldier_request);
}


/**
===============
Display number of soldiers.
===============
*/
void MilitarySite::update_statistics_string(std::string* s)
{
	s->clear();
	uint32_t present = present_soldiers().size();
	uint32_t stationed = stationed_soldiers().size();

	if (present == stationed) {
		if (m_capacity > stationed) {
			/** TRANSLATORS: %1% is the number of soldiers the plural refers to */
			/** TRANSLATORS: %2% is the maximum number of soldier slots in the building */
			*s += (boost::format(ngettext("%1% soldier (+%2%)",
																		  "%1% soldiers (+%2%)",
																		  stationed))
											% stationed % (m_capacity - stationed)).str();
		} else {
			*s += (boost::format(ngettext("%u soldier", "%u soldiers", stationed))
											% stationed).str();
		}
	} else {
		if (m_capacity > stationed) {
			/** TRANSLATORS: %1% is the number of soldiers the plural refers to */
			/** TRANSLATORS: %2% are currently open soldier slots in the building */
			/** TRANSLATORS: %3% is the maximum number of soldier slots in the building */
			*s =
					(boost::format(ngettext("%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", stationed))
					 % present % (stationed - present) % (m_capacity - stationed)).str();
		} else {
			/** TRANSLATORS: %1% is the number of soldiers the plural refers to */
			/** TRANSLATORS: %2% are currently open soldier slots in the building */
			*s += (boost::format(ngettext("%1%(+%2%) soldier", "%1%(+%2%) soldiers", stationed))
					% present % (stationed - present)).str();
		}
	}
}


void MilitarySite::init(EditorGameBase & egbase)
{
	ProductionSite::init(egbase);

	upcast(Game, game, &egbase);

	for (Worker * worker : get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			soldier->set_location_initially(*this);
			assert(!soldier->get_state()); //  Should be newly created.
			if (game)
				soldier->start_task_buildingwork(*game);
		}
	}
	update_soldier_request();

	//  schedule the first healing
	m_nexthealtime = egbase.get_gametime() + 1000;
	if (game)
		schedule_act(*game, 1000);
}


/**
===============
Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy * const e)
{
	ProductionSite::set_economy(e);

	if (m_normal_soldier_request && e)
		m_normal_soldier_request->set_economy(e);
	if (m_upgrade_soldier_request && e)
		m_upgrade_soldier_request->set_economy(e);
}

/**
===============
Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(EditorGameBase & egbase)
{
	// unconquer land
	if (m_didconquer)
		egbase.unconquer_area
			(PlayerArea<Area<FCoords> >
			 	(owner().player_number(),
			 	 Area<FCoords>
			 	 	(egbase.map().get_fcoords(get_position()), descr().get_conquers())),
			 m_defeating_player);

	ProductionSite::cleanup(egbase);

	// Note that removing workers during ProductionSite::cleanup can generate
	// new requests; that's why we delete it at the end of this function.
	m_normal_soldier_request.reset();
	m_upgrade_soldier_request.reset();
}


/*
===============
Takes one soldier and adds him to ours

returns 0 on succes, -1 if there was no room for this soldier
===============
*/
int MilitarySite::incorporate_soldier(EditorGameBase & egbase, Soldier & s)
{

	if (s.get_location(egbase) != this)
	{
		s.set_location(this);
	}

	// Soldier upgrade is done once the site is full. In soldier upgrade, we
	// request one new soldier who is better suited than the existing ones.
	// Normally, I kick out one existing soldier as soon as a new guy starts walking
	// towards here. However, since that is done via infrequent polling, a new soldier
	// can sometimes reach the site before such kick-out happens. In those cases, we
	// should either drop one of the existing soldiers or reject the new guy, to
	// avoid overstocking this site.

	if (stationed_soldiers().size()  > descr().get_max_number_of_soldiers())
	{
		return incorporate_upgraded_soldier(egbase, s) ? 0 : -1;
	}

	if (!m_didconquer) {
		conquer_area(egbase);
		// Building is now occupied - idle animation should be played
		start_animation(egbase, descr().get_animation("idle"));

		if (upcast(Game, game, &egbase)) {
			send_message
				(*game,
				 Message::Type::kEconomySiteOccupied,
				 descr().descname(),
				 descr().icon_filename(),
				 descr().descname(),
				 descr().m_occupied_str,
				 true);
		}
	}

	if (upcast(Game, game, &egbase)) {
		// Bind the worker into this house, hide him on the map
		s.reset_tasks(*game);
		s.start_task_buildingwork(*game);
	}

	// Make sure the request count is reduced or the request is deleted.
	update_soldier_request(true);

	return 0;
}


/*
 * Returns the least wanted soldier -- If player prefers zero-level guys,
 * the most trained soldier is the "weakest guy".
 */

Soldier *
MilitarySite::find_least_suited_soldier()
{
	const std::vector<Soldier *> present = present_soldiers();
	const int32_t multiplier = kPrefersHeroes == m_soldier_preference ? -1 : 1;
	int worst_soldier_level = INT_MIN;
	Soldier * worst_soldier = nullptr;
	for (Soldier* sld : present) {
		int this_soldier_level = multiplier * static_cast<int> (sld->get_level(atrTotal));
		if (this_soldier_level > worst_soldier_level)
		{
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

bool
MilitarySite::drop_least_suited_soldier(bool new_soldier_has_arrived, Soldier * newguy)
{
	const std::vector<Soldier *> present = present_soldiers();

	// If I have only one soldier, and the  new guy is not here yet, I can't release.
	if (new_soldier_has_arrived || 1 < present.size())
	{
		Soldier * kickoutCandidate = find_least_suited_soldier();

		// If the arriving guy is worse than worst present, I wont't release.
		if (nullptr != newguy && nullptr != kickoutCandidate)
		{
			int32_t old_level = kickoutCandidate->get_level(atrTotal);
			int32_t new_level = newguy->get_level(atrTotal);
			if (kPrefersHeroes == m_soldier_preference && old_level >= new_level)
			{
				return false;
			}
			else
			if (kPrefersRookies == m_soldier_preference && old_level <= new_level)
			{
				return false;
			}
		}

		// Now I know that the new guy is worthy.
		if (nullptr != kickoutCandidate)
		{
			Game & game = dynamic_cast<Game&>(owner().egbase());
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
bool
MilitarySite::incorporate_upgraded_soldier(EditorGameBase & egbase, Soldier & s)
{
	// Call to drop_least routine has side effects: it tries to drop a soldier. Order is important!
	if (stationed_soldiers().size() < m_capacity || drop_least_suited_soldier(true, &s))
	{
		Game & game = dynamic_cast<Game&>(egbase);
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
void MilitarySite::request_soldier_callback
	(Game            &       game,
	 Request         &,
	 WareIndex,
	 Worker          * const w,
	 PlayerImmovable &       target)
{
	MilitarySite& msite = dynamic_cast<MilitarySite&>(target);
	Soldier& s = dynamic_cast<Soldier&>(*w);

	msite.incorporate_soldier(game, s);
}


/**
 * Update the request for soldiers and cause soldiers to be evicted
 * as appropriate.
 */
void MilitarySite::update_normal_soldier_request()
{
	std::vector<Soldier *> present = present_soldiers();
	uint32_t const stationed = stationed_soldiers().size();

	if (stationed < m_capacity) {
		if (!m_normal_soldier_request) {
			m_normal_soldier_request.reset
				(new Request
					(*this,
					 owner().tribe().soldier(),
					 MilitarySite::request_soldier_callback,
					 wwWORKER));
			m_normal_soldier_request->set_requirements (m_soldier_requirements);
		}

		m_normal_soldier_request->set_count(m_capacity - stationed);
	} else {
		m_normal_soldier_request.reset();
	}

	if (m_capacity < present.size()) {
		Game & game = dynamic_cast<Game&>(owner().egbase());
		for (uint32_t i = 0; i < present.size() - m_capacity; ++i) {
			Soldier & soldier = *present[i];
			soldier.reset_tasks(game);
			soldier.start_task_leavebuilding(game, true);
		}
	}
}

/* There are two kinds of soldier requests: "normal", which is used whenever the military site needs
 * more soldiers, and "upgrade" which is used when there is a preference for either heroes or
 * rookies.
 *
 * In case of normal requests, the military site is filled. In case of upgrade requests, only one guy
 * is exchanged at a time.
 *
 * There would be more efficient ways to get well trained soldiers. Now, new buildings appearing in battle
 * field are more vulnerable at the beginning. This is intentional. The purpose of this upgrade thing is
 * to reduce the benefits of site micromanagement. The intention is not to make gameplay easier in other ways.
 */
void MilitarySite::update_upgrade_soldier_request()
{
	bool reqch = update_upgrade_requirements();
	if (!m_soldier_upgrade_try)
		return;

	bool do_rebuild_request = reqch;

	if (m_upgrade_soldier_request)
	{
		if (!m_upgrade_soldier_request->is_open())
			// If a replacement is already walking this way, let's not change our minds.
			do_rebuild_request = false;
		if (0 == m_upgrade_soldier_request->get_count())
			do_rebuild_request = true;
	}
	else
		do_rebuild_request = true;

	if (do_rebuild_request)
	{
		m_upgrade_soldier_request.reset
				(new Request
				(*this,
				owner().tribe().soldier(),
				MilitarySite::request_soldier_callback,
				wwWORKER));

		m_upgrade_soldier_request->set_requirements(m_soldier_upgrade_requirements);
		m_upgrade_soldier_request->set_count(1);
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

void MilitarySite::update_soldier_request(bool incd)
{
	const uint32_t capacity = soldier_capacity();
	const uint32_t stationed = stationed_soldiers().size();

	if (m_doing_upgrade_request)
	{
		if (incd && m_upgrade_soldier_request) // update requests always ask for one soldier at time!
		{
			m_upgrade_soldier_request.reset();
		}
		if (capacity > stationed)
		{
			// Somebody is killing my soldiers in the middle of upgrade
			// or I have kicked out his predecessor already.
			if
				((m_upgrade_soldier_request)
				&& (m_upgrade_soldier_request->is_open() || 0 == m_upgrade_soldier_request->get_count()))
			{
				// Economy was not able to find the soldiers I need.
				// I can safely drop the upgrade request and go to fill mode.
				m_upgrade_soldier_request.reset();
			}
			if (! m_upgrade_soldier_request)
			{
				//phoo -- I can safely request new soldiers.
				m_doing_upgrade_request = false;
				update_normal_soldier_request();
			}
			// else -- ohno please help me! Player is in trouble -- evil grin
		}
		else // military site is full or overfull
		if (capacity < stationed) // player is reducing capacity
		{
			drop_least_suited_soldier(false, nullptr);
		}
		else // capacity == stationed size
		{
			if
				(m_upgrade_soldier_request
				&& (!(m_upgrade_soldier_request->is_open()))
				&& 1 == m_upgrade_soldier_request->get_count()
				&& (!incd))
			{
				drop_least_suited_soldier(false, nullptr);
			}
			else
			{
				update_upgrade_soldier_request();
			}
		}
	}
	else // not doing upgrade request
	{
		if ((capacity != stationed) || (m_normal_soldier_request))
			update_normal_soldier_request();

		if ((capacity == stationed) && (! m_normal_soldier_request))
		{
			if (present_soldiers().size() == capacity)
			{
				m_doing_upgrade_request = true;
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
void MilitarySite::act(Game & game, uint32_t const data)
{
	// TODO(unknown): do all kinds of stuff, but if you do nothing, let
	// ProductionSite::act() handle all this. Also note, that some ProductionSite
	// commands rely, that ProductionSite::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.

	ProductionSite::act(game, data);

	const int32_t timeofgame = game.get_gametime();
	if (m_normal_soldier_request && m_upgrade_soldier_request)
	{
		throw wexception("MilitarySite::act: Two soldier requests are ongoing -- should never happen!\n");
	}

	// I do not get a callback when stationed, non-present soldier returns --
	// Therefore I must poll in some occasions. Let's do that rather infrequently,
	// to keep the game lightweight.

	//TODO(unknown): I would need two new callbacks, to get rid ot this polling.
	if (timeofgame > m_next_swap_soldiers_time)
		{
			m_next_swap_soldiers_time = timeofgame + (m_soldier_upgrade_try ? 20000 : 100000);
			update_soldier_request();
		}

	if (m_nexthealtime <= timeofgame) {
		uint32_t total_heal = descr().get_heal_per_second();
		std::vector<Soldier *> soldiers = present_soldiers();
		uint32_t max_total_level = 0;
		float max_health = 0;
		Soldier * soldier_to_heal = 0;

		for (uint32_t i = 0; i < soldiers.size(); ++i) {
			Soldier * s = soldiers[i];

			// The healing algorithm is:
			// * heal soldier with highest total level
			// * heal healthiest if multiple of same total level exist
			if (s->get_current_hitpoints() < s->get_max_hitpoints()) {
				if (0 == soldier_to_heal || s->get_total_level() > max_total_level ||
						(s->get_total_level() == max_total_level &&
								s->get_current_hitpoints() / s->get_max_hitpoints() > max_health)) {
					max_total_level = s->get_total_level();
					max_health = s->get_current_hitpoints() / s->get_max_hitpoints();
					soldier_to_heal = s;
				}
			}
		}

		if (0 != soldier_to_heal) {
			soldier_to_heal->heal(total_heal);
		}

		m_nexthealtime = timeofgame + 1000;
		schedule_act(game, 1000);
	}
}


/**
 * The worker is about to be removed.
 *
 * After the removal of the worker, check whether we need to request
 * new soldiers.
 */
void MilitarySite::remove_worker(Worker & w)
{
	ProductionSite::remove_worker(w);

	if (upcast(Soldier, soldier, &w))
		pop_soldier_job(soldier, nullptr);

	update_soldier_request();
}


/**
 * Called by soldiers in the building.
 */
bool MilitarySite::get_building_work(Game & game, Worker & worker, bool)
{
	if (upcast(Soldier, soldier, &worker)) {
		// Evict soldiers that have returned home if the capacity is too low
		if (m_capacity < present_soldiers().size()) {
			worker.reset_tasks(game);
			worker.start_task_leavebuilding(game, true);
			return true;
		}

		bool stayhome;
		if
			(MapObject * const enemy
			 =
			 pop_soldier_job(soldier, &stayhome))
		{
			if (upcast(Building, building, enemy)) {
				soldier->start_task_attack
					(game, *building);
				return true;
			} else if (upcast(Soldier, opponent, enemy)) {
				if (!opponent->get_battle()) {
					soldier->start_task_defense
						(game, stayhome);
					if (stayhome)
						opponent->send_signal(game, "sleep");
					return true;
				}
			} else
				throw wexception("MilitarySite::get_building_work: bad SoldierJob");
		}
	}

	return false;
}


/**
 * \return \c true if the soldier is currently present and idle in the building.
 */
bool MilitarySite::is_present(Soldier & soldier) const
{
	return
		soldier.get_location(owner().egbase()) == this                     &&
		soldier.get_state() == soldier.get_state(Worker::taskBuildingwork) &&
		soldier.get_position() == get_position();
}

// TODO(sirver): This method should probably return a const reference.
std::vector<Soldier *> MilitarySite::present_soldiers() const
{
	std::vector<Soldier *> soldiers;

	for (Worker * worker : get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			if (is_present(*soldier)) {
				soldiers.push_back(soldier);
			}
		}
	}
	return soldiers;
}

// TODO(sirver): This method should probably return a const reference.
std::vector<Soldier *> MilitarySite::stationed_soldiers() const
{
	std::vector<Soldier *> soldiers;

	for (Worker * worker : get_workers()) {
		if (upcast(Soldier, soldier, worker)) {
			soldiers.push_back(soldier);
		}
	}
	return soldiers;
}

uint32_t MilitarySite::min_soldier_capacity() const {
	return 1;
}
uint32_t MilitarySite::max_soldier_capacity() const {
	return descr().get_max_number_of_soldiers();
}
uint32_t MilitarySite::soldier_capacity() const
{
	return m_capacity;
}

void MilitarySite::set_soldier_capacity(uint32_t const capacity) {
	assert(min_soldier_capacity() <= capacity);
	assert                        (capacity <= max_soldier_capacity());
	assert(m_capacity != capacity);
	m_capacity = capacity;
	update_soldier_request();
}

void MilitarySite::drop_soldier(Soldier & soldier)
{
	Game & game = dynamic_cast<Game&>(owner().egbase());

	if (!is_present(soldier)) {
		// This can happen when the "drop soldier" player command is delayed
		// by network delay or a client has bugs.
		molog("MilitarySite::drop_soldier(%u): not present\n", soldier.serial());
		return;
	}
	if (present_soldiers().size() <= min_soldier_capacity()) {
		molog("cannot drop last soldier(s)\n");
		return;
	}

	soldier.reset_tasks(game);
	soldier.start_task_leavebuilding(game, true);

	update_soldier_request();
}


void MilitarySite::conquer_area(EditorGameBase & egbase) {
	assert(!m_didconquer);
	egbase.conquer_area
		(PlayerArea<Area<FCoords> >
		 	(owner().player_number(),
		 	 Area<FCoords>
		 	 	(egbase.map().get_fcoords(get_position()), descr().get_conquers())));
	m_didconquer = true;
}


bool MilitarySite::can_attack()
{
	return m_didconquer;
}

void MilitarySite::aggressor(Soldier & enemy)
{
	Game & game = dynamic_cast<Game&>(owner().egbase());
	Map  & map  = game.map();
	if
		(enemy.get_owner() == &owner() ||
		 enemy.get_battle() ||
		 descr().get_conquers()
		 <=
		 map.calc_distance(enemy.get_position(), get_position()))
		return;

	if
		(map.find_bobs
		 	(Area<FCoords>(map.get_fcoords(base_flag().get_position()), 2),
		 	 nullptr,
		 	 FindBobEnemySoldier(&owner())))
		return;

	// We're dealing with a soldier that we might want to keep busy
	// Now would be the time to implement some player-definable
	// policy as to how many soldiers are allowed to leave as defenders
	std::vector<Soldier *> present = present_soldiers();

	if (1 < present.size()) {
		for (Soldier * temp_soldier : present) {
			if (!has_soldier_job(*temp_soldier)) {
				SoldierJob sj;
				sj.soldier  = temp_soldier;
				sj.enemy = &enemy;
				sj.stayhome = false;
				m_soldierjobs.push_back(sj);
				temp_soldier->update_task_buildingwork(game);
				return;
			}
		}
	}

	// Inform the player, that we are under attack by adding a new entry to the
	// message queue - a sound will automatically be played.
	notify_player(game, true);
}

bool MilitarySite::attack(Soldier & enemy)
{
	Game & game = dynamic_cast<Game&>(owner().egbase());

	std::vector<Soldier *> present = present_soldiers();
	Soldier * defender = nullptr;

	if (!present.empty()) {
		// Find soldier with greatest hitpoints
		uint32_t current_max = 0;
		for (Soldier * temp_soldier : present) {
			if (temp_soldier->get_current_hitpoints() > current_max) {
				defender = temp_soldier;
				current_max = defender->get_current_hitpoints();
			}
		}
	} else {
		// If one of our stationed soldiers is currently walking into the
		// building, give us another chance.
		std::vector<Soldier *> stationed = stationed_soldiers();
		for (Soldier * temp_soldier : stationed) {
			if (temp_soldier->get_position() == get_position()) {
				defender = temp_soldier;
				break;
			}
		}
	}

	if (defender) {
		pop_soldier_job(defender); // defense overrides all other jobs

		SoldierJob sj;
		sj.soldier = defender;
		sj.enemy = &enemy;
		sj.stayhome = true;
		m_soldierjobs.push_back(sj);

		defender->update_task_buildingwork(game);

		// Inform the player, that we are under attack by adding a new entry to
		// the message queue - a sound will automatically be played.
		notify_player(game);

		return true;
	} else {
		// The enemy has defeated our forces, we should inform the player
		const Coords coords = get_position();
		{
			send_message
				(game,
				 Message::Type::kWarfareSiteLost,
				 /** TRANSLATORS: Militarysite lost (taken/destroyed by enemy) */
				 pgettext("building", "Lost!"),
				 descr().icon_filename(),
				 _("Militarysite lost!"),
				 descr().m_defeated_enemy_str,
				 false);
		}

		// Now let's see whether the enemy conquers our militarysite, or whether
		// we still hold the bigger military presence in that area (e.g. if there
		// is a fortress one or two points away from our sentry, the fortress has
		// a higher presence and thus the enemy can just burn down the sentry.
		if (military_presence_kept(game)) {
			// Okay we still got the higher military presence, so the attacked
			// militarysite will be destroyed.
			set_defeating_player(enemy.owner().player_number());
			schedule_destroy(game);
			return false;
		}

		// The enemy conquers the building
		// In fact we do not conquer it, but place a new building of same type at
		// the old location.

		Building::FormerBuildings former_buildings = m_old_buildings;

		// The enemy conquers the building
		// In fact we do not conquer it, but place a new building of same type at
		// the old location.
		Player* enemyplayer = enemy.get_owner();

		// Now we destroy the old building before we place the new one.
		set_defeating_player(enemyplayer->player_number());
		schedule_destroy(game);

		enemyplayer->force_building(coords, former_buildings);
		BaseImmovable * const newimm = game.map()[coords].get_immovable();
		upcast(MilitarySite, newsite, newimm);
		newsite->reinit_after_conqueration(game);

		// Of course we should inform the victorious player as well
		newsite->send_message
			(game,
			 Message::Type::kWarfareSiteDefeated,
			 /** TRANSLATORS: Message title. */
			 /** TRANSLATORS: If you run out of space, you can also translate this as "Success!" */
			 _("Enemy Defeated!"),
			 newsite->descr().icon_filename(),
			 _("Enemy at site defeated!"),
			 newsite->descr().m_defeated_you_str,
			 true);

		return false;
	}
}

/// Initialises the militarysite after it was "conquered" (the old was replaced)
void MilitarySite::reinit_after_conqueration(Game & game)
{
	clear_requirements();
	conquer_area(game);
	update_soldier_request();
	start_animation(game, descr().get_animation("idle"));

	// feature request 1247384 in launchpad bugs: Conquered buildings tend to
	// be in a hostile area; typically players want heroes there.
	set_soldier_preference(kPrefersHeroes);
}

/// Calculates whether the military presence is still kept and \returns true if.
bool MilitarySite::military_presence_kept(Game & game)
{
	// collect information about immovables in the area
	std::vector<ImmovableFound> immovables;

	// Search in a radius of 3 (needed for big militarysites)
	FCoords const fc = game.map().get_fcoords(get_position());
	game.map().find_immovables(Area<FCoords>(fc, 3), &immovables);

	for (uint32_t i = 0; i < immovables.size(); ++i)
		if (upcast(MilitarySite const, militarysite, immovables[i].object))
			if
				(this       !=  militarysite          &&
				 &owner  () == &militarysite->owner() &&
				 get_size() <=  militarysite->get_size() &&
				 militarysite->m_didconquer)
				return true;
	return false;
}

/// Informs the player about an attack of his opponent.
void MilitarySite::notify_player(Game & game, bool const discovered)
{
	// Add a message as long as no previous message was send from a point with
	// radius <= 5 near the current location in the last 60 seconds
	send_message
		(game,
		 Message::Type::kWarfareUnderAttack,
		 /** TRANSLATORS: Militarysite is being attacked */
		 pgettext("building", "Attack!"),
		 descr().icon_filename(),
		 _("You are under attack"),
		 discovered ? descr().m_aggressor_str : descr().m_attack_str,
		 false,
		 60 * 1000, 5);
}


/*
   MilitarySite::set_requirements

   Easy to use, overwrite with given requirements.
*/
void MilitarySite::set_requirements (const Requirements & r)
{
	m_soldier_requirements = r;
}

/*
   MilitarySite::clear_requirements

   This should cancel any requirement pushed at this house
*/
void MilitarySite::clear_requirements ()
{
	m_soldier_requirements = Requirements();
}

void MilitarySite::send_attacker
	(Soldier & soldier, Building & target)
{
	assert(is_present(soldier));

	if (has_soldier_job(soldier))
		return;

	SoldierJob sj;
	sj.soldier  = &soldier;
	sj.enemy    = &target;
	sj.stayhome = false;
	m_soldierjobs.push_back(sj);

	soldier.update_task_buildingwork
		(dynamic_cast<Game&>(owner().egbase()));
}


bool MilitarySite::has_soldier_job(Soldier & soldier)
{
	for (const SoldierJob& temp_job : m_soldierjobs) {
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
MapObject * MilitarySite::pop_soldier_job
	(Soldier * const soldier, bool * const stayhome)
{
	for (std::vector<SoldierJob>::iterator job_iter = m_soldierjobs.begin();
		 job_iter != m_soldierjobs.end(); ++job_iter)
	{
		if (job_iter->soldier == soldier) {
			MapObject * const enemy = job_iter->enemy.get(owner().egbase());
			if (stayhome)
				*stayhome = job_iter->stayhome;
			m_soldierjobs.erase(job_iter);
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
bool
MilitarySite::update_upgrade_requirements()
{
	int32_t soldier_upgrade_required_min = m_soldier_upgrade_requirements.get_min();
	int32_t soldier_upgrade_required_max = m_soldier_upgrade_requirements.get_max();

	if (kPrefersHeroes != m_soldier_preference && kPrefersRookies != m_soldier_preference)
	{
		log("MilitarySite::swapSoldiers: error: Unknown player preference %d.\n", m_soldier_preference);
		m_soldier_upgrade_try = false;
		return false;
	}

	// Find the level of the soldier that is currently least-suited.
	Soldier * worst_guy = find_least_suited_soldier();
	if (worst_guy == nullptr) {
		// There could be no soldier in the militarysite right now. No reason to freak out.
		return false;
	}
	int32_t wg_level = worst_guy->get_level(atrTotal);

	// Micro-optimization: I assume that the majority of military sites have only level-zero
	// soldiers and prefer rookies. Handle them separately.
	m_soldier_upgrade_try = true;
	if (kPrefersRookies == m_soldier_preference) {
		if (0 == wg_level)
			{
				m_soldier_upgrade_try = false;
				return false;
			}
	}

	// Now I actually build the new requirements.
	int32_t reqmin = kPrefersHeroes == m_soldier_preference ? 1 + wg_level : 0;
	int32_t reqmax = kPrefersHeroes == m_soldier_preference ? SHRT_MAX : wg_level - 1;

	bool maxchanged = reqmax != soldier_upgrade_required_max;
	bool minchanged = reqmin != soldier_upgrade_required_min;

	if (maxchanged || minchanged)
	{
		if (m_upgrade_soldier_request && (m_upgrade_soldier_request->is_open()))
		{
			m_upgrade_soldier_request.reset();
		}
		m_soldier_upgrade_requirements = RequireAttribute(atrTotal, reqmin, reqmax);

		return true;
	}

	return false;
}

// setters

void
MilitarySite::set_soldier_preference(MilitarySite::SoldierPreference p)
{
	assert(kPrefersHeroes == p || kPrefersRookies == p);
	m_soldier_preference = p;
	m_next_swap_soldiers_time = 0;
}

}
