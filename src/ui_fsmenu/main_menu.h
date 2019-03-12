/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "ui_basic/box.h"
#include "ui_fsmenu/base.h"

/**
 * This sets the values for alignment and size and other common properties
 * for main menus that contain only buttons.
 */
class FullscreenMenuMainMenu : public FullscreenMenuBase {
public:
	/// Assigns values for alignment and size, depending on screen size
	FullscreenMenuMainMenu();

protected:
	void layout() override;

	uint32_t box_x_, box_y_;
	uint32_t butw_, buth_;
	uint32_t title_y_;
	uint32_t padding_;

	UI::Box vbox_;
};

#endif  // end of include guard: WL_UI_FSMENU_MAIN_MENU_H
