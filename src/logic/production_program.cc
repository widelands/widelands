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

#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "config.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/wares_queue.h"
#include "graphic/graphic.h"
#include "helper.h"
#include "io/filesystem/layered_filesystem.h"
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
#include "logic/world/resource_description.h"
#include "logic/world/world.h"
#include "sound/sound_handler.h"

namespace Widelands {

namespace {

/// Returns input as a number. If the input isn't correct, uses the description to throw a GameDataError
/// Optionally define lower and upper bounds != 0 for the result value
template <typename T> T get_unsigned_number(
		const std::string& input,
		const char* description,
		T lower_bound = 0,
		T upper_bound = 0) {
	T result = 0;
	const unsigned long long int value = boost::lexical_cast<unsigned long long int>(input);
	result = value;
	if (value <= 0 || result != value) {
		throw GameDataError
			("expected %s but found \"%s\"", description, input.c_str());
	}
	if (lower_bound != 0 && value < lower_bound) {
		throw GameDataError
			("expected %s >= %u but found \"%llu\"", description, lower_bound, value);
	}
	if (upper_bound != 0 && value > upper_bound) {
		throw GameDataError
			("expected %s <= %u but found \"%llu\"", description, upper_bound, value);
	}

	return result;
}

ProductionProgram::ActReturn::Condition * create_economy_condition
	(std::vector<std::string>& parameters, const Tribes& tribes)
{
	if (parameters.size() < 2) {
		throw GameDataError("Not enough parameters for create_economy_condition");
	}
	try {
		if (parameters.front() == "needs")
			try {
				const std::string& type_name = parameters[1];
				const WareIndex& wareindex = tribes.ware_index(type_name);
				if (tribes.ware_exists(wareindex)) {
					for (int i = 0; i < static_cast<int>(tribes.nrtribes()); ++i) {
						const TribeDescr& tribe_descr = *tribes.get_tribe_descr(i);
						if (tribe_descr.has_ware(wareindex)) {
							tribes.set_ware_type_has_demand_check(wareindex, tribe_descr.name());
						}
					}
					return
						new ProductionProgram::ActReturn::EconomyNeedsWare
							(wareindex);
				} else if (tribes.worker_exists(tribes.worker_index(type_name))) {
					const WareIndex& workerindex = tribes.worker_index(type_name);
					for (int i = 0; i < static_cast<int>(tribes.nrtribes()); ++i) {
						const TribeDescr* tribe_descr = tribes.get_tribe_descr(i);
						if (tribe_descr->has_worker(workerindex)) {
							tribes.set_worker_type_has_demand_check(workerindex);
						}
					}
					return
						new ProductionProgram::ActReturn::EconomyNeedsWorker
							(wareindex);
				} else
					throw GameDataError
						("expected %s but found \"%s\"",
						 "ware type or worker type", type_name.c_str());
			} catch (const WException & e) {
				throw GameDataError("needs: %s", e.what());
			}
		else
			throw GameDataError
				("expected %s but found \"%s\"", "\"needs\"", parameters.front().c_str());
	} catch (const WException & e) {
		throw GameDataError("economy: %s", e.what());
	}
}


ProductionProgram::ActReturn::Condition * create_site_condition
	(std::vector<std::string>& parameters, const ProductionSiteDescr & descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for create_site_condition");
	}
	try {
		if (parameters.front() == "has") {
			parameters.erase(parameters.begin());
			return
				new ProductionProgram::ActReturn::SiteHas(parameters, descr, tribes);
		}
		else {
			throw GameDataError
				("expected %s but found \"%s\"", "\"has\"", parameters.front().c_str());
		}
	} catch (const WException & e) {
		throw GameDataError("site: %s", e.what());
	}
}


ProductionProgram::ActReturn::Condition * create_workers_condition
	(std::vector<std::string>& parameters)
{
	if (parameters.size() < 2) {
		throw GameDataError("Not enough parameters for create_workers_condition");
	}
	try {
		if (parameters[0] == "need" && parameters[1] == "experience")
			return new ProductionProgram::ActReturn::WorkersNeedExperience;
		else
			throw GameDataError
				("expected %s but found \"%s %s\"",
				 "\"need experience\"", parameters[0].c_str(), parameters[1].c_str());
	} catch (const WException & e) {
		throw GameDataError("workers: %s", e.what());
	}
}



}  // namespace

ProductionProgram::Action::~Action() {}

bool ProductionProgram::Action::get_building_work(Game &, ProductionSite &, Worker &) const
{
	return false;
}

void ProductionProgram::Action::building_work_failed(Game &, ProductionSite &, Worker &) const
{
}

void ProductionProgram::parse_ware_type_group
	(std::vector<std::string>& parameters,
	 WareTypeGroup& group,
	 const Tribes& tribes,
	 const BillOfMaterials& inputs)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for parse_ware_type_group");
	}
	std::set<WareIndex>::iterator last_insert_pos = group.first.end();
	while (!parameters.empty()) {

		// Get 1 set of ware parameters
		std::vector<std::string> ware_parameters;
		boost::split(ware_parameters, parameters.front(), boost::is_any_of(",:"));
		const std::string& warename = ware_parameters.front();

		WareIndex const ware_index = tribes.safe_ware_index(warename);

		uint8_t count_max = 0;
		for (BillOfMaterials::const_iterator input_it = inputs.begin(); input_it != inputs.end(); ++input_it) {
			if (input_it == inputs.end()) {
				throw GameDataError
					(
					 "%s is not declared as an input for this building",
					 warename.c_str());
			}
			else if (input_it->first == ware_index) {
				count_max += input_it->second;
				break;
			}
		}

		if (group.first.size() && ware_index <= *group.first.begin())
			throw GameDataError
				(
				 "wrong order of ware types within group: ware type %s appears "
				 "after ware type %s (fix order!)",
				 warename.c_str(),
				 tribes.get_ware_descr(*group.first.begin())->name().c_str());
		last_insert_pos = group.first.insert(last_insert_pos, ware_index);

		// More than one of this ware is wanted
		if (parameters.front().find(':') != std::string::npos) {
			uint8_t count = get_unsigned_number<uint8_t>(parameters.front(), "number of wares", 1);
			if (count_max < count)
				throw GameDataError
					(
					 "group count is %u but (total) input storage capacity of "
					 "the specified ware type(s) is only %u, so the group can "
					 "never be fulfilled by the site",
					 count, count_max);
			group.second = count;
		// Another ware can be used, put it on the queue
		} else if (parameters.front().find(',') != std::string::npos) {
			parameters.push_back(ware_parameters[1]);
		} else {
			assert(false);
		}
		parameters.erase(parameters.begin()); // Move to the next entry
	}
}


