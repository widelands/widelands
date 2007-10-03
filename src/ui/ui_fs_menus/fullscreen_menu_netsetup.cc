/*
 * Copyright (C) 2004, 2006-2007 by the Widelands Development Team
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
#include "i18n.h"
#include "network.h"
#include "network_ggz.h"
#include "profile.h"

Fullscreen_Menu_NetSetup::Fullscreen_Menu_NetSetup ()
:
Fullscreen_Menu_Base("singleplmenu.jpg"), // change this

	// Text
title(this, MENU_XRES/2, 120, _("Begin Network Game"), Align_HCenter),

joingame
(this,
 90, 220, 200, 26,
 1,
 &Fullscreen_Menu_NetSetup::clicked_joingame, this,
 _("Join a Game"), std::string(), false),

hostgame
(this,
 90, 260, 200, 26,
 1,
 &Fullscreen_Menu_NetSetup::clicked_hostgame, this,
 _("Host a New Game")),

// Internetgaming via game_server protocol does not work at the moment.
// So I took the button out of the menu, to avoid crashes and confusion
// for the players --Nasenbaer
/*
playinternet
(this,
 90, 300, 200, 26,
 1,
 &Fullscreen_Menu_NetSetup::end_modal, this, INTERNETGAME,
 _("Play in Internet")),
 */

back
(this,
 90, 340, 200, 26,
 0,
 &Fullscreen_Menu_NetSetup::end_modal, this, CANCEL,
 _("Back")),

	// Hostname
hostname(this, 310, 220, 200, 26, 2, 0),

	// Player
playername(this, 310, 260, 200, 26, 2, 0),

	// List of open games in local network
opengames(this, 310, 300, 390, 180),

// LAN or GGZ game
networktype
(this,
 550, 220, 150, 26,
 0,
 &Fullscreen_Menu_NetSetup::toggle_networktype, this,
 _("LAN games")),

internetgame(false)

{
	title     .set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	hostname  .changed.set(this, &Fullscreen_Menu_NetSetup::toggle_hostname);
	hostname  .set_text("localhost");
	playername.set_text(_("nobody").c_str());
	opengames .add_column(_("Host"), 150);
	opengames .add_column(_("Map"),  150);
	opengames .add_column(_("State"), 90);
	opengames .selected.set (this, &Fullscreen_Menu_NetSetup::game_selected);
	discovery .set_callback (discovery_callback, this);
}

void Fullscreen_Menu_NetSetup::think ()
{
	Fullscreen_Menu_Base::think ();

	if (not NetGGZ::ref()->usedcore()) discovery.run();
}

bool Fullscreen_Menu_NetSetup::get_host_address (uint32_t& addr, uint16_t& port)
{
	const char * const host = hostname.get_text();

	const uint32_t opengames_size = opengames.size();
	for (uint32_t i = 0; i < opengames_size; ++i) {
		const LAN_Open_Game & game = *opengames[i];

		if (not strcmp(game.info.hostname, host)) {
			addr = game.address;
			port = game.port;
		return true;
		}
	}

	hostent* he=gethostbyname(host);
	if (he==0)
	    return false;

	addr=((in_addr*) (he->h_addr_list[0]))->s_addr;
	port=htons(WIDELANDS_PORT);

	return true;
}

void Fullscreen_Menu_NetSetup::game_selected (uint32_t) {
	if (const LAN_Open_Game * const game = opengames.get_selected())
		hostname.set_text(game->info.hostname);
		joingame.set_enabled(true);
}

void Fullscreen_Menu_NetSetup::update_game_info
(UI::Table<const LAN_Open_Game * const>::Entry_Record & er,
 const LAN_Game_Info & info)
{
	er.set_string (0, info.hostname);
	er.set_string (1, info.map);

	switch (info.state) {
	case LAN_GAME_OPEN:   er.set_string(2, _("Open"));   break;
	case LAN_GAME_CLOSED: er.set_string(2, _("Closed")); break;
	default:              er.set_string(2, _("Unknown"));
	}
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
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_opened (game);
		break;
	case LAN_Game_Finder::GameClosed:
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_closed (game);
		break;
	case LAN_Game_Finder::GameUpdated:
	    static_cast<Fullscreen_Menu_NetSetup*>(userdata)->game_updated (game);
		break;
	default:
	    abort ();
	}
}

void Fullscreen_Menu_NetSetup::fill(std::list<std::string> tables)
{
	LAN_Game_Info info;
	std::list<std::string>::iterator it;
	for (it = tables.begin(); it != tables.end(); it++)
	{
		strncpy(info.hostname, "(ggz)", sizeof(info.hostname));
		strncpy(info.map, (*it).c_str(), sizeof(info.map));
		info.state = LAN_GAME_OPEN;
		update_game_info(opengames.add(), info);
	}
}

void Fullscreen_Menu_NetSetup::toggle_networktype() {
	if (internetgame)
	{
		NetGGZ::ref()->deinitcore();
	}

	internetgame = !internetgame;

	opengames.clear();

	if (internetgame)
	{
		Section *s;
		const char *defaultserver;
		s = g_options.pull_section("network");
		defaultserver = s->get_string("defaultserver", "live.ggzgamingzone.org");
		hostname.set_text(defaultserver);

		NetGGZ::ref()->initcore(hostname.get_text(), playername.get_text());
		networktype.set_title(_("GGZ games").c_str());
		if (NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
	}
	else
	{
		hostname.set_text("localhost");
		discovery.reset();
		networktype.set_title(_("LAN games").c_str());
	}
}

void Fullscreen_Menu_NetSetup::toggle_hostname()
{
	if (internetgame)
	{
		Section *s;
		s = g_options.pull_section("network");
		s->set_string("defaultserver", hostname.get_text());
		g_options.write("config", true);

		NetGGZ::ref()->deinitcore();
		NetGGZ::ref()->initcore(hostname.get_text(), playername.get_text());
		networktype.set_title(_("GGZ games").c_str());
		if (NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
	}
}

//bool Fullscreen_Menu_NetSetup::is_internetgame() {return internetgame;}

void Fullscreen_Menu_NetSetup::clicked_joingame() {
	if (NetGGZ::ref()->usedcore())
	{
		NetGGZ::ref()->join
			(opengames.get_selected_record().get_string(1).c_str());
		end_modal(JOINGGZGAME);
	}
	else end_modal(JOINGAME);
}

void Fullscreen_Menu_NetSetup::clicked_hostgame() {
	if (NetGGZ::ref()->usedcore()) end_modal(HOSTGGZGAME);
	else end_modal(HOSTGAME);
}
