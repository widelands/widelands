#include "log.h"/*
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

#include "trigger_time.h"
#include "trigger_null.h"
#include "trigger_building.h"
#include "wexception.h"

#include "i18n.h"

namespace Widelands {

namespace Trigger_Factory {

Type_Descr TRIGGER_TYPE_DESCRIPTIONS[] = {
	{"time",     _("Time Trigger"),
			_("This Trigger waits a certain time before it is true. It can be configured to constantly restart itself when the wait time is over for repeating events")},
	{"null",     _("Null Trigger"),
			_("This Trigger never changes its state by itself. It is useful to pass it to some event which changes triggers")},
	{"building", _("Building Trigger"),
			_("Triggers when the player has the specified number of buildings of the specified type in the specified area.")},
#if 0
	{"trigger_ownership", _("Own Area Trigger"),
			_("Triggers when the the player owns at least a specified number of locations in the specified area. Unless it is a one-time trigger, it becomes unset when this no longer holds.")},
#endif
};


Trigger & create(size_t const id) {
	return
		create
		(id,
		 i18n::translate(TRIGGER_TYPE_DESCRIPTIONS[id].name).c_str(),
		 false);
}


Trigger & create(size_t const id, char const * const name, bool const set) {
	switch (id) {
	case 0: return *new Trigger_Time    (name, set);
	case 1: return *new Trigger_Null    (name, set);
	case 2: return *new Trigger_Building(name, set);
	default: assert(false);
	}
}


Trigger & create
	(const char * const type_name, char const * const name, bool const set)
{
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
