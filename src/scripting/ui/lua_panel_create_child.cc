/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#include "scripting/ui/lua_panel.h"

#include "scripting/globals.h"
#include "scripting/ui/lua_box.h"
#include "scripting/ui/lua_button.h"
#include "scripting/ui/lua_checkbox.h"
#include "scripting/ui/lua_dropdown.h"
#include "scripting/ui/lua_listselect.h"
#include "scripting/ui/lua_main_menu.h"
#include "scripting/ui/lua_map_view.h"
#include "scripting/ui/lua_multiline_textarea.h"
#include "scripting/ui/lua_progress_bar.h"
#include "scripting/ui/lua_radio_button.h"
#include "scripting/ui/lua_slider.h"
#include "scripting/ui/lua_spin_box.h"
#include "scripting/ui/lua_tab.h"
#include "scripting/ui/lua_tab_panel.h"
#include "scripting/ui/lua_table.h"
#include "scripting/ui/lua_text_input_panel.h"
#include "scripting/ui/lua_textarea.h"
#include "scripting/ui/lua_window.h"
#include "ui_basic/messagebox.h"

namespace LuaUi {

static inline UI::PanelStyle panel_style(lua_State* L) {
	return is_main_menu(L) ? UI::PanelStyle::kFsMenu : UI::PanelStyle::kWui;
}

static UI::Align get_table_align(lua_State* L,
                                 const char* key,
                                 bool mandatory,
                                 UI::Align default_value = UI::Align::kCenter) {
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

static UI::Box::Resizing
get_table_box_resizing(lua_State* L,
                       const char* key,
                       bool mandatory,
                       UI::Box::Resizing default_value = UI::Box::Resizing::kAlign) {
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

static UI::DropdownType
get_table_dropdown_type(lua_State* L,
                        const char* key,
                        bool mandatory,
                        UI::DropdownType default_value = UI::DropdownType::kTextual) {
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

static UI::ListselectLayout
get_table_listselect_layout(lua_State* L,
                            const char* key,
                            bool mandatory,
                            UI::ListselectLayout default_value = UI::ListselectLayout::kPlain) {
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

static UI::ButtonStyle
get_table_button_style(lua_State* L,
                       const char* key,
                       bool mandatory,
                       UI::ButtonStyle default_value = UI::ButtonStyle::kWuiSecondary) {
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

static inline UI::SliderStyle get_slider_style(lua_State* L) {
	if (is_main_menu(L)) {
		return UI::SliderStyle::kFsMenu;
	}
	if (get_table_boolean(L, "dark", false)) {
		return UI::SliderStyle::kWuiDark;
	}
	return UI::SliderStyle::kWuiLight;
}

static inline UI::TabPanelStyle get_tab_panel_style(lua_State* L) {
	if (is_main_menu(L)) {
		return UI::TabPanelStyle::kFsMenu;
	}
	if (get_table_boolean(L, "dark", false)) {
		return UI::TabPanelStyle::kWuiDark;
	}
	return UI::TabPanelStyle::kWuiLight;
}

static UI::Button::VisualState get_table_button_visual_state(
   lua_State* L,
   const char* key,
   bool mandatory,
   UI::Button::VisualState default_value = UI::Button::VisualState::kRaised) {
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

static unsigned get_table_button_box_orientation(lua_State* L,
                                                 const char* key,
                                                 bool mandatory,
                                                 unsigned default_value = UI::Box::Vertical) {
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

template <typename... Args>
static inline void do_set_global_string(lua_State*, const char*, Args...) {
	NEVER_HERE();
}
template <>
inline void do_set_global_string<std::string>(lua_State* L, const char* name, std::string arg) {
	lua_pushstring(L, arg);
	lua_setglobal(L, name);
}

template <typename... Args>
static std::function<void(Args...)>
create_plugin_action_lambda(lua_State* L, const std::string& cmd, bool is_hyperlink = false) {
	if (is_main_menu(L)) {
		FsMenu::MainMenu& fsmm = get_main_menu(L);
		return [&fsmm, cmd, is_hyperlink](Args... args) {  // do not capture L directly
			try {
				if (is_hyperlink) {
					do_set_global_string(fsmm.lua().L(), "HYPERLINK_ACTION", args...);
				}
				fsmm.lua().interpret_string(cmd);
			} catch (const LuaError& e) {
				log_err("Lua error in plugin: %s", e.what());

				if (g_fail_on_lua_error) {
					throw;
				}

				UI::WLMessageBox m(&fsmm, UI::WindowStyle::kFsMenu, _("Plugin Error"),
				                   format_l(_("Error when running plugin:\n%s"), e.what()),
				                   UI::WLMessageBox::MBoxType::kOk);
				m.run<UI::Panel::Returncodes>();
			}
		};
	}

	Widelands::EditorGameBase& egbase = get_egbase(L);
	return [&egbase, cmd, is_hyperlink](Args... args) {  // do not capture L directly
		try {
			if (is_hyperlink) {
				do_set_global_string(egbase.lua().L(), "HYPERLINK_ACTION", args...);
			}
			egbase.lua().interpret_string(cmd);
		} catch (const LuaError& e) {
			log_err("Lua error in plugin: %s", e.what());

			if (g_fail_on_lua_error) {
				throw;
			}

			UI::WLMessageBox m(egbase.get_ibase(), UI::WindowStyle::kWui, _("Plugin Error"),
			                   format_l(_("Error when running plugin:\n%s"), e.what()),
			                   UI::WLMessageBox::MBoxType::kOk);
			m.run<UI::Panel::Returncodes>();
		}
	};
}

// static, recursive function that does all the work for create_child()
UI::Panel* LuaPanel::do_create_child(lua_State* L, UI::Panel* parent, UI::Box* as_box) {
	luaL_checktype(L, -1, LUA_TTABLE);
	std::string widget_type = get_table_string(L, "widget", true);

	// Actually create the panel
	UI::Panel* created_panel = nullptr;
	UI::Box* child_as_box = nullptr;

	if (widget_type == "inf_space") {
		if (as_box == nullptr) {
			report_error(L, "'inf_space' only valid in boxes");
		}
		as_box->add_inf_space();

	} else if (widget_type == "space") {
		if (as_box == nullptr) {
			report_error(L, "'space' only valid in boxes");
		}
		as_box->add_space(get_table_int(L, "value", true));

	} else if (widget_type == "box") {
		child_as_box = do_create_child_box(L, parent);
		created_panel = child_as_box;
	} else if (widget_type == "button") {
		created_panel = do_create_child_button(L, parent);
	} else if (widget_type == "checkbox") {
		created_panel = do_create_child_checkbox(L, parent);
	} else if (widget_type == "discrete_slider") {
		created_panel = do_create_child_discrete_slider(L, parent);
	} else if (widget_type == "dropdown") {
		created_panel = do_create_child_dropdown(L, parent);
	} else if (widget_type == "editbox") {
		created_panel = do_create_child_editbox(L, parent);
	} else if (widget_type == "listselect") {
		created_panel = do_create_child_listselect(L, parent);
	} else if (widget_type == "multilineeditbox") {
		created_panel = do_create_child_multilineeditbox(L, parent);
	} else if (widget_type == "multilinetextarea") {
		created_panel = do_create_child_multilinetextarea(L, parent);
	} else if (widget_type == "panel") {
		created_panel = do_create_child_panel(L, parent);
	} else if (widget_type == "progressbar") {
		created_panel = do_create_child_progressbar(L, parent);
	} else if (widget_type == "radiogroup") {
		do_create_child_radiogroup(L, parent, as_box);
	} else if (widget_type == "spinbox") {
		created_panel = do_create_child_spinbox(L, parent);
	} else if (widget_type == "slider") {
		created_panel = do_create_child_slider(L, parent);
	} else if (widget_type == "tabpanel") {
		created_panel = do_create_child_tabpanel(L, parent);
	} else if (widget_type == "table") {
		created_panel = do_create_child_table(L, parent);
	} else if (widget_type == "textarea") {
		created_panel = do_create_child_textarea(L, parent);
	} else if (widget_type == "unique_window") {
		created_panel = do_create_child_unique_window(L, parent);
	} else if (widget_type == "window") {
		created_panel = do_create_child_window(L, parent);
	} else {
		report_error(L, "Unknown widget type '%s'", widget_type.c_str());
	}

	if (created_panel != nullptr) {
		// Signal bindings
		if (std::string cmd = get_table_string(L, "on_panel_clicked", false); !cmd.empty()) {
			created_panel->clicked.connect(create_plugin_action_lambda(L, cmd));
			created_panel->set_can_focus(true);
			created_panel->set_handle_mouse(true);
		}

		if (std::string cmd = get_table_string(L, "on_position_changed", false); !cmd.empty()) {
			created_panel->position_changed.connect(create_plugin_action_lambda(L, cmd));
		}

		if (std::string cmd = get_table_string(L, "on_hyperlink", false); !cmd.empty()) {
			created_panel->set_hyperlink_action(
			   create_plugin_action_lambda<std::string>(L, cmd, true));
		}

		// If a tooltip is desired, we may need to force it for some passive widget types
		if (std::string tooltip = get_table_string(L, "tooltip", false); !tooltip.empty()) {
			created_panel->set_tooltip(tooltip);
			created_panel->set_handle_mouse(true);
		}

		// Box layouting if applicable
		if (as_box != nullptr) {
			UI::Align align = get_table_align(L, "align", false);
			UI::Box::Resizing resizing = get_table_box_resizing(L, "resizing", false);
			as_box->add(created_panel, resizing, align);
		}

		// Widget children (recursive iteration)
		lua_getfield(L, -1, "children");
		if (!lua_isnil(L, -1)) {
			luaL_checktype(L, -1, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				do_create_child(L, created_panel, child_as_box);
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);
	}

	return created_panel;
}

UI::Box* LuaPanel::do_create_child_box(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	unsigned orientation = get_table_button_box_orientation(L, "orientation", true);
	int32_t max_x = get_table_int(L, "max_x", false);
	int32_t max_y = get_table_int(L, "max_y", false);
	int32_t spacing = get_table_int(L, "spacing", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);

	UI::Box* box =
	   new UI::Box(parent, panel_style(L), name, x, y, orientation, max_x, max_y, spacing);

	box->set_scrolling(get_table_boolean(L, "scrolling", false));

	return box;
}

UI::Panel* LuaPanel::do_create_child_button(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string title = get_table_string(L, "title", false);
	std::string icon = get_table_string(L, "icon", false);
	if (title.empty() == icon.empty()) {
		report_error(L, "Button must have either a title or an icon, but not both and not neither");
	}

	UI::ButtonStyle style = get_table_button_style(L, "style", false);
	UI::Button::VisualState visual = get_table_button_visual_state(L, "visual", false);

	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::Button* button;
	if (title.empty()) {
		button =
		   new UI::Button(parent, name, x, y, w, h, style, g_image_cache->get(icon), tooltip, visual);
	} else {
		button = new UI::Button(parent, name, x, y, w, h, style, title, tooltip, visual);
	}

	button->set_repeating(get_table_boolean(L, "repeating", false));

	if (std::string on_click = get_table_string(L, "on_click", false); !on_click.empty()) {
		button->sigclicked.connect(create_plugin_action_lambda(L, on_click));
	}

	return button;
}

UI::Panel* LuaPanel::do_create_child_checkbox(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string title = get_table_string(L, "title", false);
	std::string icon = get_table_string(L, "icon", false);
	bool initial_state = get_table_boolean(L, "state", false);

	if (title.empty() == icon.empty()) {
		report_error(L, "Checkbox must have either a title or an icon, but not both and not neither");
	}

	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);

	UI::Checkbox* checkbox;
	if (title.empty()) {
		checkbox = new UI::Checkbox(
		   parent, panel_style(L), name, Vector2i(x, y), g_image_cache->get(icon), tooltip);
	} else {
		checkbox = new UI::Checkbox(parent, panel_style(L), name, Vector2i(x, y), title, tooltip);
	}

	checkbox->set_state(initial_state, false);

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		checkbox->changed.connect(create_plugin_action_lambda(L, on_changed));
	}

	return checkbox;
}

UI::Panel* LuaPanel::do_create_child_discrete_slider(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	uint32_t cursor_size = get_table_int(L, "cursor_size", false, 20);
	uint32_t init_value = get_table_int(L, "value", true);

	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	std::vector<std::string> labels;
	lua_getfield(L, -1, "labels");
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		labels.emplace_back(luaL_checkstring(L, -1));
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	if (labels.empty()) {
		report_error(L, "Discrete slider without labels");
	}
	if (init_value >= labels.size()) {
		report_error(L, "Discrete slider initial value out of range");
	}

	UI::DiscreteSlider* slider = new UI::DiscreteSlider(
	   parent, name, x, y, w, h, labels, init_value, get_slider_style(L), tooltip, cursor_size);

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		slider->changed.connect(create_plugin_action_lambda(L, on_changed));
	}

	return slider;
}

UI::Panel* LuaPanel::do_create_child_dropdown(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string label = get_table_string(L, "label", true);
	int32_t max_list_items = get_table_int(L, "max_list_items", true);
	int32_t button_dimension = get_table_int(L, "button_dimension", true);
	UI::ButtonStyle button_style = get_table_button_style(L, "button_style", true);
	UI::DropdownType type = get_table_dropdown_type(L, "type", true);
	std::string datatype = get_table_string(L, "datatype", true);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);

	UI::BaseDropdown* dropdown;
	if (datatype == "string") {
		DropdownOfString* dd =
		   new DropdownOfString(parent, name, x, y, w, max_list_items, button_dimension, label, type,
		                        panel_style(L), button_style);
		dropdown = dd;

		lua_getfield(L, -1, "entries");
		if (!lua_isnil(L, -1)) {
			luaL_checktype(L, -1, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				std::string elabel = get_table_string(L, "label", true);
				std::string value = get_table_string(L, "value", true);
				std::string etooltip = get_table_string(L, "tooltip", false);
				std::string ehotkey = get_table_string(L, "hotkey", false);
				std::string icon = get_table_string(L, "icon", type == UI::DropdownType::kPictorial);
				bool select = get_table_boolean(L, "select", false);

				dd->add(elabel, value, icon.empty() ? nullptr : g_image_cache->get(icon), select,
				        etooltip, shortcut_string_if_set(ehotkey, false));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

	} else {
		report_error(L, "Unsupported dropdown datatype '%s'", datatype.c_str());
	}

	if (std::string icon = get_table_string(L, "icon", false); !icon.empty()) {
		dropdown->set_image(g_image_cache->get(icon));
	}

	if (std::string on_selected = get_table_string(L, "on_selected", false); !on_selected.empty()) {
		dropdown->selected.connect(create_plugin_action_lambda(L, on_selected));
	}

	return dropdown;
}

UI::Panel* LuaPanel::do_create_child_editbox(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string text = get_table_string(L, "text", false);
	bool password = get_table_boolean(L, "password", false);
	bool warning = get_table_boolean(L, "warning", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);

	UI::EditBox* editbox = new UI::EditBox(parent, name, x, y, w, panel_style(L));

	editbox->set_text(text);
	editbox->set_password(password);
	editbox->set_warning(warning);

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		editbox->changed.connect(create_plugin_action_lambda(L, on_changed));
	}
	if (std::string on_cancel = get_table_string(L, "on_cancel", false); !on_cancel.empty()) {
		editbox->cancel.connect(create_plugin_action_lambda(L, on_cancel));
	}
	if (std::string on_ok = get_table_string(L, "on_ok", false); !on_ok.empty()) {
		editbox->ok.connect(create_plugin_action_lambda(L, on_ok));
	}

	return editbox;
}

UI::Panel* LuaPanel::do_create_child_listselect(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	UI::ListselectLayout layout = get_table_listselect_layout(L, "type", false);
	std::string datatype = get_table_string(L, "datatype", true);

	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::BaseListselect* listselect;
	if (datatype == "string") {
		ListselectOfString* ls =
		   new ListselectOfString(parent, name, x, y, w, h, panel_style(L), layout);
		listselect = ls;

		lua_getfield(L, -1, "entries");
		if (!lua_isnil(L, -1)) {
			luaL_checktype(L, -1, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				std::string label = get_table_string(L, "label", true);
				std::string value = get_table_string(L, "value", true);
				std::string etooltip = get_table_string(L, "tooltip", false);
				std::string ehotkey = get_table_string(L, "hotkey", false);
				std::string icon = get_table_string(L, "icon", false);
				bool select = get_table_boolean(L, "select", false);
				bool enable = get_table_boolean(L, "enable", false, true);
				int32_t indent = get_table_int(L, "indent", false);

				ls->add(label, value, icon.empty() ? nullptr : g_image_cache->get(icon), select,
				        etooltip, shortcut_string_if_set(ehotkey, false), indent, enable);
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

	} else {
		report_error(L, "Unsupported listselect datatype '%s'", datatype.c_str());
	}

	if (std::string on_selected = get_table_string(L, "on_selected", false); !on_selected.empty()) {
		listselect->selected.connect(create_plugin_action_lambda<uint32_t>(L, on_selected));
	}
	if (std::string on_double_clicked = get_table_string(L, "on_double_clicked", false);
	    !on_double_clicked.empty()) {
		listselect->double_clicked.connect(
		   create_plugin_action_lambda<uint32_t>(L, on_double_clicked));
	}

	return listselect;
}

UI::Panel* LuaPanel::do_create_child_multilineeditbox(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string text = get_table_string(L, "text", false);
	bool password = get_table_boolean(L, "password", false);
	bool warning = get_table_boolean(L, "warning", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::MultilineEditbox* editbox =
	   new UI::MultilineEditbox(parent, name, x, y, w, h, panel_style(L));

	editbox->set_text(text);
	editbox->set_password(password);
	editbox->set_warning(warning);

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		editbox->changed.connect(create_plugin_action_lambda(L, on_changed));
	}
	if (std::string on_cancel = get_table_string(L, "on_cancel", false); !on_cancel.empty()) {
		editbox->cancel.connect(create_plugin_action_lambda(L, on_cancel));
	}

	return editbox;
}

UI::Panel* LuaPanel::do_create_child_multilinetextarea(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string text = get_table_string(L, "text", true);
	UI::Align align = get_table_align(L, "text_align", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::MultilineTextarea::ScrollMode scroll_mode;
	std::string scroll = get_table_string(L, "scroll_mode", true);
	if (scroll == "none") {
		scroll_mode = UI::MultilineTextarea::ScrollMode::kNoScrolling;
	} else if (scroll == "normal") {
		scroll_mode = UI::MultilineTextarea::ScrollMode::kScrollNormal;
	} else if (scroll == "normal_force") {
		scroll_mode = UI::MultilineTextarea::ScrollMode::kScrollNormalForced;
	} else if (scroll == "log") {
		scroll_mode = UI::MultilineTextarea::ScrollMode::kScrollLog;
	} else if (scroll == "log_force") {
		scroll_mode = UI::MultilineTextarea::ScrollMode::kScrollLogForced;
	} else {
		report_error(L, "Unknown scroll mode '%s'", scroll.c_str());
	}

	UI::MultilineTextarea* txt =
	   new UI::MultilineTextarea(parent, name, x, y, w, h, panel_style(L), text, align, scroll_mode);

	if (std::string font = get_table_string(L, "font", false); !font.empty()) {
		txt->set_style(g_style_manager->safe_font_style(font));
		txt->set_text(text);  // Needed to force a recompute
	}

	return txt;
}

UI::Panel* LuaPanel::do_create_child_panel(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	return new UI::Panel(parent, panel_style(L), name, x, y, w, h, tooltip);
}

UI::Panel* LuaPanel::do_create_child_progressbar(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	unsigned orientation = get_table_button_box_orientation(L, "orientation", true);
	int32_t total = get_table_int(L, "total", true);
	int32_t state = get_table_int(L, "state", true);
	bool percent = get_table_boolean(L, "percent", false, true);

	if (total < 1) {
		report_error(L, "Progressbar total must be positive");
	}
	if (state < 0 || state > total) {
		report_error(L, "Progressbar initial state out of range");
	}

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::ProgressBar* bar =
	   new UI::ProgressBar(parent, panel_style(L), name, x, y, w, h, orientation);

	bar->set_total(total);
	bar->set_state(state);
	bar->set_show_percent(percent);

	return bar;
}

void LuaPanel::do_create_child_radiogroup(lua_State* L, UI::Panel* parent, UI::Box* as_box) {
	int32_t initial_state = get_table_int(L, "state", false);

	UI::Align align = get_table_align(L, "align", false);
	UI::Box::Resizing resizing = get_table_box_resizing(L, "resizing", false);

	UI::Radiogroup* group = new UI::Radiogroup();

	lua_getfield(L, -1, "buttons");
	luaL_checktype(L, -1, LUA_TTABLE);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		std::string name = get_table_string(L, "name", true);
		std::string icon = get_table_string(L, "icon", true);
		std::string rtooltip = get_table_string(L, "tooltip", false);
		int32_t rx = get_table_int(L, "x", false);
		int32_t ry = get_table_int(L, "y", false);

		UI::Radiobutton* radiobutton;
		group->add_button(parent, panel_style(L), name, Vector2i(rx, ry), g_image_cache->get(icon),
		                  rtooltip, &radiobutton);

		// Box layouting if applicable
		if (as_box != nullptr) {
			as_box->add(radiobutton, resizing, align);
		}

		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	group->set_state(initial_state, false);
	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		group->changed.connect(create_plugin_action_lambda(L, on_changed));
	}

	group->manage_own_lifetime();
}

UI::Panel* LuaPanel::do_create_child_slider(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	unsigned orientation = get_table_button_box_orientation(L, "orientation", true);
	int32_t val_min = get_table_int(L, "min", true);
	int32_t val_max = get_table_int(L, "max", true);
	int32_t val = get_table_int(L, "value", true);
	uint32_t cursor_size = get_table_int(L, "cursor_size", false, 20);

	if (val_min > val_max) {
		report_error(L, "Malformed slider value range");
	}
	if (val < val_min || val > val_max) {
		report_error(L, "Slider initial value out of range");
	}

	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::Slider* slider;
	if (orientation == UI::Box::Vertical) {
		slider = new UI::VerticalSlider(parent, name, x, y, w, h, val_min, val_max, val,
		                                get_slider_style(L), cursor_size, tooltip);
	} else {
		slider = new UI::HorizontalSlider(parent, name, x, y, w, h, val_min, val_max, val,
		                                  get_slider_style(L), tooltip, cursor_size);
	}

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		slider->changed.connect(create_plugin_action_lambda(L, on_changed));
	}

	return slider;
}

UI::Panel* LuaPanel::do_create_child_spinbox(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	uint32_t unit_w = get_table_int(L, "unit_w", true);
	int32_t val = get_table_int(L, "value", true);
	std::string label = get_table_string(L, "label", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);

	UI::SpinBox::Units units;

	std::string units_str = get_table_string(L, "units", false);
	if (units_str.empty() || units_str == "none") {
		units = UI::SpinBox::Units::kNone;
	} else if (units_str == "pixels") {
		units = UI::SpinBox::Units::kPixels;
	} else if (units_str == "minutes") {
		units = UI::SpinBox::Units::kMinutes;
	} else if (units_str == "weeks") {
		units = UI::SpinBox::Units::kWeeks;
	} else if (units_str == "percent") {
		units = UI::SpinBox::Units::kPercent;
	} else if (units_str == "fields") {
		units = UI::SpinBox::Units::kFields;
	} else {
		report_error(L, "Unknown spinbox unit '%s'", units_str.c_str());
	}

	std::vector<int32_t> value_list;

	lua_getfield(L, -1, "values");
	if (!lua_isnil(L, -1)) {
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			value_list.push_back(luaL_checkint32(L, -1));
			lua_pop(L, 1);
		}
		if (value_list.empty()) {
			report_error(L, "Spinbox: Empty values table");
		}
	}
	lua_pop(L, 1);

	int32_t val_min = 0;
	int32_t val_max = 0;
	int32_t step_size_small = 1;
	int32_t step_size_big = 0;
	UI::SpinBox::Type sb_type = UI::SpinBox::Type::kSmall;

	if (value_list.empty()) {
		// Spinbox with normal numeric values

		val_min = get_table_int(L, "min", true);
		val_max = get_table_int(L, "max", true);
		step_size_small = get_table_int(L, "step_size_small", false, 1);
		step_size_big = get_table_int(L, "step_size_big", false, 0);

		if (val_min > val_max) {
			report_error(L, "Malformed spinbox value range");
		}
		if (step_size_big > 0) {
			sb_type = UI::SpinBox::Type::kBig;
		}
	} else {
		// Spinbox with custom value list

		sb_type = UI::SpinBox::Type::kValueList;

		// These are ignored by SpinBox::SpinBox if type == kValueList
		val_min = 0;
		val_max = value_list.size() - 1;  // only used for range-checking the initial value below

		// Check conflicting settings
		if (luna_table_has_key(L, "min") || luna_table_has_key(L, "max")) {
			report_error(L, "Spinbox: Cannot combine value list and min/max value");
		}
		if (luna_table_has_key(L, "step_size_small") || luna_table_has_key(L, "step_size_big")) {
			report_error(L, "Spinbox: Cannot combine value list and step sizes");
		}
	}

	if (val < val_min || val > val_max) {
		report_error(L, "Spinbox initial value out of range");
	}

	UI::SpinBox* spinbox =
	   new UI::SpinBox(parent, name, x, y, w, unit_w, val, val_min, val_max, panel_style(L), label,
	                   units, sb_type, step_size_small, step_size_big);

	if (!value_list.empty()) {
		spinbox->set_value_list(value_list);
	}

	lua_getfield(L, -1, "replacements");
	if (!lua_isnil(L, -1)) {
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			spinbox->add_replacement(
			   get_table_int(L, "value", true), get_table_string(L, "replacement", true));
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
		spinbox->changed.connect(create_plugin_action_lambda(L, on_changed));
	}

	return spinbox;
}

UI::Panel* LuaPanel::do_create_child_tabpanel(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);

	UI::TabPanel* tabpanel = new UI::TabPanel(parent, get_tab_panel_style(L), name);

	lua_getfield(L, -1, "tabs");
	if (!lua_isnil(L, -1)) {
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			std::string tabname = get_table_string(L, "name", true);
			std::string title = get_table_string(L, "title", false);
			std::string icon = get_table_string(L, "icon", false);
			std::string ttooltip = get_table_string(L, "tooltip", false);

			if (title.empty() == icon.empty()) {
				report_error(
				   L, "Tabs must have either a title or an icon, but not both and not neither");
			}

			lua_getfield(L, -1, "panel");
			luaL_checktype(L, -1, LUA_TTABLE);
			UI::Panel* wrapped_tab = do_create_child(L, tabpanel, nullptr);
			lua_pop(L, 1);

			if (icon.empty()) {
				tabpanel->add(tabname, title, wrapped_tab, ttooltip);
			} else {
				tabpanel->add(tabname, g_image_cache->get(icon), wrapped_tab, ttooltip);
			}

			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "active");
	if (!lua_isnil(L, -1)) {
		if (static_cast<bool>(lua_isnumber(L, -1))) {
			tabpanel->activate(luaL_checkuint32(L, -1));
		} else {
			tabpanel->activate(luaL_checkstring(L, -1));
		}
	}
	lua_pop(L, 1);

	if (std::string on_clicked = get_table_string(L, "on_clicked", false); !on_clicked.empty()) {
		tabpanel->sigclicked.connect(create_plugin_action_lambda(L, on_clicked));
	}

	return tabpanel;
}

UI::Panel* LuaPanel::do_create_child_table(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string datatype = get_table_string(L, "datatype", true);
	bool multiselect = get_table_boolean(L, "multiselect", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::BaseTable* table;
	if (datatype == "int") {
		table = new TableOfInt(parent, name, x, y, w, h, panel_style(L),
		                       multiselect ? UI::TableRows::kMulti : UI::TableRows::kSingle);
	} else {
		report_error(L, "Unsupported table datatype '%s'", datatype.c_str());
	}

	bool has_flexible = false;
	unsigned ncolumns = 0;
	lua_getfield(L, -1, "columns");
	if (!lua_isnil(L, -1)) {
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			int32_t column_w = get_table_int(L, "w", true);
			std::string title = get_table_string(L, "title", true);
			std::string ctooltip = get_table_string(L, "tooltip", false);
			bool flexible = get_table_boolean(L, "flexible", false);
			UI::Align align = get_table_align(L, "align", false);

			if (flexible) {
				if (has_flexible) {
					report_error(L, "Table may not have multiple flexible columns");
				}
				has_flexible = true;
			}

			table->add_column(column_w, title, ctooltip, align,
			                  flexible ? UI::TableColumnType::kFlexible : UI::TableColumnType::kFixed);
			++ncolumns;
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	lua_getfield(L, -1, "rows");
	if (!lua_isnil(L, -1)) {
		upcast(TableOfInt, t, table);
		assert(t != nullptr);
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			uintptr_t value = get_table_int(L, "value", true);
			bool select = get_table_boolean(L, "select", false);
			bool disable = get_table_boolean(L, "disable", false);
			TableOfInt::EntryRecord& record = t->add(value, select);
			record.set_disabled(disable);

			for (unsigned i = 0; i < ncolumns; ++i) {
				std::string text = get_table_string(L, format("text_%u", i).c_str(), false);
				std::string icon = get_table_string(L, format("icon_%u", i).c_str(), false);

				if (icon.empty()) {
					record.set_string(i, text);
				} else {
					record.set_picture(i, g_image_cache->get(icon), text);
				}
			}

			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);

	table->set_sort_column(get_table_int(L, "sort_column", false));
	table->set_sort_descending(get_table_boolean(L, "sort_descending", false));

	if (std::string on_cancel = get_table_string(L, "on_cancel", false); !on_cancel.empty()) {
		table->cancel.connect(create_plugin_action_lambda(L, on_cancel));
	}
	if (std::string on_selected = get_table_string(L, "on_selected", false); !on_selected.empty()) {
		table->selected.connect(create_plugin_action_lambda<uint32_t>(L, on_selected));
	}
	if (std::string on_double_clicked = get_table_string(L, "on_double_clicked", false);
	    !on_double_clicked.empty()) {
		table->double_clicked.connect(create_plugin_action_lambda<uint32_t>(L, on_double_clicked));
	}

	return table;
}

UI::Panel* LuaPanel::do_create_child_textarea(lua_State* L, UI::Panel* parent) {
	std::string name = get_table_string(L, "name", true);
	std::string text = get_table_string(L, "text", true);
	UI::FontStyle font = g_style_manager->safe_font_style(get_table_string(L, "font", true));
	UI::Align align = get_table_align(L, "text_align", false);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::Textarea* txt =
	   new UI::Textarea(parent, panel_style(L), name, font, x, y, w, h, text, align);

	txt->set_fixed_width(get_table_int(L, "fixed_width", false));

	return txt;
}

UI::Panel* LuaPanel::do_create_child_unique_window(lua_State* L, UI::Panel* parent) {
	if (parent->get_parent() != nullptr) {
		report_error(L, "Unique windows must be toplevel components");
	}

	std::string registry = get_table_string(L, "registry", true);
	UI::UniqueWindow::Registry* reg;
	UI::WindowStyle style;

	if (is_main_menu(L)) {
		reg = &get_main_menu(L).unique_windows().get_registry(registry);
		style = UI::WindowStyle::kFsMenu;
	} else {
		reg = &get_egbase(L).get_ibase()->unique_windows().get_registry(registry);
		style = UI::WindowStyle::kWui;
	}

	if (reg->window != nullptr) {
		return reg->window;
	}

	std::string name = get_table_string(L, "name", true);
	std::string title = get_table_string(L, "title", true);

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	UI::UniqueWindow* window = new UI::UniqueWindow(parent, style, name, reg, x, y, w, h, title);

	lua_getfield(L, -1, "content");
	if (!lua_isnil(L, -1)) {
		window->set_center_panel(do_create_child(L, window, nullptr));
	}
	lua_pop(L, 1);

	return window;
}

UI::Panel* LuaPanel::do_create_child_window(lua_State* L, UI::Panel* parent) {
	if (parent->get_parent() != nullptr) {
		report_error(L, "Windows must be toplevel components");
	}

	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	std::string name = get_table_string(L, "name", true);
	std::string title = get_table_string(L, "title", true);
	UI::Window* window =
	   new UI::Window(parent, is_main_menu(L) ? UI::WindowStyle::kFsMenu : UI::WindowStyle::kWui,
	                  name, x, y, w, h, title);

	lua_getfield(L, -1, "content");
	if (!lua_isnil(L, -1)) {
		window->set_center_panel(do_create_child(L, window, nullptr));
	}
	lua_pop(L, 1);

	return window;
}

}  // namespace LuaUi