ProductionProgram::ActReturn::Condition::~Condition() {}

ProductionProgram::ActReturn::Negation::~Negation() {
	delete operand;
}
bool ProductionProgram::ActReturn::Negation::evaluate
	(const ProductionSite & ps) const
{
	return !operand->evaluate(ps);
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string ProductionProgram::ActReturn::Negation::description
	(const Tribes&) const
{
	return "";
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string ProductionProgram::ActReturn::Negation::description_negation
	(const Tribes&) const
{
	return "";
}


bool ProductionProgram::ActReturn::EconomyNeedsWare::evaluate
	(const ProductionSite & ps) const
{
	return ps.get_economy()->needs_ware(ware_type);
}
std::string ProductionProgram::ActReturn::EconomyNeedsWare::description
	(const Tribes& tribes) const
{
	/** TRANSLATORS: %s is a ware or worker's generic mass name, e.g. "suits of armor" for the ware "Armor". */
	/** TRANSLATORS: Completed/Skipped/Did not start ... because the economy needs suits of armor */
	std::string result =  (boost::format(_("the economy needs %s"))
			  % tribes.get_ware_descr(ware_type)->genericname()).str();
	return result;
}
std::string ProductionProgram::ActReturn::EconomyNeedsWare::description_negation
	(const Tribes& tribes) const
{
	/** TRANSLATORS: %s is a ware or worker's generic mass name, e.g. "suits of armor" for the ware "Armor". */
	/** TRANSLATORS: Completed/Skipped/Did not start ... because the economy doesn’t need suits of armor */
	std::string result = (boost::format(_("the economy doesn’t need %s"))
			  % tribes.get_ware_descr(ware_type)->genericname()).str();
	return result;
}

bool ProductionProgram::ActReturn::EconomyNeedsWorker::evaluate
	(const ProductionSite & ps) const
{
	return ps.get_economy()->needs_worker(worker_type);
}
std::string ProductionProgram::ActReturn::EconomyNeedsWorker::description
	(const Tribes& tribes) const
{
	std::string result = (boost::format(_("the economy needs %s"))
			  % tribes.get_worker_descr(worker_type)->genericname()).str();
	return result;
}

std::string ProductionProgram::ActReturn::EconomyNeedsWorker::description_negation
	(const Tribes& tribes) const
{
	std::string result = (boost::format(_("the economy doesn’t need %s"))
			  % tribes.get_worker_descr(worker_type)->descname()).str();
	return result;
}


ProductionProgram::ActReturn::SiteHas::SiteHas
	(std::vector<std::string>& parameters, const ProductionSiteDescr & descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for SiteHas");
	}
	try {
		parse_ware_type_group(parameters, group, tribes, descr.inputs());
	} catch (const WException & e) {
		throw GameDataError
			("has ware_type1[,ware_type2[,...]][:N]: %s", e.what());
	}
}
bool ProductionProgram::ActReturn::SiteHas::evaluate
	(const ProductionSite & ps) const
{
	uint8_t count = group.second;
	for (WaresQueue * ip_queue : ps.warequeues()) {
		if (group.first.count(ip_queue->get_ware())) {
			uint8_t const filled = ip_queue->get_filled();
			if (count <= filled)
				return true;
			count -= filled;
		}
	}
	return false;
}


std::string ProductionProgram::ActReturn::SiteHas::description
	(const Tribes& tribes) const
{
	std::vector<std::string> condition_list;
	for (const WareIndex& temp_ware : group.first) {
		condition_list.push_back(tribes.get_ware_descr(temp_ware)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		/** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		/** TRANSLATORS:    %1$s = "3" */
		/** TRANSLATORS:    %2$i = "water" */
		condition = (boost::format(_("%1$ix %2$s"))
						 % static_cast<unsigned int>(group.second)
						 % condition).str();
	}

	/** TRANSLATORS: %s is a list of wares*/
	std::string result = (boost::format(_("the building has the following wares: %s")) % condition).str();
	return result;
}

std::string ProductionProgram::ActReturn::SiteHas::description_negation
	(const Tribes& tribes) const
{
	std::vector<std::string> condition_list;
	for (const WareIndex& temp_ware : group.first) {
		condition_list.push_back(tribes.get_ware_descr(temp_ware)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		/** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		/** TRANSLATORS:    %1$i = "3" */
		/** TRANSLATORS:    %2$s = "water" */
		condition = (boost::format(_("%1$ix %2$s"))
						 % static_cast<unsigned int>(group.second)
						 % condition).str();
	}

	/** TRANSLATORS: %s is a list of wares*/
	std::string result = (boost::format(_("the building doesn’t have the following wares: %s"))
								 % condition).str();
	return result;
}

bool ProductionProgram::ActReturn::WorkersNeedExperience::evaluate
	(const ProductionSite & ps) const
{
	ProductionSite::WorkingPosition const * const wp = ps.m_working_positions;
	for (uint32_t i = ps.descr().nr_working_positions(); i;)
		if (wp[--i].worker->needs_experience())
			return true;
	return false;
}
std::string ProductionProgram::ActReturn::WorkersNeedExperience::description
	(const Tribes&) const
{
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because a worker needs experience'. */
	return _("a worker needs experience");
}

std::string ProductionProgram::ActReturn::WorkersNeedExperience::description_negation
	(const Tribes&) const
{
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because the workers need no experience'. */
	return _("the workers need no experience");
}


ProductionProgram::ActReturn::Condition *
ProductionProgram::ActReturn::create_condition
	(std::vector<std::string>& parameters, const ProductionSiteDescr & descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for create_condition");
	}
	try {
		if (parameters.front() == "not") {
			parameters.erase(parameters.begin());
			return new ActReturn::Negation(parameters, descr, tribes);
		} else if (parameters.front() == "economy") {
			parameters.erase(parameters.begin());
			return create_economy_condition(parameters, tribes);
		} else if (parameters.front() == "site") {
			parameters.erase(parameters.begin());
			return create_site_condition(parameters, descr, tribes);
		} else if (parameters.front() == "workers") {
			parameters.erase(parameters.begin());
			return create_workers_condition(parameters);
		} else
			throw GameDataError
				("expected %s but found \"%s\"",
				 "{\"not\"|\"economy\"|\"workers\"}", parameters.front().c_str());
	} catch (const WException & e) {
		throw GameDataError("invalid condition: %s", e.what());
	}
}


ProductionProgram::ActReturn::ActReturn
	(std::vector<std::string>& parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActReturn");
	}
	try {
		if      (parameters.front() == "failed")    m_result = Failed;
		else if (parameters.front() == "completed") m_result = Completed;
		else if (parameters.front() == "skipped")   m_result = Skipped;
		else
			throw GameDataError
				("expected %s but found \"%s\"",
				"{\"failed\"|\"completed\"|\"skipped\"}", parameters.front().c_str());

		parameters.erase(parameters.begin());
		if (!parameters.empty()) {
			if (parameters.front() == "when") {
				parameters.erase(parameters.begin());
				m_is_when = true;
				while (!parameters.empty()) {
					m_conditions.push_back(create_condition(parameters, descr, tribes));
					parameters.erase(parameters.begin());
					if (!parameters.empty()) {
						if (parameters.front() != "and")
							throw GameDataError
								("expected \"%s\" or end of input", "and");
					} else {
						parameters.erase(parameters.begin());
					}
				}
			} else if (parameters.front() == "unless") {
				parameters.erase(parameters.begin());
				m_is_when = false;
				if (parameters.empty()) {
					throw GameDataError
						("expected condition at end of input");
				}
				while (!parameters.empty()) {
					m_conditions.push_back(create_condition(parameters, descr, tribes));
					parameters.erase(parameters.begin());
					if (!parameters.empty()) {
						if (parameters.front() != "or")
							throw GameDataError
								("expected \"%s\" or end of input", "or");
					} else {
						parameters.erase(parameters.begin());
					}
				}
			} else
				throw GameDataError
					("expected %s but found \"%s\"",
					 "{\"when\"|\"unless\"}", parameters.front().c_str());
		} else {
			m_is_when = true;
		}
	} catch (const WException & e) {
		throw GameDataError("return: %s", e.what());
	}
}

ProductionProgram::ActReturn::~ActReturn() {
	for (Condition * condition : m_conditions) {
		delete condition;
	}
}

void ProductionProgram::ActReturn::execute
	(Game & game, ProductionSite & ps) const
{
	if (!m_conditions.empty()) {
		std::vector<std::string> condition_list;
		if (m_is_when) { //  'when a and b and ...' (all conditions must be true)
			for (const Condition * condition : m_conditions) {
				if (!condition->evaluate(ps)) { //  A condition is false,
					return ps.program_step(game); //  continue program.
				}
				condition_list.push_back(condition->description(game.tribes()));
			}
		} else { //  "unless a or b or ..." (all conditions must be false)
			for (const Condition * condition : m_conditions) {
				if (condition->evaluate(ps)) { //  A condition is true,
					return ps.program_step(game); //  continue program.
				}
				condition_list.push_back(condition->description_negation(game.tribes()));
			}
		}
		std::string condition_string = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);

		std::string result_string = "";
		if (m_result == Failed) {
			/** TRANSLATORS: "Did not start working because the economy needs suits of armor" */
			result_string =  (boost::format(_("Did not start %1$s because %2$s"))
									% ps.top_state().program->descname()
									% condition_string)
								  .str();
		}
		else if (m_result == Completed) {
			/** TRANSLATORS: "Completed working because the economy needs suits of armor" */
			result_string =  (boost::format(_("Completed %1$s because %2$s"))
									% ps.top_state().program->descname()
									% condition_string)
								  .str();
		}
		else {
			/** TRANSLATORS: "Skipped working because the economy needs suits of armor" */
			result_string =  (boost::format(_("Skipped %1$s because %2$s"))
									% ps.top_state().program->descname()
									% condition_string)
								  .str();
		}
		ps.set_production_result(result_string);
	}
	return ps.program_end(game, m_result);
}


