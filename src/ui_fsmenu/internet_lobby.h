/*
 * Copyright (C) 2004, 2006-2009, 2011-2012 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_INTERNET_LOBBY_H
#define FULLSCREEN_MENU_INTERNET_LOBBY_H

#include <cstring>
#include <string>
#include <vector>

#include "ui_fsmenu/base.h"
#include "network/internet_gaming.h"
#include "network/network_lan_promotion.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "wui/gamechatpanel.h"

struct Fullscreen_Menu_Internet_Lobby : public Fullscreen_Menu_Base {

	Fullscreen_Menu_Internet_Lobby (const char *, const char *, bool);

	virtual void think() override;

	/// \returns the maximum number of clients that may connect
	int32_t get_maxclients() {
		return maxclients.getValue();
	}

private:
	uint32_t m_butx;
	uint32_t m_butw;
	uint32_t m_buth;
	uint32_t m_lisw;
	uint32_t m_fs;
	uint32_t m_prev_clientlist_len;
	std::string m_fn;
	UI::Textarea title, m_clients, m_opengames;
	UI::Textarea m_servername;
	UI::Textarea m_maxclients;
	UI::SpinBox maxclients;
	UI::Button joingame, hostgame, back;
	UI::EditBox servername;
	UI::Table<const INet_Client * const> clientsonline;
	UI::Listselect<INet_Game> opengames;
	GameChatPanel chat;

	// Login information
	const char * nickname;
	const char * password;
	bool         reg;

	void fillGamesList (const std::vector<INet_Game> &);
	void fillClientList(const std::vector<INet_Client> &);

	void connectToMetaserver();

	void client_doubleclicked (uint32_t);
	void server_selected (uint32_t);
	void server_doubleclicked (uint32_t);

	void change_servername();
	void clicked_joingame();
	void clicked_hostgame();
	void clicked_back();

	uint8_t convert_clienttype(const std::string &);
	bool compare_clienttype(unsigned int rowa, unsigned int rowb);
};

#endif
