/*
 * Copyright (C) 2004, 2006-2009, 2011-2013 by the Widelands Development Team
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

FullscreenMenuInternetLobby::FullscreenMenuInternetLobby
	(char const * const nick, char const * const pwd, bool registered)
:
	FullscreenMenuBase("internetmenu.jpg"),

// Values for alignment and size
	m_butx (get_w() * 13 / 40),
	m_butw (get_w() * 36 / 125),
	m_buth (get_h() * 19 / 400),
	m_lisw (get_w() * 623 / 1000),
	m_fs   (fs_small()),
	m_prev_clientlist_len(1000),
	m_fn   (ui_fn()),

// Text labels
	title
		(this,
		 get_w() / 2, get_h() / 20,
		 _("Metaserver Lobby"), UI::Align_HCenter),
	m_clients
		(this,
		 get_w() * 4 / 125, get_h() * 15 / 100,
		 _("Clients online:")),
	m_opengames
		(this,
		 get_w() * 17 / 25, get_h() * 15 / 100,
		 _("List of games:")),
	m_servername
		(this,
		 get_w() * 17 / 25, get_h() * 63 / 100,
		 _("Name of your server:")),

// Buttons
	joingame
		(this, "join_game",
		 get_w() * 17 / 25, get_h() * 55 / 100, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Join this game"), std::string(), false, false),
	hostgame
		(this, "host_game",
		 get_w() * 17 / 25, get_h() * 81 / 100, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Open a new game"), std::string(), true, false),
	back
		(this, "back",
		 get_w() * 17 / 25, get_h() * 90 / 100, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),

// Edit boxes
	servername
		(this, get_w() * 17 / 25, get_h() * 68 / 100, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png")),

// List
	clientsonline
		(this,
		 get_w() * 4 / 125, get_h()     / 5,
		 m_lisw,          get_h() * 3 / 10),
	opengames
		(this,
		 get_w() * 17 / 25, get_h()    / 5,
		 m_butw,  get_h() * 7 / 20),

// The chat UI
	chat
		(this,
		 get_w() * 4 / 125,    get_h() * 51 / 100,
		 m_lisw, get_h() * 44 / 100,
		 InternetGaming::ref()),

// Login information
	nickname(nick),
	password(pwd),
	reg(registered)
{
	joingame.sigclicked.connect(
				boost::bind(&FullscreenMenuInternetLobby::clicked_joingame, boost::ref(*this)));
	hostgame.sigclicked.connect(
				boost::bind(&FullscreenMenuInternetLobby::clicked_hostgame, boost::ref(*this)));
	back.sigclicked.connect(boost::bind(&FullscreenMenuInternetLobby::clicked_back, boost::ref(*this)));

	// Set the texts and style of UI elements
	Section & s = g_options.pull_section("global"); //  for playername

	title       .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_opengames .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_clients     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_servername.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	std::string server = s.get_string("servername", "");
	servername  .set_text (server);
	servername  .changed.connect
		(boost::bind(&FullscreenMenuInternetLobby::change_servername, this));
	servername  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	// prepare the lists
	clientsonline .set_font(m_fn, m_fs);
	std::string t_tip = (boost::format("%s%s%s%s%s%s%s%s%s%s")
		% "<rt><p><font underline=yes>"
		% _("User Status")
		% "</font><br>"
		% "<img src=pics/roadb_yellow.png> "
		% _("Registered")
		% "<br><img src=pics/roadb_green.png> "
		% _("Administrator")
		% "<br><img src=pics/roadb_red.png> "
		% _("Unregistered")
		%  "</p></rt>").str();
	clientsonline .add_column(22, "*", t_tip);
	clientsonline .add_column((m_lisw - 22) * 3 / 8, _("Name"));
	clientsonline .add_column((m_lisw - 22) * 2 / 8, _("Points"));
	clientsonline .add_column((m_lisw - 22) * 3 / 8, _("Game"));
	clientsonline.set_column_compare
		(0, boost::bind(&FullscreenMenuInternetLobby::compare_clienttype, this, _1, _2));
	clientsonline .double_clicked.connect
		(boost::bind(&FullscreenMenuInternetLobby::client_doubleclicked, this, _1));
	opengames   .set_font(m_fn, m_fs);
	opengames   .selected.connect
		(boost::bind(&FullscreenMenuInternetLobby::server_selected, this));
	opengames   .double_clicked.connect
		(boost::bind(&FullscreenMenuInternetLobby::server_doubleclicked, this));

	// try to connect to the metaserver
	if (!InternetGaming::ref().error() && !InternetGaming::ref().logged_in())
		connect_to_metaserver();
}


/// think function of the UI (main loop)
void FullscreenMenuInternetLobby::think ()
{
	FullscreenMenuBase::think();

	if (!InternetGaming::ref().error()) {

		// If we have no connection try to connect
		if (!InternetGaming::ref().logged_in()) {
			connect_to_metaserver();
		}

		// Check whether metaserver send some data
		InternetGaming::ref().handle_metaserver_communication();
	}

	if (InternetGaming::ref().update_for_clients())
		fill_client_list(InternetGaming::ref().clients());

	if (InternetGaming::ref().update_for_games())
		fill_games_list(InternetGaming::ref().games());
}

void FullscreenMenuInternetLobby::clicked_ok()
{
	if (joingame.enabled()) {
		server_doubleclicked();
	} else {
		clicked_hostgame();
	}
}



/// connects Widelands with the metaserver
void FullscreenMenuInternetLobby::connect_to_metaserver()
{
	Section & s = g_options.pull_section("global");
	const std::string & metaserver = s.get_string("metaserver", INTERNET_GAMING_METASERVER.c_str());
	uint32_t port = s.get_natural("metaserverport", INTERNET_GAMING_PORT);

	InternetGaming::ref().login(nickname, password, reg, metaserver, port);
}


/// fills the server list
void FullscreenMenuInternetLobby::fill_games_list(const std::vector<InternetGame> & games)
{
	// List and button cleanup
	opengames.clear();
	hostgame.set_enabled(true);
	joingame.set_enabled(false);
	std::string localservername = servername.text();
	for (uint32_t i = 0; i < games.size(); ++i) {
		const Image* pic;
		if (games.at(i).connectable) {
			if (games.at(i).build_id == build_id())
				pic = g_gr->images().get("pics/continue.png");
			else {
				pic = g_gr->images().get("pics/different.png");
			}
		} else {
			pic = g_gr->images().get("pics/stop.png");
		}
		// If one of the servers has the same name as the local name of the
		// clients server, we disable the 'hostgame' button to avoid having more
		// than one server with the same name.
		if (games.at(i).name == localservername)
			hostgame.set_enabled(false);
		opengames.add(games.at(i).name, games.at(i), pic, false, games.at(i).build_id);
	}
}


uint8_t FullscreenMenuInternetLobby::convert_clienttype(const std::string & type) {
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
bool FullscreenMenuInternetLobby::compare_clienttype(unsigned int rowa, unsigned int rowb)
{
	const InternetClient * playera = clientsonline[rowa];
	const InternetClient * playerb = clientsonline[rowb];

	return convert_clienttype(playera->type) < convert_clienttype(playerb->type);
}

/// fills the client list
void FullscreenMenuInternetLobby::fill_client_list(const std::vector<InternetClient> & clients)
{
	clientsonline.clear();
	for (uint32_t i = 0; i < clients.size(); ++i) {
		const InternetClient & client(clients[i]);
		UI::Table<const InternetClient * const>::EntryRecord & er = clientsonline.add(&client);
		er.set_string(1, client.name);
		er.set_string(2, client.points);
		er.set_string(3, client.game);

		const Image* pic;
		switch (convert_clienttype(client.type)) {
			case 0: // UNREGISTERED
				pic = g_gr->images().get("pics/roadb_red.png");
				er.set_picture(0, pic);
				break;
			case 1: // REGISTERED
				pic = g_gr->images().get("pics/roadb_yellow.png");
				er.set_picture(0, pic);
				break;
			case 2: // SUPERUSER
			case 3: // BOT
				pic = g_gr->images().get("pics/roadb_green.png");
				er.set_color(RGBColor(0, 255, 0));
				er.set_picture(0, pic);
				break;
			default:
				continue;
		}
	}

	// If a new player joins the lobby, play a sound.
	if (clients.size() != m_prev_clientlist_len)
	{
		if (clients.size() > m_prev_clientlist_len && !InternetGaming::ref().sound_off())
			play_new_chat_member();
		m_prev_clientlist_len = clients.size();
	}
}


/// called when an entry of the client list was doubleclicked
void FullscreenMenuInternetLobby::client_doubleclicked (uint32_t i)
{
	// add a @clientname to the current edit text.
	if (clientsonline.has_selection()) {
		UI::Table<const InternetClient * const>::EntryRecord & er = clientsonline.get_record(i);

		std::string temp("@");
		temp += er.get_string(1);
		std::string text(chat.get_edit_text());

		if (text.size() && (text.at(0) == '@')) { // already PM ?
			if (text.find(' ') <= text.size())
				text = text.substr(text.find(' '), text.size());
			else
				text.clear();
		} else
			temp += " "; // The needed space between name and text

		temp += text;
		chat.set_edit_text(temp);
		chat.focus();
	}
}


/// called when an entry of the server list was selected
void FullscreenMenuInternetLobby::server_selected()
{
	if (opengames.has_selection()) {
		const InternetGame * game = &opengames.get_selected();
		if (game->connectable)
			joingame.set_enabled(true);
		else
			joingame.set_enabled(false);
	}
}


/// called when an entry of the server list was doubleclicked
void FullscreenMenuInternetLobby::server_doubleclicked()
{
	// if the game is open try to connect it, if not do nothing.
	if (opengames.has_selection()) {
		const InternetGame * game = &opengames.get_selected();
		if (game->connectable)
			clicked_joingame();
	}
}


/// called when the servername was changed
void FullscreenMenuInternetLobby::change_servername()
{
	// Allow client to enter a servername manually
	hostgame.set_enabled(true);

	// Check whether a server of that name is already open.
	// And disable 'hostgame' button if yes.
	const std::vector<InternetGame> & games = InternetGaming::ref().games();
	for (uint32_t i = 0; i < games.size(); ++i) {
		if (games.at(i).name == servername.text())
			hostgame.set_enabled(false);
	}
}


/// called when the 'join game' button was clicked
void FullscreenMenuInternetLobby::clicked_joingame()
{
	if (opengames.has_selection()) {
		InternetGaming::ref().join_game(opengames.get_selected().name);

		uint32_t const secs = time(nullptr);
		while (InternetGaming::ref().ip().size() < 1) {
			InternetGaming::ref().handle_metaserver_communication();
			 // give some time for the answer + for a relogin, if a problem occurs.
			if ((INTERNET_GAMING_TIMEOUT * 5 / 3) < time(nullptr) - secs) {
				// Show a popup warning message
				const std::string warning
					(_
						("Widelands was unable to get the IP address of the server in time.\n"
						 "There seems to be a network problem, either on your side or on the side\n"
						 "of the server.\n"));
				UI::WLMessageBox mmb(this,
											_("Connection timed out"),
											warning,
											UI::WLMessageBox::MBoxType::kOk,
											UI::Align_Left);
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
		if (hostent * const he = gethostbyname(ip.c_str())) {
			peer.host = (reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
DIAG_OFF("-Wold-style-cast")
			peer.port = htons(WIDELANDS_PORT);
DIAG_ON("-Wold-style-cast")
		} else {
			// Actually the game is not done, but that way we are again listed as in the lobby
			InternetGaming::ref().set_game_done();
			// Show a popup warning message
			std::string warningheader(_("Connection problem"));
			std::string warning(_("Widelands was unable to connect to the host."));
			UI::WLMessageBox mmb(this, warningheader, warning, UI::WLMessageBox::MBoxType::kOk, UI::Align_Left);
			mmb.run<UI::Panel::Returncodes>();
		}
		SDLNet_ResolveHost (&peer, ip.c_str(), WIDELANDS_PORT);

		NetClient netgame(&peer, InternetGaming::ref().get_local_clientname(), true);
		netgame.run();
	} else
		throw wexception("No server selected! That should not happen!");
}


/// called when the 'host game' button was clicked
void FullscreenMenuInternetLobby::clicked_hostgame()
{
	// Save selected servername as default for next time.
	g_options.pull_section("global").set_string("servername", servername.text());

	// Set up the game
	InternetGaming::ref().set_local_servername(servername.text());

	// Start the game
	NetHost netgame(InternetGaming::ref().get_local_clientname(), true);
	netgame.run();
}
