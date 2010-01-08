/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "economy/economy.h"
#include "economy/wares_queue.h"
#include "game.h"
#include "game_data_error.h"
#include "helper.h"
#include "mapregion.h"
#include "message_queue.h"
#include "productionsite.h"
#include "profile/profile.h"
#include "soldier.h"
#include "soldiercontrol.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "upcast.h"
#include "worker_program.h"

#include <libintl.h>

namespace Widelands {

ProductionProgram::Action::~Action() {}

void ProductionProgram::parse_ware_type_group
	(char            * & parameters,
	 Ware_Type_Group   & group,
	 Tribe_Descr const & tribe,
	 Ware_Types  const & inputs)
{
	std::set<Ware_Index>::iterator last_insert_pos = group.first.end();
	uint8_t count     = 1;
	uint8_t count_max = 0;
	for (;;) {
		char const * ware = parameters;
		while
			(*parameters        and *parameters != ',' and
			 *parameters != ':' and *parameters != ' ')
			++parameters;
		char const terminator = *parameters;
		*parameters = '\0';
		Ware_Index const ware_index = tribe.safe_ware_index(ware);
		for
			(struct {
			 	Ware_Types::const_iterator       current;
			 	Ware_Types::const_iterator const end;
			 } i = {inputs.begin(), inputs.end()};;
			 ++i.current)
			if (i.current == i.end)
				throw game_data_error
					(_
					 	("%s is not declared as an input (\"%s=<count>\" was not "
					 	 "found in the [inputs] section)"),
					 ware, ware);
			else if (i.current->first == ware_index) {
				count_max += i.current->second;
				break;
			}
		if
			(group.first.size()
			 and
			 ware_index.value() <= group.first.begin()->value())
			throw game_data_error
				(_
				 	("wrong order of ware types within group: ware type %s appears "
				 	 "after ware type %s (fix order!)"),
				 ware,
				 tribe.get_ware_descr(*group.first.begin())->name().c_str());
		last_insert_pos = group.first.insert(last_insert_pos, ware_index);
		*parameters = terminator;
		switch (terminator) {
		case ':': {
			++parameters;
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			count = value;
			if ((*endp and *endp != ' ') or value < 1 or count != value)
				throw game_data_error
					(_("expected %s but found \"%s\""), _("count"), parameters);
			parameters = endp;
			if (count_max < count)
				throw game_data_error
					(_
					 	("group count is %u but (total) input storage capacity of "
					 	 "the specified ware type(s) is only %u, so the group can "
					 	 "never be fulfilled by the site"),
					 count, count_max);
			//  fallthrough
		}
		case '\0':
		case ' ':
			group.second = count;
			return;
		case ',':
			++parameters;
		}
	}
}


ProductionProgram::ActReturn::Condition::~Condition() {}

ProductionProgram::ActReturn::Negation::~Negation() {
	delete operand;
}
bool ProductionProgram::ActReturn::Negation::evaluate
	(ProductionSite const & ps) const
{
	return not operand->evaluate(ps);
}
std::string ProductionProgram::ActReturn::Negation::description
	(Tribe_Descr const & tribe) const
{
	return _("not ") + operand->description(tribe);
}


bool ProductionProgram::ActReturn::Economy_Needs_Ware::evaluate
	(ProductionSite const & ps) const
{
#if 0
	log
		("ActReturn::Economy_Needs_Ware::evaluate(%s): (called from %s:%u) "
		 "economy_needs(%s) = %u\n",
		 ps.descname().c_str(),
		 const_cast<ProductionSite &>(ps).top_state().program->get_name().c_str(),
		 const_cast<ProductionSite &>(ps).top_state().ip,
		 ps.descr().tribe().get_ware_descr(ware_type)->descname().c_str(),
		 ps.get_economy()->needs_ware(ware_type));
#endif
	return ps.get_economy()->needs_ware(ware_type);
}
std::string ProductionProgram::ActReturn::Economy_Needs_Ware::description
	(Tribe_Descr const & tribe) const
{
	return _("economy needs ") + tribe.get_ware_descr(ware_type)->descname();
}

bool ProductionProgram::ActReturn::Economy_Needs_Worker::evaluate
	(ProductionSite const & ps) const
{
#if 0
	log
		("ActReturn::Economy_Needs_Worker::evaluate(%s): (called from %s:%u) "
		 "economy_needs(%s) = %u\n",
		 ps.descname().c_str(),
		 const_cast<ProductionSite &>(ps).top_state().program->get_name().c_str(),
		 const_cast<ProductionSite &>(ps).top_state().ip,
		 ps.descr().tribe().get_worker_descr(worker_type)->descname().c_str(),
		 ps.get_economy()->needs_worker(worker_type));
#endif
	return ps.get_economy()->needs_worker(worker_type);
}
std::string ProductionProgram::ActReturn::Economy_Needs_Worker::description
	(Tribe_Descr const & tribe) const
{
	return _("economy needs ") + tribe.get_worker_descr(worker_type)->descname();
}

ProductionProgram::ActReturn::Site_Has::Site_Has
	(char * & parameters, ProductionSite_Descr const & descr)
{
	try {
		parse_ware_type_group(parameters, group, descr.tribe(), descr.inputs());
	} catch (_wexception const & e) {
		throw game_data_error
			("has ware_type1[,ware_type2[,...]][:N]: %s", e.what());
	}
}
bool ProductionProgram::ActReturn::Site_Has::evaluate
	(ProductionSite const & ps) const
{
	uint8_t count = group.second;
	container_iterate_const(ProductionSite::Input_Queues, ps.warequeues(), i)
		if (group.first.count((*i.current)->get_ware())) {
			uint8_t const filled = (*i.current)->get_filled();
			if (count <= filled)
				return true;
			count -= filled;
		}
	return false;
}
std::string ProductionProgram::ActReturn::Site_Has::description
	(Tribe_Descr const & tribe) const
{
	std::string result = _("site has ");
	container_iterate_const(std::set<Ware_Index>, group.first, i) {
		result += tribe.get_ware_descr(*i.current)->descname();
		result += ',';
	}
	result.resize(result.size() - 1);
	if (1 < group.second) {
		char buffer[32];
		sprintf(buffer, ":%u", group.second);
		result += buffer;
	}
	return result;
}

bool ProductionProgram::ActReturn::Workers_Need_Experience::evaluate
	(ProductionSite const & ps) const
{
	ProductionSite::Working_Position const * const wp = ps.m_working_positions;
	for (uint32_t i = ps.descr().nr_working_positions(); i;)
		if (wp[--i].worker->needs_experience())
			return true;
	return false;
}
std::string ProductionProgram::ActReturn::Workers_Need_Experience::description
	(Tribe_Descr const &) const
{
	return _("workers need experience");
}


ProductionProgram::ActReturn::Condition * create_economy_condition
	(char * & parameters, Tribe_Descr const & tribe)
{
	try {
		if (match_force_skip(parameters, "needs"))
			try {
				bool reached_end;
				char const * const type_name = match(parameters, reached_end);
				if (Ware_Index index = tribe.ware_index(type_name)) {
					tribe.set_ware_type_has_demand_check(index);
					return
						new ProductionProgram::ActReturn::Economy_Needs_Ware
							(index);
				} else if ((index = tribe.worker_index(type_name))) {
					tribe.set_worker_type_has_demand_check(index);
					return
						new ProductionProgram::ActReturn::Economy_Needs_Worker
							(index);
				} else
					throw game_data_error
						(_("expected %s but found \"%s\""),
						 _("ware type or worker type"), type_name);
			} catch (_wexception const & e) {
				throw game_data_error("needs: %s", e.what());
			}
		else
			throw game_data_error
				(_("expected %s but found \"%s\""), "\"needs\"", parameters);
	} catch (_wexception const & e) {
		throw game_data_error("economy: %s", e.what());
	}
}


ProductionProgram::ActReturn::Condition * create_site_condition
	(char * & parameters, ProductionSite_Descr const & descr)
{
	try {
		if (match_force_skip(parameters, "has"))
			return
				new ProductionProgram::ActReturn::Site_Has(parameters, descr);
		else
			throw game_data_error
				(_("expected %s but found \"%s\""), "\"has\"", parameters);
	} catch (_wexception const & e) {
		throw game_data_error("site: %s", e.what());
	}
}


ProductionProgram::ActReturn::Condition * create_workers_condition
	(char * & parameters)
{
	try {
		if (match(parameters, "need experience"))
			return new ProductionProgram::ActReturn::Workers_Need_Experience;
		else
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 "\"need experience\"", parameters);
	} catch (_wexception const & e) {
		throw game_data_error("workers: %s", e.what());
	}
}


