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
#include "ui.h"


/*
==============================================================================

Scrollbar

==============================================================================
*/


/** Scrollbar::Scrollbar(Panel *parent, int x, int y, uint w, uint h, bool horiz)
 *
 * Build a scrollbar. TODO, this is a lame hack really.
 */
Scrollbar::Scrollbar(Panel *parent, int x, int y, uint w, uint h, bool horiz)
	: Panel(parent, x, y, w, h)
{
	set_think(false);

	Button *b;

	if (horiz)
	{
		uint btnw = h;

		if (btnw*2 > w)
			btnw = w / 2;

		// left button
		b = new Button(this, 0, 0, btnw, h, 1);
		b->clicked.set(this, &Scrollbar::btn_up);
		b->set_title("L");

		// right button
		b = new Button(this, w-btnw, 0, btnw, h, 1);
		b->clicked.set(this, &Scrollbar::btn_down);
		b->set_title("R");
	}
	else
	{
		uint btnh = w;

		if (btnh*2 > h)
			btnh = h / 2;

		// up button
		b = new Button(this, 0, 0, w, btnh, 1);
		b->clicked.set(this, &Scrollbar::btn_up);
		b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_up.png", RGBColor(0,0,255)));

		// down button
		b = new Button(this, 0, h-btnh, w, btnh, 1);
		b->clicked.set(this, &Scrollbar::btn_down);
		b->set_pic(g_gr->get_picture(PicMod_UI, "pics/scrollbar_down.png", RGBColor(0,0,255)));
	}
}

