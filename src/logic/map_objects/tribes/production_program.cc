/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "logic/map_objects/tribes/production_program.h"

#include <memory>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "config.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/input_queue.h"
#include "economy/wares_queue.h"
#include "helper.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker_program.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

namespace Widelands {

namespace {

/// Matches the string that candidate points to against the string that
/// template points to. Stops at when reaching a null character or the
/// character terminator. If a match is found, candidate is moved beyond the
/// matched part.
///
/// example:
///    char const * candidate = "return   75";
///    bool const result = match(candidate, "return");
/// now candidate points to "   75" and result is true
bool match(char*& candidate, const char* pattern) {
	for (char *p = candidate;; ++p, ++pattern)
		if (!*pattern) {
			candidate = p;
			return true;
		} else if (*p != *pattern)
			break;
	return false;
}

/// Skips a sequence of consecutive characters with the value c, starting at p.
/// Throws WException if no characters were skipped.
void force_skip(char*& p, char const c = ' ') {
	char* t = p;
	while (*t == c)
		++t;
	if (p < t)
		p = t;
	else
		throw wexception("expected '%c' but found \"%s\"", c, p);
}

/// Skips a sequence of consecutive characters with the value c, starting at p.
/// Returns whether any characters were skipped.
bool skip(char*& p, char const c = ' ') {
	char* t = p;
	while (*t == c)
		++t;
	if (p < t) {
		p = t;
		return true;
	} else
		return false;
}

/// Combines match and force_skip.
///
/// example:
///    char const * candidate = "return   75";
///    bool const result = match_force_skip(candidate, "return");
/// now candidate points to "75" and result is true
///
/// example:
///   char const * candidate = "return75";
///    bool const result = match_force_skip(candidate, "return");
/// throws WException
bool match_force_skip(char*& candidate, const char* pattern) {
	for (char *p = candidate;; ++p, ++pattern)
		if (!*pattern) {
			force_skip(p);
			candidate = p;
			return true;
		} else if (*p != *pattern)
			return false;

	NEVER_HERE();
}

ProductionProgram::ActReturn::Condition* create_economy_condition(char*& parameters,
                                                                  const Tribes& tribes) {
	try {
		if (match_force_skip(parameters, "needs"))
			try {
				bool reached_end;
				char const* const type_name = next_word(parameters, reached_end);
				const DescriptionIndex& wareindex = tribes.ware_index(type_name);
				if (tribes.ware_exists(wareindex)) {
					for (size_t i = 0; i < tribes.nrtribes(); ++i) {
						const TribeDescr& tribe_descr = *tribes.get_tribe_descr(i);
						if (tribe_descr.has_ware(wareindex)) {
							tribes.set_ware_type_has_demand_check(wareindex, tribe_descr.name());
						}
					}
					return new ProductionProgram::ActReturn::EconomyNeedsWare(wareindex);
				} else if (tribes.worker_exists(tribes.worker_index(type_name))) {
					const DescriptionIndex& workerindex = tribes.worker_index(type_name);
					for (size_t i = 0; i < tribes.nrtribes(); ++i) {
						const TribeDescr* tribe_descr = tribes.get_tribe_descr(i);
						if (tribe_descr->has_worker(workerindex)) {
							tribes.set_worker_type_has_demand_check(workerindex);
						}
					}
					return new ProductionProgram::ActReturn::EconomyNeedsWorker(workerindex);
				} else
					throw GameDataError(
					   "expected %s but found \"%s\"", "ware type or worker type", type_name);
			} catch (const WException& e) {
				throw GameDataError("needs: %s", e.what());
			}
		else
			throw GameDataError("expected %s but found \"%s\"", "\"needs\"", parameters);
	} catch (const WException& e) {
		throw GameDataError("economy: %s", e.what());
	}
}

ProductionProgram::ActReturn::Condition*
create_site_condition(char*& parameters, const ProductionSiteDescr& descr, const Tribes& tribes) {
	try {
		if (match_force_skip(parameters, "has"))
			return new ProductionProgram::ActReturn::SiteHas(parameters, descr, tribes);
		else
			throw GameDataError("expected %s but found \"%s\"", "\"has\"", parameters);
	} catch (const WException& e) {
		throw GameDataError("site: %s", e.what());
	}
}

ProductionProgram::ActReturn::Condition* create_workers_condition(char*& parameters) {
	try {
		if (match(parameters, "need experience"))
			return new ProductionProgram::ActReturn::WorkersNeedExperience;
		else
			throw GameDataError("expected %s but found \"%s\"", "\"need experience\"", parameters);
	} catch (const WException& e) {
		throw GameDataError("workers: %s", e.what());
	}
}

}  // namespace

ProductionProgram::Action::~Action() {
}

bool ProductionProgram::Action::get_building_work(Game&, ProductionSite&, Worker&) const {
	return false;
}

void ProductionProgram::Action::building_work_failed(Game&, ProductionSite&, Worker&) const {
}

void ProductionProgram::parse_ware_type_group(char*& parameters,
                                              WareTypeGroup& group,
                                              const Tribes& tribes,
                                              const BillOfMaterials& input_wares,
                                              const BillOfMaterials& input_workers) {
	std::set<std::pair<DescriptionIndex, WareWorker>>::iterator last_insert_pos = group.first.end();
	uint8_t count = 1;
	uint8_t count_max = 0;
	for (;;) {
		char const* ware = parameters;
		while (*parameters && *parameters != ',' && *parameters != ':' && *parameters != ' ')
			++parameters;
		char const terminator = *parameters;
		*parameters = '\0';

		// Try as ware
		WareWorker type = wwWARE;
		const BillOfMaterials* input_list = &input_wares;
		DescriptionIndex ware_index = tribes.ware_index(ware);
		if (!tribes.ware_exists(ware_index)) {
			ware_index = tribes.worker_index(ware);
			if (tribes.worker_exists(ware_index)) {
				// It is a worker
				type = wwWORKER;
				input_list = &input_workers;
			} else {
				throw GameDataError("Unknown ware or worker type \"%s\"", ware);
			}
		}

		bool found = false;
		for (const WareAmount& input : *input_list) {
			if (input.first == ware_index) {
				count_max += input.second;
				found = true;
				break;
			}
		}
		if (!found) {
			throw GameDataError("%s is not declared as an input (\"%s=<count>\" was not "
			                    "found in the [inputs] section)",
			                    ware, ware);
		}

		if (group.first.size() && ware_index <= group.first.begin()->first)
			throw GameDataError("wrong order of ware types within group: ware type %s appears "
			                    "after ware type %s (fix order!)",
			                    ware,
			                    tribes.get_ware_descr(group.first.begin()->first)->name().c_str());
		last_insert_pos = group.first.insert(last_insert_pos, std::make_pair(ware_index, type));
		*parameters = terminator;
		switch (terminator) {
		case ':': {
			++parameters;
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			count = value;
			if ((*endp && *endp != ' ') || value < 1 || count != value)
				throw GameDataError("expected %s but found \"%s\"", "count", parameters);
			parameters = endp;
			if (count_max < count)
				throw GameDataError("group count is %u but (total) input storage capacity of "
				                    "the specified ware type(s) is only %u, so the group can "
				                    "never be fulfilled by the site",
				                    count, count_max);
		}
			FALLS_THROUGH;
		case '\0':
		case ' ':
			group.second = count;
			return;
		case ',':
			++parameters;
			break;
		default:
			// scan for terminator should ensure that this cannot happen
			NEVER_HERE();
		}
	}
}

ProductionProgram::ActReturn::Condition::~Condition() {
}

ProductionProgram::ActReturn::Negation::~Negation() {
	delete operand;
}
bool ProductionProgram::ActReturn::Negation::evaluate(const ProductionSite& ps) const {
	return !operand->evaluate(ps);
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string ProductionProgram::ActReturn::Negation::description(const Tribes& t) const {
	return operand->description_negation(t);
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string ProductionProgram::ActReturn::Negation::description_negation(const Tribes& t) const {
	return operand->description(t);
}

bool ProductionProgram::ActReturn::EconomyNeedsWare::evaluate(const ProductionSite& ps) const {
	return ps.get_economy(wwWARE)->needs_ware_or_worker(ware_type);
}
std::string
ProductionProgram::ActReturn::EconomyNeedsWare::description(const Tribes& tribes) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy needs the ware
	 * '%s' */
	std::string result = (boost::format(_("the economy needs the ware ‘%s’")) %
	                      tribes.get_ware_descr(ware_type)->descname())
	                        .str();
	return result;
}
std::string
ProductionProgram::ActReturn::EconomyNeedsWare::description_negation(const Tribes& tribes) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy doesn't need the
	 * ware '%s' */
	std::string result = (boost::format(_("the economy doesn’t need the ware ‘%s’")) %
	                      tribes.get_ware_descr(ware_type)->descname())
	                        .str();
	return result;
}

bool ProductionProgram::ActReturn::EconomyNeedsWorker::evaluate(const ProductionSite& ps) const {
	return ps.get_economy(wwWORKER)->needs_ware_or_worker(worker_type);
}
std::string
ProductionProgram::ActReturn::EconomyNeedsWorker::description(const Tribes& tribes) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy needs the worker
	 * '%s' */
	std::string result = (boost::format(_("the economy needs the worker ‘%s’")) %
	                      tribes.get_worker_descr(worker_type)->descname())
	                        .str();
	return result;
}

