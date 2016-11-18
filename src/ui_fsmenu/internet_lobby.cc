/*
 * Copyright (C) 2004-2016 by the Widelands Development Team
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
#include "graphic/text_constants.h"
#include "network/constants.h"
#include "network/internet_gaming.h"
#include "network/netclient.h"
#include "network/nethost.h"
#include "profile/profile.h"
#include "ui_basic/messagebox.h"

FullscreenMenuInternetLobby::FullscreenMenuInternetLobby(char const* const nick,
                                                         char const* const pwd,
                                                         bool registered)
   : FullscreenMenuBase(),

     // Values for alignment and size
     butx_(get_w() * 13 / 40),
     butw_(get_w() * 36 / 125),
     buth_(get_h() * 19 / 400),
     lisw_(get_w() * 623 / 1000),
     fs_(fs_small()),
     prev_clientlist_len_(1000),

     // Text labels
     title(this, get_w() / 2, get_h() / 20, _("Metaserver Lobby"), UI::Align::kHCenter),
     clients_(this, get_w() * 4 / 125, get_h() * 15 / 100, _("Clients online:")),
     opengames_(this, get_w() * 17 / 25, get_h() * 15 / 100, _("List of games:")),
     servername_(this, get_w() * 17 / 25, get_h() * 63 / 100, _("Name of your server:")),

     // Buttons
     joingame_(this,
               "join_game",
               get_w() * 17 / 25,
               get_h() * 55 / 100,
               butw_,
               buth_,
               g_gr->images().get("images/ui_basic/but1.png"),
               _("Join this game")),
     hostgame_(this,
               "host_game",
               get_w() * 17 / 25,
               get_h() * 81 / 100,
               butw_,
               buth_,
               g_gr->images().get("images/ui_basic/but1.png"),
               _("Open a new game")),
     back_(this,
           "back",
           get_w() * 17 / 25,
           get_h() * 90 / 100,
           butw_,
           buth_,
           g_gr->images().get("images/ui_basic/but0.png"),
           _("Back")),

     // Edit boxes
     edit_servername_(this,
                      get_w() * 17 / 25,
                      get_h() * 68 / 100,
                      butw_,
                      buth_,
                      2,
                      g_gr->images().get("images/ui_basic/but2.png"),
                      fs_),

     // List
     clientsonline_list_(this, get_w() * 4 / 125, get_h() / 5, lisw_, get_h() * 3 / 10),
     opengames_list_(this, get_w() * 17 / 25, get_h() / 5, butw_, get_h() * 7 / 20),

     // The chat UI
     chat(this,
          get_w() * 4 / 125,
          get_h() * 51 / 100,
          lisw_,
          get_h() * 44 / 100,
          InternetGaming::ref()),

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
	Section& s = g_options.pull_section("global");  //  for playername

	title.set_fontsize(fs_big());
	opengames_.set_fontsize(fs_);
	clients_.set_fontsize(fs_);
	servername_.set_fontsize(fs_);
	std::string server = s.get_string("servername", "");
	edit_servername_.set_text(server);
	edit_servername_.changed.connect(
	   boost::bind(&FullscreenMenuInternetLobby::change_servername, this));

	// prepare the lists
	std::string t_tip =
	   (boost::format("%s%s%s%s%s%s%s%s%s%s") % "<rt><p><font underline=yes>" % _("User Status") %
	    "</font><br>" % "<img src=images/wui/overlays/roadb_yellow.png> " % _("Registered") %
	    "<br><img src=images/wui/overlays/roadb_green.png> " % _("Administrator") %
	    "<br><img src=images/wui/overlays/roadb_red.png> " % _("Unregistered") % "</p></rt>")
	      .str();
	clientsonline_list_.add_column(22, "*", t_tip);
	/** TRANSLATORS: Player Name */
	clientsonline_list_.add_column((lisw_ - 22) * 3 / 8, pgettext("player", "Name"));
	clientsonline_list_.add_column((lisw_ - 22) * 2 / 8, _("Version"));
	clientsonline_list_.add_column(
	   0, _("Game"), "", UI::Align::kLeft, UI::TableColumnType::kFlexible);
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
	Section& s = g_options.pull_section("global");
	const std::string& metaserver = s.get_string("metaserver", INTERNET_GAMING_METASERVER.c_str());
	uint32_t port = s.get_natural("metaserverport", INTERNET_GAMING_PORT);

	InternetGaming::ref().login(nickname_, password_, is_registered_, metaserver, port);
}

