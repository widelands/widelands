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

#include "editor_event_menu_choose_trigger.h"
#include "editorinteractive.h"
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "error.h"
#include "map.h"
#include "event.h"
#include "editor_event_menu_new_event.h"
#include "system.h"
#include "trigger.h"
#include "event_factory.h"

/*
===============
Editor_Event_Menu_Choose_Trigger::Editor_Event_Menu_Choose_Trigger

Create all the buttons etc...
===============
*/
Editor_Event_Menu_Choose_Trigger::Editor_Event_Menu_Choose_Trigger(Editor_Interactive *parent, Event* event)
	: UIWindow(parent, 0, 0, 380, 270, "Choose Triggers")
{
   m_parent=parent;
   m_event=event;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Choose Triggers for Event", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   // Event List
   new UITextarea(this, spacing, offsy, "Registered Triggers: ", Align_Left);
   m_selected=new UIListselect(this, spacing, offsy+20, (get_inner_w()/2)-2*spacing-20, get_inner_h()-offsy-85);
   m_selected->selected.set(this, &Editor_Event_Menu_Choose_Trigger::selected_list_selected);
   m_selected->double_clicked.set(this, &Editor_Event_Menu_Choose_Trigger::selected_list_double_clicked);
   
   // Trigger List
   new UITextarea(this, (get_inner_w()/2)+spacing, offsy, "Available Triggers", Align_Left);
   m_available=new UIListselect(this, (get_inner_w()/2)+spacing+20, offsy+20, (get_inner_w()/2)-2*spacing-20, get_inner_h()-offsy-85);
   m_available->selected.set(this, &Editor_Event_Menu_Choose_Trigger::available_list_selected);
   m_available->double_clicked.set(this, &Editor_Event_Menu_Choose_Trigger::available_list_double_clicked);

   // Toggle when to run button
   new UITextarea(this, spacing, get_inner_h()-60+spacing, 200, 20, "Event runs when trigger is: ", Align_Center);

   m_btn_toggle_event=new UIButton(this, spacing+200,  get_inner_h()-60+spacing, 60, 20, 1, 5);
   m_btn_toggle_event->set_title("---");
   m_btn_toggle_event->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);

   // OK button
   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-120-spacing;
   UIButton* b=new UIButton(this, posx, posy, 80, 20, 0, 1);
   b->set_title("OK");
   b->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);

   // Cancel button
   posx=(get_inner_w()/2)+40+spacing;
   b=new UIButton(this, posx, posy, 80, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);
   posx+=80+spacing;

   // Event options
   posx=(get_inner_w()/2)-40;
   b=new UIButton(this, posx, posy, 80, 20, 1, 4);
   b->set_title("Options");
   b->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);

   // Left to right button
   posx=get_inner_w()/2-20;
   posy=offsy+50;
   m_btn_ltor=new UIButton(this, posx, posy, 40, 20, 1, 2);
   m_btn_ltor->set_title("->");
   m_btn_ltor->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);

   // Right to left button
   posx=get_inner_w()/2-20;
   posy=offsy+70+spacing;
   m_btn_rtol=new UIButton(this, posx, posy, 40, 20, 1, 3);
   m_btn_rtol->set_title("<-");
   m_btn_rtol->clickedid.set(this, &Editor_Event_Menu_Choose_Trigger::clicked);

   center_to_parent();

   // Fill the list boxes, construct our data
   int i;
   Map* map=m_parent->get_map();
   for(i=0; i<map->get_number_of_triggers(); i++) {
      Trigger_Data* data=new Trigger_Data();
      data->trig=map->get_trigger(i);
      data->run_enabled=true;
      if(m_event->trigger_exists(data->trig)) {
         data->run_enabled=m_event->reacts_when_trigger_is_set(data->trig);
         m_selected->add_entry(data->trig->get_name(), data);
      } else {
         m_available->add_entry(data->trig->get_name(), data);
      }
   }

   update();
}

