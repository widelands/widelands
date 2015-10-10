/*
 * Copyright (C) 2004, 2006, 2008-2011 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_CHECKBOX_H
#define WL_UI_BASIC_CHECKBOX_H

#include <boost/signals2.hpp>

#include "graphic/color.h"
#include "ui_basic/panel.h"

constexpr int kStateboxSize = 20;

namespace UI {

/**
 * Virtual base class providing a box that can be checked or unchecked.
 * Serves as base for Checkbox and Radiobutton.
 */
struct Statebox : public Panel {
	Statebox
		(Panel * parent,
		 Point,
		 const Image* pic                  = nullptr,
		 const std::string& label_text = std::string(),
		 const std::string & tooltip_text = std::string(),
		 uint32_t width = 0);
	~Statebox();

	boost::signals2::signal<void ()> changed;
	boost::signals2::signal<void (bool)> changedto;
	boost::signals2::signal<void (bool)> clickedto; // same as changedto but only called when clicked

	void set_enabled(bool enabled);

	bool get_state() const {return m_flags & Is_Checked;}
	void set_state(bool on);

	void set_owns_custom_picture() {
		assert(m_flags & Has_Custom_Picture);
		set_flags(Owns_Custom_Picture, true);
	}

	// Drawing and event handlers
	void draw(RenderTarget &) override;

	void handle_mousein(bool inside) override;
	bool handle_mousepress  (uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mouserelease(uint8_t btn, int32_t x, int32_t y) override;
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;

private:
	virtual void clicked() = 0;

	enum Flags {
		Is_Highlighted      = 0x01,
		Is_Enabled          = 0x02,
		Is_Checked          = 0x04,
		Has_Custom_Picture  = 0x08,
		Owns_Custom_Picture = 0x10
	};
	uint8_t m_flags;
	void set_flags(uint8_t const flags, bool const enable) {
		m_flags &= ~flags;
		if (enable)
			m_flags |= flags;
	}
	const Image* m_pic_graphics;
	const Image* rendered_text_;
};


/**
 * A checkbox is a simplistic panel which consists of just a small box which
 * can be either checked (on) or unchecked (off)
 * A checkbox only differs from a Statebox in that clicking on it toggles the
 * state
 * Width is used only when we have a label_text. If it is set to 0, the checkbox will set its width
 * automatically. Otherwise, it will take up multiple lines if necessary (automatic height).
*/
struct Checkbox : public Statebox {
	Checkbox
		(Panel             * const parent,
		 Point               const p,
		 const Image* pic        = nullptr,
		 const std::string&       label_text = std::string(),
		 const std::string &      tooltip_text = std::string(),
		 uint32_t width = 0)
		: Statebox(parent, p, pic, label_text, tooltip_text, width)
	{}

private:
	void clicked() override;
};

}

#endif  // end of include guard: WL_UI_BASIC_CHECKBOX_H
