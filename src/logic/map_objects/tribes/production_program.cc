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

#include <cassert>
#include <memory>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "config.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/input_queue.h"
#include "economy/wares_queue.h"
#include "io/filesystem/layered_filesystem.h"
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
/// If the iterator contents match the string, increment the iterator. Returns whether it matched.
bool match_and_skip(const std::vector<std::string>& args,
                    std::vector<std::string>::const_iterator& it,
                    const std::string& matchme) {
	const bool result = (it != args.end()) && (*it == matchme);
	if (result) {
		++it;
	}
	return result;
}

ProductionProgram::ActReturn::Condition* create_economy_condition(const std::string& item,
                                                                  const ProductionSiteDescr& descr,
                                                                  const Tribes& tribes) {
	DescriptionIndex index = tribes.ware_index(item);
	if (tribes.ware_exists(index)) {
		descr.ware_demand_checks()->insert(index);
		return new ProductionProgram::ActReturn::EconomyNeedsWare(index);
	} else if (tribes.worker_exists(tribes.worker_index(item))) {
		index = tribes.worker_index(item);
		descr.worker_demand_checks()->insert(index);
		return new ProductionProgram::ActReturn::EconomyNeedsWorker(index);
	} else {
		throw GameDataError("Expected ware or worker type but found '%s'", item.c_str());
	}
}

