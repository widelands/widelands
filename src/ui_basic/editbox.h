/*
 * Copyright (C) 2003, 2006-2008 by the Widelands Development Team
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

#ifndef UI_EDITBOX_H
#define UI_EDITBOX_H

#include "font_handler.h"
#include "button.h"

#include <boost/scoped_ptr.hpp>
#include <SDL_keyboard.h>

namespace UI {

struct EditBoxImpl;

/// An editbox can be clicked, then the user can change its text (title). When
/// return is pressed, the editbox is unfocused, the keyboard released and a
/// callback function is called
struct EditBox : public Panel {
	EditBox
		(Panel *,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 uint32_t background = 2,
		 int32_t id = 0,
		 Align align = Align_Center);
	virtual ~EditBox();

	Signal changed;
	Signal1<int32_t> changedid;
	Signal ok;
	Signal1<int32_t> okid;
	Signal cancel;
	Signal1<int32_t> cancelid;

	std::string const & text() const;
	void setText(std::string const &);
	uint32_t maxLength() const;
	void setMaxLength(uint32_t);
	Align align() const;
	void setAlign(Align);
	void set_font(std::string const & name, int32_t size, RGBColor color) {
		m_fontname = name;
		m_fontsize = size;
		m_fontcolor = color;
	}

	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	bool handle_key(bool down, SDL_keysym);

	void draw(RenderTarget &);

private:
	std::string                    m_fontname;
	uint32_t                       m_fontsize;
	RGBColor                       m_fontcolor;

	boost::scoped_ptr<EditBoxImpl> m;
};

}

#endif
