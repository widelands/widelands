/*
 * Copyright (C) 2007-2024 by the Widelands Development Team
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
constexpr int kMargin = 4;
}  // namespace

GameOptionsSoundMenu::GameOptionsSoundMenu(Panel& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent,
                      UI::WindowStyle::kWui,
                      "sound_options_menu",
                      &registry,
                      100,
                      100,
                      _("Sound Options")),
     outer_(this, UI::PanelStyle::kWui, "sound_options_outer_box", 0, 0, UI::Box::Vertical),
     sound_options_(outer_, UI::SliderStyle::kWuiLight),
     music_player_(outer_) {

	outer_.add(&sound_options_, UI::Box::Resizing::kFullSize);
	outer_.add(&music_player_, UI::Box::Resizing::kFullSize);
	sound_options_.set_border(kMargin, kMargin, kMargin, kMargin);
	music_player_.set_border(kMargin, kMargin, kMargin, kMargin);

	set_center_panel(&outer_);

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

GameOptionsSoundMenu::~GameOptionsSoundMenu() {
	g_sh->save_config();
}