std::string
ProductionProgram::ActReturn::EconomyNeedsWorker::description_negation(const Tribes& tribes) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... */
	/** TRANSLATORS:      ... because the economy doesn’t need the worker '%s' */
	std::string result = (boost::format(_("the economy doesn’t need the worker ‘%s’")) %
	                      tribes.get_worker_descr(worker_type)->descname())
	                        .str();
	return result;
}

ProductionProgram::ActReturn::SiteHas::SiteHas(char*& parameters,
                                               const ProductionSiteDescr& descr,
                                               const Tribes& tribes) {
	try {
		parse_ware_type_group(parameters, group, tribes, descr.input_wares(), descr.input_workers());
	} catch (const WException& e) {
		throw GameDataError("has ware_type1[,ware_type2[,...]][:N]: %s", e.what());
	}
}
bool ProductionProgram::ActReturn::SiteHas::evaluate(const ProductionSite& ps) const {
	uint8_t count = group.second;
	for (InputQueue* ip_queue : ps.inputqueues()) {
		for (const auto& input_type : group.first) {
			if (input_type.first == ip_queue->get_index() &&
			    input_type.second == ip_queue->get_type()) {
				uint8_t const filled = ip_queue->get_filled();
				if (count <= filled)
					return true;
				count -= filled;
				break;
			}
		}
	}
	return false;
}

std::string ProductionProgram::ActReturn::SiteHas::description(const Tribes& tribes) const {
	std::vector<std::string> condition_list;
	for (const auto& entry : group.first) {
		condition_list.push_back(entry.second == wwWARE ?
		                            tribes.get_ware_descr(entry.first)->descname() :
		                            tribes.get_worker_descr(entry.first)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		condition =
		   /** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		   /** TRANSLATORS:    %1$i = "3" */
		   /** TRANSLATORS:    %2$s = "water" */
		   (boost::format(_("%1$ix %2$s")) % static_cast<unsigned int>(group.second) % condition)
		      .str();
	}

	std::string result =
	   /** TRANSLATORS: %s is a list of wares*/
	   (boost::format(_("the building has the following wares: %s")) % condition).str();
	return result;
}

std::string
ProductionProgram::ActReturn::SiteHas::description_negation(const Tribes& tribes) const {
	std::vector<std::string> condition_list;
	for (const auto& entry : group.first) {
		condition_list.push_back(entry.second == wwWARE ?
		                            tribes.get_ware_descr(entry.first)->descname() :
		                            tribes.get_worker_descr(entry.first)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		condition =
		   /** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		   /** TRANSLATORS:    %1$i = "3" */
		   /** TRANSLATORS:    %2$s = "water" */
		   (boost::format(_("%1$ix %2$s")) % static_cast<unsigned int>(group.second) % condition)
		      .str();
	}

	std::string result =
	   /** TRANSLATORS: %s is a list of wares*/
	   (boost::format(_("the building doesn’t have the following wares: %s")) % condition).str();
	return result;
}

bool ProductionProgram::ActReturn::WorkersNeedExperience::evaluate(const ProductionSite& ps) const {
	ProductionSite::WorkingPosition const* const wp = ps.working_positions_;
	for (uint32_t i = ps.descr().nr_working_positions(); i;)
		if (wp[--i].worker->needs_experience())
			return true;
	return false;
}
std::string ProductionProgram::ActReturn::WorkersNeedExperience::description(const Tribes&) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because a worker needs experience'. */
	return _("a worker needs experience");
}

std::string
ProductionProgram::ActReturn::WorkersNeedExperience::description_negation(const Tribes&) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because the workers need no experience'. */
	return _("the workers need no experience");
}

ProductionProgram::ActReturn::Condition* ProductionProgram::ActReturn::create_condition(
   char*& parameters, const ProductionSiteDescr& descr, const Tribes& tribes) {
	try {
		if (match_force_skip(parameters, "not"))
			return new ActReturn::Negation(parameters, descr, tribes);
		else if (match_force_skip(parameters, "economy"))
			return create_economy_condition(parameters, tribes);
		else if (match_force_skip(parameters, "site"))
			return create_site_condition(parameters, descr, tribes);
		else if (match_force_skip(parameters, "workers"))
			return create_workers_condition(parameters);
		else
			throw GameDataError(
			   "expected %s but found \"%s\"", "{\"not\"|\"economy\"|\"workers\"}", parameters);
	} catch (const WException& e) {
		throw GameDataError("invalid condition: %s", e.what());
	}
}

