/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include "event_set_timer_option_menu.h"

#include "editor/editorinteractive.h"
#include "events/event_set_timer.h"

#include "ui_basic/messagebox.h"

#include "upcast.h"

inline Editor_Interactive & Event_Set_Timer_Option_Menu::eia()
{
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}

Event_Set_Timer_Option_Menu::Event_Set_Timer_Option_Menu
	(Editor_Interactive & parent, Widelands::Event_Set_Timer & event)
	:
	UI::Window(&parent, 0, 0, 320, 360, _("Set Timer Event Options")),
	m_event   (event),
	label_name(*this),
	name      (*this, event.name()),
	duration  (this, Point(40, 30), event.duration()),
	timer     (*this, event.trigger()),
	ok        (*this),
	cancel    (*this)
{
	center_to_parent();
}

Event_Set_Timer_Option_Menu::Timer::Timer
	(Event_Set_Timer_Option_Menu   &       parent,
	 Widelands::Trigger_Time const * const selected_trigger)
	:
	UI::Listselect<Widelands::Trigger_Time &>(&parent, 5, 95, 310, 235)
{
	selected.set(&parent, &Event_Set_Timer_Option_Menu::timer_selected);
	Manager<Widelands::Trigger> & mtm = parent.eia().egbase().map().mtm();
	for
		(struct {
		 	Manager<Widelands::Trigger>::Index       current;
		 	Manager<Widelands::Trigger>::Index const end;
		 } i = {0, mtm.size()};
		 i.current < i.end;
		 ++i.current)
		if (upcast(Widelands::Trigger_Time, tt, &mtm[i.current])) {
			add
				(tt->name().c_str(),
				 *tt,
				 g_gr->get_no_picture(),
				 tt == selected_trigger);
			if (tt->referencers().empty())
				set_entry_color(size() - 1, RGBColor(255, 0, 0));
		}
}


void Event_Set_Timer_Option_Menu::OK::clicked() {
	Event_Set_Timer_Option_Menu & parent =
		ref_cast<Event_Set_Timer_Option_Menu, UI::Panel>(*get_parent());
	std::string const & name = parent.name.text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 	parent.eia().egbase().map().mem()[name])
			if (registered_event != & parent.m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another event registered with the name \"%s\". "
					 	 "Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(parent.get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		parent.m_event.set_name(name);
	}
	parent.m_event.set_duration(parent.duration.time());
	parent.m_event.set_trigger(&parent.timer.get_selected());
	parent.eia().set_need_save(true);
	parent.end_modal(1);
}
