/*
 * Copyright (C) 2003 by the Widelands Development Team
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
#include "widelands.h"
#include "ui.h"
#include "ui_editbox.h"

/*
=================================================

class Edit_Box

=================================================
*/

/*
===========
Edit_Box::Edit_Box()

constructor
===========
*/
Edit_Box::Edit_Box(Panel* parent, int x, int y, uint w, uint h, uint background, int id) :
   Button(parent, x, y, w, h, background, id) {
   
   set_think(false);

   m_keyboard_grabbed=false;
   m_maxchars=0xffff;
   m_text="";
   m_lasttext="";

   m_id=id;

   set_handle_mouse(true);
   set_can_focus(true);
}

/*
===========
Edit_Box::~Edit_Box()

destructor
===========
*/
Edit_Box::~Edit_Box(void) {
   grab_mouse(false);
   set_can_focus(false);
}

/*
===========
Edit_Box::handle_mouseclick()

the mouse was clicked on this editbox
===========
*/
bool Edit_Box::handle_mouseclick(uint btn, bool down, int x, int y) {
   if(btn!=0) return false;
   
   if(down && !m_keyboard_grabbed) {
      set_can_focus(true);
      grab_mouse(true);
      Button::handle_mouseclick(btn, down, x, y);
      focus();
      m_keyboard_grabbed=true;
      m_lasttext=m_text;
      return true;
   }
   if(m_keyboard_grabbed) return true;
   return false;
}

/*
===========
Edit_Box::handle_key() 

a key event must be handled
===========
*/
bool Edit_Box::handle_key(bool down, int code, char c) {
   if(down) {
      switch(code) {
         case KEY_ESCAPE: 
            set_text(m_lasttext.c_str()); 
            Button::handle_mouseclick(0, false, 0, 0); 
            set_can_focus(false);
            m_keyboard_grabbed=false;
            grab_mouse(false);
            return true; 

         case KEY_RETURN: 
            m_lasttext=m_text;
            Button::handle_mouseclick(0, false, 0, 0);
            set_can_focus(false);
            m_keyboard_grabbed=false;
            grab_mouse(false);
            changed.call();
            changedid.call(m_id);
            return true; 

         case KEY_DELETE:
         case KEY_BACKSPACE:
            if(m_text.size()) {
               m_text.erase(m_text.end() - 1);
               set_text(m_text.c_str());
            }
            return true;

         default:
            if(c && m_text.size()<m_maxchars) m_text+=c;
            set_text(m_text.c_str());
            return true;
      } 
   }

   return false;
}

/*
===========
Edit_Box::handle_mousemove()

handles the mousemove for this panel.
does not much, suppresses messages when the focus
is received
===========
*/
void Edit_Box::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns) {
   if(m_keyboard_grabbed) return;
   else Button::handle_mousemove(x,y,xdiff,ydiff,btns);
}

/*
===========
Edit_Box::handle_mousein()

Handles mouseins or rather mouse outs. 
Hides a mouseout event from the underlying button
===========
*/
void Edit_Box::handle_mousein(bool inside) {
   if(m_keyboard_grabbed) return;
   Button::handle_mousein(inside);
}
