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

#include "editor_event_menu_new_event.h"

#include "constants.h"
#include "editor/editorinteractive.h"
#include "events/event.h"
#include "events/event_factory.h"
#include "i18n.h"
#include "logic/map.h"

#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

using namespace Widelands::Event_Factory;

inline Editor_Interactive & Editor_Event_Menu_New_Event::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Editor_Event_Menu_New_Event::Editor_Event_Menu_New_Event
	(Editor_Interactive & parent)
: UI::Window(&parent, 0, 0, 400, 240, _("New Event"))
{
	int32_t const offsx   =  5;
	int32_t const offsy   =  5;
	int32_t const spacing =  5;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	//  event List
	new UI::Textarea(this, spacing, offsy, _("Available Event types: "));
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
		(this, (get_inner_w() / 2) + spacing, offsy, _("Description: "));
	m_description =
		new UI::Multiline_Textarea
			(this,
			 get_inner_w() / 2 + spacing, offsy + 20,
			 get_inner_w() / 2 - 2 * spacing, get_inner_h() - offsy - 55);

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
	Type_Descr const & descr = type_descr(m_event_type_list->get_selected());
	m_description->set_font
		(UI_FONT_SMALL,
		 descr.has_options_window ? UI_FONT_CLR_FG : UI_FONT_CLR_DISABLED);
	{
		std::string helptext = _(descr.helptext);
		if (not descr.has_options_window)
			helptext +=
				_
					(" NOTE: There is no options window for this event type yet. "
					 "It has to be created manually by editing the text files in "
					 "the map.");
		m_description->set_text(helptext);
	}
	m_ok_button->set_enabled(descr.has_options_window);
}

/**
 * Listbox got double clicked
 */
void Editor_Event_Menu_New_Event::double_clicked(uint32_t) {
	if (type_descr(m_event_type_list->get_selected()).has_options_window)
		clicked_ok();
}
