/*
 * Copyright (C) 2004, 2006 by the Widelands Development Team
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
 _("Join a Game")),

hostgame
(this,
 90, 260, 200, 26,
 1,
 &Fullscreen_Menu_NetSetup::clicked_hostgame, this,
 _("Host a New Game")),

playinternet
(this,
 90, 300, 200, 26,
 1,
 &Fullscreen_Menu_NetSetup::end_modal, this, INTERNETGAME,
 _("Play in Internet")),

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
	opengames .add_column(_("Host").c_str(), UI::Table::STRING, 150);
	opengames .add_column(_("Map").c_str(), UI::Table::STRING, 150);
	opengames .add_column(_("State").c_str(), UI::Table::STRING, 90);
	opengames .selected.set (this, &Fullscreen_Menu_NetSetup::game_selected);
	discovery .set_callback (discovery_callback, this);
}

void Fullscreen_Menu_NetSetup::think ()
{
	Fullscreen_Menu_Base::think ();

	if (not NetGGZ::ref()->usedcore()) discovery.run();
}

bool Fullscreen_Menu_NetSetup::get_host_address (ulong& addr, ushort& port)
{
	const char * const host = hostname.get_text();

	for (int i = 0; i <opengames.get_nr_entries(); ++i) {
		const LAN_Open_Game * game = static_cast<const LAN_Open_Game * const>
			(opengames.get_entry(i)->get_user_data());

	    if (!strcmp(game->info.hostname, host)) {
		addr=game->address;
		port=game->port;
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

void Fullscreen_Menu_NetSetup::game_selected (int) {
	if
		(const LAN_Open_Game * const game =
		 static_cast<const LAN_Open_Game * const>(opengames.get_selection()))
		hostname.set_text(game->info.hostname);
}

void Fullscreen_Menu_NetSetup::update_game_info (UI::Table_Entry* entry, const LAN_Game_Info& info)
{
	entry->set_string (0, info.hostname);
	entry->set_string (1, info.map);

	switch (info.state) {
	    case LAN_GAME_OPEN:
		    entry->set_string (2, _("Open").c_str());
		break;
	    case LAN_GAME_CLOSED:
		    entry->set_string (2, _("Closed").c_str());
		break;
	    default:
		    entry->set_string (2, _("Unknown").c_str());
		break;
	}
}

void Fullscreen_Menu_NetSetup::game_opened (const LAN_Open_Game* game)
{
	update_game_info
		(new UI::Table_Entry
		 (&opengames,
		  const_cast<LAN_Open_Game*>(game)),
		 game->info);
}

void Fullscreen_Menu_NetSetup::game_closed (const LAN_Open_Game *) {}

void Fullscreen_Menu_NetSetup::game_updated (const LAN_Open_Game* game)
{
	if (UI::Table_Entry * const entry = opengames.find_entry(game))
	    update_game_info (entry, game->info);
}

void Fullscreen_Menu_NetSetup::discovery_callback (int type, const LAN_Open_Game* game, void* userdata)
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
	for(it = tables.begin(); it != tables.end(); it++)
	{
		strncpy(info.hostname, "(ggz)", sizeof(info.hostname));
		strncpy(info.map, (*it).c_str(), sizeof(info.map));
		info.state = LAN_GAME_OPEN;
		update_game_info(new UI::Table_Entry(&opengames, (void*) NULL), info);
	}
}

void Fullscreen_Menu_NetSetup::toggle_networktype() {
	if(internetgame)
	{
		NetGGZ::ref()->deinitcore();
	}

	internetgame = !internetgame;

	opengames.clear();

	if(internetgame)
	{
		Section *s;
		const char *defaultserver;
		s = g_options.pull_section("network");
		defaultserver = s->get_string("defaultserver", "live.ggzgamingzone.org");
		hostname.set_text(defaultserver);

		NetGGZ::ref()->initcore(hostname.get_text(), playername.get_text());
		networktype.set_title(_("GGZ games").c_str());
		if(NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
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
	if(internetgame)
	{
		Section *s;
		s = g_options.pull_section("network");
		s->set_string("defaultserver", hostname.get_text());
		g_options.write("config", true);

		NetGGZ::ref()->deinitcore();
		NetGGZ::ref()->initcore(hostname.get_text(), playername.get_text());
		networktype.set_title(_("GGZ games").c_str());
		if(NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
	}
}

//bool Fullscreen_Menu_NetSetup::is_internetgame()
//{
//	return internetgame;
//}

void Fullscreen_Menu_NetSetup::clicked_joingame() {
	const int index = opengames.get_selection_index();
	if(index < 0) return;

	if(NetGGZ::ref()->usedcore())
	{
		UI::Table_Entry *entry = opengames.get_entry(index);
		if(!entry) return;
		NetGGZ::ref()->join(entry->get_string(1));
		end_modal(JOINGGZGAME);
	}
	else end_modal(JOINGAME);
}

void Fullscreen_Menu_NetSetup::clicked_hostgame() {
	if(NetGGZ::ref()->usedcore()) end_modal(HOSTGGZGAME);
	else end_modal(HOSTGAME);
}
