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

#include "wui/general_statistics_menu.h"

#include <memory>

#include "base/i18n.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warelist.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/slider.h"
#include "wui/interactive_player.h"

#define PLOT_HEIGHT 145
#define NR_BASE_DATASETS 11

GeneralStatisticsMenu::GeneralStatisticsMenu(InteractiveGameBase& parent,
                                             GeneralStatisticsMenu::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "statistics_menu",
                      &registry,
                      440,
                      400,
                      _("General Statistics")),
     my_registry_(&registry),
     box_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, 5),
     player_buttons_box_(&box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, 1),
     plot_(&box_,
           0,
           0,
           430,
           PLOT_HEIGHT,
           Widelands::kStatisticsSampleTime.get(),
           WuiPlotArea::Plotmode::kAbsolute),
     selected_information_(0),
     game_(*parent.get_game()),
     subscriber_(Notifications::subscribe<Widelands::NotePlayerDetailsEvent>(
        [this](const Widelands::NotePlayerDetailsEvent& note) {
	        if (note.event ==
	            Widelands::NotePlayerDetailsEvent::Event::kGeneralStatisticsVisibilityChanged) {
		        save_state_to_registry();
		        create_player_buttons();
	        }
        })) {
	assert(my_registry_);

	selected_information_ = my_registry_->selected_information;

	set_center_panel(&box_);
	box_.set_border(5, 5, 5, 5);

	// Setup plot data
	const Widelands::Game::GeneralStatsVector& genstats = game_.get_general_statistics();
	const Widelands::Game::GeneralStatsVector::size_type general_statistics_size =
	   game_.map().get_nrplayers();
	if (genstats.size() != general_statistics_size) {
		// Create first data point
		game_.sample_statistics();
	}

	// Is there a hook dataset?
	ndatasets_ = NR_BASE_DATASETS;
	std::unique_ptr<LuaTable> hook = game_.lua().get_hook("custom_statistic");
	std::string cs_name, cs_pic;
	if (hook) {
		i18n::Textdomain td("win_conditions");
		hook->do_not_warn_about_unaccessed_keys();
		cs_name = _(hook->get_string("name"));
		cs_pic = hook->get_string("pic");
		ndatasets_++;
	}

	for (Widelands::Game::GeneralStatsVector::size_type i = 0; i < general_statistics_size; ++i) {
		const Widelands::Player* p = parent.game().get_player(i + 1);
		const RGBColor& color = p ? p->get_playercolor() :
                                  // The plot is always invisible if this player doesn't
                                  // exist, but we need to assign a color anyway
                                  kPlayerColors[i];
		plot_.register_plot_data(i * ndatasets_ + 0, &genstats[i].land_size, color);
		plot_.register_plot_data(i * ndatasets_ + 1, &genstats[i].nr_workers, color);
		plot_.register_plot_data(i * ndatasets_ + 2, &genstats[i].nr_buildings, color);
		plot_.register_plot_data(i * ndatasets_ + 3, &genstats[i].nr_wares, color);
		plot_.register_plot_data(i * ndatasets_ + 4, &genstats[i].productivity, color);
		plot_.register_plot_data(i * ndatasets_ + 5, &genstats[i].nr_casualties, color);
		plot_.register_plot_data(i * ndatasets_ + 6, &genstats[i].nr_kills, color);
		plot_.register_plot_data(i * ndatasets_ + 7, &genstats[i].nr_msites_lost, color);
		plot_.register_plot_data(i * ndatasets_ + 8, &genstats[i].nr_msites_defeated, color);
		plot_.register_plot_data(i * ndatasets_ + 9, &genstats[i].nr_civil_blds_lost, color);
		plot_.register_plot_data(i * ndatasets_ + 10, &genstats[i].miltary_strength, color);
		if (hook) {
			plot_.register_plot_data(i * ndatasets_ + 11, &genstats[i].custom_statistic, color);
		}
		if (game_.get_player(i + 1)) {  // Show area plot
			plot_.show_plot(i * ndatasets_ + selected_information_, my_registry_->selected_players[i]);
		}
	}

	plot_.set_time(my_registry_->time);

	// Setup Widgets
	box_.add(&plot_);

	create_player_buttons();
	box_.add(&player_buttons_box_, UI::Box::Resizing::kFullSize);

	UI::Box* hbox2 = new UI::Box(&box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, 1);

	UI::Radiobutton* btn;

	const Vector2i zero = Vector2i::zero();

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_landsize.png"), _("Land"),
	                       &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_nrworkers.png"),
	                       _("Workers"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_nrbuildings.png"),
	                       _("Buildings"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_nrwares.png"), _("Wares"),
	                       &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_productivity.png"),
	                       _("Productivity"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_casualties.png"),
	                       _("Casualties"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_kills.png"), _("Kills"),
	                       &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_msites_lost.png"),
	                       _("Military buildings lost"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_msites_defeated.png"),
	                       _("Military buildings defeated"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_civil_blds_lost.png"),
	                       _("Civilian buildings lost"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	radiogroup_.add_button(hbox2, UI::PanelStyle::kWui, zero,
	                       g_image_cache->get("images/wui/stats/genstats_militarystrength.png"),
	                       _("Military"), &btn);
	hbox2->add(btn, UI::Box::Resizing::kFillSpace);

	if (hook) {
		radiogroup_.add_button(
		   hbox2, UI::PanelStyle::kWui, zero, g_image_cache->get(cs_pic), cs_name, &btn);
		hbox2->add(btn, UI::Box::Resizing::kFillSpace);
	}

	radiogroup_.set_state(selected_information_, false);
	radiogroup_.changedto.connect([this](int32_t i) { radiogroup_changed(i); });

	box_.add(hbox2, UI::Box::Resizing::kFullSize);

	slider_ = new WuiPlotAreaSlider(&box_, plot_, 0, 0, 100, 45);
	slider_->changedto.connect([this](int32_t i) { plot_.set_time_id(i); });
	box_.add(slider_, UI::Box::Resizing::kFullSize);

	initialization_complete();
}

void GeneralStatisticsMenu::create_player_buttons() {
	// Delete existing buttons, if any
	player_buttons_box_.clear();
	player_buttons_box_.free_children();

	Widelands::PlayerNumber const nr_players = game_.map().get_nrplayers();

	bool show_all_players = true;
	const Widelands::Player* self = nullptr;
	if (upcast(InteractivePlayer, ipl, get_parent())) {
		show_all_players = ipl->player().see_all() || ipl->omnipotent();
		self = &ipl->player();
	}

	for (UI::Button*& c : cbs_) {
		c = nullptr;
	}
	iterate_players_existing_const(p, nr_players, game_, player) {
		if (player != self && !show_all_players && player->is_hidden_from_general_statistics()) {
			// Hide player from stats
			cbs_[p - 1] = nullptr;
			// Also hide the plot for this player if it was previously visible
			show_or_hide_plot(p, false);
			continue;
		}

		const Image* player_image =
		   playercolor_image(player->get_playercolor(), "images/players/genstats_player.png");
		assert(player_image);
		UI::Button& cb = *new UI::Button(&player_buttons_box_, "playerbutton", 0, 0, 25, 25,
		                                 UI::ButtonStyle::kWuiMenu, player_image, player->get_name());
		cb.sigclicked.connect([this, p]() { cb_changed_to(p); });
		cb.set_perm_pressed(my_registry_->selected_players[p - 1]);

		cbs_[p - 1] = &cb;
		show_or_hide_plot(p, my_registry_->selected_players[p - 1]);

		player_buttons_box_.add(&cb, UI::Box::Resizing::kFillSpace);
	}
}

GeneralStatisticsMenu::~GeneralStatisticsMenu() {
	save_state_to_registry();
}

void GeneralStatisticsMenu::save_state_to_registry() {
	if (game_.is_loaded()) {
		// Save information for recreation, if window is reopened
		my_registry_->selected_information = selected_information_;
		my_registry_->time = plot_.get_time();
		Widelands::PlayerNumber const nr_players = game_.map().get_nrplayers();
		iterate_players_existing_novar(p, nr_players, game_) {
			my_registry_->selected_players[p - 1] =
			   !cbs_[p - 1] || cbs_[p - 1]->style() == UI::Button::VisualState::kPermpressed;
		}
	}
}

/*
 * Cb has been changed to this state
 */
void GeneralStatisticsMenu::cb_changed_to(int32_t const id) {
	// This represents our player number
	cbs_[id - 1]->toggle();
	show_or_hide_plot(id, cbs_[id - 1]->style() == UI::Button::VisualState::kPermpressed);
}

void GeneralStatisticsMenu::show_or_hide_plot(const int32_t id, const bool show) {
	plot_.show_plot((id - 1) * ndatasets_ + selected_information_, show);
}

/*
 * The radiogroup has changed
 */
void GeneralStatisticsMenu::radiogroup_changed(int32_t const id) {
	size_t const statistics_size = game_.get_general_statistics().size();
	for (uint32_t i = 0; i < statistics_size; ++i) {
		if (cbs_[i]) {
			plot_.show_plot(
			   i * ndatasets_ + id, cbs_[i]->style() == UI::Button::VisualState::kPermpressed);
			plot_.show_plot(i * ndatasets_ + selected_information_, false);
		}
	}
	selected_information_ = id;
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& GeneralStatisticsMenu::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			UI::UniqueWindow::Registry& r =
			   dynamic_cast<InteractivePlayer&>(ib).menu_windows_.stats_general;
			r.create();
			assert(r.window);
			GeneralStatisticsMenu& m = dynamic_cast<GeneralStatisticsMenu&>(*r.window);
			m.radiogroup_.set_state(fr.unsigned_8(), true);
			for (unsigned i = 0; i < kMaxPlayers; ++i) {
				if (fr.unsigned_8()) {
					m.cb_changed_to(i + 1);
				}
			}
			m.slider_->get_slider().set_value(fr.signed_32());
			return m;
		} else {
			throw Widelands::UnhandledVersionError(
			   "General Statistics Menu", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw Widelands::GameDataError("general statistics menu: %s", e.what());
	}
}
void GeneralStatisticsMenu::save(FileWrite& fw, Widelands::MapObjectSaver&) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.unsigned_8(radiogroup_.get_state());
	for (UI::Button* c : cbs_) {
		// The saved value indicates whether we explicitly need to toggle this button
		fw.unsigned_8((c && c->style() != UI::Button::VisualState::kPermpressed) ? 1 : 0);
	}
	fw.signed_32(slider_->get_slider().get_value());
}
