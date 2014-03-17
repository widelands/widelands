/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "logic/production_program.h"

#include <boost/format.hpp>
#include <config.h>
#include <libintl.h>

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/wares_queue.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "logic/checkstep.h"
#include "logic/findimmovable.h"
#include "logic/findnode.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/mapregion.h"
#include "logic/message_queue.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/soldier.h"
#include "logic/soldiercontrol.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/worker_program.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "upcast.h"

namespace Widelands {

// For formation of better translateable texts
using boost::format;


ProductionProgram::Action::~Action() {}

bool ProductionProgram::Action::get_building_work(Game &, ProductionSite &, Worker &) const
{
	return false;
}

void ProductionProgram::Action::building_work_failed(Game &, ProductionSite &, Worker &) const
{
}

void ProductionProgram::parse_ware_type_group
	(char            * & parameters,
	 Ware_Type_Group   & group,
	 const Tribe_Descr & tribe,
	 const BillOfMaterials  & inputs)
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
		for (wl_const_range<BillOfMaterials> i(inputs);; ++i)
			if (i.empty())
				throw game_data_error
					(
					 "%s is not declared as an input (\"%s=<count>\" was not "
					 "found in the [inputs] section)",
					 ware, ware);
			else if (i->first == ware_index) {
				count_max += i.front().second;
				break;
			}
		if
			(group.first.size()
			 and
			 ware_index.value() <= group.first.begin()->value())
			throw game_data_error
				(
				 "wrong order of ware types within group: ware type %s appears "
				 "after ware type %s (fix order!)",
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
					("expected %s but found \"%s\"", "count", parameters);
			parameters = endp;
			if (count_max < count)
				throw game_data_error
					(
					 "group count is %u but (total) input storage capacity of "
					 "the specified ware type(s) is only %u, so the group can "
					 "never be fulfilled by the site",
					 count, count_max);
			//  fallthrough
		}
		/* no break */
		case '\0':
		case ' ':
			group.second = count;
			return;
		case ',':
			++parameters;
			break;
		default:
			// scan for terminator should ensure that this cannot happen
			assert(false);
		}
	}
}


ProductionProgram::ActReturn::Condition::~Condition() {}

ProductionProgram::ActReturn::Negation::~Negation() {
	delete operand;
}
bool ProductionProgram::ActReturn::Negation::evaluate
	(const ProductionSite & ps) const
{
	return not operand->evaluate(ps);
}
std::string ProductionProgram::ActReturn::Negation::description
	(const Tribe_Descr & tribe) const
{
	/** TRANSLATORS: %s = e.g. 'economy needs ...' Context: 'and/or not %s' */
	return (boost::format(_("not %s")) % operand->description(tribe)).str();
}


bool ProductionProgram::ActReturn::Economy_Needs_Ware::evaluate
	(const ProductionSite & ps) const
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
	(const Tribe_Descr & tribe) const
{
	/** TRANSLATORS: e.g. 'economy needs water' Context: 'and/or (not) economy needs %s' */
	return (boost::format(_("economy needs %s")) % tribe.get_ware_descr(ware_type)->descname()).str();
}

bool ProductionProgram::ActReturn::Economy_Needs_Worker::evaluate
	(const ProductionSite & ps) const
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
	(const Tribe_Descr & tribe) const
{
	/** TRANSLATORS: e.g. 'economy needs worker' Context: 'and/or (not) economy needs %s' */
	return (boost::format(_("economy needs %s")) % tribe.get_ware_descr(worker_type)->descname()).str();
}

ProductionProgram::ActReturn::Site_Has::Site_Has
	(char * & parameters, const ProductionSite_Descr & descr)
{
	try {
		parse_ware_type_group(parameters, group, descr.tribe(), descr.inputs());
	} catch (const _wexception & e) {
		throw game_data_error
			("has ware_type1[,ware_type2[,...]][:N]: %s", e.what());
	}
}
bool ProductionProgram::ActReturn::Site_Has::evaluate
	(const ProductionSite & ps) const
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
	(const Tribe_Descr & tribe) const
{
	std::string condition = "";
	container_iterate_const(std::set<Ware_Index>, group.first, i) {
		condition =
		/** TRANSLATORS: Adds a ware to list of wares in 'Failed/Skipped ...' messages. */
			(boost::format(_("%1$s %2$s")) % condition % tribe.get_ware_descr(*i.current)->descname())
			 .str();
		/** TRANSLATORS: Separator for list of wares in 'Failed/Skipped ...' messages. */
		condition = (boost::format(_("%s,")) % condition).str();
	}
	if (1 < group.second) {
		// TODO this should be done with ngettext
		condition =
			(boost::format(_("%1$s (%2$i)")) % condition % static_cast<unsigned int>(group.second)).str();
	}
	/** TRANSLATORS: %s is a list of wares*/
	std::string result = (boost::format(_("site has%s")) % condition).str();
	return result;
}