ProductionProgram::ActCall::ActCall
	(std::vector<std::string>& parameters, const ProductionSiteDescr & descr)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActCall");
	}
	//  Initialize with default handling methods.
	m_handling_methods[Failed    - 1] = Continue;
	m_handling_methods[Completed - 1] = Continue;
	m_handling_methods[Skipped   - 1] = Continue;

	try {
		{
			//char const * const program_name = next_word(parameters, reached_end);
			const ProductionSiteDescr::Programs & programs = descr.programs();
			ProductionSiteDescr::Programs::const_iterator const it =
				programs.find(parameters.front());
			if (it == programs.end())
				throw GameDataError
					(
					 "the program \"%s\" has not (yet) been declared in %s "
					 "(wrong declaration order?)",
					 parameters.front().c_str(), descr.descname().c_str());
			m_program = it->second;
		}

		//  Override with specified handling methods.
		while (!parameters.empty()) {
			parameters.erase(parameters.begin());
			if(parameters.front() == "on") {
				parameters.erase(parameters.begin());
				log("found \"on \": parameters = \"%s\"\n", parameters.front().c_str());
			}

			ProgramResult result_to_set_method_for;
			if (parameters.front() == "failure") {
				parameters.erase(parameters.begin());
				if (m_handling_methods[Failed    - 1] != Continue)
					throw GameDataError
						("%s handling method already defined", "failure");
				result_to_set_method_for = Failed;
			} else if (parameters.front() == "completion") {
				parameters.erase(parameters.begin());
				if (m_handling_methods[Completed - 1] != Continue)
					throw GameDataError
						("%s handling method already defined", "completion");
				result_to_set_method_for = Completed;
			} else if (parameters.front() == "skip") {
				parameters.erase(parameters.begin());
				if (m_handling_methods[Skipped   - 1] != Continue)
					throw GameDataError
						("%s handling method already defined", "skip");
				result_to_set_method_for = Skipped;
			} else
				throw GameDataError
					("expected %s but found \"%s\"",
					 "{\"failure\"|\"completion\"|\"skip\"}", parameters.front().c_str());

			ProgramResultHandlingMethod handling_method;
			if      (parameters.front() == "fail")
				handling_method = Fail;
			else if (parameters.front() == "complete")
				handling_method = Complete;
			else if (parameters.front() == "skip")
				handling_method = Skip;
			else if (parameters.front() == "repeat")
				handling_method = Repeat;
			else
				throw GameDataError
					("expected %s but found \"%s\"",
					 "{\"fail\"|\"complete\"|\"skip\"|\"repeat\"}",
					 parameters.front().c_str());
			m_handling_methods[result_to_set_method_for - 1] = handling_method;
			log
				("read handling method for result %u: %u, parameters = \"%s\", "
				 "reached_end = %u\n",
				 result_to_set_method_for, handling_method,
				 parameters.front().c_str(), parameters.empty());
		}
	} catch (const WException & e) {
		throw GameDataError("call: %s", e.what());
	}
}