ProductionProgram::ActReturn::ActReturn(char* parameters,
                                        const ProductionSiteDescr& descr,
                                        const Tribes& tribes) {
	try {
		if (match(parameters, "failed"))
			result_ = ProgramResult::kFailed;
		else if (match(parameters, "completed"))
			result_ = ProgramResult::kCompleted;
		else if (match(parameters, "skipped"))
			result_ = ProgramResult::kSkipped;
		else if (match(parameters, "no_stats"))
			result_ = ProgramResult::kNone;
		else
			throw GameDataError("expected %s but found \"%s\"",
			                    "{\"failed\"|\"completed\"|\"skipped\"|\"no_stats\"}", parameters);

		if (skip(parameters)) {
			if (match_force_skip(parameters, "when")) {
				is_when_ = true;
				for (;;) {
					conditions_.push_back(create_condition(parameters, descr, tribes));
					if (*parameters) {
						skip(parameters);
						if (!match_force_skip(parameters, "and"))
							throw GameDataError("expected \"%s\" or end of input", "and");
					} else
						break;
				}
			} else if (match_force_skip(parameters, "unless")) {
				is_when_ = false;
				for (;;) {
					if (!*parameters)
						throw GameDataError("expected condition at end of input");
					conditions_.push_back(create_condition(parameters, descr, tribes));
					if (*parameters) {
						skip(parameters);
						if (!match_force_skip(parameters, "or"))
							throw GameDataError("expected \"%s\" or end of input", "or");
					} else
						break;
				}
			} else
				throw GameDataError(
				   "expected %s but found \"%s\"", "{\"when\"|\"unless\"}", parameters);
		} else if (*parameters)
			throw GameDataError("expected %s but found \"%s\"", ("space or end of input"), parameters);
		else
			is_when_ = true;

	} catch (const WException& e) {
		throw GameDataError("return: %s", e.what());
	}
}

ProductionProgram::ActReturn::~ActReturn() {
	for (Condition* condition : conditions_) {
		delete condition;
	}
}

void ProductionProgram::ActReturn::execute(Game& game, ProductionSite& ps) const {
	if (!conditions_.empty()) {
		std::vector<std::string> condition_list;
		if (is_when_) {  //  'when a and b and ...' (all conditions must be true)
			for (const Condition* condition : conditions_) {
				if (!condition->evaluate(ps)) {   //  A condition is false,
					return ps.program_step(game);  //  continue program.
				}
				condition_list.push_back(condition->description(game.tribes()));
			}
		} else {  //  "unless a or b or ..." (all conditions must be false)
			for (const Condition* condition : conditions_) {
				if (condition->evaluate(ps)) {    //  A condition is true,
					return ps.program_step(game);  //  continue program.
				}
				condition_list.push_back(condition->description_negation(game.tribes()));
			}
		}
		std::string condition_string =
		   i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);

		std::string result_string;
		switch (result_) {
		case ProgramResult::kFailed: {
			/** TRANSLATORS: "Did not start working because the economy needs the ware '%s'" */
			result_string = (boost::format(_("Did not start %1$s because %2$s")) %
			                 ps.top_state().program->descname() % condition_string)
			                   .str();
		} break;
		case ProgramResult::kCompleted: {
			/** TRANSLATORS: "Completed working because the economy needs the ware '%s'" */
			result_string = (boost::format(_("Completed %1$s because %2$s")) %
			                 ps.top_state().program->descname() % condition_string)
			                   .str();
		} break;
		case ProgramResult::kSkipped: {
			/** TRANSLATORS: "Skipped working because the economy needs the ware '%s'" */
			result_string = (boost::format(_("Skipped %1$s because %2$s")) %
			                 ps.top_state().program->descname() % condition_string)
			                   .str();
		} break;
		case ProgramResult::kNone: {
			// TODO(GunChleoc): Same as skipped - is this on purpose?
			result_string = (boost::format(_("Skipped %1$s because %2$s")) %
			                 ps.top_state().program->descname() % condition_string)
			                   .str();
		}
		}
		if (ps.production_result() != ps.descr().out_of_resource_heading() ||
		    ps.descr().out_of_resource_heading().empty()) {
			ps.set_production_result(result_string);
		}
	}
	return ps.program_end(game, result_);
}

ProductionProgram::ActCall::ActCall(char* parameters, const ProductionSiteDescr& descr) {
	//  Initialize with default handling methods.
	handling_methods_[program_result_index(ProgramResult::kFailed)] =
	   ProgramResultHandlingMethod::kContinue;
	handling_methods_[program_result_index(ProgramResult::kCompleted)] =
	   ProgramResultHandlingMethod::kContinue;
	handling_methods_[program_result_index(ProgramResult::kSkipped)] =
	   ProgramResultHandlingMethod::kContinue;

	try {
		bool reached_end;
		{
			char const* const program_name = next_word(parameters, reached_end);
			const ProductionSiteDescr::Programs& programs = descr.programs();
			ProductionSiteDescr::Programs::const_iterator const it = programs.find(program_name);
			if (it == programs.end())
				throw GameDataError("the program \"%s\" has not (yet) been declared in %s "
				                    "(wrong declaration order?)",
				                    program_name, descr.name().c_str());
			program_ = it->second.get();
		}

		//  Override with specified handling methods.
		while (!reached_end) {
			skip(parameters);
			match_force_skip(parameters, "on");

			ProgramResult result_to_set_method_for;
			if (match_force_skip(parameters, "failure")) {
				if (handling_methods_[program_result_index(ProgramResult::kFailed)] !=
				    ProgramResultHandlingMethod::kContinue)
					throw GameDataError("%s handling method already defined", "failure");
				result_to_set_method_for = ProgramResult::kFailed;
			} else if (match_force_skip(parameters, "completion")) {
				if (handling_methods_[program_result_index(ProgramResult::kCompleted)] !=
				    ProgramResultHandlingMethod::kContinue)
					throw GameDataError("%s handling method already defined", "completion");
				result_to_set_method_for = ProgramResult::kCompleted;
			} else if (match_force_skip(parameters, "skip")) {
				if (handling_methods_[program_result_index(ProgramResult::kSkipped)] !=
				    ProgramResultHandlingMethod::kContinue)
					throw GameDataError("%s handling method already defined", "skip");
				result_to_set_method_for = ProgramResult::kSkipped;
			} else
				throw GameDataError(
				   "expected %s but found \"%s\"", "{\"failure\"|\"completion\"|\"skip\"}", parameters);

			ProgramResultHandlingMethod handling_method;
			if (match(parameters, "fail"))
				handling_method = ProgramResultHandlingMethod::kFail;
			else if (match(parameters, "complete"))
				handling_method = ProgramResultHandlingMethod::kComplete;
			else if (match(parameters, "skip"))
				handling_method = ProgramResultHandlingMethod::kSkip;
			else if (match(parameters, "repeat"))
				handling_method = ProgramResultHandlingMethod::kRepeat;
			else
				throw GameDataError("expected %s but found \"%s\"",
				                    "{\"fail\"|\"complete\"|\"skip\"|\"repeat\"}", parameters);
			handling_methods_[program_result_index(result_to_set_method_for)] = handling_method;
			reached_end = !*parameters;
		}
	} catch (const WException& e) {
		throw GameDataError("call: %s", e.what());
	}
}

void ProductionProgram::ActCall::execute(Game& game, ProductionSite& ps) const {
	ProgramResult const program_result = ps.top_state().phase;

	if (program_result == ProgramResult::kNone) {  //  The program has not yet been called.
		return ps.program_start(game, program_->name());
	}

	switch (handling_methods_[program_result_index(program_result)]) {
	case ProgramResultHandlingMethod::kFail:
	case ProgramResultHandlingMethod::kComplete:
	case ProgramResultHandlingMethod::kSkip:
		return ps.program_end(game, ProgramResult::kNone);
	case ProgramResultHandlingMethod::kContinue:
		return ps.program_step(game);
	case ProgramResultHandlingMethod::kRepeat:
		ps.top_state().phase = ProgramResult::kNone;
		ps.program_timer_ = true;
		ps.program_time_ = ps.schedule_act(game, 10);
		break;
	}
}