ProductionProgram::ActReturn::Condition *
ProductionProgram::ActReturn::create_condition
	(char * & parameters, ProductionSite_Descr const & descr)
{
	try {
		if      (match_force_skip(parameters, "not"))
			return new ActReturn::Negation (parameters, descr);
		else if (match_force_skip(parameters, "economy"))
			return create_economy_condition(parameters, descr.tribe());
		else if (match_force_skip(parameters, "site"))
			return create_site_condition   (parameters, descr);
		else if (match_force_skip(parameters, "workers"))
			return create_workers_condition(parameters);
		else
			throw game_data_error
				("expected %s but found \"%s\"",
				 "{\"not\"|\"economy\"|\"workers\"}", parameters);
	} catch (_wexception const & e) {
		throw game_data_error(_("invalid condition: %s"), e.what());
	}
}


ProductionProgram::ActReturn::ActReturn
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		if      (match(parameters, "failed"))    m_result = Failed;
		else if (match(parameters, "completed")) m_result = Completed;
		else if (match(parameters, "skipped"))   m_result = Skipped;
		else
			throw game_data_error
				(_
				 	("expected {\"failed\"|\"completed\"|\"skipped\"} but found "
				 	 "\"%s\""),
				 parameters);

		if (skip(parameters)) {
			if      (match_force_skip(parameters, "when")) {
				m_is_when = true;
				for (;;) {
					m_conditions.push_back(create_condition(parameters, descr));
					if (*parameters) {
						skip(parameters);
						if (not match_force_skip(parameters, "and"))
							throw game_data_error
								(_("expected \"and\" or end of input"));
					} else
						break;
				}
			} else if (match_force_skip(parameters, "unless")) {
				m_is_when = false;
				for (;;) {
					if (not *parameters)
						throw game_data_error
							(_("expected condition at end of input"));
					m_conditions.push_back(create_condition(parameters, descr));
					if (*parameters) {
						skip(parameters);
						if (not match_force_skip(parameters, "or"))
							throw game_data_error
								(_("expected \"or\" or end of input"));
					} else
						break;
				}
			} else
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 "{\"when\"|\"unless\"}", parameters);
		} else if (*parameters)
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 ("space or end of input"), parameters);
		else
			m_is_when = true;

	} catch (_wexception const & e) {
		throw game_data_error("return: %s", e.what());
	}
}