/*
===============
Editor_Event_Menu_Choose_Trigger::~Editor_Event_Menu_Choose_Trigger

Unregister from the registry pointer
===============
*/
Editor_Event_Menu_Choose_Trigger::~Editor_Event_Menu_Choose_Trigger()
{
   int i=0;
   for(i=0; i<m_selected->get_nr_entries(); i++) {
      m_selected->select(i);
      delete static_cast<Trigger_Data*>(m_selected->get_selection());
   }
   for(i=0; i<m_available->get_nr_entries(); i++) {
      m_available->select(i);
      delete static_cast<Trigger_Data*>(m_available->get_selection());
   }
}

/*
 * update all user interface stuff
 */
void Editor_Event_Menu_Choose_Trigger::update(void) {

    m_available->sort();
   m_selected->sort();

   if(m_available->get_selection()==0)
      m_btn_rtol->set_enabled(false);
   if(m_selected->get_selection()==0) {
      m_btn_ltor->set_enabled(false);
      m_btn_toggle_event->set_enabled(false);
      m_btn_toggle_event->set_title("---");
   } else {
      Trigger_Data* t=static_cast<Trigger_Data*>(m_selected->get_selection());
      if(t->run_enabled)
         m_btn_toggle_event->set_title("set");
      else
         m_btn_toggle_event->set_title("unset");
   }
}

/*
 * handle mouseclick, so that we can be run as
 * a modal
 */
bool Editor_Event_Menu_Choose_Trigger::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void Editor_Event_Menu_Choose_Trigger::clicked(int id) {
   switch(id) {
      case 0:
         {
            end_modal(0);
            return;
         }

   case 1:
      {
         // OK Button
         int i;
         for(i=0; i<m_selected->get_nr_entries(); i++) {
            m_selected->select(i);
            Trigger_Data* t=static_cast<Trigger_Data*>(m_selected->get_selection());
            if(!m_event->trigger_exists(t->trig))
                  m_event->register_trigger(t->trig, m_parent->get_map(), t->run_enabled);
            m_event->set_reacts_when_trigger_is_set(t->trig, t->run_enabled);
         }
         for(i=0; i<m_available->get_nr_entries(); i++) {
            m_available->select(i);
            Trigger_Data* t=static_cast<Trigger_Data*>(m_available->get_selection());
            if(m_event->trigger_exists(t->trig))
                  m_event->unregister_trigger(t->trig, m_parent->get_map());
         }
         end_modal(1);
         return;
      }
      break;

   case 2:
      {
         // Left to right button
         Trigger_Data* t=static_cast<Trigger_Data*>(m_selected->get_selection());
         m_selected->remove_entry(m_selected->get_selection_index());
         m_available->add_entry(t->trig->get_name(), t);
         update();
      }
      break;

   case 3:
      {
         // Right to left button
         Trigger_Data* t=static_cast<Trigger_Data*>(m_available->get_selection());
         m_available->remove_entry(m_available->get_selection_index());
         m_selected->add_entry(t->trig->get_name(), t);
         update();
      }
      break;

   case 4:
      {
         // Option Button
         Event_Factory::make_event_with_option_dialog(m_event->get_id(), m_parent, m_event);
      }
      break;

   case 5:
      {
         // Toggle button
         Trigger_Data* t=static_cast<Trigger_Data*>(m_selected->get_selection());
         t->run_enabled=!t->run_enabled;
         update();
      }
   }
}

/*
 * listbox was selected
 */
void Editor_Event_Menu_Choose_Trigger::available_list_selected(int i) {
   m_btn_rtol->set_enabled(true);
   update();
}
void Editor_Event_Menu_Choose_Trigger::selected_list_selected(int i) {
   m_btn_toggle_event->set_enabled(true);
   m_btn_ltor->set_enabled(true);
   update();
}

/*
 * listbox was double clicked
 */
void Editor_Event_Menu_Choose_Trigger::available_list_double_clicked(int i) {
   // click right to left button
   clicked(3);
}
void Editor_Event_Menu_Choose_Trigger::selected_list_double_clicked(int i) {
   // click left to right button
   clicked(2);
}
