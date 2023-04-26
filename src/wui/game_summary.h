/*
 * Copyright (C) 2007-2023 by the Widelands Development Team
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

#ifndef WL_WUI_GAME_SUMMARY_H
#define WL_WUI_GAME_SUMMARY_H

#include <vector>

#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class InteractiveGameBase;
namespace Widelands {
class Game;
}  // namespace Widelands

/// Shows an ingame summary window on game end
class GameSummaryScreen : public UI::UniqueWindow {
public:
	GameSummaryScreen(InteractiveGameBase* parent, UI::UniqueWindow::Registry* r);

	bool handle_mousepress(uint8_t btn, int32_t mx, int32_t my) override;

private:
	void fill_data();
	void stop_clicked();
	void continue_clicked();
	void player_selected(uint32_t entry_index);
	std::string parse_player_info(const std::string& info);
	bool compare_status(uint32_t, uint32_t) const;

	Widelands::Game& game_;
	uint32_t desired_speed_;
	UI::Box* info_box_;
	UI::Textarea* title_area_;
	UI::Textarea* gametime_label_;
	UI::Textarea* gametime_value_;
	UI::Textarea* info_area_label_;
	UI::MultilineTextarea* info_area_;
	UI::Icon* widelands_icon_;
	UI::Button* continue_button_;
	UI::Button* stop_button_;
	UI::Table<uintptr_t const>* players_table_;
	std::vector<Widelands::PlayerNumber> playernumbers_;
};

#endif  // end of include guard: WL_WUI_GAME_SUMMARY_H
