/*
 * Copyright (C) 2002-2004 by Widelands Development Team
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

#ifndef OPTIONSMENU_H
#define OPTIONSMENU_H

#define NUM_RESOLUTIONS		3

#include "fullscreen_menu_base.h"

/**
 * Fullscreen Optionsmenu. A modal optionsmenu
 */
class Fullscreen_Menu_Options : public Fullscreen_Menu_Base {
public:
	Fullscreen_Menu_Options(int cur_x, int cur_y, bool fullscreen, bool inputgrab);

	inline bool get_fullscreen() const { return m_fullscreen->get_state(); }
	inline bool get_inputgrab() const { return m_inputgrab->get_state(); }
	inline uint get_xres() const { return resolutions[m_resolution.get_state()].width; }
	inline uint get_yres() const { return resolutions[m_resolution.get_state()].height; }
	inline int get_gfxsys() const { return GFXSYS_SW16 + m_gfxsys.get_state(); }

   enum {
      om_cancel = 0,
      om_ok = 1
   };


private:
	UICheckbox*	m_fullscreen;
	UICheckbox*	m_inputgrab;
	UIRadiogroup	m_resolution;
	UIRadiogroup	m_gfxsys;

	struct res {
		int width;
		int height;
	};
	static res resolutions[NUM_RESOLUTIONS];
};

#endif /* OPTIONSMENU_H */
