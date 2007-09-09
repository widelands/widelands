/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__EVENT_MESSAGE_BOX_OPTION_MENU_H
#define __S__EVENT_MESSAGE_BOX_OPTION_MENU_H

#include "ui_window.h"

#include <string>
#include <vector>

class Editor_Interactive;
class Event_Message_Box;
namespace UI {
template <typename T> struct Button;
struct Edit_Box;
struct Checkbox;
template <typename T> struct Listselect;
struct Multiline_Editbox;
struct Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Event_Message_Box_Option_Menu : public UI::Window {
      Event_Message_Box_Option_Menu(Editor_Interactive*, Event_Message_Box*);
      ~Event_Message_Box_Option_Menu();

	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
      static const int MAX_BUTTONS=4;

      void update();
	void clicked_ok                        ();
	void clicked_number_of_buttons_decrease();
	void clicked_number_of_buttons_increase();
	void clicked_trigger_sel_decrease      ();
	void clicked_trigger_sel_increase      ();
	void ls_selected    (uint);
      void edit_box_edited(int);

      Event_Message_Box* m_event;
      Editor_Interactive* m_parent;

      struct Button_Descr {
         std::string name;
         int trigger;
		} m_buttons[MAX_BUTTONS];

      UI::Checkbox* m_is_modal;
      UI::Edit_Box* m_caption;
      UI::Edit_Box* m_name;
      UI::Edit_Box* m_window_title;
      UI::Edit_Box* m_button_name;
      UI::Textarea* m_nr_buttons_ta;
	uint                     m_nr_buttons;
      int         m_ls_selected;
      UI::Multiline_Editbox* m_text;
      UI::Textarea* m_current_trigger_ta;
	UI::Listselect<void *> * m_buttons_ls;

      std::vector<int> m_null_triggers;
};

#endif
