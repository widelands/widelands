/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "base/i18n.h"
#include "economy/economy.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/tabpanel.h"
#include "wui/interactive_player.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";
static const char pic_tab_wares_warehouse[] = "images/wui/stats/menu_tab_wares_warehouse.png";
static const char pic_tab_workers_warehouse[] = "images/wui/stats/menu_tab_workers_warehouse.png";

StockMenu::StockMenu(InteractivePlayer& plr, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&plr, UI::WindowStyle::kWui, "stock_menu", &registry, 480, 640, _("Stock")),
     player_(plr) {
	UI::Box* main_box = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
	set_center_panel(main_box);
	UI::TabPanel* tabs = new UI::TabPanel(main_box, UI::TabPanelStyle::kWuiDark);

	all_wares_ = new StockMenuWaresDisplay(tabs, 0, 0, plr.player(), Widelands::wwWARE);
	tabs->add("total_wares", g_image_cache->get(pic_tab_wares), all_wares_, _("Wares (total)"));

	all_workers_ = new StockMenuWaresDisplay(tabs, 0, 0, plr.player(), Widelands::wwWORKER);
	tabs->add(
	   "workers_total", g_image_cache->get(pic_tab_workers), all_workers_, _("Workers (total)"));

	warehouse_wares_ = new StockMenuWaresDisplay(tabs, 0, 0, plr.player(), Widelands::wwWARE);
	tabs->add("wares_in_warehouses", g_image_cache->get(pic_tab_wares_warehouse), warehouse_wares_,
	          _("Wares in warehouses"));

	warehouse_workers_ = new StockMenuWaresDisplay(tabs, 0, 0, plr.player(), Widelands::wwWORKER);
	tabs->add("workers_in_warehouses", g_image_cache->get(pic_tab_workers_warehouse),
	          warehouse_workers_, _("Workers in warehouses"));

	UI::Checkbox* solid_icon_backgrounds = new UI::Checkbox(main_box, UI::PanelStyle::kWui, Vector2i::zero(),
			/** TRANSLATORS: If this checkbox is ticked, all icons in the stock menu are drawn with the same background color. Very little space is available. */
			_("Monochrome"));
	solid_icon_backgrounds->changedto.connect([this](const bool b) {
		all_wares_->set_solid_icon_backgrounds(b);
		all_workers_->set_solid_icon_backgrounds(b);
		warehouse_wares_->set_solid_icon_backgrounds(b);
		warehouse_workers_->set_solid_icon_backgrounds(b);
	});

	main_box->add(tabs, UI::Box::Resizing::kExpandBoth);
	main_box->set_max_size(tabs->get_w(), plr.get_h());
	main_box->add(solid_icon_backgrounds, UI::Box::Resizing::kFullSize);

	// Preselect the wares_in_warehouses tab
	tabs->activate(2);
}

/*
===============
Push the current wares status to the WaresDisplay.
===============
*/
void StockMenu::think() {
	UI::UniqueWindow::think();

	fill_total_waresdisplay(all_wares_, Widelands::wwWARE);
	fill_total_waresdisplay(all_workers_, Widelands::wwWORKER);
	fill_warehouse_waresdisplay(warehouse_wares_, Widelands::wwWARE);
	fill_warehouse_waresdisplay(warehouse_workers_, Widelands::wwWORKER);
}

/**
 * Keep the list of wares repositories up-to-date (honoring that the set of
 * \ref Economy of a player may change)
 */
void StockMenu::fill_total_waresdisplay(WaresDisplay* waresdisplay, Widelands::WareWorker type) {
	waresdisplay->remove_all_warelists();
	const Widelands::Player& player = *player_.get_player();
	for (const auto& economy : player.economies()) {
		if (economy.second->type() == type) {
			waresdisplay->add_warelist(economy.second->get_wares_or_workers());
		}
	}
}

/**
 * Keep the list of wares repositories up-to-date (consider that the available
 * \ref Warehouse may change)
 */
void StockMenu::fill_warehouse_waresdisplay(WaresDisplay* waresdisplay,
                                            Widelands::WareWorker type) {
	waresdisplay->remove_all_warelists();
	for (const auto& economy : player_.player().economies()) {
		if (economy.second->type() == type) {
			for (const auto* warehouse : economy.second->warehouses()) {
				waresdisplay->add_warelist(type == Widelands::wwWARE ? warehouse->get_wares() :
				                                                       warehouse->get_workers());
			}
		}
	}
}