ProductionProgram::ActReturn::~ActReturn() {
	container_iterate_const(Conditions, m_conditions, i)
		delete *i.current;
}

void ProductionProgram::ActReturn::execute
	(Game & game, ProductionSite & ps) const
{
	std::string statistics_string =
		m_result == Failed    ? _("failed")    :
		m_result == Completed ? _("completed") : _("skipped");
	statistics_string += ' ';
	statistics_string += ps.top_state().program->descname();
	if (m_conditions.size()) {
		char const * operator_string;
		std::string result_string = statistics_string;
		if (m_is_when) { //  "when a and b and ..." (all conditions must be true)
			operator_string = _(" and ");
			result_string += _(" because: ");
			container_iterate_const(Conditions, m_conditions, i)
				if (not (*i.current)->evaluate(ps)) //  A condition is false,
					return ps.program_step(game); //  continue program.
				else {
					result_string += (*i.current)->description(ps.owner().tribe());
					result_string += operator_string;
				}
		} else { //  "unless a or b or ..." (all conditions must be false)
			operator_string = _(" or ");
			result_string += _(" because not: ");
			container_iterate_const(Conditions, m_conditions, i)
				if ((*i.current)->evaluate(ps)) //  A condition is true,
					return ps.program_step(game); //  continue program.
				else {
					result_string += (*i.current)->description(ps.owner().tribe());
					result_string += operator_string;
				}
		}
		result_string.resize(result_string.size() - strlen(operator_string));
		snprintf
			(ps.m_result_buffer, sizeof(ps.m_result_buffer),
			 "%s", result_string.c_str());
	}
	snprintf
		(ps.m_statistics_buffer, sizeof(ps.m_statistics_buffer),
		 "%s", statistics_string.c_str());
	return ps.program_end(game, m_result);
}


