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

#include "logic/garrisonhandler.h"

#include <functional>
#include <memory>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include "container_iterate.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "log.h"
#include "logic/building.h"
#include "logic/findbob.h"
#include "logic/instances.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "upcast.h"

namespace Widelands {

GarrisonHandler::GarrisonHandler
	(Building& building, uint32_t min_soldiers, uint32_t max_soldiers, uint32_t conquer_radius,
	uint32_t heal_per_second, SoldierPref soldier_pref, bool passive)
: Garrison(),
m_building(building),
m_min_capacity(min_soldiers),
m_max_capacity(max_soldiers),
m_capacity(max_soldiers),
m_passive(passive),
m_heal_per_second(heal_per_second),
m_last_heal_time(0),
m_conquer_radius(conquer_radius),
m_didconquer(false),
m_soldier_preference(soldier_pref),
m_last_swap_soldiers_time(0),
m_try_soldier_upgrade(false),
m_doing_upgrade_request(false)
{
	assert(is_a(GarrisonOwner, &m_building));
}

GarrisonHandler::~GarrisonHandler()
{
	assert(m_normal_soldier_request.get() == nullptr);
	assert(m_upgrade_soldier_request.get() == nullptr);
}

void GarrisonHandler::load_finish(Editor_Game_Base& egbase)
{
	// If our soldiers array is empty (old savegame), fill it with soldiers
	// found in building
	if (m_soldiers.empty()) {
		container_iterate_const(std::vector<Worker *>, m_building.get_workers(), i) {
			if (upcast(Soldier, soldier, *i.current)) {
				if (m_soldiers.size() < m_capacity) {
					m_soldiers.push_back(soldier);
				} else {
					break;
				}
			}
		}
	}
}

void GarrisonHandler::init(Editor_Game_Base & egbase)
{
	// Ensure all soldiers are fresh and give them a new task
	upcast(Game, game, &egbase);
	container_iterate_const(std::vector<Soldier *>, m_soldiers, i) {
		Soldier* soldier = *i.current;
		assert(!soldier->get_state()); //  Should be newly created.
		soldier->set_location_initially(m_building);
		if (game) {
			if (m_passive) {
				soldier->start_task_idle(*game, 0, -1);
			} else {
				soldier->start_task_buildingwork(*game);
			}
		}
	}
	// Update requests and timers
	update_soldier_request();
	m_last_heal_time = egbase.get_gametime();
}

void GarrisonHandler::reinit_after_conqueral(Game& game)
{
	m_soldier_requirements = Requirements();
	conquer_area(game);
	update_soldier_request();
}

void GarrisonHandler::cleanup(Editor_Game_Base& egbase)
{
	if (m_didconquer && !m_passive) {
		egbase.unconquer_area
			(Player_Area<Area<FCoords> >
				(owner().player_number(),
					Area<FCoords>(egbase.map().get_fcoords(m_building.get_position()), m_conquer_radius)),
		m_building.get_defeating_player());
	}
}

void GarrisonHandler::cleanup_requests(Editor_Game_Base&)
{
	m_normal_soldier_request.reset();
	m_upgrade_soldier_request.reset();
}


int32_t GarrisonHandler::act(Game& game)
{
	const int32_t timeofgame = game.get_gametime();
	int32_t next_act = -1;
	// Ensure requests integrity
	if (m_normal_soldier_request && m_upgrade_soldier_request)
	{
		throw wexception("GarrisonHandler::act: Two soldier requests are ongoing -- should never happen!\n");
	}
	// Update requests periodically
	bool full = stationedSoldiers().size() >= m_capacity;
	if (!full && !m_doing_upgrade_request && !m_normal_soldier_request) {
		// if we miss soldiers
		update_soldier_request();
	} else {
		// If we may issue a new upgrade request
		int32_t time_since_last_swap = (timeofgame - m_last_swap_soldiers_time);
		if (time_since_last_swap > GARRISON_SWAP_TIMEOUT) {
			time_since_last_swap = timeofgame;
			update_soldier_request();
			if (next_act < 0 || next_act > GARRISON_SWAP_TIMEOUT) {
				next_act = GARRISON_SWAP_TIMEOUT;
			}
		}
	}

	// Calculate the amount of heal point to distribute
	uint32_t to_heal_amount = static_cast<uint32_t>
		((timeofgame - m_last_heal_time) * m_heal_per_second / 1000);
	const std::vector<Soldier*> soldiers = presentSoldiers();
	BOOST_FOREACH(Soldier* soldier, soldiers) {
		uint32_t to_heal = soldier->get_max_hitpoints() - soldier->get_current_hitpoints();
		if (to_heal > to_heal_amount) to_heal = to_heal_amount;
		if (to_heal > 0) {
			soldier->heal(to_heal);
			to_heal_amount -= to_heal;
			// Act once/s while healing
			if (next_act < 0 || next_act > 1000) {
				next_act = 1000;
			}
		}
		if (to_heal_amount <= 0) break;
	}
	m_last_heal_time = timeofgame;
	// Act once/5s when idle
	if (next_act < 0) {
		next_act = 5000;
	}
	return next_act;
}

void GarrisonHandler::popSoldier(Soldier* soldier)
{
	// Pop the soldier and update requests
	popSoldierJob(soldier, nullptr, nullptr);
	update_soldier_request();
}

void GarrisonHandler::set_economy(Economy * const e)
{
	// update requests
	if (m_normal_soldier_request && e)
		m_normal_soldier_request->set_economy(e);
	if (m_upgrade_soldier_request && e)
		m_upgrade_soldier_request->set_economy(e);
}



bool GarrisonHandler::get_garrison_work(Game& game, Soldier* soldier)
{
	// Evict soldiers that have returned home if the capacity is too low
	if (m_capacity < stationedSoldiers().size()) {
		evict_soldier(game, soldier);
		return true;
	}

	bool stayhome;
	uint8_t retreat;
	if (Map_Object* enemy = popSoldierJob(soldier, &stayhome, &retreat)) {
		if (upcast(Building, building, enemy)) {
			soldier->start_task_attack(game, *building, retreat);
			return true;
		} else if (upcast(Soldier, opponent, enemy)) {
			if (!opponent->getBattle()) {
				soldier->start_task_defense(game, stayhome, retreat);
				if (stayhome)
					opponent->send_signal(game, "sleep");
				return true;
			}
		} else
			throw wexception("GarrisonHandler::get_garrison_work: bad SoldierJob");
	}
	return false;
}

void GarrisonHandler::set_soldier_requirements(Requirements req)
{
	m_soldier_requirements = req;
}

//
// Garrison implementation
//

Player& GarrisonHandler::owner() const
{
	return m_building.owner();
}

bool GarrisonHandler::canAttack() const
{
	return m_didconquer && !m_passive;
}

void GarrisonHandler::aggressor(Soldier& enemy)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());
	Map  & map  = game.map();
	// Do not react if enemy is occupied or too far away
	if
		(enemy.get_owner() == &owner() || enemy.getBattle()
		 || m_conquer_radius <= map.calc_distance
			(enemy.get_position(), m_building.get_position()))
	{
		return;
	}

	// Do not react if enemy is at our door
	if
		(map.find_bobs
		 	(Area<FCoords>(map.get_fcoords(m_building.base_flag().get_position()), 2), 0,
		 	 FindBobEnemySoldier(&owner())))
	{
		return;
	}

	// Send some soldiers to defend, keeping min capacity inside
	// This will send 1 soldier out to defend each time agressor()
	// is called //FIXME?
	const std::vector<Soldier*>& presents = presentSoldiers();
	if (presents.size() >  m_min_capacity) {
		container_iterate_const(std::vector<Soldier *>, presents, i) {
			if (!haveSoldierJob(**i.current)) {
				SoldierJob sj;
				sj.soldier  = *i.current;
				sj.enemy = &enemy;
				sj.stayhome = false;
				sj.retreat = owner().get_retreat_percentage();
				m_soldierjobs.push_back(sj);
				(*i.current)->update_task_buildingwork(game);
				return;
			}
		}
	}

	// Inform the player, that we are under attack by adding a new entry to the
	// message queue - a sound will automatically be played.
	inform_owner(game, InfoType::AGGRESSSED);
}