bool ProductionProgram::ActReturn::Workers_Need_Experience::evaluate
	(const ProductionSite & ps) const
{
	ProductionSite::Working_Position const * const wp = ps.m_working_positions;
	for (uint32_t i = ps.descr().nr_working_positions(); i;)
		if (wp[--i].worker->needs_experience())
			return true;
	return false;
}
std::string ProductionProgram::ActReturn::Workers_Need_Experience::description
	(const Tribe_Descr &) const
{
	/** TRANSLATORS: 'Failed/Skipped ... because: workers need experience'. */
	return _("workers need experience");
}


ProductionProgram::ActReturn::Condition * create_economy_condition
	(char * & parameters, const Tribe_Descr & tribe)
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
						("expected %s but found \"%s\"",
						 "ware type or worker type", type_name);
			} catch (const _wexception & e) {
				throw game_data_error("needs: %s", e.what());
			}
		else
			throw game_data_error
				("expected %s but found \"%s\"", "\"needs\"", parameters);
	} catch (const _wexception & e) {
		throw game_data_error("economy: %s", e.what());
	}

	return nullptr; // will never be reached
}


ProductionProgram::ActReturn::Condition * create_site_condition
	(char * & parameters, const ProductionSite_Descr & descr)
{
	try {
		if (match_force_skip(parameters, "has"))
			return
				new ProductionProgram::ActReturn::Site_Has(parameters, descr);
		else
			throw game_data_error
				("expected %s but found \"%s\"", "\"has\"", parameters);
	} catch (const _wexception & e) {
		throw game_data_error("site: %s", e.what());
	}

	return nullptr; // will never be reached
}


ProductionProgram::ActReturn::Condition * create_workers_condition
	(char * & parameters)
{
	try {
		if (match(parameters, "need experience"))
			return new ProductionProgram::ActReturn::Workers_Need_Experience;
		else
			throw game_data_error
				("expected %s but found \"%s\"",
				 "\"need experience\"", parameters);
	} catch (const _wexception & e) {
		throw game_data_error("workers: %s", e.what());
	}

	return nullptr; // will never be reached
}


ProductionProgram::ActReturn::Condition *
ProductionProgram::ActReturn::create_condition
	(char * & parameters, const ProductionSite_Descr & descr)
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
	} catch (const _wexception & e) {
		throw game_data_error("invalid condition: %s", e.what());
	}

	return nullptr; // will never be reached
}


