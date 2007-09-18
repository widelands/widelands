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

#include "editor_event_menu_new_event.h"

#include "constants.h"
#include "editorinteractive.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "i18n.h"
#include "map.h"
#include "map_event_manager.h"

#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_window.h"


Editor_Event_Menu_New_Event::Editor_Event_Menu_New_Event(Editor_Interactive* parent) :
UI::Window(parent, 0, 0, 400, 240, _("New Event").c_str()),
m_parent(parent)
{

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   // Event List
   new UI::Textarea(this, spacing, offsy, _("Available Events: "), Align_Left);
   m_event_list=new UI::Listselect<Event_Descr &>(this, spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55);
   m_event_list->selected.set(this, &Editor_Event_Menu_New_Event::selected);
   m_event_list->double_clicked.set(this, &Editor_Event_Menu_New_Event::double_clicked);

   uint i=0;
   for (i=0; i<Event_Factory::get_nr_of_available_events(); i++) {
		Event_Descr & d = *Event_Factory::get_event_descr(i);
		m_event_list->add(_(d.name).c_str(), d);
	}
   m_event_list->sort();

   // Descr List
   new UI::Textarea(this, (get_inner_w()/2)+spacing, offsy, _("Description: "), Align_Left);
   m_description=new UI::Multiline_Textarea(this, (get_inner_w()/2)+spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55, "", Align_Left);

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;

	m_ok_button = new UI::Button<Editor_Event_Menu_New_Event>
		(this,
		 posx, posy, 80, 20,
		 0,
		 &Editor_Event_Menu_New_Event::clicked_ok, this,
		 _("Ok"),
		 std::string(),
		 false);

   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Editor_Event_Menu_New_Event, int>
		(this,
		 posx, posy, 80, 20,
		 1,
		 &Editor_Event_Menu_New_Event::end_modal, this, 0,
		 _("Cancel"));

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_New_Event::~Editor_Event_Menu_New_Event() {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Editor_Event_Menu_New_Event::handle_mousepress
(const Uint8 btn, int, int)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Editor_Event_Menu_New_Event::handle_mouserelease(const Uint8, int, int)
{return false;}

/*
 * a button has been clicked
 */
void Editor_Event_Menu_New_Event::clicked_ok() {
   // Create new event
	Event * const event = Event_Factory::make_event_with_option_dialog
		(m_event_list->get_selected().id.c_str(), m_parent, 0);
	if (not event) return; //  No event created, choose another, user.
	m_parent->egbase().map().get_mem().register_new_event(event);
   end_modal(1);
   return;
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_New_Event::selected(uint) {
	m_description->set_text(i18n::translate(m_event_list->get_selected().descr));
   m_ok_button->set_enabled(true);
}

/*
 * listbox got double clicked
 */
void Editor_Event_Menu_New_Event::double_clicked(uint) {clicked_ok();}
