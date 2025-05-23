/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/production_program.h"

#include <cassert>
#include <memory>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/math.h"
#include "base/wexception.h"
#include "config.h"
#include "economy/economy.h"
#include "economy/ferry_fleet.h"
#include "economy/flag.h"
#include "economy/input_queue.h"
#include "economy/ship_fleet.h"
#include "economy/wares_queue.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker_program.h"
#include "logic/map_objects/world/resource_description.h"
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

/* RST
.. _productionsite_programs:

Productionsite Programs
=======================
Productionsites have :ref:`programs <map_object_programs>` that will be executed by the game
engine. Each productionsite must have a program named ``main``, which will be started automatically
when the productionsite is created in the game, and then repeated until the productionsite is
destroyed.

Programs are defined as Lua tables. Each program must be declared as a subtable in the
productionsite's Lua table called ``programs`` and have a unique table key. The entries in a
program's subtable are the translatable ``descname`` and the table of ``actions`` to execute, like
this::

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            <list of actions>
         }
      },
   },

The translations for ``descname`` can also be fetched by ``pgettext`` to disambiguate. We recommend
that you do this whenever workers are referenced, or if your tribes have multiple wares with the
same name::

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start recruiting soldier because ...
         descname = pgettext("atlanteans_building", "recruiting soldier"),
         actions = {
            <list of actions>
         }
      },
   },

A program can call another program, for example::

   programs = {
      main = {
         -- TRANSLATORS: Completed/Skipped/Did not start working because ...
         descname = _("working"),
         actions = {
            "call=produce_ration",
            "call=produce_snack",
            "return=skipped"
         }
      },
      produce_ration = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a ration because ...
         descname = _("preparing a ration"),
         actions = {
            <list of actions>
         }
      },
      produce_snack = {
         -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
         descname = _("preparing a snack"),
         actions = {
            <list of actions>
         }
      },
   },

A program consists of a sequence of actions. An action is written as
``<type>=<parameters>``::

   produce_snack = {
      -- TRANSLATORS: Completed/Skipped/Did not start preparing a snack because ...
      descname = _("preparing a snack"),
      actions = {
         "return=skipped unless economy needs snack",
         "sleep=duration:2s500ms",
         "consume=barbarians_bread fish,meat beer",
         "playsound=sound/barbarians/taverns/inn 100",
         "animate=working duration:22s",
         "sleep=duration:10s",
         "produce=snack"
      }
   },


.. highlight:: default

For general information about the format, see :ref:`map_object_programs_syntax`.

Available actions are:

- `return`_
- `call`_
- `callworker`_
- `sleep`_
- `animate`_
- `consume`_
- `produce`_
- `recruit`_
- `mine`_
- `checksoldier`_
- `train`_
- `playsound`_
- `script`_
- `construct`_
*/

ProductionProgram::ActReturn::Condition* create_economy_condition(
   const std::string& item, ProductionSiteDescr& descr, const Descriptions& descriptions) {
	try {
		const std::pair<WareWorker, DescriptionIndex> wareworker =
		   descriptions.load_ware_or_worker(item);
		descr.set_infinite_production_useful(true);
		switch (wareworker.first) {
		case WareWorker::wwWARE: {
			descr.ware_demand_checks()->insert(wareworker.second);
			return new ProductionProgram::ActReturn::EconomyNeedsWare(wareworker.second);
		}
		case WareWorker::wwWORKER: {
			descr.worker_demand_checks()->insert(wareworker.second);
			return new ProductionProgram::ActReturn::EconomyNeedsWorker(wareworker.second);
		}
		default:
			NEVER_HERE();
		}
	} catch (const GameDataError& e) {
		throw GameDataError("economy condition: %s", e.what());
	}
}
}  // namespace

bool ProductionProgram::Action::get_building_work(Game& /* game */,
                                                  ProductionSite& /* site */,
                                                  Worker& /* worker */) const {
	return false;
}

void ProductionProgram::Action::building_work_failed(Game& /* game */,
                                                     ProductionSite& /* site */,
                                                     Worker& /* worker */) const {
}

ProductionProgram::Groups
ProductionProgram::parse_ware_type_groups(std::vector<std::string>::const_iterator begin,
                                          std::vector<std::string>::const_iterator end,
                                          const ProductionSiteDescr& descr,
                                          const Descriptions& descriptions) {
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
			DescriptionIndex item_index = descriptions.ware_index(item_name);
			if (!descriptions.ware_exists(item_index)) {
				item_index = descriptions.worker_index(item_name);
				if (descriptions.worker_exists(item_index)) {
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
		result.emplace_back(ware_worker_names, amount);
	}
	if (result.empty()) {
		throw GameDataError("No wares or workers found");
	}
	return result;
}

BillOfMaterials ProductionProgram::parse_bill_of_materials(
   const std::vector<std::string>& arguments, WareWorker ww, Descriptions& descriptions) {
	BillOfMaterials result;
	for (const std::string& argument : arguments) {
		const std::pair<std::string, std::string> produceme = read_key_value_pair(argument, ':', "1");

		const DescriptionIndex index = ww == WareWorker::wwWARE ?
		                                  descriptions.load_ware(produceme.first) :
		                                  descriptions.load_worker(produceme.first);

		result.emplace_back(index, read_positive(produceme.second));
	}
	return result;
}

ProductionProgram::Action::TrainingParameters::TrainingParameters(
   const std::vector<std::string>& arguments, const std::string& action_name) {
	for (const std::string& argument : arguments) {
		const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
		if (item.first == "soldier") {
			if (item.second == "health") {
				attribute = TrainingAttribute::kHealth;
			} else if (item.second == "attack") {
				attribute = TrainingAttribute::kAttack;
			} else if (item.second == "defense") {
				attribute = TrainingAttribute::kDefense;
			} else if (item.second == "evade") {
				attribute = TrainingAttribute::kEvade;
			} else {
				throw GameDataError(
				   "Expected health|attack|defense|evade after 'soldier' but found '%s'",
				   argument.c_str());
			}
		} else if (item.first == "level") {
			level = read_int(item.second, 0);
		} else {
			throw GameDataError(
			   "Unknown argument '%s'. Usage: %s=soldier:attack|defense|evade|health level:<number>",
			   item.first.c_str(), action_name.c_str());
		}
	}
}

/* RST
.. _productionsite_programs_act_return:

return
------

.. function:: return=completed|failed|skipped \[\<\condition\>\]

The ``return`` action determines how the program's result will update the productivity statistics
when any of its steps can't be completed:

* **completed**: Counts as a success for the productivity statistics.
* **failed**: Counts as a failure for the productivity statistics.
* **skipped**: Will be ignored by the productivity statistics.

.. note:: If the execution reaches the end of the program, the return value is implicitly set to
   ``completed``.

If ``condition`` is specified, this will cause an immediate termination of the program if the
condition can't be satisfied. There are two types of conditions, using Boolean arithmetic:

   :when \<statement1\> \[and \<statement2\> ...\]: If any of these statements is false, the program
      will terminate immediately.
   :unless \<statement1\> \[or \<statement2\> ...\]: If none of these statements is true, the
      program will terminate immediately.

The individual statements can also be negated:

   :when not <statement1> and <statement2>: If ``<statement1>`` is true or ``<statement2>`` is
      false, the program will terminate immediately.
   :unless not <statement>: If ``<statement>`` is false, the program will terminate immediately.

The following statements are available:

   :site has <ware_types>|<worker_types>: Checks whether the building's input queues are filled
      with a certain amount of wares/workers. A ware or worker type may only appear once in the
      command. You can specify more than one ware. For example, ``site has fish,meat:2`` is true if
      the building has at least 1 fish or if the building has at least 2 meat.
   :workers need experience: This is true if a worker working at the building can currently gain
      some experience.
   :economy needs <ware_type>|<worker_type>: The result of this condition depends on
      whether the economy that this productionsite belongs to needs a ware or worker of the
      specified type. How this is determined is defined by the economy. A ware or worker type may
      only appear once in the command.

Examples for ``return=failed``:

.. code-block:: lua

   -- If the building has no 'ax_sharp' in its input queues, the program will fail immediately.
   return=failed unless site has ax_sharp

   -- If the building has less than 2 items of 'barbarians_bread' in its input queues, the program
   -- will fail immediately.
   return=failed unless site has barbarians_bread:2

   -- The building needs 1 item of 'bread_frisians', 'beer', 'smoked_fish' or 'smoked_meat' in its
   -- input queues. Otherwise, the program will fail immediately.
   return=failed unless site has bread_frisians,beer,smoked_fish,smoked_meat

   -- The building needs 1 item of 'fish' or 2 items of 'meat' in its input queues. Otherwise, the
   -- program will fail immediately.
   return=failed unless site has fish,meat:2

Examples for ``return=skipped``:

.. code-block:: lua

   -- If any subsequent step fails, don't cause a hit on the statistics.
   return=skipped

   -- Only run this program if the economy needs an 'ax_sharp'.
   return=skipped unless economy needs ax_sharp

   -- Only run this program if the economy needs a 'beer' or if any of the workers working at the
   -- building can gain more experience.
   return=skipped unless economy needs beer or workers need experience

   -- Only run this program if the economy needs at least one of the listed wares: 'clay', 'fish' or
   -- 'coal'.
   return=skipped unless economy needs clay or economy needs fish or economy needs coal

   -- Only run this program if the economy needs at least one of the listed wares: 'iron' or 'gold'.
   -- If the economy has sufficient 'coal', run anyway even if none of these two wares are needed.
   return=skipped unless economy needs iron or economy needs gold or not economy needs coal

   -- If the building has no 'fur_garment_old' in its input queues, skip running this program.
   return=skipped unless site has fur_garment_old

   -- If the building has 'wheat' in its input queue and if the economy needs the ware
   -- 'flour' but does not need the ware 'cornmeal', skip running this program so we can run another
   -- program for producing 'flour' rather than 'cornmeal'.
   return=skipped when site has wheat and economy needs flour and not economy needs cornmeal

   -- If the building has at least 2 items of 'fish' in its input queues and if the economy needs
   -- any 'smoked_fish', skip running this program because we will want to produce some
   -- 'smoked_fish' instead of whatever this program produces.
   return=skipped when site has fish:2 and economy needs smoked_fish

   -- If the building has at least one item of 'fruit' or 'bread_frisians' and at least one item of
   -- 'smoked_fish' or 'smoked_meat', skip running this program. We want to do something more useful
   -- with these wares with another program.
   return=skipped when site has fruit,bread_frisians and site has smoked_fish,smoked_meat
*/

ProductionProgram::ActReturn::Negation::Negation(const std::vector<std::string>& arguments,
                                                 std::vector<std::string>::const_iterator& begin,
                                                 std::vector<std::string>::const_iterator& end,
                                                 ProductionSiteDescr& descr,
                                                 const Descriptions& descriptions)
   : operand(create_condition(arguments, begin, end, descr, descriptions)) {
}

ProductionProgram::ActReturn::Negation::~Negation() {
	delete operand;
}
bool ProductionProgram::ActReturn::Negation::evaluate(const ProductionSite& ps) const {
	return !operand->evaluate(ps);
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string ProductionProgram::ActReturn::Negation::description(const Descriptions& t) const {
	return operand->description_negation(t);
}

// Just a dummy to satisfy the superclass interface. Returns an empty string.
std::string
ProductionProgram::ActReturn::Negation::description_negation(const Descriptions& t) const {
	return operand->description(t);
}

bool ProductionProgram::ActReturn::EconomyNeedsWare::evaluate(const ProductionSite& ps) const {
	return ps.infinite_production() ||
	       ps.get_economy(wwWARE)->needs_ware_or_worker(ware_type, &ps.base_flag());
}
std::string ProductionProgram::ActReturn::EconomyNeedsWare::description(
   const Descriptions& descriptions) const {
	std::string result = format(
	   /** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy needs the ware
	    * '%s' */
	   _("the economy needs the ware ‘%s’"), descriptions.get_ware_descr(ware_type)->descname());
	return result;
}
std::string ProductionProgram::ActReturn::EconomyNeedsWare::description_negation(
   const Descriptions& descriptions) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy doesn't need the
	 * ware '%s' */
	std::string result = format(_("the economy doesn’t need the ware ‘%s’"),
	                            descriptions.get_ware_descr(ware_type)->descname());
	return result;
}

bool ProductionProgram::ActReturn::EconomyNeedsWorker::evaluate(const ProductionSite& ps) const {
	return ps.infinite_production() ||
	       ps.get_economy(wwWORKER)->needs_ware_or_worker(worker_type, &ps.base_flag());
}
std::string ProductionProgram::ActReturn::EconomyNeedsWorker::description(
   const Descriptions& descriptions) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... because the economy needs the worker
	 * '%s' */
	std::string result = format(_("the economy needs the worker ‘%s’"),
	                            descriptions.get_worker_descr(worker_type)->descname());
	return result;
}

