/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#include "ui_fsmenu/netsetup_lan.h"

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "network/constants.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/internet_gaming.h"
#include "network/network.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/main.h"
#include "wlapplication_options.h"

namespace FsMenu {

NetSetupLAN::NetSetupLAN(MenuCapsule& fsmm)
   : TwoColumnsBasicNavigationMenu(fsmm, _("Begin LAN Game")),

     // Left column content
     label_opengames_(&left_column_box_,
                      UI::PanelStyle::kFsMenu,
                      UI::FontStyle::kFsMenuLabel,
                      0,
                      0,
                      0,
                      0,
                      _("List of games in your local network:")),
     table_(&left_column_box_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),

     // Right column content
     label_playername_(&right_column_content_box_,
                       UI::PanelStyle::kFsMenu,
                       UI::FontStyle::kFsMenuLabel,
                       0,
                       0,
                       0,
                       0,
                       _("Your nickname:")),
     playername_(&right_column_content_box_, 0, 0, 0, UI::PanelStyle::kFsMenu),
     label_hostname_(&right_column_content_box_,
                     UI::PanelStyle::kFsMenu,
                     UI::FontStyle::kFsMenuLabel,
                     0,
                     0,
                     0,
                     0,
                     _("Host to connect:")),

     host_box_(&right_column_content_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
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
     joingame_(&right_column_content_box_,
               "join_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Join this game")),
     hostgame_(&right_column_content_box_,
               "host_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Host a new game")) {

	left_column_box_.add(&label_opengames_, UI::Box::Resizing::kFullSize);
	left_column_box_.add_space(kPadding);
	left_column_box_.add(&table_, UI::Box::Resizing::kExpandBoth);

	right_column_content_box_.set_inner_spacing(kPadding);
	right_column_content_box_.add(&label_playername_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add(&playername_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add(&label_hostname_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add(&host_box_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_space(kPadding);
	right_column_content_box_.add(&joingame_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add(&hostgame_, UI::Box::Resizing::kFullSize);
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add_inf_space();
	right_column_content_box_.add_inf_space();

	host_box_.add(&hostname_, UI::Box::Resizing::kExpandBoth);
	host_box_.add_space(kPadding);
	host_box_.add(&loadlasthost_);

	joingame_.sigclicked.connect([this]() { clicked_joingame(); });
	hostgame_.sigclicked.connect([this]() { clicked_hostgame(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
	loadlasthost_.sigclicked.connect([this]() { clicked_lasthost(); });

	hostname_.changed.connect([this]() { change_hostname(); });
	/** TRANSLATORS: This string may contain only English letters, numbers, and @ . + - _ characters
	 * and must not be "team". */
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

void NetSetupLAN::layout() {
	TwoColumnsBasicNavigationMenu::layout();

	joingame_.set_desired_size(0, standard_height_);
	playername_.set_desired_size(0, standard_height_);
	hostgame_.set_desired_size(0, standard_height_);
	hostname_.set_desired_size(0, standard_height_);
	loadlasthost_.set_desired_size(standard_height_, standard_height_);
}

void NetSetupLAN::think() {
	TwoColumnsBasicNavigationMenu::think();
	change_playername();

	discovery_.run();
}

bool NetSetupLAN::get_host_address(NetAddress* addr) {
	const std::string& host = hostname_.text();

	for (uint32_t i = 0; i < table_.size(); ++i) {
		const NetOpenGame& game = *table_[i];

		if (strcmp(game.info.hostname, host.c_str()) == 0) {
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

void NetSetupLAN::clicked_ok() {
	if (hostname_.text().empty()) {
		clicked_hostgame();
	} else {
		clicked_joingame();
	}
}

void NetSetupLAN::game_selected(uint32_t /* index */) {
	if (table_.has_selection()) {
		if (const NetOpenGame* const game = table_.get_selected()) {
			hostname_.set_text(game->info.hostname);
			joingame_.set_enabled(true);
		}
	}
}

void NetSetupLAN::game_doubleclicked(uint32_t /* index */) {
	assert(table_.has_selection());
	const NetOpenGame* const game = table_.get_selected();
	// Only join games that are open
	if (game->info.state == LAN_GAME_OPEN || !playername_.has_warning()) {
		clicked_joingame();
	}
}

void NetSetupLAN::update_game_info(UI::Table<NetOpenGame const* const>::EntryRecord& er,
                                   const NetGameInfo& info) {
	assert(info.hostname[sizeof(info.hostname) - 1] == '\0');
	er.set_string(0, info.hostname);
	assert(info.map[sizeof(info.map) - 1] == '\0');
	{
		// If the map is in an add-on, we have no way of knowing so. We are therefore
		// unable to set the correct textdomain for add-on maps, even if we have
		// installed the add-on ourself. Let's just hope it's an official map.
		i18n::Textdomain td("maps");
		er.set_string(1, info.map[0] != 0 ? i18n::translate(info.map) : "");
	}

	er.set_disabled(info.state != LAN_GAME_OPEN);
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

void NetSetupLAN::game_opened(const NetOpenGame* game) {
	update_game_info(table_.add(game), game->info);
}

void NetSetupLAN::game_closed(const NetOpenGame* /* game */) {
}

void NetSetupLAN::game_updated(const NetOpenGame* game) {
	if (UI::Table<const NetOpenGame* const>::EntryRecord* const er = table_.find(game)) {
		update_game_info(*er, game->info);
	}
}

void NetSetupLAN::discovery_callback(int32_t const type,
                                     NetOpenGame const* const game,
                                     void* const userdata) {
	switch (type) {
	case LanGameFinder::GameOpened:
		static_cast<NetSetupLAN*>(userdata)->game_opened(game);
		break;
	case LanGameFinder::GameClosed:
		static_cast<NetSetupLAN*>(userdata)->game_closed(game);
		break;
	case LanGameFinder::GameUpdated:
		static_cast<NetSetupLAN*>(userdata)->game_updated(game);
		break;
	default:
		abort();
	}
}

void NetSetupLAN::change_hostname() {
	// Allow user to enter a hostname manually
	table_.select(UI::Table<const NetOpenGame* const>::no_selection_index());
	joingame_.set_enabled(!hostname_.text().empty());
}

void NetSetupLAN::change_playername() {
	playername_.set_warning(false);
	playername_.set_tooltip("");
	hostgame_.set_enabled(true);

	if (!InternetGaming::ref().valid_username(playername_.text())) {
		playername_.set_warning(true);
		playername_.set_tooltip(_("Enter a valid nickname. This value may contain only "
		                          "English letters, numbers, and @ . + - _ characters "
		                          "and must not be \"team\"."));
		joingame_.set_enabled(false);
		hostgame_.set_enabled(false);
		return;
	}
	if (!hostname_.text().empty()) {
		joingame_.set_enabled(true);
	}

	set_config_string("nickname", playername_.text());
}

void NetSetupLAN::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	set_config_string("lasthost", hostname_.text());

	NetAddress addr;
	if (!get_host_address(&addr)) {
		UI::WLMessageBox mmb(
		   &capsule_.menu(), UI::WindowStyle::kFsMenu, _("Invalid Address"),
		   _("The entered hostname or address is invalid and can’t be connected to."),
		   UI::WLMessageBox::MBoxType::kOk);
		mmb.run<UI::Panel::Returncodes>();
		return;
	}

	try {
		running_game_.reset(new GameClient(
		   capsule_, running_game_, std::make_pair(addr, NetAddress()), playername_.text()));
	} catch (const std::exception& e) {
		running_game_.reset();
		UI::WLMessageBox mbox(&capsule_.menu(), UI::WindowStyle::kFsMenu, _("Network Error"),
		                      e.what(), UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
		return;
	}
}

void NetSetupLAN::clicked_hostgame() {
	std::vector<Widelands::TribeBasicInfo> tribeinfos = Widelands::get_all_tribeinfos(nullptr);
	if (tribeinfos.empty()) {
		UI::WLMessageBox mbox(
		   &capsule_.menu(), UI::WindowStyle::kFsMenu, _("No tribes found!"),
		   _("No tribes found in data/tribes/initialization/[tribename]/init.lua."),
		   UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
		return;
	}

	try {
		running_game_.reset(new GameHost(&capsule_, running_game_, playername_.text(), tribeinfos));
	} catch (const std::exception& e) {
		running_game_.reset();
		UI::WLMessageBox mbox(&capsule_.menu(), UI::WindowStyle::kFsMenu, _("Network Error"),
		                      e.what(), UI::WLMessageBox::MBoxType::kOk);
		mbox.run<UI::Panel::Returncodes>();
		return;
	}
}

void NetSetupLAN::clicked_lasthost() {
	Section& s = get_config_safe_section();
	std::string const host = s.get_string("lasthost", "");
	hostname_.set_text(host);
	if (!host.empty()) {
		joingame_.set_enabled(true);
	}
	table_.select(UI::Table<const NetOpenGame* const>::no_selection_index());
}
}  // namespace FsMenu
