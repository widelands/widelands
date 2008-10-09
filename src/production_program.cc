/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "production_program.h"

#include "game.h"
#include "mapregion.h"
#include "productionsite.h"
#include "profile.h"
#include "soldier.h"
#include "soldiercontrol.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "worker_program.h"

#include "helper.h"
#include "upcast.h"

#include <stdio.h>

namespace Widelands {

ProductionAction::~ProductionAction() {}

ActReturn::Condition::~Condition() {}

ActReturn::Negation::~Negation() {
	delete operand;
}
bool ActReturn::Negation::evaluate(ProductionSite const & ps) const {
	return not operand->evaluate(ps);
}


ActReturn::Economy_Needs::Economy_Needs
	(char const * parameters, Tribe_Descr const & tribe)
{
	try {
		ware_type = tribe.safe_ware_index(parameters);
	} catch (_wexception const & e) {
		throw wexception("needs: %s", e.what());
	}
}
bool ActReturn::Economy_Needs::evaluate(ProductionSite const & ps) const
{
#if 0
	log
		("ActReturn::Economy_Needs::evaluate(%s): (called from %s:%u) "
		 "economy_needs(%s) = %u\n",
		 ps.descname().c_str(),
		 const_cast<ProductionSite &>(ps).get_current_program()->program->get_name().c_str(),
		 const_cast<ProductionSite &>(ps).get_current_program()->ip,
		 ps.descr().tribe().get_ware_descr(ware_type)->descname().c_str(),
		 ps.get_economy()->needs_ware(ware_type));
#endif
	return ps.get_economy()->needs_ware(ware_type);
}


ActReturn::Condition * create_economy_condition
	(char * parameters, Tribe_Descr const & tribe)
{
	try {
		if (match_force_skip(parameters, "needs"))
			return new ActReturn::Economy_Needs(parameters, tribe);
		else
			throw wexception
				("expeted needs but found \"%s\"", parameters);
	} catch (_wexception const & e) {
		throw wexception("economy: %s", e.what());
	}
}


ActReturn::Condition * ActReturn::create_condition
	(char * parameters, Tribe_Descr const & tribe)
{
	try {
		if      (match_force_skip(parameters, "not"))
			return new ActReturn::Negation (parameters, tribe);
		else if (match_force_skip(parameters, "economy"))
			return create_economy_condition(parameters, tribe);
		else
			throw wexception
				("expected {not|site|economy} but found \"%s\"", parameters);
	} catch (_wexception const & e) {
		throw wexception("invalid condition: %s", e.what());
	}
}


ActReturn::ActReturn
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		if      (match(parameters, "failed"))    m_result = Failed;
		else if (match(parameters, "completed")) m_result = Completed;
		else if (match(parameters, "skipped"))   m_result = Skipped;
		else
			throw wexception
				("expected {failed|completed|skipped} but found \"%s\"",
				 parameters);

#if 0
		log
			("ActReturn::ActReturn: parsed m_result = %u, remaining parameters = "
			 "\"%s\"\n",
			 m_result, parameters);
#endif
		if (skip(parameters)) {
			if      (match_force_skip(parameters, "when"))
				m_condition = create_condition(parameters, descr.tribe());
			else if (match_force_skip(parameters, "unless"))
				m_condition = new Negation    (parameters, descr.tribe());
			else
				throw wexception
					("expected {when|unless} but found \"%s\"", parameters);
		} else if (*parameters)
			throw wexception
				("expected space or end of input but found \"%s\"", parameters);
		else
			m_condition = 0;

	} catch (_wexception const & e) {
		throw wexception("return: %s", e.what());
	}
}

void ActReturn::execute(Game & game, ProductionSite & ps) const {
	if (not m_condition or m_condition->evaluate(ps))
		return ps.program_end(game, m_result);
	else
		return ps.program_step(game);
}


