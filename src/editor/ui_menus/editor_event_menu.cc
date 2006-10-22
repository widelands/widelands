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

/*
===============
Editor_Event_Menu::Editor_Event_Menu

Create all the buttons etc...
===============
*/
Editor_Event_Menu::Editor_Event_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 620, 400, _("Event Menu"))
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Event Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   const int ls_width = 200;

   // EventChain List
   new UITextarea(this, posx, offsy, _("Event Chains: "), Align_Left);
   m_eventchain_list=new UIListselect(this, spacing, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_eventchain_list->selected.set(this, &Editor_Event_Menu::eventchain_list_selected);
   m_eventchain_list->double_clicked.set(this, &Editor_Event_Menu::eventchain_double_clicked);
   posx += ls_width + spacing;

   // Event List
   new UITextarea(this, posx, offsy, _("Registered Events: "), Align_Left);
   m_event_list=new UIListselect(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_event_list->selected.set(this, &Editor_Event_Menu::event_list_selected);
   m_event_list->double_clicked.set(this, &Editor_Event_Menu::event_double_clicked);
   posx += ls_width + spacing;

   // Trigger List
   new UITextarea(this, posx, offsy, _("Registered Triggers"), Align_Left);
   m_trigger_list=new UIListselect(this, posx, offsy+20, ls_width, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu::trigger_list_selected);
   m_trigger_list->double_clicked.set(this, &Editor_Event_Menu::trigger_double_clicked);
   posx += ls_width + spacing;

   posy=get_inner_h()-30;
   posx=spacing;
   UIButton* b=new UIButton(this, posx, posy, 90, 20, 4, 6);
   b->set_title(_("New Event Chain").c_str());
   b->clickedid.set(this, &Editor_Event_Menu::clicked);
   posx+=90+spacing;
   m_btn_edit_eventchain=new UIButton(this, posx, posy, 50, 20, 0, 8);
   m_btn_edit_eventchain->set_title(_("Edit").c_str());
   m_btn_edit_eventchain->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_edit_eventchain->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_eventchain=new UIButton(this, posx, posy, 50, 20, 0, 7);
   m_btn_del_eventchain->set_title(_("Del").c_str());
   m_btn_del_eventchain->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_del_eventchain->set_enabled(false);

   posx=spacing + ls_width + spacing;
   b=new UIButton(this, posx, posy, 90, 20, 4, 0);
   b->set_title(_("New Event").c_str());
   b->clickedid.set(this, &Editor_Event_Menu::clicked);
   posx+=90+spacing;
   m_btn_edit_event=new UIButton(this, posx, posy, 50, 20, 0, 2);
   m_btn_edit_event->set_title(_("Edit").c_str());
   m_btn_edit_event->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_edit_event->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_event=new UIButton(this, posx, posy, 50, 20, 0, 1);
   m_btn_del_event->set_title(_("Del").c_str());
   m_btn_del_event->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_del_event->set_enabled(false);


   posx= 3* spacing + 2*ls_width;
   b=new UIButton(this, posx, posy, 90, 20, 4, 3);
   b->set_title(_("New Trigger").c_str());
   b->clickedid.set(this, &Editor_Event_Menu::clicked);
   posx+=90+spacing;
   m_btn_edit_trigger=new UIButton(this, posx, posy, 50, 20, 0, 4);
   m_btn_edit_trigger->set_title(_("Edit").c_str());
   m_btn_edit_trigger->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_edit_trigger->set_enabled(false);
   posx+=50+spacing;
   m_btn_del_trigger=new UIButton(this, posx, posy, 50, 20, 0, 5);
   m_btn_del_trigger->set_title(_("Del").c_str());
   m_btn_del_trigger->clickedid.set(this, &Editor_Event_Menu::clicked);
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
			m_trigger_list->add_entry(trigger.get_name(), &trigger);
			if (trigger.get_referencers().empty() )
         m_trigger_list->set_entry_color( m_trigger_list->get_nr_entries()-1, RGBColor(255,0,0));
		}
   }

   m_event_list->clear();
	{
		const MapEventManager & mem = map.get_mem();
		const MapEventManager::Index nr_events = mem.get_nr_events();
		for (MapEventManager::Index i = 0; i < nr_events; ++i) {
			Event & event = mem.get_event_by_nr(i);
			m_event_list->add_entry(event.get_name(), &event);
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
			m_eventchain_list->add_entry(evc.get_name(), &evc);
		}
   }


   m_trigger_list->sort();
   m_event_list->sort();

   if(!m_trigger_list->get_selection()) {
      m_btn_del_trigger->set_enabled(false);
      m_btn_edit_trigger->set_enabled(false);
   }
   if(!m_event_list->get_selection()) {
      m_btn_del_event->set_enabled(false);
      m_btn_edit_event->set_enabled(false);
   }
}

/*
 * a button has been clicked
 */
