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

#include "editor_event_menu_new_trigger.h"
#include "ui_window.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "editorinteractive.h"
#include "system.h"
#include "trigger.h"
#include "trigger_factory.h"
#include "error.h"

Editor_Event_Menu_New_Trigger::Editor_Event_Menu_New_Trigger(Editor_Interactive* parent) :
   UIWindow(parent, 0, 0, 400, 240, "New Trigger") {
  m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "New Trigger Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;
   
   // Trigger List
   new UITextarea(this, spacing, offsy, "Available Triggers: ", Align_Left);
   m_trigger_list=new UIListselect(this, spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55);
   m_trigger_list->selected.set(this, &Editor_Event_Menu_New_Trigger::selected);
   uint i=0; 
   for(i=0; i<Trigger_Factory::get_nr_of_available_triggers(); i++) {
      Trigger_Descr* d=Trigger_Factory::get_correct_trigger_descr(i);
      m_trigger_list->add_entry(d->name, d);
   }

   // Descr List
   new UITextarea(this, (get_inner_w()/2)+spacing, offsy, "Description: ", Align_Left);
   m_description=new UIMultiline_Textarea(this, (get_inner_w()/2)+spacing, offsy+20, (get_inner_w()/2)-2*spacing, get_inner_h()-offsy-55, "", Align_Left);

   posy=get_inner_h()-30;
   posx=(get_inner_w()/2)-80-spacing;
   m_ok_button=new UIButton(this, posx, posy, 80, 20, 0, 1);
   m_ok_button->set_title("Ok");
   m_ok_button->clickedid.set(this, &Editor_Event_Menu_New_Trigger::clicked);
   m_ok_button->set_enabled(0);
   posx=(get_inner_w()/2)+spacing;
   UIButton* b=new UIButton(this, posx, posy, 80, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Editor_Event_Menu_New_Trigger::clicked);

   center_to_parent();
}

/*
 * cleanup
 */
Editor_Event_Menu_New_Trigger::~Editor_Event_Menu_New_Trigger(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself 
 * on close (the caller must do this) instead 
 * we simulate a cancel click
 */
bool Editor_Event_Menu_New_Trigger::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable 
}
  
/*
 * a button has been clicked
 */
void Editor_Event_Menu_New_Trigger::clicked(int i) {
   if(!i) {
      // Cancel has been clicked
      end_modal(0);
      return;
   }
   
   Trigger_Descr* d=static_cast<Trigger_Descr*>(m_trigger_list->get_selection());
   // Create new trigger
   Trigger* trig=
      Trigger_Factory::make_trigger_with_option_dialog(d->id, m_parent, 0);
   if(!trig) {
      end_modal(0); 
      return; 
   }
   log("TODO: register new trigger with map: %s!\n", trig->get_name());
}

/*
 * the listbox got selected
 */
void Editor_Event_Menu_New_Trigger::selected(int i) {
   Trigger_Descr* d=Trigger_Factory::get_correct_trigger_descr(i);
   m_description->set_text(d->descr);
   m_ok_button->set_enabled(true);
}

