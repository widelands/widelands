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

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <libintl.h>

#include "economy/flag.h"
#include "economy/request.h"
#include "i18n.h"
#include "log.h"
#include "logic/battle.h"
#include "logic/editor_game_base.h"
#include "logic/findbob.h"
#include "logic/game.h"
#include "logic/garrisonhandler.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "profile/profile.h"
#include "upcast.h"

namespace Widelands {

MilitarySite_Descr::MilitarySite_Descr
	(char        const * const _name,
	 char        const * const _descname,
	 const std::string & directory, Profile & prof,  Section & global_s,
	 const Tribe_Descr & _tribe)
:
	ProductionSite_Descr
		(_name, _descname, directory, prof, global_s, _tribe),
m_conquer_radius     (0),
m_num_soldiers       (0),
m_heal_per_second    (0)
{
	m_conquer_radius      = global_s.get_safe_int("conquers");
	m_num_soldiers        = global_s.get_safe_int("max_soldiers");
	m_heal_per_second     = global_s.get_safe_int("heal_per_second");
	if (m_conquer_radius > 0)
		m_workarea_info[m_conquer_radius].insert(descname() + _(" conquer"));
	m_prefers_heroes_at_start = global_s.get_safe_bool("prefer_heroes");
}

/**
===============
Create a new building of this type
===============
*/
Building & MilitarySite_Descr::create_object() const {
	return *new MilitarySite(*this);
}


/*
=============================

class MilitarySite

=============================
*/

MilitarySite::MilitarySite(const MilitarySite_Descr & ms_descr) :
ProductionSite(ms_descr)
{
	GarrisonHandler* gh = new GarrisonHandler
		(*this, 1,
		 descr().get_max_number_of_soldiers(),
		 descr().get_conquers(), descr().get_heal_per_second(),
		 descr().m_prefers_heroes_at_start ? GarrisonHandler::SoldierPref::Heroes
			: GarrisonHandler::SoldierPref::Rookies);
	m_garrison.reset(gh);
}

MilitarySite::~MilitarySite()
{
}

void MilitarySite::init(Editor_Game_Base & egbase)
{
	ProductionSite::init(egbase);
	m_garrison->init(egbase);
	upcast(Game, game, &egbase);

	const std::vector<Worker*>& ws = get_workers();
	container_iterate_const(std::vector<Worker *>, ws, i) {
		if (upcast(Soldier, soldier, *i.current)) {
			soldier->set_location_initially(*this);
			m_garrison->incorporateSoldier(egbase, *soldier);
		}
	}
	if (game) {
		schedule_act(*game, 1000);
	}
}


/**
===============
Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(Editor_Game_Base & egbase)
{
	// Order matters. Building needed to unconquer, building
	// cleanup will trigger new requests in garrison, garrison
	// cleanup will destroy them
	m_garrison->unconquer_area(egbase);
	ProductionSite::cleanup(egbase);
	m_garrison->cleanup(egbase);
}

/*
===============
Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game & game, uint32_t const data)
{
	// TODO: do all kinds of stuff, but if you do nothing, let
	// ProductionSite::act() handle all this. Also note, that some ProductionSite
	// commands rely, that ProductionSite::act() is not called for a certain
	// period (like cmdAnimation). This should be reworked.
	// Maybe a new queueing system like MilitaryAct could be introduced.

	ProductionSite::act(game, data);
	m_garrison->act(game);
	schedule_act(game, 1000);
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

	if (upcast(Soldier, soldier, &w)) {
		log("CGH: Soldier removed\n");
		m_garrison->soldier_removed(soldier);
	}
}


/**
===============
Display number of soldiers.
===============
*/
std::string MilitarySite::get_statistics_string()
{
	std::string str;
	uint32_t present = m_garrison->presentSoldiers().size();
	uint32_t total = m_garrison->stationedSoldiers().size();
	uint32_t capacity = m_garrison->soldierCapacity();

	if (present == total) {
		str = (boost::format
			(ngettext(_("%u soldier"), _("%u soldiers"), total)) % total).str();
	} else {
		str = (boost::format
			(ngettext(_("%u(+%u) soldier"), _("%u(+%u) soldiers"), present))
			% present % (total - present))
			.str();
	}

	if (capacity > total) {
		str += (boost::format(" (+%u)") %  (capacity - total)).str();
	}

	return str;
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
	m_garrison->set_economy(e);
	log("CGH: set_economy called with %d soldiers in playerimmovable and %d in garrison\n",
		get_workers().size(), m_garrison->stationedSoldiers().size());
}

/**
 * Called by soldiers in the building.
 */
bool MilitarySite::get_building_work(Game & game, Worker & worker, bool)
{
	if (upcast(Soldier, soldier, &worker)) {
		return m_garrison->get_garrison_work(game, soldier);
	}
	return false;
}


Garrison* MilitarySite::get_garrison() const
{
	return m_garrison.get();
}

Building* MilitarySite::get_building()
{
	return this;
}

void MilitarySite::garrison_occupied()
{
	start_animation(owner().egbase(), descr().get_animation("idle"));
}

void MilitarySite::garrison_lost(Game& game, Widelands::Player_Number defeating, bool captured)
{
	if (!captured) {
		schedule_destroy(game);
		return;
	}
	// The enemy conquers the building
	// In fact we do not conquer it, but place a new building of same type at
	// the old location.
	Player            * enemyplayer = game.get_player(defeating);
	const Tribe_Descr & enemytribe  = enemyplayer->tribe();

	// Add suffix to all descr in former buildings in cases
	// the new owner comes from another tribe
	Building_Descr::FormerBuildings former_buildings;
	BOOST_FOREACH(const Building_Descr * old_descr, m_old_buildings) {
		std::string bldname = old_descr->name();
		// Has this building already a suffix? == conquered building?
		std::string::size_type const dot = bldname.rfind('.');
		if (dot >= bldname.size()) {
			// Add suffix, if the new owner uses another tribe than we.
			if (enemytribe.name() != owner().tribe().name())
				bldname += "." + owner().tribe().name();
		} else if (enemytribe.name() == bldname.substr(dot + 1, bldname.size()))
			bldname = bldname.substr(0, dot);
		Building_Index bldi = enemytribe.safe_building_index(bldname.c_str());
		const Building_Descr * former_descr = enemytribe.get_building_descr(bldi);
		former_buildings.push_back(former_descr);
	}

	const Coords coords = get_position();
	// Now we destroy the old building before we place the new one.
	set_defeating_player(defeating);
	schedule_destroy(game);

	enemyplayer->force_building(coords, former_buildings);
	BaseImmovable * const newimm = game.map()[coords].get_immovable();
	upcast(GarrisonOwner, go, newimm);
	go->reinit_after_conqueral(game);

	// Of course we should inform the victorious player as well
	upcast(Building, new_building, newimm);
	std::string message = (boost::format
		(_("Your soldiers defeated the enemy at the %s."))
			% new_building->descname())
			.str();
	new_building->send_message
		(game, "site_defeated",
			_("Enemy at site defeated!"),
			message);
}

void MilitarySite::reinit_after_conqueral(Game& game)
{
	m_garrison->reinit_after_conqueral(game);
	start_animation(game, descr().get_animation("idle"));
}

}
