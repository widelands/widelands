/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "editor_event_menu_edit_eventchain.h"

#include "constants.h"
#include "editor/editorinteractive.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "events/event.h"
#include "events/event_chain.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "trigger/trigger_conditional.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"


inline Editor_Interactive & Editor_Event_Menu_Edit_EventChain::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Editor_Event_Menu_Edit_EventChain::Editor_Event_Menu_Edit_EventChain
	(Editor_Interactive & parent, Widelands::EventChain & chain)
:
UI::Window   (&parent, 0, 0, 505, 340, _("Edit Event Chain")),
m_event_chain(chain)
{
	int32_t const spacing  =   5;
	int32_t const ls_width = 200;
	Point         pos        (5, 5);

	new UI::Textarea
		(this, pos.x, pos.y, 60, 20, _("Name: "), UI::Align_CenterLeft);
	m_name =
		new UI::EditBox
			(this, pos.x + 60, pos.y, get_inner_w() - 2 * spacing - 60, 20,
			 g_gr->get_picture(PicMod_UI, "pics/but0.png"), 0);
	m_name->setText(m_event_chain.name());
	pos.y += 20 + spacing;

	new UI::Textarea
		(this,
		 pos.x + STATEBOX_WIDTH + spacing, pos.y, 120, STATEBOX_HEIGHT,
		 _("Runs multiple times"), UI::Align_CenterLeft);
	m_morethanonce = new UI::Checkbox(this, pos);
	m_morethanonce->set_state(m_event_chain.get_repeating());
	pos.y += STATEBOX_HEIGHT + spacing;
	int32_t const lsoffsy = pos.y;

	new UI::Textarea(this, pos.x, lsoffsy, _("Events: "), UI::Align_Left);
	m_events =
		new UI::Listselect<Widelands::Event &>
			(this, spacing, lsoffsy + 20, ls_width, get_inner_h() - lsoffsy - 55);
	m_events->selected.set
		(this, &Editor_Event_Menu_Edit_EventChain::cs_selected);
	m_events->double_clicked.set
		(this, &Editor_Event_Menu_Edit_EventChain::cs_double_clicked);
	pos.x += ls_width + spacing;

	pos.y = 75;

	new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_edit_trigger_contitional,
		 *this,
		 _("Conditional"));

	pos.y += 20 + spacing + spacing;

	new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_new_event, *this,
		 _("New Event"));

	pos.y += 20 + spacing + spacing;

	m_insert_btn = new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_ins_event, *this,
		 "<-",
		 _("Insert"),
		 false);

	pos.y += 20 + spacing + spacing;

	m_delete_btn = new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_del_event, *this,
		 _("Delete"),
		 std::string(),
		 false);

	pos.y += 20 + spacing + spacing + spacing;

	m_mvup_btn = new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x + 5, pos.y, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_move_up, *this,
		 _("Up"),
		 false);

	m_mvdown_btn = new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x + 51, pos.y, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_move_down, *this,
		 std::string(),
		 false);

	pos += Point(80 + spacing, 24 + spacing + spacing);
	new UI::Textarea(this, pos.x, lsoffsy, _("Available Events: "));
	m_available_events =
		new UI::Listselect<Widelands::Event &>
			(this, pos.x, lsoffsy + 20, ls_width, get_inner_h() - lsoffsy - 55);
	m_available_events->selected.set
		(this, &Editor_Event_Menu_Edit_EventChain::tl_selected);
	m_available_events->double_clicked.set
		(this, &Editor_Event_Menu_Edit_EventChain::tl_double_clicked);
	Manager<Widelands::Event> & mem = parent.egbase().map().mem();
	Manager<Widelands::Event>::Index const nr_events = mem.size();
	for (Manager<Widelands::Event>::Index i = 0; i < nr_events; ++i) {
		Widelands::Event & event = mem[i];
		m_available_events->add(event.name().c_str(), event);
	}
	m_available_events->sort();

	pos = Point(get_inner_w() / 2 - 80 - spacing, get_inner_h() - 30);

	new UI::Callback_Button<Editor_Event_Menu_Edit_EventChain>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_Edit_EventChain::clicked_ok, *this,
		 _("Ok"));

	pos.x = get_inner_w() / 2 + spacing;

	new UI::Callback_IDButton<Editor_Event_Menu_Edit_EventChain, int32_t>
		(this,
		 pos.x, pos.y, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Editor_Event_Menu_Edit_EventChain::end_modal, *this, 0,
		 _("Cancel"));

	for (uint32_t i = 0; i < m_event_chain.get_nr_events(); ++i) {
		Widelands::Event & event = *m_event_chain.get_event(i);
		m_events->add(event.name().c_str(), event);
	}

	m_edit_trigcond = m_event_chain.get_trigcond() ? false : true;

	center_to_parent();
}


