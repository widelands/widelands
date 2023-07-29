/*
 * Copyright (C) 2004-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_FSMENU_NETSETUP_LAN_H
#define WL_UI_FSMENU_NETSETUP_LAN_H

#include <memory>

#include "logic/game_controller.h"
#include "network/network_lan_promotion.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "ui_fsmenu/menu.h"
namespace FsMenu {
class NetSetupLAN : public TwoColumnsBasicNavigationMenu {
public:
	explicit NetSetupLAN(MenuCapsule&);

	void think() override;

	/**
	 * \param[out] addr filled in with the host name or IP address and port of the chosen server
	 * \return \c True if the address is valid, \c false otherwise. In that case \c addr is not
	 * modified
	 */
	bool get_host_address(NetAddress* addr);

protected:
	void clicked_ok();

private:
	void layout() override;

	void game_selected(uint32_t index);
	void game_doubleclicked(uint32_t index);

	static void discovery_callback(int32_t, NetOpenGame const*, void*);

	void game_opened(NetOpenGame const* game);
	void game_closed(NetOpenGame const* game);
	void game_updated(NetOpenGame const* game);

	void update_game_info(UI::Table<const NetOpenGame* const>::EntryRecord&, const NetGameInfo&);

	void change_hostname();
	void change_playername();
	void clicked_joingame();
	void clicked_hostgame();
	void clicked_lasthost();

	std::shared_ptr<GameController> running_game_;

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
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_NETSETUP_LAN_H
