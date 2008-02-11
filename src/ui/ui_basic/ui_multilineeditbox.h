/*
 * Copyright (C) 2002, 2006, 2008 by Widelands Development Team
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

#ifndef UI_MULTILINEEDITBOX_H
#define UI_MULTILINEEDITBOX_H

#include "ui_multilinetextarea.h"
#include "ui_signal.h"

namespace UI {
struct Scrollbar;

/**
 * This behaves like a editbox, but looks like
 * a Multiline Textarea
 *
 * Shift + del or Shift + backspace deletes all text
 */
struct Multiline_Editbox : public Multiline_Textarea {
	Multiline_Editbox
		(Panel *, int32_t x, int32_t y, uint32_t w, uint32_t h, char const *);
	~Multiline_Editbox();

	Signal changed;

	void draw(RenderTarget *);
	void set_maximum_chars(int32_t n) {m_maxchars=n;}
	int32_t get_maximum_chars() {return m_maxchars;}

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	bool handle_key(bool down, SDL_keysym);
	void set_text(char const *);

private:
	static const int32_t ms_darken_value = -20;
	uint32_t m_cur_pos;
	uint32_t m_char_pos;
	uint32_t m_line_pos;
	uint32_t m_maxchars;
	bool     m_needs_update;
};
};

#endif
