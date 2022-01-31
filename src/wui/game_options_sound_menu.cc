/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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
 */

#include "wui/game_options_sound_menu.h"

#include "base/i18n.h"
#include "sound/sound_handler.h"

namespace {
constexpr int kMargin = 12;
}  // namespace

GameOptionsSoundMenu::GameOptionsSoundMenu(Panel& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "sound_options_menu",
                      &registry,
                      100,
                      100,
                      _("Sound Options")),
     sound_options_(*this, UI::SliderStyle::kWuiLight) {
	sound_options_.set_border(kMargin, kMargin, kMargin, kMargin);

	set_center_panel(&sound_options_);

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

GameOptionsSoundMenu::~GameOptionsSoundMenu() {
	g_sh->save_config();
}
