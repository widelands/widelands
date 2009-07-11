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

#include "editor_event_menu.h"

#include "events/event.h"
#include "events/event_chain.h"
#include "events/event_factory.h"
#include "editor_event_menu_edit_eventchain.h"
#include "editor_event_menu_edit_trigger_conditional.h"
#include "editor_event_menu_new_trigger.h"
#include "editor_event_menu_new_event.h"
#include "editor/editorinteractive.h"
#include "i18n.h"
#include "trigger/trigger.h"
#include "map.h"
#include "trigger/trigger_factory.h"

#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/messagebox.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

#include <sstream>

using Widelands::EventChain;
using Widelands::Trigger;

inline Editor_Interactive & Editor_Event_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Editor_Event_Menu::Editor_Event_Menu
	(Editor_Interactive & parent, UI::UniqueWindow::Registry * registry)
:
UI::UniqueWindow(&parent, registry, 620, 400, _("Event Menu"))
{
	int32_t const offsx    =   5;
	int32_t const offsy    =  25;
	int32_t const spacing  =   5;
	int32_t       posx     = offsx;
	int32_t       posy     = offsy;
	int32_t const ls_width = 200;

	new UI::Textarea(this, posx, offsy, _("Event Chains: "));
	m_eventchain_list =
		new UI::Listselect<Widelands::EventChain *>
		(this, spacing, offsy + 20, ls_width, get_inner_h() - offsy - 55);
	m_eventchain_list->selected.set
		(this, &Editor_Event_Menu::eventchain_list_selected);
	m_eventchain_list->double_clicked.set
		(this, &Editor_Event_Menu::eventchain_double_clicked);
	posx += ls_width + spacing;

	new UI::Textarea(this, posx, offsy, _("Registered Events: "));
	m_event_list =
		new UI::Listselect<Widelands::Event *>
			(this, posx, offsy + 20, ls_width, get_inner_h() - offsy - 55);
	m_event_list->selected.set(this, &Editor_Event_Menu::event_list_selected);
	m_event_list->double_clicked.set
		(this, &Editor_Event_Menu::event_double_clicked);
	posx += ls_width + spacing;

	new UI::Textarea(this, posx, offsy, _("Registered Triggers"));
	m_trigger_list =
		new UI::Listselect<Widelands::Trigger *>
		(this, posx, offsy + 20, ls_width, get_inner_h() - offsy - 55);
	m_trigger_list->selected.set
		(this, &Editor_Event_Menu::trigger_list_selected);
	m_trigger_list->double_clicked.set
		(this, &Editor_Event_Menu::trigger_double_clicked);
	posx += ls_width + spacing;

	posy = get_inner_h() - 30;
	posx = spacing;

	new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 &Editor_Event_Menu::clicked_new_eventchain, *this,
		 _("New Event Chain"));

	posx += 90 + spacing;

	m_btn_edit_eventchain = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_edit_eventchain, *this,
		 _("Edit"),
		 std::string(),
		 false);

	posx += 50 + spacing;

	m_btn_del_eventchain = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_del_eventchain, *this,
		 _("Del"),
		 std::string(),
		 false);

	posx = spacing + ls_width + spacing;

	new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 &Editor_Event_Menu::clicked_new_event, *this,
		 _("New Event"));

	posx += 90 + spacing;

	m_btn_edit_event = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_edit_event, *this,
		 _("Edit"),
		 std::string(),
		 false);

	posx += 50 + spacing;

	m_btn_del_event = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_del_event, *this,
		 _("Del"),
		 std::string(),
		 false);


	posx = 3 * spacing + 2 * ls_width;

	new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 90, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 &Editor_Event_Menu::clicked_new_trigger, *this,
		 _("New Trigger"));

	posx += 90 + spacing;

	m_btn_edit_trigger = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_edit_trigger, *this,
		 _("Edit"),
		 std::string(),
		 false);

	posx += 50 + spacing;

	m_btn_del_trigger = new UI::Callback_Button<Editor_Event_Menu>
		(this,
		 posx, posy, 50, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu::clicked_del_trigger, *this,
		 _("Del"),
		 std::string(),
		 false);

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
	update();
}


/**
 * Update all lists and stuff
 */
void Editor_Event_Menu::update() {
	Widelands::Map & map = eia().egbase().map();

	m_trigger_list->clear();
	{
		Manager<Trigger> & mtm = map.mtm();
		Manager<Trigger>::Index const nr_triggers = mtm.size();
		for (Manager<Trigger>::Index i = 0; i < nr_triggers; ++i) {
			Trigger & trigger = mtm[i];
			m_trigger_list->add(trigger.name().c_str(), &trigger);
			if (trigger.referencers().empty())
				m_trigger_list->set_entry_color
					(m_trigger_list->size() - 1, RGBColor(255, 0, 0));
		}
	}

	m_event_list->clear();
	{
		Manager<Widelands::Event> & mem = map.mem();
		Manager<Widelands::Event>::Index const nr_events = mem.size();
		for (Manager<Widelands::Event>::Index i = 0; i < nr_events; ++i) {
			Widelands::Event & event = mem[i];
			m_event_list->add(event.name().c_str(), &event);
			if (event.referencers().empty())
				m_event_list->set_entry_color
					(m_event_list->size() - 1, RGBColor(255, 0, 0));
		}
	}

	m_eventchain_list->clear();
	{
		Manager<EventChain> & mcm = map.mcm();
		Manager<EventChain>::Index const nr_eventchains = mcm.size();
		for (Manager<EventChain>::Index i = 0; i < nr_eventchains; ++i) {
			Widelands::EventChain & evc = mcm[i];
			m_eventchain_list->add(evc.name().c_str(), &evc);
		}
	}


	m_trigger_list->sort();
	m_event_list  ->sort();

	if (not m_trigger_list->has_selection()) {
		m_btn_del_trigger ->set_enabled(false);
		m_btn_edit_trigger->set_enabled(false);
	}
	if (not m_event_list->has_selection()) {
		m_btn_del_event ->set_enabled(false);
		m_btn_edit_event->set_enabled(false);
	}
}


