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

#include "constants.h"
#include "ui_window.h"
#include "ui_modal_messagebox.h"
#include "ui_multilinetextarea.h"
#include "ui_button.h"
#include "graphic.h"
#include "wlapplication.h"

UIModal_Message_Box::UIModal_Message_Box(UIPanel* parent, std::string caption, std::string text, MB_Type type) :
   UIWindow(parent, 0, 0, 20, 20, caption.c_str()) {

   set_inner_size(320, 160);
   set_pos((parent->get_inner_w()-320)/2, (parent->get_inner_h()-100)/2);

   new UIMultiline_Textarea(this, 5, 5, get_inner_w()-10, get_inner_h()-70, text.c_str(), Align_Center);

   if(type==OK) {
      UIButton* but= new UIButton(this, (get_inner_w()-60)/2, get_inner_h()-30, 60, 20, 0, 0);
      but->clickedid.set(this, &UIModal_Message_Box::end_modal);
      but->set_title("OK");
   } else if(type==YESNO) {
      UIButton* but= new UIButton(this, ((get_inner_w()/2)-60)/2, get_inner_h()-30, 60, 20, 0, 1);
      but->clickedid.set(this, &UIModal_Message_Box::end_modal);
      but->set_title("YES");
      but= new UIButton(this, ((get_inner_w()/2)-60)/2+get_inner_w()/2, get_inner_h()-30, 60, 20, 1, 0);
      but->clickedid.set(this, &UIModal_Message_Box::end_modal);
      but->set_title("NO");
   }
}

UIModal_Message_Box::~UIModal_Message_Box(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we call end_modal() with NO (=0)
 */
bool UIModal_Message_Box::handle_mouseclick(uint btn, bool, int, int) {
   if(btn == MOUSE_RIGHT) {
      end_modal(0);
      return true;
   } else
      return false; // we're not dragable
}
