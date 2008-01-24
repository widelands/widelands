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

#include "editor_event_menu.h"

#include "events/event.h"
#include "events/event_chain.h"
#include "events/event_factory.h"
#include "editor_event_menu_edit_eventchain.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "editor_event_menu_new_trigger.h"
#include "editor_event_menu_new_event.h"
#include "editorinteractive.h"
#include "i18n.h"
#include "trigger/trigger.h"
#include "map.h"
#include "trigger/trigger_factory.h"

#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"

#include <sstream>

using Widelands::EventChain;
using Widelands::Trigger;

inline Editor_Interactive & Editor_Event_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Editor_Event_Menu::Editor_Event_Menu
(Editor_Interactive & parent, UI::UniqueWindow::Registry * registry)
:
UI::UniqueWindow(&parent, registry, 620, 400, _("Event Menu"))
{
   const int32_t offsx=5;
   const int32_t offsy=25;
   const int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;
   const int32_t ls_width = 200;

   // EventChain List
   new UI::Textarea(this, posx, offsy, _("Event Chains: "), Align_Left);
   m_eventchain_list=new UI::Listselect<Widelands::EventChain*>(this, spacing, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_eventchain_list->selected.set(this, &Editor_Event_Menu::eventchain_list_selected);
   m_eventchain_list->double_clicked.set(this, &Editor_Event_Menu::eventchain_double_clicked);
   posx += ls_width + spacing;

   // Event List
   new UI::Textarea(this, posx, offsy, _("Registered Events: "), Align_Left);
   m_event_list=new UI::Listselect<Widelands::Event*>(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_event_list->selected.set(this, &Editor_Event_Menu::event_list_selected);
   m_event_list->double_clicked.set(this, &Editor_Event_Menu::event_double_clicked);
   posx += ls_width + spacing;

   // Trigger List
   new UI::Textarea(this, posx, offsy, _("Registered Triggers"), Align_Left);
   m_trigger_list=new UI::Listselect<Widelands::Trigger*>(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu::trigger_list_selected);
   m_trigger_list->double_clicked.set(this, &Editor_Event_Menu::trigger_double_clicked);
   posx += ls_width + spacing;

   posy=get_inner_h()-30;
   posx=spacing;

	new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20, 4,
		 &Editor_Event_Menu::clicked_new_eventchain, this,
		 _("New Event Chain"));

   posx+=90+spacing;

	m_btn_edit_eventchain = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_edit_eventchain, this,
		 _("Edit"),
		 std::string(),
		 false);

   posx+=50+spacing;

	m_btn_del_eventchain = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_del_eventchain, this,
		 _("Del"),
		 std::string(),
		 false);

   posx=spacing + ls_width + spacing;

	new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20,
		 4,
		 &Editor_Event_Menu::clicked_new_event, this,
		 _("New Event"));

   posx+=90+spacing;

	m_btn_edit_event = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_edit_event, this,
		 _("Edit"),
		 std::string(),
		 false);

   posx+=50+spacing;

	m_btn_del_event = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_del_event, this,
		 _("Del"),
		 std::string(),
		 false);


   posx= 3* spacing + 2*ls_width;

	new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20,
		 4,
		 &Editor_Event_Menu::clicked_new_trigger, this,
		 _("New Trigger"));

   posx+=90+spacing;

	m_btn_edit_trigger = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_edit_trigger, this,
		 _("Edit"),
		 std::string(),
		 false);

   posx+=50+spacing;

	m_btn_del_trigger = new UI::Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 0,
		 &Editor_Event_Menu::clicked_del_trigger, this,
		 _("Del"),
		 std::string(),
		 false);

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
   update();
}

/*
===============
Editor_Event_Menu::~Editor_Event_Menu

Unregister from the registry pointer
===============
*/
Editor_Event_Menu::~Editor_Event_Menu()
{
}

/*
 * update all lists and stuff
 */
void Editor_Event_Menu::update() {
	Widelands::Map & map = eia().egbase().map();

	m_trigger_list->clear();
	{
		Manager<Trigger> & mtm = map.mtm();
		Manager<Trigger>::Index const nr_triggers = mtm.size();
		for (Manager<Trigger>::Index i = 0; i < nr_triggers; ++i) {
			Trigger & trigger = mtm[i];
			m_trigger_list->add(trigger.name().c_str(), &trigger);
			if (trigger.referencers().empty())
				m_trigger_list->set_entry_color
					(m_trigger_list->size() - 1, RGBColor(255, 0, 0));
		}
	}

   m_event_list->clear();
	{
		Manager<Widelands::Event> & mem = map.mem();
		Manager<Widelands::Event>::Index const nr_events = mem.size();
		for (Manager<Widelands::Event>::Index i = 0; i < nr_events; ++i) {
			Widelands::Event & event = mem[i];
			m_event_list->add(event.name().c_str(), &event);
			if (event.referencers().empty())
				m_event_list->set_entry_color
					(m_event_list->size() - 1, RGBColor(255, 0, 0));
		}
	}

   m_eventchain_list->clear();
	{
		Manager<EventChain> & mcm = map.mcm();
		Manager<EventChain>::Index const nr_eventchains = mcm.size();
		for (Manager<EventChain>::Index i = 0; i < nr_eventchains; ++i) {
			Widelands::EventChain & evc = mcm[i];
			m_eventchain_list->add(evc.name().c_str(), &evc);
		}
	}


   m_trigger_list->sort();
   m_event_list->sort();

	if (not m_trigger_list->has_selection()) {
      m_btn_del_trigger->set_enabled(false);
      m_btn_edit_trigger->set_enabled(false);
	}
	if (not m_event_list->has_selection()) {
      m_btn_del_event->set_enabled(false);
      m_btn_edit_event->set_enabled(false);
	}
}


