/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_UI_LUA_UI_HELPER_FUNCTIONS_H
#define WL_SCRIPTING_UI_LUA_UI_HELPER_FUNCTIONS_H

#include "scripting/lua.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/listselect.h"

namespace LuaUi {

UI::PanelStyle panel_style(lua_State* L);
UI::Align get_table_align(lua_State* L,
                          const char* key,
                          bool mandatory,
                          UI::Align default_value = UI::Align::kCenter);
UI::Box::Resizing
get_table_box_resizing(lua_State* L,
                       const char* key,
                       bool mandatory,
                       UI::Box::Resizing default_value = UI::Box::Resizing::kAlign);
UI::DropdownType
get_table_dropdown_type(lua_State* L,
                        const char* key,
                        bool mandatory,
                        UI::DropdownType default_value = UI::DropdownType::kTextual);
UI::ListselectLayout
get_table_listselect_layout(lua_State* L,
                            const char* key,
                            bool mandatory,
                            UI::ListselectLayout default_value = UI::ListselectLayout::kPlain);
UI::ButtonStyle
get_table_button_style(lua_State* L,
                       const char* key,
                       bool mandatory,
                       UI::ButtonStyle default_value = UI::ButtonStyle::kWuiSecondary);
UI::SliderStyle get_slider_style(lua_State* L);
UI::TabPanelStyle get_tab_panel_style(lua_State* L);
UI::Button::VisualState get_table_button_visual_state(
   lua_State* L,
   const char* key,
   bool mandatory,
   UI::Button::VisualState default_value = UI::Button::VisualState::kRaised);
unsigned get_table_button_box_orientation(lua_State* L,
                                          const char* key,
                                          bool mandatory,
                                          unsigned default_value = UI::Box::Vertical);

template <typename... Args>
static inline void do_set_global_string(lua_State*, const char*, Args...) {
	NEVER_HERE();
}
template <>
inline void do_set_global_string<std::string>(lua_State* L, const char* name, std::string arg) {
	lua_pushstring(L, arg);
	lua_setglobal(L, name);
}

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_UI_HELPER_FUNCTIONS_H