std::string ProductionProgram::ActReturn::EconomyNeedsWorker::description_negation(
   const Descriptions& descriptions) const {
	/** TRANSLATORS: e.g. Completed/Skipped/Did not start ... */
	/** TRANSLATORS:      ... because the economy doesn’t need the worker '%s' */
	std::string result = format(_("the economy doesn’t need the worker ‘%s’"),
	                            descriptions.get_worker_descr(worker_type)->descname());
	return result;
}

ProductionProgram::ActReturn::SiteHas::SiteHas(std::vector<std::string>::const_iterator begin,
                                               std::vector<std::string>::const_iterator end,
                                               const ProductionSiteDescr& descr,
                                               const Descriptions& descriptions) {
	try {
		group = parse_ware_type_groups(begin, end, descr, descriptions).front();
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
				if (count <= filled) {
					return true;
				}
				count -= filled;
				break;
			}
		}
	}
	return false;
}

std::string
ProductionProgram::ActReturn::SiteHas::description(const Descriptions& descriptions) const {
	std::vector<std::string> condition_list(group.first.size());
	size_t i = 0;
	for (const auto& entry : group.first) {
		condition_list.at(i++) =
		   (entry.second == wwWARE ? descriptions.get_ware_descr(entry.first)->descname() :
		                             descriptions.get_worker_descr(entry.first)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		condition =
		   /** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		   /** TRANSLATORS:    %1$i = "3" */
		   /** TRANSLATORS:    %2$s = "water" */
		   format(_("%1$ix %2$s"), static_cast<unsigned int>(group.second), condition);
	}

	std::string result =
	   /** TRANSLATORS: %s is a list of wares*/
	   format(_("the building has the following wares: %s"), condition);
	return result;
}

std::string ProductionProgram::ActReturn::SiteHas::description_negation(
   const Descriptions& descriptions) const {
	std::vector<std::string> condition_list(group.first.size());
	size_t i = 0;
	for (const auto& entry : group.first) {
		condition_list.at(i++) =
		   (entry.second == wwWARE ? descriptions.get_ware_descr(entry.first)->descname() :
		                             descriptions.get_worker_descr(entry.first)->descname());
	}
	std::string condition = i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);
	if (1 < group.second) {
		condition =
		   /** TRANSLATORS: This is an item in a list of wares, e.g. "3x water": */
		   /** TRANSLATORS:    %1$i = "3" */
		   /** TRANSLATORS:    %2$s = "water" */
		   format(_("%1$ix %2$s"), static_cast<unsigned int>(group.second), condition);
	}

	std::string result =
	   /** TRANSLATORS: %s is a list of wares*/
	   format(_("the building doesn’t have the following wares: %s"), condition);
	return result;
}

bool ProductionProgram::ActReturn::WorkersNeedExperience::evaluate(const ProductionSite& ps) const {
	const std::vector<ProductionSite::WorkingPosition>& wp = ps.working_positions_;
	for (uint32_t i = ps.descr().nr_working_positions(); i != 0u;) {
		if (wp.at(--i).worker.get(ps.get_owner()->egbase())->needs_experience()) {
			return true;
		}
	}
	return false;
}
std::string ProductionProgram::ActReturn::WorkersNeedExperience::description(
   const Descriptions& /* descriptions */) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because a worker needs experience'. */
	return _("a worker needs experience");
}

std::string ProductionProgram::ActReturn::WorkersNeedExperience::description_negation(
   const Descriptions& /* descriptions */) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because the workers need no experience'. */
	return _("the workers need no experience");
}

bool ProductionProgram::ActReturn::FleetNeeds::evaluate(const ProductionSite& ps) const {
	if (type_ == Type::kShip) {
		for (ShipFleetYardInterface* interface : ps.get_ship_fleet_interfaces()) {
			if (interface->get_fleet()->lacks_ship() || ps.infinite_production()) {
				BaseImmovable* immo = interface->get_position().field->get_immovable();
				if (immo == nullptr || immo->get_size() == BaseImmovable::Size::NONE) {
					return true;
				}
			}
		}
	} else {
		for (FerryFleetYardInterface* interface : ps.get_ferry_fleet_interfaces()) {
			if (interface->get_fleet()->lacks_ferry() || ps.infinite_production()) {
				BaseImmovable* immo = interface->get_position().field->get_immovable();
				if (immo == nullptr || immo->get_size() == BaseImmovable::Size::NONE) {
					return true;
				}
			}
		}
	}

	return false;
}
std::string ProductionProgram::ActReturn::FleetNeeds::description(
   const Descriptions& /* descriptions */) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because the fleet needs a ship/ferry'. */
	return type_ == Type::kShip ? _("the fleet needs a ship") : _("the fleet needs a ferry");
}

