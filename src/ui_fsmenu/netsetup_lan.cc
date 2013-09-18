/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#include "compile_diagnostics.h"
#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "network/network.h"
#include "profile/profile.h"

Fullscreen_Menu_NetSetupLAN::Fullscreen_Menu_NetSetupLAN () :
	Fullscreen_Menu_Base("singleplmenu.jpg"), //  FIXME change this

// Values for alignment and size
	m_butx (get_w() * 13 / 40),
	m_butw (get_w() / 4),
	m_buth (get_h() * 19 / 400),
	m_lisw (get_w() * 9 / 16),

// Text labels
	title
		(this,
		 get_w() / 2, get_h() / 10,
		 _("Begin Network Game"), UI::Align_HCenter),
	m_opengames
		(this,
		 get_w() * 3 / 50, get_h() * 27 / 100,
		 _("List of games in your local network:")),
	m_playername
		(this,
		 get_w() * 16 / 25, get_h() * 27 / 100,
		 _("Your nickname:")),
	m_hostname
		(this,
		 get_w() * 16 / 25, get_h() * 17 / 40,
		 _("Host to connect:")),

// Buttons
	joingame
		(this, "join_game",
		 get_w() * 16 / 25, get_h() * 5333 / 10000, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Join this game"), std::string(), true, false),
	hostgame
		(this, "host_game",
		 get_w() * 16 / 25, get_h() * 6083 / 10000, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Host a new game"), std::string(), true, false),
	back
		(this, "back",
		 get_w() * 16 / 25, get_h() * 8333 / 10000, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),
	loadlasthost
		(this, "load_previous_host",
		 get_w() * 171 / 200, get_h() * 19 / 40, m_buth, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 g_gr->images().get("pics/menu_load_game.png"),
		 _("Load previous host"), true, false),

// Edit boxes
	playername
		(this, get_w() * 16 / 25, get_h() * 3333 / 10000, m_butw,       m_buth,
		 g_gr->images().get("pics/but2.png")),
	hostname
		(this, get_w() * 16 / 25, get_h() * 19 / 40,  get_w() * 17 / 80, m_buth,
		 g_gr->images().get("pics/but2.png")),

// List
	opengames
		(this,
		 get_w() * 3 / 50, get_h() * 3333 / 10000, m_lisw, get_h() * 5433 / 10000)
{
	joingame.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_NetSetupLAN::clicked_joingame, boost::ref(*this)));
	hostgame.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_NetSetupLAN::clicked_hostgame, boost::ref(*this)));
	back.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_NetSetupLAN::end_modal, boost::ref(*this),
			  static_cast<int32_t>(CANCEL)));
	loadlasthost.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_NetSetupLAN::clicked_lasthost, boost::ref(*this)));

	Section & s = g_options.pull_section("global"); //  for playername

	joingame.set_font(font_small());
	hostgame.set_font(font_small());
	back.set_font(font_small());
	loadlasthost.set_font(font_small());

	title       .set_textstyle(ts_big());
	m_opengames .set_textstyle(ts_small());
	m_playername.set_textstyle(ts_small());
	m_hostname  .set_textstyle(ts_small());
	hostname    .changed.connect
		(boost::bind(&Fullscreen_Menu_NetSetupLAN::change_hostname, this));
	hostname    .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	playername  .setText  (s.get_string("nickname", (_("nobody"))));
	playername  .changed.connect
		(boost::bind(&Fullscreen_Menu_NetSetupLAN::change_playername, this));
	playername  .set_font(ui_fn(), fs_small(), UI_FONT_CLR_FG);
	opengames   .set_font(ui_fn(), fs_small());
	opengames   .add_column(m_lisw * 2 / 5, _("Host"));
	opengames   .add_column(m_lisw * 2 / 5, _("Map"));
	opengames   .add_column(m_lisw     / 5, _("State"));
	opengames   .selected.connect(boost::bind(&Fullscreen_Menu_NetSetupLAN::game_selected, this, _1));
	opengames   .double_clicked.connect
		(boost::bind(&Fullscreen_Menu_NetSetupLAN::game_doubleclicked, this, _1));
	discovery   .set_callback (discovery_callback, this);

	joingame.set_enabled(false);
}