ProductionProgram::ActCall::ActCall
	(char * parameters, ProductionSite_Descr const & descr)
{
	//  Initialize with default handling methods.
	m_handling_methods[Failed    - 1] = Continue;
	m_handling_methods[Completed - 1] = Continue;
	m_handling_methods[Skipped   - 1] = Continue;

	try {
		bool reached_end;
		{
			char const * const program_name = match(parameters, reached_end);
			ProductionSite_Descr::Programs const & programs = descr.programs();
			ProductionSite_Descr::Programs::const_iterator const it =
				programs.find(program_name);
			if (it == programs.end())
				throw game_data_error
					(_
					 	("the program \"%s\" has not (yet) been declared in %s "
					 	 "(wrong declaration order?)"),
					 program_name, descr.descname().c_str());
			m_program = it->second;
		}

		//  Override with specified handling methods.
#if 0
		log
			("ActCall::ActCall: parsed m_program = \"%s\", remaining parameters "
			 "= \"%s\"\n",
			 m_program.c_str(), parameters);
#endif
		while (not reached_end) {
			skip(parameters);
			match_force_skip(parameters, "on");
			log("found \"on \": parameters = \"%s\"\n", parameters);

			Program_Result result_to_set_method_for;
			if        (match_force_skip(parameters, "failure"))    {
				if (m_handling_methods[Failed    - 1] != Continue)
					throw game_data_error
						(_("%s handling method already defined"), _("failure"));
				result_to_set_method_for = Failed;
			} else if (match_force_skip(parameters, "completion")) {
				if (m_handling_methods[Completed - 1] != Continue)
					throw game_data_error
						(_("%s handling method already defined"), _("completion"));
				result_to_set_method_for = Completed;
			} else if (match_force_skip(parameters, "skip"))       {
				if (m_handling_methods[Skipped   - 1] != Continue)
					throw game_data_error
						(_("%s handling method already defined"), _("skip"));
				result_to_set_method_for = Skipped;
			} else
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("{\"failure\"|\"completion\"|\"skip\"}"), parameters);

			Program_Result_Handling_Method handling_method;
			if      (match(parameters, "fail"))
				handling_method = Fail;
			else if (match(parameters, "complete"))
				handling_method = Complete;
			else if (match(parameters, "skip"))
				handling_method = Skip;
			else if (match(parameters, "repeat"))
				handling_method = Repeat;
			else
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("{\"fail\"|\"complete\"|\"skip\"|\"repeat\"}"),
					 parameters);
			m_handling_methods[result_to_set_method_for - 1] = handling_method;
			reached_end = not *parameters;
			log
				("read handling method for result %u: %u, parameters = \"%s\", "
				 "reached_end = %u\n",
				 result_to_set_method_for, handling_method,
				 parameters, reached_end);
		}
	} catch (_wexception const & e) {
		throw game_data_error("call: %s", e.what());
	}
}

void ProductionProgram::ActCall::execute
	(Game & game, ProductionSite & ps) const
{
	Program_Result const program_result =
		static_cast<Program_Result>(ps.top_state().phase);

	if (program_result == None) //  The program has not yet been called.
		//ps.molog("%s  Call %s\n", ps.descname().c_str(),
		//         m_program->get_name().c_str());
		return ps.program_start(game, m_program->name());

	switch (m_handling_methods[program_result - 1]) {
	case Fail:
	case Complete:
	case Skip:
		return ps.program_end(game, None);
	case Continue:
		return ps.program_step(game);
	case Repeat:
		ps.top_state().phase = None;
		ps.m_program_timer   = true;
		ps.m_program_time    = ps.schedule_act(game, 10);
	}
}


