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

#ifndef WL_WUI_GAME_OBJECTIVES_MENU_H
#define WL_WUI_GAME_OBJECTIVES_MENU_H

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class Objective;
}
class InteractiveBase;

///  Shows the not already fulfilled objectives.
class GameObjectivesMenu : public UI::UniqueWindow {
public:
	GameObjectivesMenu(InteractivePlayer& parent, UI::UniqueWindow::Registry&);
	void think() override;
	void draw(RenderTarget&) override;

	UI::Panel::SaveType save_type() const override {
		return UI::Panel::SaveType::kObjectives;
	}
	void save(FileWrite&, Widelands::MapObjectSaver&) const override;
	static UI::Window& load(FileRead&, InteractiveBase&);

private:
	InteractivePlayer& iplayer_;
	void selected(uint32_t);

	UI::Box objective_box_;

	using ListType = UI::Listselect<const Widelands::Objective&>;
	ListType objective_list_;
	UI::MultilineTextarea objective_text_;
};

#endif  // end of include guard: WL_WUI_GAME_OBJECTIVES_MENU_H