ProductionProgram::ActReturn::ActReturn
	(char * parameters, const ProductionSite_Descr & descr)
{
	try {
		if      (match(parameters, "failed"))    m_result = Failed;
		else if (match(parameters, "completed")) m_result = Completed;
		else if (match(parameters, "skipped"))   m_result = Skipped;
		else
			throw game_data_error
				("expected %s but found \"%s\"",
				"{\"failed\"|\"completed\"|\"skipped\"}", parameters);

		if (skip(parameters)) {
			if      (match_force_skip(parameters, "when")) {
				m_is_when = true;
				for (;;) {
					m_conditions.push_back(create_condition(parameters, descr));
					if (*parameters) {
						skip(parameters);
						if (not match_force_skip(parameters, "and"))
							throw game_data_error
								("expected \"%s\" or end of input", "and");
					} else
						break;
				}
			} else if (match_force_skip(parameters, "unless")) {
				m_is_when = false;
				for (;;) {
					if (not *parameters)
						throw game_data_error
							("expected condition at end of input");
					m_conditions.push_back(create_condition(parameters, descr));
					if (*parameters) {
						skip(parameters);
						if (not match_force_skip(parameters, "or"))
							throw game_data_error
								("expected \"%s\" or end of input", "or");
					} else
						break;
				}
			} else
				throw game_data_error
					("expected %s but found \"%s\"",
					 "{\"when\"|\"unless\"}", parameters);
		} else if (*parameters)
			throw game_data_error
				("expected %s but found \"%s\"",
				 ("space or end of input"), parameters);
		else
			m_is_when = true;

	} catch (const _wexception & e) {
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
		/** TRANSLATORS: 'Failed %s because (not): %s {and/or %s}' */
		m_result == Failed    ? (boost::format(_("Failed %s")) % ps.top_state().program->descname()).str() :
		/** TRANSLATORS: 'Completed %s because (not): %s {and/or %s}' */
		m_result == Completed ? (boost::format(_("Completed %s")) % ps.top_state().program->descname()).str() :
		/** TRANSLATORS: 'Skipped %s because (not): %s {and/or %s}' */
					(boost::format(_("Skipped %s")) % ps.top_state().program->descname()).str();

	if (!m_conditions.empty()) {
		std::string result_string = statistics_string;
		if (m_is_when) { //  'when a and b and ...' (all conditions must be true)
			std::string condition_string = "";
			for (wl_const_range<Conditions> i(m_conditions); i;)
			{
				if (not (i.front()->evaluate(ps))) //  A condition is false,
					return ps.program_step(game); //  continue program.

				condition_string += i.front()->description(ps.owner().tribe());
				if (i.advance().empty())
					break;
				// TODO  Would prefer "%1$s and %2$s" but getting segfaults, so leaving this for now
				/** TRANSLATORS: 'Failed/Completed/Skipped %s because: %s {and %s}' */
				condition_string = (boost::format(_("%s and ")) % condition_string).str();
			}
			result_string =
				/** TRANSLATORS: 'Failed/Completed/Skipped %s because: %s {and %s}' */
				(boost::format(_("%1$s because: %2$s")) % statistics_string % condition_string).str();
		} else { //  "unless a or b or ..." (all conditions must be false)
			std::string condition_string = "";
			for (wl_const_range<Conditions> i(m_conditions); i;)
			{
				if ((*i.current)->evaluate(ps)) //  A condition is true,
					return ps.program_step(game); //  continue program.

				condition_string += i.front()->description(ps.owner().tribe());
				if (i.advance().empty())
					break;
				// TODO  Would prefer '%1$s or %2$s' but getting segfaults, so leaving this for now
				/** TRANSLATORS: 'Failed/Completed/Skipped %s because not: %s {or %s}' */
				condition_string = (boost::format(_("%s or ")) % condition_string).str();
			}
			result_string =
				/** TRANSLATORS: 'Failed/Completed/Skipped %s because not: %s {or %s}' */
				(boost::format(_("%1$s because not: %2$s")) % statistics_string % condition_string).str();
		}
		snprintf
			(ps.m_result_buffer, sizeof(ps.m_result_buffer),
			 "%s", result_string.c_str());
	}
	// Commented out, as the information is already given in the hover text and
	// should *not* be shown in general statistics string, as:
	// 1) the strings are that long, that they clutter the screen (especially
	//    if you build up an "industry park" ;)
	// 2) in many cases the "real" statistics (in percent) are nearly no more
	//    shown, so the user has no idea if "skipped because .. is missing" is
	//    an exception or the normal case at the moment.
	/*
	snprintf
		(ps.m_statistics_buffer, sizeof(ps.m_statistics_buffer),
		 "%s", statistics_string.c_str());
	*/
	return ps.program_end(game, m_result);
}


ProductionProgram::ActCall::ActCall
	(char * parameters, const ProductionSite_Descr & descr)
{
	//  Initialize with default handling methods.
	m_handling_methods[Failed    - 1] = Continue;
	m_handling_methods[Completed - 1] = Continue;
	m_handling_methods[Skipped   - 1] = Continue;

	try {
		bool reached_end;
		{
			char const * const program_name = match(parameters, reached_end);
			const ProductionSite_Descr::Programs & programs = descr.programs();
			ProductionSite_Descr::Programs::const_iterator const it =
				programs.find(program_name);
			if (it == programs.end())
				throw game_data_error
					(
					 "the program \"%s\" has not (yet) been declared in %s "
					 "(wrong declaration order?)",
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
						("%s handling method already defined", "failure");
				result_to_set_method_for = Failed;
			} else if (match_force_skip(parameters, "completion")) {
				if (m_handling_methods[Completed - 1] != Continue)
					throw game_data_error
						("%s handling method already defined", "completion");
				result_to_set_method_for = Completed;
			} else if (match_force_skip(parameters, "skip"))       {
				if (m_handling_methods[Skipped   - 1] != Continue)
					throw game_data_error
						("%s handling method already defined", "skip");
				result_to_set_method_for = Skipped;
			} else
				throw game_data_error
					("expected %s but found \"%s\"",
					 "{\"failure\"|\"completion\"|\"skip\"}", parameters);

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
					("expected %s but found \"%s\"",
					 "{\"fail\"|\"complete\"|\"skip\"|\"repeat\"}",
					 parameters);
			m_handling_methods[result_to_set_method_for - 1] = handling_method;
			reached_end = not *parameters;
			log
				("read handling method for result %u: %u, parameters = \"%s\", "
				 "reached_end = %u\n",
				 result_to_set_method_for, handling_method,
				 parameters, reached_end);
		}
	} catch (const _wexception & e) {
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
		break;
	default:
		throw wexception("ProductionProgram call: bad result handling method");
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
		const Worker_Descr & main_worker_descr =
			*descr.tribe().get_worker_descr(descr.working_positions()[0].first);

		//  This will fail unless the main worker has a program with the given
		//  name, so it also validates the parameter.
		const Workarea_Info & worker_workarea_info =
			main_worker_descr.get_program(m_program)->get_workarea_info();
		Workarea_Info & building_workarea_info = descr.m_workarea_info;
		container_iterate_const(Workarea_Info, worker_workarea_info, i) {
			std::set<std::string> & building_radius_infos =
				building_workarea_info[i.current->first];
			const std::set<std::string> & descriptions = i.current->second;
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
	} catch (const _wexception & e) {
		throw game_data_error("worker: %s", e.what());
	}
}

void ProductionProgram::ActWorker::execute
	(Game & game, ProductionSite & ps) const
{
	// Always main worker is doing stuff
	ps.m_working_positions[0].worker->update_task_buildingwork(game);
}

bool ProductionProgram::ActWorker::get_building_work
	(Game & game, ProductionSite & psite, Worker & worker) const
{
	ProductionSite::State & state = psite.top_state();
	if (state.phase == 0) {
		worker.start_task_program(game, program());
		++state.phase;
		return true;
	} else {
		psite.program_step(game);
		return false;
	}
}

void ProductionProgram::ActWorker::building_work_failed
	(Game & game, ProductionSite & psite, Worker &) const
{
	psite.program_end(game, Failed);
}


ProductionProgram::ActSleep::ActSleep(char * parameters, const ProductionSite_Descr &)
{
	try {
		if (*parameters) {
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			m_duration = value;
			if (*endp or value <= 0 or m_duration != value)
				throw game_data_error
					("expected %s but found \"%s\"",
					 "duration in ms", parameters);
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (const _wexception & e) {
		throw game_data_error("sleep: %s", e.what());
	}
}

void ProductionProgram::ActSleep::execute(Game & game, ProductionSite & ps) const
{
	return
		ps.program_step(game, m_duration ? m_duration : ps.top_state().phase);
}


ProductionProgram::ActCheck_Map::ActCheck_Map(char * parameters, const ProductionSite_Descr &)
{
	try {
		if (*parameters) {
			if (!strcmp(parameters, "seafaring"))
				m_feature = SEAFARING;
			else
				throw game_data_error("Unknown parameter \"%s\"", parameters);
		} else
			throw game_data_error("No parameter given!");
	} catch (const _wexception & e) {
		throw game_data_error("sleep: %s", e.what());
	}
}

void ProductionProgram::ActCheck_Map::execute(Game & game, ProductionSite & ps) const
{
	switch (m_feature) {
		case SEAFARING: {
			if (game.map().get_port_spaces().size() > 1) // we need at least two port build spaces
				return ps.program_step(game, 0);
			else {
				snprintf(ps.m_result_buffer, sizeof(ps.m_result_buffer), "No use for ships on this map!");
				return ps.program_end(game, None);
			}
		}
		default:
			assert(false);
			break;
	}
}


ProductionProgram::ActAnimate::ActAnimate
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & directory, Profile & prof)
{
	try {
		bool reached_end;
		char * const animation_name = match(parameters, reached_end);
		if (not strcmp(animation_name, "idle"))
			throw game_data_error
				("idle animation is default; calling is not allowed");
		if (descr.is_animation_known(animation_name))
			m_id = descr.get_animation(animation_name);
		else {
			m_id = g_gr->animations().load(directory.c_str(), prof.get_safe_section(animation_name));
			descr.add_animation(animation_name, m_id);
		}
		if (not reached_end) { //  The next parameter is the duration.
			char * endp;
			long long int const value = strtoll(parameters, &endp, 0);
			m_duration = value;
			if (*endp or value <= 0 or m_duration != value)
				throw game_data_error
					("expected %s but found \"%s\"",
					 "duration in ms", parameters);
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (const _wexception & e) {
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
	(char * parameters, const ProductionSite_Descr & descr)
{
	try {
		const Tribe_Descr & tribe = descr.tribe();
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
				("expected ware_type1[,ware_type2[,...]][:N] ...");
	} catch (const _wexception & e) {
		throw game_data_error("consume: %s", e.what());
	}
}

void ProductionProgram::ActConsume::execute
	(Game & game, ProductionSite & ps) const
{
	std::vector<WaresQueue *> const warequeues = ps.warequeues();
	size_t const nr_warequeues = warequeues.size();
	std::vector<uint8_t> consumption_quantities(nr_warequeues, 0);

	Groups l_groups = m_groups; //  make a copy for local modification
	//log("ActConsume::execute(%s):\n", ps.descname().c_str());

	//  Iterate over all input queues and see how much we should consume from
	//  each of them.
	for (size_t i = 0; i < nr_warequeues; ++i) {
		Ware_Index const ware_type = warequeues[i]->get_ware();
		uint8_t nr_available = warequeues[i]->get_filled();
		consumption_quantities[i] = 0;

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
				} else {
					consumption_quantities[i] += nr_available;
					it->second                -= nr_available;
					++it; //  Now check if the next group includes this ware type.
				}
			} else
				++it;
	}

	if (uint8_t const nr_missing_groups = l_groups.size()) {
		const Tribe_Descr & tribe = ps.owner().tribe();
		std::string result_string =
			/** TRANSLATORS: e.g. 'Failed work because: water, wheat (2) are missing' */
			(boost::format(_("Failed %s because:")) % ps.top_state().program->descname()).str();

		for (wl_const_range<Groups> i(l_groups); i;)
		{
			assert(i.current->first.size());
			for (wl_const_range<std::set<Ware_Index> > j(i.current->first); j;)
			{
				result_string =
					/** TRANSLATORS: Adds a ware to list of wares in 'Failed/Skipped ...' messages. */
					(boost::format(_("%1$s %2$s")) % result_string
					 % tribe.get_ware_descr(j.front())->descname())
					 .str();
				if (j.advance().empty())
					break;
				/** TRANSLATORS: Separator for list of wares in 'Failed/Skipped ...' messages. */
				result_string = (boost::format(_("%s,")) % result_string).str();
			}
			{
				uint8_t const count = i.current->second;
				if (1 < count) {
					// TODO this should be done with ngettext
					result_string =
						/** TRANSLATORS: e.g. 'Failed work because: water, wheat (2) are missing' */
						(boost::format(_("%1$s (%2$i)")) % result_string
						 % static_cast<unsigned int>(count))
						 .str();
				}
			}
			if (i.advance().empty())
				break;
			result_string = (boost::format(_("%s and")) % result_string).str();
		}
		result_string =
			/** TRANSLATORS: e.g. %1$s = 'Failed work because: water, wheat (2)' %2$s = 'are missing' */
			(boost::format(_("%1$s %2$s")) % result_string
			/** TRANSLATORS: e.g. 'Failed work because: water, wheat (2) are missing' */
			 % ngettext(" is missing", " are missing", nr_missing_groups))
			 .str();

		snprintf
			(ps.m_result_buffer, sizeof(ps.m_result_buffer),
			 "%s", result_string.c_str());
		return ps.program_end(game, Failed);
	} else { //  we fulfilled all consumption requirements
		for (size_t i = 0; i < nr_warequeues; ++i)
			if (uint8_t const q = consumption_quantities[i]) {
				assert(q <= warequeues[i]->get_filled());
				warequeues[i]->set_filled(warequeues[i]->get_filled() - q);

				//update consumption statistic
				ps.owner().ware_consumed(warequeues[i]->get_ware(), q);
			}
		return ps.program_step(game);
	}
}


ProductionProgram::ActProduce::ActProduce
	(char * parameters, const ProductionSite_Descr & descr)
{
	try {
		const Tribe_Descr & tribe = descr.tribe();
		for (bool more = true; more; ++parameters) {
			m_items.resize(m_items.size() + 1);
			std::pair<Ware_Index, uint8_t> & item = *m_items.rbegin();
			skip(parameters);
			char const * ware = parameters;
			for (;; ++parameters) {
				switch (*parameters) {
				default:
					break;
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
							("expected %s but found \"%s\"",
							 "count", parameters);
					parameters = endp;
					goto item_end;
				}
				}
			}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			if
				(not
				 descr.is_output_ware_type
				 	(item.first = tribe.safe_ware_index(ware)))
				throw game_data_error
					(
					 "%s is not declared as an output (\"output=%s\" was not "
					 "found in the [global] section)",
					 ware, ware);
		}
	} catch (const _wexception & e) {
		throw game_data_error("produce: %s", e.what());
	}
}

void ProductionProgram::ActProduce::execute
	(Game & game, ProductionSite & ps) const
{
	//ps.molog("  Produce\n");
	assert(ps.m_produced_wares.empty());
	ps.m_produced_wares = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	const Tribe_Descr & tribe = ps.owner().tribe();
	std::string result_string = "";
	assert(m_items.size());

	for (wl_const_range<Items> i(m_items); i;)
	{
		{
			uint8_t const count = i.current->second;
			if (1 < count) {
				char buffer[5];
				/** TRANSLATORS: Number used in list of wares */
				sprintf(buffer, _("%u "), count);
				result_string += buffer;
			}
		}
		result_string += tribe.get_ware_descr(i.current->first)->descname();
		if (i.advance().empty())
			break;
		/** TRANSLATORS: Separator for list of wares */
		result_string += _(", ");
	}
	// Keep translateability in mind!
	/** TRANSLATORS: %s is a list of wares */
	result_string = str(format(_("Produced %s")) % result_string);
	snprintf
		(ps.m_result_buffer, sizeof(ps.m_result_buffer),
		 "%s", result_string.c_str());
}

bool ProductionProgram::ActProduce::get_building_work
	(Game & game, ProductionSite & psite, Worker & /* worker */) const
{
	// We reach this point once all wares have been carried outside the building
	psite.program_step(game);
	return false;
}


ProductionProgram::ActRecruit::ActRecruit
	(char * parameters, const ProductionSite_Descr & descr)
{
	try {
		const Tribe_Descr & tribe = descr.tribe();
		for (bool more = true; more; ++parameters) {
			m_items.resize(m_items.size() + 1);
			std::pair<Ware_Index, uint8_t> & item = *m_items.rbegin();
			skip(parameters);
			char const * worker = parameters;
			for (;; ++parameters) {
				switch (*parameters) {
				default:
					break;
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
							("expected %s but found \"%s\"",
							 "count", parameters);
					parameters = endp;
					goto item_end;
				}
				}
			}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			if
				(not
				 descr.is_output_worker_type
				 	(item.first = tribe.safe_worker_index(worker)))
				throw game_data_error
					(
					 "%s is not declared as an output (\"output=%s\" was not "
					 "found in the [global] section)",
					 worker, worker);
		}
	} catch (const _wexception & e) {
		throw game_data_error("recruit: %s", e.what());
	}
}

void ProductionProgram::ActRecruit::execute
	(Game & game, ProductionSite & ps) const
{
	assert(ps.m_recruited_workers.empty());
	ps.m_recruited_workers = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	const Tribe_Descr & tribe = ps.owner().tribe();
	std::string unit_string = ("");
	assert(m_items.size());
	for (wl_const_range<Items> i(m_items); i;)
	{
		{
			uint8_t const count = i.current->second;
			if (1 < count) {
				char buffer[5];
				/** TRANSLATORS: Number used in list of workers */
				sprintf(buffer, _("%u "), count);
				unit_string += buffer;
			}
		}
		unit_string += tribe.get_worker_descr(i.current->first)->descname();
		if (i.advance().empty())
			break;
		/** TRANSLATORS: Separator for list of workers */
		unit_string += _(", ");
	}
	/** TRANSLATORS: %s is a lost of workers */
	std::string result_string = (boost::format(_("Recruited %s")) % unit_string).str();
	snprintf
		(ps.m_result_buffer, sizeof(ps.m_result_buffer),
		 "%s", result_string.c_str());
}

bool ProductionProgram::ActRecruit::get_building_work
	(Game & game, ProductionSite & psite, Worker & /* worker */) const
{
	// We reach this point once all recruits have been guided outside the building
	psite.program_step(game);
	return false;
}


ProductionProgram::ActMine::ActMine
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		const World & world = descr.tribe().world();
		assert(&world);
		bool reached_end;
		m_resource = world.safe_resource_index(match(parameters, reached_end));

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_distance = value;
			if (*endp != ' ' or m_distance != value)
				throw game_data_error
					("expected %s but found \"%s\"", "distance", parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_max = value;
			if (*endp != ' ' or value < 1 or 100 < value)
				throw game_data_error
					("expected %s but found \"%s\"",
					 "percentage", parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			m_chance = value;
			if (*endp or value < 1 or 100 < value)
				throw game_data_error
					("expected %s but found \"%s\"",
					 "percentage", parameters);
		}
		std::string description =
			/** TRANSLATORS: %1$s = production site name, %2$s = production program name, %3$s = resource */
			(boost::format(_("%1$s %2$s mine %3$s")) % descr.descname() % production_program_name
				% world.get_resource(m_resource)->descname())
				.str();

		descr.m_workarea_info[m_distance].insert(description);
	} catch (const _wexception & e) {
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
		if (m_chance <= 20) {
			informPlayer(game, ps);
			// and change the default animation
			ps.set_default_anim("empty");
		}

		//  Mine has reached its limits, still try to produce something but
		//  independent of sourrunding resources. Do not decrease resources
		//  further.
		if (m_chance <= game.logic_rand() % 100) {
			snprintf
				(ps.m_result_buffer, sizeof(ps.m_result_buffer),
				 _("Can’t find any more resources!"));
			return ps.program_end(game, Failed);
		}
	}

	//  done successful
	//  FIXME Should pass the time it takes to mine in the phase parameter of
	//  FIXME ProductionSite::program_step so that the following sleep/animate
	//  FIXME command knows how long it should last.
	return ps.program_step(game);
}

/// Copied from militarysite.cc, MilitarySite::informPlayer
/// Informs the player about a mine that has run empty, if the mine has not
/// sent this message within the last 60 minutes.
void ProductionProgram::ActMine::informPlayer
	(Game & game, ProductionSite & ps) const
{
	ps.send_message
		(game,
		 "mine",
		 _("Main Vein Exhausted"),
		 _
		 ("This mine’s main vein is exhausted. Expect strongly diminished returns on investment. "
		  "You should consider expanding, dismantling or destroying it."),
		 true,
		 60 * 60 * 1000,
		 0);
}


ProductionProgram::ActCheck_Soldier::ActCheck_Soldier
	(char * parameters, const ProductionSite_Descr &)
{
	//  FIXME This is currently hardcoded for "soldier", but should allow any
	//  FIXME soldier type name.
	if (not match_force_skip(parameters, "soldier"))
		throw game_data_error
			("expected %s but found \"%s\"", "soldier type", parameters);
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
				("expected %s but found \"%s\"",
					"{\"hp\"|\"attack\"|\"defense\"|\"evade\"}", parameters);

		char * endp;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		level = value;
		if (*endp or level != value)
			throw game_data_error
				("expected %s but found \"%s\"", "level", parameters);
	} catch (const _wexception & e) {
		throw game_data_error("check_soldier: %s", e.what());
	}
}

void ProductionProgram::ActCheck_Soldier::execute
	(Game & game, ProductionSite & ps) const
{
	SoldierControl & ctrl = dynamic_cast<SoldierControl &>(ps);
	const std::vector<Soldier *> soldiers = ctrl.presentSoldiers();
	if (soldiers.empty()) {
			snprintf
				(ps.m_result_buffer, sizeof(ps.m_result_buffer),
				 _("No soldier to train!"));
		return ps.program_end(game, Skipped);
	}
	ps.molog("  Checking soldier (%u) level %d)\n", attribute, level);

	const std::vector<Soldier *>::const_iterator soldiers_end = soldiers.end();
	for (std::vector<Soldier *>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end) {
			snprintf
				(ps.m_result_buffer, sizeof(ps.m_result_buffer),
				 _("No soldier found for this training level!"));
			return ps.program_end(game, Skipped);
		}
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

	upcast(TrainingSite, ts, &ps);
	ts->trainingAttempted(attribute, level);

	ps.molog("  Check done!\n");

	return ps.program_step(game);
}


ProductionProgram::ActTrain::ActTrain
	(char * parameters, const ProductionSite_Descr &)
{
	//  FIXME This is currently hardcoded for "soldier", but should allow any
	//  FIXME soldier type name.
	if (not match_force_skip(parameters, "soldier"))
		throw game_data_error
			("expected %s but found \"%s\"", "soldier type", parameters);
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
				("expected %s but found \"%s\"",
				 "{\"hp\"|\"attack\"|\"defense\"|\"evade\"}", parameters);

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			level = value;
			if (*endp != ' ' or level != value)
				throw game_data_error
					("expected %s but found \"%s\"", "level", parameters);
			parameters = endp;
		}

		{
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			target_level = value;
			if (*endp or target_level != value or target_level <= level)
				throw game_data_error
					("expected level > %u but found \"%s\"", level, parameters);
		}
	} catch (const _wexception & e) {
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
		if (it == soldiers_end) {
			snprintf
				(ps.m_result_buffer, sizeof(ps.m_result_buffer),
				 _("No soldier found for this training level!"));
			return ps.program_end(game, Skipped);
		}
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

	upcast(TrainingSite, ts, &ps);
	ts->trainingSuccessful(attribute, level);


	return ps.program_step(game);
}

