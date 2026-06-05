/*
 * Copyright (C) 2020-2026 by the Widelands Development Team
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

#include "graphic/styles/button_style.h"

namespace UI {

PanelStyle to_panel_style(const ButtonStyle s) {
	switch (s) {
	case ButtonStyle::kFsMenuMenu:
	case ButtonStyle::kFsMenuPrimary:
	case ButtonStyle::kFsMenuSecondary:
		return PanelStyle::kFsMenu;
	default:
		return PanelStyle::kWui;
	}
}

ButtonStyle to_button_style_primary(const PanelStyle style) {
	return style == PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuPrimary :
	                                      UI::ButtonStyle::kWuiPrimary;
}

ButtonStyle to_button_style_secondary(const PanelStyle style) {
	return style == PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuSecondary :
	                                      UI::ButtonStyle::kWuiSecondary;
}

ButtonStyle to_button_style_menu(const PanelStyle style) {
	return style == PanelStyle::kFsMenu ? UI::ButtonStyle::kFsMenuMenu : UI::ButtonStyle::kWuiMenu;
}

}  // namespace UI
