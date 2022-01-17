/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#include <memory>

#include "logic/game_controller.h"
#include "network/internet_gaming.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/menu.h"
#include "wui/game_chat_panel.h"

namespace Widelands {
struct TribeBasicInfo;
}
namespace FsMenu {
class InternetLobby : public TwoColumnsBasicNavigationMenu {
public:
	InternetLobby(MenuCapsule&,
	              std::string&,
	              std::string&,
	              bool,
	              const std::vector<Widelands::TribeBasicInfo>& tribeinfos);
	~InternetLobby() override;

	void think() override;

protected:
	void clicked_ok();

private:
	void layout() override;

	void fill_games_list(const std::vector<InternetGame>*);
	void fill_client_list(const std::vector<InternetClient>*);

	void connect_to_metaserver();

	void client_doubleclicked(uint32_t);
	void server_selected();
	void server_doubleclicked();

	void change_servername();
	bool wait_for_ip();
	void clicked_joingame();
	void clicked_hostgame();

	uint8_t convert_clienttype(const std::string&);
	bool compare_clienttype(unsigned int rowa, unsigned int rowb);

	std::shared_ptr<GameController> running_game_;

	// Left Column
	UI::Textarea label_clients_online_;
	UI::Table<const InternetClient* const> clientsonline_table_;
	GameChatPanel chat_;

	// Right Column
	UI::Textarea label_opengames_;
	UI::Listselect<InternetGame> opengames_list_;
	UI::Button joingame_;
	UI::Textarea servername_label_;
	UI::EditBox servername_;
	UI::Button hostgame_;

	uint32_t prev_clientlist_len_;
	FxId new_client_fx_;

	// Login information
	const std::string nickname_;
	const std::string password_;
	bool is_registered_;

	// Tribes check
	std::vector<Widelands::TribeBasicInfo> tribeinfos_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_INTERNET_LOBBY_H