ProductionProgram::ActPlayFX::ActPlayFX
	(const std::string & directory, char * parameters, const ProductionSite_Descr &)
{
	try {
		bool reached_end;
		std::string filename = match(parameters, reached_end);
		name = directory + "/" + filename;

		if (not reached_end) {
			char * endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			priority = value;
			if (*endp or priority != value)
				throw game_data_error
					("expected %s but found \"%s\"", "priority", parameters);
		} else
			priority = 127;

		g_sound_handler.load_fx_if_needed(directory, filename, name);
	} catch (const _wexception & e) {
		throw game_data_error("playFX: %s", e.what());
	}
}

void ProductionProgram::ActPlayFX::execute
	(Game & game, ProductionSite & ps) const
{
	g_sound_handler.play_fx(name, ps.m_position, priority);
	return ps.program_step(game);
}

ProductionProgram::ActConstruct::ActConstruct
	(char * parameters, ProductionSite_Descr & descr,
	 const std::string & production_program_name)
{
	try {
		std::vector<std::string> params = split_string(parameters, " ");

		if (params.size() != 3)
			throw game_data_error("usage: construct object-name worker-program radius:NN");

		objectname = params[0];
		workerprogram = params[1];
		radius = stringTo<uint32_t>(params[2]);

		std::set<std::string> & building_radius_infos = descr.m_workarea_info[radius];
		std::string description = descr.name() + ' ' + production_program_name;
		description += " construct ";
		description += objectname;
		building_radius_infos.insert(description);
	} catch (const _wexception & e) {
		throw game_data_error("construct: %s", e.what());
	}
}

