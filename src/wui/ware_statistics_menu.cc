/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wui/ware_statistics_menu.h"

#include <functional>

#include "base/i18n.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/player.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "ui_basic/tabpanel.h"
#include "wui/interactive_player.h"
#include "wui/plot_area.h"
#include "wui/waresdisplay.h"

constexpr int kPlotHeight = 145;
constexpr int kPlotWidth = 250;
constexpr int kSpacing = 5;

constexpr int kInactiveColorIndex = 0;

static const char pic_tab_production[] = "images/wui/stats/menu_tab_wares_production.png";
static const char pic_tab_consumption[] = "images/wui/stats/menu_tab_wares_consumption.png";
static const char pic_tab_economy[] = "images/wui/stats/menu_tab_wares_econ_health.png";
static const char pic_tab_stock[] = "images/wui/stats/menu_tab_wares_stock.png";

static const RGBColor colors[] = {
   RGBColor(115, 115, 115),  // inactive
   RGBColor(255, 0, 0),     RGBColor(0, 144, 12),    RGBColor(0, 0, 255),    RGBColor(249, 125, 6),
   RGBColor(255, 204, 0),   RGBColor(101, 0, 33),    RGBColor(0, 143, 255),  RGBColor(0, 70, 31),
   RGBColor(106, 44, 5),    RGBColor(143, 1, 1),     RGBColor(0, 255, 0),    RGBColor(85, 0, 111),
   RGBColor(0, 243, 255),   RGBColor(32, 36, 32),    RGBColor(189, 118, 24), RGBColor(255, 0, 51),
   RGBColor(153, 204, 0),   RGBColor(0, 60, 77),     RGBColor(77, 15, 224),  RGBColor(255, 0, 208),
   RGBColor(255, 51, 0),    RGBColor(43, 72, 183),   RGBColor(255, 93, 93),  RGBColor(15, 177, 18),
   RGBColor(252, 89, 242),  RGBColor(255, 255, 255), RGBColor(61, 214, 128), RGBColor(102, 102, 0),
   RGBColor(169, 118, 93),  RGBColor(255, 204, 102), RGBColor(65, 50, 106),  RGBColor(220, 255, 0),
   RGBColor(204, 51, 0),    RGBColor(192, 0, 67),    RGBColor(120, 203, 65), RGBColor(204, 255, 51),
   RGBColor(20, 104, 109),  RGBColor(129, 80, 21),   RGBColor(153, 153, 0),  RGBColor(221, 69, 77),
   RGBColor(34, 136, 83),   RGBColor(106, 107, 207), RGBColor(204, 102, 51), RGBColor(127, 28, 75),
   RGBColor(111, 142, 204), RGBColor(174, 10, 0),    RGBColor(96, 143, 71),  RGBColor(163, 74, 128),
   RGBColor(183, 142, 10),  RGBColor(105, 155, 160),  // shark infested water, run!
};

static const uint32_t colors_length = sizeof(colors) / sizeof(RGBColor);

struct StatisticWaresDisplay : public AbstractWaresDisplay {
private:
	std::map<Widelands::DescriptionIndex, uint8_t>& color_map_;

public:
	StatisticWaresDisplay(UI::Panel* const parent,
	                      int32_t const x,
	                      int32_t const y,
	                      const Widelands::TribeDescr& tribe,
	                      std::function<void(Widelands::DescriptionIndex, bool)> callback_function,
	                      std::map<Widelands::DescriptionIndex, uint8_t>& color_map)
	   : AbstractWaresDisplay(
	        parent, x, y, tribe, Widelands::wwWARE, true, std::move(callback_function)),
	     color_map_(color_map) {
		int w, h;
		get_desired_size(&w, &h);
		set_size(w, h);
	}

protected:
	std::string info_for_ware(Widelands::DescriptionIndex const /* ware */) override {
		return "";
	}

	RGBColor info_color_for_ware(Widelands::DescriptionIndex const ware) override {
		return colors[color_map_.at(ware)];
	}
};