void Editor_Event_Menu::clicked(int id) {
   if(id==0) {
      // New Event
      // Create the event if needed
      Editor_Event_Menu_New_Event* ntm=new Editor_Event_Menu_New_Event(m_parent);
      int retval=ntm->run();
      delete ntm;
      if(retval) {
         update();
         m_parent->set_need_save(true);
      }
   } else if(id==1) {
      // Delete event
      Event* event=static_cast<Event*>(m_event_list->get_selection());
      if(!event->get_referencers().empty()) {
         std::string str=_("Can't delete Event. It is in use by ");
         std::map<EventReferencer*,uint>::const_iterator i = event->get_referencers().begin();
         while( i != event->get_referencers().end() ) {
            str += i->first->get_type();
            str += ":";
            str += i->first->get_name();
            str += "\n";
            i++;
         }
         UIModal_Message_Box* mmb=new UIModal_Message_Box(m_parent, _("Error!"), str.c_str(), UIModal_Message_Box::OK);
         mmb->run();
         delete mmb;
         return;
      }

      m_parent->get_map()->get_mem().delete_event( event->get_name() );
      m_parent->unreference_player_tribe(0, event);  // Remove all references done by this event
      m_parent->set_need_save(true);
      update();
   } else if(id==2) {
      // Edit event
      Event* event=static_cast<Event*>(m_event_list->get_selection());
      Event_Factory::make_event_with_option_dialog(event->get_id(), m_parent, event);
      update();
   } else if(id==3) {
      // New Trigger
      Editor_Event_Menu_New_Trigger* ntm=new Editor_Event_Menu_New_Trigger(m_parent);
      int retval=ntm->run();
      if(retval)  {
         update();
         m_parent->set_need_save(true);
      }
      delete ntm;
   } else if(id==4) {
      // Edit trigger
      Trigger* trig=static_cast<Trigger*>(m_trigger_list->get_selection());
      trig=Trigger_Factory::make_trigger_with_option_dialog(trig->get_id(), m_parent, trig);
      m_parent->set_need_save(true);
      update();
   } else if(id==5) {
      // Delete trigger
      Trigger* trig=static_cast<Trigger*>(m_trigger_list->get_selection());
      if(!trig->get_referencers().empty()) {
         std::string str=_("Can't delete Trigger. It is in use by ");
         std::map<TriggerReferencer*,uint>::const_iterator i = trig->get_referencers().begin();
         while( i != trig->get_referencers().end() ) {
            str += i->first->get_type();
            str += ":";
            str += i->first->get_name();
            str += "\n";
            i++;
         }
         UIModal_Message_Box* mmb=new UIModal_Message_Box(m_parent, _("Error!"), str.c_str(), UIModal_Message_Box::OK);
         mmb->run();
         delete mmb;
         return;
      }
      m_parent->unreference_player_tribe(0, trig);  // Remove all references done by this trigger
      m_parent->get_egbase()->get_map()->get_mtm().delete_trigger(trig->get_name());
      m_parent->set_need_save(true);
      update();
   } else if( id == 6) {
      // First, create new TriggerConditional
      EventChain* ev = new EventChain();
      Editor_Event_Menu_Edit_TriggerConditional* menu = new Editor_Event_Menu_Edit_TriggerConditional( m_parent, 0, ev );
      int code = menu->run();
      if( code ) { // TriggerConditional has been accepted
         ev->set_trigcond( menu->get_trigcond());

         // Get the a name
         char buffer[256];

         int n = 1;
         const Map & map = *m_parent->get_egbase()->get_map();
         while( 1 ) {
		 snprintf(buffer, sizeof(buffer), "%s%i", _("Unnamed").c_str(), n);
            if (not map.get_mecm().get_eventchain(buffer))
               break;
            ++n;
         }

         ev->set_name( buffer );
         map.get_mecm().register_new_eventchain(ev);
	 m_eventchain_list->add_entry( _("Unnamed").c_str(), ev, true);
         m_eventchain_list->sort();
      } else {
         // TriggerConditional was not accepted. Remove this EventChain straithly.
         // No dereferencing of triggers is needed, since they are not referenced at all on cancel
         delete ev;
      }
      delete menu;
      if( code )
         clicked(8); // Simulate click on edit
   } else if( id == 7) {
      // Delete event chain
      EventChain* ev = static_cast<EventChain*>(m_eventchain_list->get_selection( ));
      m_parent->get_egbase()->get_map()->get_mecm().delete_eventchain(ev->get_name());
      m_eventchain_list->remove_entry( m_eventchain_list->get_selection_index() );
      m_btn_del_eventchain->set_enabled( false );
      m_btn_edit_eventchain->set_enabled( false );
      update();
   } else if( id == 8) {
      // Edit event chain
      EventChain* ev = static_cast<EventChain*>(m_eventchain_list->get_selection( ));
      Editor_Event_Menu_Edit_EventChain* menu = new Editor_Event_Menu_Edit_EventChain( m_parent, ev );
      menu->run();
      delete menu;
      update();
   }
}

/*
 * listbox was selected
 */
void Editor_Event_Menu::trigger_list_selected(int) {
   m_btn_del_trigger->set_enabled(true);
   m_btn_edit_trigger->set_enabled(true);
}
void Editor_Event_Menu::event_list_selected(int) {
   m_btn_del_event->set_enabled(true);
   m_btn_edit_event->set_enabled(true);
}
void Editor_Event_Menu::eventchain_list_selected(int) {
   m_btn_del_eventchain->set_enabled(true);
   m_btn_edit_eventchain->set_enabled(true);
}

/*
 * listbox was double clicked
 */
void Editor_Event_Menu::trigger_double_clicked   (int) {clicked(4);}
void Editor_Event_Menu::event_double_clicked     (int) {clicked(2);}
void Editor_Event_Menu::eventchain_double_clicked(int) {clicked(8);}
