/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "netsetup_ggz.h"
#include "network/network_ggz.h"

#include <boost/bind.hpp>

#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "network/network.h"

#include "profile/profile.h"

Fullscreen_Menu_NetSetupGGZ::Fullscreen_Menu_NetSetupGGZ
	(char const * const nick, char const * const pwd, bool registered)
:
	Fullscreen_Menu_Base("internetmenu.jpg"),

// Values for alignment and size
	m_butx (m_xres * 13 / 40),
	m_butw (m_xres * 36 / 125),
	m_buth (m_yres * 19 / 400),
	m_lisw (m_xres * 623 / 1000),
	m_namechange(0),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Text labels
	title
		(this,
		 m_xres / 2, m_yres / 20,
		 _("Metaserver Lobby"), UI::Align_HCenter),
	m_users
		(this,
		 m_xres * 4 / 125, m_yres * 15 / 100,
		 _("Users online:")),
	m_opengames
		(this,
		 m_xres * 17 / 25, m_yres * 15 / 100,
		 _("List of games:")),
	m_servername
		(this,
		 m_xres * 17 / 25, m_yres * 63 / 100,
		 _("Name of your server:")),
	m_maxplayers
		(this,
		 m_xres * 17 / 25, m_yres * 73 / 100,
		 _("Maximum of players:")),

// Spinboxes
	maxplayers
		(this,
		 m_xres * 17 / 25, m_yres * 77 / 100, m_butw, m_buth * 7 / 10,
		 7, 1, 7), //  start/min./max. value dummy initializations

// Buttons
	joingame
		(this, "join_game",
		 m_xres * 17 / 25, m_yres * 55 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind(&Fullscreen_Menu_NetSetupGGZ::clicked_joingame, boost::ref(*this)),
		 _("Join this game"), std::string(), false, false,
		 m_fn, m_fs),
	hostgame
		(this, "host_game",
		 m_xres * 17 / 25, m_yres * 81 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind(&Fullscreen_Menu_NetSetupGGZ::clicked_hostgame, boost::ref(*this)),
		 _("Open a new game"), std::string(), true, false,
		 m_fn, m_fs),
	back
		(this, "back",
		 m_xres * 17 / 25, m_yres * 90 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind(&Fullscreen_Menu_NetSetupGGZ::end_modal, boost::ref(*this), (int32_t)CANCEL),
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),

// Edit boxes
	servername
		(this, m_xres * 17 / 25, m_yres * 68 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0),

// List
	usersonline
		(this,
		 m_xres * 4 / 125, m_yres     / 5,
		 m_lisw,          m_yres * 3 / 10),
	opengames
		(this,
		 m_xres * 17 / 25, m_yres    / 5,
		 m_butw,  m_yres * 7 / 20),

// The chat UI
	chat
		(this,
		 m_xres * 4 / 125,    m_yres * 51 / 100,
		 m_lisw, m_yres * 44 / 100,
		 NetGGZ::ref()),

// Login information
	nickname(nick),
	password(pwd),
	reg(registered),
	tried_login(false)
{
	// Set the texts and style of UI elements
	Section & s = g_options.pull_section("global"); //  for playername

	title       .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_opengames .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_users     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_servername.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_maxplayers.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	maxplayers  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	std::string server = s.get_string("servername", "");
	servername  .setText (server);
	servername  .changed.set
		(this, &Fullscreen_Menu_NetSetupGGZ::change_servername);
	servername  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	// prepare the lists
	usersonline .set_font(m_fn, m_fs);
	usersonline .add_column(22, "°");
	usersonline .add_column((m_lisw - 22) * 3 / 8, _("Name"));
	usersonline .add_column((m_lisw - 22) * 2 / 8, _("Points"));
	usersonline .add_column((m_lisw - 22) * 3 / 8, _("Server"));
	usersonline.set_column_compare
		(0, boost::bind(&Fullscreen_Menu_NetSetupGGZ::compare_usertype,
		 this, _1, _2));
	usersonline .double_clicked.set
		(this, &Fullscreen_Menu_NetSetupGGZ::user_doubleclicked);
	opengames   .set_font(m_fn, m_fs);
	opengames   .selected.set
		(this, &Fullscreen_Menu_NetSetupGGZ::server_selected);
	opengames   .double_clicked.set
		(this, &Fullscreen_Menu_NetSetupGGZ::server_doubleclicked);

	// try to connect to the metaserver
	if (!NetGGZ::ref().logged_in())
		connectToMetaserver();
}


/// think function of the UI (main loop)
void Fullscreen_Menu_NetSetupGGZ::think ()
{
	Fullscreen_Menu_Base::think ();

	// If we have no connection try to connect
	if 
		(not NetGGZ::ref().logged_in() and not NetGGZ::ref().is_connecting()
		 and not tried_login) {
		// Wait two seconds after the user changed the name to avoid reconnecting
		// after each changed character.
		if (m_namechange >= time(0) - 1)
			return;
		connectToMetaserver();
	}

	// Check ggz peers for new data
	NetGGZ::ref().process();

	if (NetGGZ::ref().updateForUsers())
		fillUserList(NetGGZ::ref().users());

	if (NetGGZ::ref().updateForTables())
		fillServersList(NetGGZ::ref().tables());
}


/// connects Widelands with the metaserver
void Fullscreen_Menu_NetSetupGGZ::connectToMetaserver()
{
	Section & s = g_options.pull_section("global");
	char const * const metaserver = s.get_string("metaserver", WL_METASERVER);

	if (NetGGZ::ref().initcore(metaserver, nickname, password, reg))
	{
		// Update of server spinbox
		maxplayers.setInterval(1, NetGGZ::ref().max_players());

		// Only one time registration
	}
	tried_login = true;
}