bool GarrisonHandler::attack(Soldier& enemy)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());

	const std::vector<Soldier*>& present = presentSoldiers();
	Soldier * defender = 0;

	if (!present.empty()) {
		// Find soldier with greatest hitpoints
		uint32_t current_max = 0;
		container_iterate_const(std::vector<Soldier *>, present, i) {
			if ((*i.current)->get_current_hitpoints() > current_max) {
				defender = *i.current;
				current_max = defender->get_current_hitpoints();
			}
		}
	} else {
		// If one of our stationed soldiers is currently walking into the
		// building, give us another chance.
		const std::vector<Soldier *>& stationed = stationedSoldiers();
		container_iterate_const(std::vector<Soldier *>, stationed, i) {
			if ((*i.current)->get_position() == m_building.get_position()) {
				defender = *i.current;
				break;
			}
		}
	}

	if (defender) {
		popSoldierJob(defender); // defense overrides all other jobs

		SoldierJob sj;
		sj.soldier = defender;
		sj.enemy = &enemy;
		sj.stayhome = true;
		sj.retreat = 0;         // Flag defenders could not retreat
		m_soldierjobs.push_back(sj);

		defender->update_task_buildingwork(game);

		// Inform the player, that we are under attack by adding a new entry to
		// the message queue - a sound will automatically be played.
		inform_owner(game, InfoType::UNDER_ATTACK);
		return true;
	}

	// The enemy has defeated our forces, we should inform the player
	// Code for handling change of owner are handled in the garrison
	// owner class.
	upcast(GarrisonOwner, go, &m_building);
	if (military_presence_kept(game)) {
		inform_owner(game, InfoType::GARRISON_LOST);
		go->garrison_lost(game, owner().player_number(), false);
	} else {
		inform_owner(game, InfoType::GARRISON_CAPTURED);
		go->garrison_lost(game, enemy.owner().player_number(), true);
	}
	return false;
}