ProductionProgram::ActWorker::ActWorker
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		m_program = parameters;

		//  Quote form "void ProductionSite::program_act(Game &)":
		//  "Always main worker is doing stuff"
		Worker_Descr const & main_worker_descr =
			*descr.tribe().get_worker_descr(descr.working_positions()[0].first);

		//  This will fail unless the main worker has a program with the given
		//  name, so it also validates the parameter.
		Workarea_Info const & worker_workarea_info =
			main_worker_descr.get_program(m_program)->get_workarea_info();
		Workarea_Info & building_workarea_info = descr.m_workarea_info;
		container_iterate_const(Workarea_Info, worker_workarea_info, i) {
			std::set<std::string> & building_radius_infos =
				building_workarea_info[i.current->first];
			std::set<std::string> const & descriptions = i.current->second;
			container_iterate_const(std::set<std::string>, descriptions, de) {
				std::string description = descr.descname();
				description += ' ';
				description += production_program_name;
				description += " worker ";
				description += main_worker_descr.name();
				description += *de.current;
				building_radius_infos.insert(description);
			}
		}
	} catch (_wexception const & e) {
		throw game_data_error("worker: %s", e.what());
	}
}

void ProductionProgram::ActWorker::execute
	(Game & game, ProductionSite & ps) const
{
	//ps.molog("  Worker(%s)\n", m_program.c_str());

	// Always main worker is doing stuff
	ps.m_working_positions[0].worker->update_task_buildingwork(game);
}


ProductionProgram::ActSleep::ActSleep
	(char * parameters, ProductionSite_Descr const &)
{
	try {
		if (*parameters) {
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			m_duration = value;
			if (*endp or value <= 0 or m_duration != value)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("duration in ms"), parameters);
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (_wexception const & e) {
		throw game_data_error(_("sleep: %s"), e.what());
	}
}

void ProductionProgram::ActSleep::execute
	(Game & game, ProductionSite & ps) const
{
	return
		ps.program_step(game, m_duration ? m_duration : ps.top_state().phase);
}


ProductionProgram::ActAnimate::ActAnimate
	(char * parameters, ProductionSite_Descr & descr,
	 std::string const & directory, Profile & prof,
	 EncodeData const * const encdata)
{
	try {
		bool reached_end;
		char * const animation_name = match(parameters, reached_end);
		if (not strcmp(animation_name, "idle"))
			throw game_data_error
				(_("idle animation is default; calling is not allowed"));
		if (descr.is_animation_known(animation_name))
			m_id = descr.get_animation(animation_name);
		else {
			m_id = g_anim.get
				(directory.c_str(),
				 prof.get_safe_section(animation_name),
				 0,
				 encdata);
			descr.add_animation(animation_name, m_id);
		}
		if (not reached_end) { //  The next parameter is the duration.
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			m_duration = value;
			if (*endp or value <= 0 or m_duration != value)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("duration in ms"), parameters);
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (_wexception const & e) {
		throw game_data_error("animate: %s", e.what());
	}
}

void ProductionProgram::ActAnimate::execute
	(Game & game, ProductionSite & ps) const
{
	ps.start_animation(game, m_id);
	return
		ps.program_step(game, m_duration ? m_duration : ps.top_state().phase);
}


ProductionProgram::ActConsume::ActConsume
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		Tribe_Descr const & tribe = descr.tribe();
		for (;;) {
			m_groups.resize(m_groups.size() + 1);
			parse_ware_type_group
				(parameters, *m_groups.rbegin(), tribe, descr.inputs());
			if (not *parameters)
				break;
			force_skip(parameters);
		}
		if (m_groups.empty())
			throw game_data_error
				(_("expected ware_type1[,ware_type2[,...]][:N] ..."));
	} catch (_wexception const & e) {
		throw game_data_error("consume: %s", e.what());
	}
}