ActCall::ActCall(char * parameters, ProductionSite_Descr const & descr) {
	try {
		bool reached_end;
		{
			char const * const program_name = match(parameters, reached_end);
			ProductionSite_Descr::ProgramMap const & programs =
				descr.get_all_programs();
			ProductionSite_Descr::ProgramMap::const_iterator const it =
				programs.find(program_name);
			if (it == programs.end())
				throw wexception
					("the program \"%s\" has not (yet) been declared in %s (wrong "
					 "declaration order?)",
					 program_name, descr.descname().c_str());
			m_program = it->second;
		}

#if 0
		log
			("ActCall::ActCall: parsed m_program = \"%s\", remaining parameters "
			 "= \"%s\"\n",
			 m_program.c_str(), parameters);
#endif
		if (reached_end)
			m_failure_handling_method = Ignore;
		else {
			skip(parameters);
			if      (match(parameters, "on failure ignore"))
				m_failure_handling_method = Ignore;
			else if (match(parameters, "on failure repeat"))
				m_failure_handling_method = Repeat;
			else if (match(parameters, "on failure fail"))
				m_failure_handling_method = Fail;
			else
				throw wexception
					("expected [on failure {fail|repeat|ignore}] but found \"%s\"",
					 parameters);
		}

	} catch (_wexception const & e) {
		throw wexception("call: %s", e.what());
	}
}

void ActCall::execute(Game & game, ProductionSite & ps) const {
	switch (ps.get_current_program()->phase) {
	case None: // The program has not yet been called.
		//ps.molog("%s  Call %s\n", ps.descname().c_str(), m_program->get_name().c_str());
		return ps.program_start(game, m_program->get_name());
	case Completed:
	case Skipped:
		return ps.program_step(game);
	case Failed:
		switch (m_failure_handling_method) {
		case Ignore:
			return ps.program_step(game);
		case Repeat:
			ps.get_current_program()->phase = None;
			ps.m_program_timer = true;
			ps.m_program_time  = ps.schedule_act(&game, 10);
			return;
		case Fail:
			return ps.program_end(game, Failed);
		default:
			assert(false);
		}
	default:
		assert(false);
	}
}


ActWorker::ActWorker
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		m_program = parameters;

		//  Quote form "void ProductionSite::program_act(Game &)":
		//  "Always main worker is doing stuff"
		char const * const main_worker_name = descr.workers()[0].c_str();
		const Tribe_Descr & tribe = descr.tribe();

		//  This will fail unless the main worker has a program with the given
		//  name, so it also validates the parameter.
		const Workarea_Info & worker_workarea_info
			= tribe.get_worker_descr(tribe.safe_worker_index(main_worker_name))
			->get_program(m_program)->get_workarea_info();

		const Workarea_Info::const_iterator worker_workarea_info_end =
			worker_workarea_info.end();
		Workarea_Info & building_workarea_info = descr.m_workarea_info;
		for
			(Workarea_Info::const_iterator it = worker_workarea_info.begin();
			 it != worker_workarea_info_end;
			 ++it)
		{
			std::set<std::string> & building_radius_infos =
				building_workarea_info[it->first];
			const std::set<std::string> & descriptions = it->second;
			const std::set<std::string>::const_iterator descriptions_end =
				descriptions.end();
			for
				(std::set<std::string>::const_iterator de = descriptions.begin();
				 de != descriptions_end;
				 ++de)
			{
				std::string description = descr.descname();
				description += ' ';
				description += production_program_name;
				description += " worker ";
				description += main_worker_name;
				description += *de;
				building_radius_infos.insert(description);
			}
		}
	} catch (_wexception const & e) {
		throw wexception("worker: %s", e.what());
	}
}

void ActWorker::execute(Game & game, ProductionSite & ps) const {
	//ps.molog("  Worker(%s)\n", m_program.c_str());

	// Always main worker is doing stuff
	ps.m_workers[0]->update_task_buildingwork(&game);
}