/// fills the server list
void FullscreenMenuInternetLobby::fill_games_list(const std::vector<InternetGame>* games) {
	// List and button cleanup
	opengames_list_.clear();
	hostgame_.set_enabled(true);
	joingame_.set_enabled(false);
	std::string localservername = edit_servername_.text();

	if (games != nullptr) {  // If no communication error occurred, fill the list.
		for (uint32_t i = 0; i < games->size(); ++i) {
			const Image* pic;
			const InternetGame& game = games->at(i);
			if (game.connectable) {
				if (game.build_id == build_id())
					pic = g_gr->images().get("images/ui_basic/continue.png");
				else {
					pic = g_gr->images().get("images/ui_basic/different.png");
				}
			} else {
				pic = g_gr->images().get("images/ui_basic/stop.png");
			}
			// If one of the servers has the same name as the local name of the
			// clients server, we disable the 'hostgame' button to avoid having more
			// than one server with the same name.
			if (game.name == localservername) {
				hostgame_.set_enabled(false);
			}
			opengames_list_.add(game.name, game, pic, false, game.build_id);
		}
	}
}

uint8_t FullscreenMenuInternetLobby::convert_clienttype(const std::string& type) {
	if (type == INTERNET_CLIENT_REGISTERED)
		return 1;
	if (type == INTERNET_CLIENT_SUPERUSER)
		return 2;
	if (type == INTERNET_CLIENT_BOT)
		return 3;
	// if (type == INTERNET_CLIENT_UNREGISTERED)
	return 0;
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
		for (uint32_t i = 0; i < clients->size(); ++i) {
			const InternetClient& client(clients->at(i));
			UI::Table<const InternetClient* const>::EntryRecord& er = clientsonline_list_.add(&client);
			er.set_string(1, client.name);
			er.set_string(2, client.build_id);
			er.set_string(3, client.game);

			const Image* pic;
			switch (convert_clienttype(client.type)) {
			case 0:  // UNREGISTERED
				pic = g_gr->images().get("images/wui/overlays/roadb_red.png");
				er.set_picture(0, pic);
				break;
			case 1:  // REGISTERED
				pic = g_gr->images().get("images/wui/overlays/roadb_yellow.png");
				er.set_picture(0, pic);
				break;
			case 2:  // SUPERUSER
			case 3:  // BOT
				pic = g_gr->images().get("images/wui/overlays/roadb_green.png");
				er.set_color(RGBColor(0, 255, 0));
				er.set_picture(0, pic);
				break;
			default:
				continue;
			}
		}
		// If a new player joins the lobby, play a sound.
		if (clients->size() > prev_clientlist_len_ && !InternetGaming::ref().sound_off()) {
			play_new_chat_member();
		}
		prev_clientlist_len_ = clients->size();
	}
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
		chat.focus();
	}
}

/// called when an entry of the server list was selected
void FullscreenMenuInternetLobby::server_selected() {
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable)
			joingame_.set_enabled(true);
		else
			joingame_.set_enabled(false);
	}
}

/// called when an entry of the server list was doubleclicked
void FullscreenMenuInternetLobby::server_doubleclicked() {
	// if the game is open try to connect it, if not do nothing.
	if (opengames_list_.has_selection()) {
		const InternetGame* game = &opengames_list_.get_selected();
		if (game->connectable)
			clicked_joingame();
	}
}