bool GarrisonHandler::is_passive()
{
	return m_passive;
}

Building& GarrisonHandler::get_building()
{
	return m_building;
}



const std::vector< Soldier* > GarrisonHandler::presentSoldiers() const
{
	std::vector<Soldier *> present_soldiers;
	container_iterate_const(std::vector<Soldier*>, m_soldiers, i) {
		if (isPresent(**i.current)) {
			present_soldiers.push_back(*i.current);
		}
	}
	return present_soldiers;
}

const std::vector< Soldier* > GarrisonHandler::stationedSoldiers() const
{
	return m_soldiers;
}

uint32_t GarrisonHandler::minSoldierCapacity() const
{
	return m_min_capacity;
}

uint32_t GarrisonHandler::maxSoldierCapacity() const
{
	return m_max_capacity;
}

uint32_t GarrisonHandler::soldierCapacity() const
{
	return m_capacity;
}

void GarrisonHandler::setSoldierCapacity(uint32_t capacity)
{
	assert(minSoldierCapacity() <= capacity);
	assert                        (capacity <= maxSoldierCapacity());
	assert(m_capacity != capacity);
	m_capacity = capacity;
	update_soldier_request();
}

void GarrisonHandler::dropSoldier(Soldier& soldier)
{
	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());

	if (!isPresent(soldier)) {
		// This can happen when the "drop soldier" player command is delayed
		// by network delay or a client has bugs.
		return;
	}
	// Ensure min capacity is respected
	if (presentSoldiers().size() <= minSoldierCapacity()) {
		return;
	}
	// Drop the soldier and update requests
	evict_soldier(game, &soldier);
	update_soldier_request();
}

