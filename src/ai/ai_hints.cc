/*
 * Copyright (C) 2004, 2008-2009 by the Widelands Development Team
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

#include "profile/profile.h"

BuildingHints::BuildingHints(Section* const section)
   : log_producer_(section ? section->get_bool("logproducer") : false),
     stone_producer_(section ? section->get_bool("stoneproducer") : false),
     needs_water_(section ? section->get_bool("needs_water") : false),
     mines_water_(section ? section->get_bool("mines_water") : false),
     recruitment_(section ? section->get_bool("recruitment") : false),
     space_consumer_(section ? section->get_bool("space_consumer") : false),
     expansion_(section ? section->get_bool("expansion") : false),
     fighting_(section ? section->get_bool("fighting") : false),
     mountain_conqueror_(section ? section->get_bool("mountain_conqueror") : false),
     prohibited_till_(section ? section->get_natural("prohibited_till", 0) : 0),
     forced_after_(section ? section->get_natural("forced_after", 864000) : 864000),  // 10 days default
     mines_percent_(section ? section->get_int("mines_percent", 100) : 0),
     trainingsite_type_(TrainingSiteType::kNoTS)

{
	if (section) {
		if (section->has_val("renews_map_resource"))
			renews_map_resource_ = section->get_string("renews_map_resource");
		if (section->has_val("mines"))
			mines_ = section->get_string("mines");
	}
	if (section) {
		if (!section->has_val("trainingsite_type")) {
			trainingsite_type_ =  TrainingSiteType::kNoTS;
		} else {
			if (!strcmp(section->get_string("trainingsite_type", "basic"), "basic")) {
				trainingsite_type_ =  TrainingSiteType::kBasic;
			} else {
				trainingsite_type_ =  TrainingSiteType::kAdvanced;
			}
		}
	}
}
