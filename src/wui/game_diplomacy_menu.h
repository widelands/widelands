/*
 * Copyright (C) 2022-2025 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_DIPLOMACY_MENU_H
#define WL_WUI_GAME_DIPLOMACY_MENU_H

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class InteractiveBase;

///  Shows the current teams lineup and allows the player to perform diplomatic actions.
class GameDiplomacyMenu : public UI::UniqueWindow {
public:
	GameDiplomacyMenu(InteractiveGameBase& parent, UI::UniqueWindow::Registry&);
	void think() override;
	void draw(RenderTarget&) override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kDiplomacy;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	InteractiveGameBase& igbase_;
	InteractivePlayer* iplayer_;

	void update(bool always);
	void update_diplomacy_details();
	void update_trades_offers(bool always);
	void update_trades_proposed(bool always);
	void update_trades_active(bool always);

	UI::Box diplomacy_box_;

	UI::Box hbox_, vbox_flag_, vbox_name_, vbox_team_, vbox_status_, vbox_action_;
	UI::Box actions_hbox_, actions_vbox_descr_, actions_vbox_yes_, actions_vbox_no_;
	std::map<Widelands::PlayerNumber, UI::Icon*> diplomacy_teams_;
	std::map<Widelands::PlayerNumber, UI::Textarea*> diplomacy_status_;
	std::map<Widelands::PlayerNumber, std::pair<UI::Button*, UI::Button*>> diplomacy_buttons_;
	std::list<Widelands::Game::PendingDiplomacyAction> cached_diplomacy_actions_;

	UI::TabPanel trades_tabs_;
	UI::Box trades_box_offers_;
	UI::Box trades_box_proposed_;
	UI::Box trades_box_active_;
	std::vector<Widelands::TradeID> cached_trades_offers_;
	std::vector<Widelands::TradeID> cached_trades_proposed_;
	std::vector<Widelands::TradeID> cached_trades_active_;
};

#endif  // end of include guard: WL_WUI_GAME_DIPLOMACY_MENU_H
