/*
 * Copyright (C) 2002 by Holger Rapp,
 *                       Nicolai Haehnle
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
#include "ui.h"
#include "input.h"
#include "graphic.h"
#include "cursor.h"
#include "fileloc.h"
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
	const char *name = g_fileloc.locate_file(bgpic, TYPE_PIC);
	if (!name)
		critical_error("%s: File not found. Check your installation.", bgpic);
	bg.load(name);
}

/** BaseMenu::start()
 *
 * Change the resolution to menu resolution before the event loop starts
 */
void BaseMenu::start()
{
	g_gr.set_mode(MENU_XRES, MENU_YRES, g_gr.get_mode());
	g_ip.set_max_cords(MENU_XRES-g_cur.get_w(), MENU_YRES-g_cur.get_h());
}

/** BaseMenu::draw(Bitmap *dst, int ofsx, int ofsy)
 *
 * Draw the splash screen
 */
void BaseMenu::draw(Bitmap *dst, int ofsx, int ofsy)
{
	copy_pic(dst, &bg, 0, 0, 0, 0, bg.get_w(), bg.get_h());
}

