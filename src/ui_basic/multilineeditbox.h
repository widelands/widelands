/*
 * Copyright (C) 2002-2016 by Widelands Development Team
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

#ifndef WL_UI_BASIC_MULTILINEEDITBOX_H
#define WL_UI_BASIC_MULTILINEEDITBOX_H

#include <memory>

#include <boost/signals2.hpp>

#include "graphic/graphic.h"
#include "ui_basic/panel.h"

namespace UI {

struct TextStyle;

/**
 * A panel that allows entering multi-line string, i.e. like a hybrid between
 * @ref Editbox and @ref MultilineTextarea
 */
struct MultilineEditbox : public Panel {
	MultilineEditbox
		(Panel *, int32_t x, int32_t y, uint32_t w, uint32_t h,
		 const std::string & text, const Image* background = g_gr->images().get("images/ui_basic/but2.png"));

	boost::signals2::signal<void ()> changed;

	const std::string & get_text() const;
	void set_text(const std::string &);

	void set_maximum_bytes(uint32_t n);
	uint32_t get_maximum_bytes() const;

	void focus(bool topcaller = true) override;

protected:
	void draw(RenderTarget &) override;

	bool handle_key(bool down, SDL_Keysym) override;
	bool handle_textinput(const std::string& text) override;

private:
	void scrollpos_changed(int32_t);

	struct Data;
	std::unique_ptr<Data> d_;
};

}

#endif  // end of include guard: WL_UI_BASIC_MULTILINEEDITBOX_H