std::string ProductionProgram::ActReturn::FleetNeeds::description_negation(
   const Descriptions& /* descriptions */) const {
	/** TRANSLATORS: 'Completed/Skipped/Did not start ... because the fleet needs no ships/ferries'.
	 */
	return type_ == Type::kShip ? _("the fleet needs no ships") : _("the fleet needs no ferries");
}

ProductionProgram::ActReturn::Condition*
ProductionProgram::ActReturn::create_condition(const std::vector<std::string>& arguments,
                                               std::vector<std::string>::const_iterator& begin,
                                               std::vector<std::string>::const_iterator& end,
                                               ProductionSiteDescr& descr,
                                               const Descriptions& descriptions) {
	if (begin == end) {
		throw GameDataError("Expected a condition after '%s'", (begin - 1)->c_str());
	}
	try {
		if (match_and_skip(arguments, begin, "not")) {
			return new ActReturn::Negation(arguments, begin, end, descr, descriptions);
		}
		if (match_and_skip(arguments, begin, "economy")) {
			if (!match_and_skip(arguments, begin, "needs")) {
				throw GameDataError("Expected 'needs' after 'economy' but found '%s'", begin->c_str());
			}
			return create_economy_condition(*begin, descr, descriptions);
		}
		if (match_and_skip(arguments, begin, "site")) {
			if (!match_and_skip(arguments, begin, "has")) {
				throw GameDataError("Expected 'has' after 'site' but found '%s'", begin->c_str());
			}
			return new ProductionProgram::ActReturn::SiteHas(begin, end, descr, descriptions);
		}
		if (match_and_skip(arguments, begin, "workers")) {
			if (!match_and_skip(arguments, begin, "need")) {
				throw GameDataError(
				   "Expected 'need experience' after 'workers' but found '%s'", begin->c_str());
			}
			if (!match_and_skip(arguments, begin, "experience")) {
				throw GameDataError(
				   "Expected 'experience' after 'workers need' but found '%s'", begin->c_str());
			}
			return new ProductionProgram::ActReturn::WorkersNeedExperience();
		}
		if (match_and_skip(arguments, begin, "fleet")) {
			if (!match_and_skip(arguments, begin, "needs")) {
				throw GameDataError(
				   "Expected 'needs ship|ferry' after 'fleet' but found '%s'", begin->c_str());
			}
			descr.set_infinite_production_useful(true);
			if (match_and_skip(arguments, begin, "ship")) {
				descr.has_ship_fleet_check_ = true;
				return new ProductionProgram::ActReturn::FleetNeeds(
				   ProductionProgram::ActReturn::FleetNeeds::Type::kShip);
			}
			if (match_and_skip(arguments, begin, "ferry")) {
				descr.has_ferry_fleet_check_ = true;
				return new ProductionProgram::ActReturn::FleetNeeds(
				   ProductionProgram::ActReturn::FleetNeeds::Type::kFerry);
			}
			throw GameDataError("Expected 'ship' or 'ferry' after 'fleet needs' but found '%s'",
			                    begin == end ? "" : begin->c_str());
		}
		throw GameDataError("Expected not|economy|site|workers after '%s' but found '%s'",
		                    (begin - 1)->c_str(), begin->c_str());
	} catch (const WException& e) {
		throw GameDataError("Invalid condition. %s", e.what());
	}
}

ProductionProgram::ActReturn::ActReturn(const std::vector<std::string>& arguments,
                                        ProductionSiteDescr& descr,
                                        const Descriptions& descriptions) {
	if (arguments.empty()) {
		throw GameDataError("Usage: return=failed|completed|skipped [when|unless <conditions>]");
	}
	auto begin = arguments.begin();

	if (match_and_skip(arguments, begin, "failed")) {
		result_ = ProgramResult::kFailed;
	} else if (match_and_skip(arguments, begin, "completed")) {
		result_ = ProgramResult::kCompleted;
	} else if (match_and_skip(arguments, begin, "skipped")) {
		result_ = ProgramResult::kSkipped;
	} else {
		throw GameDataError("Usage: return=failed|completed|skipped [when|unless <conditions>]");
	}

	// Parse all arguments starting from the given iterator into our 'conditions_', splitting
	// individual conditions by the given 'separator'
	auto parse_conditions = [this, &descr, &descriptions](
	                           const std::vector<std::string>& args,
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

			conditions_.push_back(create_condition(args, it, end, descr, descriptions));
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
				condition_list.push_back(condition->description(game.descriptions()));
			}
		} else {  //  "unless a or b or ..." (all conditions must be false)
			for (const Condition* condition : conditions_) {
				if (condition->evaluate(ps)) {    //  A condition is true,
					return ps.program_step(game);  //  continue program.
				}
				condition_list.push_back(condition->description_negation(game.descriptions()));
			}
		}
		std::string condition_string =
		   i18n::localize_list(condition_list, i18n::ConcatenateWith::AND);

		std::string result_string;
		switch (result_) {
		case ProgramResult::kFailed: {
			/** TRANSLATORS: "Did not start working because the economy needs the ware '%s'" */
			result_string = format(_("Did not start %1$s because %2$s"),
			                       ps.top_state().program->descname(), condition_string);
		} break;
		case ProgramResult::kCompleted: {
			result_string = format(
			   /** TRANSLATORS: "Completed working because the economy needs the ware '%s'" */
			   _("Completed %1$s because %2$s"), ps.top_state().program->descname(), condition_string);
		} break;
		case ProgramResult::kSkipped:
		case ProgramResult::kNone: {
			// TODO(GunChleoc): kNone same as kSkipped - is this on purpose?
			result_string = format(
			   /** TRANSLATORS: "Skipped working because the economy needs the ware '%s'" */
			   _("Skipped %1$s because %2$s"), ps.top_state().program->descname(), condition_string);
		} break;
		default:
			NEVER_HERE();
		}
		if (ps.production_result() != ps.descr().out_of_resource_heading() ||
		    ps.descr().out_of_resource_heading().empty()) {
			ps.set_production_result(result_string);
		}
	}
	return ps.program_end(game, result_);
}