/// called when the servername was changed
void FullscreenMenuInternetLobby::change_servername() {
	// Allow client to enter a servername manually
	hostgame_.set_enabled(true);

	// Check whether a server of that name is already open.
	// And disable 'hostgame' button if yes.
	const std::vector<InternetGame>* games = InternetGaming::ref().games();
	if (games != nullptr) {
		for (uint32_t i = 0; i < games->size(); ++i) {
			if (games->at(i).name == edit_servername_.text()) {
				hostgame_.set_enabled(false);
			}
		}
	}
}

/// called when the 'join game' button was clicked
void FullscreenMenuInternetLobby::clicked_joingame() {
	if (opengames_list_.has_selection()) {
		InternetGaming::ref().join_game(opengames_list_.get_selected().name);

		uint32_t const secs = time(nullptr);
		while (InternetGaming::ref().ip().size() < 1) {
			InternetGaming::ref().handle_metaserver_communication();
			// give some time for the answer + for a relogin, if a problem occurs.
			if ((INTERNET_GAMING_TIMEOUT * 5 / 3) < time(nullptr) - secs) {
				// Show a popup warning message
				const std::string warning(
				   _("Widelands was unable to get the IP address of the server in time.\n"
				     "There seems to be a network problem, either on your side or on the side\n"
				     "of the server.\n"));
				UI::WLMessageBox mmb(this, _("Connection timed out"), warning,
				                     UI::WLMessageBox::MBoxType::kOk, UI::Align::kLeft);
				mmb.run<UI::Panel::Returncodes>();
				return InternetGaming::ref().set_error();
			}
		}
		std::string ip = InternetGaming::ref().ip();

		//  convert IPv6 addresses returned by the metaserver to IPv4 addresses.
		//  At the moment SDL_net does not support IPv6 anyways.
		if (!ip.compare(0, 7, "::ffff:")) {
			ip = ip.substr(7);
			log("InternetGaming: cut IPv6 address: %s\n", ip.c_str());
		}

		IPaddress peer;
		if (hostent* const he = gethostbyname(ip.c_str())) {
			peer.host = (reinterpret_cast<in_addr*>(he->h_addr_list[0]))->s_addr;
			DIAG_OFF("-Wold-style-cast")
			peer.port = htons(WIDELANDS_PORT);
			DIAG_ON("-Wold-style-cast")
		} else {
			// Actually the game is not done, but that way we are again listed as in the lobby
			InternetGaming::ref().set_game_done();
			// Show a popup warning message
			std::string warningheader(_("Connection problem"));
			std::string warning(_("Widelands was unable to connect to the host."));
			UI::WLMessageBox mmb(
			   this, warningheader, warning, UI::WLMessageBox::MBoxType::kOk, UI::Align::kLeft);
			mmb.run<UI::Panel::Returncodes>();
		}
		SDLNet_ResolveHost(&peer, ip.c_str(), WIDELANDS_PORT);

		NetClient netgame(&peer, InternetGaming::ref().get_local_clientname(), true);
		netgame.run();
	} else
		throw wexception("No server selected! That should not happen!");
}

/// called when the 'host game' button was clicked
void FullscreenMenuInternetLobby::clicked_hostgame() {
	// Save selected servername as default for next time and during that take care that the name is
	// not empty.
	std::string servername_ui = edit_servername_.text();
	if (servername_ui.empty()) {
		/** TRANSLATORS: This is shown for multiplayer games when no host */
		/** TRANSLATORS: server to connect to has been specified yet. */
		servername_ui = pgettext("server_name", "unnamed");
	}

	g_options.pull_section("global").set_string("servername", servername_ui);

	// Set up the game
	InternetGaming::ref().set_local_servername(servername_ui);

	// Start the game
	NetHost netgame(InternetGaming::ref().get_local_clientname(), true);
	netgame.run();
}
