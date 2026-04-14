/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_UI_LUA_PANEL_H
#define WL_SCRIPTING_UI_LUA_PANEL_H

#include "scripting/lua_ui.h"
#include "ui/basic/panel.h"

namespace UI {
struct Box;
}  // namespace UI

namespace LuaUi {

class LuaPanel : public LuaUiModuleClass {
protected:
	UI::Panel* panel_{nullptr};

public:
	LUNA_CLASS_HEAD(LuaPanel);

	LuaPanel() = default;
	explicit LuaPanel(UI::Panel* p) : panel_(p) {
	}
	explicit LuaPanel(lua_State* L) : panel_(nullptr) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	~LuaPanel() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override {
		report_error(L, "Trying to persist a User Interface Panel which is not supported!");
	}
	void __unpersist(lua_State* L) override {
		report_error(L, "Trying to unpersist a User Interface Panel which is "
		                "not supported!");
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_name(lua_State* L);
	int get_parent(lua_State* L);
	int get_children(lua_State* L);
	int get_buttons(lua_State* L);
	int get_dropdowns(lua_State* L);
	int get_tabs(lua_State* L);
	int get_windows(lua_State* L);
	int get_width(lua_State* L);
	int set_width(lua_State* L);
	int get_height(lua_State* L);
	int set_height(lua_State* L);
	int get_position_x(lua_State* L);
	int set_position_x(lua_State* L);
	int get_position_y(lua_State* L);
	int set_position_y(lua_State* L);
	int get_visible(lua_State* L);
	int set_visible(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_descendant_position(lua_State* L);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	int indicate(lua_State* L);
#endif
	int create_child(lua_State* L);
	int get_child(lua_State* L);
	int layout(lua_State* L);
	int die(lua_State* L);
	int force_redraw(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Panel* get() {
		return panel_;
	}
	static UI::Panel* do_create_child(lua_State* L, UI::Panel* parent, UI::Box* as_box);
	static UI::Box* do_create_child_box(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_button(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_checkbox(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_discrete_slider(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_dropdown(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_editbox(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_listselect(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_multilineeditbox(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_multilinetextarea(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_panel(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_progressbar(lua_State* L, UI::Panel* parent);
	static void do_create_child_radiogroup(lua_State* L, UI::Panel* parent, UI::Box* as_box);
	static UI::Panel* do_create_child_slider(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_spinbox(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_tabpanel(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_table(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_textarea(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_unique_window(lua_State* L, UI::Panel* parent);
	static UI::Panel* do_create_child_window(lua_State* L, UI::Panel* parent);
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_PANEL_H
