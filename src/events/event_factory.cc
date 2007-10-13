/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editorinteractive.h"
#include "event.h"
#include "event_allow_building.h"
#include "event_allow_building_option_menu.h"
#include "event_conquer_area.h"
#include "event_conquer_area_option_menu.h"
#include "event_message_box.h"
#include "event_message_box_option_menu.h"
#include "event_move_view.h"
#include "event_move_view_option_menu.h"
#include "event_unhide_area.h"
#include "event_unhide_area_option_menu.h"
/* EVENTS BELOW HAVE NO OPTION MENU YET */
#include "event_set_null_trigger.h"
#include "event_unhide_objective.h"
#include "event_set_visiblity.h"
#include "wexception.h"

namespace Event_Factory {

Event_Descr EVENT_DESCRIPTIONS[] = {
	{"message_box", _("Message Box"), _("This Event shows a messagebox. The user can choose to make it modal/non-modal and to add a picture. Events can be assigned to each button to use this as a Choose Dialog for the user")},
	{"move_view", _("Move View"), _("This Event centers the Players View on a certain field")},
	{"unhide_area", _("Unhide Area"), _("This Event makes a user definable part of the map visible for a selectable user")},
	{"conquer_area", _("Conquer Area"), _("This Event conquers a user definable part of the map for one player if there isn't a player already there")},
	{"allow_building", _("Allow Building"), _("Allows/Disables a certain building for a player so that it can be build or it can't any longer")},
// TODO: Events below are not creatable in the editor. Make UI Windows for them
	// {"set_null_trigger", _("Set Null Trigger"), _("Manually set a Null Trigger to a given value")},
	// {"unhide_objective", _("Unhide Objective"), _("Hide or unhide an objective so that the player can see it")},
	// {"set_visiblity", _("Set Visiblity"), _("Make a campaign or map visible or invisible in the Campaign UI")},
};


/**
 * return the correct event for this id
 */
Event * get_correct_event(const char * id) {
	if (strcmp("message_box",      id) == 0)
		return new Event_Message_Box     ();
	if (strcmp("move_view",        id) == 0)
		return new Event_Move_View       ();
	if (strcmp("unhide_area",      id) == 0)
		return new Event_Unhide_Area     ();
	if (strcmp("conquer_area",     id) == 0)
		return new Event_Conquer_Area    ();
	if (strcmp("allow_building",   id) == 0)
		return new Event_Allow_Building  ();
	if (strcmp("set_null_trigger", id) == 0)
		return new Event_Set_Null_Trigger();
	if (strcmp("unhide_objective", id) == 0)
		return new Event_Unhide_Objective();
	if (strcmp("set_visiblity",    id) == 0)
		return new Event_Set_Visiblity   ();
	throw wexception
		("Event_Factory::get_correct_event: Unknown event id found: %s", id);
}


/**
 * create the correct option dialog and initialize it with the given
 * event. if the given event is zero, create a new event
 * and let it be initalised through it.
 * if it fails, return zero/unmodified given event, elso return the created/modified event
 */
Event * make_event_with_option_dialog
(const char * const id, Editor_Interactive & m_parent, Event * const gevent)
{
	Event* event=gevent;
	if (!event)
		event=get_correct_event(id);

	int32_t retval;
	if        (strcmp("message_box",    id) == 0) {
		Event_Message_Box_Option_Menu t
			(m_parent, dynamic_cast<Event_Message_Box    &>(*event));
		retval = t.run();
	} else if (strcmp("move_view",      id) == 0) {
		Event_Move_View_Option_Menu t
			(m_parent, dynamic_cast<Event_Move_View      &>(*event));
		retval = t.run();
	} else if (strcmp("unhide_area",    id) == 0) {
		Event_Unhide_Area_Option_Menu t
			(m_parent, dynamic_cast<Event_Unhide_Area    &>(*event));
		retval = t.run();
	} else if (strcmp("conquer_area",   id) == 0) {
		Event_Conquer_Area_Option_Menu t
			(m_parent, dynamic_cast<Event_Conquer_Area   &>(*event));
		retval = t.run();
	} else if (strcmp("allow_building", id) == 0) {
		Event_Allow_Building_Option_Menu t
			(m_parent, dynamic_cast<Event_Allow_Building &>(*event));
		retval = t.run();
	} else
		throw wexception
			("Event_Factory::make_event_with_option_dialog: Unknown event id "
			 "found: %s",
			 id);
	if (retval)
		return event;
	if (!gevent) {
		delete event;
		return 0;
	} else
		return gevent;
}


/**
 * Get event description by number
 */
Event_Descr * get_event_descr(const uint32_t id) {
	assert(id < get_nr_of_available_events());

	return &EVENT_DESCRIPTIONS[id];
}


/**
 * return the nummer of available events
 */
const uint32_t get_nr_of_available_events() {
	return sizeof(EVENT_DESCRIPTIONS) / sizeof(*EVENT_DESCRIPTIONS);
}

};