/* RST
call
----
.. function:: call=\<program_name\> \[on failure|completion|skip fail|complete|skip|repeat\]

   :arg string program_name: The name of a :ref:`program <productionsite_programs>`
      defined in this productionsite.

   :arg string on: Defines what to do if the program fails, completes or skips.

      * ``complete``: The failure is ignored, and the program returns as successful.
      * ``fail``: The command fails, with the same effect as executing the :ref:`return program
        <productionsite_programs_act_return>` with ``return=failed``.
      * ``repeat``: The command is repeated.
      * ``skip``: The failure is ignored, and the program is continued. This is the default setting
        if ``on`` is ommitted.

Calls another program of the same productionsite. Example:

.. code-block:: lua

      -- Productionsite program that will mine marble 1 out of 3 times
      programs = {
         main = {
            -- TRANSLATORS: Completed/Skipped/Did not start working because ...
            descname = _("working"),
            actions = {
               "call=mine_granite on failure fail",
               "call=mine_granite on failure fail",
               "call=mine_marble on failure fail",
            }
         },
         mine_granite = {
            -- TRANSLATORS: Completed/Skipped/Did not start quarrying granite because ...
            descname = _("quarrying granite"),
            actions = {
               "callworker=cut_granite",
               "sleep=duration:17s500ms"
            }
         },
         mine_marble = {
            -- TRANSLATORS: Completed/Skipped/Did not start quarrying marble because ...
            descname = _("quarrying marble"),
            actions = {
               "callworker=cut_marble",
               "sleep=duration:17s500ms"
            }
         },
      },
*/
ProductionProgram::ActCall::ActCall(const std::vector<std::string>& arguments) {
	if (arguments.empty() || arguments.size() > 4) {
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
	program_name_ = arguments.front();

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

	assert(!program_name_.empty());
}

void ProductionProgram::ActCall::execute(Game& game, ProductionSite& ps) const {
	ProgramResult const program_result = ps.top_state().phase;

	if (program_result == ProgramResult::kNone) {  //  The program has not yet been called.
		return ps.program_start(game, program_name_);
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
		ps.program_time_ = ps.schedule_act(game, Duration(10));
		break;
	default:
		NEVER_HERE();
	}
}

/* RST
callworker
----------
.. function:: callworker=\<worker_program_name\> \[on failure fail|complete|skip\]

   :arg string worker_program_name: The name of a :ref:`worker program <tribes_worker_programs>`
      defined in the productionsite's main :ref:`worker <lua_tribes_basic_workers>`.

   :arg string on: Defines what to do if the worker program fails. The production program
      is always terminated immediately when ``callworker`` fails; this parameter specifies
      what result status the production program should report. Default is ``fail``.

Calls a program of the productionsite's main worker. Example:

.. code-block:: lua

      -- Productionsite program actions
      actions = {
         -- Send the farmer out to harvest wheat from a wheat field
         "callworker=harvest",
         "animate=working duration:3s",
         "sleep=duration:1s"
      }

      -- Corresponding worker program for harvesting wheat from a wheat field
      harvest = {
         "findobject=attrib:ripe_wheat radius:2",
         "walk=object",
         "playsound=sound/farm/scythe priority:70% allow_multiple",
         "animate=harvest duration:10s",
         "callobject=harvest",
         "animate=gather duration:4s",
         "createware=wheat",
         "return"
      }
*/
ProductionProgram::ActCallWorker::ActCallWorker(const std::vector<std::string>& arguments,
                                                const std::string& production_program_name,
                                                ProductionSiteDescr* descr,
                                                const Descriptions& descriptions) {
	const size_t nr_args = arguments.size();
	if (nr_args != 1 && nr_args != 4) {
		throw GameDataError(
		   "Usage: callworker=<worker_program_name> [on failure fail|complete|skip]");
	}

	program_ = arguments.front();

	if (nr_args > 1) {
		if (arguments.at(1) != "on" || arguments.at(2) != "failure") {
			throw GameDataError("Expected 'on failure' after worker program name");
		}
		if (arguments.at(3) == "fail") {
			on_failure_ = ProgramResult::kFailed;
		} else if (arguments.at(3) == "complete") {
			on_failure_ = ProgramResult::kCompleted;
		} else if (arguments.at(3) == "skip") {
			on_failure_ = ProgramResult::kSkipped;
		} else {
			throw GameDataError("Expected fail|complete|skip in final position but found '%s'",
			                    arguments.at(3).c_str());
		}
	}

	//  Quote from "void ProductionSite::program_act(Game &)":
	//  "Always main worker is doing stuff"
	const WorkerDescr& main_worker_descr =
	   *descriptions.get_worker_descr(descr->working_positions().front().first);

	WorkerProgram const* workerprogram = main_worker_descr.get_program(program_);

	//  This will fail unless the main worker has a program with the given
	//  name, so it also validates the parameter.
	const WorkareaInfo& worker_workarea_info = workerprogram->get_workarea_info();

	// Add to building outputs for help and AI
	for (const DescriptionIndex produced_ware : workerprogram->produced_ware_types()) {
		descr->add_output_ware_type(produced_ware);
	}

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

	for (const auto& attribute_info : workerprogram->needed_attributes()) {
		descr->add_needed_attribute(attribute_info);
	}
	for (const auto& attribute_info : workerprogram->collected_attributes()) {
		descr->add_collected_attribute(attribute_info);
	}
	for (const auto& attribute_info : workerprogram->created_attributes()) {
		descr->add_created_attribute(attribute_info);
	}
	for (const std::string& resourcename : workerprogram->collected_resources()) {
		// Workers always collect 100% of the resource, and then find no more
		descr->add_collected_resource(resourcename, 100, 0);
	}
	for (const std::string& resourcename : workerprogram->created_resources()) {
		descr->add_created_resource(resourcename);
	}
	for (const std::string& bobname : workerprogram->created_bobs()) {
		descr->add_created_bob(bobname);
	}

	const DescriptionMaintainer<ImmovableDescr>& all_immovables = descriptions.immovables();

	for (const auto& object_info : workerprogram->needed_named_map_objects()) {
		// Add needed entities
		if (object_info.first == MapObjectType::IMMOVABLE) {
			descr->add_needed_immovable(object_info.second);
		}
	}
	for (const auto& object_info : workerprogram->collected_named_map_objects()) {
		const MapObjectType mapobjecttype = object_info.first;

		// Add collected entities
		switch (mapobjecttype) {
		case MapObjectType::IMMOVABLE: {
			descr->add_collected_immovable(object_info.second);
			const Widelands::DescriptionIndex immo_id = all_immovables.get_index(object_info.second);
			const ImmovableDescr& immovable_descr = all_immovables.get(immo_id);
			const_cast<ImmovableDescr&>(immovable_descr).add_collected_by(descriptions, descr->name());
		} break;
		case MapObjectType::BOB: {
			descr->add_collected_bob(object_info.second);
		} break;
		default:
			NEVER_HERE();
		}
	}
}

void ProductionProgram::ActCallWorker::execute(Game& game, ProductionSite& ps) const {
	// Always main worker is doing stuff
	ps.working_positions_.at(ps.main_worker_).worker.get(game)->update_task_buildingwork(game);
}

bool ProductionProgram::ActCallWorker::get_building_work(Game& game,
                                                         ProductionSite& psite,
                                                         Worker& worker) const {
	ProductionSite::State& state = psite.top_state();
	if (state.phase == ProgramResult::kNone) {
		worker.start_task_program(game, program());
		if ((state.flags & ProductionSite::State::StateFlags::kStateFlagHasExtraData) != 0u) {
			worker.top_state().objvar1 = state.objvar;
		}
		state.phase = ProgramResult::kFailed;
		return true;
	}
	psite.program_step(game);
	return false;
}

void ProductionProgram::ActCallWorker::building_work_failed(Game& game,
                                                            ProductionSite& psite,
                                                            Worker& /* worker */) const {
	psite.program_end(game, on_failure_);
}

/* RST
sleep
-----
.. function:: sleep=duration:\<duration\>

   :arg duration duration: The time :ref:`map_object_programs_datatypes_duration` for which the
      program will wait before continuing on to the next action. If ``0``, the result from the most
      recent command that returned a value is used.

Blocks the execution of the program for the specified duration. Example:

.. code-block:: lua

      actions = {
         "consume=ration",
         -- Do nothing for 30 seconds
         "sleep=duration:30s",
         "callworker=scout"
      }
*/
ProductionProgram::ActSleep::ActSleep(const std::vector<std::string>& arguments) {
	if (arguments.size() != 1) {
		throw GameDataError("Usage: sleep=duration:<duration>");
	}
	const std::pair<std::string, std::string> item = read_key_value_pair(arguments.front(), ':');
	if (item.first == "duration") {
		duration_ = read_duration(item.second);
	} else {
		throw GameDataError(
		   "Unknown argument '%s'. Usage: duration:<duration>", arguments.front().c_str());
	}
}

void ProductionProgram::ActSleep::execute(Game& game, ProductionSite& ps) const {
	return ps.program_step(
	   game, duration_.get() != 0u ? duration_ : Duration(0), ps.top_state().phase);
}

/* RST
animate
-------
Runs an animation. See :ref:`map_object_programs_animate`.
*/
ProductionProgram::ActAnimate::ActAnimate(const std::vector<std::string>& arguments,
                                          ProductionSiteDescr* descr)
   : parameters(MapObjectProgram::parse_act_animate(arguments, *descr, false)) {
}

void ProductionProgram::ActAnimate::execute(Game& game, ProductionSite& ps) const {
	ps.start_animation(game, parameters.animation);
	return ps.program_step(game, parameters.duration.get() != 0u ? parameters.duration : Duration(0),
	                       ps.top_state().phase);
}

/* RST
consume
-------
.. function:: consume=ware_name\{,ware_name\}\[:count\] \[ware_name\{,ware_name\}\[:amount\]\]...\]

   :arg string ware_name: a list of :ref:`ware types <lua_tribes_wares>` to choose from for
      consumption. A ware type may only appear once in the command.

   :arg int amount: The amount of wares of the chosen type to consume. A positive integer. If
      omitted, the value ``1`` is used.

Consumes wares from the input storages. For each ware group, the number of wares specified in
``amount`` is consumed. The consumed wares may be of any type in the group.

If there are not enough wares in the input storages, the command fails (with the same effect as
executing ``return=failed``). Then no wares will be consumed.

Selecting which ware types to consume for a group so that the whole command succeeds is a constraint
satisfaction problem. The implementation does not implement an exhaustive search for a solution to
it. It is just a greedy algorithm which gives up instead of backtracking. Therefore the command may
fail even if there is a solution.

However it may be possible to help the algorithm by ordering the groups carefully. Suppose that the
input storage has the wares ``a:1, b:1`` and a consume command has the parameters ``a,b:1 a:1``. The
algorithm tries to consume its input wares in order. It starts with the first group and consumes 1
ware of type ``a`` (the group becomes satisfied). Then it proceeds with the second group, but there
are no wares of type ``a`` left to consume. Since there is no other ware type that can satisfy the
group, the command will fail. If the groups are reordered so that the parameters become ``a:1
a,b:1``, it will work. The algorithm will consume 1 ware of type ``a`` for the first group. When it
proceeds with the second group, it will not have any wares of type ``a`` left. Then it will go on
and consume 1 ware of type ``b`` for the second group (which becomes satisfied) and the command
succeeds.

.. note:: It is not possible to reorder ware types within a group. ``a,b`` is equivalent to ``b,a``
    because in the internal representation the ware types of a group are sorted.

Examples:

.. code-block:: lua

      actions = {
         "return=skipped unless economy needs shield_advanced",
         -- Try to consume 2x iron, then 2x coal, then 1x gold
         "consume=iron:2 coal:2 gold",
         "sleep=duration:32s",
         "animate=working duration:45s",
         "produce=shield_advanced"
      },

      actions = {
         "checksoldier=soldier:evade level:0",
         "return=failed unless site has empire_bread",
         "return=failed unless site has fish,meat",
         "sleep=duration:30s",
         "checksoldier=soldier:evade level:0",
         -- Try to consume 1x empire_bread, then 1x fish or 1x meat
         "consume=empire_bread fish,meat",
         "train=soldier:evade level:1"
      }
*/
ProductionProgram::ActConsume::ActConsume(const std::vector<std::string>& arguments,
                                          const ProductionSiteDescr& descr,
                                          const Descriptions& descriptions) {
	if (arguments.empty()) {
		throw GameDataError(
		   "Usage: consume=<ware or worker>[,<ware or worker>[,...]][:<amount>] ...");
	}
	consumed_wares_workers_ =
	   parse_ware_type_groups(arguments.begin(), arguments.end(), descr, descriptions);
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
			if (!found) {
				++it;
			}
		}
	}

	// "Did not start working because .... is/are missing"
	if (uint8_t const nr_missing_groups = l_groups.size()) {
		const TribeDescr& tribe = ps.owner().tribe();

		std::vector<std::string> group_list;
		for (const auto& group : l_groups) {
			assert(!group.first.empty());

			std::vector<std::string> ware_list(group.first.size());
			size_t i = 0;
			for (const auto& entry : group.first) {
				ware_list.at(i++) =
				   (entry.second == wwWARE ? tribe.get_ware_descr(entry.first)->descname() :
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
				   format(_("%1$ix %2$s"), static_cast<unsigned int>(count), ware_string);
			}
			group_list.push_back(ware_string);
		}

		const std::string is_missing_string =
		   /** TRANSLATORS: e.g. 'Did not start working because 3x water and 3x wheat are missing' */
		   /** TRANSLATORS: e.g. 'Did not start working because fish, meat or pitta bread is missing'
		    */
		   format(ngettext("%s is missing", "%s are missing", nr_missing_groups),
		          i18n::localize_list(group_list, i18n::ConcatenateWith::AND));

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
		   format(_("Did not start %1$s because %2$s"), ps.top_state().program->descname(),
		          is_missing_string);

		if (ps.production_result() != ps.descr().out_of_resource_heading() ||
		    ps.descr().out_of_resource_heading().empty()) {
			ps.set_production_result(result_string);
		}
		return ps.program_end(game, ProgramResult::kFailed);
	}
	//  we fulfilled all consumption requirements
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

/* RST
produce
-------
.. function:: produce=\<ware_name\>\[:\<amount\>\] \[\<ware_name\>\[:\<amount\>\]...\]

   :arg string ware_name: The name of a :ref:`ware type <lua_tribes_wares>`. A ware
      type may only appear once in the command.

   :arg int amount: The amount of wares of this type to produce. A positive integer. If omitted,
      the value ``1`` is used.

Produces wares. For each group, the number of wares specified in ``amount`` is produced and then
placed on the building's flag to be carried where they are needed. The produced wares are of the
type specified by
``ware_name`` in the group. Example:

.. code-block:: lua

      actions = {
         "return=skipped unless economy needs fur",
         "consume=barley water",
         "sleep=duration:15s",
         "animate=working duration:20s",
         -- Produce 2x fur and 1x meat
         "produce=fur:2 meat"
      }
*/
ProductionProgram::ActProduce::ActProduce(const std::vector<std::string>& arguments,
                                          ProductionSiteDescr& descr,
                                          Descriptions& descriptions) {
	if (arguments.empty()) {
		throw GameDataError("Usage: produce=<ware name>[:<amount>] [<ware name>[:<amount>]...]");
	}
	produced_wares_ = parse_bill_of_materials(arguments, WareWorker::wwWARE, descriptions);

	// Add to building outputs for help and AI
	for (auto& produced_ware : produced_wares_) {
		descr.add_output_ware_type(produced_ware.first);
	}
}

void ProductionProgram::ActProduce::execute(Game& game, ProductionSite& ps) const {
	assert(ps.produced_wares_.empty());
	ps.produced_wares_ = produced_wares_;
	ps.working_positions_.at(ps.main_worker_).worker.get(game)->update_task_buildingwork(game);

	const TribeDescr& tribe = ps.owner().tribe();
	assert(!produced_wares_.empty());

	std::vector<std::string> ware_descnames;
	uint8_t count = 0;
	for (const auto& item_pair : produced_wares_) {
		count += item_pair.second;
		std::string ware_descname = tribe.get_ware_descr(item_pair.first)->descname();
		if (1 < item_pair.second || 1 < produced_wares_.size()) {
			ware_descname = format(
			   /** TRANSLATORS: This is an item in a list of wares, e.g. "Produced 2x Coal": */
			   /** TRANSLATORS:    %1$i = "2" */
			   /** TRANSLATORS:    %2$s = "Coal" */
			   _("%1$i× %2$s"), static_cast<unsigned int>(item_pair.second), ware_descname);
		}
		ware_descnames.push_back(ware_descname);
	}
	std::string ware_list = i18n::localize_list(ware_descnames, i18n::ConcatenateWith::AND);

	const std::string result_string =
	   /** TRANSLATORS: %s is a list of wares. String is fetched according to total amount of
	      wares. */
	   format(ngettext("Produced %s", "Produced %s", count), ware_list);
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

/* RST
recruit
-------
.. function:: recruit=\<worker_name\>\[:\<amount\>\] \[\<worker_name\>\[:\<amount\>\]...\]

   :arg string worker_name: The name of a :ref:`worker type <lua_tribes_basic_workers>`. A worker
      type may only appear once in the command.

   :arg int amount: The amount of workers of this type to create. A positive integer. If omitted,
      the value ``1`` is used.

Produces workers. For each group, the number of workers specified in ``amount`` is produced, which
then leave the site looking for employment. The produced workers are of the type specified by
``worker_name`` in the group. Example:

.. code-block:: lua

      actions = {
         "return=skipped unless economy needs atlanteans_horse",
         "consume=corn water",
         "sleep=duration:15s",
         "playsound=sound/farm/horse priority:50% allow_multiple",
         "animate=working duration:15s",
         -- Create 2 horses
         "recruit=atlanteans_horse:2"
      }
*/
ProductionProgram::ActRecruit::ActRecruit(const std::vector<std::string>& arguments,
                                          ProductionSiteDescr& descr,
                                          Descriptions& descriptions) {
	if (arguments.empty()) {
		throw GameDataError("Usage: recruit=<worker_name>[:<amount>] [<worker_name>[:<amount>]...]");
	}
	recruited_workers_ = parse_bill_of_materials(arguments, WareWorker::wwWORKER, descriptions);

	// Add to building outputs for help and AI
	for (auto& recruited_worker : recruited_workers_) {
		descr.add_output_worker_type(recruited_worker.first);
	}
}

void ProductionProgram::ActRecruit::execute(Game& game, ProductionSite& ps) const {
	assert(ps.recruited_workers_.empty());
	ps.recruited_workers_ = recruited_workers_;
	ps.working_positions_.at(ps.main_worker_).worker.get(game)->update_task_buildingwork(game);

	const TribeDescr& tribe = ps.owner().tribe();
	assert(!recruited_workers_.empty());
	std::vector<std::string> worker_descnames;
	uint8_t count = 0;
	for (const auto& item_pair : recruited_workers_) {
		count += item_pair.second;
		std::string worker_descname = tribe.get_worker_descr(item_pair.first)->descname();
		if (1 < item_pair.second || 1 < recruited_workers_.size()) {
			worker_descname = format(
			   /** TRANSLATORS: This is an item in a list of workers, e.g. "Recruited 2x Ox": */
			   /** TRANSLATORS:    %1$i = "2" */
			   /** TRANSLATORS:    %2$s = "Ox" */
			   _("%1$ix %2$s"), static_cast<unsigned int>(item_pair.second), worker_descname);
		}
		worker_descnames.push_back(worker_descname);
	}
	std::string unit_string = i18n::localize_list(worker_descnames, i18n::ConcatenateWith::AND);

	const std::string result_string =
	   /** TRANSLATORS: %s is a list of workers. String is fetched according to total amount of
	      workers. */
	   format(ngettext("Recruited %s", "Recruited %s", count), unit_string);
	ps.set_production_result(result_string);
}

bool ProductionProgram::ActRecruit::get_building_work(Game& game,
                                                      ProductionSite& psite,
                                                      Worker& /* worker */) const {
	// We reach this point once all recruits have been guided outside the building
	psite.program_step(game);
	return false;
}

/* RST
mine
----

.. function:: mine=\<resource_name\> radius:\<number\> yield:\<percent\> when_empty:\<percent\>
     \[experience_on_fail:\<percent\>\] [no_notify]

   :arg string resource_name: The name of the resource to mine, e.g. 'coal' or 'water'.
   :arg int radius: The workarea radius that is searched for resources. Must be ``>0``.
   :arg percent yield: The :ref:`map_object_programs_datatypes_percent` of resources that the
      mine can dig up before its resource is depleted.
   :arg percent when_empty: The :ref:`map_object_programs_datatypes_percent` chance that the mine
      will still find some resources after it has been depleted.
   :arg percent experience_on_fail: The :ref:`map_object_programs_datatypes_percent` chance that the
      mine's workers will still gain some experience when mining fails after its resources have been
      depleted.
   :arg empty no_notify: Do not send a message to the player if this step fails.

   Takes resources from the ground. A building that mines will deplete when the percentage of
   resources given in ``resources`` has been dug up, leaving a chance of ``depleted`` that it
   will still find some resources anyway. Examples:

.. code-block:: lua

     actions = {
         "return=skipped unless economy needs iron_ore",
         "consume=ration",
         "sleep=duration:45s",
         "animate=working duration:20s",
          -- Search radius of 2 for iron. Will always find iron until 33.33% of it has been dug up.
          -- After that, there's still a chance of 5% for finding iron.
          -- If this fails, the workers still have a chance of 17% of gaining experience.
         "mine=resource_iron radius:2 yield:33.33% when_empty:5% experience_on_fail:17%",
         "produce=iron_ore"
     }

     actions = {
         "sleep=duration:20s",
         "animate=working duration:20s",
          -- Search radius of 1 for water. Will always find water until 100% of it has been drawn.
          -- After that, there's still a chance of 65% for finding water.
         "mine=resource_water radius:1 yield:100% when_empty:65%",
         "produce=water"
     }
*/
ProductionProgram::ActMine::ActMine(const std::vector<std::string>& arguments,
                                    Descriptions& descriptions,
                                    const std::string& production_program_name,
                                    ProductionSiteDescr* descr) {
	if (arguments.size() > 6 || arguments.size() < 4) {
		throw GameDataError("Usage: mine=<resource name> radius:<number> yield:<percent> "
		                    "when_empty:<percent> [experience:<percent>] [no_notify]");
	}
	experience_chance_ = 0U;

	for (const std::string& argument : arguments) {
		const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
		if (item.second.empty()) {
			// The safeguard is for the case that someone creates a resource called "no_notify"
			if (item.first == "no_notify" && resource_ != INVALID_INDEX) {
				notify_on_failure_ = false;
			} else {
				resource_ = descriptions.load_resource(item.first);
			}
		} else if (item.first == "radius") {
			workarea_ = read_positive(item.second);
		} else if (item.first == "yield") {
			max_resources_ = math::read_percent_to_int(item.second);
		} else if (item.first == "when_empty") {
			depleted_chance_ = math::read_percent_to_int(item.second);
		} else if (item.first == "experience_on_fail") {
			experience_chance_ = math::read_percent_to_int(item.second);
		} else {
			throw GameDataError("Unknown argument '%s'. Usage: mine=<resource name> radius:<number> "
			                    "yield:<percent> when_empty:<percent> [experience_on_fail:<percent>]",
			                    item.first.c_str());
		}
	}

	const std::string description = descr->name() + " " + production_program_name + " mine " +
	                                descriptions.get_resource_descr(resource_)->name();
	descr->workarea_info_[workarea_].insert(description);

	descr->add_collected_resource(arguments.front(), max_resources_, depleted_chance_);
}

void ProductionProgram::ActMine::execute(Game& game, ProductionSite& ps) const {
	Map* map = game.mutable_map();

	//  select one of the nodes randomly
	uint32_t totalres = 0;
	uint32_t totalchance = 0;
	uint32_t totalstart = 0;

	{
		MapRegion<Area<FCoords>> mr(
		   *map, Area<FCoords>(map->get_fcoords(ps.get_position()), workarea_));
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
			if (amount == 0) {
				totalchance += 0;
			} else if (amount <= 2) {
				totalchance += 6;
			} else if (amount <= 4) {
				totalchance += 4;
			} else if (amount <= 6) {
				totalchance += 2;
			}
		} while (mr.advance(*map));
	}

	//  how much is dug
	unsigned dug_percentage = math::k100PercentAsInt;
	if (totalstart != 0u) {
		dug_percentage = (totalstart - totalres) * math::k100PercentAsInt / totalstart;
	}
	if (totalres == 0u) {
		dug_percentage = math::k100PercentAsInt;
	}

	if (dug_percentage < max_resources_) {
		//  mine can produce normally
		if (totalres == 0) {
			return ps.program_end(game, ProgramResult::kFailed);
		}

		//  second pass through nodes
		assert(totalchance);
		int32_t pick = game.logic_rand() % totalchance;

		{
			MapRegion<Area<FCoords>> mr(
			   *map, Area<FCoords>(map->get_fcoords(ps.get_position()), workarea_));
			do {
				DescriptionIndex fres = mr.location().field->get_resources();
				ResourceAmount amount = mr.location().field->get_resources_amount();

				if (fres != resource_) {
					amount = 0;
				}

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
		if (notify_on_failure_ && depleted_chance_ <= 20 * math::k100PercentAsInt / 100U) {
			ps.notify_player(game, 60);
			// and change the default animation
			ps.set_default_anim("empty");
		}

		//  Mine has reached its limits, still try to produce something but
		//  independent of sourrunding resources. Do not decrease resources
		//  further.
		if (depleted_chance_ <= game.logic_rand() % math::k100PercentAsInt) {

			// Gain experience
			if (experience_chance_ > 0 &&
			    experience_chance_ >= game.logic_rand() % math::k100PercentAsInt) {
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

/* RST
checksoldier
------------
.. function:: checksoldier=soldier:attack|defense|evade|health level:\<number\>

   :arg string soldier: The soldier training attribute to check for.

   :arg int level: The level that the soldier should have for the given training attribute.

.. note:: This action is only available to :ref:`training sites
   <lua_tribes_buildings_trainingsites>`.

Returns failure unless there is a soldier present with the given training attribute at the given
level.

.. note:: The program's name must match the attribute to be trained and the level checked for, in
   the form ``upgrade_soldier_<attribute>_<level>``.

Example:

.. code-block:: lua

      upgrade_soldier_attack_3 = {
         -- TRANSLATORS: Completed/Skipped/Did not start upgrading ... because ...
         descname = _("upgrading soldier attack from level 3 to level 4"),
         actions = {
            -- Fails when there aren't any soldiers with attack level 3
            "checksoldier=soldier:attack level:3",
            "return=failed unless site has sword_long",
            "return=failed unless site has honey_bread,mead",
            "return=failed unless site has smoked_fish,smoked_meat",
            "sleep=duration:10s800ms",
            "animate=working duration:12s",
            -- Check again because the soldier could have been expelled by the player
            "checksoldier=soldier:attack level:3",
            "consume=sword_long honey_bread,mead smoked_fish,smoked_meat",
            "train=soldier:attack level:4"
         }
      },
*/
ProductionProgram::ActCheckSoldier::ActCheckSoldier(const std::vector<std::string>& arguments,
                                                    const ProductionSiteDescr& descr) {
	if (descr.type() != MapObjectType::TRAININGSITE) {
		throw GameDataError("Illegal 'checksoldier' action in productionsite '%s'. This action is "
		                    "only available to trainingsites.",
		                    descr.name().c_str());
	}
	if (arguments.size() != 2) {
		throw GameDataError("Usage: checksoldier=soldier:attack|defense|evade|health level:<number>");
	}

	training_ = TrainingParameters(arguments, "checksoldier");
}

void ProductionProgram::ActCheckSoldier::execute(Game& game, ProductionSite& ps) const {
	assert(ps.descr().type() == MapObjectType::TRAININGSITE);
	const SoldierControl* ctrl = ps.soldier_control();
	assert(ctrl != nullptr);
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();

	upcast(TrainingSite, ts, &ps);

	if (soldiers.empty()) {
		ps.set_production_result(ts->descr().no_soldier_to_train_message());
		return ps.program_end(game, ProgramResult::kSkipped);
	}
	ps.molog(game.get_gametime(), "  Checking soldier (%u) level %u)\n",
	         static_cast<unsigned int>(training_.attribute), training_.level);

	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();
	for (std::vector<Soldier*>::const_iterator it = soldiers.begin();; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(ts->descr().no_soldier_for_training_level_message());
			return ps.program_end(game, ProgramResult::kSkipped);
		}

		if (training_.attribute == TrainingAttribute::kHealth) {
			if ((*it)->get_health_level() == training_.level) {
				break;
			}
		} else if (training_.attribute == TrainingAttribute::kAttack) {
			if ((*it)->get_attack_level() == training_.level) {
				break;
			}
		} else if (training_.attribute == TrainingAttribute::kDefense) {
			if ((*it)->get_defense_level() == training_.level) {
				break;
			}
		} else if (training_.attribute == TrainingAttribute::kEvade) {
			if ((*it)->get_evade_level() == training_.level) {
				break;
			}
		}
	}
	ps.molog(game.get_gametime(), "    okay\n");  // okay, do nothing

	ts->training_attempted(training_.attribute, training_.level);

	ps.molog(game.get_gametime(), "  Check done!\n");

	return ps.program_step(game);
}

/* RST
train
-----
.. function:: train=soldier:attack|defense|evade|health level:\<number\>

   :arg string soldier: The soldier training attribute to be trained.

   :arg int level: The level that the soldier will receive for the given training attribute.

.. note:: This action is only available to :ref:`training sites
   <lua_tribes_buildings_trainingsites>`.

Increases a soldier's training attribute to the given level. It is mandatory to call 'checksoldier'
before calling this action to ensure that an appropriate soldier will be present at the site.
Example:

.. code-block:: lua

      actions = {
         "checksoldier=soldier:attack level:1",
         "return=failed unless site has ax_broad",
         "return=failed unless site has fish,meat",
         "return=failed unless site has barbarians_bread",
         "sleep=duration:30s",
         -- This is called first to ensure that we have a matching soldier
         "checksoldier=soldier:attack level:1",
         "consume=ax_broad fish,meat barbarians_bread",
         -- Now train the soldier's attack to level 2
         "train=soldier:attack level:2"
      }
*/
ProductionProgram::ActTrain::ActTrain(const std::vector<std::string>& arguments,
                                      const ProductionSiteDescr& descr) {
	if (descr.type() != MapObjectType::TRAININGSITE) {
		throw GameDataError("Illegal 'train' action in productionsite '%s'. This action is "
		                    "only available to trainingsites.",
		                    descr.name().c_str());
	}

	if (arguments.size() != 2) {
		throw GameDataError("Usage: train=soldier:attack|defense|evade|health level:<number>");
	}

	training_ = TrainingParameters(arguments, "train");
}

void ProductionProgram::ActTrain::execute(Game& game, ProductionSite& ps) const {
	assert(ps.descr().type() == MapObjectType::TRAININGSITE);
	TrainingSite& ts = dynamic_cast<TrainingSite&>(ps);
	const SoldierControl* ctrl = ps.soldier_control();
	const std::vector<Soldier*> soldiers = ctrl->present_soldiers();
	const std::vector<Soldier*>::const_iterator soldiers_end = soldiers.end();

	const unsigned current_level = ts.checked_soldier_training().level;
	assert(current_level != INVALID_INDEX);

	ps.molog(game.get_gametime(), "  Training soldier's %u (%u to %u)",
	         static_cast<unsigned int>(training_.attribute), current_level, training_.level);

	assert(current_level < training_.level);
	assert(ts.checked_soldier_training().attribute == training_.attribute);

	bool training_done = false;
	for (auto it = soldiers.begin(); !training_done; ++it) {
		if (it == soldiers_end) {
			ps.set_production_result(_("No soldier found for this training level!"));
			return ps.program_end(game, ProgramResult::kSkipped);
		}
		try {
			switch (training_.attribute) {
			case TrainingAttribute::kHealth:
				if ((*it)->get_health_level() == current_level) {
					(*it)->set_health_level(training_.level);
					training_done = true;
				}
				break;
			case TrainingAttribute::kAttack:
				if ((*it)->get_attack_level() == current_level) {
					(*it)->set_attack_level(training_.level);
					training_done = true;
				}
				break;
			case TrainingAttribute::kDefense:
				if ((*it)->get_defense_level() == current_level) {
					(*it)->set_defense_level(training_.level);
					training_done = true;
				}
				break;
			case TrainingAttribute::kEvade:
				if ((*it)->get_evade_level() == current_level) {
					(*it)->set_evade_level(training_.level);
					training_done = true;
				}
				break;
			case TrainingAttribute::kTotal:
				throw wexception("'total' training attribute can't be trained");
			default:
				NEVER_HERE();
			}
		} catch (...) {
			throw wexception("Fail training soldier!!");
		}
	}

	ps.molog(game.get_gametime(), "  Training done!\n");
	ps.set_production_result(
	   /** TRANSLATORS: Success message of a trainingsite '%s' stands for the description of the
	    * training program, e.g. Completed upgrading soldier evade from level 0 to level 1 */
	   format(_("Completed %s"), ps.top_state().program->descname()));

	ts.training_successful(training_.attribute, current_level);

	return ps.program_step(game);
}

/* RST
playsound
---------
Plays a sound effect. See :ref:`map_object_programs_playsound`.
*/
ProductionProgram::ActPlaySound::ActPlaySound(const std::vector<std::string>& arguments)
   : parameters(MapObjectProgram::parse_act_play_sound(arguments)) {
}

void ProductionProgram::ActPlaySound::execute(Game& game, ProductionSite& ps) const {
	Notifications::publish(NoteSound(SoundType::kAmbient, parameters.fx, ps.position_,
	                                 parameters.priority, parameters.allow_multiple));
	return ps.program_step(game);
}

/* RST
script
------
Runs a Lua function. See :ref:`map_object_programs_script`.
*/
ProductionProgram::ActRunScript::ActRunScript(const std::vector<std::string>& arguments)
   : parameters(MapObjectProgram::parse_act_script(arguments)) {
}

void ProductionProgram::ActRunScript::execute(Game& game, ProductionSite& ps) const {
	MapObjectProgram::do_run_script(game.lua(), &ps, parameters.function);
	return ps.program_step(game);
}

/* RST
construct
---------
.. function:: construct=\<immovable_name\> worker:\<program_name\> radius:\<number\>

   :arg string immovable_name: The name of the :ref:`immovable <lua_tribes_immovables>` to be
      constructed, e.g. ``barbarians_shipconstruction``.

   :arg string worker: The :ref:`worker's program <tribes_worker_programs>` that makes the worker
      walk to the immovable's location and do some work.

   :arg radius radius: The radius used by the worker to find a suitable construction spot on the
      map.

Sends the main worker to look for a suitable spot on the shore and to perform construction work on
an immovable. Example:

.. code-block:: lua

      -- Production program actions
      actions = {
         "construct=barbarians_shipconstruction worker:buildship radius:6",
         "sleep=duration:20s",
      }

      -- Corresponding worker program
      buildship = {
         "walk=object-or-coords",
         "plant=attrib:barbarians_shipconstruction unless object",
         "playsound=sound/sawmill/sawmill priority:80% allow_multiple",
         "animate=work duration:500ms",
         "construct",
         "animate=work duration:5s",
         "return"
      },
*/
ProductionProgram::ActConstruct::ActConstruct(const std::vector<std::string>& arguments,
                                              const std::string& production_program_name,
                                              ProductionSiteDescr* descr,
                                              const Descriptions& descriptions) {
	if (arguments.size() != 3) {
		throw GameDataError(
		   "Usage: construct=<immovable_name> worker:<program_name> radius:<number>");
	}

	for (const std::string& argument : arguments) {
		const std::pair<std::string, std::string> item = read_key_value_pair(argument, ':');
		if (item.first == "worker") {
			workerprogram = item.second;
		} else if (item.first == "radius") {
			radius = read_positive(item.second);
		} else if (item.second.empty()) {
			objectname = item.first;
		} else {
			throw GameDataError("Unknown parameter '%s'. Usage: construct=<immovable_name> "
			                    "worker:<program_name> radius:<number>",
			                    item.first.c_str());
		}
	}

	const std::string description =
	   descr->name() + ' ' + production_program_name + " construct " + objectname;
	descr->workarea_info_[radius].insert(description);

	// Register created immovable with productionsite
	const WorkerDescr& main_worker_descr =
	   *descriptions.get_worker_descr(descr->working_positions().front().first);
	for (const auto& attribute_info :
	     main_worker_descr.get_program(workerprogram)->created_attributes()) {
		descr->add_created_attribute(attribute_info);
	}
	descr->has_ship_fleet_check_ = true;
}

const ImmovableDescr&
ProductionProgram::ActConstruct::get_construction_descr(const Descriptions& descriptions) const {
	const ImmovableDescr* descr =
	   descriptions.get_immovable_descr(descriptions.immovable_index(objectname));
	if (descr == nullptr) {
		throw wexception("ActConstruct: immovable '%s' does not exist", objectname.c_str());
	}

	return *descr;
}

void ProductionProgram::ActConstruct::execute(Game& game, ProductionSite& psite) const {
	ProductionSite::State& state = psite.top_state();
	const ImmovableDescr& descr = get_construction_descr(game.descriptions());

	// Early check for no resources
	const Buildcost& buildcost = descr.buildcost();
	DescriptionIndex available_resource = INVALID_INDEX;

	for (const auto& item : buildcost) {
		if (psite.inputqueue(item.first, wwWARE, nullptr, 0).get_filled() > 0) {
			available_resource = item.first;
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
	Area<FCoords> area(map.get_fcoords(psite.get_position()), radius);
	FindImmovableAnd finder;
	finder.add(FindImmovableByDescr(descr));
	finder.add(FindImmovableNotReserved());
	if (map.find_reachable_immovables(game, area, &immovables, cstep, finder) != 0U) {
		state.objvar = immovables.at(game.logic_rand() % immovables.size()).object;

		psite.working_positions_.at(psite.main_worker_)
		   .worker.get(game)
		   ->update_task_buildingwork(game);
		return;
	}

	std::vector<ShipFleetYardInterface*> candidates;
	bool ships_needed = false;
	for (ShipFleetYardInterface* interface : psite.get_ship_fleet_interfaces()) {
		if (interface->get_fleet()->lacks_ship()) {
			ships_needed = true;
			BaseImmovable* immo = interface->get_position().field->get_immovable();
			if (immo == nullptr || immo->get_size() == BaseImmovable::Size::NONE) {
				candidates.push_back(interface);
			}
		}
	}

	if (candidates.empty()) {
		if (ships_needed || psite.get_ship_fleet_interfaces().empty()) {
			psite.molog(game.get_gametime(), "construct: no space for ships\n");
			psite.program_end(game, ProgramResult::kFailed);
		} else {
			psite.molog(game.get_gametime(), "construct: no ships needed\n");
			psite.program_end(game, ProgramResult::kSkipped);
		}
		return;
	}

	state.coord = candidates.at(game.logic_rand() % candidates.size())->get_position();
	psite.working_positions_.at(psite.main_worker_).worker.get(game)->update_task_buildingwork(game);
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
	if (construction != nullptr) {
		if (!construction->construct_remaining_buildcost(&remaining)) {
			psite.molog(game.get_gametime(), "construct: immovable %u not under construction",
			            construction->serial());
			psite.program_end(game, ProgramResult::kFailed);
			return false;
		}
	} else {
		const ImmovableDescr& descr = get_construction_descr(game.descriptions());
		remaining = descr.buildcost();
	}

	for (Buildcost::const_iterator it = remaining.begin(); it != remaining.end(); ++it) {
		WaresQueue& thiswq =
		   dynamic_cast<WaresQueue&>(psite.inputqueue(it->first, wwWARE, nullptr, 0));
		if (thiswq.get_filled() > 0) {
			wq = &thiswq;
			break;
		}
	}

	if (wq == nullptr) {
		psite.program_end(game, ProgramResult::kFailed);
		return false;
	}

	// Second step: give ware to worker
	WareInstance* ware =
	   new WareInstance(wq->get_index(), game.descriptions().get_ware_descr(wq->get_index()));
	ware->init(game);
	worker.set_carried_ware(game, ware);
	wq->set_filled(wq->get_filled() - 1);

	// Third step: send worker on his merry way, giving the target object or coords
	worker.start_task_program(game, workerprogram);
	worker.top_state().objvar1 = construction;
	worker.top_state().coords = state.coord;
	if (construction != nullptr) {
		construction->set_reserved_by_worker(true);
	}

	state.phase = ProgramResult::kFailed;
	return true;
}

void ProductionProgram::ActConstruct::building_work_failed(Game& game,
                                                           ProductionSite& psite,
                                                           Worker& /* worker */) const {
	psite.program_end(game, ProgramResult::kFailed);
}

ProductionProgram::ProductionProgram(const std::string& init_name,
                                     const LuaTable& program_table,
                                     Descriptions& descriptions,
                                     ProductionSiteDescr* building)
   : MapObjectProgram(init_name), descname_(program_table.get_string("descname")) {

	std::unique_ptr<LuaTable> actions_table = program_table.get_table("actions");

	for (const std::string& line : actions_table->array_entries<std::string>()) {
		if (line.empty()) {
			throw GameDataError("Empty line");
		}
		try {
			ProgramParseInput parseinput = parse_program_string(line);

			if (parseinput.name == "return") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActReturn(parseinput.arguments, *building, descriptions)));
			} else if (parseinput.name == "call") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActCall(parseinput.arguments)));
			} else if (parseinput.name == "sleep") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActSleep(parseinput.arguments)));
			} else if (parseinput.name == "animate") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActAnimate(parseinput.arguments, building)));
			} else if (parseinput.name == "consume") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActConsume(parseinput.arguments, *building, descriptions)));
			} else if (parseinput.name == "produce") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActProduce(parseinput.arguments, *building, descriptions)));
			} else if (parseinput.name == "recruit") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActRecruit(parseinput.arguments, *building, descriptions)));
			} else if (parseinput.name == "callworker") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActCallWorker(parseinput.arguments, name(), building, descriptions)));
			} else if (parseinput.name == "mine") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActMine(parseinput.arguments, descriptions, name(), building)));
			} else if (parseinput.name == "checksoldier") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActCheckSoldier(parseinput.arguments, *building)));
			} else if (parseinput.name == "train") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActTrain(parseinput.arguments, *building)));
			} else if (parseinput.name == "playsound") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActPlaySound(parseinput.arguments)));
			} else if (parseinput.name == "script") {
				actions_.push_back(
				   std::unique_ptr<ProductionProgram::Action>(new ActRunScript(parseinput.arguments)));
			} else if (parseinput.name == "construct") {
				actions_.push_back(std::unique_ptr<ProductionProgram::Action>(
				   new ActConstruct(parseinput.arguments, name(), building, descriptions)));
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
			// Add trained attributes
			if (upcast(const ActCheckSoldier, act_cs, &action)) {
				const auto& train = act_cs->training();
				train_from_level_ = train.level;
				if (train.attribute == Widelands::TrainingAttribute::kHealth) {
					trained_attribute_ = "Health";
				} else if (train.attribute == Widelands::TrainingAttribute::kAttack) {
					trained_attribute_ = "Attack";
				} else if (train.attribute == Widelands::TrainingAttribute::kDefense) {
					trained_attribute_ = "Defense";
				} else if (train.attribute == Widelands::TrainingAttribute::kEvade) {
					trained_attribute_ = "Evade";
				}
			} else if (upcast(const ActTrain, act_tr, &action)) {
				const auto& train = act_tr->training();
				train_to_level_ = train.level;
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

void ProductionProgram::validate_calls(const ProductionSiteDescr& descr) const {
	for (const auto& action : actions_) {
		if (upcast(const ActCall, act_call, action.get())) {
			const std::string& program_name = act_call->program_name();
			if (name() == program_name) {
				throw GameDataError("Production program '%s' in %s is calling itself",
				                    program_name.c_str(), descr.name().c_str());
			}
			const ProductionSiteDescr::Programs& programs = descr.programs();
			ProductionSiteDescr::Programs::const_iterator const it = programs.find(program_name);
			if (it == programs.end()) {
				throw GameDataError("Trying to call unknown program '%s' in %s", program_name.c_str(),
				                    descr.name().c_str());
			}
		}
	}
}
}  // namespace Widelands