WareStatisticsMenu::WareStatisticsMenu(InteractivePlayer& parent,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "ware_statistics",
                      &registry,
                      kPlotWidth + 2 * kSpacing,
                      270,
                      _("Ware Statistics")),
     iplayer_(parent),
     main_box_(nullptr),
     tab_panel_(nullptr),
     display_(nullptr),
     slider_(nullptr) {

	const Widelands::TribeDescr& player_tribe = parent.get_player()->tribe();

	// Init color sets
	for (Widelands::DescriptionIndex d : player_tribe.wares()) {
		color_map_[d] = kInactiveColorIndex;
	}
	active_colors_.resize(colors_length);
	std::fill(active_colors_.begin(), active_colors_.end(), 0);

	//  First, we must decide about the size.
	main_box_ = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, 5);
	main_box_->set_border(kSpacing, kSpacing, kSpacing, kSpacing);
	set_center_panel(main_box_);

	// Setup plot widgets
	// Create a tabbed environment for the different plots
	tab_panel_ = new UI::TabPanel(main_box_, UI::TabPanelStyle::kWuiDark);

	plot_production_ =
	   new WuiPlotArea(tab_panel_, 0, 0, kPlotWidth, kPlotHeight + kSpacing,
	                   Widelands::kStatisticsSampleTime.get(), WuiPlotArea::Plotmode::kRelative);

	tab_panel_->add(
	   "production", g_image_cache->get(pic_tab_production), plot_production_, _("Production"));

	plot_consumption_ =
	   new WuiPlotArea(tab_panel_, 0, 0, kPlotWidth, kPlotHeight + kSpacing,
	                   Widelands::kStatisticsSampleTime.get(), WuiPlotArea::Plotmode::kRelative);

	tab_panel_->add(
	   "consumption", g_image_cache->get(pic_tab_consumption), plot_consumption_, _("Consumption"));

	plot_economy_ = new DifferentialPlotArea(tab_panel_, 0, 0, kPlotWidth, kPlotHeight + kSpacing,
	                                         Widelands::kStatisticsSampleTime.get(),
	                                         WuiPlotArea::Plotmode::kRelative);

	tab_panel_->add(
	   "economy_health", g_image_cache->get(pic_tab_economy), plot_economy_, _("Economy health"));

	plot_stock_ =
	   new WuiPlotArea(tab_panel_, 0, 0, kPlotWidth, kPlotHeight + kSpacing,
	                   Widelands::kStatisticsSampleTime.get(), WuiPlotArea::Plotmode::kAbsolute);

	tab_panel_->add("stock", g_image_cache->get(pic_tab_stock), plot_stock_, _("Stock"));

	tab_panel_->activate(0);

	// Add tabbed environment to box
	main_box_->add(tab_panel_, UI::Box::Resizing::kFullSize);

	// Register statistics data
	for (Widelands::DescriptionIndex cur_ware : player_tribe.wares()) {
		plot_production_->register_plot_data(
		   cur_ware,
		   parent.get_player()->get_ware_production_statistics(Widelands::DescriptionIndex(cur_ware)),
		   colors[kInactiveColorIndex]);

		plot_consumption_->register_plot_data(cur_ware,
		                                      parent.get_player()->get_ware_consumption_statistics(
		                                         Widelands::DescriptionIndex(cur_ware)),
		                                      colors[kInactiveColorIndex]);

		plot_economy_->register_plot_data(
		   cur_ware,
		   parent.get_player()->get_ware_production_statistics(Widelands::DescriptionIndex(cur_ware)),
		   colors[kInactiveColorIndex]);

		plot_economy_->register_negative_plot_data(
		   cur_ware, parent.get_player()->get_ware_consumption_statistics(
		                Widelands::DescriptionIndex(cur_ware)));

		plot_stock_->register_plot_data(
		   cur_ware,
		   parent.get_player()->get_ware_stock_statistics(Widelands::DescriptionIndex(cur_ware)),
		   colors[kInactiveColorIndex]);
	}

	display_ = new StatisticWaresDisplay(
	   main_box_, 0, 0, parent.get_player()->tribe(),
	   [this](const int ware_index, const bool what) { cb_changed_to(ware_index, what); },
	   color_map_);
	display_->set_min_free_vertical_space(400);
	main_box_->add(display_, UI::Box::Resizing::kFullSize);

	slider_ = new WuiPlotAreaSlider(main_box_, *plot_production_, 0, 0, kPlotWidth, 45);
	slider_->changedto.connect([this](const int32_t timescale) { set_time(timescale); });
	main_box_->add(slider_, UI::Box::Resizing::kFullSize);

	initialization_complete();
}