void ProductionProgram::ActCall::execute
	(Game& game, ProductionSite& ps) const
{
	ProgramResult const program_result =
		static_cast<ProgramResult>(ps.top_state().phase);

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

ProductionProgram::ActWorker::ActWorker(
		std::vector<std::string>& parameters,
		const std::string& production_program_name,
		ProductionSiteDescr* descr,
		const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActWorker");
	}
	try {
		m_program = parameters.front();

		//  Quote form "void ProductionSite::program_act(Game &)":
		//  "Always main worker is doing stuff"
		const WorkerDescr & main_worker_descr =
			*tribes.get_worker_descr(descr->working_positions()[0].first);

		//  This will fail unless the main worker has a program with the given
		//  name, so it also validates the parameter.
		const WorkareaInfo & worker_workarea_info =
			main_worker_descr.get_program(m_program)->get_workarea_info();

		for (const std::pair<uint32_t, std::set<std::string> >& area_info : worker_workarea_info) {
			std::set<std::string> & building_radius_infos =
				descr->m_workarea_info[area_info.first];

			for (const std::string& worker_descname : area_info.second) {
				std::string description = descr->descname();
				description += ' ';
				description += production_program_name;
				description += " worker ";
				description += main_worker_descr.name();
				description += worker_descname;
				building_radius_infos.insert(description);
			}
		}
	} catch (const WException & e) {
		throw GameDataError("worker: %s", e.what());
	}
}

void ProductionProgram::ActWorker::execute
	(Game& game, ProductionSite& ps) const
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

ProductionProgram::ActSleep::ActSleep(std::vector<std::string>& parameters) {
	try {
		if (!parameters.empty()) {
			m_duration = get_unsigned_number<Duration>(parameters.front(), "duration in ms");
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (const WException & e) {
		throw GameDataError("sleep: %s", e.what());
	}
}

void ProductionProgram::ActSleep::execute(Game & game, ProductionSite & ps) const
{
	return
		ps.program_step(game, m_duration ? m_duration : ps.top_state().phase);
}


ProductionProgram::ActCheckMap::ActCheckMap(std::vector<std::string>& parameters)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActCheckMap");
	}
	try {
		if (parameters.front() == "seafaring")
			m_feature = SEAFARING;
		else
			throw GameDataError("Unknown parameter \"%s\"", parameters.front().c_str());
	} catch (const WException & e) {
		throw GameDataError("sleep: %s", e.what());
	}
}

void ProductionProgram::ActCheckMap::execute(Game& game, ProductionSite& ps) const
{
	switch (m_feature) {
		case SEAFARING: {
			if (game.map().get_port_spaces().size() > 1) // we need at least two port build spaces
				return ps.program_step(game, 0);
			else {
				ps.set_production_result("No use for ships on this map!");
				return ps.program_end(game, None);
			}
		}
		default:
			assert(false);
			break;
	}
}

