/*
 * Copyright (C) 2003, 2006-2008, 2011 by the Widelands Development Team
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

#ifndef UI_EDITBOX_H
#define UI_EDITBOX_H

#include <SDL_keyboard.h>
#include <boost/signals2.hpp>

#include "align.h"
#include "ui_basic/button.h"
#include "graphic/graphic.h"

#define CHAT_HISTORY_SIZE 5

namespace UI {

struct EditBoxImpl;

/// An editbox can be clicked, then the user can change its text (title). When
/// return is pressed, the editbox is unfocused, the keyboard released and a
/// callback function is called
struct EditBox : public Panel {
	EditBox
		(Panel *,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 const Image* background = g_gr->images().get("pics/but2.png"), Align align = Align_Center);
	virtual ~EditBox();

	boost::signals2::signal<void ()> changed;
	boost::signals2::signal<void ()> ok;
	boost::signals2::signal<void ()> cancel;

	const std::string & text() const;
	void setText(const std::string &);
	uint32_t maxLength() const;
	void setMaxLength(uint32_t);
	Align align() const;
	void setAlign(Align);
	void set_font(const std::string & name, int32_t size, RGBColor color);

	void activate_history(bool activate) {m_history_active = activate;}

	bool handle_mousepress(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y) override;
	bool handle_key(bool down, SDL_keysym) override;

	void draw(RenderTarget &) override;

private:
	std::unique_ptr<EditBoxImpl> m;

	void check_caret();
	void insert(SDL_keysym code);

	bool        m_history_active;
	int16_t     m_history_position;
	std::string m_history[CHAT_HISTORY_SIZE];
};

}

#endif
