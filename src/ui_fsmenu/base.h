/*
 * Copyright (C) 2002, 2006, 2008-2009 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_BASE_H
#define FULLSCREEN_MENU_BASE_H

#include <string>

#include "ui_basic/panel.h"

namespace UI {
struct Font;
struct TextStyle;
}

/**
 * This class is the base class for a fullscreen menu.
 * A fullscreen menu is a menu which takes the full screen; it has the size
 * MENU_XRES and MENU_YRES and is a modal UI Element
 */
struct Fullscreen_Menu_Base : public UI::Panel {
	Fullscreen_Menu_Base(char const * bgpic);
	~Fullscreen_Menu_Base();

	virtual void draw(RenderTarget &) override;

public:
	///\return the size for texts fitting to current resolution
	uint32_t fs_small();
	uint32_t fs_big();

	UI::TextStyle & ts_small();
	UI::TextStyle & ts_big();

	UI::Font * font_small();
	UI::Font * font_big();

private:
	/**
	 * Query the configured screen resolution.
	 */
	uint32_t gr_x();
	uint32_t gr_y();

	struct Data;
	std::unique_ptr<Data> d;
};


#endif
