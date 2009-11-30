/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#ifndef EVENT_SET_PLAYER_FRONTIER_STYLE_OPTION_MENU_H
#define EVENT_SET_PLAYER_FRONTIER_STYLE_OPTION_MENU_H

#include "editor/editorinteractive.h"

#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/window.h"
#include "ui_basic/textarea.h"

namespace Widelands {struct Event_Set_Player_Frontier_Style;}

struct Event_Set_Player_Frontier_Style_Option_Menu : public UI::Window {
	Event_Set_Player_Frontier_Style_Option_Menu
		(Editor_Interactive &, Widelands::Event_Set_Player_Frontier_Style &);

	void draw(RenderTarget &);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void change_player(bool up);

	Widelands::Event_Set_Player_Frontier_Style & m_event;
	Widelands::Player_Number                     m_player_number;
	uint8_t                                      m_style_index;
	struct Label_Name : public UI::Textarea {
		Label_Name(Event_Set_Player_Frontier_Style_Option_Menu & parent) :
			UI::Textarea(&parent, 5, 5, _("Name:"))
		{}
	} label_name;
	struct Name : public UI::EditBox {
		Name
			(Event_Set_Player_Frontier_Style_Option_Menu & parent,
			 std::string const & event_name)
			:
			UI::EditBox
				(&parent,
				 parent.label_name.get_x() + parent.label_name.get_w() + 5, 5,
				 parent.get_inner_w() - parent.label_name.get_w() - 15, 20)
		{
			setText(event_name);
		}
	} name;
	struct Label_Player : public UI::Textarea {
		Label_Player(Event_Set_Player_Frontier_Style_Option_Menu & parent) :
			UI::Textarea(&parent, 5, 50, _("Player:"))
		{}
	} label_player;
	struct Decrement_Player : public UI::Button {
		Decrement_Player(Event_Set_Player_Frontier_Style_Option_Menu & parent)
			:
			UI::Button
				(&parent,
				 65, 50, 20, 20,
				 g_gr->get_no_picture(),
				 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
				 std::string(),
				 1 < parent.eia().egbase().map().get_nrplayers())
		{
			set_repeating(true);
		}
		void clicked() {
			ref_cast<Event_Set_Player_Frontier_Style_Option_Menu, UI::Panel>
				(*get_parent())
			.change_player(false);
		}
	} decrement_player;
	struct Increment_Player : public UI::Button {
		Increment_Player(Event_Set_Player_Frontier_Style_Option_Menu & parent)
			:
			UI::Button
				(&parent,
				 115, 50, 20, 20,
				 g_gr->get_no_picture(),
				 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
				 std::string(),
				 1 < parent.eia().egbase().map().get_nrplayers())
		{
			set_repeating(true);
		}
		void clicked() {
			ref_cast<Event_Set_Player_Frontier_Style_Option_Menu, UI::Panel>
				(*get_parent())
			.change_player(true);
		}
	} increment_player;
	struct OK : public UI::Button {
		OK(Event_Set_Player_Frontier_Style_Option_Menu & parent) :
			UI::Button
				(&parent,
				 5, 95, 150, 20,
				 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
				 _("Ok"))
		{}
		void clicked();
	} ok;
	struct Cancel : public UI::Button {
		Cancel(Event_Set_Player_Frontier_Style_Option_Menu & parent) :
			UI::Button
				(&parent,
				 165, 95, 150, 20,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
				 _("Cancel"))
		{}
		void clicked() {
			ref_cast<Event_Set_Player_Frontier_Style_Option_Menu, UI::Panel>
				(*get_parent())
			.end_modal(0);
		}
	} cancel;
};

#endif