ProductionProgram::ActAnimate::ActAnimate(
	std::vector<std::string>& parameters, ProductionSiteDescr* descr) {
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActAnimate");
	}
	try {
		const std::string animation_name = parameters.front();
		if (animation_name == "idle")
			throw GameDataError
				("idle animation is default; calling is not allowed");
		if (!descr->is_animation_known(animation_name)) {
			throw GameDataError("unknown animation \"%s\" in production program for building \"%s\"",
									  animation_name.c_str(), descr->name().c_str());
		}

		m_id = descr->get_animation(animation_name);

		parameters.erase(parameters.begin());
		if (!parameters.empty()) { //  The next parameter is the duration.
			m_duration = get_unsigned_number<Duration>(parameters.front(), "duration in ms");
		} else
			m_duration = 0; //  Get duration from the result of a previous action.
	} catch (const WException & e) {
		throw GameDataError("animate: %s", e.what());
	}
}

void ProductionProgram::ActAnimate::execute
	(Game& game, ProductionSite & ps) const
{
	ps.start_animation(game, m_id);
	return
		ps.program_step(game, m_duration ? m_duration : ps.top_state().phase);
}


ProductionProgram::ActConsume::ActConsume
	(std::vector<std::string>& parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActConsume");
	}
	try {
		while (!parameters.empty()) {
			m_groups.resize(m_groups.size() + 1);
			parse_ware_type_group
				(parameters, *m_groups.rbegin(), tribes, descr.inputs());
			parameters.erase(parameters.begin());
		}
		if (m_groups.empty())
			throw GameDataError
				("expected ware_type1[,ware_type2[,...]][:N] ...");
	} catch (const WException & e) {
		throw GameDataError("consume: %s", e.what());
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
		WareIndex const ware_type = warequeues[i]->get_ware();
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

	// "Did not start working because .... is/are missing"
	if (uint8_t const nr_missing_groups = l_groups.size()) {
		const TribeDescr & tribe = ps.owner().tribe();

		std::vector<std::string> group_list;
		for (const WareTypeGroup& group : l_groups) {
			assert(group.first.size());

			std::vector<std::string> ware_list;
			for (const WareIndex& ware : group.first) {
				ware_list.push_back(tribe.get_ware_descr(ware)->descname());
			}
			std::string ware_string = i18n::localize_list(ware_list, i18n::ConcatenateWith::OR);

			uint8_t const count = group.second;
			if (1 < count) {
				ware_string =
					/** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
					/** TRANSLATORS: For this example, this is what's in the place holders: */
					/** TRANSLATORS:    %1$i = "3" */
					/** TRANSLATORS:    %2$s = "water" */
					(boost::format(_("%1$ix %2$s"))
					 % static_cast<unsigned int>(count)
					 % ware_string)
					 .str();
			}
			group_list.push_back(ware_string);
		}

		const std::string is_missing_string =
				/** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
				/** TRANSLATORS: e.g. 'Did not start working because fish, meat or pitta bread is missing' */
				(boost::format(ngettext("%s is missing", "%s are missing", nr_missing_groups))
				 % i18n::localize_list(group_list, i18n::ConcatenateWith::AND))
				 .str();

		std::string result_string =
			/** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
			/** TRANSLATORS: For this example, this is what's in the place holders: */
			/** TRANSLATORS:    %1$s = "working" */
			/** TRANSLATORS:    %2$s = "3x water and 3x wheat are missing" */
			/** TRANSLATORS: This appears in the hover text on buildings. Please test these in context*/
			/** TRANSLATORS: on a development build if you can, and let us know if there are any issues */
			/** TRANSLATORS: we need to address for your language. */
			(boost::format(_("Did not start %1$s because %2$s"))
			 % ps.top_state().program->descname()
			 % is_missing_string).str();

		ps.set_production_result(result_string);
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
	(std::vector<std::string>& parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActProduce");
	}
	try {
		while (!parameters.empty()) {
			m_items.resize(m_items.size() + 1);
			std::pair<WareIndex, uint8_t> & item = *m_items.rbegin();
			// Get 1 set of ware parameters
			std::vector<std::string> ware_parameters;
			boost::split(ware_parameters, parameters.front(), boost::is_any_of(",:"));
			const std::string& warename = ware_parameters.front();
			item.first = tribes.safe_ware_index(warename);

			// More than one of this ware is wanted
			if (parameters.front().find(':') != std::string::npos) {
				item.second = get_unsigned_number<uint8_t>(parameters.front(), "number of wares", 1);
			} else {
				item.second = 1;
			}
			if (!descr.is_output_ware_type(item.first)) {
				throw GameDataError("the ware \"%s\" is not listed with the output for this building", warename.c_str());
			}
		}
	} catch (const WException & e) {
		throw GameDataError("produce: %s", e.what());
	}
}

void ProductionProgram::ActProduce::execute
	(Game & game, ProductionSite & ps) const
{
	//ps.molog("  Produce\n");
	assert(ps.m_produced_wares.empty());
	ps.m_produced_wares = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	const TribeDescr & tribe = ps.owner().tribe();
	assert(m_items.size());

	std::vector<std::string> ware_descnames;
	for (const auto& item_pair : m_items) {
		uint8_t const count = item_pair.second;
		std::string ware_descname = tribe.get_ware_descr(item_pair.first)->descname();
		// TODO(GunChleoc): needs ngettext when we have one_tribe.
		if (1 < count) {
			/** TRANSLATORS: This is an item in a list of wares, e.g. "Produced 2x Coal": */
			/** TRANSLATORS:    %%1$i = "2" */
			/** TRANSLATORS:    %2$s = "Coal" */
			ware_descname = (boost::format(_("%1$ix %2$s"))
								  % static_cast<unsigned int>(count)
								  % ware_descname).str();
		}
		ware_descnames.push_back(ware_descname);
	}
	std::string ware_list = i18n::localize_list(ware_descnames, i18n::ConcatenateWith::AND);

	/** TRANSLATORS: %s is a list of wares */
	const std::string result_string = (boost::format(_("Produced %s")) % ware_list).str();
	ps.set_production_result(result_string);
}

bool ProductionProgram::ActProduce::get_building_work
	(Game & game, ProductionSite & psite, Worker & /* worker */) const
{
	// We reach this point once all wares have been carried outside the building
	psite.program_step(game);
	return false;
}


ProductionProgram::ActRecruit::ActRecruit
	(std::vector<std::string>& parameters, const ProductionSiteDescr& descr, const Tribes& tribes)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActRecruit");
	}
	try {
		while (!parameters.empty()) {
			m_items.resize(m_items.size() + 1);
			std::pair<WareIndex, uint8_t> & item = *m_items.rbegin();
			// Get 1 set of worker parameters
			std::vector<std::string> worker_parameters;
			boost::split(worker_parameters, parameters.front(), boost::is_any_of(",:"));
			const std::string& workername = worker_parameters.front();
			item.first = tribes.safe_worker_index(workername);

			// More than one of this worker is wanted
			if (parameters.front().find(':') != std::string::npos) {
				item.second = get_unsigned_number<uint8_t>(parameters.front(), "number of wares", 1);
			} else {
				item.second = 1;
			}
			if (!descr.is_output_worker_type(item.first)) {
				throw GameDataError("the worker \"%s\" is not listed with the output for this building", workername.c_str());
			}
		}
	} catch (const WException & e) {
		throw GameDataError("recruit: %s", e.what());
	}
}