ProductionProgram::ActCallWorker::ActCallWorker(char* parameters,
                                                const std::string& production_program_name,
                                                ProductionSiteDescr* descr,
                                                const Tribes& tribes) {
	try {
		program_ = parameters;

		//  Quote form "void ProductionSite::program_act(Game &)":
		//  "Always main worker is doing stuff"
		const WorkerDescr& main_worker_descr =
		   *tribes.get_worker_descr(descr->working_positions()[0].first);

		//  This will fail unless the main worker has a program with the given
		//  name, so it also validates the parameter.
		const WorkareaInfo& worker_workarea_info =
		   main_worker_descr.get_program(program_)->get_workarea_info();

		for (const auto& area_info : worker_workarea_info) {
			std::set<std::string>& building_radius_infos = descr->workarea_info_[area_info.first];

			for (const std::string& worker_name : area_info.second) {
				std::string description = descr->name();
				description += ' ';
				description += production_program_name;
				description += " worker ";
				description += main_worker_descr.name();
				description += worker_name;
				building_radius_infos.insert(description);
			}
		}
	} catch (const WException& e) {
		throw GameDataError("worker: %s", e.what());
	}
}

void ProductionProgram::ActCallWorker::execute(Game& game, ProductionSite& ps) const {
	// Always main worker is doing stuff
	ps.working_positions_[ps.main_worker_].worker->update_task_buildingwork(game);
}

bool ProductionProgram::ActCallWorker::get_building_work(Game& game,
                                                         ProductionSite& psite,
                                                         Worker& worker) const {
	ProductionSite::State& state = psite.top_state();
	if (state.phase == ProgramResult::kNone) {
		worker.start_task_program(game, program());
		state.phase = ProgramResult::kFailed;
		return true;
	} else {
		psite.program_step(game);
		return false;
	}
}

void ProductionProgram::ActCallWorker::building_work_failed(Game& game,
                                                            ProductionSite& psite,
                                                            Worker&) const {
	psite.program_end(game, ProgramResult::kFailed);
}

ProductionProgram::ActSleep::ActSleep(char* parameters) {
	try {
		if (*parameters) {
			char* endp;
			long long int const value = strtoll(parameters, &endp, 0);
			duration_ = value;
			if (*endp || value <= 0 || duration_ != value)
				throw GameDataError("expected %s but found \"%s\"", "duration in ms", parameters);
		} else
			duration_ = 0;  //  Get duration from the result of a previous action.
	} catch (const WException& e) {
		throw GameDataError("sleep: %s", e.what());
	}
}

void ProductionProgram::ActSleep::execute(Game& game, ProductionSite& ps) const {
	return ps.program_step(game, duration_ ? duration_ : 0, ps.top_state().phase);
}

ProductionProgram::ActCheckMap::ActCheckMap(char* parameters) {
	try {
		if (*parameters) {
			if (!strcmp(parameters, "seafaring"))
				feature_ = SEAFARING;
			else
				throw GameDataError("Unknown parameter \"%s\"", parameters);
		} else
			throw GameDataError("No parameter given!");
	} catch (const WException& e) {
		throw GameDataError("sleep: %s", e.what());
	}
}

void ProductionProgram::ActCheckMap::execute(Game& game, ProductionSite& ps) const {
	switch (feature_) {
	case SEAFARING: {
		if (game.map().allows_seafaring()) {
			return ps.program_step(game, 0);
		} else {
			ps.set_production_result(_("No use for ships on this map!"));
			return ps.program_end(game, ProgramResult::kFailed);
		}
	}
	default:
		NEVER_HERE();
	}
}

ProductionProgram::ActAnimate::ActAnimate(char* parameters, ProductionSiteDescr* descr) {
	try {
		bool reached_end;
		animation_name_ = std::string(next_word(parameters, reached_end));
		if (animation_name_ == "idle") {
			throw GameDataError("idle animation is default; calling is not allowed");
		}
		if (!descr->is_animation_known(animation_name_)) {
			throw GameDataError("Unknown animation '%s'", animation_name_.c_str());
		}
		if (!reached_end) {  //  The next parameter is the duration.
			char* endp;
			long long int const value = strtoll(parameters, &endp, 0);
			duration_ = value;
			if (*endp || value <= 0 || duration_ != value)
				throw GameDataError("expected %s but found \"%s\"", "duration in ms", parameters);
		} else
			duration_ = 0;  //  Get duration from the result of a previous action.
	} catch (const WException& e) {
		throw GameDataError("animate: %s", e.what());
	}
}

void ProductionProgram::ActAnimate::execute(Game& game, ProductionSite& ps) const {
	ps.start_animation(game, ps.descr().get_animation(animation_name_, &ps));
	return ps.program_step(game, duration_ ? duration_ : 0, ps.top_state().phase);
}

ProductionProgram::ActConsume::ActConsume(char* parameters,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	try {
		for (;;) {
			consumed_wares_workers_.resize(consumed_wares_workers_.size() + 1);
			parse_ware_type_group(parameters, *consumed_wares_workers_.rbegin(), tribes,
			                      descr.input_wares(), descr.input_workers());
			if (!*parameters)
				break;
			force_skip(parameters);
		}
		if (consumed_wares_workers_.empty()) {
			throw GameDataError("expected ware_type1[,ware_type2[,...]][:N] ...");
		}
	} catch (const WException& e) {
		throw GameDataError("consume: %s", e.what());
	}
}

