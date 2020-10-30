/*
 * Copyright (C) 2004-2020 by the Widelands Development Team
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

#include "ui_fsmenu/netsetup_lan.h"

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "network/constants.h"
#include "network/internet_gaming.h"
#include "network/network.h"
#include "wlapplication_options.h"

FullscreenMenuNetSetupLAN::FullscreenMenuNetSetupLAN(FullscreenMenuMain& fsmm)
   : FullscreenMenuLoadMapOrGame(fsmm, _("Begin LAN Game")),
     // Boxes
     left_column_(this, 0, 0, UI::Box::Vertical),
     right_column_(this, 0, 0, UI::Box::Vertical),

     // Left column content
     label_opengames_(&left_column_, 0, 0, 0, 0, _("List of games in your local network:")),
     table_(&left_column_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),

     // Right column content
     label_playername_(&right_column_, 0, 0, 0, 0, _("Your nickname:")),
     playername_(&right_column_, 0, 0, 0, UI::PanelStyle::kFsMenu),
     label_hostname_(&right_column_, 0, 0, 0, 0, _("Host to connect:")),

     host_box_(&right_column_, 0, 0, UI::Box::Horizontal),
     hostname_(&host_box_, 0, 0, 0, UI::PanelStyle::kFsMenu),
     loadlasthost_(&host_box_,
                   "load_previous_host",
                   0,
                   0,
                   hostname_.get_h(),
                   hostname_.get_h(),
                   UI::ButtonStyle::kFsMenuSecondary,
                   g_image_cache->get("images/ui_fsmenu/menu_load_game.png"),
                   _("Load previous host")),
     // Buttons
     joingame_(&right_column_,
               "join_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Join this game")),
     hostgame_(&right_column_,
               "host_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Host a new game")) {

	ok_.set_visible(false);  // We have 2 starting buttons, so we need a different layout here.

	left_column_.add(&label_opengames_, UI::Box::Resizing::kFullSize);
	left_column_.add_space(padding_);
	left_column_.add(&table_, UI::Box::Resizing::kExpandBoth);

	right_column_.set_inner_spacing(padding_);
	right_column_.add(&label_playername_, UI::Box::Resizing::kFullSize);
	right_column_.add(&playername_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add(&label_hostname_, UI::Box::Resizing::kFullSize);
	right_column_.add(&host_box_, UI::Box::Resizing::kFullSize);
	right_column_.add_space(padding_);
	right_column_.add(&joingame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add_inf_space();
	right_column_.add(&hostgame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add_inf_space();
	right_column_.add_inf_space();
	right_column_.add_inf_space();

	host_box_.add(&hostname_, UI::Box::Resizing::kExpandBoth);
	host_box_.add_space(padding_);
	host_box_.add(&loadlasthost_);

	joingame_.sigclicked.connect([this]() { clicked_joingame(); });
	hostgame_.sigclicked.connect([this]() { clicked_hostgame(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
	loadlasthost_.sigclicked.connect([this]() { clicked_lasthost(); });

	hostname_.changed.connect([this]() { change_hostname(); });
	playername_.set_text(get_config_string("nickname", (_("nobody"))));
	playername_.changed.connect([this]() { change_playername(); });
	table_.add_column(190, _("Host"));
	table_.add_column(0, _("Map"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.add_column(90, _("State"));
	table_.selected.connect([this](int32_t i) { game_selected(i); });
	table_.double_clicked.connect([this](int32_t i) { game_doubleclicked(i); });
	discovery_.set_callback(discovery_callback, this);

	joingame_.set_enabled(false);
	layout();

	initialization_complete();
}

void FullscreenMenuNetSetupLAN::layout() {
	FullscreenMenuLoadMapOrGame::layout();

	butw_ = get_inner_w() - right_column_x_ - right_column_margin_;
	const int colum_header_h = label_opengames_.get_h() + padding_;

	left_column_.set_size(tablew_, tableh_ + colum_header_h);
	left_column_.set_pos(Vector2i(tablex_, tabley_ - colum_header_h));

	right_column_.set_size(
	   get_right_column_w(right_column_x_), tableh_ + colum_header_h - buth_ - 4 * padding_);
	right_column_.set_pos(Vector2i(right_column_x_, tabley_ - colum_header_h));

	// Buttons
	joingame_.set_desired_size(butw_, buth_);
	hostgame_.set_desired_size(butw_, buth_);

	back_.set_size(butw_, buth_);
	back_.set_pos(Vector2i(right_column_x_, buty_));
}

void FullscreenMenuNetSetupLAN::think() {
	FullscreenMenuLoadMapOrGame::think();
	change_playername();

	discovery_.run();
}

bool FullscreenMenuNetSetupLAN::get_host_address(NetAddress* addr) {
	const std::string& host = hostname_.text();

	for (uint32_t i = 0; i < table_.size(); ++i) {
		const NetOpenGame& game = *table_[i];

		if (!strcmp(game.info.hostname, host.c_str())) {
			*addr = game.address;
			return true;
		}
	}

	// The user probably entered a hostname on his own. Try to resolve it
	if (NetAddress::resolve_to_v6(addr, host, kWidelandsLanPort)) {
		return true;
	}
	if (NetAddress::resolve_to_v4(addr, host, kWidelandsLanPort)) {
		return true;
	}
	return false;
}

const std::string& FullscreenMenuNetSetupLAN::get_playername() {
	return playername_.text();
}

void FullscreenMenuNetSetupLAN::clicked_ok() {
	if (hostname_.text().empty()) {
		clicked_hostgame();
	} else {
		clicked_joingame();
	}
}

void FullscreenMenuNetSetupLAN::game_selected(uint32_t) {
	if (table_.has_selection()) {
		if (const NetOpenGame* const game = table_.get_selected()) {
			hostname_.set_text(game->info.hostname);
			joingame_.set_enabled(true);
		}
	}
}

void FullscreenMenuNetSetupLAN::game_doubleclicked(uint32_t) {
	assert(table_.has_selection());
	const NetOpenGame* const game = table_.get_selected();
	// Only join games that are open
	if (game->info.state == LAN_GAME_OPEN || !playername_.has_warning()) {
		clicked_joingame();
	}
}

void FullscreenMenuNetSetupLAN::update_game_info(
   UI::Table<NetOpenGame const* const>::EntryRecord& er, const NetGameInfo& info) {
	assert(info.hostname[sizeof(info.hostname) - 1] == '\0');
	er.set_string(0, info.hostname);
	assert(info.map[sizeof(info.map) - 1] == '\0');
	er.set_string(1, info.map);

	switch (info.state) {
	case LAN_GAME_OPEN:
		er.set_string(2, _("Open"));
		break;
	case LAN_GAME_CLOSED:
		er.set_string(2, _("Closed"));
		break;
	default:
		/** TRANSLATORS: The state of a LAN game can be open, closed or unknown */
		er.set_string(2, pgettext("game_state", "Unknown"));
		break;
	}
}