int GarrisonHandler::incorporateSoldier(Editor_Game_Base& egbase, Soldier& s)
{
	// Adjust the soldier location if required.
	if (s.get_location(egbase) != &m_building)
	{
		s.set_location(&m_building);
	}
	m_soldiers.push_back(&s);

	upcast(Game, game, &egbase);
	// If it's the first soldier, conquer the area
	if (!m_passive && !m_didconquer) {
		conquer_area(egbase);
		if (game) {
			inform_owner(*game, InfoType::GARRISON_OCCUPIED);
		}
		upcast(GarrisonOwner, go, &m_building);
		go->garrison_occupied();
	}

	// Bind the worker into this house, hide him on the map
	if (game) {
		s.reset_tasks(*game);
		if (m_passive) {
			s.start_task_idle(*game, 0, -1);
		} else {
			s.start_task_buildingwork(*game);
		}
	}
	// Make sure the request count is reduced or the request is deleted.
	update_soldier_request();
	return 0;
}

int GarrisonHandler::outcorporateSoldier(Editor_Game_Base&, Soldier&)
{
	// not needed
	log("CGH Outcorporate called\n");
	return -1;
}

uint32_t GarrisonHandler::conquerRadius() const
{
	return m_conquer_radius;
}

void GarrisonHandler::set_soldier_preference(GarrisonHandler::SoldierPref p)
{
	assert(SoldierPref::Heroes == p || SoldierPref::Rookies == p);
	m_soldier_preference = p;
	update_normal_soldier_request();
}


void GarrisonHandler::sendAttacker(Soldier& soldier, Building& target, uint8_t retreat)
{
	assert(isPresent(soldier));

	if (haveSoldierJob(soldier)) {
		return;
	}

	SoldierJob sj;
	sj.soldier  = &soldier;
	sj.enemy    = &target;
	sj.stayhome = false;
	sj.retreat  = retreat;
	m_soldierjobs.push_back(sj);

	soldier.update_task_buildingwork(ref_cast<Game, Editor_Game_Base>(owner().egbase()));
}

//
// Private helper methods
//
void GarrisonHandler::inform_owner(Game& game, GarrisonHandler::InfoType info)
{
	if (m_passive) {
		return;
	}
	std::string message;
	std::string message_id;
	std::string message_title;
	std::string building_name = m_building.descname();
	switch (info) {
		case InfoType::AGGRESSSED:
			message =
				(boost::format
					(_("Your %s discovered an aggressor")) % building_name)
				.str();
			message_id = "under_attack";
			message_title = _("You are under attack");
			break;
		case InfoType::UNDER_ATTACK:
			message =
				(boost::format
					(_("Your %s is under attack")) % building_name)
				.str();
			message_id = "under_attack";
			message_title = _("You are under attack");
			break;
		case InfoType::GARRISON_LOST:
			message =
				(boost::format
					(_("The enemy defeated your soldiers and destroyed your %s")) % building_name)
				.str();
			message_id = "garrison_lost";
			message_title = _("Garrison lost");
			break;
		case InfoType::GARRISON_CAPTURED:
			message =
				(boost::format
					(_("The enemy defeated your soldiers and captured your %s")) % building_name)
				.str();
			message_id = "garrison_lost";
			message_title = _("Garrison captured");
			break;
		case InfoType::GARRISON_OCCUPIED:
			message =
				(boost::format
					(_("Your soldier occupied your %s")) % building_name)
				.str();
			message_id = "garrison_occupied";
			message_title = _("Garrison occupied");
			break;
		default:
			assert(false);
	}
	m_building.send_message(game, message_id, message_title, message);
}

