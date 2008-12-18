/*
 * Copyright (C) 2004, 2006-2008 by the Widelands Development Team
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

#include "fullscreen_menu_netsetup.h"
#include "constants.h"
#include "graphic.h"
#include "i18n.h"
#include "network.h"
#include "profile.h"

Fullscreen_Menu_NetSetup::Fullscreen_Menu_NetSetup ()
:
Fullscreen_Menu_Base("singleplmenu.jpg"), // change this

// Values for alignment and size
m_xres
	(gr_x()),
m_yres
	(gr_y()),
m_butx
	(m_xres * 13 / 40),
m_butw
	(m_xres / 4),
m_buth
	(m_yres * 19 / 400),
m_lisw
	(m_xres * 9 / 16),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Text labels
title
	(this,
	 m_xres/2, m_yres / 10,
	 _("Begin Network Game"), Align_HCenter),
m_opengames
	(this,
	 m_xres * 3 / 50, m_yres * 27 / 100,
	 _("List of games in your local network:"), Align_Left),
m_playername
	(this,
	 m_xres * 16 / 25, m_yres * 27 / 100,
	 _("Your nickname:"), Align_Left),
m_hostname
	(this,
	 m_xres * 16 / 25, m_yres * 17 / 40,
	 _("Host to connect:"), Align_Left),

// Buttons
joingame
	(this,
	 m_xres * 16 / 25, m_yres * 5333 / 10000, m_butw, m_buth,
	 1,
	 &Fullscreen_Menu_NetSetup::clicked_joingame, this,
	 _("Join this game"), std::string(), true, false,
	 m_fn, m_fs),
hostgame
	(this,
	 m_xres * 16 / 25, m_yres * 6083 / 10000, m_butw, m_buth,
	 1,
	 &Fullscreen_Menu_NetSetup::clicked_hostgame, this,
	 _("Host a new game"), std::string(), true, false,
	 m_fn, m_fs),
back
	(this,
	 m_xres * 16 / 25, m_yres * 8333 / 10000, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_NetSetup::end_modal, this, CANCEL,
	 _("Back"), std::string(), true, false,
	 m_fn, m_fs),
loadlasthost
	(this,
	 m_xres * 171 / 200, m_yres * 19 / 40, m_buth, m_buth,
	 1,
	 g_gr->get_picture(PicMod_UI, "pics/menu_load_game.png"),
	 &Fullscreen_Menu_NetSetup::clicked_lasthost, this,
	 _("Load previous host"), true, false,
	 m_fn, m_fs),

// Edit boxes
playername
	(this, m_xres * 16 / 25, m_yres * 3333 / 10000, m_butw,       m_buth, 2, 0),
hostname
	(this, m_xres * 16 / 25, m_yres * 19 / 40,  m_xres * 17 / 80, m_buth, 2, 0),

// List
opengames
	(this, m_xres * 3 / 50, m_yres * 3333 / 10000, m_lisw, m_yres * 5433 / 10000)
{
	Section *s = g_options.pull_section("global");//for playername

	title       .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	m_opengames .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_playername.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_hostname  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	hostname    .changed.set(this, &Fullscreen_Menu_NetSetup::change_hostname);
	hostname    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	playername  .setText  (s->get_string("nickname", (_("nobody"))));
	playername  .changed.set(this, &Fullscreen_Menu_NetSetup::change_playername);
	playername  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	opengames   .set_font(m_fn, m_fs);
	opengames   .add_column(m_lisw * 2 / 5, _("Host"));
	opengames   .add_column(m_lisw * 2 / 5, _("Map"));
	opengames   .add_column(m_lisw     / 5, _("State"));
	opengames   .selected.set (this, &Fullscreen_Menu_NetSetup::game_selected);
	opengames   .double_clicked.set
		(this, &Fullscreen_Menu_NetSetup::game_doubleclicked);
	discovery   .set_callback (discovery_callback, this);

	joingame.set_enabled(false);
}

void Fullscreen_Menu_NetSetup::think ()
{
	Fullscreen_Menu_Base::think ();

	discovery.run();
}

bool Fullscreen_Menu_NetSetup::get_host_address (uint32_t& addr, uint16_t& port)
{
	const std::string& host = hostname.text();

	const uint32_t opengames_size = opengames.size();
	for (uint32_t i = 0; i < opengames_size; ++i) {
		const LAN_Open_Game & game = *opengames[i];

		if (not strcmp(game.info.hostname, host.c_str())) {
			addr = game.address;
			port = game.port;
			return true;
		}
	}

	hostent* he=gethostbyname(host.c_str());
	if (he==0)
		return false;

	addr = (reinterpret_cast<in_addr *>(he->h_addr_list[0]))->s_addr;
	port=htons(WIDELANDS_PORT);

	return true;
}

const std::string& Fullscreen_Menu_NetSetup::get_playername()
{
	return playername.text();
}


void Fullscreen_Menu_NetSetup::game_selected (uint32_t) {
	if (opengames.has_selection()) {
		if (const LAN_Open_Game * const game = opengames.get_selected()) {
			hostname.setText(game->info.hostname);
			joingame.set_enabled(true);
		}
	}
}

void Fullscreen_Menu_NetSetup::game_doubleclicked (uint32_t) {
	clicked_joingame();
}

void Fullscreen_Menu_NetSetup::update_game_info
	(UI::Table<LAN_Open_Game const * const>::Entry_Record & er,
	 LAN_Game_Info const & info)
{
	er.set_string (0, info.hostname);
	er.set_string (1, info.map);

	switch (info.state) {
	case LAN_GAME_OPEN:   er.set_string(2, _("Open"));   break;
	case LAN_GAME_CLOSED: er.set_string(2, _("Closed")); break;
	default:              er.set_string(2, _("Unknown"));
	};
}

void Fullscreen_Menu_NetSetup::game_opened (const LAN_Open_Game * game)
{update_game_info(opengames.add(game), game->info);}

void Fullscreen_Menu_NetSetup::game_closed (const LAN_Open_Game *) {}

void Fullscreen_Menu_NetSetup::game_updated (const LAN_Open_Game * game)
{
	if
		(UI::Table<const LAN_Open_Game * const>::Entry_Record * const er =
		 opengames.find(game))
		update_game_info(*er, game->info);
}

void Fullscreen_Menu_NetSetup::discovery_callback (int32_t type, const LAN_Open_Game* game, void* userdata)
{
	switch (type) {
	case LAN_Game_Finder::GameOpened:
		static_cast<Fullscreen_Menu_NetSetup *>(userdata)->game_opened (game);
		break;
	case LAN_Game_Finder::GameClosed:
		static_cast<Fullscreen_Menu_NetSetup *>(userdata)->game_closed (game);
		break;
	case LAN_Game_Finder::GameUpdated:
		static_cast<Fullscreen_Menu_NetSetup *>(userdata)->game_updated(game);
		break;
	default:
		abort ();
	}
}

void Fullscreen_Menu_NetSetup::change_hostname()
{
	// Allow user to enter a hostname manually
	opengames.select(opengames.no_selection_index());
	joingame.set_enabled(hostname.text().size());
}

void Fullscreen_Menu_NetSetup::change_playername()
{
	Section *s = g_options.pull_section("global");
	s->set_string("nickname", playername.text());
}

void Fullscreen_Menu_NetSetup::clicked_joingame() {
	// Save selected host so users can reload it for reconnection.
	Section *s = g_options.pull_section("global");
	s->set_string("lasthost", hostname.text());

	end_modal(JOINGAME);
}

void Fullscreen_Menu_NetSetup::clicked_hostgame() {
	end_modal(HOSTGAME);
}

void Fullscreen_Menu_NetSetup::clicked_lasthost() {
	Section *s = g_options.get_section("global");
	std::string host = s->get_string("lasthost", "");
	hostname.setText(host);
	if (host != "")
		joingame.set_enabled(hostname.text().size());
	opengames.select(opengames.no_selection_index());
}

