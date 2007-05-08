/*
 * Copyright (C) 2003, 2006-2007 by the Widelands Development Team
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

#include "waresdisplay.h"

#include "editor_game_base.h"
#include "font_handler.h"
#include "graphic.h"
#include "i18n.h"
#include "player.h"
#include "rendertarget.h"
#include "tribe.h"
#include "ui_textarea.h"
#include "worker.h"

#include <stdio.h>


WaresDisplay::WaresDisplay(UI::Panel* parent, int x, int y, Editor_Game_Base* game, Player* player)
	: UI::Panel(parent, x, y, Width, 0)
{

	m_game = game;
	m_player = player;

   set_size(Width, 100);
   m_curware = new UI::Textarea(this, 0, get_inner_h()-25, get_inner_w(), 20, _("Stock"), Align_Center);
}


/*
===============
WaresDisplay::~WaresDisplay

Cleanup
===============
*/
WaresDisplay::~WaresDisplay()
{
   remove_all_warelists();
}

/*
 * handles mouse move
 */
bool WaresDisplay::handle_mousemove(const Uint8, int x, int y, int, int) {
   int row = y / (WARE_MENU_PIC_HEIGHT + 8 + 3);
   int index=row*WaresPerRow;
   index += x / (WARE_MENU_PIC_WIDTH +4) + 1;
   std::string str;

   assert( m_warelists.size() );

   if(index > (m_warelists[0]->get_nrwareids())) {
      m_curware->set_text("");
   }
   else {
      if(m_type == WORKER) {
         index--;
         str=m_player->tribe().get_worker_descr(index)->descname();
         m_curware->set_text(str.c_str());
      } else {
         index--;
         str=m_player->tribe().get_ware_descr(index)->descname();
         m_curware->set_text(str.c_str());
      }
   }

   return true;
}

/*
===============
WaresDisplay::set_wares

add a ware list to be displayed in this WaresDisplay
===============
*/
void WaresDisplay::add_warelist(const WareList* wares, wdType type)
{
   // If you register something twice, it is counted twice. Not my problem
	m_warelists.push_back(wares);

   int rows, height;

   rows = (wares->get_nrwareids() + WaresPerRow - 1) / WaresPerRow;
	height = rows * (WARE_MENU_PIC_HEIGHT + 8 + 3) + 1;

	set_size(get_inner_w(), height+30);
	m_curware->set_pos(Point(0, get_inner_h() - 25));
   m_curware->set_size(get_inner_w(), 20);

   m_type = type;


	update(0, 0, get_w(), get_h());
}

/*
 * Delete all ware lists
 */
void WaresDisplay::remove_all_warelists( void ) {
   m_warelists.clear();
}

/*
===============
WaresDisplay::draw

Draw the wares.
===============
*/
void WaresDisplay::draw(RenderTarget* dst)
{
	Point p(2, 2);

   int number = m_player->tribe().get_nrwares();
   bool is_worker = false;

   if( m_type == WORKER ) {
      number = m_player->tribe().get_nrworkers();
      is_worker = true;
   }
   int totid=0;
	for (int id = 0; id < number; ++id, ++totid) {
      uint totalstock = 0;
      for( uint i = 0; i < m_warelists.size(); i++)
         totalstock += m_warelists[i]->stock(id);

		draw_ware(*dst, p, id, totalstock, is_worker);

		if ((totid + 1) % WaresPerRow) {p.x += WARE_MENU_PIC_WIDTH + 3;}
		else {p.x = 2; p.y += WARE_MENU_PIC_HEIGHT + 8 + 3;}
   }
}


/*
===============
WaresDisplay::draw_ware [virtual]

Draw one ware icon + additional information.
===============
*/
void WaresDisplay::draw_ware
(RenderTarget & dst, const Point p, const uint id, const uint stock, const bool worker)
{
	uint pic;

	// Get the picture
	if (worker)
	{
		Worker_Descr* w = m_player->tribe().get_worker_descr(id);
		pic = w->get_menu_pic();
	}
	else
		pic = m_player->tribe().get_ware_descr(id)->get_icon();

   // Draw a background
	const uint picid = g_gr->get_picture(PicMod_Game, "pics/ware_list_bg.png");
	uint w, h;
	g_gr->get_picture_size(picid, w, h);

	dst.blit(p, picid);

	const Point pos = p + Point((w - WARE_MENU_PIC_WIDTH) / 2, 1);
	// Draw it
	dst.blit(pos, pic);
	dst.fill_rect
		(Rect(pos + Point(0, WARE_MENU_PIC_HEIGHT), WARE_MENU_PIC_WIDTH, 8),
		 RGBColor(0, 0, 0));

	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%i", stock);

	g_fh->draw_string
		(dst,
		 UI_FONT_ULTRASMALL,
		 UI_FONT_SMALL_CLR,
		 p + Point(WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT - 4),
		 buffer,
		 Align_Right);
}