void Editor_Event_Menu::clicked_new_event() {
      // Create the event if needed
	Editor_Event_Menu_New_Event ntm(eia());
	if (ntm.run()) {
         update();
		eia().set_need_save(true);
	}
}


void Editor_Event_Menu::clicked_del_event() {
	Widelands::Event & event = *m_event_list->get_selected();
	Widelands::Event::Referencers const & event_referencers =
		event.referencers();
	if (event_referencers.empty()) {
		eia().egbase().map().mem().remove(event);
		eia().unreference_player_tribe(0, &event);  // Remove all references done by this event
		eia().set_need_save(true);
      update();
	} else {
		std::ostringstream s(_("Can't delete Event. It is in use by "));
		Widelands::Event::Referencers::const_iterator
			event_referencers_end = event_referencers.end();
		for
			(Widelands::Event::Referencers::const_iterator it =
			 event_referencers.begin();
			 it != event_referencers_end;
			 ++it)
			s << it->first->identifier() << '\n';
		UI::Modal_Message_Box mmb
			(&eia(), _("Error!"), s.str(), UI::Modal_Message_Box::OK);
		mmb.run();
         return;
	}

}


void Editor_Event_Menu::clicked_edit_event() {
	m_event_list->get_selected()->option_menu(eia());
      update();
}


void Editor_Event_Menu::clicked_new_trigger() {
	Editor_Event_Menu_New_Trigger ntm(eia());
	if (ntm.run())  {
         update();
		eia().set_need_save(true);
	}
}


void Editor_Event_Menu::clicked_del_trigger() {
	Widelands::Trigger & trigger = *m_trigger_list->get_selected();
	Widelands::Trigger::Referencers const & trigger_referencers =
		trigger.referencers();
	if (trigger_referencers.empty()) {
		eia().unreference_player_tribe(0, &trigger);  // Remove all references done by this trigger
		eia().egbase().map().mtm().remove(trigger);
		eia().set_need_save(true);
      update();
	} else {
		std::ostringstream s(_("Can't delete Trigger. It is in use by "));
		Widelands::Trigger::Referencers::const_iterator const
			trigger_referencers_end = trigger_referencers.end();
		for
			(Widelands::Trigger::Referencers::const_iterator it =
			 trigger_referencers.begin();
			 it != trigger_referencers_end;
			 ++it)
			s << it->first->identifier() << '\n';
		UI::Modal_Message_Box messagebox
			(&eia(), _("Error!"), s.str(), UI::Modal_Message_Box::OK);
		messagebox.run();
	}
}


void Editor_Event_Menu::clicked_edit_trigger() {
	m_trigger_list->get_selected()->option_menu(eia());
	eia().set_need_save(true);
      update();
}


void Editor_Event_Menu::clicked_new_eventchain() {
      // First, create new TriggerConditional
	Widelands::EventChain & evch = *new Widelands::EventChain("");
	Editor_Event_Menu_Edit_TriggerConditional menu(eia(), 0, &evch);
	if (menu.run()) { // TriggerConditional has been accepted
		evch.set_trigcond(*menu.get_trigcond());

         // Get the a name
         char buffer[256];

		Widelands::Map & map = eia().egbase().map();
		for (uint32_t n = 1;; ++n) {
			snprintf(buffer, sizeof(buffer), "%s%u", _("Unnamed").c_str(), n);
			if (not map.mcm()[buffer])
				break;
		}

		evch.set_name(buffer);
		map.mcm().register_new(evch);
		m_eventchain_list->add(_("Unnamed").c_str(), &evch, -1, true);
		m_eventchain_list->sort();
		clicked_edit_eventchain();
	} else
         // TriggerConditional was not accepted. Remove this EventChain straithly.
         // No dereferencing of triggers is needed, since they are not referenced at all on cancel
		delete &evch;
}


void Editor_Event_Menu::clicked_del_eventchain() {
	eia().egbase().map().mcm().remove
		(m_eventchain_list->get_selected()->name());
	m_eventchain_list->remove_selected();
      m_btn_del_eventchain->set_enabled(false);
      m_btn_edit_eventchain->set_enabled(false);
      update();
}


void Editor_Event_Menu::clicked_edit_eventchain() {
	Editor_Event_Menu_Edit_EventChain menu
		(eia(), *m_eventchain_list->get_selected());
	menu.run();
      update();
}

/*
 * listbox was selected
 */
void Editor_Event_Menu::trigger_list_selected(uint32_t) {
   m_btn_del_trigger->set_enabled(true);
   m_btn_edit_trigger->set_enabled(true);
}
void Editor_Event_Menu::event_list_selected(uint32_t) {
   m_btn_del_event->set_enabled(true);
   m_btn_edit_event->set_enabled(true);
}
void Editor_Event_Menu::eventchain_list_selected(uint32_t) {
   m_btn_del_eventchain->set_enabled(true);
   m_btn_edit_eventchain->set_enabled(true);
}

/*
 * listbox was double clicked
 */
void Editor_Event_Menu::trigger_double_clicked(uint32_t) {clicked_edit_trigger();}
void Editor_Event_Menu::event_double_clicked  (uint32_t) {clicked_edit_event  ();}
void Editor_Event_Menu::eventchain_double_clicked(uint32_t)
{clicked_edit_eventchain();}
