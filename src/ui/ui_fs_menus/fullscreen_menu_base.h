/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_BASE_H
#define FULLSCREEN_MENU_BASE_H

#include "ui_basic/panel.h"
#include <string>

/**
 * This class is the base class for a fullscreen menu.
 * A fullscreen menu is a menu which takes the full screen; it has the size
 * MENU_XRES and MENU_YRES and is a modal UI Element
 */
struct Fullscreen_Menu_Base : public UI::Panel {
	Fullscreen_Menu_Base(const char *bgpic);
	~Fullscreen_Menu_Base();

	virtual void draw(RenderTarget &);

public:
///\return the x/y value of current set resolution
uint32_t gr_x();
uint32_t gr_y();

///\return the size for texts fitting to current resolution
uint32_t fs_small();
uint32_t fs_big();

///\return the current set UI font
std::string ui_fn();

protected:
	uint32_t m_xres;
	uint32_t m_yres;
private:
	uint32_t m_pic_background;
	uint32_t m_res_background;
};


#endif