void ProductionProgram::ActConsume::execute
	(Game & game, ProductionSite & ps) const
{
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
				if (it->second <= nr_available) {
					//  There are enough wares of the currently considered type
					//  to fulfill the requirements of the current group. We can
					//  therefore erase the group.
					consumption_quantities[i] += it->second;
					nr_available              -= it->second;
					it = l_groups.erase(it);
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

	if (uint8_t const nr_missing_groups = l_groups.size()) {
		Tribe_Descr const & tribe = ps.owner().tribe();
		std::string result_string = _("failed");
		result_string            += ' ';
		result_string            += ps.top_state().program->descname();
		result_string            += _(" because: ");
		for
			(struct {
			 	Groups::const_iterator       current;
			 	Groups::const_iterator const end;
			 } i = {l_groups.begin(), l_groups.end()};;)
		{
			assert(i.current->first.size());
			for
				(struct {
				 	std::set<Ware_Index>::const_iterator       current;
				 	std::set<Ware_Index>::const_iterator const end;
				 } j = {i.current->first.begin(), i.current->first.end()};;)
			{
				result_string += tribe.get_ware_descr(*j.current)->descname();
				if (++j.current == j.end)
					break;
				result_string += ',';
			}
			{
				uint8_t const count = i.current->second;
				if (1 < count) {
					char buffer[5];
					sprintf(buffer, ":%u", count);
					result_string += buffer;
				}
			}
			if (++i.current == i.end)
				break;
			result_string += _(" and ");
		}
		result_string +=
			ngettext(" is missing", " are missing", nr_missing_groups);
		snprintf
			(ps.m_result_buffer, sizeof(ps.m_result_buffer),
			 "%s", result_string.c_str());
		return ps.program_end(game, Failed);
	} else { //  we fulfilled all consumption requirements
		for (size_t i = 0; i < nr_warequeues; ++i)
			if (uint8_t const q = consumption_quantities[i]) {
				assert(q <= warequeues[i]->get_filled());
				warequeues[i]->set_filled(warequeues[i]->get_filled() - q);
				warequeues[i]->update();
			}
		return ps.program_step(game);
	}
}


ProductionProgram::ActProduce::ActProduce
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
					unsigned long long int const value =
						strtoull(parameters, &endp, 0);
					item.second = value;
					if
						((*endp and *endp != ' ')
						 or
						 value < 1 or item.second != value)
						throw game_data_error
							(_("expected %s but found \"%s\""),
							 _("count"), parameters);
					parameters = endp;
					goto item_end;
				}
				}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			if (not descr.is_output(item.first = tribe.safe_ware_index(ware)))
				throw game_data_error
					(_
					 	("%s is not declared as an output (\"output=%s\" was not "
					 	 "found in the [global] section)"),
					 ware, ware);
		}
	} catch (_wexception const & e) {
		throw game_data_error("produce: %s", e.what());
	}
}

void ProductionProgram::ActProduce::execute
	(Game & game, ProductionSite & ps) const
{
	//ps.molog("  Produce\n");
	assert(ps.m_produced_items.empty());
	ps.m_produced_items = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	Tribe_Descr const & tribe = ps.owner().tribe();
	std::string result_string = _("Produced ");
	assert(m_items.size());
	for
		(struct {Items::const_iterator current; Items::const_iterator const end;}
		 i = {m_items.begin(), m_items.end()};;)
	{
		{
			uint8_t const count = i.current->second;
			if (1 < count) {
				char buffer[5];
				sprintf(buffer, _("%u "), count);
				result_string += buffer;
			}
		}
		result_string += tribe.get_ware_descr(i.current->first)->descname();
		if (++i.current == i.end)
			break;
		result_string += _(", ");
	}
	snprintf
		(ps.m_result_buffer, sizeof(ps.m_result_buffer),
		 "%s", result_string.c_str());
}


ProductionProgram::ActRecruit::ActRecruit
	(char * parameters, ProductionSite_Descr const & descr)
{
	try {
		Tribe_Descr const & tribe = descr.tribe();
		for (bool more = true; more; ++parameters) {
			m_items.resize(m_items.size() + 1);
			std::pair<Ware_Index, uint8_t> & item = *m_items.rbegin();
			skip(parameters);
			char const * worker = parameters;
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
					unsigned long long int const value =
						strtoull(parameters, &endp, 0);
					item.second = value;
					if
						((*endp and *endp != ' ')
						 or
						 value < 1 or item.second != value)
						throw game_data_error
							(_("expected %s but found \"%s\""),
							 _("count"), parameters);
					parameters = endp;
					goto item_end;
				}
				}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			item.first = tribe.safe_worker_index(worker);
		}
	} catch (_wexception const & e) {
		throw game_data_error("recruit: %s", e.what());
	}
}

