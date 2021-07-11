/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
#include "graphic/style_manager.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "wui/interactive_player.h"

static const char pic_tab_wares[] = "images/wui/buildings/menu_tab_wares.png";
static const char pic_tab_workers[] = "images/wui/buildings/menu_tab_workers.png";
static const char pic_tab_wares_warehouse[] = "images/wui/stats/menu_tab_wares_warehouse.png";
static const char pic_tab_workers_warehouse[] = "images/wui/stats/menu_tab_workers_warehouse.png";

static inline std::string
color_tag(const RGBColor& c, const std::string& text1, const std::string& text2) {
	return (boost::format(_("%1$s %2$s")) % StyleManager::color_tag(text1, c) % text2).str();
}

StockMenu::StockMenu(InteractivePlayer& plr, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&plr, UI::WindowStyle::kWui, "stock_menu", &registry, 480, 640, _("Stock")),
     player_(plr),
     colors_(g_style_manager->building_statistics_style()),
     main_box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     tabs_(&main_box_, UI::TabPanelStyle::kWuiDark),
     solid_icon_backgrounds_(
        &main_box_,
        UI::PanelStyle::kWui,
        Vector2i::zero(),
        /** TRANSLATORS: If this checkbox is ticked, all icons in the stock menu are drawn with
           different background colors; each icon's color indicates whether the stock is higher or
           lower than the economy target setting. Very little space is available. */
        _("Evaluate"),
        (boost::format("<rt><p>%s</p><p>%s<br>%s<br>%s</p></rt>") %
         g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
            .as_font_tag(_("Compare stocked amounts to economy target quantities")) %
         g_style_manager->font_style(UI::FontStyle::kWuiTooltip)
            .as_font_tag(color_tag(
               colors_.alternative_low_color(), _("Red:"), _("Stock is lower than the target"))) %
         g_style_manager->font_style(UI::FontStyle::kWuiTooltip)
            .as_font_tag(color_tag(colors_.alternative_medium_color(),
                                   _("Yellow:"),
                                   _("Stock is equal to the target"))) %
         g_style_manager->font_style(UI::FontStyle::kWuiTooltip)
            .as_font_tag(color_tag(colors_.alternative_high_color(),
                                   _("Green:"),
                                   _("Stock is higher than the target"))))
           .str()) {
	set_center_panel(&main_box_);

	all_wares_ = new StockMenuWaresDisplay(&tabs_, 0, 0, plr.player(), Widelands::wwWARE);
	tabs_.add("total_wares", g_image_cache->get(pic_tab_wares), all_wares_, _("Wares (total)"));

	all_workers_ = new StockMenuWaresDisplay(&tabs_, 0, 0, plr.player(), Widelands::wwWORKER);
	tabs_.add(
	   "workers_total", g_image_cache->get(pic_tab_workers), all_workers_, _("Workers (total)"));

	warehouse_wares_ = new StockMenuWaresDisplay(&tabs_, 0, 0, plr.player(), Widelands::wwWARE);
	tabs_.add("wares_in_warehouses", g_image_cache->get(pic_tab_wares_warehouse), warehouse_wares_,
	          _("Wares in warehouses"));

	warehouse_workers_ = new StockMenuWaresDisplay(&tabs_, 0, 0, plr.player(), Widelands::wwWORKER);
	tabs_.add("workers_in_warehouses", g_image_cache->get(pic_tab_workers_warehouse),
	          warehouse_workers_, _("Workers in warehouses"));

	solid_icon_backgrounds_.changedto.connect([this](const bool b) {
		all_wares_->set_solid_icon_backgrounds(!b);
		all_workers_->set_solid_icon_backgrounds(!b);
		warehouse_wares_->set_solid_icon_backgrounds(!b);
		warehouse_workers_->set_solid_icon_backgrounds(!b);
	});

	main_box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	main_box_.add(&solid_icon_backgrounds_, UI::Box::Resizing::kFullSize);

	// Preselect the wares_in_warehouses tab
	tabs_.activate(2);

	initialization_complete();
}

void StockMenu::layout() {
	UI::UniqueWindow::layout();
	if (!is_minimal()) {
		int w1, w2, h1, h2;
		solid_icon_backgrounds_.get_desired_size(&w1, &h1);
		tabs_.get_desired_size(&w2, &h2);
		main_box_.set_size(w2, h1 + h2);
	}
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

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& StockMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_stock;
			r.create();
			assert(r.window);
			StockMenu& sm = dynamic_cast<StockMenu&>(*r.window);

			sm.tabs_.activate(fr.unsigned_8());
			sm.solid_icon_backgrounds_.set_state(fr.unsigned_8());

			return sm;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Stock Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("stock menu: %s", e.what());
	}
}
void StockMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(tabs_.active());
	fw.unsigned_8(solid_icon_backgrounds_.get_state() ? 1 : 0);
}
