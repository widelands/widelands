/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
#include "editorinteractive.h"
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "error.h"
#include "editor_event_menu_new_trigger.h"
#include "editor_event_menu_new_event.h"
#include "editor_event_menu_choose_trigger.h"
#include "trigger.h"
#include "map.h"
#include "trigger_factory.h"

/*
===============
Editor_Event_Menu::Editor_Event_Menu

Create all the buttons etc...
===============
*/
Editor_Event_Menu::Editor_Event_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 400, 300, "Event Menu")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Event Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   // Event List
   new UITextarea(this, spacing, offsy, "Registered Events: ", Align_Left);
   m_event_list=new UIListselect(this, spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55);
   m_event_list->selected.set(this, &Editor_Event_Menu::event_list_selected);

   // Trigger List
   new UITextarea(this, (get_inner_w()/2)+spacing, offsy, "Registered Triggers", Align_Left);
   m_trigger_list=new UIListselect(this, (get_inner_w()/2)+spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu::trigger_list_selected);

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;
   UIButton* b=new UIButton(this, posx, posy, 80, 20, 4, 0);
   b->set_title("New Event");
   b->clickedid.set(this, &Editor_Event_Menu::clicked);
   posx-=45+spacing;
   m_btn_del_event=new UIButton(this, posx, posy, 45, 20, 0, 1);
   m_btn_del_event->set_title("Del");
   m_btn_del_event->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_del_event->set_enabled(false);
   posx-=45+spacing;
   m_btn_edit_event=new UIButton(this, posx, posy, 45, 20, 0, 2);
   m_btn_edit_event->set_title("Edit");
   m_btn_edit_event->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_edit_event->set_enabled(false);


   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 80, 20, 4, 3);
   b->set_title("New Trigger");
   b->clickedid.set(this, &Editor_Event_Menu::clicked);
   posx+=80+spacing;
   m_btn_edit_trigger=new UIButton(this, posx, posy, 45, 20, 0, 4);
   m_btn_edit_trigger->set_title("Edit");
   m_btn_edit_trigger->clickedid.set(this, &Editor_Event_Menu::clicked);
   m_btn_edit_trigger->set_enabled(false);
   posx+=45+spacing;
   m_btn_del_trigger=new UIButton(this, posx, posy, 45, 20, 0, 5);
   m_btn_del_trigger->set_title("Del");
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
   Trigger* trig=0;
   m_trigger_list->clear();
   int i=0;
   for(i=0; i<m_parent->get_map()->get_number_of_triggers(); i++) {
      trig=m_parent->get_map()->get_trigger(i);
      m_trigger_list->add_entry(trig->get_name(), trig);
   }

   Event* event=0;
   m_event_list->clear();
   for(i=0; i<m_parent->get_map()->get_number_of_events(); i++) {
      event=m_parent->get_map()->get_event(i);
      m_event_list->add_entry(event->get_name(), event);
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
   if(id<3) {
      if(id==0) {
         // New Event
         Map* map=m_parent->get_map();
         // Create the event if needed
         Editor_Event_Menu_New_Event* ntm=new Editor_Event_Menu_New_Event(m_parent);
         int retval=ntm->run();
         delete ntm;
         if(retval) {
            Editor_Event_Menu_Choose_Trigger* ntm=new Editor_Event_Menu_Choose_Trigger(m_parent, map->get_event(map->get_number_of_events()-1));
            ntm->run();
            delete ntm;
            update();
            m_parent->set_need_save(true);
         }
      } else if(id==1) {
         // Delete event
         Event* event=static_cast<Event*>(m_event_list->get_selection());
         // event unregisters itself from the map
         event->cleanup(m_parent->get_egbase());
         // Some paranoia
         m_parent->get_map()->delete_unreferenced_triggers();
         m_parent->set_need_save(true);
         update();
      } else if(id==2) {
         Event* event=static_cast<Event*>(m_event_list->get_selection());
         Editor_Event_Menu_Choose_Trigger* ntm=new Editor_Event_Menu_Choose_Trigger(m_parent, event);
         if(ntm->run())
            m_parent->set_need_save(true);
         delete ntm;
         update();
      }
   } else {
      if(id==3) {
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
         trig->decr_reference();
         if(!trig->is_unreferenced()) {
            trig->incr_reference();
            UIModal_Message_Box* mmb=new UIModal_Message_Box(m_parent, "Error!", "Can't delete Trigger. It is in use!", UIModal_Message_Box::OK);
            mmb->run();
            delete mmb;
            return;
         }
         m_parent->get_map()->unregister_trigger(trig);
         m_parent->set_need_save(true);
         update();
      }
   }
}

/*
 * listbox was selected
 */
void Editor_Event_Menu::trigger_list_selected(int i) {
   m_btn_del_trigger->set_enabled(true);
   m_btn_edit_trigger->set_enabled(true);
}
void Editor_Event_Menu::event_list_selected(int i) {
   m_btn_del_event->set_enabled(true);
   m_btn_edit_event->set_enabled(true);
}