bool GarrisonHandler::isPresent(Soldier& soldier) const
{
	if
		(soldier.get_location(owner().egbase()) != &m_building
		|| soldier.get_position() != m_building.get_position())
	{
		return false;
	}
	// Present soldiers are working for the garrison only if it is not passive
	if (!m_passive) {
		return
			soldier.get_state() == soldier.get_state(Worker::taskBuildingwork);
	}
	return true;
}

void GarrisonHandler::conquer_area(Editor_Game_Base& egbase)
{
	assert(!m_didconquer);
	egbase.conquer_area
		(Player_Area<Area<FCoords> >
			(owner().player_number(),
			Area<FCoords>
				(egbase.map().get_fcoords(m_building.get_position()), m_conquer_radius)));
	m_didconquer = true;
}

bool GarrisonHandler::military_presence_kept(Game& game)
{
	// collect information about immovables in the area
	std::vector<ImmovableFound> immovables;

	// Search in a radius of 3 (needed for big militarysites)
	FCoords const fc = game.map().get_fcoords(m_building.get_position());
	game.map().find_immovables(Area<FCoords>(fc, 3), &immovables);

	for (uint32_t i = 0; i < immovables.size(); ++i) {
		upcast(GarrisonOwner, go, immovables[i].object);
		if (!go) {
			continue;
		}
		Garrison* g = go->get_garrison();
		upcast(GarrisonOwner, mygo, &m_building);
		// NOCOM I changed the logic here to check for any stationed soldier
		// and not for m_didconquer. So a msite with all slots empty and soldiers
		// on their way won't keep military presence anymore.

		// Presence kept if any non-empty garrison with higher conquer radius
		// and owned by our owner is nearby
		if
			(mygo != go
			&& owner().player_number() == g->owner().player_number()
			&& conquerRadius() < g->conquerRadius()
			&& !g->stationedSoldiers().empty())
		{
			return true;
		}
	}
	return false;
}

//FIXME CGH use a cleaner way without polling required
void GarrisonHandler::update_soldier_request(bool upgraded_in)
{
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
	const uint32_t capacity = soldierCapacity();
	const uint32_t stationed = stationedSoldiers().size();

	if (m_doing_upgrade_request) {
		if (upgraded_in && m_upgrade_soldier_request) {// update requests always ask for one soldier at time!
			m_upgrade_soldier_request.reset();
		}
		if (capacity > stationed) {
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
			if (!m_upgrade_soldier_request) {
				//phoo -- I can safely request new soldiers.
				m_doing_upgrade_request = false;
				update_normal_soldier_request();
			}
			// else -- ohno please help me! Player is in trouble -- evil grin
			// An upgraded soldier is on his way, but since we only make one request
			// at a time, we have to wait for him to arrive before doing another
			// request to fill capacity.
			//FIXME
		} else if (capacity < stationed) {// player is reducing capacity
			drop_least_suited_soldier();
		} else {// capacity == stationed size
			update_upgrade_soldier_request();
		}
	} else {// not doing upgrade request
		if ((capacity != stationed) or (m_normal_soldier_request)) {
			update_normal_soldier_request();
		}
		if ((capacity == stationed) && (! m_normal_soldier_request) && !m_passive) {
			// Our site is full, we might try to get upgraded soldiers
			// TODO not allowed for passive garrison for now
			if (presentSoldiers().size() == capacity && capacity > m_min_capacity) {
				m_doing_upgrade_request = true;
			}
		}
	}
}

void GarrisonHandler::update_normal_soldier_request()
{
	// Request new soldiers if needed
	std::vector<Soldier*> soldiers = stationedSoldiers();
	if (soldiers.size() < m_capacity) {
		if (!m_normal_soldier_request) {
			// No ongoing request, fill a new one
			Request* r = new Request
				(m_building, m_building.tribe().safe_worker_index("soldier"),
				 GarrisonHandler::request_soldier_callback, wwWORKER);
			r->set_requirements(m_soldier_requirements);
			m_normal_soldier_request.reset(r);
		}
		// Already an ongoing request, update counts
		m_normal_soldier_request->set_count(m_capacity - soldiers.size());
	} else {
		// No new soldier required. Destroy request if there is one
		m_normal_soldier_request.reset();
	}
	// Evict present soldiers if required
	const std::vector<Soldier *>& present = presentSoldiers();
	if (present.size() > m_capacity) {
		Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());
		for (uint32_t i = 0; i < present.size() - m_capacity; ++i) {
			Soldier * soldier = present[i];
			evict_soldier(game, soldier);
		}
	}
}