void FullscreenMenuNetSetupLAN::game_opened(const NetOpenGame* game) {
	update_game_info(table_.add(game), game->info);
}

void FullscreenMenuNetSetupLAN::game_closed(const NetOpenGame*) {
}

void FullscreenMenuNetSetupLAN::game_updated(const NetOpenGame* game) {
	if (UI::Table<const NetOpenGame* const>::EntryRecord* const er = table_.find(game)) {
		update_game_info(*er, game->info);
	}
}

void FullscreenMenuNetSetupLAN::discovery_callback(int32_t const type,
                                                   NetOpenGame const* const game,
                                                   void* const userdata) {
	switch (type) {
	case LanGameFinder::GameOpened:
		static_cast<FullscreenMenuNetSetupLAN*>(userdata)->game_opened(game);
		break;
	case LanGameFinder::GameClosed:
		static_cast<FullscreenMenuNetSetupLAN*>(userdata)->game_closed(game);
		break;
	case LanGameFinder::GameUpdated:
		static_cast<FullscreenMenuNetSetupLAN*>(userdata)->game_updated(game);
		break;
	default:
		abort();
	}
}

void FullscreenMenuNetSetupLAN::change_hostname() {
	// Allow user to enter a hostname manually
	table_.select(UI::Table<const NetOpenGame* const>::no_selection_index());
	joingame_.set_enabled(hostname_.text().size());
}

void FullscreenMenuNetSetupLAN::change_playername() {
	playername_.set_warning(false);
	playername_.set_tooltip("");
	hostgame_.set_enabled(true);

	if (!InternetGaming::ref().valid_username(playername_.text())) {
		playername_.set_warning(true);
		playername_.set_tooltip(_("Enter a valid nickname. This value may contain only "
		                          "English letters, numbers, and @ . + - _ characters."));
		joingame_.set_enabled(false);
		hostgame_.set_enabled(false);
		return;
	}
	if (!hostname_.text().empty()) {
		joingame_.set_enabled(true);
	}

	set_config_string("nickname", playername_.text());
}

void FullscreenMenuNetSetupLAN::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	set_config_string("lasthost", hostname_.text());

	end_modal<MenuTarget>(MenuTarget::kJoingame);
}

void FullscreenMenuNetSetupLAN::clicked_hostgame() {
	end_modal<MenuTarget>(MenuTarget::kHostgame);
}

void FullscreenMenuNetSetupLAN::clicked_lasthost() {
	Section& s = get_config_safe_section();
	std::string const host = s.get_string("lasthost", "");
	hostname_.set_text(host);
	if (host.size()) {
		joingame_.set_enabled(true);
	}
	table_.select(UI::Table<const NetOpenGame* const>::no_selection_index());
}
