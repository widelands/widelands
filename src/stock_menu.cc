/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "stock_menu.h"

#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "player.h"
#include "transport.h"


Stock_Menu::Stock_Menu
(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&plr, &registry, 640, 480, _("Stock")),
m_player(plr),
waresdisplay(this, 0, 0, plr.player().tribe()),

help
(this,
 0, 0, buttonw(3),                  30,
 4,
 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
 &Stock_Menu::clicked_help, this),

switchpage
(this,
 0, 0, buttonw(3) * 2 + hspacing(), 30,
 4,
 g_gr->get_picture(PicMod_Game, "pics/warehousewindow_switchpage.png"),
 &Stock_Menu::clicked_switch_page, this),
current_page(Wares)

{
	fill_waredisplay_with_wares();
	help      .set_pos(Point(posx(0, 3), waresdisplay.get_h()));
	switchpage.set_pos(Point(posx(1, 3), waresdisplay.get_h()));
	set_inner_size
		(waresdisplay.get_w(), waresdisplay.get_h() + 30 + vmargin());
}


void Stock_Menu::clicked_help() {}


/*
 * Switch to the next page, that is, show
 * wares -> workers -> soldier
 */
void Stock_Menu::clicked_switch_page() {
	switch (current_page) {
	case Wares:
		current_page = Workers;
      fill_waredisplay_with_workers();
		switchpage.set_tooltip(_("Show wares").c_str());
		break;
	case Workers:
		current_page = Wares;
      fill_waredisplay_with_wares();
		switchpage.set_tooltip(_("Show workers").c_str());
		break;
	default: assert(false);
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
	switch (current_page) {
	case Wares:   fill_waredisplay_with_wares  (); break;
	case Workers: fill_waredisplay_with_workers(); break;
	default: assert(false);
	}
}

/*
 * Fills the waresdisplay with wares
 */
void Stock_Menu::fill_waredisplay_with_wares() {
	waresdisplay.remove_all_warelists();
	const Player & player = *m_player.get_player();
	const uint nrecos = player.get_nr_economies();
	for (uint i = 0; i < nrecos; ++i)
		waresdisplay.add_warelist
			(&player.get_economy_by_number(i)->get_wares(), WaresDisplay::WARE);
}

/*
 * Fill it with workers
 */
void Stock_Menu::fill_waredisplay_with_workers() {
	waresdisplay.remove_all_warelists();
	const Player & player = *m_player.get_player();
	const uint nrecos = player.get_nr_economies();
	for (uint i = 0; i < nrecos; ++i)
		waresdisplay.add_warelist
			(&player.get_economy_by_number(i)->get_workers(),
			 WaresDisplay::WORKER);
}
