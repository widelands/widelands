/*
 * Copyright (C) 2002, 2007 by the Widelands Development Team
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

#include "fullscreen_menu_base.h"

#include "graphic.h"
#include "profile.h"
#include "rendertarget.h"
#include "wlapplication.h"

#include <stdio.h>

/*
==============================================================================

Fullscreen_Menu_Base

==============================================================================
*/

/** Fullscreen_Menu_Base::Fullscreen_Menu_Base(const char *bgpic)
 *
 * Initialize a pre-game menu
 *
 * Args: bgpic  name of the background picture
 */
Fullscreen_Menu_Base::Fullscreen_Menu_Base(const char *bgpic)
	: UI::Panel(0, 0, 0, MENU_XRES, MENU_YRES)
{
	// Switch graphics mode if necessary
	Section *s = g_options.pull_section("global");

	WLApplication::get()->init_graphics(MENU_XRES, MENU_YRES, s->get_int("depth", 16), s->get_bool("fullscreen", false));

	// Load background graphics
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "pics/%s", bgpic);
	m_pic_background = g_gr->get_picture(PicMod_Menu, buffer);
}


/**
 * Draw the splash screen
*/
void Fullscreen_Menu_Base::draw(RenderTarget & dst) {
	dst.blit(Point(0, 0), m_pic_background);
}