void Fullscreen_Menu_NetSetupLAN::think ()
{
	Fullscreen_Menu_Base::think ();

	discovery.run();
}

bool Fullscreen_Menu_NetSetupLAN::get_host_address
	(uint32_t & addr, uint16_t & port)
{
	const std::string & host = hostname.text();

	const uint32_t opengames_size = opengames.size();
	for (uint32_t i = 0; i < opengames_size; ++i) {
		const Net_Open_Game & game = *opengames[i];

		if (not strcmp(game.info.hostname, host.c_str())) {
			addr = game.address;
			port = game.port;
			return true;
		}
	}

	if (hostent * const he = gethostbyname(host.c_str())) {
		addr = (reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
GCC_DIAG_OFF("-Wold-style-cast")
		port = htons(WIDELANDS_PORT);
GCC_DIAG_ON("-Wold-style-cast")
		return true;
	} else
		return false;
}

const std::string & Fullscreen_Menu_NetSetupLAN::get_playername()
{
	return playername.text();
}


void Fullscreen_Menu_NetSetupLAN::game_selected (uint32_t) {
	if (opengames.has_selection()) {
		if (const Net_Open_Game * const game = opengames.get_selected()) {
			hostname.setText(game->info.hostname);
			joingame.set_enabled(true);
		}
	}
}

void Fullscreen_Menu_NetSetupLAN::game_doubleclicked (uint32_t) {
	clicked_joingame();
}

void Fullscreen_Menu_NetSetupLAN::update_game_info
	(UI::Table<Net_Open_Game const * const>::Entry_Record & er,
	 const Net_Game_Info & info)
{
	er.set_string (0, info.hostname);
	er.set_string (1, info.map);

	switch (info.state) {
	case LAN_GAME_OPEN:   er.set_string(2, _("Open"));   break;
	case LAN_GAME_CLOSED: er.set_string(2, _("Closed")); break;
	default:              er.set_string(2, _("Unknown")); break;
	};
}

void Fullscreen_Menu_NetSetupLAN::game_opened (const Net_Open_Game * game)
{
	update_game_info(opengames.add(game), game->info);
}

void Fullscreen_Menu_NetSetupLAN::game_closed (const Net_Open_Game *) {}

void Fullscreen_Menu_NetSetupLAN::game_updated (const Net_Open_Game * game)
{
	if
		(UI::Table<const Net_Open_Game * const>::Entry_Record * const er =
		 opengames.find(game))
		update_game_info(*er, game->info);
}

void Fullscreen_Menu_NetSetupLAN::discovery_callback
	(int32_t               const type,
	 Net_Open_Game const * const game,
	 void                * const userdata)
{
	switch (type) {
	case LAN_Game_Finder::GameOpened:
		static_cast<Fullscreen_Menu_NetSetupLAN *>(userdata)->game_opened (game);
		break;
	case LAN_Game_Finder::GameClosed:
		static_cast<Fullscreen_Menu_NetSetupLAN *>(userdata)->game_closed (game);
		break;
	case LAN_Game_Finder::GameUpdated:
		static_cast<Fullscreen_Menu_NetSetupLAN *>(userdata)->game_updated(game);
		break;
	default:
		abort ();
	}
}

void Fullscreen_Menu_NetSetupLAN::change_hostname()
{
	// Allow user to enter a hostname manually
	opengames.select(opengames.no_selection_index());
	joingame.set_enabled(hostname.text().size());
}

void Fullscreen_Menu_NetSetupLAN::change_playername()
{
	g_options.pull_section("global").set_string("nickname", playername.text());
}

void Fullscreen_Menu_NetSetupLAN::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	g_options.pull_section("global").set_string("lasthost", hostname.text());

	end_modal(JOINGAME);
}

void Fullscreen_Menu_NetSetupLAN::clicked_hostgame() {
	end_modal(HOSTGAME);
}

void Fullscreen_Menu_NetSetupLAN::clicked_lasthost() {
	Section & s = g_options.get_safe_section("global");
	std::string const host = s.get_string("lasthost", "");
	hostname.setText(host);
	if (host.size())
		joingame.set_enabled(true);
	opengames.select(opengames.no_selection_index());
}