ActSleep::ActSleep(char * parameters, ProductionSite_Descr const &)
{
	try {
		if (*parameters) {
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp or value <= 0)
				throw wexception
					("expected duration in ms but found \"%s\"", parameters);
			m_duration = value;
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (_wexception const & e) {
		throw wexception("sleep: %s", e.what());
	}
}

void ActSleep::execute(Game & game, ProductionSite & ps) const {
	return
		ps.program_step
			(game, m_duration ? m_duration : ps.get_current_program()->phase);
}


ActAnimate::ActAnimate
	(char * parameters, ProductionSite_Descr & descr,
	 std::string const & directory, Profile & prof,
	 EncodeData const * const encdata)
{
	try {
		bool reached_end;
		char * const animation = match(parameters, reached_end);
		if (not strcmp(animation, "idle"))
			throw wexception("Idle animation is default, no calling senseful!");
		if (descr.is_animation_known(animation))
			m_id = descr.get_animation(animation);
		else {
			Section & s = prof.get_safe_section(animation);
			m_id = g_anim.get(directory.c_str(), s, 0, encdata);
			descr.add_animation(animation, m_id);
		}
		if (not reached_end) { //  The next parameter is the duration.
			char * endp;
			long int const value = strtol(parameters, &endp, 0);
			if (*endp or value <= 0)
				throw wexception
					("expected duration in ms but found \"%s\"", parameters);
			m_duration = value;
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (_wexception const & e) {
		throw wexception("animate: %s", e.what());
	}
}

void ActAnimate::execute(Game & game, ProductionSite & ps) const {
	ps.start_animation(&game, m_id);
	return
		ps.program_step
			(game, m_duration ? m_duration : ps.get_current_program()->phase);
}


ActConsume::ActConsume
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		Tribe_Descr const & tribe = descr.tribe();
		for (;;) {
			m_groups.resize(m_groups.size() + 1);
			std::pair<std::set<Ware_Index>, uint8_t> & group =
				*m_groups.rbegin();
			std::set<Ware_Index>::iterator last_insert_pos = group.first.end();
			uint8_t count = 1;
#if 0
			log
				("ActConsume::ActConsume: creating new group starting at \"%s\"\n",
				 parameters);
#endif
			for (;;) {
				char const * ware = parameters;
				while
					(*parameters        and *parameters != ',' and
					 *parameters != ':' and *parameters != ' ')
					++parameters;
				char const terminator = *parameters;
				*parameters = '\0';
				Ware_Index const ware_index = tribe.safe_ware_index(ware);
				if
					(group.first.size()
					 and
					 ware_index.value() <= group.first.begin()->value())
					throw wexception
						("wrong order of wares within group: ware type %s appears "
						 "after ware type %s (fix order!)",
						 ware,
						 tribe.get_ware_descr(*group.first.begin())->name().c_str());
				last_insert_pos = group.first.insert(last_insert_pos, ware_index);
				//log("\t\tadded %s to group\n", ware);
				*parameters = terminator;
				switch (terminator) {
				case ':': {
					++parameters;
					char * endp;
					long long int value = strtoll(parameters, &endp, 0);
					if
						((*endp and *endp != ' ') or
						 value < 1 or std::numeric_limits<uint8_t>::max() < value)
						throw wexception
							("expected count but found \"%s\"", parameters);
					parameters = endp;
					count = value;
					//  fallthrough
				}
				case '\0':
				case ' ':
					goto group_done;
				case ',':
					++parameters;
				}
			}
		group_done:
			group.second = count;
			//log("\tfinished group with count %u\n", count);
			if (not *parameters)
				break;
			force_skip(parameters);
		}
		if (m_groups.empty())
			throw wexception("expected ware1[,ware2[,...]][:N] ...");
	} catch (_wexception const & e) {
		throw wexception("consume: %s", e.what());
	}
}

void ActConsume::execute(Game & game, ProductionSite & ps) const {
	std::vector<WaresQueue *> const warequeues = ps.warequeues();
	size_t const nr_warequeues = warequeues.size();
	uint8_t consumption_quantities[nr_warequeues];
	Groups l_groups = m_groups; //  make a copy for local modification
	//log("ActConsume::execute(%s):\n", ps.descname().c_str());

	//  Iterate over all input queues and see how much we should consume from
	//  each of them.
	for (size_t i = 0; i < nr_warequeues; ++i) {
		Ware_Index const ware_type = warequeues[i]->get_ware();
		uint8_t nr_available = warequeues[i]->get_filled();
		consumption_quantities[i] = 0;
		Groups::const_iterator const groups_end = l_groups.end();
#if 0
		size_t const nr_groups = l_groups.size(); //  for debug messages
		log
			("\tconsidering input queue for %s (%u/%u)\n",
			 ps.descr().tribe().get_ware_descr(ware_type)->descname().c_str(),
			 nr_available, warequeues[i]->get_size());
#endif

		//  Iterate over all consume groups and see if they want us to consume
		//  any thing from the currently considered input queue.
		for (Groups::iterator it = l_groups.begin(); it != l_groups.end();)
			if (it->first.count(ware_type)) {
				it->first.erase(ware_type);
				if (it->second <= nr_available) {
					//  There are enough wares of the currently considered type
					//  to fulfill the requirements of the current group. We can
					//  therefore erase the group.
					consumption_quantities[i] += it->second;
					nr_available              -= it->second;
					l_groups.erase(it);
					//  No increment here, erase moved next element to the position
					//  pointed to by it.
#if 0
					log
						("\t\tgroup %u/%u needs %u: enough wares to fill this "
						 "group\n",
						 it - l_groups.begin() + 1, nr_groups, it->second);
#endif
				} else {
#if 0
					log
						("\t\tgroup %u/%u needs %u: unsufficient remaining "
						 "available wares (%u) to fill this group, will use all "
						 "remaining wares, input queue will be depleted\n",
						 it - l_groups.begin() + 1, nr_groups, it->second,
						 nr_available);
#endif
					consumption_quantities[i] += nr_available;
					it->second                -= nr_available;
					++it; //  Now check if the next group includes this ware type.
				}
			} else
				++it;
	}

	if (l_groups.empty()) { //  we fulfilled all consumption requirements
		for (size_t i = 0; i < nr_warequeues; ++i)
			if (uint8_t const q = consumption_quantities[i]) {
				assert(q <= warequeues[i]->get_filled());
				warequeues[i]->set_filled(warequeues[i]->get_filled() - q);
				warequeues[i]->update();
			}
		return ps.program_step(game);
	} else
		return ps.program_end(game, Failed);
}


ActProduce::ActProduce
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		Tribe_Descr const & tribe = descr.tribe();
		for (bool more = true; more; ++parameters) {
			m_items.resize(m_items.size() + 1);
			std::pair<Ware_Index, uint8_t> & item = *m_items.rbegin();
			skip(parameters);
			char const * ware = parameters;
			for (;; ++parameters)
				switch (*parameters) {
				case '\0':
				case ' ':
					item.second = 1;
					goto item_end;
				case ':': {
					*parameters = '\0';
					++parameters;
					char * endp;
					long long int value = strtoll(parameters, &endp, 0);
					if
						((*endp and *endp != ' ')
						 or
						 value < 1 or std::numeric_limits<uint8_t>::max() < value)
						throw wexception
							("expected count but found \"%s\"", parameters);
					parameters = endp;
					item.second = value;
					goto item_end;
				}
				}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			item.first = tribe.safe_ware_index(ware);
		}
	} catch (_wexception const & e) {
		throw wexception("produce: %s", e.what());
	}
}

