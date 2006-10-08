/*
 * Copyright (C) 2004 by the Widelands Development Team
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
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "ui_table.h"
#include "constants.h"
#include "i18n.h"
#include "network.h"
#include "network_lan_promotion.h"
#include "network_ggz.h"
#include "profile.h"

Fullscreen_Menu_NetSetup::Fullscreen_Menu_NetSetup ()
	:Fullscreen_Menu_Base("singleplmenu.jpg") // change this
{
	discovery=new LAN_Game_Finder();
	discovery->set_callback (discovery_callback, this);

	// Text
	UITextarea* title= new UITextarea(this, MENU_XRES/2, 120, _("Begin Network Game"), Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	// UIButtons
	UIButton* b;

	b = new UIButton(this, 90, 220, 200, 26, 1, JOINGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::joingame);
	b->set_title(_("Join a Game").c_str());

	b = new UIButton(this, 90, 260, 200, 26, 1, HOSTGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::hostgame);
	b->set_title(_("Host a New Game").c_str());

	b = new UIButton(this, 90, 300, 200, 26, 1, INTERNETGAME);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title(_("Play in Internet").c_str());

	b = new UIButton(this, 90, 340, 200, 26, 0, CANCEL);
	b->clickedid.set(this, &Fullscreen_Menu_NetSetup::end_modal);
	b->set_title(_("Back").c_str());

	// Hostname
	hostname=new UIEdit_Box(this, 310, 220, 200, 26, 2, 0);
	hostname->changed.set(this, &Fullscreen_Menu_NetSetup::toggle_hostname);
	hostname->set_text("localhost");

	// Player
	playername=new UIEdit_Box(this, 310, 260, 200, 26, 2, 0);
	playername->set_text(_("nobody").c_str());

	// LAN or GGZ game
	networktype = new UIButton(this, 550, 220, 150, 26, 0, -1);
	networktype->clickedid.set(this, &Fullscreen_Menu_NetSetup::toggle_networktype);
	networktype->set_title(_("LAN games").c_str());
	internetgame = false;

	// List of open games in local network
	opengames=new UITable(this, 310, 300, 390, 180);
	opengames->add_column (_("Host").c_str(), UITable::STRING, 150);
	opengames->add_column (_("Map").c_str(), UITable::STRING, 150);
	opengames->add_column (_("State").c_str(), UITable::STRING, 90);
	opengames->selected.set (this, &Fullscreen_Menu_NetSetup::game_selected);
}

Fullscreen_Menu_NetSetup::~Fullscreen_Menu_NetSetup ()
{
	delete discovery;
}

void Fullscreen_Menu_NetSetup::think ()
{
	Fullscreen_Menu_Base::think ();

	if(!NetGGZ::ref()->usedcore())
	{
		discovery->run ();
	}
}

bool Fullscreen_Menu_NetSetup::get_host_address (ulong& addr, ushort& port)
{
	const char* host=hostname->get_text();

	int i;
	for (i=0;i<opengames->get_nr_entries();i++) {
	    LAN_Open_Game* game=(LAN_Open_Game*) (opengames->get_entry(i)->get_user_data());

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
	LAN_Open_Game* game=(LAN_Open_Game*) (opengames->get_selection());

	if(game) hostname->set_text (game->info.hostname);
}

void Fullscreen_Menu_NetSetup::update_game_info (UITable_Entry* entry, const LAN_Game_Info& info)
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
	update_game_info (new UITable_Entry(opengames, (void*) game), game->info);
}

void Fullscreen_Menu_NetSetup::game_closed (const LAN_Open_Game *) {}

void Fullscreen_Menu_NetSetup::game_updated (const LAN_Open_Game* game)
{
	UITable_Entry* entry=opengames->find_entry(game);

	if (entry!=0)
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
		update_game_info (new UITable_Entry(opengames, (void*) NULL), info);
	}
}

void Fullscreen_Menu_NetSetup::toggle_networktype(int) {
	if(internetgame)
	{
		NetGGZ::ref()->deinitcore();
	}

	internetgame = !internetgame;

	opengames->clear();

	if(internetgame)
	{
		Section *s;
		const char *defaultserver;
		s = g_options.pull_section("network");
		defaultserver = s->get_string("defaultserver", "live.ggzgamingzone.org");
		hostname->set_text(defaultserver);

		NetGGZ::ref()->initcore(hostname->get_text(), playername->get_text());
		networktype->set_title(_("GGZ games").c_str());
		if(NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
	}
	else
	{
		hostname->set_text("localhost");
		discovery->reset();
		networktype->set_title(_("LAN games").c_str());
	}
}

void Fullscreen_Menu_NetSetup::toggle_hostname()
{
	if(internetgame)
	{
		Section *s;
		s = g_options.pull_section("network");
		s->set_string("defaultserver", hostname->get_text());
		g_options.write("config", true);

		NetGGZ::ref()->deinitcore();
		NetGGZ::ref()->initcore(hostname->get_text(), playername->get_text());
		networktype->set_title(_("GGZ games").c_str());
		if(NetGGZ::ref()->tables().size() > 0) fill(NetGGZ::ref()->tables());
	}
}

//bool Fullscreen_Menu_NetSetup::is_internetgame()
//{
//	return internetgame;
//}

void Fullscreen_Menu_NetSetup::joingame(int) {
	int index = opengames->get_selection_index();
	if(index < 0) return;

	if(NetGGZ::ref()->usedcore())
	{
		UITable_Entry *entry = opengames->get_entry(index);
		if(!entry) return;
		NetGGZ::ref()->join(entry->get_string(1));
		end_modal(JOINGGZGAME);
	}
	else end_modal(JOINGAME);
}

void Fullscreen_Menu_NetSetup::hostgame(int) {
	if(NetGGZ::ref()->usedcore()) end_modal(HOSTGGZGAME);
	else end_modal(HOSTGAME);
}
