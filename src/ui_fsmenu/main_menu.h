/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MAIN_MENU_H
#define WL_UI_FSMENU_MAIN_MENU_H

#include "ui_fsmenu/base.h"

/**
 * This sets the values for alignment and size and other common properties
 * for main menus that contain only buttons.
 */
class FullscreenMenuMainMenu : public FullscreenMenuBase {
public:

	/// Calls FullscreenMenuMainMenu(const std::string& background_image)
	/// with a default background image
	FullscreenMenuMainMenu();

	/// Sets the background image and assigns values
	/// for alignment and size, depending on screen size
	FullscreenMenuMainMenu(const std::string& background_image);

protected:
	const uint32_t m_box_x, m_box_y;
	const uint32_t m_butw, m_buth;
	const uint32_t m_title_y;
	const uint32_t m_padding;

	const std::string m_button_background;
};

#endif  // end of include guard: WL_UI_FSMENU_MAIN_MENU_H
