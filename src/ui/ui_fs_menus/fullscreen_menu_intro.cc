/*
 * Copyright (C) 2002, 2006-2006 by the Widelands Development Team
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

#include "fullscreen_menu_intro.h"


Fullscreen_Menu_Intro::Fullscreen_Menu_Intro()
	: Fullscreen_Menu_Base("splash.jpg")
{}

bool Fullscreen_Menu_Intro::handle_mousepress(const Uint8, int32_t, int32_t) {
	end_modal(0);

	return true;
}
bool Fullscreen_Menu_Intro::handle_mouserelease(const Uint8, int32_t, int32_t)
{return true;}

bool Fullscreen_Menu_Intro::handle_key(bool down, int32_t code, char) {
	if (down and code == SDLK_ESCAPE) end_modal(0);

	return true;
}
