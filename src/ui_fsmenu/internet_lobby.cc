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

#include "ui_fsmenu/internet_lobby.h"

#include "base/i18n.h"
#include "base/random.h"
#include "build_info.h"
#include "graphic/image_cache.h"
#include "graphic/text_layout.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "sound/sound_handler.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"

namespace {

// Constants for convert_clienttype() / compare_clienttype()
const uint8_t kClientSuperuser = 0;
const uint8_t kClientRegistered = 1;
const uint8_t kClientUnregistered = 2;
// 3 was INTERNET_CLIENT_BOT which is not used
const uint8_t kClientIRC = 4;
}  // namespace

FullscreenMenuInternetLobby::FullscreenMenuInternetLobby(FullscreenMenuMain& fsmm, std::string& nick,
                                                         std::string& pwd,
                                                         bool registered)
   : FullscreenMenuLoadMapOrGame(fsmm, _("Metaserver Lobby")),
     fsmm_(fsmm),
     // Boxes
     left_column_(this, 0, 0, UI::Box::Vertical),
     right_column_(this, 0, 0, UI::Box::Vertical),

     // Left column content
     label_clients_online_(&left_column_, 0, 0, 0, 0, _("Clients online:")),
     clientsonline_table_(&left_column_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),
     chat_(&left_column_, 0, 0, 0, 0, InternetGaming::ref(), UI::PanelStyle::kFsMenu),

     // Right column content
     label_opengames_(&right_column_, 0, 0, 0, 0, _("Open Games:")),
     opengames_list_(&right_column_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),
     joingame_(&right_column_,
               "join_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Join this game")),
     servername_label_(&right_column_, 0, 0, 0, 0, _("Name of your server:")),
     servername_(&right_column_, 0, 0, 0, UI::PanelStyle::kFsMenu),
     hostgame_(&right_column_,
               "host_game",
               0,
               0,
               0,
               0,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Open a new game")),
     prev_clientlist_len_(1000),
     new_client_fx_(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_freshmen")),
     // Login information
     nickname_(nick),
     password_(pwd),
     is_registered_(registered) {

	ok_.set_visible(false);  // We have 2 starting buttons, so we need a different layout here.
	back_.set_title(_("Leave Lobby"));

	left_column_.set_inner_spacing(padding_);
	left_column_.add(&label_clients_online_, UI::Box::Resizing::kFullSize);
	left_column_.add(&clientsonline_table_, UI::Box::Resizing::kExpandBoth);
	left_column_.add(&chat_, UI::Box::Resizing::kFullSize);

	right_column_.set_inner_spacing(padding_);
	right_column_.add(&label_opengames_, UI::Box::Resizing::kFullSize);
	right_column_.add(&opengames_list_, UI::Box::Resizing::kFullSize);
	right_column_.add_space(0);
	right_column_.add(&joingame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();
	right_column_.add(&servername_label_, UI::Box::Resizing::kFullSize);
	right_column_.add(&servername_, UI::Box::Resizing::kFullSize);
	right_column_.add_space(0);
	right_column_.add(&hostgame_, UI::Box::Resizing::kFullSize);
	right_column_.add_inf_space();

	joingame_.sigclicked.connect([this]() { clicked_joingame(); });
	hostgame_.sigclicked.connect([this]() { clicked_hostgame(); });
	back_.sigclicked.connect([this]() { clicked_back(); });

	// Set the texts and style of UI elements
	const std::string server = get_config_string("servername", "");
	servername_.set_text(server);
	servername_.changed.connect([this]() { change_servername(); });

	// Handle focus interaction between our 2 edit boxes
	servername_.clicked.connect([this]() {
		servername_.focus();
		chat_.unfocus_edit();
	});
	servername_.cancel.connect([this]() {
		servername_.set_can_focus(false);
		servername_.set_can_focus(true);
		chat_.focus_edit();
	});
	chat_.clicked.connect([this]() {
		servername_.set_can_focus(false);
		servername_.set_can_focus(true);
		chat_.focus_edit();
	});

	// Prepare the lists
	const std::string t_tip =
	   (boost::format("<rt padding=2><p align=center spacing=3>%s</p>"
	                  "<p valign=bottom><img src=images/wui/overlays/road_building_green.png> %s"
	                  "<br><img src=images/wui/overlays/road_building_yellow.png> %s"
	                  "<br><img src=images/wui/overlays/road_building_red.png> %s</p></rt>") %
	    g_style_manager->font_style(UI::FontStyle::kTooltipHeader).as_font_tag(_("User Status")) %
	    g_style_manager->font_style(UI::FontStyle::kTooltip).as_font_tag(_("Administrator")) %
	    g_style_manager->font_style(UI::FontStyle::kTooltip).as_font_tag(_("Registered")) %
	    g_style_manager->font_style(UI::FontStyle::kTooltip).as_font_tag(_("Unregistered")))
	      .str();
	clientsonline_table_.add_column(22, "*", t_tip);
	/** TRANSLATORS: Player Name */
	clientsonline_table_.add_column(180, pgettext("player", "Name"));
	/** TRANSLATORS: Widelands version */
	clientsonline_table_.add_column(120, _("Version"));
	clientsonline_table_.add_column(
	   0, _("Game"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	clientsonline_table_.set_column_compare(
	   0, [this](uint32_t a, uint32_t b) { return compare_clienttype(a, b); });
	clientsonline_table_.double_clicked.connect(
	   [this](uint32_t a) { return client_doubleclicked(a); });
	opengames_list_.selected.connect([this](uint32_t) { server_selected(); });
	opengames_list_.double_clicked.connect([this](uint32_t) { server_doubleclicked(); });

	// try to connect to the metaserver
	if (!InternetGaming::ref().error() && !InternetGaming::ref().logged_in()) {
		connect_to_metaserver();
	}

	layout();
	// set focus to chat input
	chat_.focus_edit();
}

void FullscreenMenuInternetLobby::layout() {
	FullscreenMenuLoadMapOrGame::layout();

	uint32_t butw = get_w() - right_column_x_ - right_column_margin_;
	uint32_t buth = text_height(UI::FontStyle::kLabel) + 8;

	tabley_ = tabley_ / 2;
	tableh_ += tabley_;

	left_column_.set_size(tablew_, tableh_);
	left_column_.set_pos(Vector2i(tablex_, tabley_));

	right_column_.set_size(get_right_column_w(right_column_x_), tableh_ - buth - 4 * padding_);
	right_column_.set_pos(Vector2i(right_column_x_, tabley_));

	// Chat
	chat_.set_desired_size(tablew_, tableh_ * 7 / 12);

	// Identical list height
	opengames_list_.set_desired_size(opengames_list_.get_w(), clientsonline_table_.get_h());

	// Buttons
	joingame_.set_size(butw, buth);
	hostgame_.set_size(butw, buth);
	back_.set_size(butw, buth);
}

/// think function of the UI (main loop)
void FullscreenMenuInternetLobby::think() {
	FullscreenMenuLoadMapOrGame::think();

	if (!InternetGaming::ref().error()) {

		// If we have no connection try to connect
		if (!InternetGaming::ref().logged_in()) {
			connect_to_metaserver();
		}

		// Check whether metaserver send some data
		InternetGaming::ref().handle_metaserver_communication();
	}

	if (InternetGaming::ref().update_for_clients()) {
		fill_client_list(InternetGaming::ref().clients());
	}

	if (InternetGaming::ref().update_for_games()) {
		fill_games_list(InternetGaming::ref().games());
	}
	// unfocus chat window when other UI element has focus
	if (!chat_.has_focus()) {
		chat_.unfocus_edit();
	}
	if (servername_.has_focus()) {
		change_servername();
	}
}

void FullscreenMenuInternetLobby::clicked_ok() {
	if (joingame_.enabled()) {
		server_doubleclicked();
	} else {
		clicked_hostgame();
	}
}

/// connects Widelands with the metaserver
void FullscreenMenuInternetLobby::connect_to_metaserver() {
	const std::string& metaserver = get_config_string("metaserver", INTERNET_GAMING_METASERVER);
	uint32_t port = get_config_natural("metaserverport", kInternetGamingPort);
	std::string auth = is_registered_ ? password_ : get_config_string("uuid", "");
	assert(!auth.empty());
	InternetGaming::ref().login(nickname_, auth, is_registered_, metaserver, port);
}

/// fills the server list
void FullscreenMenuInternetLobby::fill_games_list(const std::vector<InternetGame>* games) {
	// List and button cleanup
	opengames_list_.clear();
	hostgame_.set_enabled(true);
	joingame_.set_enabled(false);

	const std::string& localbuildid = build_id();

	if (games != nullptr) {  // If no communication error occurred, fill the list.
		for (const InternetGame& game : *games) {
			if (game.connectable == INTERNET_GAME_SETUP && game.build_id == localbuildid) {
				// only clients with the same build number are displayed
				opengames_list_.add(richtext_escape(game.name), game,
				                    g_image_cache->get("images/ui_basic/continue.png"), false,
				                    game.build_id);
			} else if (game.connectable == INTERNET_GAME_SETUP &&
			           game.build_id.compare(0, 6, "build-") != 0 &&
			           localbuildid.compare(0, 6, "build-") != 0) {
				// only development clients are allowed to see games openend by such
				opengames_list_.add(richtext_escape(game.name), game,
				                    g_image_cache->get("images/ui_basic/different.png"), false,
				                    game.build_id);
			}
		}
	}
}

uint8_t FullscreenMenuInternetLobby::convert_clienttype(const std::string& type) {
	if (type == INTERNET_CLIENT_REGISTERED) {
		return kClientRegistered;
	}
	if (type == INTERNET_CLIENT_SUPERUSER) {
		return kClientSuperuser;
	}
	if (type == INTERNET_CLIENT_IRC) {
		return kClientIRC;
	}
	// if (type == INTERNET_CLIENT_UNREGISTERED)
	return kClientUnregistered;
}

/**
 * \return \c true if the client in row \p rowa should come before the client in
 * row \p rowb when sorted according to clienttype
 */
bool FullscreenMenuInternetLobby::compare_clienttype(unsigned int rowa, unsigned int rowb) {
	const InternetClient* playera = clientsonline_table_[rowa];
	const InternetClient* playerb = clientsonline_table_[rowb];

	return convert_clienttype(playera->type) < convert_clienttype(playerb->type);
}

/// fills the client list
void FullscreenMenuInternetLobby::fill_client_list(const std::vector<InternetClient>* clients) {
	clientsonline_table_.clear();
	if (clients != nullptr) {  // If no communication error occurred, fill the list.
		for (const InternetClient& client : *clients) {
			UI::Table<const InternetClient* const>::EntryRecord& er =
			   clientsonline_table_.add(&client);
			er.set_string(1, client.name);
			er.set_string(2, client.build_id);
			er.set_string(3, client.game);

			const Image* pic;
			switch (convert_clienttype(client.type)) {
			case kClientUnregistered:
				pic = g_image_cache->get("images/wui/overlays/road_building_red.png");
				er.set_picture(0, pic);
				break;
			case kClientRegistered:
				pic = g_image_cache->get("images/wui/overlays/road_building_yellow.png");
				er.set_picture(0, pic);
				break;
			case kClientSuperuser:
				pic = g_image_cache->get("images/wui/overlays/road_building_green.png");
				er.set_font_style(g_style_manager->font_style(UI::FontStyle::kFsGameSetupSuperuser));
				er.set_picture(0, pic);
				break;
			case kClientIRC:
				// No icon for IRC users
				er.set_font_style(g_style_manager->font_style(UI::FontStyle::kFsGameSetupIrcClient));
				continue;
			default:
				continue;
			}
		}
		// If a new player joins the lobby, play a sound.
		if (clients->size() > prev_clientlist_len_ && !InternetGaming::ref().sound_off()) {
			g_sh->play_fx(SoundType::kChat, new_client_fx_);
		}
		prev_clientlist_len_ = clients->size();
	}
	clientsonline_table_.sort();
}

/// called when an entry of the client list was doubleclicked
void FullscreenMenuInternetLobby::client_doubleclicked(uint32_t i) {
	// add a @clientname to the current edit text.
	if (clientsonline_table_.has_selection()) {
		UI::Table<const InternetClient* const>::EntryRecord& er = clientsonline_table_.get_record(i);

		std::string temp("@");
		temp += er.get_string(1);
		std::string text(chat_.get_edit_text());

		if (text.size() && (text.at(0) == '@')) {  // already PM ?
			if (text.find(' ') <= text.size()) {
				text = text.substr(text.find(' '), text.size());
			} else {
				text.clear();
			}
		} else {
			temp += " ";  // The needed space between name and text
		}

		temp += text;
		chat_.set_edit_text(temp);
		chat_.focus_edit();
	}
}

/// called when an entry of the server list was selected
void FullscreenMenuInternetLobby::server_selected() {
	// remove focus from chat
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable == INTERNET_GAME_SETUP) {
			joingame_.set_enabled(true);
		}
	}
}

/// called when an entry of the server list was doubleclicked
void FullscreenMenuInternetLobby::server_doubleclicked() {
	// if the game is open try to connect it, if not do nothing.
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable == INTERNET_GAME_SETUP) {
			clicked_joingame();
		}
	}
}

/// called when the servername was changed
void FullscreenMenuInternetLobby::change_servername() {
	// Allow client to enter a servername manually
	hostgame_.set_enabled(true);
	servername_.set_tooltip("");
	servername_.set_warning(false);
	// Check whether a server of that name is already open.
	// And disable 'hostgame' button if yes.
	const std::vector<InternetGame>* games = InternetGaming::ref().games();
	if (games != nullptr) {
		for (const InternetGame& game : *games) {
			if (game.name == servername_.text()) {
				hostgame_.set_enabled(false);
				servername_.set_warning(true);
				servername_.set_tooltip(
				   (boost::format(
				       _("The game %s is already running. Please choose a different name.")) %
				    g_style_manager->font_style(UI::FontStyle::kWarning).as_font_tag(game.name))
				      .str());
			}
		}
	}
}

bool FullscreenMenuInternetLobby::wait_for_ip() {
	if (!InternetGaming::ref().wait_for_ips()) {
		// Only display a message box if a network error occurred
		if (InternetGaming::ref().error()) {
			// Show a popup warning message
			const std::string warning(
			   _("Widelands was unable to get the IP address of the server in time. "
			     "There seems to be a network problem, either on your side or on the side "
			     "of the server.\n"));
			UI::WLMessageBox mmb(this, UI::WindowStyle::kFsMenu, _("Connection Timed Out"), warning,
			                     UI::WLMessageBox::MBoxType::kOk, UI::Align::kLeft);
			mmb.run<UI::Panel::Returncodes>();
		}
		return false;
	}
	return true;
}

/// called when the 'join game' button was clicked
void FullscreenMenuInternetLobby::clicked_joingame() {
	if (opengames_list_.has_selection()) {
		InternetGaming::ref().join_game(opengames_list_.get_selected().name);

		if (!wait_for_ip()) {
			return;
		}
		const std::pair<NetAddress, NetAddress>& ips = InternetGaming::ref().ips();

		GameClient netgame(fsmm_, ips, InternetGaming::ref().get_local_clientname(), true,
		                   opengames_list_.get_selected().name);
		netgame.run();
	} else {
		throw wexception("No server selected! That should not happen!");
	}
}

/// called when the 'host game' button was clicked
void FullscreenMenuInternetLobby::clicked_hostgame() {
	// Save selected servername as default for next time and during that take care that the name is
	// not empty.
	std::string servername_ui = servername_.text();

	const std::vector<InternetGame>* games = InternetGaming::ref().games();
	if (games != nullptr) {
		for (const InternetGame& game : *games) {
			if (servername_ui.empty()) {
				uint32_t i = 1;
				do {
					/** TRANSLATORS: This is shown for multiplayer games when no host */
					/** TRANSLATORS: server to connect to has been specified yet. */
					servername_ui = (boost::format(_("unnamed %u")) % i++).str();
				} while (servername_ui == game.name);
			} else if (game.name == servername_ui) {
				change_servername();
				return;
			}
		}
		if (games->empty() && servername_ui.empty()) {
			servername_ui = _("unnamed");
		}
	}

	set_config_string("servername", servername_ui);

	// Set up the game
	InternetGaming::ref().set_local_servername(servername_ui);

	// Start the game
	try {

		// Tell the metaserver about it
		InternetGaming::ref().open_game();

		// Wait for the response with the IPs of the relay server
		if (!wait_for_ip()) {
			InternetGaming::ref().set_error();
			return;
		}

		// Start our relay host
		GameHost netgame(fsmm_, InternetGaming::ref().get_local_clientname(), true);
		netgame.run();
	} catch (...) {
		// Log out before going back to the main menu
		InternetGaming::ref().logout("SERVER_CRASHED");
		throw;
	}
}
