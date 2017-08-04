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
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "network/constants.h"
#include "network/network.h"
#include "profile/profile.h"

FullscreenMenuNetSetupLAN::FullscreenMenuNetSetupLAN()
   : FullscreenMenuBase(),

     // Values for alignment and size
     butx_(get_w() * 13 / 40),
     butw_(get_w() / 4),
     buth_(get_h() * 19 / 400),
     listw_(get_w() * 9 / 16),

     // Text labels
     title(this, get_w() / 2, get_h() / 10, _("Begin Network Game"), UI::Align::kCenter),
     opengames_(
        this, get_w() * 3 / 50, get_h() * 27 / 100, _("List of games in your local network:")),
     playername_(this, get_w() * 16 / 25, get_h() * 27 / 100, _("Your nickname:")),
     hostname_(this, get_w() * 16 / 25, get_h() * 17 / 40, _("Host to connect:")),

     // Buttons
     joingame(this,
              "join_game",
              get_w() * 16 / 25,
              get_h() * 5333 / 10000,
              butw_,
              buth_,
              g_gr->images().get("images/ui_basic/but1.png"),
              _("Join this game")),
     hostgame(this,
              "host_game",
              get_w() * 16 / 25,
              get_h() * 6083 / 10000,
              butw_,
              buth_,
              g_gr->images().get("images/ui_basic/but1.png"),
              _("Host a new game")),
     back(this,
          "back",
          get_w() * 16 / 25,
          get_h() * 8333 / 10000,
          butw_,
          buth_,
          g_gr->images().get("images/ui_basic/but0.png"),
          _("Back")),
     loadlasthost(this,
                  "load_previous_host",
                  get_w() * 171 / 200,
                  get_h() * 19 / 40,
                  buth_,
                  buth_,
                  g_gr->images().get("images/ui_basic/but1.png"),
                  g_gr->images().get("images/ui_fsmenu/menu_load_game.png"),
                  _("Load previous host")),

     // Edit boxes
     playername(this,
                get_w() * 16 / 25,
                get_h() * 3333 / 10000,
                butw_,
                buth_,
                2,
                g_gr->images().get("images/ui_basic/but2.png"),
                fs_small()),
     hostname(this,
              get_w() * 16 / 25,
              get_h() * 19 / 40,
              get_w() * 17 / 80,
              buth_,
              2,
              g_gr->images().get("images/ui_basic/but2.png"),
              fs_small()),

     // List
     opengames(this, get_w() * 3 / 50, get_h() * 3333 / 10000, listw_, get_h() * 5433 / 10000) {
	joingame.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_joingame, boost::ref(*this)));
	hostgame.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_hostgame, boost::ref(*this)));
	back.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_back, boost::ref(*this)));
	loadlasthost.sigclicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::clicked_lasthost, boost::ref(*this)));

	Section& s = g_options.pull_section("global");  //  for playername

	title.set_fontsize(UI_FONT_SIZE_BIG);
	hostname.changed.connect(boost::bind(&FullscreenMenuNetSetupLAN::change_hostname, this));
	playername.set_text(s.get_string("nickname", (_("nobody"))));
	playername.changed.connect(boost::bind(&FullscreenMenuNetSetupLAN::change_playername, this));
	opengames.add_column(listw_ * 2 / 5, _("Host"));
	opengames.add_column(listw_ * 2 / 5, _("Map"));
	opengames.add_column(listw_ / 5, _("State"));
	opengames.selected.connect(boost::bind(&FullscreenMenuNetSetupLAN::game_selected, this, _1));
	opengames.double_clicked.connect(
	   boost::bind(&FullscreenMenuNetSetupLAN::game_doubleclicked, this, _1));
	discovery.set_callback(discovery_callback, this);

	joingame.set_enabled(false);
}

void FullscreenMenuNetSetupLAN::layout() {
	// TODO(GunChleoc): Box layout and then implement
	opengames.layout();
}

void FullscreenMenuNetSetupLAN::think() {
	FullscreenMenuBase::think();

	discovery.run();
}

bool FullscreenMenuNetSetupLAN::get_host_address(NetAddress* addr) {
	const std::string& host = hostname.text();

	const uint32_t opengames_size = opengames.size();
	for (uint32_t i = 0; i < opengames_size; ++i) {
		const NetOpenGame& game = *opengames[i];

		if (!strcmp(game.info.hostname, host.c_str())) {
			*addr = game.address;
			return true;
		}
	}

	// The user probably entered a hostname on his own. Try to resolve it
	if (NetAddress::resolve_to_v6(addr, host, WIDELANDS_PORT))
		return true;
	if (NetAddress::resolve_to_v4(addr, host, WIDELANDS_PORT))
		return true;
	return false;
}

const std::string& FullscreenMenuNetSetupLAN::get_playername() {
	return playername.text();
}

void FullscreenMenuNetSetupLAN::clicked_ok() {
	if (hostname.text().empty()) {
		clicked_hostgame();
	} else {
		clicked_joingame();
	}
}

void FullscreenMenuNetSetupLAN::game_selected(uint32_t) {
	if (opengames.has_selection()) {
		if (const NetOpenGame* const game = opengames.get_selected()) {
			hostname.set_text(game->info.hostname);
			joingame.set_enabled(true);
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
	update_game_info(opengames.add(game), game->info);
}

void FullscreenMenuNetSetupLAN::game_closed(const NetOpenGame*) {
}

void FullscreenMenuNetSetupLAN::game_updated(const NetOpenGame* game) {
	if (UI::Table<const NetOpenGame* const>::EntryRecord* const er = opengames.find(game))
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
	opengames.select(opengames.no_selection_index());
	joingame.set_enabled(hostname.text().size());
}

void FullscreenMenuNetSetupLAN::change_playername() {
	g_options.pull_section("global").set_string("nickname", playername.text());
}

void FullscreenMenuNetSetupLAN::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	g_options.pull_section("global").set_string("lasthost", hostname.text());

	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kJoingame);
}

void FullscreenMenuNetSetupLAN::clicked_hostgame() {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kHostgame);
}

void FullscreenMenuNetSetupLAN::clicked_lasthost() {
	Section& s = g_options.get_safe_section("global");
	std::string const host = s.get_string("lasthost", "");
	hostname.set_text(host);
	if (host.size())
		joingame.set_enabled(true);
	opengames.select(opengames.no_selection_index());
}
