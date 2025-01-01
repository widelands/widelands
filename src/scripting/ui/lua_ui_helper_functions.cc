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

#include "scripting/ui/lua_ui_helper_functions.h"

#include "scripting/globals.h"
#include "scripting/report_error.h"

namespace LuaUi {

UI::PanelStyle panel_style(lua_State* L) {
	return is_main_menu(L) ? UI::PanelStyle::kFsMenu : UI::PanelStyle::kWui;
}

UI::Align get_table_align(lua_State* L, const char* key, bool mandatory, UI::Align default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "center") {
			default_value = UI::Align::kCenter;
		} else if (str == "left" || str == "top") {
			default_value = UI::Align::kLeft;
		} else if (str == "right" || str == "bottom") {
			default_value = UI::Align::kRight;
		} else {
			report_error(L, "Unknown align '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing align: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

UI::Box::Resizing get_table_box_resizing(lua_State* L,
                                         const char* key,
                                         bool mandatory,
                                         UI::Box::Resizing default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "align") {
			default_value = UI::Box::Resizing::kAlign;
		} else if (str == "expandboth") {
			default_value = UI::Box::Resizing::kExpandBoth;
		} else if (str == "fullsize") {
			default_value = UI::Box::Resizing::kFullSize;
		} else if (str == "fillspace") {
			default_value = UI::Box::Resizing::kFillSpace;
		} else {
			report_error(L, "Unknown box resizing '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing box resizing: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

UI::DropdownType get_table_dropdown_type(lua_State* L,
                                         const char* key,
                                         bool mandatory,
                                         UI::DropdownType default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "textual") {
			default_value = UI::DropdownType::kTextual;
		} else if (str == "textual_narrow") {
			default_value = UI::DropdownType::kTextualNarrow;
		} else if (str == "pictorial") {
			default_value = UI::DropdownType::kPictorial;
		} else if (str == "pictorial_menu") {
			default_value = UI::DropdownType::kPictorialMenu;
		} else if (str == "textual_menu") {
			default_value = UI::DropdownType::kTextualMenu;
		} else {
			report_error(L, "Unknown dropdown type '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing dropdown type: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

UI::ListselectLayout get_table_listselect_layout(lua_State* L,
                                                 const char* key,
                                                 bool mandatory,
                                                 UI::ListselectLayout default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "plain") {
			default_value = UI::ListselectLayout::kPlain;
		} else if (str == "check") {
			default_value = UI::ListselectLayout::kShowCheck;
		} else if (str == "dropdown") {
			default_value = UI::ListselectLayout::kDropdown;
		} else {
			report_error(L, "Unknown listselect layout '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing listselect layout: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

UI::ButtonStyle get_table_button_style(lua_State* L,
                                       const char* key,
                                       bool mandatory,
                                       UI::ButtonStyle default_value) {
	const bool mainmenu = is_main_menu(L);

	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "primary") {
			default_value = mainmenu ? UI::ButtonStyle::kFsMenuPrimary : UI::ButtonStyle::kWuiPrimary;
		} else if (str == "secondary") {
			default_value =
			   mainmenu ? UI::ButtonStyle::kFsMenuSecondary : UI::ButtonStyle::kWuiSecondary;
		} else if (str == "menu") {
			default_value = mainmenu ? UI::ButtonStyle::kFsMenuMenu : UI::ButtonStyle::kWuiMenu;
		} else {
			report_error(L, "Unknown button style '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing button style: %s", key);
	} else if (mainmenu) {
		default_value = UI::ButtonStyle::kFsMenuSecondary;
	}
	lua_pop(L, 1);
	return default_value;
}

UI::SliderStyle get_slider_style(lua_State* L) {
	if (is_main_menu(L)) {
		return UI::SliderStyle::kFsMenu;
	}
	if (get_table_boolean(L, "dark", false)) {
		return UI::SliderStyle::kWuiDark;
	}
	return UI::SliderStyle::kWuiLight;
}

UI::TabPanelStyle get_tab_panel_style(lua_State* L) {
	if (is_main_menu(L)) {
		return UI::TabPanelStyle::kFsMenu;
	}
	if (get_table_boolean(L, "dark", false)) {
		return UI::TabPanelStyle::kWuiDark;
	}
	return UI::TabPanelStyle::kWuiLight;
}

UI::Button::VisualState get_table_button_visual_state(lua_State* L,
                                                      const char* key,
                                                      bool mandatory,
                                                      UI::Button::VisualState default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "raised") {
			default_value = UI::Button::VisualState::kRaised;
		} else if (str == "permpressed") {
			default_value = UI::Button::VisualState::kPermpressed;
		} else if (str == "flat") {
			default_value = UI::Button::VisualState::kFlat;
		} else {
			report_error(L, "Unknown button visual state '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing button visual state: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

unsigned get_table_button_box_orientation(lua_State* L,
                                          const char* key,
                                          bool mandatory,
                                          unsigned default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "v" || str == "vert" || str == "vertical") {
			default_value = UI::Box::Vertical;
		} else if (str == "h" || str == "horz" || str == "horizontal") {
			default_value = UI::Box::Horizontal;
		} else {
			report_error(L, "Unknown orientation '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing orientation: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

}  // namespace LuaUi
