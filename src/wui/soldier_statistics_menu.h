/*
 * Copyright (C) 2020-2021 by the Widelands Development Team
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

#ifndef WL_WUI_SOLDIER_STATISTICS_MENU_H
#define WL_WUI_SOLDIER_STATISTICS_MENU_H

#include "ui_basic/box.h"
#include "ui_basic/icon.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_player.h"

struct SoldierStatisticsPanel : public UI::Box {
	using CountingFn = std::function<uint32_t(uint32_t, uint32_t, uint32_t, uint32_t)>;
	SoldierStatisticsPanel(UI::Panel&, const Widelands::Player&, const CountingFn&);
	~SoldierStatisticsPanel() override = default;

	void think() override;

private:
	void update();

	CountingFn counting_function_;

	unsigned max_attack_, max_defense_, max_health_, max_evade_;

	std::vector<UI::Icon*> icons_all_;
	std::vector<UI::Textarea*> labels_all_;
};

struct SoldierStatisticsMenu : public UI::UniqueWindow {
	SoldierStatisticsMenu(InteractivePlayer&, UI::UniqueWindow::Registry&);
	~SoldierStatisticsMenu() override = default;

	void think() override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kSoldierStats;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	void update();

	const Widelands::Player& player_;

	unsigned max_attack_, max_defense_, max_health_, max_evade_;

	UI::TabPanel tabs_;
	std::vector<UI::Icon*> icons_detail_;
	std::vector<UI::Textarea*> labels_detail_;
};

#endif  // end of include guard: WL_WUI_SOLDIER_STATISTICS_MENU_H