const Immovable_Descr & ProductionProgram::ActConstruct::get_construction_descr
	(ProductionSite & psite) const
{
	const Immovable_Descr * descr = psite.tribe().get_immovable_descr(objectname);
	if (!descr)
		throw wexception("ActConstruct: immovable '%s' does not exist", objectname.c_str());

	return *descr;
}


void ProductionProgram::ActConstruct::execute(Game & g, ProductionSite & psite) const
{
	ProductionSite::State & state = psite.top_state();
	const Immovable_Descr & descr = get_construction_descr(psite);

	// Early check for no resources
	const Buildcost & buildcost = descr.buildcost();
	Ware_Index available_resource = Ware_Index::Null();

	for (Buildcost::const_iterator it = buildcost.begin(); it != buildcost.end(); ++it) {
		if (psite.waresqueue(it->first).get_filled() > 0) {
			available_resource = it->first;
			break;
		}
	}

	if (!available_resource) {
		psite.program_end(g, Failed);
		return;
	}

	// Look for an appropriate object in the given radius
	std::vector<ImmovableFound> immovables;
	CheckStepWalkOn cstep(MOVECAPS_WALK, true);
	Area<FCoords> area (g.map().get_fcoords(psite.base_flag().get_position()), radius);
	if
		(g.map().find_reachable_immovables
		 (area, &immovables, cstep, FindImmovableByDescr(descr)))
	{
		state.objvar = immovables[0].object;

		psite.m_working_positions[0].worker->update_task_buildingwork(g);
		return;
	}

	// No object found, look for a field where we can build
	std::vector<Coords> fields;
	FindNodeAnd fna;
	fna.add(FindNodeShore());
	fna.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone));
	if
		(g.map().find_reachable_fields
			(area, &fields, cstep, fna))
	{
		state.coord = fields[0];

		psite.m_working_positions[0].worker->update_task_buildingwork(g);
		return;
	}

	psite.molog("construct: no object or buildable field\n");
	psite.program_end(g, Failed);
}

