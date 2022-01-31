/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#ifndef WL_WUI_TRIBAL_ENCYCLOPEDIA_H
#define WL_WUI_TRIBAL_ENCYCLOPEDIA_H

#include "scripting/lua_interface.h"
#include "ui_basic/unique_window.h"
#include "wui/encyclopedia_window.h"

class InteractivePlayer;

struct TribalEncyclopedia : public UI::EncyclopediaWindow {
	TribalEncyclopedia(InteractivePlayer&, UI::UniqueWindow::Registry&, LuaInterface* const lua);
};

#endif  // end of include guard: WL_WUI_TRIBAL_ENCYCLOPEDIA_H