void ProductionProgram::ActRecruit::execute
	(Game & game, ProductionSite & ps) const
{
	assert(ps.m_recruited_workers.empty());
	ps.m_recruited_workers = m_items;
	ps.m_working_positions[0].worker->update_task_buildingwork(game);

	const TribeDescr & tribe = ps.owner().tribe();
	assert(m_items.size());
	std::vector<std::string> worker_descnames;
	for (const auto& item_pair : m_items) {
		uint8_t const count = item_pair.second;
		std::string worker_descname = tribe.get_worker_descr(item_pair.first)->descname();
		// TODO(GunChleoc): needs ngettext when we have one_tribe.
		if (1 < count) {
			/** TRANSLATORS: This is an item in a list of workers, e.g. "Recruited 2x Ox": */
			/** TRANSLATORS:    %1$i = "2" */
			/** TRANSLATORS:    %2$s = "Ox" */
			worker_descname = (boost::format(_("%1$ix %2$s"))
									 % static_cast<unsigned int>(count)
									 % worker_descname).str();
		}
		worker_descnames.push_back(worker_descname);
	}
	std::string unit_string = i18n::localize_list(worker_descnames, i18n::ConcatenateWith::AND);

	/** TRANSLATORS: %s is a list of workers */
	const std::string result_string = (boost::format(_("Recruited %s")) % unit_string).str();
	ps.set_production_result(result_string);
}

bool ProductionProgram::ActRecruit::get_building_work
	(Game & game, ProductionSite & psite, Worker & /* worker */) const
{
	// We reach this point once all recruits have been guided outside the building
	psite.program_step(game);
	return false;
}

ProductionProgram::ActMine::ActMine(
		std::vector<std::string>& parameters, const World& world, const std::string& production_program_name,
		ProductionSiteDescr* descr)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActMine");
	}
	try {
		m_resource = world.safe_resource_index(parameters.front().c_str());
		parameters.erase(parameters.begin());

		m_distance = get_unsigned_number<uint8_t>(parameters.front(), "distance");
		parameters.erase(parameters.begin());

		m_max = get_unsigned_number<uint8_t>(parameters.front(), "percentage", 1, 100);
		parameters.erase(parameters.begin());

		m_chance = get_unsigned_number<uint8_t>(parameters.front(), "percentage", 1, 100);
		parameters.erase(parameters.begin());

		m_training = get_unsigned_number<uint8_t>(parameters.front(), "percentage", 1, 100);
		parameters.erase(parameters.begin());

		std::string description =
			/** TRANSLATORS: %1$s = production site name, %2$s = production program name, %3$s = resource */
			(boost::format(_("%1$s %2$s mine %3$s")) % descr->descname() % production_program_name
				% world.get_resource(m_resource)->descname())
				.str();

		descr->m_workarea_info[m_distance].insert(description);
	} catch (const WException & e) {
		throw GameDataError("mine: %s", e.what());
	}
}

