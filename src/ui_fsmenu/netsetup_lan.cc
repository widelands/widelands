/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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
#include "base/macros.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "graphic/text_layout.h"
#include "network/constants.h"
#include "network/network.h"
#include "profile/profile.h"

FullscreenMenuNetSetupLAN::FullscreenMenuNetSetupLAN()
   : FullscreenMenuLoadMapOrGame(),
     labelh_(text_height(UI::g_fh1->fontset()->representative_character(), UI_FONT_SIZE_SMALL) + 8),

     // Main title
     title_(this, 0, 0, _("Begin Network Game"), UI::Align::kCenter),

     // Boxes
     left_column_(this, 0, 0, UI::Box::Vertical),
     right_column_(this, 0, 0, UI::Box::Vertical),

     // Left column content
     label_opengames_(&left_column_, 0, 0, _("List of games in your local network:")),
     table_(&left_column_, 0, 0, 0, 0),

     // Right column content
     label_playername_(&right_column_, 0, 0, _("Your nickname:")),
     playername_(
        &right_column_, 0, 0, 0, labelh_, 2, g_gr->images().get("images/ui_basic/but2.png")),
     label_hostname_(&right_column_, 0, 0, _("Host to connect:")),

     host_box_(&right_column_, 0, 0, UI::Box::Horizontal),
     hostname_(&host_box_, 0, 0, 0, labelh_, 2, g_gr->images().get("images/ui_basic/but2.png")),
     loadlasthost_(&host_box_,
                   "load_previous_host",
                   0,
                   0,
                   labelh_,
                   labelh_,
                   g_gr->images().get("images/ui_basic/but1.png"),
                   g_gr->images().get("images/ui_fsmenu/menu_load_game.png"),
                   _("Load previous host")),

     // Buttons
     joingame_(&right_column_,
               "join_game",
               0,
               0,
               0,
               0,
               g_gr->images().get("images/ui_basic/but1.png"),
               _("Join this game")),
     hostgame_(&right_column_,
               "host_game",
               0,
               0,
               0,
               0,
               g_gr->images().get("images/ui_basic/but1.png"),
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
	right_column_.add_space(0);
	right_column_.add(&joingame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add(&hostgame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add_inf_space();
	right_column_.add_inf_space();
	right_column_.add_inf_space();

	host_box_.add(&hostname_, UI::Box::Resizing::kFillSpace);
	host_box_.add_space(padding_);
	host_box_.add(&loadlasthost_);

	joingame_.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_joingame, boost::ref(*this)));
	hostgame_.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_hostgame, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_back, boost::ref(*this)));
	loadlasthost_.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_lasthost, boost::ref(*this)));

	Section& s = g_options.pull_section("global");  //  for playername

	title_.set_fontsize(UI_FONT_SIZE_BIG);
	hostname_.changed.connect(boost::bind(&FullscreenMenuNetSetupLAN::change_hostname, this));
	playername_.set_text(s.get_string("nickname", (_("nobody"))));
	playername_.changed.connect(boost::bind(&FullscreenMenuNetSetupLAN::change_playername, this));
	table_.add_column(190, _("Host"));
	table_.add_column(0, _("Map"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.add_column(90, _("State"));
	table_.selected.connect(boost::bind(&FullscreenMenuNetSetupLAN::game_selected, this, _1));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::game_doubleclicked, this, _1));
	discovery_.set_callback(discovery_callback, this);

	joingame_.set_enabled(false);
	layout();
}

void FullscreenMenuNetSetupLAN::layout() {
	FullscreenMenuLoadMapOrGame::layout();

	butw_ = get_w() - right_column_x_ - right_column_margin_;
	const int colum_header_h = label_opengames_.get_h() + padding_;

	title_.set_size(get_w(), title_.get_h());
	title_.set_pos(Vector2i(0, (tabley_ - colum_header_h) / 3));

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
	FullscreenMenuBase::think();

	discovery_.run();
}

bool FullscreenMenuNetSetupLAN::get_host_address(uint32_t& addr, uint16_t& port) {
	const std::string& host = hostname_.text();

	const uint32_t opengames_size = table_.size();
	for (uint32_t i = 0; i < opengames_size; ++i) {
		const NetOpenGame& game = *table_[i];

		if (!strcmp(game.info.hostname, host.c_str())) {
			addr = game.address;
			port = game.port;
			return true;
		}
	}

	if (hostent* const he = gethostbyname(host.c_str())) {
		addr = (reinterpret_cast<in_addr*>(he->h_addr_list[0]))->s_addr;
		DIAG_OFF("-Wold-style-cast")
		port = htons(WIDELANDS_PORT);
		DIAG_ON("-Wold-style-cast")
		return true;
	} else
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
	clicked_joingame();
}

void FullscreenMenuNetSetupLAN::update_game_info(
   UI::Table<NetOpenGame const* const>::EntryRecord& er, const NetGameInfo& info) {
	er.set_string(0, info.hostname);
	er.set_string(1, info.map);

	switch (info.state) {
	case LAN_GAME_OPEN:
		er.set_string(2, _("Open"));
		break;
	case LAN_GAME_CLOSED:
		er.set_string(2, _("Closed"));
		break;
	/** TRANSLATORS: The state of a LAN game can be open, closed or unknown */
	default:
		er.set_string(2, pgettext("game_state", "Unknown"));
		break;
	};
}

void FullscreenMenuNetSetupLAN::game_opened(const NetOpenGame* game) {
	update_game_info(table_.add(game), game->info);
}

void FullscreenMenuNetSetupLAN::game_closed(const NetOpenGame*) {
}

void FullscreenMenuNetSetupLAN::game_updated(const NetOpenGame* game) {
	if (UI::Table<const NetOpenGame* const>::EntryRecord* const er = table_.find(game))
		update_game_info(*er, game->info);
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
	table_.select(table_.no_selection_index());
	joingame_.set_enabled(hostname_.text().size());
}

void FullscreenMenuNetSetupLAN::change_playername() {
	g_options.pull_section("global").set_string("nickname", playername_.text());
}

void FullscreenMenuNetSetupLAN::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	g_options.pull_section("global").set_string("lasthost", hostname_.text());

	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kJoingame);
}

void FullscreenMenuNetSetupLAN::clicked_hostgame() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kHostgame);
}

void FullscreenMenuNetSetupLAN::clicked_lasthost() {
	Section& s = g_options.get_safe_section("global");
	std::string const host = s.get_string("lasthost", "");
	hostname_.set_text(host);
	if (host.size())
		joingame_.set_enabled(true);
	table_.select(table_.no_selection_index());
}