bool ProductionProgram::ActConstruct::get_building_work
	(Game & game, ProductionSite & psite, Worker & worker) const
{
	ProductionSite::State & state = psite.top_state();
	if (state.phase >= 1) {
		psite.program_step(game);
		return false;
	}

	// First step: figure out which ware item to bring along
	Buildcost remaining;
	WaresQueue * wq = nullptr;

	Immovable * construction = dynamic_cast<Immovable *>(state.objvar.get(game));
	if (construction) {
		if (!construction->construct_remaining_buildcost(game, &remaining)) {
			psite.molog("construct: immovable %u not under construction", construction->serial());
			psite.program_end(game, Failed);
			return false;
		}
	} else {
		const Immovable_Descr & descr = get_construction_descr(psite);
		remaining = descr.buildcost();
	}

	for (Buildcost::const_iterator it = remaining.begin(); it != remaining.end(); ++it) {
		WaresQueue & thiswq = psite.waresqueue(it->first);
		if (thiswq.get_filled() > 0) {
			wq = &thiswq;
			break;
		}
	}

	if (!wq) {
		psite.program_end(game, Failed);
		return false;
	}

	// Second step: give ware to worker
	WareInstance * ware = new WareInstance(wq->get_ware(), psite.tribe().get_ware_descr(wq->get_ware()));
	ware->init(game);
	worker.set_carried_ware(game, ware);
	wq->set_filled(wq->get_filled() - 1);

	// Third step: send worker on his merry way, giving the target object or coords
	worker.start_task_program(game, workerprogram);
	worker.top_state().objvar1 = construction;
	worker.top_state().coords = state.coord;

	state.phase = 1;
	return true;
}

void ProductionProgram::ActConstruct::building_work_failed
	(Game & game, ProductionSite & psite, Worker &) const
{
	psite.program_end(game, Failed);
}



ProductionProgram::ProductionProgram
	(const std::string    & directory,
	 Profile              & prof,
	 const std::string    &       _name,
	 char           const * const _descname,
	 ProductionSite_Descr * const building)
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
				ActAnimate(v->get_string(), *building, directory, prof);
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
			action = new ActPlayFX (directory, v->get_string(), *building);
		else if (not strcmp(v->get_name(), "construct"))
			action = new ActConstruct (v->get_string(), *building, _name);
		else if (not strcmp(v->get_name(), "check_map"))
			action = new ActCheck_Map(v->get_string(), *building);
		else
			throw game_data_error
				("unknown command type \"%s\"", v->get_name());
		m_actions.push_back(action);
	}
	if (m_actions.empty())
		throw game_data_error("no actions");
}

}
