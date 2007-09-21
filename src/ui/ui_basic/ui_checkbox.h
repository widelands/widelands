/*
 * Copyright (C) 2004, 2006 by the Widelands Development Team
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

#ifndef included_ui_checkbox_h
#define included_ui_checkbox_h

#include "rgbcolor.h"
#include <stdint.h>
#include "ui_panel.h"
#include "ui_signal.h"

#define STATEBOX_WIDTH 20
#define STATEBOX_HEIGHT 20

namespace UI {
/**
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
struct Statebox : public Panel {
	Statebox(Panel *parent, int x, int y, uint32_t picid = 0);
	~Statebox();

	Signal changed;
	Signal1<bool> changedto;
	Signal2<int, bool> changedtoid;

	void set_enabled(bool enabled);

	inline bool get_state() const {return m_state;}
	void set_state(bool on);

   inline void set_id(int n) {m_id=n;}

	// Drawing and event handlers
	void draw(RenderTarget* dst);

	void handle_mousein(bool inside);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
	virtual void clicked() = 0;

   int      m_id;

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
	Checkbox(Panel *parent, int x, int y, int picid=0) : Statebox(parent, x, y, picid) {}

private:
	void clicked();
};
};

#endif // included_ui_checkbox_h
