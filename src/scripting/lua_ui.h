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

#ifndef WL_SCRIPTING_LUA_UI_H
#define WL_SCRIPTING_LUA_UI_H

#include "scripting/lua.h"
#include "scripting/luna.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/slider.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"
#include "wui/interactive_base.h"

namespace LuaUi {

// The currently supported kinds of templated UI elements.
using TableOfInt = UI::Table<uintptr_t>;
using DropdownOfString = UI::Dropdown<std::string>;
using ListselectOfString = UI::Listselect<std::string>;

/*
 * Base class for all classes in wl.ui
 */
class LuaUiModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "ui";
	}
};

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
	int die(lua_State* L);
	int force_redraw(lua_State* L);

	/*
	 * C Methods
	 */
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

class LuaButton : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaButton);

	LuaButton() = default;
	explicit LuaButton(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaButton(lua_State* L) : LuaPanel(L) {
	}
	~LuaButton() override = default;

	/*
	 * Properties
	 */
	int get_title(lua_State* L);
	int set_title(lua_State* L);
	int get_enabled(lua_State* L);
	int set_enabled(lua_State* L);

	/*
	 * Lua Methods
	 */
	int press(lua_State* L);
	int click(lua_State* L);
	int set_repeating(lua_State* L);
	int set_perm_pressed(lua_State* L);
	int toggle(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Button* get() {
		return dynamic_cast<UI::Button*>(panel_);
	}
};

class LuaMultilineTextarea : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMultilineTextarea);

	LuaMultilineTextarea() = default;
	explicit LuaMultilineTextarea(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaMultilineTextarea(lua_State* L) : LuaPanel(L) {
	}
	~LuaMultilineTextarea() override = default;

	/*
	 * Properties
	 */
	int get_text(lua_State* L);
	int set_text(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::MultilineTextarea* get() {
		return dynamic_cast<UI::MultilineTextarea*>(panel_);
	}
};

class LuaTextarea : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTextarea);

	LuaTextarea() = default;
	explicit LuaTextarea(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTextarea(lua_State* L) : LuaPanel(L) {
	}
	~LuaTextarea() override = default;

	/*
	 * Properties
	 */
	int get_text(lua_State* L);
	int set_text(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::Textarea* get() {
		return dynamic_cast<UI::Textarea*>(panel_);
	}
};

class LuaCheckbox : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaCheckbox);

	LuaCheckbox() = default;
	explicit LuaCheckbox(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaCheckbox(lua_State* L) : LuaPanel(L) {
	}
	~LuaCheckbox() override = default;

	/*
	 * Properties
	 */
	int get_state(lua_State* L);
	int set_state(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_enabled(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Checkbox* get() {
		return dynamic_cast<UI::Checkbox*>(panel_);
	}
};

class LuaRadioButton : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaRadioButton);

	LuaRadioButton() = default;
	explicit LuaRadioButton(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaRadioButton(lua_State* L) : LuaPanel(L) {
	}
	~LuaRadioButton() override = default;

	/*
	 * Properties
	 */
	int get_state(lua_State* L);
	int set_state(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_enabled(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Radiobutton* get() {
		return dynamic_cast<UI::Radiobutton*>(panel_);
	}
};

class LuaProgressBar : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaProgressBar);

	LuaProgressBar() = default;
	explicit LuaProgressBar(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaProgressBar(lua_State* L) : LuaPanel(L) {
	}
	~LuaProgressBar() override = default;

	/*
	 * Properties
	 */
	int get_state(lua_State* L);
	int set_state(lua_State* L);
	int get_total(lua_State* L);
	int set_total(lua_State* L);
	int get_show_percent(lua_State* L);
	int set_show_percent(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::ProgressBar* get() {
		return dynamic_cast<UI::ProgressBar*>(panel_);
	}
};

class LuaSpinBox : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaSpinBox);

	LuaSpinBox() = default;
	explicit LuaSpinBox(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaSpinBox(lua_State* L) : LuaPanel(L) {
	}
	~LuaSpinBox() override = default;

	/*
	 * Properties
	 */
	int get_value(lua_State* L);
	int set_value(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_unit_width(lua_State* L);
	int set_interval(lua_State* L);
	int add_replacement(lua_State* L);

	/*
	 * C Methods
	 */
	UI::SpinBox* get() {
		return dynamic_cast<UI::SpinBox*>(panel_);
	}
};

class LuaSlider : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaSlider);

	LuaSlider() = default;
	explicit LuaSlider(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaSlider(lua_State* L) : LuaPanel(L) {
	}
	~LuaSlider() override = default;

	/*
	 * Properties
	 */
	int get_value(lua_State* L);
	int set_value(lua_State* L);
	int get_min_value(lua_State* L);
	int set_min_value(lua_State* L);
	int get_max_value(lua_State* L);
	int set_max_value(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_enabled(lua_State* L);
	int set_cursor_fixed_height(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Slider* get() {
		return dynamic_cast<UI::Slider*>(panel_);
	}
};

class LuaTextInputPanel : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTextInputPanel);

	LuaTextInputPanel() = default;
	explicit LuaTextInputPanel(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTextInputPanel(lua_State* L) : LuaPanel(L) {
	}
	~LuaTextInputPanel() override = default;

	/*
	 * Properties
	 */
	int get_text(lua_State* L);
	int set_text(lua_State* L);
	int get_selected_text(lua_State* L);
	int get_password(lua_State* L);
	int set_password(lua_State* L);
	int get_warning(lua_State* L);
	int set_warning(lua_State* L);
	int get_caret_pos(lua_State* L);
	int set_caret_pos(lua_State* L);
	int get_multiline(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::AbstractTextInputPanel* get() {
		return dynamic_cast<UI::AbstractTextInputPanel*>(panel_);
	}
};

class LuaDropdown : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaDropdown);

	LuaDropdown() = default;
	explicit LuaDropdown(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaDropdown(lua_State* L) : LuaPanel(L) {
	}
	~LuaDropdown() override = default;

	/*
	 * Properties
	 */
	int get_datatype(lua_State* L);
	int get_expanded(lua_State* L);
	int get_no_of_items(lua_State* L);
	int get_selection(lua_State* L);

	/*
	 * Lua Methods
	 */
	int open(lua_State* L);
	int highlight_item(lua_State* L);
	int indicate_item(lua_State* L);
	int select(lua_State* L);
	int add(lua_State* L);
	int get_value_at(lua_State* L);
	int get_label_at(lua_State* L);
	int get_tooltip_at(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseDropdown* get() {
		return dynamic_cast<UI::BaseDropdown*>(panel_);
	}
};

class LuaListselect : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaListselect);

	LuaListselect() = default;
	explicit LuaListselect(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaListselect(lua_State* L) : LuaPanel(L) {
	}
	~LuaListselect() override = default;

	/*
	 * Properties
	 */
	int get_datatype(lua_State* L);
	int get_no_of_items(lua_State* L);
	int get_selection(lua_State* L);

	/*
	 * Lua Methods
	 */
	int add(lua_State* L);
	int get_value_at(lua_State* L);
	int get_label_at(lua_State* L);
	int get_tooltip_at(lua_State* L);
	int get_enable_at(lua_State* L);
	int get_indent_at(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseListselect* get() {
		return dynamic_cast<UI::BaseListselect*>(panel_);
	}
};

class LuaTable : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTable);

	LuaTable() = default;
	explicit LuaTable(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTable(lua_State* L) : LuaPanel(L) {
	}
	~LuaTable() override = default;

	/*
	 * Properties
	 */
	int get_datatype(lua_State* L);
	int get_no_of_rows(lua_State* L);
	int get_selection_index(lua_State* L);
	int set_selection_index(lua_State* L);
	int get_selections(lua_State* L);
	int get_sort_column(lua_State* L);
	int set_sort_column(lua_State* L);
	int get_sort_descending(lua_State* L);
	int set_sort_descending(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get(lua_State* L);
	int add(lua_State* L);
	int remove_row(lua_State* L);
	int remove_entry(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseTable* get() {
		return dynamic_cast<UI::BaseTable*>(panel_);
	}
};

class LuaTabPanel : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTabPanel);

	LuaTabPanel() = default;
	explicit LuaTabPanel(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTabPanel(lua_State* L) : LuaPanel(L) {
	}
	~LuaTabPanel() override = default;

	/*
	 * Properties
	 */
	int get_no_of_tabs(lua_State* L);
	int get_active(lua_State* L);
	int set_active(lua_State* L);

	/*
	 * Lua Methods
	 */
	int remove_last_tab(lua_State* L);

	/*
	 * C Methods
	 */
	UI::TabPanel* get() {
		return dynamic_cast<UI::TabPanel*>(panel_);
	}
};

class LuaTab : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTab);

	LuaTab() = default;
	explicit LuaTab(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTab(lua_State* L) : LuaPanel(L) {
	}
	~LuaTab() override = default;

	/*
	 * Properties
	 */
	int get_active(lua_State* L);

	/*
	 * Lua Methods
	 */
	int click(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Tab* get() {
		return dynamic_cast<UI::Tab*>(panel_);
	}
};

class LuaWindow : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaWindow);

	LuaWindow() = default;
	explicit LuaWindow(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaWindow(lua_State* L) : LuaPanel(L) {
	}
	~LuaWindow() override = default;

	/*
	 * Properties
	 */
	int get_title(lua_State* L);
	int set_title(lua_State* L);
	int get_pinned(lua_State* L);
	int set_pinned(lua_State* L);
	int get_minimal(lua_State* L);
	int set_minimal(lua_State* L);

	/*
	 * Lua Methods
	 */
	int close(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Window* get() {
		return dynamic_cast<UI::Window*>(panel_);
	}
};

class LuaMapView : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMapView);

	LuaMapView() = default;
	explicit LuaMapView(MapView* p) : LuaPanel(p) {
	}
	explicit LuaMapView(lua_State* L);
	~LuaMapView() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_average_fps(lua_State* L);
	int get_center_map_pixel(lua_State*);
	int get_buildhelp(lua_State* L);
	int set_buildhelp(lua_State* L);
	int get_census(lua_State* L);
	int set_census(lua_State* L);
	int get_statistics(lua_State* L);
	int set_statistics(lua_State* L);
	int get_is_building_road(lua_State* L);
	int get_auto_roadbuilding_mode(lua_State* L);
	int get_is_animating(lua_State*);
	int get_toolbar(lua_State* L);

	/*
	 * Lua Methods
	 */
	int close(lua_State* L);
	int click(lua_State* L);
	int abort_road_building(lua_State* L);
	int start_road_building(lua_State* L);
	int scroll_to_map_pixel(lua_State* L);
	int scroll_to_field(lua_State* L);
	int is_visible(lua_State* L);
	int mouse_to_field(lua_State* L);
	int mouse_to_pixel(lua_State* L);
	int add_toolbar_plugin(lua_State* L);
	int update_toolbar(lua_State* L);
	int set_keyboard_shortcut(lua_State* L);
	int set_keyboard_shortcut_release(lua_State* L);
	int add_plugin_timer(lua_State* L);

	/*
	 * C Methods
	 */
	InteractiveBase* get() {
		return dynamic_cast<InteractiveBase*>(panel_);
	}
};

class LuaMainMenu : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMainMenu);

	LuaMainMenu() = default;
	explicit LuaMainMenu(FsMenu::MainMenu* p) : LuaPanel(p) {
	}
	explicit LuaMainMenu(lua_State* L);
	~LuaMainMenu() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int set_keyboard_shortcut(lua_State* L);
	int set_keyboard_shortcut_release(lua_State* L);
	int add_plugin_timer(lua_State* L);

	/*
	 * C Methods
	 */
	FsMenu::MainMenu* get() {
		return dynamic_cast<FsMenu::MainMenu*>(panel_);
	}
};

int upcasted_panel_to_lua(lua_State* L, UI::Panel* panel);

void luaopen_wlui(lua_State*, bool game_or_editor);
}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_LUA_UI_H
