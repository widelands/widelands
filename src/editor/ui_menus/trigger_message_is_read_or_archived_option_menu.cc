/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "trigger_message_is_read_or_archived_option_menu.h"

#include "i18n.h"
#include "trigger/trigger_message_is_read_or_archived.h"
#include "editor/editorinteractive.h"

#include "ui_basic/messagebox.h"


int32_t Widelands::Trigger_Message_Is_Read_Or_Archived::option_menu
	(Editor_Interactive & eia)
{
	Trigger_Message_Is_Read_Or_Archived_Option_Menu m(eia, *this);
	return m.run();
}


inline Editor_Interactive & Trigger_Message_Is_Read_Or_Archived_Option_Menu::
eia()
{
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Trigger_Message_Is_Read_Or_Archived_Option_Menu::
Trigger_Message_Is_Read_Or_Archived_Option_Menu
	(Editor_Interactive                             & parent,
	 Widelands::Trigger_Message_Is_Read_Or_Archived & trigger)
	:
	UI::Window
		(&parent,
		 0, 0, 320, 55,
		 _("Trigger Message is Read or Archived Options")),
	m_trigger          (trigger),
	label_name         (this, 5, 5, _("Name:")),
	name               (*this, trigger.name()),
	ok                 (*this),
	cancel             (*this)
{
	center_to_parent();
}


void Trigger_Message_Is_Read_Or_Archived_Option_Menu::OK::clicked() {
	Trigger_Message_Is_Read_Or_Archived_Option_Menu & menu =
		ref_cast<Trigger_Message_Is_Read_Or_Archived_Option_Menu, UI::Panel>
			(*get_parent());
	std::string const & name = menu.name.text();
	if (name.size()) {
		if
			(Widelands::Trigger * const registered_trigger =
			 	menu.eia().egbase().map().mtm()[name])
			if (registered_trigger != &menu.m_trigger) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another trigger registered with the name "
					 	 "\"%s\". Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(menu.get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		menu.m_trigger.set_name(name);
	}
	menu.eia().set_need_save(true);
	menu.end_modal(1);
}
