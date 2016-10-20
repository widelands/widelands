/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "ui_fsmenu/main_menu.h"

FullscreenMenuMainMenu::FullscreenMenuMainMenu()
   : FullscreenMenuBase(),

     // Values for alignment and size
     box_x_(get_w() * 13 / 40),
     box_y_(get_h() * 6 / 25),
     butw_(get_w() * 7 / 20),
     buth_(get_h() * 9 / 200),
     title_y_(get_h() * 3 / 40),
     padding_(buth_ / 3),
     button_background_("images/ui_basic//but3.png") {
}
