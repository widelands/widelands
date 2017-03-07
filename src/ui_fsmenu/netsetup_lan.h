/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_NETSETUP_LAN_H
#define WL_UI_FSMENU_NETSETUP_LAN_H

#include <cstring>
#include <list>
#include <string>

#include "network/network_lan_promotion.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/load_map_or_game.h"

struct NetOpenGame;
struct NetGameInfo;

class FullscreenMenuNetSetupLAN : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuNetSetupLAN();

	void think() override;

	/**
	 * \param[out] addr filled in with the IP address of the chosen server
	 * \param[out] port filled in with the port of the chosen server
	 * \return \c true if a valid server has been chosen. If \c false is
	 * returned, the values of \p addr and \p port are undefined.
	 */
	bool get_host_address(uint32_t& addr, uint16_t& port);

	/**
	 * \return the name chosen by the player
	 */
	const std::string& get_playername();

protected:
	void clicked_ok() override;

private:
	void layout() override;

	void game_selected(uint32_t);
	void game_doubleclicked(uint32_t);

	static void discovery_callback(int32_t, NetOpenGame const*, void*);

	void game_opened(NetOpenGame const*);
	void game_closed(NetOpenGame const*);
	void game_updated(NetOpenGame const*);

	void update_game_info(UI::Table<const NetOpenGame* const>::EntryRecord&, const NetGameInfo&);

	void change_hostname();
	void change_playername();
	void clicked_joingame();
	void clicked_hostgame();
	void clicked_lasthost();


	const int labelh_;

	UI::Textarea title_;

	UI::Box left_column_, right_column_;

	// Left Column
	UI::Textarea label_opengames_;
	UI::Table<const NetOpenGame* const> table_;

	// Right Column
	UI::Textarea label_playername_;
	UI::EditBox playername_;
	UI::Textarea label_hostname_;
	UI::Box host_box_;
	UI::EditBox hostname_;
	UI::Button loadlasthost_;

	UI::Button joingame_, hostgame_;

	LanGameFinder discovery_;
};

#endif  // end of include guard: WL_UI_FSMENU_NETSETUP_LAN_H
