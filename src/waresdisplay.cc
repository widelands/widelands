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

#include <stdio.h>
#include "editor_game_base.h"
#include "font_handler.h"
#include "graphic.h"
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

	m_game = game;
	m_player = player;

   set_size(Width, 100);
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
   remove_all_warelists();
}

/*
 * handles mouse move
 */
void WaresDisplay::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns) {
   int row= y / (WARE_MENU_PIC_H + 8 + 3);
   int index=row*WaresPerRow;
   index += x / (WARE_MENU_PIC_W +4)+1;
   std::string str;

   assert( m_warelists.size() );

   if(index > (m_warelists[0]->get_nrwareids())) {
      m_curware->set_text("");
   } 
   else {
      if(m_type == WORKER) {
         index--;
         str=m_player->get_tribe()->get_worker_descr(index)->get_descname();
         m_curware->set_text(str.c_str());
      } else {
         index--;
         str=m_player->get_tribe()->get_ware_descr(index)->get_descname();
         m_curware->set_text(str.c_str());
      }
   }
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
	height = rows * (WARE_MENU_PIC_H + 8 + 3) + 1;

	set_size(get_inner_w(), height+30);
   m_curware->set_pos(0, get_inner_h()-25);
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
   int x, y;

   x = 2;
   y = 2;

   int number = m_player->get_tribe()->get_nrwares();
   bool is_worker = false;

   if( m_type == WORKER ) { 
      number = m_player->get_tribe()->get_nrworkers();
      is_worker = true;
   }
   int totid=0;
   for(int id = 0; id < number; id++, totid++)	{
      uint totalstock = 0;
      for( uint i = 0; i < m_warelists.size(); i++)
         totalstock += m_warelists[i]->stock(id);

      draw_ware(dst, x, y, id, totalstock, is_worker);

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

   // Draw a background
   int picid = g_gr->get_picture(PicMod_Game, "pics/ware_list_bg.png", false);
   int w, h;
   g_gr->get_picture_size(picid, &w, &h);
   
   dst->blit(x, y, picid); 
   
   int posx = x + (w - WARE_MENU_PIC_W)/2;
   int posy = y + 1;
	// Draw it
	dst->blit(posx, posy, pic);
	dst->fill_rect(posx, posy+WARE_MENU_PIC_H, WARE_MENU_PIC_W, 8, RGBColor(0, 0, 0));

	char buf[32];
	snprintf(buf, sizeof(buf), "%i", stock);

	g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR,  x+WARE_MENU_PIC_W, y+WARE_MENU_PIC_H, buf, Align_Right);
}

