/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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

#include "trigger_factory.h"

#include "trigger_building.h"
#include "trigger_defeated.h"
#include "trigger_military_influence.h"
#include "trigger_ownership.h"
#include "trigger_time.h"
#include "trigger_vision.h"
#include "wexception.h"

#include "i18n.h"

namespace Widelands {

namespace Trigger_Factory {

///\todo This data belongs into the triggers themselves
Type_Descr TRIGGER_TYPE_DESCRIPTIONS[] = {
	{
		"building",           _("Building Trigger"),
		_
			("Triggers when the player has the specified number of buildings of "
			 "the specified type in the specified area.")
	},
	{
		"defeated",           _("Defeated Player Trigger"),
		_
			("Triggers when the player was defeated. The trigger is set to true, "
			 "if the player has 0 workers.")
	},
	{
		"military_influence", _("Military influence Trigger"),
		_
			("Triggers when the player has some (or highest) military influence "
			 "over the specified number of nodes in the specified area.")
	},
	{
		"ownership",          _("Ownership Trigger"),
		_
			("Triggers when the the player owns at least the specified number of "
			 "locations in the specified area. Unless it is a one-time trigger, "
			 "it becomes unset when this no longer holds.")
	},
	{
		"time",               _("Time Trigger"),
		_
			("Triggers at a certain time. The time can be set to a value in ms. "
			 "If the trigger has no time is set, a time can be set for it with "
			 "an event.")
	},
	{
		"vision",             _("Vision Trigger"),
		_
			("Triggers when the the player sees (or has seen) at least a "
			 "specified number of locations in the specified area. Unless it is "
			 "a one-time trigger, it becomes unset when this no longer holds.")
	},
};


Trigger & create(size_t const id) {
	return
		create(id, i18n::translate(TRIGGER_TYPE_DESCRIPTIONS[id].name), false);
}


Trigger & create(size_t const id, char const * const name, bool const set) {
	switch (id) {
	case 0: return *new Trigger_Building          (name, set);
	case 1: return *new Trigger_Defeated          (name, set);
	case 2: return *new Trigger_Military_Influence(name, set);
	case 3: return *new Trigger_Ownership         (name, set);
	case 4: return *new Trigger_Time              (name, set);
	case 5: return *new Trigger_Vision            (name, set);
	default:
		assert(false);
	}
}


Trigger & create
	(char const * type_name, char const * const name, bool const set)
{
	//  Handle old names.
	if (not strcmp(type_name, "null"))
		type_name = "time";

	size_t i = 0;
	while (strcmp(type_name, TRIGGER_TYPE_DESCRIPTIONS[i].id))
		if (++i == nr_trigger_types())
			throw wexception("invalid type \"%s\"", type_name);
	return create(i, name, set);
}


Type_Descr const & type_descr(size_t const id) {
	assert(id < nr_trigger_types());
	return TRIGGER_TYPE_DESCRIPTIONS[id];
}


size_t nr_trigger_types() {
	return
		sizeof(TRIGGER_TYPE_DESCRIPTIONS) / sizeof(*TRIGGER_TYPE_DESCRIPTIONS);
}

};

};
