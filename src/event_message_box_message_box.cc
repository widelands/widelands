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

#include "event_message_box_message_box.h"
#include "event_message_box.h"
#include "graphic.h"
#include "editorinteractive.h"
#include "ui_multilinetextarea.h"
#include "ui_button.h"
#include "system.h"

/*
 * The message box himself
 */
Message_Box_Event_Message_Box::Message_Box_Event_Message_Box(Editor_Game_Base* egbase, Event_Message_Box* event) :
  UIWindow(egbase->get_iabase(), 0, 0, 600, 400, event->get_window_title()) {
  
     m_event=event;
      UIMultiline_Textarea* m_text; 

      int spacing=5;
      log("Picture id is: %i\n", m_event->get_pic_id());
      if(static_cast<int>(m_event->get_pic_id())!=-1) {
        int picw, pich;
        g_gr->get_picture_size(m_event->get_pic_id(), &picw, &pich);
        
        //UIButton* b=new UIButton(this, get_inner_w()-picw-spacing, picw, 30, pich, 0, 0, 1); 
        UIButton* b=new UIButton(this, 0, 0, get_inner_w(), get_inner_h(), 0, 0, 0); 
//        b->set_enabled(false);
        b->set_pic(m_event->get_pic_id());

        m_text=new UIMultiline_Textarea(this, spacing, 30, get_inner_w()-picw-3*spacing, get_inner_h()-2*spacing, "", Align_Left);
     } else {
        m_text=new UIMultiline_Textarea(this, 0,0,get_w(),get_h(), "", Align_Left);
     }
     m_text->set_text(m_event->get_text());
     center_to_parent();
}


/*
 * Handle mouseclick
 *
 * we might be a modal, therefore, if we are, we delete ourself through end_modal()
 * otherwise through die()
 */
bool Message_Box_Event_Message_Box::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(-1);
      return true;
   } else
      return false; // we're not dragable 
}

/*
 * clicked
 */
void Message_Box_Event_Message_Box::clicked(int i) {
   if(i==-1) {
      // we should end this dialog
      if(m_event->get_is_modal()) {
         end_modal(0);
         return;
      } else {
         die();
         return;
      }
   }

}