void Editor_Event_Menu::clicked_new_event() {
	//  Create the event if needed.
	Editor_Event_Menu_New_Event ntm(eia());
	if (ntm.run()) {
		update();
		eia().set_need_save(true);
	}
}


void Editor_Event_Menu::clicked_del_event() {
	Widelands::Event & event = *m_event_list->get_selected();
	Widelands::Event::Referencers const & event_referencers =
		event.referencers();
	if (event_referencers.empty()) {
		eia().egbase().map().mem().remove(event);

		// Remove all references done by this event
		eia().unreference_player_tribe(0, &event);
		eia().set_need_save(true);
		update();
	} else {
		std::ostringstream s(_("Can not delete event. It is in use by "));
		Widelands::Event::Referencers::const_iterator
			event_referencers_end = event_referencers.end();
		container_iterate_const
			(Widelands::Event::Referencers, event_referencers, i)
			s << i.current->first->identifier() << '\n';
		UI::WLMessageBox mmb
			(&eia(), _("Error!"), s.str(), UI::WLMessageBox::OK);
		mmb.run();
		return;
	}
}


void Editor_Event_Menu::clicked_edit_event() {
	m_event_list->get_selected()->option_menu(eia());
	update();
}


void Editor_Event_Menu::clicked_new_trigger() {
	Editor_Event_Menu_New_Trigger ntm(eia());
	if (ntm.run())  {
		update();
		eia().set_need_save(true);
	}
}


void Editor_Event_Menu::clicked_del_trigger() {
	Widelands::Trigger & trigger = *m_trigger_list->get_selected();
	Widelands::Trigger::Referencers const & trigger_referencers =
		trigger.referencers();
	if (trigger_referencers.empty()) {
		// Remove all references done by this trigger
		eia().unreference_player_tribe(0, &trigger);
		eia().egbase().map().mtm().remove(trigger);
		eia().set_need_save(true);
		update();
	} else {
		std::ostringstream s(_("Can not delete trigger. It is in use by "));
		Widelands::Trigger::Referencers::const_iterator const
			trigger_referencers_end = trigger_referencers.end();
		container_iterate_const
			(Widelands::Trigger::Referencers, trigger_referencers, i)
			s << i.current->first->identifier() << '\n';
		UI::WLMessageBox messagebox
			(&eia(), _("Error!"), s.str(), UI::WLMessageBox::OK);
		messagebox.run();
	}
}


void Editor_Event_Menu::clicked_edit_trigger() {
	m_trigger_list->get_selected()->option_menu(eia());
	eia().set_need_save(true);
	update();
}


void Editor_Event_Menu::clicked_new_eventchain() {
	//  First, create new TriggerConditional.
	Widelands::EventChain & evch = *new Widelands::EventChain("");
	Editor_Event_Menu_Edit_TriggerConditional menu(eia(), 0, &evch);
	if (menu.run()) { // TriggerConditional has been accepted
		evch.set_trigcond(*menu.get_trigcond());

		char buffer[256];

		Widelands::Map & map = eia().egbase().map();
		for (uint32_t n = 1;; ++n) {
			snprintf(buffer, sizeof(buffer), _("Unnamed%u"), n);
			if (not map.mcm()[buffer])
				break;
		}

		evch.set_name(buffer);
		map.mcm().register_new(evch);
		m_eventchain_list->add(_("Unnamed"), &evch, g_gr->get_no_picture(), true);
		m_eventchain_list->sort();
		clicked_edit_eventchain();
	} else
		//  TriggerConditional was not accepted. Remove this EventChain.
		//  No dereferencing of triggers is needed, since they are not referenced
		//  at all on cancel
		delete &evch;
}


void Editor_Event_Menu::clicked_del_eventchain() {
	eia().egbase().map().mcm().remove
		(m_eventchain_list->get_selected()->name());
	m_eventchain_list->remove_selected();
	m_btn_del_eventchain->set_enabled(false);
	m_btn_edit_eventchain->set_enabled(false);
	update();
}


void Editor_Event_Menu::clicked_edit_eventchain() {
	Editor_Event_Menu_Edit_EventChain menu
		(eia(), *m_eventchain_list->get_selected());
	menu.run();
	update();
}

/**
 * Listbox was selected
 */
void Editor_Event_Menu::trigger_list_selected(uint32_t) {
	m_btn_del_trigger ->set_enabled(true);
	m_btn_edit_trigger->set_enabled(true);
}
void Editor_Event_Menu::event_list_selected(uint32_t) {
	m_btn_del_event ->set_enabled(true);
	m_btn_edit_event->set_enabled(true);
}
void Editor_Event_Menu::eventchain_list_selected(uint32_t) {
	m_btn_del_eventchain ->set_enabled(true);
	m_btn_edit_eventchain->set_enabled(true);
}

/**
 * Listbox was double clicked
 */
void Editor_Event_Menu::trigger_double_clicked(uint32_t)
{
	clicked_edit_trigger();
}
void Editor_Event_Menu::event_double_clicked(uint32_t)
{
	clicked_edit_event  ();
}
void Editor_Event_Menu::eventchain_double_clicked(uint32_t)
{
	clicked_edit_eventchain();
}