TrainingAttribute parse_training_attribute(const std::string& argument) {
	if (argument == "health") {
		return TrainingAttribute::kHealth;
	} else if (argument == "attack") {
		return TrainingAttribute::kAttack;
	} else if (argument == "defense") {
		return TrainingAttribute::kDefense;
	} else if (argument == "evade") {
		return TrainingAttribute::kEvade;
	} else {
		throw GameDataError(
		   "Expected health|attack|defense|evade after 'soldier' but found '%s'", argument.c_str());
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

ProductionProgram::Groups
ProductionProgram::parse_ware_type_groups(std::vector<std::string>::const_iterator begin,
                                          std::vector<std::string>::const_iterator end,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	ProductionProgram::Groups result;

	for (auto& it = begin; it != end; ++it) {
		const std::pair<std::string, std::string> names_to_amount =
		   read_key_value_pair(*it, ':', "1");
		const uint8_t amount = read_positive(names_to_amount.second);
		uint8_t max_amount = 0;
		std::set<std::pair<DescriptionIndex, WareWorker>> ware_worker_names;
		for (const std::string& item_name : split_string(names_to_amount.first, ",")) {
			// Try as ware
			WareWorker type = wwWARE;
			DescriptionIndex item_index = tribes.ware_index(item_name);
			if (!tribes.ware_exists(item_index)) {
				item_index = tribes.worker_index(item_name);
				if (tribes.worker_exists(item_index)) {
					// It is a worker
					type = wwWORKER;
				} else {
					throw GameDataError(
					   "Expected ware or worker type but found '%s'", item_name.c_str());
				}
			}

			// Sanity checks
			bool found = false;
			const BillOfMaterials& inputs =
			   (type == wwWARE) ? descr.input_wares() : descr.input_workers();
			for (const WareAmount& input : inputs) {
				if (input.first == item_index) {
					max_amount += input.second;
					found = true;
					break;
				}
			}
			if (!found) {
				throw GameDataError(
				   "%s was not declared in the building's 'inputs' table", item_name.c_str());
			}

			if (max_amount < amount) {
				throw GameDataError(
				   "Ware/worker count is %u but (total) input storage capacity of "
				   "the specified ware type(s) is only %u, so the ware/worker requirement can "
				   "never be fulfilled by the site",
				   static_cast<unsigned int>(amount), static_cast<unsigned int>(max_amount));
			}
			// Add item
			ware_worker_names.insert(std::make_pair(item_index, type));
		}
		// Add set
		result.push_back(std::make_pair(ware_worker_names, amount));
	}
	if (result.empty()) {
		throw GameDataError("No wares or workers found");
	}
	return result;
}

BillOfMaterials
ProductionProgram::parse_bill_of_materials(const std::vector<std::string>& arguments,
                                           WareWorker ww,
                                           const ProductionSiteDescr& descr,
                                           const Tribes& tribes) {
	BillOfMaterials result;
	for (const std::string& argument : arguments) {
		const std::pair<std::string, std::string> produceme = read_key_value_pair(argument, ':', "1");

		const DescriptionIndex index = ww == WareWorker::wwWARE ?
		                                  tribes.safe_ware_index(produceme.first) :
		                                  tribes.safe_worker_index(produceme.first);

		// Verify the building outputs
		switch (ww) {
		case WareWorker::wwWARE:
			if (!descr.is_output_ware_type(index)) {
				throw GameDataError("Ware '%s' is not listed in the building's 'outputs' table",
				                    produceme.first.c_str());
			}
			break;
		case WareWorker::wwWORKER:
			if (!descr.is_output_worker_type(index)) {
				throw GameDataError("Worker '%s' is not listed in the building's 'outputs' table",
				                    produceme.first.c_str());
			}
			break;
		}

		result.push_back(std::make_pair(index, read_positive(produceme.second)));
	}
	return result;
}

ProductionProgram::ActReturn::Condition::~Condition() {
}

ProductionProgram::ActReturn::Negation::Negation(const std::vector<std::string>& arguments,
                                                 std::vector<std::string>::const_iterator& begin,
                                                 std::vector<std::string>::const_iterator& end,
                                                 const ProductionSiteDescr& descr,
                                                 const Tribes& tribes)
   : operand(create_condition(arguments, begin, end, descr, tribes)) {
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

ProductionProgram::ActReturn::SiteHas::SiteHas(std::vector<std::string>::const_iterator begin,
                                               std::vector<std::string>::const_iterator end,
                                               const ProductionSiteDescr& descr,
                                               const Tribes& tribes) {
	try {
		group = parse_ware_type_groups(begin, end, descr, tribes).front();
	} catch (const GameDataError& e) {
		throw GameDataError("Expected <ware or worker>[,<ware or worker>[,...]][:<amount>] after "
		                    "'site has' but got %s",
		                    e.what());
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

ProductionProgram::ActReturn::Condition*
ProductionProgram::ActReturn::create_condition(const std::vector<std::string>& arguments,
                                               std::vector<std::string>::const_iterator& begin,
                                               std::vector<std::string>::const_iterator& end,
                                               const ProductionSiteDescr& descr,
                                               const Tribes& tribes) {
	if (begin == end) {
		throw GameDataError("Expected a condition after '%s'", (begin - 1)->c_str());
	}
	try {
		if (match_and_skip(arguments, begin, "not")) {
			return new ActReturn::Negation(arguments, begin, end, descr, tribes);
		} else if (match_and_skip(arguments, begin, "economy")) {
			if (!match_and_skip(arguments, begin, "needs")) {
				throw GameDataError("Expected 'needs' after 'economy' but found '%s'", begin->c_str());
			}
			return create_economy_condition(*begin, descr, tribes);
		} else if (match_and_skip(arguments, begin, "site")) {
			if (!match_and_skip(arguments, begin, "has")) {
				throw GameDataError("Expected 'has' after 'site' but found '%s'", begin->c_str());
			}
			return new ProductionProgram::ActReturn::SiteHas(begin, end, descr, tribes);
		} else if (match_and_skip(arguments, begin, "workers")) {
			if (!match_and_skip(arguments, begin, "need")) {
				throw GameDataError(
				   "Expected 'need experience' after 'workers' but found '%s'", begin->c_str());
			}
			if (!match_and_skip(arguments, begin, "experience")) {
				throw GameDataError(
				   "Expected 'experience' after 'workers need' but found '%s'", begin->c_str());
			}
			return new ProductionProgram::ActReturn::WorkersNeedExperience();
		} else {
			throw GameDataError("Expected not|economy|site|workers after '%s' but found '%s'",
			                    (begin - 1)->c_str(), begin->c_str());
		}
	} catch (const WException& e) {
		throw GameDataError("Invalid condition. %s", e.what());
	}
}

ProductionProgram::ActReturn::ActReturn(const std::vector<std::string>& arguments,
                                        const ProductionSiteDescr& descr,
                                        const Tribes& tribes) {
	if (arguments.empty()) {
		throw GameDataError(
		   "Usage: return=failed|completed|skipped|no_stats [when|unless <conditions>]");
	}
	auto begin = arguments.begin();

	if (match_and_skip(arguments, begin, "failed")) {
		result_ = ProgramResult::kFailed;
	} else if (match_and_skip(arguments, begin, "completed")) {
		result_ = ProgramResult::kCompleted;
	} else if (match_and_skip(arguments, begin, "skipped")) {
		result_ = ProgramResult::kSkipped;
	} else if (match_and_skip(arguments, begin, "no_stats")) {
		result_ = ProgramResult::kNone;
	} else {
		throw GameDataError(
		   "Usage: return=failed|completed|skipped|no_stats [when|unless <conditions>]");
	}

	// Parse all arguments starting from the given iterator into our 'conditions_', splitting
	// individual conditions by the given 'separator'
	auto parse_conditions = [this, &descr, &tribes](const std::vector<std::string>& args,
	                                                std::vector<std::string>::const_iterator it,
	                                                const std::string& separator) {
		while (it != args.end()) {
			auto end = it + 1;
			while (end != args.end() && *end != separator) {
				++end;
			}
			if (it == end) {
				throw GameDataError(
				   "Expected: [%s] <condition> after '%s'", separator.c_str(), (it - 1)->c_str());
			}

			conditions_.push_back(create_condition(args, it, end, descr, tribes));
			match_and_skip(args, end, separator);
			it = end;
		}
	};

	is_when_ = true;
	if (begin != arguments.end()) {
		if (match_and_skip(arguments, begin, "when")) {
			parse_conditions(arguments, begin, "and");
		} else if (match_and_skip(arguments, begin, "unless")) {
			is_when_ = false;
			parse_conditions(arguments, begin, "or");
		} else {
			throw GameDataError("Expected when|unless but found '%s'", begin->c_str());
		}
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

ProductionProgram::ActCall::ActCall(const std::vector<std::string>& arguments,
                                    const ProductionSiteDescr& descr) {
	if (arguments.size() < 1 || arguments.size() > 4) {
		throw GameDataError(
		   "Usage: call=<program name> [on failure|completion|skip fail|complete|skip|repeat]");
	}

	//  Initialize with default handling methods.
	handling_methods_[program_result_index(ProgramResult::kFailed)] =
	   ProgramResultHandlingMethod::kContinue;
	handling_methods_[program_result_index(ProgramResult::kCompleted)] =
	   ProgramResultHandlingMethod::kContinue;
	handling_methods_[program_result_index(ProgramResult::kSkipped)] =
	   ProgramResultHandlingMethod::kContinue;

	// Fetch program to call
	const std::string& program_name = arguments.front();
	const ProductionSiteDescr::Programs& programs = descr.programs();
	ProductionSiteDescr::Programs::const_iterator const it = programs.find(program_name);
	if (it == programs.end()) {
		throw GameDataError("The program '%s' has not (yet) been declared in %s "
		                    "(wrong declaration order?)",
		                    program_name.c_str(), descr.name().c_str());
	}
	program_ = it->second.get();

	//  Override with specified handling methods.
	if (arguments.size() > 1) {
		if (arguments.at(1) != "on") {
			throw GameDataError("Expected 'on' keyword in second position");
		}

		ProgramResult result_to_set_method_for;
		if (arguments.at(2) == "failure") {
			if (handling_methods_[program_result_index(ProgramResult::kFailed)] !=
			    ProgramResultHandlingMethod::kContinue) {
				throw GameDataError("%s handling method already defined", "failure");
			}
			result_to_set_method_for = ProgramResult::kFailed;
		} else if (arguments.at(2) == "completion") {
			if (handling_methods_[program_result_index(ProgramResult::kCompleted)] !=
			    ProgramResultHandlingMethod::kContinue) {
				throw GameDataError("%s handling method already defined", "completion");
			}
			result_to_set_method_for = ProgramResult::kCompleted;
		} else if (arguments.at(2) == "skip") {
			if (handling_methods_[program_result_index(ProgramResult::kSkipped)] !=
			    ProgramResultHandlingMethod::kContinue) {
				throw GameDataError("%s handling method already defined", "skip");
			}
			result_to_set_method_for = ProgramResult::kSkipped;
		} else {
			throw GameDataError(
			   "Expected failure|completion|skip after 'on' but found '%s'", arguments.at(2).c_str());
		}

		ProgramResultHandlingMethod handling_method;
		if (arguments.at(3) == "fail") {
			handling_method = ProgramResultHandlingMethod::kFail;
		} else if (arguments.at(3) == "complete") {
			handling_method = ProgramResultHandlingMethod::kComplete;
		} else if (arguments.at(3) == "skip") {
			handling_method = ProgramResultHandlingMethod::kSkip;
		} else if (arguments.at(3) == "repeat") {
			handling_method = ProgramResultHandlingMethod::kRepeat;
		} else {
			throw GameDataError("Expected fail|complete|skip|repeat in final position but found '%s'",
			                    arguments.at(3).c_str());
		}
		handling_methods_[program_result_index(result_to_set_method_for)] = handling_method;
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

ProductionProgram::ActCallWorker::ActCallWorker(const std::vector<std::string>& arguments,
                                                const std::string& production_program_name,
                                                ProductionSiteDescr* descr,
                                                const Tribes& tribes) {
	if (arguments.size() != 1) {
		throw GameDataError("Usage: callworker=<worker program name>");
	}

	program_ = arguments.front();

	//  Quote from "void ProductionSite::program_act(Game &)":
	//  "Always main worker is doing stuff"
	const WorkerDescr& main_worker_descr =
	   *tribes.get_worker_descr(descr->working_positions().front().first);

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

ProductionProgram::ActSleep::ActSleep(const std::vector<std::string>& arguments) {
	if (arguments.size() != 1) {
		throw GameDataError("Usage: sleep=<duration>");
	}
	duration_ = read_positive(arguments.front());
}

void ProductionProgram::ActSleep::execute(Game& game, ProductionSite& ps) const {
	return ps.program_step(game, duration_ ? duration_ : 0, ps.top_state().phase);
}

ProductionProgram::ActCheckMap::ActCheckMap(const std::vector<std::string>& arguments) {
	if (arguments.size() != 1 || arguments.front() != "seafaring") {
		throw GameDataError("Usage: checkmap=seafaring");
	}
	feature_ = Feature::kSeafaring;
}

void ProductionProgram::ActCheckMap::execute(Game& game, ProductionSite& ps) const {
	switch (feature_) {
	case Feature::kSeafaring: {
		if (game.map().allows_seafaring()) {
			return ps.program_step(game, 0);
		} else {
			ps.set_production_result(_("No use for ships on this map!"));
			return ps.program_end(game, ProgramResult::kFailed);
		}
	}
	}
	NEVER_HERE();
}

ProductionProgram::ActAnimate::ActAnimate(const std::vector<std::string>& arguments,
                                          ProductionSiteDescr* descr) {
	parameters = MapObjectProgram::parse_act_animate(arguments, *descr, false);
}

void ProductionProgram::ActAnimate::execute(Game& game, ProductionSite& ps) const {
	ps.start_animation(game, parameters.animation);
	return ps.program_step(
	   game, parameters.duration ? parameters.duration : 0, ps.top_state().phase);
}

ProductionProgram::ActConsume::ActConsume(const std::vector<std::string>& arguments,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	if (arguments.empty()) {
		throw GameDataError(
		   "Usage: consume=<ware or worker>[,<ware or worker>[,...]][:<amount>] ...");
	}
	consumed_wares_workers_ =
	   parse_ware_type_groups(arguments.begin(), arguments.end(), descr, tribes);
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

ProductionProgram::ActProduce::ActProduce(const std::vector<std::string>& arguments,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	if (arguments.empty()) {
		throw GameDataError("Usage: produce=<ware name>[:<amount>] [<ware name>[:<amount>]...]");
	}
	produced_wares_ = parse_bill_of_materials(arguments, WareWorker::wwWARE, descr, tribes);
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

ProductionProgram::ActRecruit::ActRecruit(const std::vector<std::string>& arguments,
                                          const ProductionSiteDescr& descr,
                                          const Tribes& tribes) {
	if (arguments.empty()) {
		throw GameDataError("Usage: recruit=<worker name>[:<amount>] [<worker name>[:<amount>]...]");
	}
	recruited_workers_ = parse_bill_of_materials(arguments, WareWorker::wwWORKER, descr, tribes);
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

ProductionProgram::ActMine::ActMine(const std::vector<std::string>& arguments,
                                    const World& world,
                                    const std::string& production_program_name,
                                    ProductionSiteDescr* descr) {
	if (arguments.size() != 5) {
		throw GameDataError(
		   "Usage: mine=resource <workarea radius> <max> <chance> <worker experience gained>");
	}

	resource_ = world.safe_resource_index(arguments.front().c_str());
	distance_ = read_positive(arguments.at(1));
	max_ = read_positive(arguments.at(2));
	chance_ = read_positive(arguments.at(3));
	training_ = read_positive(arguments.at(4));

	const std::string description = descr->name() + " " + production_program_name + " mine " +
	                                world.get_resource(resource_)->name();
	descr->workarea_info_[distance_].insert(description);
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

ProductionProgram::ActCheckSoldier::ActCheckSoldier(const std::vector<std::string>& arguments) {
	if (arguments.size() != 3) {
		throw GameDataError("Usage: checksoldier=soldier <training attribute> <level>");
	}

	if (arguments.front() != "soldier") {
		throw GameDataError("Expected 'soldier' but found '%s'", arguments.front().c_str());
	}
	attribute_ = parse_training_attribute(arguments.at(1));
	level_ = read_int(arguments.at(2), 0);
}

void ProductionProgram::ActCheckSoldier::execute(Game& game, ProductionSite& ps) const {
	const SoldierControl* ctrl = ps.soldier_control();
	assert(ctrl != nullptr);
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();
	if (soldiers.empty()) {
		ps.set_production_result(_("No soldier to train!"));
		return ps.program_end(game, ProgramResult::kSkipped);
	}
	ps.molog("  Checking soldier (%u) level %d)\n", static_cast<unsigned int>(attribute_),
	         static_cast<unsigned int>(level_));

	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();
	for (std::vector<Soldier*>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
			return ps.program_end(game, ProgramResult::kSkipped);
		}

		if (attribute_ == TrainingAttribute::kHealth) {
			if ((*it)->get_health_level() == level_) {
				break;
			}
		} else if (attribute_ == TrainingAttribute::kAttack) {
			if ((*it)->get_attack_level() == level_) {
				break;
			}
		} else if (attribute_ == TrainingAttribute::kDefense) {
			if ((*it)->get_defense_level() == level_) {
				break;
			}
		} else if (attribute_ == TrainingAttribute::kEvade) {
			if ((*it)->get_evade_level() == level_) {
				break;
			}
		}
	}
	ps.molog("    okay\n");  // okay, do nothing

	upcast(TrainingSite, ts, &ps);
	ts->training_attempted(attribute_, level_);

	ps.molog("  Check done!\n");

	return ps.program_step(game);
}

ProductionProgram::ActTrain::ActTrain(const std::vector<std::string>& arguments) {
	if (arguments.size() != 4) {
		throw GameDataError(
		   "Usage: checksoldier=soldier <training attribute> <level before> <level after>");
	}

	if (arguments.front() != "soldier") {
		throw GameDataError("Expected 'soldier' but found '%s'", arguments.front().c_str());
	}

	attribute_ = parse_training_attribute(arguments.at(1));
	level_ = read_int(arguments.at(2), 0);
	target_level_ = read_positive(arguments.at(3));
}

void ProductionProgram::ActTrain::execute(Game& game, ProductionSite& ps) const {
	const SoldierControl* ctrl = ps.soldier_control();
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();
	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();

	ps.molog("  Training soldier's %u (%d to %d)", static_cast<unsigned int>(attribute_),
	         static_cast<unsigned int>(level_), static_cast<unsigned int>(target_level_));

	bool training_done = false;
	for (auto it = soldiers.begin(); !training_done; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
			return ps.program_end(game, ProgramResult::kSkipped);
		}
		try {
			switch (attribute_) {
			case TrainingAttribute::kHealth:
				if ((*it)->get_health_level() == level_) {
					(*it)->set_health_level(target_level_);
					training_done = true;
				}
				break;
			case TrainingAttribute::kAttack:
				if ((*it)->get_attack_level() == level_) {
					(*it)->set_attack_level(target_level_);
					training_done = true;
				}
				break;
			case TrainingAttribute::kDefense:
				if ((*it)->get_defense_level() == level_) {
					(*it)->set_defense_level(target_level_);
					training_done = true;
				}
				break;
			case TrainingAttribute::kEvade:
				if ((*it)->get_evade_level() == level_) {
					(*it)->set_evade_level(target_level_);
					training_done = true;
				}
				break;
			default:
				throw wexception(
				   "Unknown training attribute index %d", static_cast<unsigned int>(attribute_));
			}
		} catch (...) {
			throw wexception("Fail training soldier!!");
		}
	}

	ps.molog("  Training done!\n");
	ps.set_production_result(
	   /** TRANSLATORS: Success message of a trainingsite '%s' stands for the description of the
	    * training program, e.g. Completed upgrading soldier evade from level 0 to level 1 */
	   (boost::format(_("Completed %s")) % ps.top_state().program->descname()).str());

	upcast(TrainingSite, ts, &ps);
	ts->training_successful(attribute_, level_);

	return ps.program_step(game);
}

ProductionProgram::ActPlaySound::ActPlaySound(const std::vector<std::string>& arguments) {
	parameters = MapObjectProgram::parse_act_play_sound(arguments, kFxPriorityAllowMultiple - 1);
}

void ProductionProgram::ActPlaySound::execute(Game& game, ProductionSite& ps) const {
	Notifications::publish(
	   NoteSound(SoundType::kAmbient, parameters.fx, ps.position_, parameters.priority));
	return ps.program_step(game);
}

ProductionProgram::ActConstruct::ActConstruct(const std::vector<std::string>& arguments,
                                              const std::string& production_program_name,
                                              ProductionSiteDescr* descr) {
	if (arguments.size() != 3) {
		throw GameDataError("Usage: construct=<object name> <worker program> <workarea radius>");
	}
	objectname = arguments.at(0);
	workerprogram = arguments.at(1);
	radius = read_positive(arguments.at(2));

	const std::string description =
	   descr->name() + ' ' + production_program_name + " construct " + objectname;
	descr->workarea_info_[radius].insert(description);
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
   : MapObjectProgram(init_name), descname_(init_descname) {

	for (const std::string& line : actions_table->array_entries<std::string>()) {
		if (line.empty()) {
			throw GameDataError("Empty line");
		}
		try {
			ProgramParseInput parseinput = parse_program_string(line);

			if (parseinput.name == "return") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActReturn(parseinput.arguments, *building, tribes)));
			} else if (parseinput.name == "call") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActCall(parseinput.arguments, *building)));
			} else if (parseinput.name == "sleep") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActSleep(parseinput.arguments)));
			} else if (parseinput.name == "animate") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActAnimate(parseinput.arguments, building)));
			} else if (parseinput.name == "consume") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActConsume(parseinput.arguments, *building, tribes)));
			} else if (parseinput.name == "produce") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActProduce(parseinput.arguments, *building, tribes)));
			} else if (parseinput.name == "recruit") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActRecruit(parseinput.arguments, *building, tribes)));
			} else if (parseinput.name == "callworker") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActCallWorker(parseinput.arguments, name(), building, tribes)));
			} else if (parseinput.name == "mine") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActMine(parseinput.arguments, world, name(), building)));
			} else if (parseinput.name == "checksoldier") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActCheckSoldier(parseinput.arguments)));
			} else if (parseinput.name == "train") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActTrain(parseinput.arguments)));
			} else if (parseinput.name == "playsound") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActPlaySound(parseinput.arguments)));
			} else if (parseinput.name == "construct") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActConstruct(parseinput.arguments, name(), building)));
			} else if (parseinput.name == "checkmap") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActCheckMap(parseinput.arguments)));
			} else {
				throw GameDataError(
				   "Unknown command '%s' in line '%s'", parseinput.name.c_str(), line.c_str());
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
		} catch (const std::exception& e) {
			throw GameDataError("Error reading line '%s': %s", line.c_str(), e.what());
		}
	}

	if (actions_.empty()) {
		throw GameDataError("No actions found");
	}
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