void ActProduce::execute(Game & game, ProductionSite & ps) const {
	//ps.molog("  Produce\n");
	assert(ps.m_produced_items.empty());
	ps.m_produced_items = m_items;
	ps.m_workers[0]->update_task_buildingwork(&game);
}


ActMine::ActMine
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		World const & world = descr.tribe().world();
		bool reached_end;
		m_resource = world.safe_resource_index(match(parameters, reached_end));

		{
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			if (*endp != ' ' and value != static_cast<uint8_t>(value))
				throw wexception
					("expected distance but found \"%s\"", parameters);
			m_distance = value;
			parameters = endp;
		}

		{
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			if
				(*endp != ' ' and
				 (value < 1 or static_cast<uint8_t>(value) < value))
				throw wexception
					("expected percent but found \"%s\"", parameters);
			m_max = value;
			parameters = endp;
		}

		{
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			if
				(*endp and
				 (value < 1 or static_cast<uint8_t>(value) < value))
				throw wexception
					("expected percent but found \"%s\"", parameters);
			m_chance = value;
		}

		std::string description = descr.descname();
		description            += ' ';
		description            += production_program_name;
		description            += " mine ";
		description            += world.get_resource(m_resource)->descrname();
		descr.m_workarea_info[m_distance].insert(description);
	} catch (_wexception const & e) {
		throw wexception("mine: %s", e.what());
	}
}

