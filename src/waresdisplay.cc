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

#include "editor_game_base.h"
#include "font_handler.h"
#include "player.h"
#include "rendertarget.h"
#include "tribe.h"
#include "ui_textarea.h"
#include "waresdisplay.h"
#include "worker.h"

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
WaresDisplay::WaresDisplay(UIPanel* parent, int x, int y, Editor_Game_Base* game, Player* player)
	: UIPanel(parent, x, y, Width, 0)
{
	int rows, height;

	m_game = game;
	m_player = player;

	rows = (player->get_tribe()->get_nrwares() + player->get_tribe()->get_nrworkers() + WaresPerRow - 1) / WaresPerRow;
	height = rows * (WARE_MENU_PIC_H + 8 + 3) + 1;

	set_size(Width, height+30);

   m_curware = new UITextarea(this, 0, get_inner_h()-25, get_inner_w(), 20, "Testtext", Align_Center);
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
	m_workers.clear(); // okay with accounting + avoid error messages
}

/*
 * handles mouse move
 */
void WaresDisplay::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns) {
   int row= y / (WARE_MENU_PIC_H + 8 + 3);
   int index=row*WaresPerRow;
   index += x / (WARE_MENU_PIC_W +4)+1;
   std::string str;

   if(index > (m_wares.get_nrwareids()+m_workers.get_nrwareids())) {
      m_curware->set_text("");
   } else if(index>m_wares.get_nrwareids()) {
      str=m_player->get_tribe()->get_worker_descr(index-(m_wares.get_nrwareids()+1))->get_descname();
      str+=" (worker)";
      m_curware->set_text(str.c_str());
   } else {
		index--;
      str=m_player->get_tribe()->get_ware_descr(index)->get_descname();
      str+=" (ware)";
      m_curware->set_text(str.c_str());
   }
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
void WaresDisplay::set_workers(const WareList& workers) {
	if (m_workers == workers)
		return;

	m_workers = workers;

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


   int totid=0;
	for(int id = 0; id < m_player->get_tribe()->get_nrwares(); id++, totid++)	{
		draw_ware(dst, x, y, id, m_wares.stock(id), false);

		if (((totid+1) % WaresPerRow) != 0)
		{
			x += WARE_MENU_PIC_W + 3;
		}
		else
		{
			x = 2;
			y += WARE_MENU_PIC_H+8 + 3;
		}
	}
	for(int id = 0; id < m_player->get_tribe()->get_nrworkers(); id++, totid++)	{
		draw_ware(dst, x, y, id, m_workers.stock(id), true);

		if (((totid+1) % WaresPerRow) != 0)
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
void WaresDisplay::draw_ware(RenderTarget* dst, int x, int y, uint id, uint stock, bool worker)
{
	uint pic;

	// Get the picture
	if (worker)
	{
		Worker_Descr* worker = m_player->get_tribe()->get_worker_descr(id);
		pic = worker->get_menu_pic();
	}
	else
	{
      Item_Ware_Descr* wd = m_player->get_tribe()->get_ware_descr(id);
		pic = wd->get_menu_pic();
	}

	// Draw it
	dst->blit(x, y, pic);
	dst->fill_rect(x, y+WARE_MENU_PIC_H, WARE_MENU_PIC_W, 8, RGBColor(0, 0, 0));

	char buf[32];
	snprintf(buf, sizeof(buf), "%i", stock);

	g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR,  x+WARE_MENU_PIC_W, y+WARE_MENU_PIC_H, buf, Align_Right);
}

