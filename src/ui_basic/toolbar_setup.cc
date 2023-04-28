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

#include "ui_basic/toolbar_setup.h"

#include "graphic/style_manager.h"
#include "wlapplication_options.h"

namespace UI {

int main_toolbar_button_size() {
	return g_style_manager->styled_size(StyledSize::kToolbarButtonSize);
}

bool main_toolbar_at_bottom() {
	const int mode = get_config_int("toolbar_pos", 0);
	return (mode & ToolbarDisplayMode::kCmdSwap) == 0;
}

}  // namespace UI