void ActMine::execute(Game & game, ProductionSite & ps) const {
	Map & map = game.map();

	//ps.molog("  Mine '%s'", map.world().get_resource(m_resource)->get_name());

	//  select one of the nodes randomly
	uint32_t totalres    = 0;
	uint32_t totalchance = 0;
	uint32_t totalstart  = 0;
	int32_t pick;

	{
		MapRegion<Area<FCoords> > mr
			(map, Area<FCoords> (map.get_fcoords(ps.get_position()), m_distance));
		do {
			uint8_t  fres   = mr.location().field->get_resources();
			uint32_t amount = mr.location().field->get_resources_amount();
			uint32_t start_amount =
				mr.location().field->get_starting_res_amount();

			if (fres != m_resource) {
				amount       = 0;
				start_amount = 0;
			}

			totalres    += amount;
			totalstart  += start_amount;
			totalchance += 8 * amount;

			//  Add penalty for fields that are running out
			if (amount == 0)
				// we already know it's completely empty, so punish is less
				totalchance += 1;
			else if (amount <= 2)
				totalchance += 6;
			else if (amount <= 4)
				totalchance += 4;
			else if (amount <= 6)
				totalchance += 2;
		} while (mr.advance(map));
	}

	//  how much is digged
	int32_t digged_percentage=100;
	if (totalstart)
		digged_percentage = 100 - totalres * 100 / totalstart;
	if (not totalres)
		digged_percentage = 100;

	ps.molog
		("  Mine has already digged %i percent (%i/%i)!\n",
		 digged_percentage, totalres, totalstart);

	if (digged_percentage < m_max) {
		//  mine can produce normally
		if (totalres == 0)
			return ps.program_end(game, Failed);

		//  second pass through nodes
		pick = game.logic_rand() % totalchance;

		{
			MapRegion<Area<FCoords> > mr
				(map,
				 Area<FCoords>(map.get_fcoords(ps.get_position()), m_distance));
			do {
				uint8_t  fres   = mr.location().field->get_resources();
				uint32_t amount = mr.location().field->get_resources_amount();

				if (fres != m_resource)
					amount = 0;

				pick -= 8 * amount;
				if (pick < 0) {
					assert(amount > 0);

					--amount;

					mr.location().field->set_resources(m_resource, amount);
					break;
				}
			} while (mr.advance(map));
		}

		if (pick >= 0)
			return ps.program_end(game, Failed);

		ps.molog("  Mined one item\n");
	} else {
		//  Mine has reached its limits, still try to produce something but
		//  independent of sourrunding resources. Do not decrease resources
		//  further.
		if (m_chance <= game.logic_rand() % 100)
			return ps.program_end(game, Failed);
	}

	//  done successful
	//  FIXME Should pass the time it takes to mine in the phase parameter of
	//  FIXME ProductionSite::program_step so that the following sleep/animate
	//  FIXME command knows how long it should last.
	return ps.program_step(game);
}


ActCheck_Soldier::ActCheck_Soldier
	(char * parameters, ProductionSite_Descr const &)
{
	try {
		if      (match_force_skip(parameters, "hp"))
			attribute = atrHP;
		else if (match_force_skip(parameters, "attack"))
			attribute = atrAttack;
		else if (match_force_skip(parameters, "defense"))
			attribute = atrDefense;
		else if (match_force_skip(parameters, "evade"))
			attribute = atrEvade;
		else
			throw wexception
				("expected {hp|attack|defense|evade} but found \"%s\"",
				 parameters);

		char * endp;
		long long int value = strtoll(parameters, &endp, 0);
		if (*endp or static_cast<uint8_t>(value) != value)
			throw wexception
				("expected level but found \"%s\"", parameters);
		level = value;
	} catch (_wexception const & e) {
		throw wexception("check_soldier: %s", e.what());
	}
}

void ActCheck_Soldier::execute(Game & game, ProductionSite & ps) const {
	SoldierControl & ctrl = dynamic_cast<SoldierControl &>(ps);
	const std::vector<Soldier *> soldiers = ctrl.presentSoldiers();
	const std::vector<Soldier *>::const_iterator soldiers_end = soldiers.end();

	ps.molog("  Checking soldier (%u) level %d)\n", attribute, level);

	for (std::vector<Soldier *>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end)
			return ps.program_end(game, Failed);
		if        (attribute == atrHP)      {
			if ((*it)->get_hp_level()      == level)
				break;
		} else if (attribute == atrAttack)  {
			if ((*it)->get_attack_level()  == level)
				break;
		} else if (attribute == atrDefense) {
			if ((*it)->get_defense_level() == level)
				break;
		} else if (attribute == atrEvade)   {
			if ((*it)->get_evade_level()   == level)
				break;
		}
	}
	ps.molog("    okay\n"); // okay, do nothing

	ps.molog("  Check done!\n");

	return ps.program_step(game);
}


ActTrain::ActTrain(char * parameters, ProductionSite_Descr const &) {
	try {
		if      (match_force_skip(parameters, "hp"))
			attribute = atrHP;
		else if (match_force_skip(parameters, "attack"))
			attribute = atrAttack;
		else if (match_force_skip(parameters, "defense"))
			attribute = atrDefense;
		else if (match_force_skip(parameters, "evade"))
			attribute = atrEvade;
		else
			throw wexception
				("expected {hp|attack|defense|evade} but found \"%s\"",
				 parameters);

		{
			char * endp;
			long long int value = strtoll(parameters, &endp, 0);
			if (*endp != ' ' or static_cast<uint8_t>(value) != value)
				throw wexception
					("expected level but found \"%s\"", parameters);
			level = value;
			parameters = endp;
		}

		{
			char * endp;
			long long int value = strtoll(parameters, &endp, 0);
			if
				(*endp or static_cast<uint8_t>(value) != value or
				 value <= level)
				throw wexception
					("expected level > %u but found \"%s\"", level, parameters);
			target_level = value;
		}
	} catch (_wexception const & e) {
		throw wexception("train: %s", e.what());
	}
}

