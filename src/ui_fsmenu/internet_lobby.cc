/*
 * Copyright (C) 2004-2019 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "graphic/graphic.h"
#include "network/crypto.h"
#include "network/gameclient.h"
#include "network/gamehost.h"
#include "network/internet_gaming.h"
#include "random/random.h"
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

FullscreenMenuInternetLobby::FullscreenMenuInternetLobby(char const* const nick,
                                                         char const* const pwd,
                                                         bool registered)
   : FullscreenMenuBase(),

     // Values for alignment and size
     butx_(get_w() * 13 / 40),
     butw_(get_w() * 36 / 125),
     buth_(get_h() * 19 / 400),
     lisw_(get_w() * 623 / 1000),
     prev_clientlist_len_(1000),
     new_client_fx_(SoundHandler::register_fx(SoundType::kChat, "sound/lobby_freshmen")),

     // Text labels
     title(this,
           get_w() / 2,
           get_h() / 20,
           0,
           0,
           _("Metaserver Lobby"),
           UI::Align::kCenter,
           g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
     clients_(this, get_w() * 4 / 125, get_h() * 15 / 100, 0, 0, _("Clients online:")),
     opengames_(this, get_w() * 17 / 25, get_h() * 15 / 100, 0, 0, _("Open Games:")),
     servername_(this, get_w() * 17 / 25, get_h() * 63 / 100, 0, 0, _("Name of your server:")),

     // Buttons
     joingame_(this,
               "join_game",
               get_w() * 17 / 25,
               get_h() * 55 / 100,
               butw_,
               buth_,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Join this game")),
     hostgame_(this,
               "host_game",
               get_w() * 17 / 25,
               get_h() * 73 / 100,
               butw_,
               buth_,
               UI::ButtonStyle::kFsMenuSecondary,
               _("Open a new game")),
     back_(this,
           "back",
           get_w() * 17 / 25,
           get_h() * 90 / 100,
           butw_,
           buth_,
           UI::ButtonStyle::kFsMenuSecondary,
           _("Leave Lobby")),

     // Edit boxes
     edit_servername_(this, get_w() * 17 / 25, get_h() * 68 / 100, butw_, UI::PanelStyle::kFsMenu),

     // List
     clientsonline_list_(
        this, get_w() * 4 / 125, get_h() / 5, lisw_, get_h() * 3 / 10, UI::PanelStyle::kFsMenu),
     opengames_list_(
        this, get_w() * 17 / 25, get_h() / 5, butw_, get_h() * 7 / 20, UI::PanelStyle::kFsMenu),

     // The chat UI
     chat(this,
          get_w() * 4 / 125,
          get_h() * 51 / 100,
          lisw_,
          get_h() * 44 / 100,
          InternetGaming::ref(),
          UI::PanelStyle::kFsMenu),

     // Login information
     nickname_(nick),
     password_(pwd),
     is_registered_(registered) {

	joingame_.sigclicked.connect(
	   boost::bind(&FullscreenMenuInternetLobby::clicked_joingame, boost::ref(*this)));
	hostgame_.sigclicked.connect(
	   boost::bind(&FullscreenMenuInternetLobby::clicked_hostgame, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuInternetLobby::clicked_back, boost::ref(*this)));

	// Set the texts and style of UI elements
	title.set_font_scale(scale_factor());

	opengames_.set_font_scale(scale_factor());
	clients_.set_font_scale(scale_factor());
	servername_.set_font_scale(scale_factor());

	std::string server = get_config_string("servername", "");
	edit_servername_.set_font_scale(scale_factor());
	edit_servername_.set_text(server);
	edit_servername_.changed.connect(
	   boost::bind(&FullscreenMenuInternetLobby::change_servername, this));

	// Prepare the lists
	const std::string t_tip =
	   (boost::format("<rt padding=2><p align=center spacing=3>%s</p>"
	                  "<p valign=bottom><img src=images/wui/overlays/roadb_green.png> %s"
	                  "<br><img src=images/wui/overlays/roadb_yellow.png> %s"
	                  "<br><img src=images/wui/overlays/roadb_red.png> %s</p></rt>") %
	    g_gr->styles().font_style(UI::FontStyle::kTooltipHeader).as_font_tag(_("User Status")) %
	    g_gr->styles().font_style(UI::FontStyle::kTooltip).as_font_tag(_("Administrator")) %
	    g_gr->styles().font_style(UI::FontStyle::kTooltip).as_font_tag(_("Registered")) %
	    g_gr->styles().font_style(UI::FontStyle::kTooltip).as_font_tag(_("Unregistered")))
	      .str();
	clientsonline_list_.add_column(22, "*", t_tip);
	/** TRANSLATORS: Player Name */
	clientsonline_list_.add_column((lisw_ - 22) * 3 / 8, pgettext("player", "Name"));
	clientsonline_list_.add_column((lisw_ - 22) * 2 / 8, _("Version"));
	clientsonline_list_.add_column(
	   (lisw_ - 22) * 3 / 8, _("Game"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
	clientsonline_list_.set_column_compare(
	   0, boost::bind(&FullscreenMenuInternetLobby::compare_clienttype, this, _1, _2));
	clientsonline_list_.double_clicked.connect(
	   boost::bind(&FullscreenMenuInternetLobby::client_doubleclicked, this, _1));
	opengames_list_.selected.connect(
	   boost::bind(&FullscreenMenuInternetLobby::server_selected, this));
	opengames_list_.double_clicked.connect(
	   boost::bind(&FullscreenMenuInternetLobby::server_doubleclicked, this));

	// try to connect to the metaserver
	if (!InternetGaming::ref().error() && !InternetGaming::ref().logged_in())
		connect_to_metaserver();

	// set focus to chat input
	chat.focus_edit();
}

void FullscreenMenuInternetLobby::layout() {
	// TODO(GunChleoc): Box layout and then implement
	clientsonline_list_.layout();
}

/// think function of the UI (main loop)
void FullscreenMenuInternetLobby::think() {
	FullscreenMenuBase::think();

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
	if (!chat.has_focus()) {
		chat.unfocus_edit();
	}
	if (edit_servername_.has_focus()) {
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
	const std::string& metaserver =
	   get_config_string("metaserver", INTERNET_GAMING_METASERVER.c_str());
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
	std::string localservername = edit_servername_.text();
	std::string localbuildid = build_id();

	if (games != nullptr) {  // If no communication error occurred, fill the list.
		for (const InternetGame& game : *games) {
			if (game.connectable == INTERNET_GAME_SETUP && game.build_id == localbuildid) {
				// only clients with the same build number are displayed
				opengames_list_.add(richtext_escape(game.name), game,
				                    g_gr->images().get("images/ui_basic/continue.png"), false,
				                    game.build_id);
			} else if (game.connectable == INTERNET_GAME_SETUP &&
			           game.build_id.compare(0, 6, "build-") != 0 &&
			           localbuildid.compare(0, 6, "build-") != 0) {
				// only development clients are allowed to see games openend by such
				opengames_list_.add(richtext_escape(game.name), game,
				                    g_gr->images().get("images/ui_basic/different.png"), false,
				                    game.build_id);
			}
		}
	}
}

uint8_t FullscreenMenuInternetLobby::convert_clienttype(const std::string& type) {
	if (type == INTERNET_CLIENT_REGISTERED)
		return kClientRegistered;
	if (type == INTERNET_CLIENT_SUPERUSER)
		return kClientSuperuser;
	if (type == INTERNET_CLIENT_IRC)
		return kClientIRC;
	// if (type == INTERNET_CLIENT_UNREGISTERED)
	return kClientUnregistered;
}

/**
 * \return \c true if the client in row \p rowa should come before the client in
 * row \p rowb when sorted according to clienttype
 */
bool FullscreenMenuInternetLobby::compare_clienttype(unsigned int rowa, unsigned int rowb) {
	const InternetClient* playera = clientsonline_list_[rowa];
	const InternetClient* playerb = clientsonline_list_[rowb];

	return convert_clienttype(playera->type) < convert_clienttype(playerb->type);
}

/// fills the client list
void FullscreenMenuInternetLobby::fill_client_list(const std::vector<InternetClient>* clients) {
	clientsonline_list_.clear();
	if (clients != nullptr) {  // If no communication error occurred, fill the list.
		for (const InternetClient& client : *clients) {
			UI::Table<const InternetClient* const>::EntryRecord& er = clientsonline_list_.add(&client);
			er.set_string(1, client.name);
			er.set_string(2, client.build_id);
			er.set_string(3, client.game);

			const Image* pic;
			switch (convert_clienttype(client.type)) {
			case kClientUnregistered:
				pic = g_gr->images().get("images/wui/overlays/roadb_red.png");
				er.set_picture(0, pic);
				break;
			case kClientRegistered:
				pic = g_gr->images().get("images/wui/overlays/roadb_yellow.png");
				er.set_picture(0, pic);
				break;
			case kClientSuperuser:
				pic = g_gr->images().get("images/wui/overlays/roadb_green.png");
				er.set_font_style(g_gr->styles().font_style(UI::FontStyle::kFsGameSetupSuperuser));
				er.set_picture(0, pic);
				break;
			case kClientIRC:
				// No icon for IRC users
				er.set_font_style(g_gr->styles().font_style(UI::FontStyle::kFsGameSetupIrcClient));
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
	clientsonline_list_.sort();
}

/// called when an entry of the client list was doubleclicked
void FullscreenMenuInternetLobby::client_doubleclicked(uint32_t i) {
	// add a @clientname to the current edit text.
	if (clientsonline_list_.has_selection()) {
		UI::Table<const InternetClient* const>::EntryRecord& er = clientsonline_list_.get_record(i);

		std::string temp("@");
		temp += er.get_string(1);
		std::string text(chat.get_edit_text());

		if (text.size() && (text.at(0) == '@')) {  // already PM ?
			if (text.find(' ') <= text.size())
				text = text.substr(text.find(' '), text.size());
			else
				text.clear();
		} else
			temp += " ";  // The needed space between name and text

		temp += text;
		chat.set_edit_text(temp);
		chat.focus_edit();
	}
}

/// called when an entry of the server list was selected
void FullscreenMenuInternetLobby::server_selected() {
	// remove focus from chat
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable == INTERNET_GAME_SETUP)
			joingame_.set_enabled(true);
	}
}

/// called when an entry of the server list was doubleclicked
void FullscreenMenuInternetLobby::server_doubleclicked() {
	// if the game is open try to connect it, if not do nothing.
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable == INTERNET_GAME_SETUP)
			clicked_joingame();
	}
}

/// called when the servername was changed
void FullscreenMenuInternetLobby::change_servername() {
	// Allow client to enter a servername manually
	hostgame_.set_enabled(true);
	edit_servername_.set_tooltip("");
	edit_servername_.set_warning(false);
	// Check whether a server of that name is already open.
	// And disable 'hostgame' button if yes.
	const std::vector<InternetGame>* games = InternetGaming::ref().games();
	if (games != nullptr) {
		for (const InternetGame& game : *games) {
			if (game.name == edit_servername_.text()) {
				hostgame_.set_enabled(false);
				edit_servername_.set_warning(true);
				edit_servername_.set_tooltip(
				   (boost::format(
				       _("The game %s is already running. Please choose a different name.")) %
				    g_gr->styles().font_style(UI::FontStyle::kWarning).as_font_tag(game.name))
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
			UI::WLMessageBox mmb(this, _("Connection Timed Out"), warning,
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

		GameClient netgame(ips, InternetGaming::ref().get_local_clientname(), true,
		                   opengames_list_.get_selected().name);
		netgame.run();
	} else
		throw wexception("No server selected! That should not happen!");
}

/// called when the 'host game' button was clicked
void FullscreenMenuInternetLobby::clicked_hostgame() {
	// Save selected servername as default for next time and during that take care that the name is
	// not empty.
	std::string servername_ui = edit_servername_.text();

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
		GameHost netgame(InternetGaming::ref().get_local_clientname(), true);
		netgame.run();
	} catch (...) {
		// Log out before going back to the main menu
		InternetGaming::ref().logout("SERVER_CRASHED");
		throw;
	}
}
