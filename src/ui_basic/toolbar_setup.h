/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_TOOLBAR_SETUP_H
#define WL_UI_BASIC_TOOLBAR_SETUP_H

#include <cstdint>

namespace UI {

[[nodiscard]] int main_toolbar_button_size();

[[nodiscard]] bool main_toolbar_at_bottom();

constexpr uint8_t kMaxPopupMessages = 4;

enum ToolbarDisplayMode {
	kCmdSwap = 1,
	kPinned = 2,
	kMinimized = 4,
	kOnMouse_Visible = 8,
	kOnMouse_Hidden = 16
};

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_TOOLBAR_SETUP_H