/**
 * Think.
 *
 * Maybe we have to simulate a click
 */
void Editor_Event_Menu_Edit_EventChain::think()
{
	if (m_edit_trigcond)
		clicked_edit_trigger_contitional();
}

void Editor_Event_Menu_Edit_EventChain::clicked_ok() {
	m_event_chain.set_name(m_name->text());
	m_event_chain.set_repeating(m_morethanonce->get_state());
	//  trigger conditional is always updated
	//  events
	m_event_chain.clear_events();
	const uint32_t nr_events = m_events->size();
	for (uint32_t i = 0; i < nr_events; ++i)
		m_event_chain.add_event(&(*m_events)[i]);
	end_modal(1);
}

void Editor_Event_Menu_Edit_EventChain::clicked_new_event() {
	//  TODO
}

void Editor_Event_Menu_Edit_EventChain::clicked_edit_trigger_contitional() {
	Editor_Event_Menu_Edit_TriggerConditional menu
		(eia(), m_event_chain.get_trigcond(), &m_event_chain);
	if (menu.run()) {
		if (m_event_chain.get_trigcond()) {
			m_event_chain.get_trigcond()->unreference_triggers(m_event_chain);
			delete m_event_chain.get_trigcond();
		}
		m_event_chain.set_trigcond(*menu.get_trigcond());
	}
}


void Editor_Event_Menu_Edit_EventChain::clicked_ins_event() {
	Widelands::Event & event = m_available_events->get_selected();
	m_events->add(event.name().c_str(), event, g_gr->get_no_picture(), true);
}


void Editor_Event_Menu_Edit_EventChain::clicked_del_event() {
	m_events    ->remove_selected();
	m_mvup_btn  ->set_enabled(false);
	m_mvdown_btn->set_enabled(false);
	m_delete_btn->set_enabled(false);
}


void Editor_Event_Menu_Edit_EventChain::clicked_move_up() {
	assert(m_events->has_selection());  //  Button should have been disabled.
	const uint32_t n = m_events->selection_index();
	assert(n != 0);  //  Button should have been disabled.
	m_events->switch_entries(n, n - 1);
}


void Editor_Event_Menu_Edit_EventChain::clicked_move_down() {
	assert(m_events->has_selection());  //  Button should have been disabled.
	const uint32_t n = m_events->selection_index();
	assert(n < m_events->size() - 1);  //  Button should have been disabled.
	m_events->switch_entries(n, n + 1);
}

/**
 * the listbox got selected
 */
void Editor_Event_Menu_Edit_EventChain::tl_selected(uint32_t) {
	m_insert_btn->set_enabled(true);
}
void Editor_Event_Menu_Edit_EventChain::cs_selected(uint32_t) {
	m_mvdown_btn->set_enabled(true);
	m_mvup_btn  ->set_enabled(true);
	m_delete_btn->set_enabled(true);
}

/**
 * Listbox got double clicked
 */
void Editor_Event_Menu_Edit_EventChain::tl_double_clicked(uint32_t)
{
	clicked_ins_event();
}

void Editor_Event_Menu_Edit_EventChain::cs_double_clicked(uint32_t)
{
	clicked_del_event();
}
