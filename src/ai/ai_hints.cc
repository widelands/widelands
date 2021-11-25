/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include "ai/ai_hints.h"

#include <memory>

#include "base/log.h"
#include "logic/game_data_error.h"

namespace AI {

/* RST
AI Hints and Restrictions
=========================

Every :doc:`building <autogen_toc_lua_tribes_buildings>`'s ``init.lua`` file has an ``aihints``
table in its ``new_<building_type>_type{table}`` function. This ``aihints`` table can contain any
number of entries, which will help the AI decide when and where to build or dismantle a building of
that type and/or how to treat it.

All entries in ``aihints`` are optional.

- :ref:`ai_hints_introduction`
- :ref:`ai_hints_common`
- :ref:`ai_hints_military`
- :ref:`ai_hints_production`

.. _ai_hints_introduction:

Introduction
------------

The AI has separate code paths for various building categories. Many of them are
categories that contain one unique building (ranger, well, port, fish breeder,
barracks and so on), and these cannot be combined with other types of buildings
and are mostly mandatory.

The main categories where you can freely modify and add buildings are:

- Military sites
- Training sites
- Pure production sites (they have outputs, and optionally inputs, but no production hints)
- Pure supporters (they have production hints, but neither inputs nor outputs)
- A combination of supporter and production site is possible, but suboptimal

You can create as many building types as you want in these main categories, but
make sure that you don't combine any incompatible features (for example,
``shipyard`` and ``mines`` don't combine).

With the exception of the barracks and the building that produces carrier2
(see: :ref:`lua_tribes_tribes_units`), production of workers in production sites
is not supported at this time.

.. _ai_hints_common:

Common Building Hints
---------------------

**basic_amount**
    The amount of this building to be built while establishing a basic economy, e.g.::

        basic_amount = 1,

**forced_after**
    If a building of this type hasn't been built already, force that a building
    of this type will be constructed after the given time (in seconds) has elapsed,
    e.g.::

        forced_after = 890,

    **Note:** ``forced after`` can interfere with setting up the basic economy
    defined by ``basic_amount``: if we don't want the building to be part of the
    basic economy but it is forced before all the buildings for the basic economy
    have been built, this can lead to unobvious behavior. Part of this ambiguity
    is due to the genetic algorithm.

**prohibited_till**
    Do not build this building before the given time (in seconds) has elapsed, e.g.::

        prohibited_till = 1500,

    This takes precedence over ``basic_amount``, so it means it can delay achieving
    the basic economy.

**very_weak_ai_limit**
    The maximum number of this building type that the very weak AI is allowed to build, e.g.::

        very_weak_ai_limit = 1,

**weak_ai_limit_**
    The maximum number of this building type that the weak AI is allowed to build, e.g.::

        weak_ai_limit_ = 2,

.. _ai_hints_military:

Military Sites
--------------

**expansion**
    The building will be used by the AI for expanding the territory, e.g.::

        expansion = true,

    **Note:** It is recommended to have at least one building has this feature.

**fighting**
    The building is suitable for military conflicts, e.g.::

        fighting = true,

    **Note:** It is recommended to have at least one building has this feature.

**mountain_conqueror**
    Prefer this type of military site near mountains, e.g.::

        mountain_conqueror = true,

    **Note:** It is recommended to have at least one building has this feature.

.. _ai_hints_production:

Production Sites
----------------

**collects_ware_from_map**
    **DEPRECATED** The building will generate this ware from the map, e.g. a well mining the
    ``water`` ware, or the hunter returning from the hunt with the ``meat`` ware. The same
    ware needs also to be listed as the first one of the building's outputs, e.g.::

        aihints = {
            collects_ware_from_map = "meat"
        },

        outputs = {
            "meat",
            "fur"
        },

    **Note:** The AI expects exactly one such building type for each of the following wares:
    ``fish`` (fisher), ``granite`` (quarry), ``log`` (lumberjack/woodcutter), ``meat`` (hunter),
    ``water`` (well).

**mines**
    **DEPRECATED** The building will mine to obtain the given ware, e.g.::

        mines = "resource_gold",

**mines_percent**
    **DEPRECATED** The percentage that a mine will mine of its resource before it needs enhancing,
    e.g.::

        mines_percent = 60,

**needs_water**
    The building needs to be placed near a body of water, e.g.::

        needs_water = true,

    **Note:** The AI expects exactly one such building type.

**recruitment**
    The building will recruit the tribe's carrier2, for example oxen or horses, e.g.::

        recruitment = true,

**shipyard**
    This building will construct ships, e.g.::

        shipyard = true,

    **Note:** The AI expects exactly one such building type.

**supports_seafaring**
    This building is needed for expedition and seafaring, e.g.::

        supports_seafaring = true,

**space_consumer**
    The building needs a lot of space around it, for example a farm needs space for
    its fields, e.g.::

        space_consumer = true,

**supports_production_of**
    **DEPRECATED** This building will support the production of the given wares without producing
    it directly, e.g.::

        supports_production_of = { "fish" },

    For example, if a building supports the production of fish, it should be placed
    near a building that has fish in its output.

    **Note:** The AI expects that supporters will have no inputs and no outputs. Although
    the AI can tolerate such buildings, they will be primarily treated as normal
    production sites when deciding on the building's location.

**requires_supporters**
    This building will be built only if a supporter is nearby::

        requires_supporters = true,

    For example if set for a frisian aqua farm, it will be built only if a clay pit producing the
ponds is nearby.
    **Note:** This hint shouldn't be used for lumberjacks due to internal AI mechanics

**trainingsites_max_percent**
    The maximum percengate this training site will have among all training sites, e.g.::

        trainingsites_max_percent = 20,

    In this example, if an AI has built 5 training sites, it will have a maximum of
    1 site of this type. If it has built 10 training sites, the maximum is 2.
    For training sites that don't have this value set, their percentage will be
    distributed evenly.

*/

BuildingHints::BuildingHints(std::unique_ptr<LuaTable> table, const std::string& building_name)
   : needs_water_(table->has_key("needs_water") ? table->get_bool("needs_water") : false),
     space_consumer_(table->has_key("space_consumer") ? table->get_bool("space_consumer") : false),
     expansion_(table->has_key("expansion") ? table->get_bool("expansion") : false),
     fighting_(table->has_key("fighting") ? table->get_bool("fighting") : false),
     mountain_conqueror_(
        table->has_key("mountain_conqueror") ? table->get_bool("mountain_conqueror") : false),
     shipyard_(table->has_key("shipyard") ? table->get_bool("shipyard") : false),
     supports_seafaring_(
        table->has_key("supports_seafaring") ? table->get_bool("supports_seafaring") : false),
     prohibited_till_(table->has_key("prohibited_till") ? table->get_int("prohibited_till") : 0),
     basic_amount_(table->has_key("basic_amount") ? table->get_int("basic_amount") : 0),
     // 10 days default
     forced_after_(table->has_key("forced_after") ? table->get_int("forced_after") : 864000),
     very_weak_ai_limit_(
        table->has_key("very_weak_ai_limit") ? table->get_int("very_weak_ai_limit") : -1),
     weak_ai_limit_(table->has_key("weak_ai_limit") ? table->get_int("weak_ai_limit") : -1),
     normal_ai_limit_(table->has_key("normal_ai_limit") ? table->get_int("normal_ai_limit") : -1),
     requires_supporters_(
        table->has_key("requires_supporters") ? table->get_bool("requires_supporters") : false),
     trainingsites_max_percent_(table->has_key("trainingsites_max_percent") ?
                                   table->get_int("trainingsites_max_percent") :
                                   0) {
	if (table->has_key("supports_production_of")) {
		log_warn("%s: The 'supports_production_of' key in 'ai_hints' is no longer used",
		         building_name.c_str());
	}

	if (table->has_key("collects_ware_from_map")) {
		log_warn("%s: The 'collects_ware_from_map' key in 'ai_hints' is no longer used",
		         building_name.c_str());
	}

	if (table->has_key("mines")) {
		log_warn("%s: The 'mines' key in 'ai_hints' is no longer used", building_name.c_str());
	}
	if (table->has_key("mines_percent")) {
		log_warn(
		   "%s: The 'mines_percent' key in 'ai_hints' is no longer used", building_name.c_str());
	}
}

void BuildingHints::set_trainingsites_max_percent(int percent) {
	trainingsites_max_percent_ = percent;
}

uint8_t BuildingHints::trainingsites_max_percent() const {
	return trainingsites_max_percent_;
}

int16_t BuildingHints::get_ai_limit(const AiType ai_type) const {
	switch (ai_type) {
	case AiType::kVeryWeak:
		return very_weak_ai_limit_;
	case AiType::kWeak:
		return weak_ai_limit_;
	case AiType::kNormal:
		return normal_ai_limit_;
	}
	NEVER_HERE();
}

/// Returns the preciousness of the ware, or kInvalidWare if the tribe doesn't use the ware.
int WareWorkerHints::preciousness(const std::string& tribename) const {
	if (preciousnesses_.count(tribename) > 0) {
		return preciousnesses_.at(tribename);
	}
	return Widelands::kInvalidWare;
}

void WareWorkerHints::set_preciousness(const std::string& ware_worker,
                                       const std::string& tribename,
                                       int p) {
	constexpr int kMaxRecommendedPreciousness = 50;
	if (p > 200) {
		throw Widelands::GameDataError(
		   "Preciousness of %d is far too high for ware/worker '%s' and tribe '%s'. "
		   "We recommend not going over %d.",
		   p, ware_worker.c_str(), tribename.c_str(), kMaxRecommendedPreciousness);
	} else if (p > kMaxRecommendedPreciousness) {
		log_warn("Preciousness of %d is a bit high for ware/worker '%s' and tribe '%s'. We "
		         "recommend not going over %d.\n",
		         p, ware_worker.c_str(), tribename.c_str(), kMaxRecommendedPreciousness);
	}
	preciousnesses_.insert(std::make_pair(tribename, p));
}
}  // namespace AI
