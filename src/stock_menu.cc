/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "stock_menu.h"
#include "transport.h"
#include "ui_button.h"
#include "waresdisplay.h"

/*
===============
Stock_Menu::Stock_Menu

Open the window, create the window buttons and add to the registry.
===============
*/
Stock_Menu::Stock_Menu(Interactive_Player *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 640, 480, _("Stock"))
{
   m_parent = parent;

	// Add wares display
   m_waresdisplay = new WaresDisplay(this, 0, 0, parent->get_game(), parent->get_player());
   // Add with wares
   fill_waredisplay_with_wares();

   set_inner_size(m_waresdisplay->get_w(), 0);

   int spacing = 5;
   int nr_buttons = 4; // one more, turn page button is bigger
   int button_w = (get_inner_w() - (nr_buttons+1)*spacing) / nr_buttons;
   int posx = spacing;
   int posy = m_waresdisplay->get_h() + spacing;
   m_curpage = 0;


   UIButton* b = new UIButton(this, posx, posy, button_w, 25, 4, 100);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_help.png" ));
   b->clickedid.set(this, &Stock_Menu::clicked);
   posx += button_w + spacing;
   b = new UIButton(this, posx, posy, button_w*2+spacing, 25, 4, 1);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/warehousewindow_switchpage.png" ));
   b->clickedid.set(this, &Stock_Menu::clicked);
   posx += button_w*2 + 2*spacing;
   posy += 25 + spacing;

   set_inner_size(get_inner_w(), posy);
}


/*
===============
Stock_Menu::~Stock_Menu

Deinitialize, remove from registry
===============
*/
Stock_Menu::~Stock_Menu()
{
}

/*
 * A button has been clicked
 */
void Stock_Menu::clicked( int id ) {
   switch(id) {
      case 100:
      {
         // Help
         log("TODO: Implement help!\n");
         break;
      }

      case 1:
      {
         // Switch page
         switch_page();
      }

   }

}

/*
 * Switch to the next page, that is, show
 * wares -> workers -> soldier
 */
void Stock_Menu::switch_page(void) {
   if(m_curpage == 0) {
      m_curpage = 1;
      fill_waredisplay_with_workers();
   } else if( m_curpage == 1) {
      m_curpage = 0;
      fill_waredisplay_with_wares();
   }
}

/*
===============
Stock_Menu::think

Push the current wares status to the WaresDisplay.
===============
*/
void Stock_Menu::think()
{
      if(m_curpage == 0)
         fill_waredisplay_with_wares();
      else if(m_curpage == 1)
         fill_waredisplay_with_workers();
}

/*
 * Fills the waresdisplay with wares
 */
void Stock_Menu::fill_waredisplay_with_wares( void ) {
   m_waresdisplay->remove_all_warelists();

   int nrecos = m_parent->get_player()->get_nr_economies();
   for( int i = 0; i < nrecos; i++) {
      Economy* eco = m_parent->get_player()->get_economy_by_number(i);
      m_waresdisplay->add_warelist( &eco->get_wares(), WaresDisplay::WARE);
   }
   m_last_nreconomies = nrecos;
}

/*
 * Fill it with workers
 */
void Stock_Menu::fill_waredisplay_with_workers( void ) {
   m_waresdisplay->remove_all_warelists();

   int nrecos = m_parent->get_player()->get_nr_economies();
   for( int i = 0; i < nrecos; i++) {
      Economy* eco = m_parent->get_player()->get_economy_by_number(i);
      m_waresdisplay->add_warelist( &eco->get_workers(), WaresDisplay::WORKER);
   }
   m_last_nreconomies = nrecos;
}