void ProductionProgram::ActRecruit::execute
	(Game & game, ProductionSite & ps) const
{
	assert(ps.m_recruited_workers.empty());
	ps.m_recruited_workers = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	Tribe_Descr const & tribe = ps.owner().tribe();
	std::string result_string = _("Recruited ");
	assert(m_items.size());
	for
		(struct {Items::const_iterator current; Items::const_iterator const end;}
		 i = {m_items.begin(), m_items.end()};;)
	{
		{
			uint8_t const count = i.current->second;
			if (1 < count) {
				char buffer[5];
				sprintf(buffer, _("%u "), count);
				result_string += buffer;
			}
		}
		result_string += tribe.get_worker_descr(i.current->first)->descname();
		if (++i.current == i.end)
			break;
		result_string += _(", ");
	}
	snprintf
		(ps.m_result_buffer, sizeof(ps.m_result_buffer),
		 "%s", result_string.c_str());
}


ProductionProgram::ActMine::ActMine
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		World const & world = descr.tribe().world();
		assert(&world);
		bool reached_end;
		m_resource = world.safe_resource_index(match(parameters, reached_end));

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_distance = value;
			if (*endp != ' ' or m_distance != value)
				throw game_data_error
					(_("expected %s but found \"%s\""), _("distance"), parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_max = value;
			if (*endp != ' ' or value < 1 or 100 < value)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("percentage"), parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_chance = value;
			if (*endp or value < 1 or 100 < value)
				throw game_data_error
					(_("expected %s but found \"%s\""),
					 _("percentage"), parameters);
		}

		std::string description = descr.descname();
		description            += ' ';
		description            += production_program_name;
		description            += " mine ";
		description            += world.get_resource(m_resource)->descname();
		descr.m_workarea_info[m_distance].insert(description);
	} catch (_wexception const & e) {
		throw game_data_error("mine: %s", e.what());
	}
}

void ProductionProgram::ActMine::execute
	(Game & game, ProductionSite & ps) const
{
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
	int32_t digged_percentage = 100;
	if (totalstart)
		digged_percentage = 100 - totalres * 100 / totalstart;
	if (not totalres)
		digged_percentage = 100;

	if (digged_percentage < m_max) {
		//  mine can produce normally
		if (totalres == 0)
			return ps.program_end(game, Failed);

		//  second pass through nodes
		assert(totalchance);
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

	} else {
		//  Inform the player about an empty mine, unless
		//  there is a sufficiently high chance, that the mine
		//  will still produce enough.
		//  e.g. mines have m_chance=5, wells have 65
		if (m_chance <= 20)
			informPlayer(game, ps);

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

// Copied from militarysite.cc, MilitarySite::informPlayer
// Informs the player about a mine that has run empty, if the
// mine has not sent this message withing the last 60 seconds.
void ProductionProgram::ActMine::informPlayer
	(Game & game, ProductionSite & ps) const
{
	MessageQueue::addWithTimeout
		(game, ps.owner().player_number(),
		 60000, 0,
		 Message::create_building_message
		 	(MSG_MINE,
		 	 game.get_gametime(),
		 	 _("Mine empty"),
		 	 "<p font-size=14 font-face=FreeSerif>" +
		 	 std::string
		 	 	(_("One of your mines has run empty. Consider expanding it."))
		 	 	+
		 	 	"</p>",
		 	 ps));
}


ProductionProgram::ActCheck_Soldier::ActCheck_Soldier
	(char * parameters, ProductionSite_Descr const &)
{
	//  FIXME This is currently hardcoded for "soldier", but should allow any
	//  FIXME soldier type name.
	if (not match_force_skip(parameters, "soldier"))
		throw game_data_error
			(_("expected %s but found \"%s\""), _("soldier type"), parameters);
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
			throw game_data_error
				(_
				 	("expected {\"hp\"|\"attack\"|\"defense\"|evade\"} but found "
				 	 "\"%s\""),
				 parameters);

		char * endp;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		level = value;
		if (*endp or level != value)
			throw game_data_error
				(_("expected %s but found \"%s\""), _("level"), parameters);
	} catch (_wexception const & e) {
		throw game_data_error("check_soldier: %s", e.what());
	}
}

void ProductionProgram::ActCheck_Soldier::execute
	(Game & game, ProductionSite & ps) const
{
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


ProductionProgram::ActTrain::ActTrain
	(char * parameters, ProductionSite_Descr const &)
{
	//  FIXME This is currently hardcoded for "soldier", but should allow any
	//  FIXME soldier type name.
	if (not match_force_skip(parameters, "soldier"))
		throw game_data_error
			(_("expected %s but found \"%s\""), _("soldier type"), parameters);
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
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 "{\"hp\"|\"attack\"|\"defense\"|\"evade\"}", parameters);

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			level = value;
			if (*endp != ' ' or level != value)
				throw game_data_error
					(_("expected %s but found \"%s\""), _("level"), parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			target_level = value;
			if (*endp or target_level != value or target_level <= level)
				throw game_data_error
					(_("expected level > %u but found \"%s\""), level, parameters);
		}
	} catch (_wexception const & e) {
		throw game_data_error("train: %s", e.what());
	}
}

