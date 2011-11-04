/*
 * Copyright (C) 2002, 2006, 2008-2011 by Widelands Development Team
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

#include "panel.h"
#include "m_signal.h"

#include <boost/scoped_ptr.hpp>

namespace UI {

struct TextStyle;

/**
 * A panel that allows entering multi-line string, i.e. like a hybrid between
 * @ref Editbox and @ref Multiline_Textarea
 */
struct Multiline_Editbox : public Panel {
	Multiline_Editbox
		(Panel *, int32_t x, int32_t y, uint32_t w, uint32_t h, const std::string & text);

	Signal changed;

	std::string const & get_text() const;
	void set_text(std::string const &);
	void set_textstyle(const TextStyle &);

	void set_maximum_bytes(uint32_t n);
	uint32_t get_maximum_bytes() const;

protected:
	void draw(RenderTarget &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);
	bool handle_key(bool down, SDL_keysym);

private:
	void scrollpos_changed(int32_t);
	void insert(SDL_keysym code);

	struct Data;
	boost::scoped_ptr<Data> d;
};

}

#endif
