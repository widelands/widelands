/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include "event_expire_message_option_menu.h"

#include "editor/editorinteractive.h"
#include "events/event_expire_message.h"

#include "ui_basic/messagebox.h"

#include "upcast.h"

inline Editor_Interactive & Event_Expire_Message_Option_Menu::eia()
{
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}

Event_Expire_Message_Option_Menu::Event_Expire_Message_Option_Menu
	(Editor_Interactive & parent, Widelands::Event_Expire_Message & event)
	:
	UI::Window(&parent, 0, 0, 320, 55, _("Expire Message Event Options")),
	m_event   (event),
	label_name(*this),
	name      (*this, event.name()),
	ok        (*this),
	cancel    (*this)
{
	center_to_parent();
}

void Event_Expire_Message_Option_Menu::OK::clicked() {
	Event_Expire_Message_Option_Menu & menu =
		ref_cast<Event_Expire_Message_Option_Menu, UI::Panel>(*get_parent());
	std::string const & name = menu.name.text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 	menu.eia().egbase().map().mem()[name])
			if (registered_event != & menu.m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another event registered with the name \"%s\". "
					 	 "Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(menu.get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		menu.m_event.set_name(name);
	}
	menu.eia().set_need_save(true);
	menu.end_modal(1);
}
