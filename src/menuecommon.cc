/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "options.h"
#include "ui.h"
#include "graphic.h"
#include "menuecommon.h"

/*
==============================================================================

BaseMenu

==============================================================================
*/

/** BaseMenu::BaseMenu(const char *bgpic)
 *
 * Initialize a pre-game menu
 *
 * Args: bgpic	name of the background picture
 */
BaseMenu::BaseMenu(const char *bgpic)
	: Panel(0, 0, 0, MENU_XRES, MENU_YRES)
{
	// Switch graphics mode if necessary
	Section *s = g_options.pull_section("global");

	Sys_InitGraphics(GFXSYS_SW16, MENU_XRES, MENU_YRES, s->get_bool("fullscreen", false));
	
	// Load background graphics
	char buf[256];
	snprintf(buf, sizeof(buf), "pics/%s", bgpic);
	m_pic_background = g_gr->get_picture(PicMod_Menu, buf);
}


/*
===============
BaseMenu::draw

Draw the splash screen
===============
*/
void BaseMenu::draw(RenderTarget* dst)
{
	dst->blit(0, 0, m_pic_background);
}