void ProductionProgram::ActMine::execute
	(Game & game, ProductionSite & ps) const
{
	Map & map = game.map();

	//  select one of the nodes randomly
	uint32_t totalres    = 0;
	uint32_t totalchance = 0;
	uint32_t totalstart  = 0;

	{
		MapRegion<Area<FCoords> > mr
			(map, Area<FCoords> (map.get_fcoords(ps.get_position()), m_distance));
		do {
			uint8_t  fres   = mr.location().field->get_resources();
			uint32_t amount = mr.location().field->get_resources_amount();
			uint32_t start_amount =
				mr.location().field->get_initial_res_amount();

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
	if (!totalres)
		digged_percentage = 100;

	if (digged_percentage < m_max) {
		//  mine can produce normally
		if (totalres == 0)
			return ps.program_end(game, Failed);

		//  second pass through nodes
		assert(totalchance);
		int32_t pick = game.logic_rand() % totalchance;

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

		if (pick >= 0) {
			return ps.program_end(game, Failed);
		}

	} else {
		//  Inform the player about an empty mine, unless
		//  there is a sufficiently high chance, that the mine
		//  will still produce enough.
		//  e.g. mines have m_chance=5, wells have 65
		if (m_chance <= 20) {
				ps.notify_player(game, 60);
			// and change the default animation
			ps.set_default_anim("empty");
		}

		//  Mine has reached its limits, still try to produce something but
		//  independent of sourrunding resources. Do not decrease resources
		//  further.
		if (m_chance <= game.logic_rand() % 100) {

			// Gain experience
			if (m_training >= game.logic_rand() % 100) {
			  ps.train_workers(game);
			}
			return ps.program_end(game, Failed);
		}
	}

	//  done successful
	//  TODO(unknown): Should pass the time it takes to mine in the phase parameter of
	//  ProductionSite::program_step so that the following sleep/animate
	//  command knows how long it should last.
	return ps.program_step(game);
}

ProductionProgram::ActCheckSoldier::ActCheckSoldier(std::vector<std::string>& parameters) {
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActCheckSoldier");
	}
	//  TODO(unknown): This is currently hardcoded for "soldier", but should allow any
	//  soldier type name.
	if (parameters.front() != "soldier") {
		throw GameDataError
			("expected %s but found \"%s\"", "soldier type", parameters.front().c_str());
	}
	parameters.erase(parameters.begin());
	try {
		if (parameters.front() == "hp") {
			parameters.erase(parameters.begin());
			attribute = atrHP;
		} else if (parameters.front() == "attack") {
			parameters.erase(parameters.begin());
			attribute = atrAttack;
		} else if (parameters.front() == "defense") {
			parameters.erase(parameters.begin());
			attribute = atrDefense;
		} else if (parameters.front() == "evade") {
			parameters.erase(parameters.begin());
			attribute = atrEvade;
		} else
			throw GameDataError
				("expected %s but found \"%s\"",
					"{\"hp\"|\"attack\"|\"defense\"|\"evade\"}", parameters.front().c_str());

		level = get_unsigned_number<uint8_t>(parameters.front(), "level");

	} catch (const WException & e) {
		throw GameDataError("check_soldier: %s", e.what());
	}
}

void ProductionProgram::ActCheckSoldier::execute
	(Game & game, ProductionSite & ps) const
{
	SoldierControl & ctrl = dynamic_cast<SoldierControl &>(ps);
	const std::vector<Soldier *> soldiers = ctrl.present_soldiers();
	if (soldiers.empty()) {
		ps.set_production_result(_("No soldier to train!"));
		return ps.program_end(game, Skipped);
	}
	ps.molog("  Checking soldier (%u) level %d)\n", attribute, level);

	const std::vector<Soldier *>::const_iterator soldiers_end = soldiers.end();
	for (std::vector<Soldier *>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
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
	ts->training_attempted(attribute, level);

	ps.molog("  Check done!\n");

	return ps.program_step(game);
}

ProductionProgram::ActTrain::ActTrain(std::vector<std::string>& parameters) {
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActTrain");
	}
	//  TODO(unknown): This is currently hardcoded for "soldier", but should allow any
	//  soldier type name.
	if (parameters.front() != "soldier")
		throw GameDataError
			("expected %s but found \"%s\"", "soldier type", parameters.front().c_str());

	parameters.erase(parameters.begin());
	try {
		if (parameters.front() == "hp") {
			parameters.erase(parameters.begin());
			attribute = atrHP;
		} else if (parameters.front() == "attack") {
			parameters.erase(parameters.begin());
			attribute = atrAttack;
		} else if (parameters.front() == "defense") {
			parameters.erase(parameters.begin());
			attribute = atrDefense;
		} else if (parameters.front() == "evade") {
			parameters.erase(parameters.begin());
			attribute = atrEvade;
		} else
			throw GameDataError
				("expected %s but found \"%s\"",
				 "{\"hp\"|\"attack\"|\"defense\"|\"evade\"}", parameters.front().c_str());

		level = get_unsigned_number<uint8_t>(parameters.front(), "level");
		target_level = get_unsigned_number<uint8_t>(parameters.front(), "target level", 0, level);
	} catch (const WException & e) {
		throw GameDataError("train: %s", e.what());
	}
}

void ProductionProgram::ActTrain::execute
	(Game & game, ProductionSite & ps) const
{
	SoldierControl & ctrl = dynamic_cast<SoldierControl &>(ps);
	const std::vector<Soldier *> soldiers = ctrl.present_soldiers();
	const std::vector<Soldier *>::const_iterator soldiers_end =
		soldiers.end();
	std::vector<Soldier *>::const_iterator it = soldiers.begin();

	ps.molog
		("  Training soldier's %u (%d to %d)",
		 attribute, level, target_level);

	for (;; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
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
	ts->training_successful(attribute, level);


	return ps.program_step(game);
}

ProductionProgram::ActPlayFX::ActPlayFX
	(std::vector<std::string>& parameters)
{
	if (parameters.empty()) {
		throw GameDataError("Empty parameters for ActPlayFX");
	}
	try {
		name = parameters.front();
		parameters.erase(parameters.begin());

		if (!parameters.empty()) {
			priority = get_unsigned_number<uint8_t>(parameters.front(), "priority");
		} else {
			priority = 127;
		}

		// NOCOM(GunChleoc): Check if this works
		g_sound_handler.load_fx_if_needed(FileSystem::fs_dirname(name),
													 FileSystem::fs_filename(name.c_str()),
													 name);
	} catch (const WException & e) {
		throw GameDataError("playFX: %s", e.what());
	}
}

void ProductionProgram::ActPlayFX::execute
	(Game & game, ProductionSite & ps) const
{
	g_sound_handler.play_fx(name, ps.m_position, priority);
	return ps.program_step(game);
}

ProductionProgram::ActConstruct::ActConstruct(
		std::vector<std::string>& parameters, const std::string& production_program_name, ProductionSiteDescr* descr) {
	if (parameters.size() != 3)
		throw GameDataError("usage: construct object-name worker-program radius:NN");
	try {
		objectname = parameters[0];
		workerprogram = parameters[1];
		radius = boost::lexical_cast<uint32_t>(parameters[2]);

		std::set<std::string> & building_radius_infos = descr->m_workarea_info[radius];
		std::string description = descr->name() + ' ' + production_program_name;
		description += " construct ";
		description += objectname;
		building_radius_infos.insert(description);
	} catch (const WException & e) {
		throw GameDataError("construct: %s", e.what());
	}
}

const ImmovableDescr & ProductionProgram::ActConstruct::get_construction_descr
	(const Tribes& tribes) const
{
	const ImmovableDescr * descr = tribes.get_immovable_descr(tribes.immovable_index(objectname));
	if (!descr)
		throw wexception("ActConstruct: immovable '%s' does not exist", objectname.c_str());

	return *descr;
}


void ProductionProgram::ActConstruct::execute(Game & game, ProductionSite & psite) const
{
	ProductionSite::State & state = psite.top_state();
	const ImmovableDescr & descr = get_construction_descr(game.tribes());

	// Early check for no resources
	const Buildcost & buildcost = descr.buildcost();
	WareIndex available_resource = INVALID_INDEX;

	for (Buildcost::const_iterator it = buildcost.begin(); it != buildcost.end(); ++it) {
		if (psite.waresqueue(it->first).get_filled() > 0) {
			available_resource = it->first;
			break;
		}
	}

	if (available_resource == INVALID_INDEX) {
		psite.program_end(game, Failed);
		return;
	}

	// Look for an appropriate object in the given radius
	std::vector<ImmovableFound> immovables;
	CheckStepWalkOn cstep(MOVECAPS_WALK, true);
	Area<FCoords> area (game.map().get_fcoords(psite.base_flag().get_position()), radius);
	if
		(game.map().find_reachable_immovables
		 (area, &immovables, cstep, FindImmovableByDescr(descr)))
	{
		state.objvar = immovables[0].object;

		psite.m_working_positions[0].worker->update_task_buildingwork(game);
		return;
	}

	// No object found, look for a field where we can build
	std::vector<Coords> fields;
	FindNodeAnd fna;
	fna.add(FindNodeShore());
	fna.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone));
	if
		(game.map().find_reachable_fields
			(area, &fields, cstep, fna))
	{
		state.coord = fields[0];

		psite.m_working_positions[0].worker->update_task_buildingwork(game);
		return;
	}

	psite.molog("construct: no object or buildable field\n");
	psite.program_end(game, Failed);
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
		const ImmovableDescr & descr = get_construction_descr(game.tribes());
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
	WareInstance* ware =
		new WareInstance(wq->get_ware(), game.tribes().get_ware_descr(wq->get_ware()));
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


ProductionProgram::ProductionProgram(const std::string& _name,
		const std::string& _descname,
		std::unique_ptr<LuaTable> actions_table,
		const EditorGameBase& egbase,
		ProductionSiteDescr* building)
	: m_name(_name), m_descname(_descname) {

	for (const std::string& action_string : actions_table->keys<std::string>()) {
		std::vector<std::string> parts;
		boost::split(parts, action_string, boost::is_any_of("="));
		if (parts.size() != 2) {
			throw GameDataError("invalid line: \"%s\" in production program \"%s\" for building \"%s\"",
									  action_string.c_str(), _name.c_str(), building->name().c_str());
		}

		std::vector<std::string> parameters;
		boost::split(parameters, parts[1], boost::is_any_of(" "));

		ProductionProgram::Action* action;

		if (boost::iequals(parts[0], "return"))
			action = new ActReturn(parameters, *building, egbase.tribes());
		else if (boost::iequals(parts[0], "call"))
			action = new ActCall(parameters, *building);
		else if (boost::iequals(parts[0], "sleep"))
			action = new ActSleep(parameters);
		else if (boost::iequals(parts[0], "animate"))
		action = new ActAnimate(parameters, building);
		else if (boost::iequals(parts[0], "consume"))
			action = new ActConsume(parameters, *building, egbase.tribes());
		else if (boost::iequals(parts[0], "produce"))
			action = new ActProduce(parameters, *building, egbase.tribes());
		else if (boost::iequals(parts[0], "recruit"))
			action = new ActRecruit(parameters, *building, egbase.tribes());
		else if (boost::iequals(parts[0], "worker"))
			action = new ActWorker(parameters, _name, building, egbase.tribes());
		else if (boost::iequals(parts[0], "mine"))
			action = new ActMine(parameters, egbase.world(), _name, building);
		else if (boost::iequals(parts[0], "check_soldier"))
			action = new ActCheckSoldier(parameters);
		else if (boost::iequals(parts[0], "train"))
			action = new ActTrain(parameters);
		else if (boost::iequals(parts[0], "playFX"))
			action = new ActPlayFX(parameters);
		else if (boost::iequals(parts[0], "construct"))
			action = new ActConstruct(parameters, _name, building);
		else if (boost::iequals(parts[0], "check_map"))
			action = new ActCheckMap(parameters);
		else
			throw GameDataError("unknown command type \"%s\" in production program \"%s\" for building \"%s\"",
									  parts[1].c_str(), _name.c_str(), building->name().c_str());
		m_actions.push_back(action);
	}
	if (m_actions.empty())
		throw GameDataError("no actions in production program \"%s\" for building \"%s\"",
								  _name.c_str(), building->name().c_str());
}

}