void ProductionProgram::ActConsume::execute(Game& game, ProductionSite& ps) const {
	std::vector<InputQueue*> const inputqueues = ps.inputqueues();
	std::vector<uint8_t> consumption_quantities(inputqueues.size(), 0);

	Groups l_groups = consumed_wares_workers_;  //  make a copy for local modification

	//  Iterate over all input queues and see how much we should consume from
	//  each of them.
	bool found;
	for (size_t i = 0; i < inputqueues.size(); ++i) {
		DescriptionIndex const input_index = inputqueues[i]->get_index();
		WareWorker const input_type = inputqueues[i]->get_type();
		uint8_t nr_available = inputqueues[i]->get_filled();
		consumption_quantities[i] = 0;

		//  Iterate over all consume groups and see if they want us to consume
		//  any thing from the currently considered input queue.
		for (Groups::iterator it = l_groups.begin(); it != l_groups.end();) {
			found = false;
			for (auto input_it = it->first.begin(); input_it != it->first.end(); input_it++) {
				if (input_it->first == input_index && input_it->second == input_type) {
					found = true;
					if (it->second <= nr_available) {
						//  There are enough wares of the currently considered type
						//  to fulfill the requirements of the current group. We can
						//  therefore erase the group.
						consumption_quantities[i] += it->second;
						nr_available -= it->second;
						it = l_groups.erase(it);
						//  No increment here, erase moved next element to the position
						//  pointed to by it.
					} else {
						consumption_quantities[i] += nr_available;
						it->second -= nr_available;
						++it;  //  Now check if the next group includes this ware type.
					}
					break;
				}
			}
			// group does not request ware
			if (!found)
				++it;
		}
	}

	// "Did not start working because .... is/are missing"
	if (uint8_t const nr_missing_groups = l_groups.size()) {
		const TribeDescr& tribe = ps.owner().tribe();

		std::vector<std::string> group_list;
		for (const auto& group : l_groups) {
			assert(group.first.size());

			std::vector<std::string> ware_list;
			for (const auto& entry : group.first) {
				ware_list.push_back(entry.second == wwWARE ?
				                       tribe.get_ware_descr(entry.first)->descname() :
				                       tribe.get_worker_descr(entry.first)->descname());
			}
			std::string ware_string = i18n::localize_list(ware_list, i18n::ConcatenateWith::OR);

			uint8_t const count = group.second;
			if (1 < count) {
				ware_string =
				   /** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are
				      missing' */
				   /** TRANSLATORS: For this example, this is what's in the place holders: */
				   /** TRANSLATORS:    %1$i = "3" */
				   /** TRANSLATORS:    %2$s = "water" */
				   (boost::format(_("%1$ix %2$s")) % static_cast<unsigned int>(count) % ware_string)
				      .str();
			}
			group_list.push_back(ware_string);
		}

		const std::string is_missing_string =
		   /** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
		   /** TRANSLATORS: e.g. 'Did not start working because fish, meat or pitta bread is missing'
		    */
		   (boost::format(ngettext("%s is missing", "%s are missing", nr_missing_groups)) %
		    i18n::localize_list(group_list, i18n::ConcatenateWith::AND))
		      .str();

		std::string result_string =
		   /** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
		   /** TRANSLATORS: For this example, this is what's in the place holders: */
		   /** TRANSLATORS:    %1$s = "working" */
		   /** TRANSLATORS:    %2$s = "3x water and 3x wheat are missing" */
		   /** TRANSLATORS: This appears in the hover text on buildings. Please test these in
		      context*/
		   /** TRANSLATORS: on a development build if you can, and let us know if there are any issues
		    */
		   /** TRANSLATORS: we need to address for your language. */
		   (boost::format(_("Did not start %1$s because %2$s")) % ps.top_state().program->descname() %
		    is_missing_string)
		      .str();

		if (ps.production_result() != ps.descr().out_of_resource_heading() ||
		    ps.descr().out_of_resource_heading().empty()) {
			ps.set_production_result(result_string);
		}
		return ps.program_end(game, ProgramResult::kFailed);
	} else {  //  we fulfilled all consumption requirements
		for (size_t i = 0; i < inputqueues.size(); ++i) {
			if (uint8_t const q = consumption_quantities[i]) {
				assert(q <= inputqueues[i]->get_filled());
				inputqueues[i]->set_filled(inputqueues[i]->get_filled() - q);

				// Update consumption statistics
				if (inputqueues[i]->get_type() == wwWARE) {
					ps.get_owner()->ware_consumed(inputqueues[i]->get_index(), q);
				}
			}
		}
		return ps.program_step(game);
	}
}

ProductionProgram::ActProduce::ActProduce(char* parameters,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	try {
		for (bool more = true; more; ++parameters) {
			produced_wares_.resize(produced_wares_.size() + 1);
			WareAmount& item = *produced_wares_.rbegin();
			skip(parameters);
			char const* ware = parameters;
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
					char* endp;
					unsigned long long int const value = strtoull(parameters, &endp, 0);
					item.second = value;
					if ((*endp && *endp != ' ') || value < 1 || item.second != value)
						throw GameDataError("expected %s but found \"%s\"", "count", parameters);
					parameters = endp;
					goto item_end;
				}
				}
			}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			if (!descr.is_output_ware_type(item.first = tribes.safe_ware_index(ware)))
				throw GameDataError("%s is not declared as an output (\"%s\" was not "
				                    "found in the \"outputs\" table)",
				                    ware, ware);
		}
	} catch (const WException& e) {
		throw GameDataError("produce: %s", e.what());
	}
}

void ProductionProgram::ActProduce::execute(Game& game, ProductionSite& ps) const {
	assert(ps.produced_wares_.empty());
	ps.produced_wares_ = produced_wares_;
	ps.working_positions_[ps.main_worker_].worker->update_task_buildingwork(game);

	const TribeDescr& tribe = ps.owner().tribe();
	assert(produced_wares_.size());

	std::vector<std::string> ware_descnames;
	uint8_t count = 0;
	for (const auto& item_pair : produced_wares_) {
		count += item_pair.second;
		std::string ware_descname = tribe.get_ware_descr(item_pair.first)->descname();
		if (1 < item_pair.second || 1 < produced_wares_.size()) {
			/** TRANSLATORS: This is an item in a list of wares, e.g. "Produced 2x Coal": */
			/** TRANSLATORS:    %%1$i = "2" */
			/** TRANSLATORS:    %2$s = "Coal" */
			ware_descname = (boost::format(_("%1$ix %2$s")) %
			                 static_cast<unsigned int>(item_pair.second) % ware_descname)
			                   .str();
		}
		ware_descnames.push_back(ware_descname);
	}
	std::string ware_list = i18n::localize_list(ware_descnames, i18n::ConcatenateWith::AND);

	const std::string result_string =
	   /** TRANSLATORS: %s is a list of wares. String is fetched according to total amount of
	      wares. */
	   (boost::format(ngettext("Produced %s", "Produced %s", count)) % ware_list).str();
	if (ps.production_result() != ps.descr().out_of_resource_heading() ||
	    ps.descr().out_of_resource_heading().empty()) {
		ps.set_production_result(result_string);
	}
}

bool ProductionProgram::ActProduce::get_building_work(Game& game,
                                                      ProductionSite& psite,
                                                      Worker& /* worker */) const {
	// We reach this point once all wares have been carried outside the building
	psite.program_step(game);
	return false;
}

ProductionProgram::ActRecruit::ActRecruit(char* parameters,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	try {
		for (bool more = true; more; ++parameters) {
			recruited_workers_.resize(recruited_workers_.size() + 1);
			WareAmount& item = *recruited_workers_.rbegin();
			skip(parameters);
			char const* worker = parameters;
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
					char* endp;
					unsigned long long int const value = strtoull(parameters, &endp, 0);
					item.second = value;
					if ((*endp && *endp != ' ') || value < 1 || item.second != value)
						throw GameDataError("expected %s but found \"%s\"", "count", parameters);
					parameters = endp;
					goto item_end;
				}
				}
			}
		item_end:
			more = *parameters != '\0';
			*parameters = '\0';
			if (!descr.is_output_worker_type(item.first = tribes.safe_worker_index(worker)))
				throw GameDataError("%s is not declared as an output (\"%s\" was not "
				                    "found in the \"outputs\" table)",
				                    worker, worker);
		}
	} catch (const WException& e) {
		throw GameDataError("recruit: %s", e.what());
	}
}

void ProductionProgram::ActRecruit::execute(Game& game, ProductionSite& ps) const {
	assert(ps.recruited_workers_.empty());
	ps.recruited_workers_ = recruited_workers_;
	ps.working_positions_[ps.main_worker_].worker->update_task_buildingwork(game);

	const TribeDescr& tribe = ps.owner().tribe();
	assert(recruited_workers_.size());
	std::vector<std::string> worker_descnames;
	uint8_t count = 0;
	for (const auto& item_pair : recruited_workers_) {
		count += item_pair.second;
		std::string worker_descname = tribe.get_worker_descr(item_pair.first)->descname();
		if (1 < item_pair.second || 1 < recruited_workers_.size()) {
			/** TRANSLATORS: This is an item in a list of workers, e.g. "Recruited 2x Ox": */
			/** TRANSLATORS:    %1$i = "2" */
			/** TRANSLATORS:    %2$s = "Ox" */
			worker_descname = (boost::format(_("%1$ix %2$s")) %
			                   static_cast<unsigned int>(item_pair.second) % worker_descname)
			                     .str();
		}
		worker_descnames.push_back(worker_descname);
	}
	std::string unit_string = i18n::localize_list(worker_descnames, i18n::ConcatenateWith::AND);

	const std::string result_string =
	   /** TRANSLATORS: %s is a list of workers. String is fetched according to total amount of
	      workers. */
	   (boost::format(ngettext("Recruited %s", "Recruited %s", count)) % unit_string).str();
	ps.set_production_result(result_string);
}

