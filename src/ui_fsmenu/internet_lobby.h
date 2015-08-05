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

#ifndef WL_UI_FSMENU_INTERNET_LOBBY_H
#define WL_UI_FSMENU_INTERNET_LOBBY_H

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

class FullscreenMenuInternetLobby : public FullscreenMenuBase {
public:
	FullscreenMenuInternetLobby (const char *, const char *, bool);

	void think() override;

protected:
	void clicked_ok() override;

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
	UI::Button joingame, hostgame, back;
	UI::EditBox servername;
	UI::Table<const InternetClient * const> clientsonline;
	UI::Listselect<InternetGame> opengames;
	GameChatPanel chat;

	// Login information
	const char * nickname;
	const char * password;
	bool         reg;

	void fill_games_list (const std::vector<InternetGame> &);
	void fill_client_list(const std::vector<InternetClient> &);

	void connect_to_metaserver();

	void client_doubleclicked (uint32_t);
	void server_selected();
	void server_doubleclicked();

	void change_servername();
	void clicked_joingame();
	void clicked_hostgame();

	uint8_t convert_clienttype(const std::string &);
	bool compare_clienttype(unsigned int rowa, unsigned int rowb);
};

#endif  // end of include guard: WL_UI_FSMENU_INTERNET_LOBBY_H
