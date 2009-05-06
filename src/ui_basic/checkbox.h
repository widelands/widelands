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

#include "panel.h"
#include "m_signal.h"

#include "rgbcolor.h"

#define STATEBOX_WIDTH 20
#define STATEBOX_HEIGHT 20

namespace UI {
/**
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
struct Statebox : public Panel {
	Statebox
		(Panel * parent,
		 Point,
		 uint32_t picid                   = 0,
		 std::string const & tooltip_text = std::string());
	~Statebox();

	Signal changed;
	Signal1<bool> changedto;
	Signal2<int32_t, bool> changedtoid;

	void set_enabled(bool enabled);

	bool get_state() const throw () {return m_flags & Is_Checked;}
	void set_state(bool on);

	void set_id(int32_t n) {m_id = n;}
	void set_owns_custom_picture() throw () {
		assert(m_flags & Has_Custom_Picture);
		set_flags(Owns_Custom_Picture, true);
	}

	// Drawing and event handlers
	void draw(RenderTarget &);

	void handle_mousein(bool inside);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	virtual void clicked() = 0;

	int32_t  m_id;
	enum Flags {
		Is_Highlighted      = 0x01,
		Is_Enabled          = 0x02,
		Is_Checked          = 0x04,
		Has_Custom_Picture  = 0x08,
		Owns_Custom_Picture = 0x10
	};
	uint8_t m_flags;
	void set_flags(uint8_t const flags, bool const enable) throw () {
		m_flags &= ~flags; if (enable) m_flags |= flags;
	}
	uint32_t     m_pic_graphics;
};


/**
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 * A checkbox only differs from a Statebox in that clicking on it toggles the
 * state
*/
struct Checkbox : public Statebox {
	Checkbox
		(Panel             * const parent,
		 Point               const p,
		 uint32_t            const picid        = 0,
		 std::string const &       tooltip_text = std::string())
		: Statebox(parent, p, picid, tooltip_text)
	{}

private:
	void clicked();
};
};

#endif
