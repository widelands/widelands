/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "trigger_time_option_menu.h"

#include "graphic/graphic.h"
#include "i18n.h"
#include "trigger/trigger_time.h"
#include "editor/editorinteractive.h"


int32_t Widelands::Trigger_Time::option_menu(Editor_Interactive & eia) {
	Trigger_Time_Option_Menu m(eia, *this); return m.run();
}


inline Editor_Interactive & Trigger_Time_Option_Menu::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Trigger_Time_Option_Menu::Trigger_Time_Option_Menu
	(Editor_Interactive & parent, Widelands::Trigger_Time & trigger)
	:
	UI::Window         (&parent, 0, 0, 320, 145, _("Trigger Time Options")),
	m_trigger          (trigger),
	label_name         (this, 5, 5, _("Name:")),
	name               (*this, trigger.name()),
	absolute_time
		(this, Point(5, 30),
		 g_gr->get_no_picture(),
		 _
		 	("Trigger at an absolute time from the beginning of the game "
		 	 "(otherwise, only trigger if activated by an event set timer).")),
	label_absolute_time(this, 30, 30, _("Trigger at absolute time")),
	time
		(this,
		 Point(40, 55),
		 trigger.time() != Widelands::Never() ? trigger.time() : 0,
		 trigger.time() != Widelands::Never()),
	ok                 (*this),
	cancel             (*this)
{
	absolute_time.clickedto.set
		(this, &Trigger_Time_Option_Menu::absolute_time_clickedto);
	absolute_time.set_state(trigger.time() != Widelands::Never());

	center_to_parent();
}



void Trigger_Time_Option_Menu::absolute_time_clickedto(bool const on) {
	time.set_enabled(on);
}


void Trigger_Time_Option_Menu::OK::clicked() {
	Trigger_Time_Option_Menu & parent =
		ref_cast<Trigger_Time_Option_Menu, UI::Panel>(*get_parent());
	std::string const & name = parent.name.text();
	if (name.size()) {
		if
			(Widelands::Trigger * const registered_trigger =
			 	parent.eia().egbase().map().mtm()[name])
			if (registered_trigger != &parent.m_trigger) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another trigger registered with the name "
					 	 "\"%s\". Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(parent.get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		parent.m_trigger.set_name(name);
	}
	parent.m_trigger.set_time
		(parent.absolute_time.get_state() ? parent.time.time() :
		 Widelands::Never());
	parent.eia().set_need_save(true);
	parent.end_modal(1);
}
