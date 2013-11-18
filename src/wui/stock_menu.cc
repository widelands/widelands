/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/stock_menu.h"

#include "economy/economy.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/player.h"
#include "logic/warehouse.h"
#include "ui_basic/tabpanel.h"
#include "wui/interactive_player.h"

static const char pic_tab_wares[] = "pics/menu_tab_wares.png";
static const char pic_tab_workers[] = "pics/menu_tab_workers.png";
static const char pic_tab_wares_warehouse[] =
	 "pics/menu_tab_wares_warehouse.png";
static const char pic_tab_workers_warehouse[] =
	 "pics/menu_tab_workers_warehouse.png";

Stock_Menu::Stock_Menu
	(Interactive_Player & plr, UI::UniqueWindow::Registry & registry)
:
UI::UniqueWindow(&plr, "stock_menu", &registry, 480, 640, _("Stock")),
m_player(plr)
{
	UI::Tab_Panel * tabs =
		 new UI::Tab_Panel
			 (this, 0, 0, g_gr->images().get("pics/but1.png"));
	set_center_panel(tabs);

	m_all_wares = new WaresDisplay(tabs, 0, 0, plr.player().tribe(), Widelands::wwWARE, false);
	tabs->add
		("total_wares", g_gr->images().get(pic_tab_wares),
		 m_all_wares, _("Wares (total)"));

	m_all_workers = new WaresDisplay(tabs, 0, 0, plr.player().tribe(), Widelands::wwWORKER, false);
	tabs->add
		("workers_total", g_gr->images().get(pic_tab_workers),
		 m_all_workers, _("Workers (total)"));

	m_warehouse_wares = new WaresDisplay(tabs, 0, 0, plr.player().tribe(), Widelands::wwWARE, false);
	tabs->add
		("wares_in_warehouses",
		 g_gr->images().get (pic_tab_wares_warehouse),
		 m_warehouse_wares, _("Wares in warehouses")
	);

	m_warehouse_workers = new WaresDisplay(tabs, 0, 0, plr.player().tribe(), Widelands::wwWORKER, false);
	tabs->add
		("workers_in_warehouses",
		 g_gr->images().get(pic_tab_workers_warehouse),
		 m_warehouse_workers, _("Workers in warehouses")
	);
}

/*
===============
Push the current wares status to the WaresDisplay.
===============
*/
void Stock_Menu::think()
{
	UI::UniqueWindow::think();

	fill_total_waresdisplay(m_all_wares, Widelands::wwWARE);
	fill_total_waresdisplay(m_all_workers, Widelands::wwWORKER);
	fill_warehouse_waresdisplay(m_warehouse_wares, Widelands::wwWARE);
	fill_warehouse_waresdisplay(m_warehouse_workers, Widelands::wwWORKER);
}

/**
 * Keep the list of wares repositories up-to-date (honoring that the set of
 * \ref Economy of a player may change)
 */
void Stock_Menu::fill_total_waresdisplay
	(WaresDisplay * waresdisplay, Widelands::WareWorker type)
{
	waresdisplay->remove_all_warelists();
	const Widelands::Player & player = *m_player.get_player();
	const uint32_t nrecos = player.get_nr_economies();
	for (uint32_t i = 0; i < nrecos; ++i)
		waresdisplay->add_warelist
			(type == Widelands::wwWARE ?
			 player.get_economy_by_number(i)->get_wares() :
			 player.get_economy_by_number(i)->get_workers());
}

/**
 * Keep the list of wares repositories up-to-date (consider that the available
 * \ref Warehouse may change)
 */
void Stock_Menu::fill_warehouse_waresdisplay
	(WaresDisplay * waresdisplay, Widelands::WareWorker type)
{
	waresdisplay->remove_all_warelists();
	const Widelands::Player & player = *m_player.get_player();
	const uint32_t nrecos = player.get_nr_economies();
	for (uint32_t i = 0; i < nrecos; ++i) {
		const std::vector<Widelands::Warehouse *> & warehouses =
			player.get_economy_by_number(i)->warehouses();

		for
			(std::vector<Widelands::Warehouse *>::const_iterator it =
			 warehouses.begin();
			 it != warehouses.end();
			 ++it)
		{
			waresdisplay->add_warelist
				(type == Widelands::wwWARE ?
				 (*it)->get_wares() : (*it)->get_workers());
		}
	}
}