void GarrisonHandler::update_upgrade_soldier_request()
{
	bool reqirement_changed = update_upgrade_requirements();
	// Update the requirements
	if (!m_try_soldier_upgrade) {
		return;
	}
	// Do not perform a new request if someone is on his way
	// or if we are going to request the same thing
	if (m_upgrade_soldier_request) {
		if (!m_upgrade_soldier_request->is_open()) {
			// upgraded soldier is on his way
			return;
		}
		if (!reqirement_changed && m_upgrade_soldier_request->get_count() > 0) {
			// Current request is still valid
			return;
		}
	}
	// Issue a new request
	Request* r = new Request
		(m_building, m_building.tribe().safe_worker_index("soldier"),
		GarrisonHandler::request_soldier_callback, wwWORKER,
		GarrisonHandler::request_soldier_transfer_callback);
	// Honor soldier requirements if set by the garrison owner
	RequireAnd upgrade_requirement;
	upgrade_requirement.add(m_soldier_requirements);
	upgrade_requirement.add(m_soldier_upgrade_requirements);
	r->set_requirements(upgrade_requirement);
	r->set_count(1);
	m_upgrade_soldier_request.reset(r);
}

void GarrisonHandler::request_soldier_callback
	(Game& game, Request&, Ware_Index, Worker* w, PlayerImmovable& pi)
{
	Soldier& s = ref_cast<Soldier, Worker>(*w);
	upcast(GarrisonOwner, go, &pi);
	upcast(GarrisonHandler, gh, go->get_garrison());

	if (!gh->m_doing_upgrade_request) {
		gh->incorporateSoldier(game, s);
	} else {
		gh->incorporateUpgradedSoldier(game, s);
	}
}

void GarrisonHandler::request_soldier_transfer_callback
	(Game&, Request&, Ware_Index, Worker* w, PlayerImmovable& pi)
{
	Soldier& s = ref_cast<Soldier, Worker>(*w);
	upcast(GarrisonOwner, go, &pi);
	upcast(GarrisonHandler, gh, go->get_garrison());

	if (gh->m_doing_upgrade_request) {
		gh->drop_least_suited_soldier(&s);
	}
}