/**
 * Callback for the ware buttons. Change the state of all ware statistics
 * simultaneously.
 */
void WareStatisticsMenu::cb_changed_to(Widelands::DescriptionIndex id, bool what) {
	if (what) {  // Activate ware
		// Search lowest free color
		uint8_t color_index = kInactiveColorIndex;

		for (uint32_t i = 0; i < active_colors_.size(); ++i) {
			if (!active_colors_[i]) {
				// Prevent index out of bounds
				color_index = std::min(i + 1, colors_length - 1);
				active_colors_[i] = true;
				break;
			}
		}

		// Assign color
		color_map_[static_cast<size_t>(id)] = color_index;
		plot_production_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_consumption_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_economy_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);
		plot_stock_->set_plotcolor(static_cast<size_t>(id), colors[color_index]);

		active_indices_.push_back(id);
	} else {  // Deactivate ware
		uint8_t old_color = color_map_[static_cast<size_t>(id)];
		if (old_color != kInactiveColorIndex) {
			active_colors_[old_color - 1] = false;
			color_map_[static_cast<size_t>(id)] = kInactiveColorIndex;
		}
		active_indices_.erase(std::find(active_indices_.begin(), active_indices_.end(), id));
	}

	plot_production_->show_plot(static_cast<size_t>(id), what);
	plot_consumption_->show_plot(static_cast<size_t>(id), what);
	plot_economy_->show_plot(static_cast<size_t>(id), what);
	plot_stock_->show_plot(static_cast<size_t>(id), what);
}

static bool layouting = false;
void WareStatisticsMenu::layout() {
	if (layouting || !tab_panel_ || !display_ || !slider_ || !main_box_) {
		return;
	}
	layouting = true;

	display_->set_hgap(3, false);
	int w1, h1, w2, h2, w3, h3;
	tab_panel_->get_desired_size(&w1, &h1);
	display_->get_desired_size(&w2, &h2);
	slider_->get_desired_size(&w3, &h3);

	display_->set_hgap(
	   std::max(3, AbstractWaresDisplay::calc_hgap(display_->get_extent().w, kPlotWidth)), false);
	display_->get_desired_size(&w2, &h2);

	main_box_->set_desired_size(std::max(w2, kPlotWidth) + 2 * kSpacing,
	                            h1 + h2 + h3 + text_height(UI::FontStyle::kWuiLabel));
	UI::UniqueWindow::layout();
	layouting = false;
}

/**
 * Callback for the time buttons. Change the time axis of all ware
 * statistics simultaneously.
 */
void WareStatisticsMenu::set_time(int32_t timescale) {
	plot_production_->set_time_id(timescale);
	plot_consumption_->set_time_id(timescale);
	plot_economy_->set_time_id(timescale);
	plot_stock_->set_time_id(timescale);
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& WareStatisticsMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_wares;
			r.create();
			assert(r.window);
			WareStatisticsMenu& m = dynamic_cast<WareStatisticsMenu&>(*r.window);
			m.tab_panel_->activate(fr.unsigned_8());
			m.slider_->get_slider().set_value(fr.signed_32());
			for (size_t i = fr.unsigned_32(); i; --i) {
				m.display_->select_ware(ib.egbase().descriptions().safe_ware_index(fr.string()));
			}
			return m;
		} else {
			throw Widelands::UnhandledVersionError(
			   "Wares Statistics Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("wares statistics menu: %s", e.what());
	}
}
void WareStatisticsMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(tab_panel_->active());
	fw.signed_32(slider_->get_slider().get_value());
	fw.unsigned_32(active_indices_.size());
	for (const Widelands::DescriptionIndex& di : active_indices_) {
		fw.string(iplayer_.egbase().descriptions().get_ware_descr(di)->name());
	}
}