/// fills the server list
void Fullscreen_Menu_NetSetupGGZ::fillServersList
	(std::vector<Net_Game_Info> const & tables)
{
	// List and button cleanup
	opengames.clear();
	hostgame.set_enabled(true);
	joingame.set_enabled(false);
	std::string localservername = servername.text();
	localservername += " (" + build_id() + ")";
	for (uint32_t i = 0; i < tables.size(); ++i) {
		Net_Open_Game newOG;
		newOG.info = Net_Game_Info(tables[i]);
		PictureID pic;
		switch (newOG.info.state) {
			case LAN_GAME_OPEN:
				pic = g_gr->get_picture(PicMod_UI, "pics/continue.png");
				break;
			case LAN_GAME_CLOSED:
				pic = g_gr->get_picture(PicMod_UI, "pics/stop.png");
				break;
			default:
				continue;
		}
		// If one of the servers has the same name as the local name of the
		// users server, we disable the 'hostgame' button to avoid having more
		// than one server with the same name.
		if (tables[i].hostname == localservername)
			hostgame.set_enabled(false);
		opengames.add(newOG.info.hostname, newOG, pic);
	}
}

static int usertype_sortorder(GGZPlayerType type)
{
	switch (type) {
	case GGZ_PLAYER_BOT: return 0;
	case GGZ_PLAYER_ADMIN: return 1;
	case GGZ_PLAYER_HOST: return 2;
	case GGZ_PLAYER_NORMAL: return 3;
	case GGZ_PLAYER_GUEST: return 4;
	default: return 5;
	}
}

/**
 * \return \c true if the user in row \p rowa should come before the user in
 * row \p rowb when sorted according to usertype
 */
bool Fullscreen_Menu_NetSetupGGZ::compare_usertype
	(unsigned int rowa, unsigned int rowb)
{
	const Net_Player * playera = usersonline[rowa];
	const Net_Player * playerb = usersonline[rowb];

	return usertype_sortorder(playera->type) < usertype_sortorder(playerb->type);
}

/// fills the user list
void Fullscreen_Menu_NetSetupGGZ::fillUserList
	(std::vector<Net_Player> const & users)
{
	usersonline.clear();
	for (uint32_t i = 0; i < users.size(); ++i) {
		const Net_Player & user(users[i]);
		UI::Table<const Net_Player * const>::Entry_Record & er =
			usersonline.add(&user);
		er.set_string(1, user.name);
		er.set_string(2, user.stats);
		er.set_string(3, user.table);

		PictureID pic;
		switch (user.type) {
			// NOTE the chars in set_picture() are only there to make list sortable
			case GGZ_PLAYER_GUEST:
				pic = g_gr->get_picture(PicMod_UI, "pics/roadb_red.png");
				er.set_picture(0, pic);
				break;
			case GGZ_PLAYER_NORMAL:
				pic = g_gr->get_picture(PicMod_UI, "pics/roadb_yellow.png");
				er.set_picture(0, pic);
				break;
			case GGZ_PLAYER_ADMIN:
			case GGZ_PLAYER_HOST:
			case GGZ_PLAYER_BOT:
				pic = g_gr->get_picture(PicMod_UI, "pics/roadb_green.png");
				er.set_color(RGBColor(0, 255, 0));
				er.set_picture(0, pic);
				break;
			case GGZ_PLAYER_UNKNOWN:
				pic = g_gr->get_picture(PicMod_UI, "pics/low_priority_button.png");
				er.set_picture(0, pic);
				break;
			default:
				continue;
		}
	}
}


/// called when an entry of the user list was doubleclicked
void Fullscreen_Menu_NetSetupGGZ::user_doubleclicked (uint32_t i)
{
	// add a @username to the current edit text.
	if (usersonline.has_selection()) {
		UI::Table<const Net_Player * const>::Entry_Record & er
			= usersonline.get_record(i);

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
	}
}


/// called when an entry of the server list was selected
void Fullscreen_Menu_NetSetupGGZ::server_selected (uint32_t)
{
	if (opengames.has_selection()) {
		const Net_Open_Game * game = &opengames.get_selected();
		if (game->info.state == LAN_GAME_OPEN)
			joingame.set_enabled(true);
		else
			joingame.set_enabled(false);
	}
}


/// called when an entry of the server list was doubleclicked
void Fullscreen_Menu_NetSetupGGZ::server_doubleclicked (uint32_t)
{
	// if the game is open try to connect it, if not do nothing.
	if (opengames.has_selection()) {
		const Net_Open_Game * game = &opengames.get_selected();
		if (game->info.state == LAN_GAME_OPEN)
			clicked_joingame();
	}
}


/// called when the servername was changed
void Fullscreen_Menu_NetSetupGGZ::change_servername()
{
	// Allow user to enter a servername manually
	hostgame.set_enabled(true);

	// Check whether a server of that name is already open.
	// And disable 'hostgame' button if yes.
	std::vector<Net_Game_Info> const & tables = NetGGZ::ref().tables();
	for (uint32_t i = 0; i < tables.size(); ++i) {
		if (tables[i].hostname == servername.text())
			hostgame.set_enabled(false);
	}
}


/// called when the 'join game' button was clicked
void Fullscreen_Menu_NetSetupGGZ::clicked_joingame()
{
	if (opengames.has_selection()) {
		NetGGZ::ref().join(opengames.get_selected().info.hostname);
		end_modal(JOINGAME);
	} else
		throw wexception("No server selected! That should not happen!");
}


/// called when the 'host game' button was clicked
void Fullscreen_Menu_NetSetupGGZ::clicked_hostgame()
{
	// Save selected servername as default for next time.
	g_options.pull_section("global").set_string("servername", servername.text());
	NetGGZ::ref().set_local_servername(servername.text());
	end_modal(HOSTGAME);
}
