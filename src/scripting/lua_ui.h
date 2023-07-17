/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/textinput.h"
#include "ui_basic/window.h"
#include "wui/interactive_base.h"

namespace LuaUi {

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

	/*
	 * C Methods
	 */
	static UI::Panel* do_create_child(lua_State* L, UI::Panel* parent, UI::Box* as_box);
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

	/*
	 * Lua Methods
	 */
	int press(lua_State* L);
	int click(lua_State* L);

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
	int get_expanded(lua_State* L);
	int get_no_of_items(lua_State* L);

	/*
	 * Lua Methods
	 */
	int open(lua_State* L);
	int highlight_item(lua_State* L);
	int indicate_item(lua_State* L);
	int select(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseDropdown* get() {
		return dynamic_cast<UI::BaseDropdown*>(panel_);
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
	int update_toolbar(lua_State* L);

	/*
	 * C Methods
	 */
	InteractiveBase* get() {
		return dynamic_cast<InteractiveBase*>(panel_);
	}
};

int upcasted_panel_to_lua(lua_State* L, UI::Panel* panel);

void luaopen_wlui(lua_State*);
}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_LUA_UI_H
