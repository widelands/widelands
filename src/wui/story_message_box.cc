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


#include "constants.h"
#include "graphic/graphic.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"

#include "story_message_box.h"

/**
 * The message box itself
 */
Story_Message_Box::Story_Message_Box
	(UI::Panel * const parent,
	 std::string title, std::string body, std::string button_text,
	 int32_t gposx, int32_t gposy, int32_t w, int32_t h)
	: UI::Window(parent, 0, 0, 600, 400, title.c_str())
{
	UI::Multiline_Textarea * m_text = 0;
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

	int32_t const but_width = 80;
	int32_t space = get_inner_w() - 2 * spacing;
	space -= but_width;
	space /= 2; // center button
	posx = spacing;
	posy = get_inner_h() - 30;
	posx += space;
	new UI::Callback_IDButton<Story_Message_Box, int32_t>
		(this,
		 posx, posy, but_width, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Story_Message_Box::clicked_ok, *this, 0, button_text);
		posx += but_width;

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
void Story_Message_Box::clicked_ok(int32_t const i) {
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