void ProductionProgram::ActTrain::execute
	(Game & game, ProductionSite & ps) const
{
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


//TODO: check if fx exists, load fx, lots of other checks for
//parameter correctness
ProductionProgram::ActPlayFX::ActPlayFX
	(char * parameters, ProductionSite_Descr const &)
{
	try {
		bool reached_end;
		name = match(parameters, reached_end);

		if (not reached_end) {
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			priority = value;
			if (*endp or priority != value)
				throw game_data_error
					(_("expected %s but found \"%s\""), _("priority"), parameters);
		} else
			priority = 127;
	} catch (_wexception const & e) {
		throw game_data_error("playFX: %s", e.what());
	}
}

void ProductionProgram::ActPlayFX::execute
	(Game & game, ProductionSite & ps) const
{
	g_sound_handler.play_fx(name, ps.m_position, priority);
	return ps.program_step(game);
}


ProductionProgram::ProductionProgram
	(std::string    const & directory,
	 Profile              & prof,
	 std::string    const &       _name,
	 char           const * const _descname,
	 ProductionSite_Descr * const building,
	 EncodeData     const * const encdata)
	: m_name(_name), m_descname(_descname)
{
	Section & program_s = prof.get_safe_section(_name.c_str());
	while (Section::Value * const v = program_s.get_next_val()) {
		ProductionProgram::Action * action;
		if      (not strcmp(v->get_name(), "return"))
			action = new ActReturn(v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "call"))
			action = new ActCall  (v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "sleep"))
			action = new ActSleep (v->get_string(),  *building);
		else if (not strcmp(v->get_name(), "animate"))
			action = new
				ActAnimate(v->get_string(), *building, directory, prof, encdata);
		else if (not strcmp(v->get_name(), "consume"))
			action = new ActConsume(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "produce"))
			action = new ActProduce(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "recruit"))
			action = new ActRecruit(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "worker"))
			action = new ActWorker (v->get_string(), *building, _name);
		else if (not strcmp(v->get_name(), "mine"))
			action = new ActMine   (v->get_string(), *building, _name);
		else if (not strcmp(v->get_name(), "check_soldier"))
			action = new ActCheck_Soldier(v->get_string(), *building);
		else if (not strcmp(v->get_name(), "train"))
			action = new ActTrain  (v->get_string(), *building);
		else if (not strcmp(v->get_name(), "playFX"))
			action = new ActPlayFX (v->get_string(), *building);
		else
			throw game_data_error
				(_("unknown command type \"%s\""), v->get_name());
		m_actions.push_back(action);
	}
	if (m_actions.empty())
		throw game_data_error("no actions");
}

}