void ActTrain::execute(Game & game, ProductionSite & ps) const {
	SoldierControl & ctrl = dynamic_cast<SoldierControl &>(ps);
	const std::vector<Soldier *> soldiers = ctrl.presentSoldiers();
	const std::vector<Soldier *>::const_iterator soldiers_end =
		soldiers.end();
	std::vector<Soldier *>::const_iterator it = soldiers.begin();

	ps.molog
		("  Training soldier's %u (%d to %d)",
		 attribute, level, target_level);

	for (;; ++it) {
		if (it == soldiers_end)
			return ps.program_end(game, Failed);
		if        (attribute == atrHP)      {
			if ((*it)->get_hp_level     () == level)
				break;
		} else if (attribute == atrAttack)  {
			if ((*it)->get_attack_level () == level)
				break;
		} else if (attribute == atrDefense) {
			if ((*it)->get_defense_level() == level)
				break;
		} else if (attribute == atrEvade)
			if ((*it)->get_evade_level  () == level)
				break;
	}
	ps.molog("    okay\n"); // okay, do nothing

		try {
			if      (attribute == atrHP)
				(*it)->set_hp_level      (target_level);
			else if (attribute == atrAttack)
				(*it)->set_attack_level  (target_level);

			else if (attribute == atrDefense)
				(*it)->set_defense_level (target_level);

			else if (attribute == atrEvade)
				(*it)->set_evade_level   (target_level);

		} catch (...) {
			throw wexception ("Fail training soldier!!");
		}
		ps.molog("  Training done!\n");

	return ps.program_step(game);
}


//TODO: check if fx exists, load fx, lots of other checks for aprameter correctness
ActPlayFX::ActPlayFX (char * parameters, ProductionSite_Descr const &) {
	try {
		bool reached_end;
		name = match(parameters, reached_end);

		if (not reached_end) {
			char * endp;
			long long int value = strtoll(parameters, &endp, 0);
			if (*endp)
				throw wexception
					("expected priority but found \"%s\"", parameters);
			priority = value;
		} else
			priority = 127;
	} catch (_wexception const & e) {
		throw wexception("playFX: %s", e.what());
	}
}

void ActPlayFX::execute(Game & game, ProductionSite & ps) const {
	g_sound_handler.play_fx(name, ps.m_position, priority);
	return ps.program_step(game);
}


ProductionProgram::ProductionProgram(const std::string & name) :
	m_name(name)
{}

/*
===============
ProductionProgram::parse

Parse a program. The building is parsed completely (hopefully).
===============
*/
void ProductionProgram::parse
	(std::string    const & directory,
	 Profile              * const prof,
	 std::string    const & name,
	 ProductionSite_Descr * const building,
	 EncodeData     const * const encdata)
{
	Section & program_s = prof->get_safe_section(name.c_str());
	while (Section::Value * const v = program_s.get_next_val(0)) {
		ProductionAction * action;
		if      (not strcmp(v->get_name(), "return"))
			action = new ActReturn(v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "call"))
			action = new ActCall  (v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "sleep"))
			action = new ActSleep (v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "animate"))
			action =
				new ActAnimate
					(v->get_string(), *building, directory, *prof, encdata);
		else if (not strcmp(v->get_name(), "consume"))
			action = new ActConsume(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "produce"))
			action = new ActProduce(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "worker"))
			action = new ActWorker (v->get_string(), *building, name);
		else if (not strcmp(v->get_name(), "mine"))
			action = new ActMine   (v->get_string(), *building, name);
		else if (not strcmp(v->get_name(), "check_soldier"))
			action = new ActCheck_Soldier(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "train"))
			action = new ActTrain  (v->get_string(), *building);
		else if (not strcmp(v->get_name(), "playFX"))
			action = new ActPlayFX (v->get_string(), *building);
		else
			throw wexception("unknown command \"%s\"", v->get_name());
		m_actions.push_back(action);
	}
}

};
