/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

/* RST
AI Hints
========

Every :doc:`building <autogen_toc_lua_tribes_buildings>`'s ``init.lua`` file has an ``aihints`` table in its ``new_<building_type>_type{table}``
function. This ``aihints`` table can contain any number of entries, which will help the AI decide
when and where to build or dismantle a building of that type.

All entries in ``aihints`` are optional.

- :ref:`ai_hints_common`
- :ref:`ai_hints_military`
- :ref:`ai_hints_production`

.. _ai_hints_common:

Common
------

**basic_amount**
    The amount of this building to be built while establishing a basic economy, e.g.::

        basic_amount = 1,

**forced_after**
    If a building of this type hasn't been built already, force that a building of this type
    is constructed after the given time (in seconds) has elapsed, e.g.::

        forced_after = 890,

**prohibited_till**
    Do not build this building before the given time (in seconds) has elapsed, e.g.::

        prohibited_till = 1500,

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

**fighting**
    The building is suitable for military conflicts, e.g.::

        fighting = true,

**mountain_conqueror**
    Prefer this type of military site near mountains, e.g.::

        mountain_conqueror = true,


.. _ai_hints_production:

Production Sites
----------------

**graniteproducer**
    The building will produce the ``granite`` ware, e.g.::

        graniteproducer = true,

**logproducer**
    The building will produce the ``log`` ware, e.g.::

        logproducer = true,

**mines**
    The building will mine to obtain the given ware, e.g.::

        mines = "gold",

**mines_percent**
    The percentage that a mine will mine of its resource before it needs enhancing, e.g.::

        mines_percent = 60,

**mines_water**
    The building will mine to obtain the ``water`` ware, e.g.::

        mines_water = true,

**needs_water**
    The building needs to be placed near a body of water, e.g.::

        needs_water = true,

**recruitment**
    The building will recruit the tribe's carrier2, for example oxen or horses, e.g.::

        recruitment = true,

**shipyard**
    This building will construct ships, e.g.::

        shipyard = true,

**space_consumer**
    The building needs a lot of space around it, for example a farm needs space for its fields, e.g.::

        space_consumer = true,

**supports_production_of**
    This building will support the production of the given wares without producing it directly, e.g.::

        supports_production_of = { "fish" },

    For example, if a building supports the production of fish, it should be placed near a building
    that has fish in its output.

**trainingsites_max_percent**
    The maximum percentate this trainingsite will have among all training sites, e.g.::

        trainingsites_max_percent = 20,

    For example, if an AI has built 5 trainingsites, it will have a maximum of 1 site of this type
    if the value is set to ``20``. For trainingsites that don't have this value set, their percentage
    will be distributed evenly.

*/
BuildingHints::BuildingHints(std::unique_ptr<LuaTable> table)
   : mines_(table->has_key("mines") ? table->get_string("mines") : ""),
     log_producer_(table->has_key("logproducer") ? table->get_bool("logproducer") : false),
     granite_producer_(table->has_key("graniteproducer") ? table->get_bool("graniteproducer") :
                                                           false),
     needs_water_(table->has_key("needs_water") ? table->get_bool("needs_water") : false),
     mines_water_(table->has_key("mines_water") ? table->get_bool("mines_water") : false),
     recruitment_(table->has_key("recruitment") ? table->get_bool("recruitment") : false),
     space_consumer_(table->has_key("space_consumer") ? table->get_bool("space_consumer") : false),
     expansion_(table->has_key("expansion") ? table->get_bool("expansion") : false),
     fighting_(table->has_key("fighting") ? table->get_bool("fighting") : false),
     mountain_conqueror_(
        table->has_key("mountain_conqueror") ? table->get_bool("mountain_conqueror") : false),
     shipyard_(table->has_key("shipyard") ? table->get_bool("shipyard") : false),
     prohibited_till_(table->has_key("prohibited_till") ? table->get_int("prohibited_till") : 0),
     basic_amount_(table->has_key("basic_amount") ? table->get_int("basic_amount") : 0),
     // 10 days default
     forced_after_(table->has_key("forced_after") ? table->get_int("forced_after") : 864000),
     mines_percent_(table->has_key("mines_percent") ? table->get_int("mines_percent") : 100),
     very_weak_ai_limit_(
        table->has_key("very_weak_ai_limit") ? table->get_int("very_weak_ai_limit") : -1),
     weak_ai_limit_(table->has_key("weak_ai_limit") ? table->get_int("weak_ai_limit") : -1),
     trainingsites_max_percent_(table->has_key("trainingsites_max_percent") ?
                                   table->get_int("trainingsites_max_percent") :
                                   0) {
	if (table->has_key("supports_production_of")) {
		for (const std::string& ware_name :
		     table->get_table("supports_production_of")->array_entries<std::string>()) {
			supported_production_.insert(ware_name);
		}
	}
}

void BuildingHints::set_trainingsites_max_percent(int percent) {
	trainingsites_max_percent_ = percent;
}

uint8_t BuildingHints::trainingsites_max_percent() const {
	return trainingsites_max_percent_;
}
