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

BuildingHints::BuildingHints(std::unique_ptr<LuaTable> table)
   : renews_map_resource_(
        table->has_key("renews_map_resource") ? table->get_string("renews_map_resource") : ""),
     mines_(table->has_key("mines") ? table->get_string("mines") : ""),
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
     // 10 days default
     forced_after_(table->has_key("forced_after") ? table->get_int("forced_after") : 864000),
     mines_percent_(table->has_key("mines_percent") ? table->get_int("mines_percent") : 100),
     very_weak_ai_limit_(
        table->has_key("very_weak_ai_limit") ? table->get_int("very_weak_ai_limit") : -1),
     weak_ai_limit_(table->has_key("weak_ai_limit") ? table->get_int("weak_ai_limit") : -1),
     trainingsites_max_percent_(table->has_key("trainingsites_max_percent") ?
                                   table->get_int("trainingsites_max_percent") :
                                   0) {
}

void BuildingHints::set_trainingsites_max_percent(int percent) {
	trainingsites_max_percent_ = percent;
}

uint8_t BuildingHints::trainingsites_max_percent() const {
	return trainingsites_max_percent_;
}
