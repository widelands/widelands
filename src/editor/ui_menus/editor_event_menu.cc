/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "event.h"
#include "event_chain.h"
#include "event_factory.h"
#include "editor_event_menu.h"
#include "editorinteractive.h"
#include "i18n.h"
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "error.h"
#include "editor_event_menu_edit_eventchain.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "editor_event_menu_new_trigger.h"
#include "editor_event_menu_new_event.h"
#include "trigger.h"
#include "trigger_referencer.h"
#include "map.h"
#include "map_event_manager.h"
#include "map_eventchain_manager.h"
#include "map_trigger_manager.h"
#include "trigger_factory.h"
#include "util.h"

#include <sstream>

/*
===============
Editor_Event_Menu::Editor_Event_Menu

Create all the buttons etc...
===============
*/
Editor_Event_Menu::Editor_Event_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
	: UI::UniqueWindow(parent, registry, 620, 400, _("Event Menu"))
{
   m_parent=parent;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("Event Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   const int ls_width = 200;

   // EventChain List
   new UI::Textarea(this, posx, offsy, _("Event Chains: "), Align_Left);
   m_eventchain_list=new UI::Listselect<EventChain &>(this, spacing, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_eventchain_list->selected.set(this, &Editor_Event_Menu::eventchain_list_selected);
   m_eventchain_list->double_clicked.set(this, &Editor_Event_Menu::eventchain_double_clicked);
   posx += ls_width + spacing;

   // Event List
   new UI::Textarea(this, posx, offsy, _("Registered Events: "), Align_Left);
   m_event_list=new UI::Listselect<Event &>(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_event_list->selected.set(this, &Editor_Event_Menu::event_list_selected);
   m_event_list->double_clicked.set(this, &Editor_Event_Menu::event_double_clicked);
   posx += ls_width + spacing;

   // Trigger List
   new UI::Textarea(this, posx, offsy, _("Registered Triggers"), Align_Left);
   m_trigger_list=new UI::Listselect<Trigger &>(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu::trigger_list_selected);
   m_trigger_list->double_clicked.set(this, &Editor_Event_Menu::trigger_double_clicked);
   posx += ls_width + spacing;

   posy=get_inner_h()-30;
   posx=spacing;
   UI::Button* b=new UI::Button(this, posx, posy, 90, 20, 4);
   b->set_title(_("New Event Chain").c_str());
   b->clicked.set(this, &Editor_Event_Menu::clicked_new_eventchain);
   posx+=90+spacing;
   m_btn_edit_eventchain=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_edit_eventchain->set_title(_("Edit").c_str());
   m_btn_edit_eventchain->clicked.set(this, &Editor_Event_Menu::clicked_edit_eventchain);
   m_btn_edit_eventchain->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_eventchain=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_del_eventchain->set_title(_("Del").c_str());
   m_btn_del_eventchain->clicked.set(this, &Editor_Event_Menu::clicked_del_eventchain);
   m_btn_del_eventchain->set_enabled(false);

   posx=spacing + ls_width + spacing;
   b=new UI::Button(this, posx, posy, 90, 20, 4);
   b->set_title(_("New Event").c_str());
   b->clicked.set(this, &Editor_Event_Menu::clicked_new_event);
   posx+=90+spacing;
   m_btn_edit_event=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_edit_event->set_title(_("Edit").c_str());
   m_btn_edit_event->clicked.set(this, &Editor_Event_Menu::clicked_edit_event);
   m_btn_edit_event->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_event=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_del_event->set_title(_("Del").c_str());
   m_btn_del_event->clicked.set(this, &Editor_Event_Menu::clicked_del_event);
   m_btn_del_event->set_enabled(false);


   posx= 3* spacing + 2*ls_width;
   b=new UI::Button(this, posx, posy, 90, 20, 4);
   b->set_title(_("New Trigger").c_str());
   b->clicked.set(this, &Editor_Event_Menu::clicked_new_trigger);
   posx+=90+spacing;
   m_btn_edit_trigger=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_edit_trigger->set_title(_("Edit").c_str());
   m_btn_edit_trigger->clicked.set(this, &Editor_Event_Menu::clicked_edit_trigger);
   m_btn_edit_trigger->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_trigger=new UI::Button(this, posx, posy, 50, 20, 0);
   m_btn_del_trigger->set_title(_("Del").c_str());
   m_btn_del_trigger->clicked.set(this, &Editor_Event_Menu::clicked_del_trigger);
   m_btn_del_trigger->set_enabled(false);

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
void Editor_Event_Menu::update(void) {
	const Map & map = *m_parent->get_map();

	m_trigger_list->clear();
	{
		const MapTriggerManager & mtm = map.get_mtm();
		const MapTriggerManager::Index nr_triggers = mtm.get_nr_triggers();
		for (MapTriggerManager::Index i = 0; i < nr_triggers; ++i) {
			Trigger & trigger = mtm.get_trigger_by_nr(i);
			m_trigger_list->add_entry(trigger.get_name(), trigger);
			if (trigger.get_referencers().empty())
         m_trigger_list->set_entry_color( m_trigger_list->get_nr_entries()-1, RGBColor(255,0,0));
		}
   }

   m_event_list->clear();
	{
		const MapEventManager & mem = map.get_mem();
		const MapEventManager::Index nr_events = mem.get_nr_events();
		for (MapEventManager::Index i = 0; i < nr_events; ++i) {
			Event & event = mem.get_event_by_nr(i);
			m_event_list->add_entry(event.get_name(), event);
			if (event.get_referencers().empty())
         m_event_list->set_entry_color( m_event_list->get_nr_entries()-1, RGBColor(255,0,0));
		}
   }

   m_eventchain_list->clear();
	{
		const MapEventChainManager & mecm = map.get_mecm();
		const MapEventChainManager::Index nr_eventchains =
			mecm.get_nr_eventchains();
		for (MapEventChainManager::Index i = 0; i < nr_eventchains; ++i) {
			EventChain & evc = mecm.get_eventchain_by_nr(i);
			m_eventchain_list->add_entry(evc.get_name(), evc);
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
      Editor_Event_Menu_New_Event* ntm=new Editor_Event_Menu_New_Event(m_parent);
      int retval=ntm->run();
      delete ntm;
      if(retval) {
         update();
         m_parent->set_need_save(true);
      }
}


void Editor_Event_Menu::clicked_del_event() {
	const Event & event = m_event_list->get_selection();
	const Event::EventReferencerMap & event_referencers =
		event.get_referencers();
	if (event_referencers.empty()) {
		m_parent->get_map()->get_mem().delete_event(event.get_name());
		m_parent->unreference_player_tribe(0, &event);  // Remove all references done by this event
      m_parent->set_need_save(true);
      update();
	} else {
		std::ostringstream s(_("Can't delete Event. It is in use by "));
		Event::EventReferencerMap::const_iterator event_referencers_end =
			event_referencers.end();
		for
			(Event::EventReferencerMap::const_iterator it =
			 event_referencers.begin();
			 it != event_referencers_end;
			 ++it)
			s << it->first->get_type() << ':' << it->first->get_name() << '\n';
		UI::Modal_Message_Box* mmb=new UI::Modal_Message_Box(m_parent, _("Error!"), s.str(), UI::Modal_Message_Box::OK);
         mmb->run();
         delete mmb;
         return;
	}

}


void Editor_Event_Menu::clicked_edit_event() {
	Event & event = m_event_list->get_selection();
	Event_Factory::make_event_with_option_dialog
		(event.get_id(), m_parent, &event);
      update();
}


void Editor_Event_Menu::clicked_new_trigger() {
      Editor_Event_Menu_New_Trigger* ntm=new Editor_Event_Menu_New_Trigger(m_parent);
      int retval=ntm->run();
      if(retval)  {
         update();
         m_parent->set_need_save(true);
      }
      delete ntm;
	}


void Editor_Event_Menu::clicked_del_trigger() {
	Trigger & trigger = m_trigger_list->get_selection();
	const Trigger::TriggerReferencerMap & trigger_referencers =
		trigger.get_referencers();
	if (trigger_referencers.empty()) {
		m_parent->unreference_player_tribe(0, &trigger);  // Remove all references done by this trigger
		m_parent->get_egbase()->get_map()->get_mtm().delete_trigger(trigger.get_name());
      m_parent->set_need_save(true);
      update();
	} else {
		std::ostringstream s(_("Can't delete Trigger. It is in use by "));
		const Trigger::TriggerReferencerMap::const_iterator
			trigger_referencers_end = trigger_referencers.end();
		for
			(Trigger::TriggerReferencerMap::const_iterator it =
			 trigger_referencers.begin();
			 it != trigger_referencers_end;
			 ++it)
			s << it->first->get_type() << ':' << it->first->get_name() << '\n';
		UI::Modal_Message_Box messagebox
			(m_parent, _("Error!"), s.str(), UI::Modal_Message_Box::OK);
		messagebox.run();
	}
}


void Editor_Event_Menu::clicked_edit_trigger() {
	Trigger & trigger = m_trigger_list->get_selection();
	Trigger_Factory::make_trigger_with_option_dialog
		(trigger.get_id(), m_parent, &trigger);
      m_parent->set_need_save(true);
      update();
}


void Editor_Event_Menu::clicked_new_eventchain() {
      // First, create new TriggerConditional
      EventChain* ev = new EventChain();
      Editor_Event_Menu_Edit_TriggerConditional* menu = new Editor_Event_Menu_Edit_TriggerConditional( m_parent, 0, ev );
      int code = menu->run();
      if( code ) { // TriggerConditional has been accepted
         ev->set_trigcond( menu->get_trigcond());

         // Get the a name
         char buffer[256];

         int n = 1;
		Map & map = m_parent->get_egbase()->map();
         while( 1 ) {
		 snprintf(buffer, sizeof(buffer), "%s%i", _("Unnamed").c_str(), n);
            if (not map.get_mecm().get_eventchain(buffer))
               break;
            ++n;
         }

         ev->set_name( buffer );
         map.get_mecm().register_new_eventchain(ev);
	 m_eventchain_list->add_entry( _("Unnamed").c_str(), *ev, true);
         m_eventchain_list->sort();
      } else {
         // TriggerConditional was not accepted. Remove this EventChain straithly.
         // No dereferencing of triggers is needed, since they are not referenced at all on cancel
         delete ev;
      }
      delete menu;
	if (code) clicked_edit_eventchain();
}


void Editor_Event_Menu::clicked_del_eventchain() {
	m_parent->get_egbase()->get_map()->get_mecm().delete_eventchain
		(m_eventchain_list->get_selection().get_name());
      m_eventchain_list->remove_entry( m_eventchain_list->get_selection_index() );
      m_btn_del_eventchain->set_enabled( false );
      m_btn_edit_eventchain->set_enabled( false );
      update();
}


void Editor_Event_Menu::clicked_edit_eventchain() {
	Editor_Event_Menu_Edit_EventChain menu(m_parent, &m_eventchain_list->get_selection());
	menu.run();
      update();
}

/*
 * listbox was selected
 */
void Editor_Event_Menu::trigger_list_selected(uint) {
   m_btn_del_trigger->set_enabled(true);
   m_btn_edit_trigger->set_enabled(true);
}
void Editor_Event_Menu::event_list_selected(uint) {
   m_btn_del_event->set_enabled(true);
   m_btn_edit_event->set_enabled(true);
}
void Editor_Event_Menu::eventchain_list_selected(uint) {
   m_btn_del_eventchain->set_enabled(true);
   m_btn_edit_eventchain->set_enabled(true);
}

/*
 * listbox was double clicked
 */
void Editor_Event_Menu::trigger_double_clicked(uint) {clicked_edit_trigger();}
void Editor_Event_Menu::event_double_clicked  (uint) {clicked_edit_event  ();}
void Editor_Event_Menu::eventchain_double_clicked(uint)
{clicked_edit_eventchain();}