bool ProductionProgram::ActRecruit::get_building_work(Game& game,
                                                      ProductionSite& psite,
                                                      Worker& /* worker */) const {
	// We reach this point once all recruits have been guided outside the building
	psite.program_step(game);
	return false;
}

ProductionProgram::ActMine::ActMine(char* parameters,
                                    const World& world,
                                    const std::string& production_program_name,
                                    ProductionSiteDescr* descr) {
	try {
		bool reached_end;
		resource_ = world.safe_resource_index(next_word(parameters, reached_end));

		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			distance_ = value;
			if (*endp != ' ' || distance_ != value)
				throw GameDataError("expected %s but found \"%s\"", "distance", parameters);
			parameters = endp;
		}

		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			max_ = value;
			if (*endp != ' ' || value < 1 || 100 < value)
				throw GameDataError("expected %s but found \"%s\"", "percentage", parameters);
			parameters = endp;
		}

		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			chance_ = value;
			if (*endp != ' ' || value < 1 || 100 < value)
				throw GameDataError("expected %s but found \"%s\"", "percentage", parameters);
			parameters = endp;
		}
		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			training_ = value;
			if (*endp || value < 1 || 100 < value)
				throw GameDataError("expected %s but found \"%s\"", "percentage", parameters);
		}
		std::string description = (boost::format("%1$s %2$s mine %3$s") % descr->name() %
		                           production_program_name % world.get_resource(resource_)->name())
		                             .str();

		descr->workarea_info_[distance_].insert(description);
	} catch (const WException& e) {
		throw GameDataError("mine: %s", e.what());
	}
}

void ProductionProgram::ActMine::execute(Game& game, ProductionSite& ps) const {
	Map* map = game.mutable_map();

	//  select one of the nodes randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	uint32_t totalstart = 0;

	{
		MapRegion<Area<FCoords>> mr(
		   *map, Area<FCoords>(map->get_fcoords(ps.get_position()), distance_));
		do {
			DescriptionIndex fres = mr.location().field->get_resources();
			ResourceAmount amount = mr.location().field->get_resources_amount();
			ResourceAmount start_amount = mr.location().field->get_initial_res_amount();

			if (fres != resource_) {
				amount = 0;
				start_amount = 0;
			}

			totalres += amount;
			totalstart += start_amount;
			totalchance += 8 * amount;

			// Add penalty for fields that are running out
			// Except for totally depleted fields or wrong ressource fields
			// if we already know there is no ressource (left) we won't mine there
			if (amount == 0)
				totalchance += 0;
			else if (amount <= 2)
				totalchance += 6;
			else if (amount <= 4)
				totalchance += 4;
			else if (amount <= 6)
				totalchance += 2;
		} while (mr.advance(*map));
	}

	//  how much is digged
	int32_t digged_percentage = 100;
	if (totalstart)
		digged_percentage = (totalstart - totalres) * 100 / totalstart;
	if (!totalres)
		digged_percentage = 100;

	if (digged_percentage < max_) {
		//  mine can produce normally
		if (totalres == 0)
			return ps.program_end(game, ProgramResult::kFailed);

		//  second pass through nodes
		assert(totalchance);
		int32_t pick = game.logic_rand() % totalchance;

		{
			MapRegion<Area<FCoords>> mr(
			   *map, Area<FCoords>(map->get_fcoords(ps.get_position()), distance_));
			do {
				DescriptionIndex fres = mr.location().field->get_resources();
				ResourceAmount amount = mr.location().field->get_resources_amount();

				if (fres != resource_)
					amount = 0;

				pick -= 8 * amount;
				if (pick < 0) {
					assert(amount > 0);

					--amount;
					map->set_resources(mr.location(), amount);
					break;
				}
			} while (mr.advance(*map));
		}

		if (pick >= 0) {
			return ps.program_end(game, ProgramResult::kFailed);
		}

	} else {
		//  Inform the player about an empty mine, unless
		//  there is a sufficiently high chance, that the mine
		//  will still produce enough.
		//  e.g. mines have chance=5, wells have 65
		if (chance_ <= 20) {
			ps.notify_player(game, 60);
			// and change the default animation
			ps.set_default_anim("empty");
		}

		//  Mine has reached its limits, still try to produce something but
		//  independent of sourrunding resources. Do not decrease resources
		//  further.
		if (chance_ <= game.logic_rand() % 100) {

			// Gain experience
			if (training_ >= game.logic_rand() % 100) {
				ps.train_workers(game);
			}
			return ps.program_end(game, ProgramResult::kFailed);
		}
	}

	//  done successful
	//  TODO(unknown): Should pass the time it takes to mine in the phase parameter of
	//  ProductionSite::program_step so that the following sleep/animate
	//  command knows how long it should last.
	return ps.program_step(game);
}

ProductionProgram::ActCheckSoldier::ActCheckSoldier(char* parameters) {
	//  TODO(unknown): This is currently hardcoded for "soldier", but should allow any
	//  soldier type name.
	if (!match_force_skip(parameters, "soldier"))
		throw GameDataError("expected %s but found \"%s\"", "soldier type", parameters);
	try {
		if (match_force_skip(parameters, "health"))
			attribute = TrainingAttribute::kHealth;
		else if (match_force_skip(parameters, "attack"))
			attribute = TrainingAttribute::kAttack;
		else if (match_force_skip(parameters, "defense"))
			attribute = TrainingAttribute::kDefense;
		else if (match_force_skip(parameters, "evade"))
			attribute = TrainingAttribute::kEvade;
		else
			throw GameDataError("expected %s but found \"%s\"",
			                    "{\"health\"|\"attack\"|\"defense\"|\"evade\"}", parameters);

		char* endp;
		unsigned long long int const value = strtoull(parameters, &endp, 0);
		level = value;
		if (*endp || level != value)
			throw GameDataError("expected %s but found \"%s\"", "level", parameters);
	} catch (const WException& e) {
		throw GameDataError("checksoldier: %s", e.what());
	}
}

