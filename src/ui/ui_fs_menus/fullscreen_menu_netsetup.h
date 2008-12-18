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

#ifndef FULLSCREEN_MENU_NETSETUP_H
#define FULLSCREEN_MENU_NETSETUP_H

#include "network_lan_promotion.h"

#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "ui_table.h"

#include <list>
#include <string>
#include <cstring>

#include "fullscreen_menu_base.h"

class LAN_Open_Game;
struct LAN_Game_Info;

struct Fullscreen_Menu_NetSetup : public Fullscreen_Menu_Base {
	enum {
		CANCEL = 0,
		HOSTGAME,
		JOINGAME
	};

	Fullscreen_Menu_NetSetup ();

	virtual void think();

	/**
	 * \param[out] addr filled in with the IP address of the chosen server
	 * \param[out] port filled in with the port of the chosen server
	 * \return \c true if a valid server has been chosen. If \c false is
	 * returned, the values of \p addr and \p port are undefined.
	 */
	bool get_host_address (uint32_t& addr, uint16_t& port);

	/**
	 * \return the name chosen by the player
	 */
	const std::string& get_playername();

private:
	uint32_t                                    m_butx;
	uint32_t                                    m_butw;
	uint32_t                                    m_buth;
	uint32_t                                    m_lisw;
	uint32_t                                    m_fs;
	std::string                                 m_fn;
	UI::Textarea                                title, m_opengames;
	UI::Textarea                                m_playername, m_hostname;
	UI::Button<Fullscreen_Menu_NetSetup>        joingame;
	UI::Button<Fullscreen_Menu_NetSetup>        hostgame;
	//UI::IDButton<Fullscreen_Menu_NetSetup, int32_t> playinternet;
	UI::IDButton<Fullscreen_Menu_NetSetup, int32_t> back;
	UI::Button<Fullscreen_Menu_NetSetup>        loadlasthost;
	UI::EditBox                                 playername;
	UI::EditBox                                 hostname;
	UI::Table<const LAN_Open_Game * const>      opengames;
	LAN_Game_Finder                             discovery;

	void game_selected (uint32_t);
	void game_doubleclicked (uint32_t);

	static void discovery_callback (int32_t, LAN_Open_Game const *, void *);

	void game_opened  (LAN_Open_Game const *);
	void game_closed  (LAN_Open_Game const *);
	void game_updated (LAN_Open_Game const *);

	void update_game_info
		(UI::Table<const LAN_Open_Game * const>::Entry_Record &,
		 const LAN_Game_Info &);

	void change_hostname();
	void change_playername();
	void clicked_joingame();
	void clicked_hostgame();
	void clicked_lasthost();
};

#endif
