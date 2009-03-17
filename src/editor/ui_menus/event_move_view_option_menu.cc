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

#include "event_move_view_option_menu.h"

#include "graphic.h"
#include "events/event_move_view.h"
#include "editorinteractive.h"
#include "i18n.h"
#include "map.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_messagebox.h"
#include "ui_textarea.h"
#include "ui_window.h"

using Widelands::X_Coordinate;
using Widelands::Y_Coordinate;


inline Editor_Interactive & Event_Move_View_Option_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


Event_Move_View_Option_Menu::Event_Move_View_Option_Menu
	(Editor_Interactive & parent, Widelands::Event_Move_View & event)
:
UI::Window(&parent, 0, 0, 180, 200, _("Move View Event Options")),
m_event   (event),
m_location(event.location())
{
	int32_t const offsx   =  5;
	int32_t const offsy   = 25;
	int32_t       spacing =  5;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
	m_name =
		new UI::EditBox
		(this, spacing + 60, posy, get_inner_w() - 2 * spacing - 60, 20, 0, 0);
	m_name->setText(event.name());
	posy += 20 + spacing;

	new UI::Textarea
		(this,
		 spacing, posy, get_inner_w(), 15,
		 _("Current position: "), Align_CenterLeft);
	posy += 20 + spacing;
	//  X

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 3);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 4);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 5);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 6);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 60, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 7);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 60, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 8);

	new UI::Textarea
		(this, spacing + 20, posy + 20, 20, 20, "X: ", Align_CenterLeft);
	m_x_ta =
		new UI::Textarea
		(this, spacing + 40, posy + 20, 20, 20, "X: ", Align_CenterLeft);

	//  Y
	int32_t const oldspacing = spacing;
	spacing = get_inner_w() / 2 + spacing;

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 9);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 10);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 11);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 12);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 13);

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Move_View_Option_Menu::clicked, this, 14);

	new UI::Textarea
		(this, spacing,      posy + 20, 20, 20, "Y: ", Align_CenterLeft);
	m_y_ta =
		new UI::Textarea
		(this, spacing + 20, posy + 20, 20, 20, "Y: ", Align_CenterLeft);
	spacing = oldspacing;

	posx = get_inner_w() / 2 - 60 - spacing;
	posy = get_inner_h()     - 20 - spacing;
	new UI::Button<Event_Move_View_Option_Menu>
		(this,
		 posx, posy, 60, 20,
		 0,
		 &Event_Move_View_Option_Menu::clicked_ok, this,
		 _("Ok"));

	posx = get_inner_w() / 2 + spacing;

	new UI::IDButton<Event_Move_View_Option_Menu, int32_t>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Event_Move_View_Option_Menu::end_modal, this, 0,
		 _("Cancel"));

	center_to_parent();
	update();
}


/**
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Event_Move_View_Option_Menu::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Event_Move_View_Option_Menu::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}


void Event_Move_View_Option_Menu::clicked_ok() {
	std::string const & name = m_name->text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 eia().egbase().map().mem()[name])
			if (registered_event != & m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another event registered with the name \"%s\". "
					 	 "Choose another name."),
					 name.c_str());
				UI::MessageBox mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::MessageBox::OK);
				mb.run();
				return;
			}
		m_event.set_name(name);
	}
	m_event.set_location(m_location);
	end_modal(1);
}


void Event_Move_View_Option_Menu::clicked(int32_t const i) {
	switch (i) {
	case  3: m_location.x +=                                      100;  break;
	case  4: m_location.x -= std::min<X_Coordinate>(m_location.x, 100); break;
	case  5: m_location.x +=                                       10;  break;
	case  6: m_location.x -= std::min<X_Coordinate>(m_location.x,  10); break;
	case  7: m_location.x +=                                        1;  break;
	case  8: m_location.x -= std::min<X_Coordinate>(m_location.x,   1); break;
	case  9: m_location.y +=                                      100;  break;
	case 10: m_location.y -= std::min<Y_Coordinate>(m_location.y, 100); break;
	case 11: m_location.y +=                                       10;  break;
	case 12: m_location.y -= std::min<Y_Coordinate>(m_location.y,  10); break;
	case 13: m_location.y +=                                        1;  break;
	case 14: m_location.y -= std::min<Y_Coordinate>(m_location.y,   1); break;
	default:
		assert(false);
	}
	update();
}

/**
 * Update function: update all UI elements
 */
void Event_Move_View_Option_Menu::update() {
	Widelands::Extent const extent = eia().egbase().map().extent();
	if (extent.w <= static_cast<uint16_t>(m_location.x))
		m_location.x = extent.w - 1;
	if (extent.h <= static_cast<uint16_t>(m_location.y))
		m_location.y = extent.h - 1;

	char buf[6];
	snprintf(buf, sizeof(buf), "%i", m_location.x);
	m_x_ta->set_text(buf);
	snprintf(buf, sizeof(buf), "%i", m_location.y);
	m_y_ta->set_text(buf);
}
