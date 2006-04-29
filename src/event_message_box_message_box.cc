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
#include "game.h"
#include "graphic.h"
#include "editorinteractive.h"
#include "ui_multilinetextarea.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "system.h"
#include "constants.h"
#include "trigger_null.h"
#include "util.h"
#include "wlapplication.h"

/*
 * The message box himself
 */
Message_Box_Event_Message_Box::Message_Box_Event_Message_Box(Game* game, Event_Message_Box* event,
      int gposx, int gposy, int w, int h) :
UIWindow(game->get_iabase(), 0, 0, 600, 400, event->get_window_title() ) {

   m_game = game;

   UIMultiline_Textarea* m_text=0;
   int spacing=5;
   int offsy=5;
   int offsx=spacing;
   int posx=offsx;
   int posy=offsy;

   set_inner_size(w,h);
   m_text=new UIMultiline_Textarea(this, posx, posy, get_inner_w()-posx-spacing, get_inner_h()-posy-2*spacing-50, "", Align_Left);

   if(m_text)
      m_text->set_text( event->get_text());

   // Buttons
   int but_width=80;
   int space=get_inner_w()-2*spacing;
   space-=but_width*event->get_nr_buttons();
   space/=event->get_nr_buttons()+1;
   UIButton* b;
   posx=spacing;
   posy=get_inner_h()-30;
   m_trigger.resize(event->get_nr_buttons());
   for(int i=0; i<event->get_nr_buttons(); i++) {
      posx+=space;
      b=new UIButton(this, posx, posy, but_width, 20, 0, i);
      posx+=but_width;
      b->clickedid.set(this, &Message_Box_Event_Message_Box::clicked);
      b->set_title( event->get_button_name(i) );
      m_trigger[i]=event->get_button_trigger(i);
   }

   m_is_modal = event->get_is_modal();

   center_to_parent();

   if( gposx != -1 )
      set_pos( gposy, get_y());
   if( gposy != -1 )
      set_pos( get_x(), gposy);
}


/*
 * Handle mouseclick
 *
 * we might be a modal, therefore, if we are, we delete ourself through end_modal()
 * otherwise through die()
 */
bool Message_Box_Event_Message_Box::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT) {
      // We are not closable by right clicking
      // so that we are not closed by accidental scrolling
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
      if(m_is_modal) {
         end_modal(0);
         return;
      } else {
         die();
         return;
      }
   } else {
      // One of the buttons has been pressed
//      NoLog("Button %i has been pressed, nr of buttons: %i!\n", i, event->get_nr_buttons());
      Trigger_Null* t=m_trigger[i];
      if(t) {
         t->set_trigger_manually(true);
         t->check_set_conditions( m_game ); // forcefully update this trigger
      }
      clicked(-1);
      return;
   }
}
