/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_EVENT_MENU_EDIT_TRIGGER_CONDITIONAL_H
#define EDITOR_EVENT_MENU_EDIT_TRIGGER_CONDITIONAL_H

#include "trigger/trigger_conditional.h"

#include "ui_window.h"

struct Editor_Interactive;
namespace UI {
template <typename T> struct Listselect;
template <typename T> struct Button;
};

/**
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Editor_Event_Menu_Edit_TriggerConditional : public UI::Window {
	Editor_Event_Menu_Edit_TriggerConditional
		(Editor_Interactive &,
		 Widelands::TriggerConditional *,
		 Widelands::EventChain *);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	Widelands::TriggerConditional * get_trigcond() {return m_given_cond;}

private:
	Editor_Interactive & eia();
	void clicked_cancel     ();
	void clicked_ok         ();
	void clicked_operator   (Widelands::TriggerConditional_Factory::TokenNames);
	void clicked_ins_trigger();
	void clicked_del_trigger();
	void clicked_move_up    ();
	void clicked_move_down  ();
	void tl_selected        (uint32_t);
	void tl_double_clicked  (uint32_t);
	void cs_selected        (uint32_t);
	void cs_double_clicked  (uint32_t);

	UI::Listselect<Widelands::Trigger                  &> * m_trigger_list;
	UI::Listselect<Widelands::TriggerConditional_Factory::Token &>
		* m_construction;
	UI::Button<Editor_Event_Menu_Edit_TriggerConditional> * m_insert_btn;
	UI::Button<Editor_Event_Menu_Edit_TriggerConditional> * m_delete_btn;
	UI::Button<Editor_Event_Menu_Edit_TriggerConditional> * m_mvup_btn;
	UI::Button<Editor_Event_Menu_Edit_TriggerConditional> * m_mvdown_btn;
	Widelands::TriggerConditional                         * m_given_cond;
	Widelands::EventChain                                 * m_event_chain;
};

#endif
