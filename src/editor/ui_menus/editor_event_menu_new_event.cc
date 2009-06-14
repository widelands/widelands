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

#include "editor_event_menu_new_event.h"

#include "constants.h"
#include "editor/editorinteractive.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "i18n.h"
#include "map.h"

#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

using namespace Widelands::Event_Factory;

inline Editor_Interactive & Editor_Event_Menu_New_Event::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Editor_Event_Menu_New_Event::Editor_Event_Menu_New_Event
	(Editor_Interactive & parent)
: UI::Window(&parent, 0, 0, 400, 240, _("New Event"))
{
	int32_t const offsx   =  5;
	int32_t const offsy   = 25;
	int32_t const spacing =  5;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	//  event List
	new UI::Textarea
		(this, spacing, offsy, _("Available Event types: "), Align_Left);
	m_event_type_list =
		new UI::BaseListselect
			(this, spacing,
			 offsy + 20, get_inner_w() / 2 - 2 * spacing,
			 get_inner_h() - offsy - 55);
	m_event_type_list->selected.set
		(this, &Editor_Event_Menu_New_Event::selected);
	m_event_type_list->double_clicked.set
		(this, &Editor_Event_Menu_New_Event::double_clicked);

	for (uint32_t i = 0; i < nr_event_types(); ++i)
		m_event_type_list->add(_(type_descr(i).name), i);
	m_event_type_list->sort();

	//  descr List
	new UI::Textarea
		(this,
		 (get_inner_w() / 2) + spacing, offsy,
		 _("Description: "), Align_Left);
	m_description =
		new UI::Multiline_Textarea
			(this,
			 get_inner_w() / 2 + spacing, offsy + 20,
			 get_inner_w() / 2 - 2 * spacing, get_inner_h() - offsy - 55,
			 "", Align_Left);

	posy = get_inner_h() - 30;
	posx = get_inner_w() / 2 - 80 - spacing;

	m_ok_button = new UI::Callback_Button<Editor_Event_Menu_New_Event>
		(this,
		 posx, posy, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Editor_Event_Menu_New_Event::clicked_ok, *this,
		 _("Ok"),
		 std::string(),
		 false);

	posx = get_inner_w() / 2 + spacing;

	new UI::Callback_IDButton<Editor_Event_Menu_New_Event, int32_t>
		(this,
		 posx, posy, 80, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Editor_Event_Menu_New_Event::end_modal, *this, 0,
		 _("Cancel"));

	center_to_parent();
}


/**
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Editor_Event_Menu_New_Event::handle_mousepress
	(Uint8 const btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Editor_Event_Menu_New_Event::handle_mouserelease
	(Uint8,           int32_t, int32_t)
{return false;}

/**
 * A button has been clicked
 */
void Editor_Event_Menu_New_Event::clicked_ok() {
	assert(m_event_type_list->has_selection());
	Widelands::Event & event = create(m_event_type_list->get_selected());
	if (event.option_menu(eia())) {
		eia().egbase().map().mem().register_new(event);
		end_modal(1);
	} else
		delete &event;
}

/**
 * The listbox got selected
 */
void Editor_Event_Menu_New_Event::selected(uint32_t) {
	m_description->set_text
		(_(type_descr(m_event_type_list->get_selected()).helptext));
	m_ok_button->set_enabled(true);
}

/**
 * Listbox got double clicked
 */
void Editor_Event_Menu_New_Event::double_clicked(uint32_t) {clicked_ok();}
