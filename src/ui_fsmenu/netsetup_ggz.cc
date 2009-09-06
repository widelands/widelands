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

#if HAVE_GGZ

#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "network/network.h"
#include "network/network_ggz.h"
#include "profile/profile.h"

Fullscreen_Menu_NetSetupGGZ::Fullscreen_Menu_NetSetupGGZ () :
	Fullscreen_Menu_Base("internetmenu.jpg"), //  FIXME change this

// Values for alignment and size
	m_butx (m_xres * 13 / 40),
	m_butw (m_xres / 4),
	m_buth (m_yres * 19 / 400),
	m_lisw (m_xres * 20 / 63),
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
		 m_xres * 3 / 50, m_yres * 15 / 100,
		 _("Users online:")),
	m_opengames
		(this,
		 m_lisw + m_xres * 85 / 1000, m_yres * 15 / 100,
		 _("List of games:")),
	m_playername
		(this,
		 m_xres * 17 / 25, m_yres * 15 / 100,
		 _("Your nickname:")),
	m_servername
		(this,
		 m_xres * 17 / 25, m_yres * 28 / 100,
		 _("Name of your server:")),
	m_maxplayers
		(this,
		 m_xres * 17 / 25, m_yres * 38 / 100,
		 _("Maximum of players:")),

// Spinboxes
	maxplayers
		(this,
		 m_xres * 17 / 25, m_yres * 42 / 100, m_butw, m_buth * 7 / 10,
		 4, 1, 7), //  start/min./max. value dummy initializations

// Buttons
	joingame
		(this,
		 m_lisw + m_xres * 85 / 1000, m_yres * 55 / 100, m_lisw * 4 / 5, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_NetSetupGGZ::clicked_joingame, *this,
		 _("Join this game"), std::string(), false, false,
		 m_fn, m_fs),
	hostgame
		(this,
		 m_xres * 17 / 25, m_yres * 46 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_NetSetupGGZ::clicked_hostgame, *this,
		 _("Open a new game"), std::string(), true, false,
		 m_fn, m_fs),
	back
		(this,
		 m_xres * 17 / 25, m_yres * 55 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_NetSetupGGZ::end_modal, *this, CANCEL,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs),

// Edit boxes
	playername
		(this, m_xres * 17 / 25, m_yres     /  5, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0),
	servername
		(this, m_xres * 17 / 25, m_yres * 33 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0),

// List
	usersonline
		(this,
		 m_xres * 3 / 50, m_yres     / 5,
		 m_lisw,          m_yres * 2 / 5),
	opengames
		(this,
		 m_lisw + m_xres * 17 / 200, m_yres     /  5,
		 m_lisw * 4 / 5,             m_yres * 7 / 20),

// The chat UI
	chat
		(this,
		 m_xres * 3 / 50,  m_yres * 5 / 8,
		 m_xres * 217 / 250, m_yres * 8 / 25,
		 NetGGZ::ref())
{
	// Set the texts and style of UI elements
	Section & s = g_options.pull_section("global"); //  for playername

	title       .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_opengames .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_users     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_playername.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_servername.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_maxplayers.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	maxplayers  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	std::string server = s.get_string("servername", "");
	servername  .setText (server);
	servername  .changed.set
		(this, &Fullscreen_Menu_NetSetupGGZ::change_servername);
	servername  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	playername  .setText(s.get_string("nickname", (_("nobody"))));
	playername  .changed.set
		(this, &Fullscreen_Menu_NetSetupGGZ::change_playername);
	playername  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);

	// prepare the lists
	usersonline .set_font(m_fn, m_fs);
	usersonline .add_column(m_lisw / 2, _("Name"));
	usersonline .add_column(m_lisw / 2, _("Server"));
	opengames   .set_font(m_fn, m_fs);
	opengames   .selected.set
		(this, &Fullscreen_Menu_NetSetupGGZ::server_selected);
	opengames   .double_clicked.set
		(this, &Fullscreen_Menu_NetSetupGGZ::server_doubleclicked);

	// try to connect to the metaserver
	if (!NetGGZ::ref().usedcore())
		connectToMetaserver();
#ifdef WIN32
	//bug in ggz on windows - not able to open a server
	hostgame.set_enabled(false);
#endif
}


/// think function of the UI (main loop)
void Fullscreen_Menu_NetSetupGGZ::think ()
{
	Fullscreen_Menu_Base::think ();

	// If we have no connection try to connect
	if (!NetGGZ::ref().usedcore()) {
		// Wait two seconds after the user changed the name to avoid reconnecting
		// after each changed character.
		if (m_namechange >= time(0) - 1)
			return;
		connectToMetaserver();
	}

	// Check ggz peers for new data
	NetGGZ::ref().datacore();

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

	if (NetGGZ::ref().initcore(metaserver, playername.text().c_str())) {
		// Update of server spinbox
		maxplayers.setInterval(1, NetGGZ::ref().max_players());
	}
}


/// fills the server list
void Fullscreen_Menu_NetSetupGGZ::fillServersList
	(std::vector<Net_Game_Info> const & tables)
{
	// List and button cleanup
	opengames.clear();
#ifndef WIN32
	//bug in ggz on windows - not able to open a server
	hostgame.set_enabled(true);
#endif
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


/// fills the user list
void Fullscreen_Menu_NetSetupGGZ::fillUserList
	(std::vector<Net_Player> const & users)
{
	usersonline.clear();
	for (uint32_t i = 0; i < users.size(); ++i) {
		Net_Player user(users[i]);
		UI::Table<const Net_Player * const>::Entry_Record & er =
			usersonline.add(&user);
			er.set_string(0, users[i].name);
			er.set_string(1, users[i].table);
	}
}


/// \returns the name of the player
std::string const & Fullscreen_Menu_NetSetupGGZ::get_playername()
{
	return playername.text();
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
#ifdef WIN32
	//bug in ggz on windows - not able to open a server
	return;
#endif
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


/// called when the playername was changed
void Fullscreen_Menu_NetSetupGGZ::change_playername()
{
	g_options.pull_section("global").set_string("nickname", playername.text());
	NetGGZ::ref().deinitcore();
	// instead of directly reconnecting to the server, we wait 2 seconds
	// to avoid reconnection after each character change.
	m_namechange = time(0);
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

#endif // if HAVE_GGZ
