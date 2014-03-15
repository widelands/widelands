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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/story_message_box.h"

#include "constants.h"
#include "graphic/graphic.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

/**
 * The message box itself
 */
Story_Message_Box::Story_Message_Box
	(UI::Panel * const parent,
	 const std::string & title,
	 const std::string & body,
	 const std::string & button_text,
	 int32_t  const gposx, int32_t  const gposy,
	 uint32_t const w,     uint32_t const h)
	: UI::Window(parent, "story_message_box", 0, 0, 600, 400, title.c_str())
{
	UI::Multiline_Textarea * m_text = nullptr;
	int32_t const spacing = 5;
	int32_t       offsy   = 5;
	int32_t       offsx   = spacing;
	int32_t       posx    = offsx;
	int32_t       posy    = offsy;

	set_inner_size(w, h);
	m_text =
		new UI::Multiline_Textarea
			(this,
			 posx, posy,
			 get_inner_w() - posx -     spacing,
			 get_inner_h() - posy - 2 * spacing - 50);

	if (m_text)
		m_text->set_text(body);

	int32_t const but_width = 120;
	int32_t space = get_inner_w() - 2 * spacing;
	space -= but_width;
	space /= 2; // center button
	posx = spacing;
	posy = get_inner_h() - 30;
	posx += space;
	UI::Button * okbtn = new UI::Button
		(this, "ok",
		 posx, posy, but_width, 20,
		 g_gr->images().get("pics/but0.png"),
		 button_text);
	okbtn->sigclicked.connect(boost::bind(&Story_Message_Box::clicked_ok, boost::ref(*this)));

	center_to_parent();

	if (gposx != -1)
		set_pos(Point(gposx, get_y()));
	if (gposy != -1)
		set_pos(Point(get_x(), gposy));

	move_inside_parent();
}

/**
 * Clicked
 */
void Story_Message_Box::clicked_ok() {
	end_modal(0);
	return;
}

/*
 * Avoid being closed by right click
 */
bool Story_Message_Box::handle_mousepress
	(const Uint8 btn, int32_t mx, int32_t my)
{
	if (btn == SDL_BUTTON_RIGHT)
		return true;

	return UI::Window::handle_mousepress(btn, mx, my);
}

