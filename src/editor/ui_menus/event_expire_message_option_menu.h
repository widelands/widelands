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

#ifndef EVENT_EXPIRE_MESSAGE_OPTION_MENU_H
#define EVENT_EXPIRE_MESSAGE_OPTION_MENU_H

#include "ui_basic/editbox.h"
#include "ui_basic/window.h"
#include "ui_basic/textarea.h"

#include "i18n.h"

#include "ref_cast.h"

struct Editor_Interactive;
namespace Widelands {struct Event_Expire_Message;}

struct Event_Expire_Message_Option_Menu : public UI::Window {
	Event_Expire_Message_Option_Menu
		(Editor_Interactive &, Widelands::Event_Expire_Message &);

private:
	Editor_Interactive & eia();
	Widelands::Event_Expire_Message & m_event;
	struct Label_Name : public UI::Textarea {
		Label_Name(Event_Expire_Message_Option_Menu & parent) :
			UI::Textarea(&parent, 5, 5, _("Name:"))
		{}
	} label_name;
	struct Name : public UI::EditBox {
		Name
			(Event_Expire_Message_Option_Menu & parent,
			 std::string const & event_name)
			:
			UI::EditBox
				(&parent,
				 parent.label_name.get_x() + parent.label_name.get_w() + 5, 5,
				 parent.get_inner_w() - parent.label_name.get_w() - 15, 20)
		{
			setText(event_name);
		}
	} name;
	struct OK : public UI::Button {
		OK(Event_Expire_Message_Option_Menu & parent) :
			UI::Button
				(&parent,
				 5, 30, 150, 20,
				 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
				 _("Ok"),
				 std::string())
		{}
		void clicked();
	} ok;
	struct Cancel : public UI::Button {
		Cancel(Event_Expire_Message_Option_Menu & parent) :
			UI::Button
				(&parent,
				 165, 30, 150, 20,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
				 _("Cancel"))
		{}
		virtual void clicked() {
			ref_cast<Event_Expire_Message_Option_Menu, UI::Panel>(*get_parent())
			.end_modal(0);
		}
	} cancel;
};

#endif
