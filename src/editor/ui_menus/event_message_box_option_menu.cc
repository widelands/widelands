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

#include "event_message_box_option_menu.h"
#include "event_message_box.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_multilineeditbox.h"
#include "editorinteractive.h"
#include "system.h"
#include "error.h"

Event_Message_Box_Option_Menu::Event_Message_Box_Option_Menu(Editor_Interactive* parent, Event_Message_Box* event) :
   UIWindow(parent, 0, 0, 164, 180, "Event Option Menu") {
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Message Box Event Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   m_event=event;
   
   new UITextarea(this, spacing, offsy, "Only run once: ", Align_Left);
   UICheckbox* cb=new UICheckbox(this, spacing+130, offsy-3);
   m_is_one_time_event=event->is_one_time_event();
   cb->set_state(m_is_one_time_event);
   cb->changedto.set(this, &Event_Message_Box_Option_Menu::cb_changed);
   posy+=20+spacing;

   new UITextarea(this, spacing, posy, 50, 20, "Name:", Align_CenterLeft);
   m_name=new UIEdit_Box(this, spacing+50, posy, get_inner_w()-50-2*spacing, 20, 0, 0);
   m_name->set_text(event->get_name());
   
   posy+=20+spacing;

   posx=spacing;
   m_textarea=new UIMultiline_Editbox(this, posx, posy, get_inner_w()-posx, get_inner_h()-posx-60, event->get_string());
   
   posy+=2*spacing;
   // Buttons
   posx=(get_inner_w()/2)-60-spacing;
   UIButton* b=new UIButton(this, posx, posy, 60, 20, 0, 1);
   b->set_title("Ok");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);
   posx=(get_inner_w()/2)+spacing;
   b=new UIButton(this, posx, posy, 60, 20, 1, 0);
   b->set_title("Cancel");
   b->clickedid.set(this, &Event_Message_Box_Option_Menu::clicked);

   center_to_parent();
}

/*
 * cleanup
 */
Event_Message_Box_Option_Menu::~Event_Message_Box_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself 
 * on close (the caller must do this) instead 
 * we simulate a cancel click
 */
bool Event_Message_Box_Option_Menu::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable 
}
  
/*
 * a button has been clicked
 */
void Event_Message_Box_Option_Menu::clicked(int i) {
   if(!i) {
      // Cancel has been clicked
      end_modal(0);
      return;
   } else if(i==1) {
      // ok button
      m_event->set_is_one_time_event(m_is_one_time_event);
      if(m_name->get_text())
         m_event->set_name(m_name->get_text());
      if(m_textarea->get_text().c_str())
         m_event->set_string(m_textarea->get_text().c_str());
      end_modal(1);
      return;
   }
}

/*
 * checkbox has been changed
 */
void Event_Message_Box_Option_Menu::cb_changed(bool t) {
   m_is_one_time_event=t;
}
