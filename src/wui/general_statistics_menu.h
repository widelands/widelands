/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_WUI_GENERAL_STATISTICS_MENU_H
#define WL_WUI_GENERAL_STATISTICS_MENU_H

#include <memory>

#include "graphic/playercolor.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/unique_window.h"
#include "wui/plot_area.h"

class InteractiveGameBase;

struct GeneralStatisticsMenu : public UI::UniqueWindow {

	// Custom registry, to store the selected_information as well.
	struct Registry : public UI::UniqueWindow::Registry {
		Registry()
		   : UI::UniqueWindow::Registry(),
		     selected_information(0),
		     selected_players(true, kMaxPlayers),
		     time(WuiPlotArea::TIME_GAME) {
		}

		int32_t selected_information;
		std::vector<bool> selected_players;
		WuiPlotArea::TIME time;
	};

	GeneralStatisticsMenu(InteractiveGameBase&, Registry&);
	~GeneralStatisticsMenu() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kGeneralStats;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	Registry* my_registry_;
	UI::Box box_, player_buttons_box_;
	WuiPlotArea plot_;
	UI::Radiogroup radiogroup_;
	int32_t selected_information_;
	UI::Button* cbs_[kMaxPlayers];
	WuiPlotAreaSlider* slider_;
	uint32_t ndatasets_;
	Widelands::Game& game_;

	void create_player_buttons();
	std::unique_ptr<Notifications::Subscriber<Widelands::NotePlayerDetailsEvent>> subscriber_;

	void cb_changed_to(int32_t);
	void radiogroup_changed(int32_t);
	void show_or_hide_plot(int32_t id, bool show);
	void save_state_to_registry();
};

#endif  // end of include guard: WL_WUI_GENERAL_STATISTICS_MENU_H
