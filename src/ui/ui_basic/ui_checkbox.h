/*
 * Copyright (C) 2004, 2006, 2008 by the Widelands Development Team
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

#ifndef UI_CHECKBOX_H
#define UI_CHECKBOX_H

#include "ui_panel.h"
#include "ui_signal.h"

#include "rgbcolor.h"

#define STATEBOX_WIDTH 20
#define STATEBOX_HEIGHT 20

namespace UI {
/**
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
struct Statebox : public Panel {
	Statebox(Panel *parent, int32_t x, int32_t y, uint32_t picid = 0);
	~Statebox();

	Signal changed;
	Signal1<bool> changedto;
	Signal2<int32_t, bool> changedtoid;

	void set_enabled(bool enabled);

	bool get_state() const throw () {return m_state;}
	void set_state(bool on);

	void set_id(int32_t n) {m_id = n;}

	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	virtual void clicked() = 0;

	int32_t  m_id;

	bool     m_custom_picture; ///< the statebox displays a custom picture
	uint32_t     m_pic_graphics;

	bool     m_highlighted;
	bool     m_enabled;        ///< true if the checkbox can be clicked
	bool     m_state;          ///< true if the box is checked

	RGBColor m_clr_state; ///< color of border when checked (custom picture only)
	RGBColor m_clr_highlight;  ///< color of border when highlighted
};


/**
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 * A checkbox only differs from a Statebox in that clicking on it toggles the
 * state
*/
class Checkbox : public Statebox {
public:
	Checkbox(Panel *parent, int32_t x, int32_t y, int32_t picid=0) : Statebox(parent, x, y, picid) {}

private:
	void clicked();
};
};

#endif
