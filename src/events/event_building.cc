/*
 * Copyright (C) 2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "event_building.h"

#include "game.h"
#include "interactive_base.h"
#include "militarysite.h"
#include "player.h"
#include "profile.h"
#include "soldier.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"

#include "upcast.h"

#define EVENT_VERSION 2

namespace Widelands {

Event_Building::Event_Building
	(Section & s, Editor_Game_Base & egbase,
	 Tribe_Descr const * tribe, Building_Index building)
	: Event(s), m_ware_counts(0), m_worker_counts(0)
{
	try {
		uint32_t const packet_version = s.get_safe_positive("version");
		if (packet_version <= EVENT_VERSION) {
			if (not tribe) {
				Map const & map = egbase.map();
				m_player   = s.get_Player_Number("player", map.get_nrplayers(), 1);
				m_location =
					s.get_Coords
						("point", map.extent(), map.get_starting_pos(m_player));
				egbase.get_iabase()->reference_player_tribe(m_player, this);
				tribe =
					&egbase.manually_load_tribe
						(map.get_scenario_player_tribe(m_player));
				building =
					tribe->safe_building_index(s.get_safe_string("building"));
			}
			m_building = building;
			Building_Descr const & descr = *tribe->get_building_descr(building);
			if (dynamic_cast<Warehouse_Descr const *>(&descr)) {
				{ //  wares
					Ware_Index const nr_ware_types = tribe->get_nrwares();
					assert(not m_ware_counts);
					m_ware_counts = new uint32_t[nr_ware_types.value()];
					for (Ware_Index i = Ware_Index::First(); i < nr_ware_types; ++i)
						m_ware_counts[i.value()] =
							s.get_positive
								(tribe->get_ware_descr(i)->name().c_str(), 0);
				}
				{ //  workers
					Ware_Index const nr_worker_types = tribe->get_nrworkers();
					assert(not m_worker_counts);
					m_worker_counts = new uint32_t[nr_worker_types.value()];
					for
						(Ware_Index i = Ware_Index::First();
						 i < nr_worker_types;
						 ++i)
						m_worker_counts[i.value()] =
							s.get_positive
								(tribe->get_worker_descr(i)->name().c_str(), 0);
				}
				Soldier_Descr const & soldier_descr =  //  soldiers
					dynamic_cast<Soldier_Descr const &>
						(*tribe->get_worker_descr(tribe->worker_index("soldier")));
				uint32_t const max_hp_level      =
					soldier_descr.get_max_hp_level     ();
				uint32_t const max_attack_level  =
					soldier_descr.get_max_attack_level ();
				uint32_t const max_defense_level =
					soldier_descr.get_max_defense_level();
				uint32_t const max_evade_level   =
					soldier_descr.get_max_evade_level  ();
				while (Section::Value const * const v = s.get_next_val())
					try {
						char const *       key    = v->get_name();
						uint32_t     const amount = v->get_positive();
						Soldier_Strength ss;
						char * endp;
						{
							long long int const value = strtoll(key, &endp, 0);
							ss.hp      = value;
							if (*endp != '/' or max_hp_level      < ss.hp)
								throw false;
							key = endp + 1;
						}
						{
							long long int const value = strtoll(key, &endp, 0);
							ss.attack  = value;
							if (*endp != '/' or max_attack_level  < ss.attack)
								throw false;
							key = endp + 1;
						}
						{
							long long int const value = strtoll(key, &endp, 0);
							ss.defense = value;
							if (*endp != '/' or max_defense_level < ss.defense)
								throw false;
							key = endp + 1;
						}
						{
							long long int const value = strtoll(key, &endp, 0);
							ss.evade   = value;
							if (*endp        or max_evade_level   < ss.evade)
								throw false;
						}
						if (m_soldier_counts.find(ss) != m_soldier_counts.end())
							throw wexception("duplicated");
						m_soldier_counts.insert
							(std::pair<Soldier_Strength, uint32_t>(ss, amount));
					} catch (...) {
						throw wexception
							("%s=\"%s\": invalid", v->get_name(), v->get_string());
					}
			} else { //  not a warehouse
				if (upcast(ProductionSite_Descr const, ps_descr, &descr))
					try {
						{ //  wares
							Ware_Types const & inputs = ps_descr->inputs();
							uint8_t const nr_ware_types = inputs.size();
							m_ware_counts = new uint32_t[nr_ware_types];
							if (packet_version == 1)
								memset
									(m_ware_counts,
									 '\0',
									 nr_ware_types * sizeof(uint32_t));
							else
								for
									(struct {
									 	uint8_t                    i;
									 	Ware_Types::const_iterator it;
									 } i = {0, inputs.begin()};
									 i.i < nr_ware_types;
									 ++i.i, ++i.it)
								{
									char const * const wname =
										tribe->get_ware_descr(i.it->first)->name().c_str
											();
									uint32_t     const count = s.get_positive(wname, 0);
									uint32_t     const max   = i.it->second;
									if (max < count)
										throw wexception
											("can not have %u %s (only %u)",
											 count, wname, max);
									m_ware_counts[i.i] = count;
								}
						}
						{ //  workers
							bool const fill =
								packet_version == 1 ? s.get_bool("fill", true) : false;
							Ware_Types const & working_positions =
								ps_descr->working_positions();
							uint8_t const nr_worker_types = working_positions.size();
							m_worker_counts = new uint32_t[nr_worker_types];
							for
								(struct {uint8_t i; Ware_Types::const_iterator it;} i =
								 	{0, working_positions.begin()};
								 i.i < nr_worker_types;
								 ++i.i, ++i.it)
							{
								char const * const wname =
									tribe->get_worker_descr(i.it->first)->name().c_str
										();
								uint32_t     const max   = i.it->second;
								uint32_t     const count =
									fill ? max : s.get_positive(wname, 0);
								if (max < count)
									throw wexception
										("can not have %u %s (only %u)",
										 count, wname, max);
								m_worker_counts[i.i] = count;
							}
						}
					} catch (_wexception const & e) {
						throw wexception("(productionsite): %s", e.what());
					}
				if (upcast(SoldierControl const, soldiercontrol, &descr))
					try {
						uint32_t soldier_count = 0;
						Soldier_Descr const & soldier_descr =
							dynamic_cast<Soldier_Descr const &>
								(*tribe->get_worker_descr
								 	(tribe->worker_index("soldier")));
						uint32_t const max_hp_level      =
							soldier_descr.get_max_hp_level     ();
						uint32_t const max_attack_level  =
							soldier_descr.get_max_attack_level ();
						uint32_t const max_defense_level =
							soldier_descr.get_max_defense_level();
						uint32_t const max_evade_level   =
							soldier_descr.get_max_evade_level  ();
						while (Section::Value const * const v = s.get_next_val())
							try {
								char const *       key    = v->get_name();
								uint32_t     const amount = v->get_positive();
								Soldier_Strength ss;
								char * endp;
								{
									long long int const value = strtoll(key, &endp, 0);
									ss.hp      = value;
									if (*endp != '/' or max_hp_level      < ss.hp)
										throw false;
									key = endp + 1;
								}
								{
									long long int const value = strtoll(key, &endp, 0);
									ss.attack  = value;
									if (*endp != '/' or max_attack_level  < ss.attack)
										throw false;
									key = endp + 1;
								}
								{
									long long int const value = strtoll(key, &endp, 0);
									ss.defense = value;
									if (*endp != '/' or max_defense_level < ss.defense)
										throw false;
									key = endp + 1;
								}
								{
									long long int const value = strtoll(key, &endp, 0);
									ss.evade   = value;
									if (*endp        or max_evade_level   < ss.evade)
										throw false;
								}
								if (m_soldier_counts.count(ss))
									throw wexception("duplicated");
								m_soldier_counts.insert
									(std::pair<Soldier_Strength, uint32_t>(ss, amount));
								soldier_count += amount;
							} catch (...) {
								throw wexception
									("%s=\"%s\":invalid",
									 v->get_name(), v->get_string());
							}
						uint32_t const max_soldier_capacity =
							soldiercontrol->maxSoldierCapacity();
						if (max_soldier_capacity < soldier_count)
							throw wexception
							("%u soldiers but only %u allowed",
							 soldier_count, max_soldier_capacity);
					} catch (_wexception const & e) {
						throw wexception("(soldiercontrol): %s", e.what());
					}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("(building): %s", e.what());
	}
}


Event_Building::~Event_Building() {
	delete[] m_ware_counts;
	delete[] m_worker_counts;
}


void Event_Building::Write(Section & s, Editor_Game_Base & egbase) const
{
	s.set_string        ("type",     "building");
	s.set_int           ("version",  EVENT_VERSION);
	s.set_Coords        ("point",    m_location);
	if (m_player != 1)
		s.set_int        ("player",   m_player);
	Tribe_Descr const & tribe =
		egbase.manually_load_tribe
			(egbase.map().get_scenario_player_tribe(m_player));
	Building_Descr const & descr = *tribe.get_building_descr(m_building);
	s.set_string("building", descr.name().c_str());
	if (dynamic_cast<Warehouse_Descr const *>(&descr)) {
		for //  wares
			(struct {Ware_Index i; Ware_Index const nr_ware_types;} i =
			 	{Ware_Index::First(), tribe.get_nrwares()};
			 i.i < i.nr_ware_types;
			 ++i.i)
			if (uint32_t const count = m_ware_counts[i.i.value()])
				s.set_int(tribe.get_ware_descr(i.i)->name().c_str(), count);
		for //  workers
			(struct {Ware_Index i; Ware_Index const nr_worker_types;} i =
			 	{Ware_Index::First(), tribe.get_nrworkers()};
			 i.i < i.nr_worker_types;
			 ++i.i)
			if (uint32_t const count = m_worker_counts[i.i.value()])
				s.set_int(tribe.get_worker_descr(i.i)->name().c_str(), count);
	} else if (upcast(ProductionSite_Descr const, ps_descr, &descr)) {
		{ //  wares
			Ware_Types const & inputs = ps_descr->inputs();
			uint8_t const nr_ware_types = inputs.size();
			for
				(struct {uint8_t i; Ware_Types::const_iterator it;} i =
				 	{0, inputs.begin()};
				 i.i < nr_ware_types;
				 ++i.i, ++i.it)
				if (uint32_t const count = m_ware_counts[i.i])
					s.set_int
						(tribe.get_ware_descr(i.it->first)->name().c_str(), count);
		}
		{ //  workers
			Ware_Types const & working_positions = ps_descr->working_positions();
			uint8_t const nr_worker_types = working_positions.size();
			for
				(struct {uint8_t i; Ware_Types::const_iterator it;} i =
				 	{0, working_positions.begin()};
				 i.i < nr_worker_types;
				 ++i.i, ++i.it)
				if (uint32_t const count = m_worker_counts[i.i])
					s.set_int
						(tribe.get_worker_descr(i.it->first)->name().c_str(), count);
		}
	}
	container_iterate_const(Soldier_Counts, m_soldier_counts, i) {
		char buffer[strlen("255/255/255/255") + 1]; //  max length and terminator
		Soldier_Strength const ss = i.current->first;
		sprintf(buffer, "%u/%u/%u/%u", ss.hp, ss.attack, ss.defense, ss.evade);
		s.set_int(buffer, i.current->second);
	}
}


Event::State Event_Building::run(Game * game) {
	game->player(m_player).force_building
		(m_location, m_building,
		 m_ware_counts, m_worker_counts, m_soldier_counts);
	return m_state = DONE;
}

};