void ProductionProgram::ActCheckSoldier::execute(Game& game, ProductionSite& ps) const {
	const SoldierControl* ctrl = ps.soldier_control();
	assert(ctrl != nullptr);
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();
	if (soldiers.empty()) {
		ps.set_production_result(_("No soldier to train!"));
		return ps.program_end(game, ProgramResult::kSkipped);
	}
	ps.molog("  Checking soldier (%u) level %d)\n", static_cast<unsigned int>(attribute),
	         static_cast<unsigned int>(level));

	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();
	for (std::vector<Soldier*>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
			return ps.program_end(game, ProgramResult::kSkipped);
		}
		if (attribute == TrainingAttribute::kHealth) {
			if ((*it)->get_health_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kAttack) {
			if ((*it)->get_attack_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kDefense) {
			if ((*it)->get_defense_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kEvade) {
			if ((*it)->get_evade_level() == level)
				break;
		}
	}
	ps.molog("    okay\n");  // okay, do nothing

	upcast(TrainingSite, ts, &ps);
	ts->training_attempted(attribute, level);

	ps.molog("  Check done!\n");

	return ps.program_step(game);
}

ProductionProgram::ActTrain::ActTrain(char* parameters) {
	//  TODO(unknown): This is currently hardcoded for "soldier", but should allow any
	//  soldier type name.
	if (!match_force_skip(parameters, "soldier"))
		throw GameDataError("expected %s but found \"%s\"", "soldier type", parameters);
	try {
		if (match_force_skip(parameters, "health"))
			attribute = TrainingAttribute::kHealth;
		else if (match_force_skip(parameters, "attack"))
			attribute = TrainingAttribute::kAttack;
		else if (match_force_skip(parameters, "defense"))
			attribute = TrainingAttribute::kDefense;
		else if (match_force_skip(parameters, "evade"))
			attribute = TrainingAttribute::kEvade;
		else
			throw GameDataError("expected %s but found \"%s\"",
			                    "{\"health\"|\"attack\"|\"defense\"|\"evade\"}", parameters);

		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			level = value;
			if (*endp != ' ' || level != value)
				throw GameDataError("expected %s but found \"%s\"", "level", parameters);
			parameters = endp;
		}

		{
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			target_level = value;
			if (*endp || target_level != value || target_level <= level)
				throw GameDataError("expected level > %u but found \"%s\"", level, parameters);
		}
	} catch (const WException& e) {
		throw GameDataError("train: %s", e.what());
	}
}

void ProductionProgram::ActTrain::execute(Game& game, ProductionSite& ps) const {
	const SoldierControl* ctrl = ps.soldier_control();
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();
	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();
	std::vector<Soldier*>::const_iterator it = soldiers.begin();

	ps.molog("  Training soldier's %u (%d to %d)", static_cast<unsigned int>(attribute),
	         static_cast<unsigned int>(level), static_cast<unsigned int>(target_level));

	for (;; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
			return ps.program_end(game, ProgramResult::kSkipped);
		}
		if (attribute == TrainingAttribute::kHealth) {
			if ((*it)->get_health_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kAttack) {
			if ((*it)->get_attack_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kDefense) {
			if ((*it)->get_defense_level() == level)
				break;
		} else if (attribute == TrainingAttribute::kEvade) {
			if ((*it)->get_evade_level() == level)
				break;
		}
	}
	ps.molog("    okay\n");  // okay, do nothing

	try {
		if (attribute == TrainingAttribute::kHealth)
			(*it)->set_health_level(target_level);
		else if (attribute == TrainingAttribute::kAttack)
			(*it)->set_attack_level(target_level);

		else if (attribute == TrainingAttribute::kDefense)
			(*it)->set_defense_level(target_level);

		else if (attribute == TrainingAttribute::kEvade)
			(*it)->set_evade_level(target_level);

	} catch (...) {
		throw wexception("Fail training soldier!!");
	}
	ps.molog("  Training done!\n");
	ps.set_production_result(
	   /** TRANSLATORS: Success message of a trainingsite '%s' stands for the description of the
	    * training program, e.g. Completed upgrading soldier evade from level 0 to level 1 */
	   (boost::format(_("Completed %s")) % ps.top_state().program->descname()).str());

	upcast(TrainingSite, ts, &ps);
	ts->training_successful(attribute, level);

	return ps.program_step(game);
}

ProductionProgram::ActPlaySound::ActPlaySound(char* parameters) {
	try {
		bool reached_end;
		const char* const name = next_word(parameters, reached_end);
		fx = SoundHandler::register_fx(SoundType::kAmbient, name);

		if (!reached_end) {
			char* endp;
			unsigned long long int const value = strtoull(parameters, &endp, 0);
			priority = value;
			if (*endp || priority != value)
				throw GameDataError("expected %s but found \"%s\"", "priority", parameters);
		} else {
			priority = kFxPriorityAllowMultiple - 1;
		}
		if (priority < kFxPriorityLowest) {
			throw GameDataError("Minmum priority for sounds is %d, but only %d was specified for %s",
			                    kFxPriorityLowest, priority, name);
		}
	} catch (const WException& e) {
		throw GameDataError("playsound: %s", e.what());
	}
}

void ProductionProgram::ActPlaySound::execute(Game& game, ProductionSite& ps) const {
	Notifications::publish(NoteSound(SoundType::kAmbient, fx, ps.position_, priority));
	return ps.program_step(game);
}

ProductionProgram::ActConstruct::ActConstruct(char* parameters,
                                              const std::string& production_program_name,
                                              ProductionSiteDescr* descr) {
	try {
		std::vector<std::string> params = split_string(parameters, " ");

		if (params.size() != 3)
			throw GameDataError("usage: construct object-name worker-program radius:NN");

		objectname = params[0];
		workerprogram = params[1];
		radius = boost::lexical_cast<uint32_t>(params[2]);

		std::set<std::string>& building_radius_infos = descr->workarea_info_[radius];
		std::string description = descr->name() + ' ' + production_program_name;
		description += " construct ";
		description += objectname;
		building_radius_infos.insert(description);
	} catch (const WException& e) {
		throw GameDataError("construct: %s", e.what());
	}
}

const ImmovableDescr&
ProductionProgram::ActConstruct::get_construction_descr(const Tribes& tribes) const {
	const ImmovableDescr* descr = tribes.get_immovable_descr(tribes.immovable_index(objectname));
	if (!descr)
		throw wexception("ActConstruct: immovable '%s' does not exist", objectname.c_str());

	return *descr;
}

void ProductionProgram::ActConstruct::execute(Game& game, ProductionSite& psite) const {
	ProductionSite::State& state = psite.top_state();
	const ImmovableDescr& descr = get_construction_descr(game.tribes());

	// Early check for no resources
	const Buildcost& buildcost = descr.buildcost();
	DescriptionIndex available_resource = INVALID_INDEX;

	for (Buildcost::const_iterator it = buildcost.begin(); it != buildcost.end(); ++it) {
		if (psite.inputqueue(it->first, wwWARE).get_filled() > 0) {
			available_resource = it->first;
			break;
		}
	}

	if (available_resource == INVALID_INDEX) {
		psite.program_end(game, ProgramResult::kFailed);
		return;
	}

	// Look for an appropriate object in the given radius
	const Map& map = game.map();
	std::vector<ImmovableFound> immovables;
	CheckStepWalkOn cstep(MOVECAPS_WALK, true);
	Area<FCoords> area(map.get_fcoords(psite.base_flag().get_position()), radius);
	if (map.find_reachable_immovables(game, area, &immovables, cstep, FindImmovableByDescr(descr))) {
		state.objvar = immovables[0].object;

		psite.working_positions_[psite.main_worker_].worker->update_task_buildingwork(game);
		return;
	}

	// No object found, look for a field where we can build
	std::vector<Coords> fields;
	FindNodeAnd fna;
	// 10 is custom value to make sure the "water" is at least 10 nodes big
	fna.add(FindNodeShore(10));
	fna.add(FindNodeImmovableSize(FindNodeImmovableSize::sizeNone));
	if (map.find_reachable_fields(game, area, &fields, cstep, fna)) {
		// Testing received fields to get one with less immovables nearby
		Coords best_coords = fields.back();  // Just to initialize it
		uint32_t best_score = std::numeric_limits<uint32_t>::max();
		while (!fields.empty()) {
			Coords coords = fields.back();

			// Counting immovables nearby
			std::vector<ImmovableFound> found_immovables;
			const uint32_t imm_count =
			   map.find_immovables(game, Area<FCoords>(map.get_fcoords(coords), 2), &found_immovables);
			if (best_score > imm_count) {
				best_score = imm_count;
				best_coords = coords;
			}

			// No need to go on, it cannot be better
			if (imm_count == 0) {
				break;
			}

			fields.pop_back();
		}

		state.coord = best_coords;

		psite.working_positions_[psite.main_worker_].worker->update_task_buildingwork(game);
		return;
	}

	psite.molog("construct: no object or buildable field\n");
	psite.program_end(game, ProgramResult::kFailed);
}

bool ProductionProgram::ActConstruct::get_building_work(Game& game,
                                                        ProductionSite& psite,
                                                        Worker& worker) const {
	ProductionSite::State& state = psite.top_state();
	if (state.phase > ProgramResult::kNone) {
		psite.program_step(game);
		return false;
	}

	// First step: figure out which ware item to bring along
	Buildcost remaining;
	WaresQueue* wq = nullptr;

	Immovable* construction = dynamic_cast<Immovable*>(state.objvar.get(game));
	if (construction) {
		if (!construction->construct_remaining_buildcost(game, &remaining)) {
			psite.molog("construct: immovable %u not under construction", construction->serial());
			psite.program_end(game, ProgramResult::kFailed);
			return false;
		}
	} else {
		const ImmovableDescr& descr = get_construction_descr(game.tribes());
		remaining = descr.buildcost();
	}

	for (Buildcost::const_iterator it = remaining.begin(); it != remaining.end(); ++it) {
		WaresQueue& thiswq = dynamic_cast<WaresQueue&>(psite.inputqueue(it->first, wwWARE));
		if (thiswq.get_filled() > 0) {
			wq = &thiswq;
			break;
		}
	}

	if (!wq) {
		psite.program_end(game, ProgramResult::kFailed);
		return false;
	}

	// Second step: give ware to worker
	WareInstance* ware =
	   new WareInstance(wq->get_index(), game.tribes().get_ware_descr(wq->get_index()));
	ware->init(game);
	worker.set_carried_ware(game, ware);
	wq->set_filled(wq->get_filled() - 1);

	// Third step: send worker on his merry way, giving the target object or coords
	worker.start_task_program(game, workerprogram);
	worker.top_state().objvar1 = construction;
	worker.top_state().coords = state.coord;

	state.phase = ProgramResult::kFailed;
	return true;
}

void ProductionProgram::ActConstruct::building_work_failed(Game& game,
                                                           ProductionSite& psite,
                                                           Worker&) const {
	psite.program_end(game, ProgramResult::kFailed);
}

ProductionProgram::ProductionProgram(const std::string& init_name,
                                     const std::string& init_descname,
                                     std::unique_ptr<LuaTable> actions_table,
                                     const Tribes& tribes,
                                     const World& world,
                                     ProductionSiteDescr* building)
   : name_(init_name), descname_(init_descname) {

	for (const std::string& action_string : actions_table->array_entries<std::string>()) {
		std::vector<std::string> parts;
		boost::split(parts, action_string, boost::is_any_of("="));
		if (parts.size() != 2) {
			throw GameDataError(
			   "invalid line: \"%s\" in production program \"%s\" for building \"%s\"",
			   action_string.c_str(), name().c_str(), building->name().c_str());
		}
		std::unique_ptr<char[]> arguments(new char[parts[1].size() + 1]);
		strncpy(arguments.get(), parts[1].c_str(), parts[1].size() + 1);

		if (boost::iequals(parts[0], "return")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActReturn(arguments.get(), *building, tribes)));
		} else if (boost::iequals(parts[0], "call")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActCall(arguments.get(), *building)));
		} else if (boost::iequals(parts[0], "sleep")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActSleep(arguments.get())));
		} else if (boost::iequals(parts[0], "animate")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActAnimate(arguments.get(), building)));
		} else if (boost::iequals(parts[0], "consume")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActConsume(arguments.get(), *building, tribes)));
		} else if (boost::iequals(parts[0], "produce")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActProduce(arguments.get(), *building, tribes)));
		} else if (boost::iequals(parts[0], "recruit")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActRecruit(arguments.get(), *building, tribes)));
		} else if (boost::iequals(parts[0], "callworker")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActCallWorker(arguments.get(), name(), building, tribes)));
		} else if (boost::iequals(parts[0], "mine")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActMine(arguments.get(), world, name(), building)));
		} else if (boost::iequals(parts[0], "checksoldier")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActCheckSoldier(arguments.get())));
		} else if (boost::iequals(parts[0], "train")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActTrain(arguments.get())));
		} else if (boost::iequals(parts[0], "playsound")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActPlaySound(arguments.get())));
		} else if (boost::iequals(parts[0], "construct")) {
			actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
			   new ActConstruct(arguments.get(), name(), building)));
		} else if (boost::iequals(parts[0], "checkmap")) {
			actions_.push_back(
			   std::unique_ptr<ProductionProgram::Action>(new ActCheckMap(arguments.get())));
		} else {
			throw GameDataError(
			   "unknown command type \"%s\" in production program \"%s\" for building \"%s\"",
			   arguments.get(), name().c_str(), building->name().c_str());
		}

		const ProductionProgram::Action& action = *actions_.back();
		for (const auto& group : action.consumed_wares_workers()) {
			consumed_wares_workers_.push_back(group);
		}
		// Add produced wares. If the ware already exists, increase the amount
		for (const auto& ware : action.produced_wares()) {
			if (produced_wares_.count(ware.first) == 1) {
				produced_wares_.at(ware.first) += ware.second;
			} else {
				produced_wares_.insert(ware);
			}
		}
		// Add recruited workers. If the worker already exists, increase the amount
		for (const auto& worker : action.recruited_workers()) {
			if (recruited_workers_.count(worker.first) == 1) {
				recruited_workers_.at(worker.first) += worker.second;
			} else {
				recruited_workers_.insert(worker);
			}
		}
	}
	if (actions_.empty())
		throw GameDataError("no actions in production program \"%s\" for building \"%s\"",
		                    name().c_str(), building->name().c_str());
}

const std::string& ProductionProgram::name() const {
	return name_;
}
const std::string& ProductionProgram::descname() const {
	return descname_;
}
size_t ProductionProgram::size() const {
	return actions_.size();
}

const ProductionProgram::Action& ProductionProgram::operator[](size_t const idx) const {
	return *actions_.at(idx);
}

const ProductionProgram::Groups& ProductionProgram::consumed_wares_workers() const {
	return consumed_wares_workers_;
}
const Buildcost& ProductionProgram::produced_wares() const {
	return produced_wares_;
}
const Buildcost& ProductionProgram::recruited_workers() const {
	return recruited_workers_;
}
}  // namespace Widelands
