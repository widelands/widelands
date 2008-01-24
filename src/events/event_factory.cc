/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "event_factory.h"

#include "event_allow_building.h"
#include "event_conquer_area.h"
#include "event_message_box.h"
#include "event_move_view.h"
#include "event_unhide_area.h"
#include "event_set_null_trigger.h"
#include "event_reveal_campaign.h"
#include "event_reveal_objective.h"
#include "event_reveal_scenario.h"
#include "wexception.h"

#include "i18n.h"

namespace Widelands {

namespace Event_Factory {

Type_Descr EVENT_TYPE_DESCRIPTIONS[] = {
	{"message_box",            _("Show message box"),
			_("This Event shows a messagebox. The user can choose to make it modal/non-modal and to add a picture. Events can be assigned to each button to use this as a Choose Dialog for the user")},
	{"move_view",              _("Move view"),
			_("This Event centers the Players View on a certain location")},
	{"unhide_area",            _("Unhide area"),
			_("Gives vision for a specified amount of time of all locations whithin an area to the player.")},
	{"conquer_area",           _("Conquer area"),
			_("Gives ownership of all unowned locations whithin an area to the player")},
	{"allow_building",         _("Allow building type"),
			_("Allows (or forbids) tye construction of a building type for a player")},
	{"set_null_trigger",       _("Set null trigger"),
			_("Manually set a Null Trigger to a given value")},
	{"reveal_objective",       _("Reveal objective"),
			_("Reveal (or hide) an objective")},
	{"reveal_scenario",        _("Reveal scenario"),
			_("Reveal (or hide) a scenario")},
	{"reveal_campaign",        _("Reveal campaign"),
			_("Reveal (or hide) a campaign")},
};


Event & create(size_t const id) {
	return
		create
		(id,
		 i18n::translate(EVENT_TYPE_DESCRIPTIONS[id].name).c_str(),
		 Event::INIT);
}


Event & create
	(size_t const id, char const * const name, Event::State const state)
{
	switch (id) {
	case 0: return *new Event_Message_Box     (name, state);
	case 1: return *new Event_Move_View       (name, state);
	case 2: return *new Event_Unhide_Area     (name, state);
	case 3: return *new Event_Conquer_Area    (name, state);
	case 4: return *new Event_Allow_Building  (name, state);
	case 5: return *new Event_Set_Null_Trigger(name, state);
	case 6: return *new Event_Reveal_Objective(name, state);
	case 7: return *new Event_Reveal_Scenario (name, state);
	case 8: return *new Event_Reveal_Campaign (name, state);
	default: assert(false);
	}
}


Event & create
	(const char * type_name, char const * const name, Event::State const state)
{

	//  Handle old names.
	if (not strcmp(type_name, "unhide_objective"))
		type_name = "reveal_objective";
	if (not strcmp(type_name, "set_map_visiblity"))
		type_name = "reveal_scenario";
	if (not strcmp(type_name, "set_campaign_visiblity"))
		type_name = "reveal_campaign";

	size_t i = 0;
	while (strcmp(type_name, EVENT_TYPE_DESCRIPTIONS[i].id))
		if (++i == nr_event_types())
			throw wexception("invalid type \"%s\"", type_name);
	return create(i, name, state);
}


Type_Descr const & type_descr(size_t const id) {
	assert(id < nr_event_types());
	return EVENT_TYPE_DESCRIPTIONS[id];
}


size_t nr_event_types() {
	return sizeof(EVENT_TYPE_DESCRIPTIONS) / sizeof(*EVENT_TYPE_DESCRIPTIONS);
}

};

};