bool GarrisonHandler::update_upgrade_requirements()
{
	int32_t soldier_upgrade_required_min = m_soldier_upgrade_requirements.getMin();
	int32_t soldier_upgrade_required_max = m_soldier_upgrade_requirements.getMax();

	if
		(SoldierPref::Heroes != m_soldier_preference
		 && SoldierPref::Rookies != m_soldier_preference)
	{
		m_try_soldier_upgrade = false;
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
	if (m_soldier_preference == SoldierPref::Rookies && wg_level == 0) {
		m_try_soldier_upgrade = false;
		return false;
	}
	m_try_soldier_upgrade = true;

	// Now I actually build the new requirements.
	int32_t reqmin = SoldierPref::Heroes == m_soldier_preference ? 1 + wg_level : 0;
	int32_t reqmax = SoldierPref::Heroes == m_soldier_preference ? SHRT_MAX : wg_level - 1;

	bool maxchanged = reqmax != soldier_upgrade_required_max;
	bool minchanged = reqmin != soldier_upgrade_required_min;

	if (maxchanged or minchanged) {
		m_soldier_upgrade_requirements = RequireAttribute(atrTotal, reqmin, reqmax);
		return true;
	}
	return false;
}

bool GarrisonHandler::incorporateUpgradedSoldier(Editor_Game_Base& egbase, Soldier& s)
{
	// Call to drop_least routine has side effects: it tries to drop a soldier. Order is important!
	assert(isPresent(s));
	std::vector<Soldier*> stationned = stationedSoldiers();
	if (stationned.size() < m_capacity || drop_least_suited_soldier(&s)) {
		Game & game = ref_cast<Game, Editor_Game_Base>(egbase);
		s.set_location(&m_building);
		m_soldiers.push_back(&s);
		s.reset_tasks(game);
		if (!m_passive) {
			s.start_task_buildingwork(game);
		} else {
			s.start_task_idle(game, 0, -1);
		}
		// Reset timer for new request
		m_upgrade_soldier_request.reset();
		m_last_swap_soldiers_time = game.get_gametime();
		return true;
	}
	return false;
}

Soldier* GarrisonHandler::find_least_suited_soldier()
{
	const std::vector<Soldier *>& present = presentSoldiers();
	const int32_t multiplier = m_soldier_preference == SoldierPref::Heroes ? -1 : 1;
	int worst_soldier_level = INT_MIN;
	Soldier * worst_soldier = nullptr;
	BOOST_FOREACH(Soldier * sld, present)
	{
		int this_soldier_level = multiplier * static_cast<int> (sld->get_level(atrTotal));
		if (this_soldier_level > worst_soldier_level)
		{
			worst_soldier_level = this_soldier_level;
			worst_soldier = sld;
		}
	}
	return worst_soldier;
}

bool GarrisonHandler::drop_least_suited_soldier(Soldier* newguy)
{
	const std::vector<Soldier *>& present = presentSoldiers();
	// Don't drop under the minimum capacity if new guy is not here yet
	if (newguy == nullptr && present.size() <= m_min_capacity) {
		return false;
	}
	Soldier * kickoutCandidate = find_least_suited_soldier();

	// If the arriving guy is worse than worst present, I wont't release.
	if (newguy != nullptr && kickoutCandidate != nullptr) {
		int32_t old_level = kickoutCandidate->get_level(atrTotal);
		int32_t new_level = newguy->get_level(atrTotal);
		if (m_soldier_preference == SoldierPref::Heroes && old_level >= new_level) {
			return false;
		} else if (m_soldier_preference == SoldierPref::Rookies && old_level <= new_level) {
			return false;
		}
	}

	// Now I know that the new guy is worthy.
	if (kickoutCandidate != nullptr) {
		Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());
		evict_soldier(game, kickoutCandidate);
		return true;
	}
	return false;
}

bool GarrisonHandler::haveSoldierJob(Soldier& soldier) const
{
	container_iterate_const(std::vector<SoldierJob>, m_soldierjobs, i) {
		if (i.current->soldier == &soldier) {
			return true;
		}
	}
	return false;
}

Map_Object* GarrisonHandler::popSoldierJob(Soldier* soldier, bool* stayhome, uint8_t* retreat)
{
	container_iterate(std::vector<SoldierJob>, m_soldierjobs, i)
		if (i.current->soldier == soldier) {
			Map_Object * const enemy = i.current->enemy.get(owner().egbase());
			if (stayhome)
				*stayhome = i.current->stayhome;
			if (retreat)
				*retreat = i.current->retreat;
			m_soldierjobs.erase(i.current);
			return enemy;
		}
	return 0;
}

void GarrisonHandler::evict_soldier(Game& game, Soldier* s)
{
	assert(s);
	upcast(StorageOwner, storage_owner, &m_building);
	std::vector<Soldier*>::iterator it;
	for (it = m_soldiers.begin(); it != m_soldiers.end(); ++it) {
		if ((*it)->serial() == s->serial()) {
			m_soldiers.erase(it);
			s->reset_tasks(game);
			if (storage_owner) {
				storage_owner->get_storage()->incorporate_worker(game, *s);
			} else {
				s->start_task_leavebuilding(game, true);
			}
			return;
		}
	}
}

}
