/*
 * Copyright (C) 2003 by Widelands Development Team
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
#include "editor_game_base.h"
#include "player.h"
#include "ware.h"

#include "ui.h"
#include "waresdisplay.h"



/*
==============================================================================

WaresDisplay IMPLEMENTATION

==============================================================================
*/


/*
===============
WaresDisplay::WaresDisplay

Initialize the wares display
===============
*/
WaresDisplay::WaresDisplay(Panel* parent, int x, int y, Editor_Game_Base* game, Player* player)
	: Panel(parent, x, y, Width, 0)
{
	int rows, height;

	m_game = game;
	m_player = player;

	rows = (game->get_nrwares() + WaresPerRow - 1) / WaresPerRow;
	height = rows * (WARE_MENU_PIC_H + 8 + 3) + 1;

	set_size(Width, height);
}


/*
===============
WaresDisplay::~WaresDisplay

Cleanup
===============
*/
WaresDisplay::~WaresDisplay()
{
	m_wares.clear(); // okay with accounting + avoid error messages
}


/*
===============
WaresDisplay::set_wares

Set the new wares state. Update the window if anything has changed.
===============
*/
void WaresDisplay::set_wares(const WareList& wares)
{
	if (m_wares == wares)
		return;

	m_wares = wares;

	update(0, 0, get_w(), get_h());
}


/*
===============
WaresDisplay::draw

Draw the wares.
===============
*/
void WaresDisplay::draw(RenderTarget* dst)
{
	int x, y;

	x = 2;
	y = 2;

	for(int id = 0; id < m_game->get_nrwares(); id++)	{
		draw_ware(dst, x, y, id, m_wares.stock(id));

		if (((id+1) % WaresPerRow) != 0)
		{
			x += WARE_MENU_PIC_W + 3;
		}
		else
		{
			x = 2;
			y += WARE_MENU_PIC_H+8 + 3;
		}
	}
}


/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void WaresDisplay::draw_ware(RenderTarget* dst, int x, int y, uint id, uint stock)
{
	Ware_Descr* wd = m_game->get_ware_description(id);
	uint pic;

	// Get the picture
	if (wd->is_worker())
	{
		Tribe_Descr* tribe = m_player->get_tribe();
		Worker_Descr* worker = ((Worker_Ware_Descr*)wd)->get_worker(tribe);
		pic = worker->get_menu_pic();
	}
	else
	{
		pic = ((Item_Ware_Descr*)wd)->get_menu_pic();
	}

	// Draw it
	dst->blit(x, y, pic);
	dst->fill_rect(x, y+WARE_MENU_PIC_H, WARE_MENU_PIC_W, 8, RGBColor(0, 0, 0));

	char buf[32];
	snprintf(buf, sizeof(buf), "%i", stock);

	g_font->draw_string(dst, x+WARE_MENU_PIC_W, y+WARE_MENU_PIC_H, buf, Align_Right);
}

