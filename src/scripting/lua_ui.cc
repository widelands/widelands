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

#include "scripting/lua_ui.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/log.h"
#include "base/macros.h"
#include "graphic/style_manager.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/lua_map.h"
#include "scripting/luna.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"
#include "wui/unique_window_handler.h"

namespace LuaUi {

/* RST
:mod:`wl.ui`
=============

.. module:: wl.ui
   :synopsis: Provides access on user interface. Mainly for tutorials and
      debugging.

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.ui

.. Note::

   The objects inside this module can not be persisted. That is if the player
   tries to save the game while any of these objects are assigned to variables,
   the game will crash. So when using these, make sure that you only create
   objects for a short amount of time where the user can't take control to do
   something else.

*/

int upcasted_panel_to_lua(lua_State* L, UI::Panel* panel) {
	if (panel == nullptr) {
		return 0;
	}

	// TODO(Nordfriese): This trial-and-error approach is inefficient and extremely ugly,
	// use a virtual function call similar to Widelands::MapObjectDescr::type.
#define TRY_TO_LUA(PanelType, LuaType)                                                             \
	if (upcast(UI::PanelType, temp_##PanelType, panel)) {                                           \
		to_lua<LuaType>(L, new LuaType(temp_##PanelType));                                           \
	}

	// clang-format off
	TRY_TO_LUA(Window, LuaWindow)
	else TRY_TO_LUA(Button, LuaButton)
	else TRY_TO_LUA(Checkbox, LuaCheckbox)
	else TRY_TO_LUA(Radiobutton, LuaRadioButton)
	else TRY_TO_LUA(ProgressBar, LuaProgressBar)
	else TRY_TO_LUA(SpinBox, LuaSpinBox)
	else TRY_TO_LUA(Slider, LuaSlider)
	else if (upcast(UI::DiscreteSlider, temp_DiscreteSlider, panel)) {
		// Discrete sliders are wrapped, so we pass the actual slider through.
		to_lua<LuaSlider>(L, new LuaSlider(&temp_DiscreteSlider->get_slider()));
	}
	else TRY_TO_LUA(MultilineTextarea, LuaMultilineTextarea)
	else TRY_TO_LUA(Textarea, LuaTextarea)
	else TRY_TO_LUA(AbstractTextInputPanel, LuaTextInputPanel)
	else TRY_TO_LUA(TabPanel, LuaTabPanel)
	else TRY_TO_LUA(Tab, LuaTab)
	else TRY_TO_LUA(BaseDropdown, LuaDropdown)
	else TRY_TO_LUA(BaseListselect, LuaListselect)
	else TRY_TO_LUA(BaseTable, LuaTable)
	else {
		to_lua<LuaPanel>(L, new LuaPanel(panel));
	}
	// clang-format on
#undef TRY_TO_LUA

	return 1;
}

static std::string shortcut_string_if_set(const std::string& name, bool rt_escape) {
	if (name.empty()) {
		return std::string();
	}
	return shortcut_string_for(shortcut_from_string(name), rt_escape);
}

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/

/* RST
Panel
-----

.. class:: Panel

   The Panel is the most basic ui class. Each UI element is a panel.
*/
const char LuaPanel::className[] = "Panel";
const PropertyType<LuaPanel> LuaPanel::Properties[] = {
   PROP_RO(LuaPanel, children),   PROP_RO(LuaPanel, buttons), PROP_RO(LuaPanel, dropdowns),
   PROP_RO(LuaPanel, tabs),       PROP_RO(LuaPanel, windows), PROP_RW(LuaPanel, position_x),
   PROP_RW(LuaPanel, position_y), PROP_RW(LuaPanel, width),   PROP_RW(LuaPanel, height),
   PROP_RW(LuaPanel, visible),    PROP_RO(LuaPanel, name),    {nullptr, nullptr, nullptr},
};
const MethodType<LuaPanel> LuaPanel::Methods[] = {
   METHOD(LuaPanel, get_descendant_position),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaPanel, indicate),
#endif
   METHOD(LuaPanel, get_child),
   METHOD(LuaPanel, create_child),
   METHOD(LuaPanel, die),
   METHOD(LuaPanel, force_redraw),
   {nullptr, nullptr},
};

// Look for all descendant panels of class P and add the corresponding Lua version to the currently
// active Lua table. Class P needs to be a Panel.
template <class P, class LuaP>
static void put_all_visible_panels_into_table(lua_State* L, UI::Panel* g) {
	if (g == nullptr) {
		return;
	}

	for (UI::Panel* child = g->get_first_child(); child; child = child->get_next_sibling()) {
		put_all_visible_panels_into_table<P, LuaP>(L, child);

		if (upcast(P, specific_panel, child)) {
			if (specific_panel->is_visible()) {
				lua_pushstring(L, specific_panel->get_name());
				to_lua<LuaP>(L, new LuaP(specific_panel));
				lua_rawset(L, -3);
			}
		}
	}
}

/*
 * Properties
 */

/* RST
   .. attribute:: name

      .. versionadded:: 1.2

      (RO) The name of this panel.
*/
int LuaPanel::get_name(lua_State* L) {
	lua_pushstring(L, panel_->get_name());
	return 1;
}

/* RST
   .. attribute:: children

      .. versionadded:: 1.2

      (RO) An :class:`array` of all direct children of this panel.

      The array also contains invisible children. It does not contain indirect descendants.
*/
int LuaPanel::get_children(lua_State* L) {
	assert(panel_ != nullptr);

	lua_newtable(L);
	int i = 1;
	for (UI::Panel* child = panel_->get_first_child(); child != nullptr;
	     child = child->get_next_sibling()) {
		lua_pushint32(L, i++);
		upcasted_panel_to_lua(L, child);
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: buttons

      (RO) An :class:`array` of all visible buttons inside this Panel.
*/
int LuaPanel::get_buttons(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_visible_panels_into_table<UI::Button, LuaButton>(L, panel_);

	return 1;
}

/* RST
   .. attribute:: dropdowns

      (RO) An :class:`array` of all visible dropdowns inside this Panel.
*/
int LuaPanel::get_dropdowns(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_visible_panels_into_table<UI::BaseDropdown, LuaDropdown>(L, panel_);

	return 1;
}

/* RST
   .. attribute:: tabs

      (RO) An :class:`array` of all visible tabs inside this Panel.
*/
static void put_all_tabs_into_table(lua_State* L, UI::Panel* g) {
	if (g == nullptr) {
		return;
	}

	for (UI::Panel* f = g->get_first_child(); f != nullptr; f = f->get_next_sibling()) {
		put_all_tabs_into_table(L, f);

		if (upcast(UI::TabPanel, t, f)) {
			for (UI::Tab* tab : t->tabs()) {
				lua_pushstring(L, tab->get_name());
				to_lua<LuaTab>(L, new LuaTab(tab));
				lua_rawset(L, -3);
			}
		}
	}
}
int LuaPanel::get_tabs(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_tabs_into_table(L, panel_);

	return 1;
}

/* RST
   .. attribute:: windows

      (RO) A :class:`array` of all currently open windows that are
         children of this Panel.
*/
int LuaPanel::get_windows(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_visible_panels_into_table<UI::Window, LuaWindow>(L, panel_);

	return 1;
}

/* RST
   .. attribute:: width, height

      (RW) The dimensions of this panel in pixels
*/
int LuaPanel::get_width(lua_State* L) {
	assert(panel_);
	lua_pushint32(L, panel_->get_w());
	return 1;
}
int LuaPanel::set_width(lua_State* L) {
	assert(panel_);
	panel_->set_size(luaL_checkint32(L, -1), panel_->get_h());
	return 0;
}
int LuaPanel::get_height(lua_State* L) {
	assert(panel_);
	lua_pushint32(L, panel_->get_h());
	return 1;
}
int LuaPanel::set_height(lua_State* L) {
	assert(panel_);
	panel_->set_size(panel_->get_w(), luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: position_x, position_y

      (RO) The top left pixel of the our inner canvas relative to the
      parent's element inner canvas.
*/
int LuaPanel::get_position_x(lua_State* L) {
	assert(panel_);
	Vector2i p = panel_->to_parent(Vector2i::zero());

	lua_pushint32(L, p.x);
	return 1;
}
int LuaPanel::set_position_x(lua_State* L) {
	assert(panel_);
	Vector2i p(luaL_checkint32(L, -1) - panel_->get_lborder(), panel_->get_y());
	panel_->set_pos(p);
	return 0;
}
int LuaPanel::get_position_y(lua_State* L) {
	assert(panel_);
	Vector2i p = panel_->to_parent(Vector2i::zero());

	lua_pushint32(L, p.y);
	return 1;
}
int LuaPanel::set_position_y(lua_State* L) {
	assert(panel_);
	Vector2i p(panel_->get_x(), luaL_checkint32(L, -1) - panel_->get_tborder());
	panel_->set_pos(p);
	return 0;
}

/* RST
   .. attribute:: visible

      .. versionadded:: 1.2

      (RW) Whether this panel is visible to the user.
*/
int LuaPanel::get_visible(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(panel_->is_visible()));
	return 1;
}
int LuaPanel::set_visible(lua_State* L) {
	panel_->set_visible(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: get_descendant_position(child)

      Get the child position relative to the inner canvas of this Panel in
      pixels. Throws an error if child is not really a child.

      :arg child: children to get position for
      :type child: :class:`Panel`

      :returns: x, y
      :rtype: both are :class:`integers`
*/
int LuaPanel::get_descendant_position(lua_State* L) {
	assert(panel_);

	UI::Panel* cur = (*get_base_user_class<LuaPanel>(L, 2))->panel_;

	Vector2i cp = Vector2i::zero();
	while (cur != nullptr && cur != panel_) {
		cp += cur->to_parent(Vector2i::zero());
		cur = cur->get_parent();
	}

	if (cur == nullptr) {
		report_error(L, "Widget is not a descendant!");
	}

	lua_pushint32(L, cp.x);
	lua_pushint32(L, cp.y);
	return 2;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: indicate(on)

      Show/Hide an arrow that points to this panel. You can only point to 1 panel at the same time.

      :arg on: Whether to show or hide the arrow
      :type on: :class:`boolean`
*/
// UNTESTED
int LuaPanel::indicate(lua_State* L) {
	assert(panel_);
	if (lua_gettop(L) != 2) {
		report_error(L, "Expected 1 boolean");
	}

	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}

	const bool on = luaL_checkboolean(L, -1);
	if (on) {
		int x = panel_->get_x() + panel_->get_w();
		int y = panel_->get_y();
		UI::Panel* parent = panel_->get_parent();
		while (parent != nullptr) {
			x += parent->get_x() + parent->get_lborder();
			y += parent->get_y() + parent->get_tborder();
			parent = parent->get_parent();
		}
		ipl->set_training_wheel_indicator_pos(Vector2i(x, y));
	} else {
		ipl->set_training_wheel_indicator_pos(Vector2i::invalid());
	}
	return 2;
}
#endif

/* RST
   .. method:: die()

      .. versionadded:: 1.2

      Delete this panel in the next frame.
*/
int LuaPanel::die(lua_State* /* L */) {
	panel_->die();
	return 0;
}

/* RST
   .. method:: force_redraw()

      .. versionadded:: 1.2

      Force the screen to redraw immediately.

      .. warning:: Only call this during a blocking operation from a plugin, otherwise it
         will interfere with the regular redrawing and may result in glitches or crashes.
*/
int LuaPanel::force_redraw(lua_State* L) {
	if (!is_initializer_thread()) {
		report_error(L, "May only be called from plugins!");
	}

	panel_->do_redraw_now(false);
	return 0;
}

/* RST
   .. method:: get_child(name[, recursive=true])

      .. versionadded:: 1.2

      Get a panel by name that is a descendant of this panel.

      If the optional parameter is ``false``, only direct children of this panel are considered.

      :arg name: Name of the descendant to look for.
      :type child: :class:`string`
      :arg recursive: Also consider non-direct descendants.
      :type child: :class:`boolean`

      :returns: The child upcasted to the correct class, or ``nil`` if the child does not exist.
*/
int LuaPanel::get_child(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	bool recurse = lua_gettop(L) < 3 || luaL_checkboolean(L, 3);

	return upcasted_panel_to_lua(L, panel_->find_child_by_name(name, recurse));
}

/* RST
   .. method:: create_child(table)

      .. versionadded:: 1.2

      Create a UI widget as a child of this panel as specified by the provided table.

      A UI descriptor table contains multiple keys of type :class:`string`. Common properties are:

         * ``"widget"``: **Mandatory**. The type of widget to create. See below for allowed values.
         * ``"name"``: **Mandatory**. The internal name of the panel.
         * ``"x"``: **Optional**. The horizontal offset inside the parent from the left. Default: 0.
         * ``"y"``: **Optional**. The vertical offset inside the parent from the top. Default: 0.
         * ``"w"``: **Optional**. The widget's width. Default: automatic.
         * ``"h"``: **Optional**. The widget's height. Default: automatic.
         * ``"tooltip"``: **Optional**. The widget's tooltip.

         .. _resizing_strategies:

         * ``"resizing"``: **Optional**. If the parent component is a Box,
           the Resizing strategy to use for layouting. Valid values are:

           * ``"align"``: Use the widget's desired size. This is the default.
           * ``"fullsize"``: Use the widget's desired depth and the full available breadth.
           * ``"fillspace"``: Use the widget's desired breadth and the full available depth.
           * ``"expandboth"``: Use the full available space.

           A box's depth is its primary layout direction, and the breadth is the direction
           orthogonal to it. See: :ref:`Box widgets <box_description>`

         * ``"align"``: **Optional**. If the parent component is a Box,
           the Alignment strategy to use for layouting.
           Valid values are ``"center"`` (the default,) ``"left"``/``"top"``,
           and ``"right"``/``"bottom"``.
         * ``"on_panel_clicked"``: **Optional**. Callback code to run when the
           user clicks anywhere inside the widget.
         * ``"on_position_changed"``: **Optional**. Callback code to run when the
           widget's position changes.
         * ``"on_hyperlink"``: **Optional**. **New in version 1.3**. Callback code to run when
            the panel is the target of a hyperlink clicked by the user. The hyperlink's action
            argument will be stored in a global variable called ``HYPERLINK_ACTION``.
         * ``"children"``: **Optional**. An array of widget descriptor tables.

      Keys that are not supported by the widget type are silently ignored.

      Currently supported types of widgets are:

         * ``"window"``: A window. Windows can only be added to the top-level
           :class:`~wl.ui.MapView`. Properties:

           * ``"title"``: **Mandatory**. The title bar text.
           * ``"content"``: **Optional**. The main panel descriptor table.
             The window will resize itself to accommodate this widget perfectly.

         * ``"unique_window"``: A window of which only one instance can exist.
           If a unique window with the same registry as the provided one is already open,
           this function does nothing. Unique windows have all attributes of non-unique
           windows and the following additional property:

           * ``"registry"``: **Mandatory**. The window's unique registry identifier.

         .. _box_description:

         * ``"box"``: A wrapper around other components with intelligent layouting.
           It is strongly recommended to layout all your components
           exclusively with Boxes. Properties:

           * ``"orientation"``: **Mandatory**. The box's layouting direction:
             ``"vertical"`` or ``"horizontal"``.
             The shorthands ``"vert"``, ``"v"``, ``"horz"``, and ``"h"`` may be used.

             The size of a child component along this direction is called its *depth*, and the size
             of a child component along the direction orthogonal to it is called its *breadth*.

             See also: The :ref:`"resizing" <resizing_strategies>` property

           * ``"max_x"``: **Optional**. The maximum horizontal size. Default: unlimited.
           * ``"max_y"``: **Optional**. The maximum vertical size. Default: unlimited.
           * ``"spacing"``: **Optional**. The inner spacing between items. Default: 0.
           * ``"scrolling"``: **Optional**. Whether the box may scroll if its content
             is too large. Default: false.

         * ``"inf_space"``: Only valid as the direct child of a Box. A flexible spacer.
            Takes no properties.
         * ``"space"``: Only valid as the direct child of a Box. A fixed-size spacer.
            Only accepted property:

           * ``"value"``: **Mandatory**. The size of the space.

         * ``"panel"``: A plain panel that can act as a spacer or as a container for other widgets.

         * ``"textarea"``: A static text area with a single line of text. Properties:

           * ``"text"``: **Mandatory**. The text to display.
           * ``"font"``: **Mandatory**. The font style to use. See :ref:`theme_fonts`
           * ``"text_align"``: **Optional**. The alignment of the text. Valid values are
             ``"center"`` (the default), ``"left"``, and ``"right"``.
           * ``"fixed_width"``: **Optional**. If set, the text area's width is fixed instead
             of resizing to accomodate the text or the parent. Default: not set.

         * ``"multilinetextarea"``: A static text area displaying multiple lines of text.
           Properties:

           * ``"text"``: **Mandatory**. The text to display. The text can be formatted with richtext
             (see :ref:`richtext.lua`), in which case the ``font`` attribute will have no effect.
           * ``"scroll_mode"``: **Mandatory**. The text area's scrolling behaviour. One of:

             * ``"none"``: The text area expands to accommodate its content instead of scrolling.
             * ``"normal"``: Explicit scrolling only.
             * ``"normal_force"``: Explicit and forced scrolling.
             * ``"log"``: Follow the bottom of the log.
             * ``"log_force"``: Follow the bottom of the log with forced scrolling.

           * ``"font"``: **Optional**. The font style to use. See :ref:`theme_fonts`
           * ``"text_align"``: **Optional**. The alignment of the text. Valid values are
             ``"center"`` (the default), ``"left"``, and ``"right"``.

         .. _button_description:

         * ``"button"``: A clickable button. A button must have either a title or an icon,
           but not both. Properties:

           * ``"title"``: **Optional**. The text on the button.
           * ``"icon"``: **Optional**. The file path of the button's icon.
           * ``"style"``: **Optional**. The button's style. One of:

             * ``"primary"``
             * ``"secondary"`` (default)
             * ``"menu"``

           * ``"visual"``: **Optional**. The button's appearance. One of:

             * ``"raised"`` (default)
             * ``"permpressed"``
             * ``"flat"``

           * ``"repeating"``: **Optional**. Whether pressing and holding
             the button generates repeated events. Default: false.

           * ``"on_click"``: **Optional**. Callback code to run when the
             button is clicked. To associate actions with a button press,
             prefer this over the ``on_panel_clicked`` event.

         * ``"checkbox"``: A toggleable checkbox. A checkbox must have either a title or an icon,
           but not both. Properties:

           * ``"title"``: **Optional**. The text on the button.
           * ``"icon"``: **Optional**. The file path of the button's icon.
           * ``"state"``: **Optional**. Whether the checkbox is initially checked.
           * ``"on_changed"``: **Optional**. Callback code to run when the checkbox's state changes.

         * ``"radiogroup"``: A group of pictorial buttons of which at most one can be selected
           at the same time. Properties:

           * ``"buttons"``: **Mandatory**. An array of tables, each of which defines a radiobutton
             with the following properties:

             * ``"name"``: **Mandatory**. The internal name of the button.
             * ``"icon"``: **Mandatory**. The file path of the radiobutton's icon.
             * ``"tooltip"``: **Optional**. The radiobutton's tooltip.
             * ``"x"``: **Optional**. The radiobutton's x position.
             * ``"y"``: **Optional**. The radiobutton's y position.

           * ``"state"``: **Optional**. The ID of the initially active radiobutton
             (0-based; -1 for none).
           * ``"on_changed"``: **Optional**. Callback code to run when the
             radiogroup's active button changes.

           This widget does not use the sizing and positioning properties and has no tooltip.

         * ``"progressbar"``: A partially filled bar that indicates the progress
           of an operation. Properties:

           * ``"orientation"``: **Mandatory**. The progress bar's direction:
             ``"vertical"`` or ``"horizontal"``.
             The shorthands ``"vert"``, ``"v"``, ``"horz"``, and ``"h"`` may be used.
           * ``"total"``: **Mandatory**. The progress bar's maximum value.
           * ``"state"``: **Mandatory**. The progress bar's initial value.
           * ``"percent"``: **Optional**. Whether to show a percentage instead of absolute values.
             Default: :const:`true`.

         * ``"spinbox"``: A box with buttons to increase or decrease a numerical value. There are
           two kinds of spinboxes by their value ranges: normal spinboxes that can have any integer
           value within a range, and spinboxes that can only use values from a value list. Some
           properties are only used by one kind of spinbox and forbidden for the other kind.

           * Common Properties:

             * ``"unit_w"``: **Mandatory**. The total width of the buttons and value display.
             * ``"value"``: **Mandatory**. The spinbox's initial value. When ``"values"`` is used,
               this is a 0 based index within the value list.
             * ``"label"``: **Optional**. Text to display next to the spinbox.
             * ``"units"``: **Optional**. The unit for the spinbox's value. One of:

               * ``"none"`` (default)
               * ``"pixels"``
               * ``"percent"``
               * ``"fields"``
               * ``"minutes"``
               * ``"weeks"``

             * ``"replacements"``: **Optional**. An array of tables with keys ``"value"`` and
               ``"replacement"``. When the spinbox's value is equal to any replaced value,
               the replacement string is displayed instead of the value.
             * ``"on_changed"``: **Optional**. Callback code to run when the spinbox's value
               changes.

           * Properties for normal spinboxes:

             * ``"min"``: **Mandatory** for normal spinboxes. The spinbox's minimum value.
             * ``"max"``: **Mandatory** for normal spinboxes. The spinbox's maximum value.
             * ``"step_size_small"``: **Optional**.
               The amount by which the value changes on each button click.
             * ``"step_size_big"``: **Optional**. If set, the spinbox additionally shows
               buttons to change the value by this larger amount.

           * Properties for value list spinboxes:

             * ``"values"``: **Mandatory** to set the type to value list. An array of integers.
               The spinbox can only switch between the values in this array.

             The properties for normal spinboxes above cannot be used when ``"values"`` is used.

           This widget can not have a custom tooltip.

         * ``"slider"``: A button that can be slid along a line to change a value. Properties:

           * ``"orientation"``: **Mandatory**. The slider's direction:
             ``"vertical"`` or ``"horizontal"``.
             The shorthands ``"vert"``, ``"v"``, ``"horz"``, and ``"h"`` may be used.
           * ``"value"``: **Mandatory**. The slider's initial value.
           * ``"min"``: **Mandatory**. The slider's minimum value.
           * ``"max"``: **Mandatory**. The slider's maximum value.
           * ``"cursor_size"``: **Optional**. The size of the slider button in pixels (default 20).
           * ``"dark"``: **Optional**. Ignored in the main menu.
             Draw the slider darker instead of lighter (default :const:`false`).
           * ``"on_changed"``: **Optional**. Callback code to run when the slider's value changes.

         * ``"discrete_slider"``: A button that can be slid along a horizontal line to change
           a value between several predefined points. Properties:

           * ``"labels"``: **Mandatory**. Array of strings. Each string defines one slider point.
           * ``"value"``: **Mandatory**. The initially selected value.
           * ``"cursor_size"``: **Optional**. The size of the slider button in pixels (default 20).
           * ``"dark"``: **Optional**. Ignored in the main menu.
             Draw the slider darker instead of lighter (default :const:`false`).
           * ``"on_changed"``: **Optional**. Callback code to run when the slider's value changes.

         * ``"multilineeditbox"``: A multi-line field where the user can enter text. Properties:

           * ``"text"``: **Optional**. The initial text in the field.
           * ``"password"``: **Optional**. Whether the editbox's content will be hidden.
           * ``"warning"``: **Optional**. Whether the editbox will be highlighted as having errors.
           * ``"on_changed"``: **Optional**. Callback code to run whenever the content changes.
           * ``"on_cancel"``: **Optional**. Callback code to run whenever the user presses Escape.

         * ``"editbox"``: A single-line field where the user can enter text.
           Has all the properties of multilineeditbox and the following additional property:

           * ``"on_ok"``: **Optional**. Callback code to run whenever the user presses Return.

         * ``"dropdown"``: A dropdown that allows the user to select an entry from a popup list.
           Properties:

           * ``"label"``: **Mandatory**. The text on the dropdown.
           * ``"max_list_items"``: **Mandatory**. Maximum number of items to show in the list.
           * ``"button_dimension"``: **Mandatory**. Size of the button in pixels.
           * ``"button_style"``: **Mandatory**. The style for the button.
             See: :ref:`Button widgets <button_description>`.
           * ``"type"``: **Mandatory**. The behaviour of the dropdown. One of:

             * ``"textual"``: Shows the name of the selected entry and a push button.
             * ``"textual_narrow"``: Only shows the name of the selected entry.
             * ``"pictorial"``: Only shows the icon of the selected entry.
             * ``"pictorial_menu"``: The shown icon remains always unchanged.
             * ``"textual_menu"``: The shown text remains always unchanged.

           * ``"datatype"``: **Mandatory**. The data type of the dropdown's entries.
             Currently only ``"string"`` is supported.

           * ``"icon"``: **Optional**. The icon filepath for the dropdown's button, if any.
             If an icon is set, the label or selected value are not displayed on the
             button and only shown as a tooltip.

           * ``"entries"``: **Optional**. The entries in the dropdown.
             An array of tables with the following keys:

             * ``"label"``: **Mandatory**. The text for this entry.
             * ``"value"``: **Mandatory**. The internal value of this entry.
             * ``"icon"``: **Mandatory** for dropdowns of type ``"pictorial"``,
               **optional** for other types. The icon filepath for the entry.
             * ``"tooltip"``: **Optional**. The entry's tooltip.
             * ``"select"``: **Optional**. Whether to select this entry (default :const:`false`).
             * ``"hotkey"``: **Optional**. **New in version 1.3**.
                The internal name of the hotkey for this entry.

           * ``"on_selected"``: **Optional**. Callback code to run when the user selects an entry.

         * ``"listselect"``: A list of entries from which the user may choose. Properties:

           * ``"type"``: **Optional**. The highlighting mode for the selected entry. One of:

             * ``"plain"``: Highlight the selection's background (default).
             * ``"check"``: Draw a check mark before the selected item.
             * ``"dropdown"``: Select entries as soon as the mouse hovers over them.

           * ``"datatype"``: **Mandatory**. The data type of the listselect's entries.
             Currently only ``"string"`` is supported.

           * ``"entries"``: **Optional**. The entries in the listselect.
             An array of tables with the following keys:

             * ``"label"``: **Mandatory**. The text for this entry.
             * ``"value"``: **Mandatory**. The internal value of this entry.
             * ``"icon"``: **Optional**. The icon filepath for the entry.
             * ``"tooltip"``: **Optional**. The entry's tooltip.
             * ``"select"``: **Optional**. Whether to select this entry (default :const:`false`).
             * ``"indent"``: **Optional**. How many levels to indent the item (default 0).
             * ``"enable"``: **Optional**. **New in version 1.3**.
               Whether to enable this entry (default :const:`true`).
             * ``"hotkey"``: **Optional**. **New in version 1.3**.
                The internal name of the hotkey for this entry.

           * ``"on_selected"``: **Optional**. Callback code to run when the user selects an entry.
           * ``"on_double_clicked"``: **Optional**.
             Callback code to run when the user double-clicks on an entry.

           This widget can not have a custom tooltip.

         * ``"table"``: A table with multiple rows and columns. Properties:

           * ``"datatype"``: **Mandatory**. The data type of the table's entries.
             Currently only ``"int"`` is supported.
           * ``"multiselect"``: **Optional**.
             Whether the user can select multiple rows (default :const:`false`).
           * ``"sort_column"``: **Optional**.
             The index of the column by which the table is sorted (default 0).
           * ``"sort_descending"``: **Optional**.
             Whether the sorting order is inverted (default :const:`false`).
           * ``"columns"``: **Optional**. The columns in the table.
             An array of tables with the following keys:

             * ``"w"``: **Mandatory**. The width of the column in pixels.
             * ``"title"``: **Mandatory**. The title of the column.
             * ``"tooltip"``: **Optional**. The tooltip in pixels.
             * ``"flexible"``: **Optional**.
               Whether the column width adapts automatically (default :const:`false`).
               A table must not have multiple flexible columns.
             * ``"align"``: **Optional**. The alignment of the column content. Valid values are
               ``"center"`` (the default), ``"left"``, and ``"right"``.

           * ``"rows"``: **Optional**. The rows in the table.
             An array of tables with the following keys:

             * ``"value"``: **Mandatory**. The internal value associated with the row.
             * ``"select"``: **Optional**. Whether to select this row (default :const:`false`).
             * ``"disable"``: **Optional**. Whether to disable this row (default :const:`false`).
             * For each column ``i`` (indices are zero-based):

               * ``"text_<i>"``: **Optional**. The text to show in the cell.
               * ``"icon_<i>"``: **Optional**. The icon filepath in the cell.

           * ``"on_cancel"``: **Optional**. Callback code to run when the user presses Escape.
           * ``"on_selected"``: **Optional**. Callback code to run when the user selects a row.
           * ``"on_double_clicked"``: **Optional**.
             Callback code to run when the user double-clicks on an entry.

           This widget can not have a custom tooltip.

         * ``"tabpanel"``: A panel that allows switching between multiple tabs.

           * ``"dark"``: **Optional**. Ignored in the main menu.
             Whether to use dark appearance (default :const:`false`).
           * ``"active"``: **Optional**. The name or index of the initially active tab.
           * ``"tabs"``: **Optional**. The tabs in the tab panel.
             An array of tables with the following keys:

             * ``"name"``: **Mandatory**. The name of the tab.
             * ``"panel"``: **Mandatory**. The descriptor table for the panel wrapped in this tab.
             * ``"title"``: **Optional**. The title of the tab.
               If set, ``"icon"`` must be ``nil``.
             * ``"icon"``: **Optional**. The icon filepath for the tab.
               If set, ``"title"`` must be ``nil``.
             * ``"tooltip"``: **Optional**. The tooltip for the tab.

           * ``"on_clicked"``: **Optional**. Callback code to run when the user selects a tab.

      It is strongly recommended to use box layouting for all widgets.
      All widgets that are the child of a Box widget are layouted automatically by the Box.
      Their x and y positions will be ignored, and their width and/or height may be changed in
      accordance with their resizing strategy in order to optimally layout all components.

      If a widget appears to be missing, usually its width and/or height has been determined
      as zero. In this case, setting a minimum width and height or using a more dynamic
      resizing strategy will fix the widget's layout.

      Note that event callbacks functions must be provided as raw code in string form.
      During the lifetime of a *toolbar* widget, the Lua Interface used by the game may be reset.
      Therefore, any callbacks attached to such widgets must not use any functions or variables
      defined at an arbitrary earlier time by your script -
      they may have been deleted by the time the callback is invoked.

      Similarly, in the main menu, a plugin's init script may be called multiple times
      without resetting the user interface in the mean time. Therefore, the script
      needs to check whether the elements it intends to add already exist from an
      earlier invocation using a different Lua context.

      Example:

      .. code-block:: lua

         push_textdomain("yes_no.wad", true)

         local mv = wl.ui.MapView()

         -- Create a toolbar button
         mv.toolbar:create_child({
            widget   = "button",
            name     = "yes_no_toolbar_button",
            w        = styles.get_size("toolbar_button_size"),
            h        = styles.get_size("toolbar_button_size"),
            tooltip  = _("Yes or No"),
            icon     = "images/ui_basic/different.png",
            -- Callback code to run when the user presses the button
            on_click = [[
               push_textdomain("yes_no.wad", true)
               -- Open a new window
               wl.ui.MapView():create_child({
                  widget   = "window",
                  name     = "yes_or_no_window",
                  title    = _("Yes or No"),
                  x        = wl.ui.MapView().width  // 2,
                  y        = wl.ui.MapView().height // 2,
                  -- The window's content
                  content  = {
                     -- The window's central panel: A box with three children
                     widget      = "box",
                     orientation = "vert",
                     name        = "main_box",
                     children    = {
                        {
                           widget = "textarea",
                           font   = "wui_info_panel_paragraph",
                           text   = _("Click Yes or No"),
                           name   = "label_yes_no",
                        },
                        {
                           -- Space between the text and the buttons is also created like a widget
                           widget = "space",
                           value  = 10,
                        },
                        {
                           -- Place the buttons side by side in a horizontal box
                           widget = "box",
                           orientation = "horz",
                           name   = "buttons_box",
                           children = {
                              {
                                 widget   = "button",
                                 name     = "no",
                                 title    = _("No"),
                                 on_click = [=[
                                    wl.ui.show_messagebox(_("Hello"), _("You clicked no!"), false)
                                 ]=],
                              },
                              {
                                 widget = "space",
                                 value  = 10,
                              },
                              {
                                 widget   = "button",
                                 name     = "yes",
                                 title    = _("Yes"),
                                 on_click = [=[
                                    wl.ui.show_messagebox(_("Hello"), _("You clicked yes!"))
                                 ]=],
                              },
                           },
                        },
                     }
                  }
               })
               pop_textdomain()
            ]]
         })

         mv:update_toolbar()

         pop_textdomain()

*/
int LuaPanel::create_child(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes exactly one argument");
	}
	if (panel_ == nullptr) {
		report_error(L, "Parent does not exist");
	}

	try {
		UI::Panel* new_panel = do_create_child(L, panel_, dynamic_cast<UI::Box*>(panel_));
		new_panel->initialization_complete();
	} catch (const std::exception& e) {
		report_error(L, "Could not create child: %s", e.what());
	}

	return 0;
}

/*
 * C Functions
 */
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

/* RST
Button
------

.. class:: Button

   This represents a simple push button.
*/
const char LuaButton::className[] = "Button";
const MethodType<LuaButton> LuaButton::Methods[] = {
   METHOD(LuaButton, press),         METHOD(LuaButton, click),
   METHOD(LuaButton, set_repeating), METHOD(LuaButton, set_perm_pressed),
   METHOD(LuaButton, toggle),        {nullptr, nullptr},
};
const PropertyType<LuaButton> LuaButton::Properties[] = {
   PROP_RW(LuaButton, title),
   PROP_RW(LuaButton, enabled),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: title

      .. versionadded:: 1.2

      (RW) The text shown on the button.
*/
int LuaButton::get_title(lua_State* L) {
	lua_pushstring(L, get()->get_title().c_str());
	return 1;
}
int LuaButton::set_title(lua_State* L) {
	get()->set_title(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: enabled

      .. versionadded:: 1.2

      (RW) Whether the user may interact with the button.
*/
int LuaButton::get_enabled(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->enabled()));
	return 1;
}
int LuaButton::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: press()

      Press and hold this button. This is mainly to visualize a pressing
      event in tutorials.

      Holding a button does not generate a Clicked event.
      Use :meth:`click` to release the button.
*/
int LuaButton::press(lua_State* /* L */) {
	log_info("Pressing button '%s'\n", get()->get_name().c_str());
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
   .. method:: click()

      Click this button just as if the user would have moused over and clicked
      it.
*/
int LuaButton::click(lua_State* /* L */) {
	log_info("Clicking button '%s'\n", get()->get_name().c_str());
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}

/* RST
   .. method:: set_repeating(b)

      .. versionadded:: 1.2

      Set whether holding the button will generate repeated clicked events.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaButton::set_repeating(lua_State* L) {
	get()->set_repeating(luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: set_perm_pressed(b)

      .. versionadded:: 1.2

      Set whether the button will be styled as if permanently pressed.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaButton::set_perm_pressed(lua_State* L) {
	get()->set_perm_pressed(luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: toggle()

      .. versionadded:: 1.2

      Toggle whether the button will be styled as if permanently pressed.
*/
int LuaButton::toggle(lua_State* /* L */) {
	get()->toggle();
	return 0;
}

/*
 * C Functions
 */

/* RST
Checkbox
--------

.. class:: Checkbox

   .. versionadded:: 1.2

   A tick box that can be toggled on or off by the user.
*/
const char LuaCheckbox::className[] = "Checkbox";
const MethodType<LuaCheckbox> LuaCheckbox::Methods[] = {
   METHOD(LuaCheckbox, set_enabled),
   {nullptr, nullptr},
};
const PropertyType<LuaCheckbox> LuaCheckbox::Properties[] = {
   PROP_RW(LuaCheckbox, state),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) Whether the checkbox is currently checked.
*/
int LuaCheckbox::get_state(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_state()));
	return 1;
}
int LuaCheckbox::set_state(lua_State* L) {
	get()->set_state(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can change the state of this checkbox.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaCheckbox::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * C Functions
 */

/* RST
RadioButton
-----------

.. class:: RadioButton

   .. versionadded:: 1.2

   One of the buttons in a radio group.
   In each radio group, at most one button can be active at the same time.

   Note that each button in the group also acts as a representation of the group itself.
*/
const char LuaRadioButton::className[] = "RadioButton";
const MethodType<LuaRadioButton> LuaRadioButton::Methods[] = {
   METHOD(LuaRadioButton, set_enabled),
   {nullptr, nullptr},
};
const PropertyType<LuaRadioButton> LuaRadioButton::Properties[] = {
   PROP_RW(LuaRadioButton, state),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) The index of the radio group's currently active button (0-based; -1 for none).
*/
int LuaRadioButton::get_state(lua_State* L) {
	lua_pushinteger(L, get()->group().get_state());
	return 1;
}
int LuaRadioButton::set_state(lua_State* L) {
	get()->group().set_state(luaL_checkint32(L, -1), true);
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can change the state of the radio group.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaRadioButton::set_enabled(lua_State* L) {
	get()->group().set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * C Functions
 */

/* RST
ProgressBar
-----------

.. class:: ProgressBar

   .. versionadded:: 1.2

   A partially filled bar that indicates the progress of an operation.
*/
const char LuaProgressBar::className[] = "ProgressBar";
const MethodType<LuaProgressBar> LuaProgressBar::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaProgressBar> LuaProgressBar::Properties[] = {
   PROP_RW(LuaProgressBar, state),
   PROP_RW(LuaProgressBar, total),
   PROP_RW(LuaProgressBar, show_percent),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) The current progress value.

      .. Note:: When using a progress bar to visualize the progress of a blocking script,
         you may need to call :meth:`~Panel.force_redraw` after changing this property to ensure
         the change becomes visible to the user immediately.
*/
int LuaProgressBar::get_state(lua_State* L) {
	lua_pushinteger(L, get()->get_state());
	return 1;
}
int LuaProgressBar::set_state(lua_State* L) {
	get()->set_state(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: total

      (RW) The maximum progress value.
*/
int LuaProgressBar::get_total(lua_State* L) {
	lua_pushinteger(L, get()->get_total());
	return 1;
}
int LuaProgressBar::set_total(lua_State* L) {
	get()->set_total(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: show_percent

      (RW) Whether the progress bar label displays the absolute progress
      or the percentage completed.
*/
int LuaProgressBar::get_show_percent(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_show_percent()));
	return 1;
}
int LuaProgressBar::set_show_percent(lua_State* L) {
	get()->set_show_percent(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

/* RST
SpinBox
-------

.. class:: SpinBox

   .. versionadded:: 1.2

   A box with buttons to increase or decrease a numerical value.
*/
const char LuaSpinBox::className[] = "SpinBox";
const MethodType<LuaSpinBox> LuaSpinBox::Methods[] = {
   METHOD(LuaSpinBox, set_unit_width),
   METHOD(LuaSpinBox, set_interval),
   METHOD(LuaSpinBox, add_replacement),
   {nullptr, nullptr},
};
const PropertyType<LuaSpinBox> LuaSpinBox::Properties[] = {
   PROP_RW(LuaSpinBox, value),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: value

      (RW) The currently selected value.
*/
int LuaSpinBox::get_value(lua_State* L) {
	lua_pushinteger(L, get()->get_value());
	return 1;
}
int LuaSpinBox::set_value(lua_State* L) {
	get()->set_value(luaL_checkint32(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_unit_width(w)

      Set the width of the spinbox's buttons and content.

      :arg w: Width in pixels.
      :type min: :class:`int`
*/
int LuaSpinBox::set_unit_width(lua_State* L) {
	get()->set_unit_width(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. function:: set_interval(min, max)

      Set the minimum and maximum value of the spinbox.

      :arg min: Minimum value.
      :type min: :class:`int`
      :arg max: Maximum value.
      :type max: :class:`int`
*/
int LuaSpinBox::set_interval(lua_State* L) {
	get()->set_interval(luaL_checkint32(L, 2), luaL_checkint32(L, 3));
	return 0;
}

/* RST
   .. function:: add_replacement(value, label)

      Replacement string to display instead of a specific value when that value is selected.

      :arg value: Value to replace.
      :type value: :class:`int`
      :arg label: Replacement text.
      :type label: :class:`string`
*/
int LuaSpinBox::add_replacement(lua_State* L) {
	get()->add_replacement(luaL_checkint32(L, 2), luaL_checkstring(L, 3));
	return 0;
}

/*
 * C Functions
 */

/* RST
Slider
------

.. class:: Slider

   .. versionadded:: 1.2

   A button that can be slid along a line to change a value.
*/
const char LuaSlider::className[] = "Slider";
const MethodType<LuaSlider> LuaSlider::Methods[] = {
   METHOD(LuaSlider, set_enabled),
   METHOD(LuaSlider, set_cursor_fixed_height),
   {nullptr, nullptr},
};
const PropertyType<LuaSlider> LuaSlider::Properties[] = {
   PROP_RW(LuaSlider, value),
   PROP_RW(LuaSlider, min_value),
   PROP_RW(LuaSlider, max_value),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: value

      (RW) The currently selected value.
*/
int LuaSlider::get_value(lua_State* L) {
	lua_pushinteger(L, get()->get_value());
	return 1;
}
int LuaSlider::set_value(lua_State* L) {
	get()->set_value(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: min_value

      (RW) The lowest selectable value.
*/
int LuaSlider::get_min_value(lua_State* L) {
	lua_pushinteger(L, get()->get_min_value());
	return 1;
}
int LuaSlider::set_min_value(lua_State* L) {
	get()->set_min_value(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: max_value

      (RW) The highest selectable value.
*/
int LuaSlider::get_max_value(lua_State* L) {
	lua_pushinteger(L, get()->get_max_value());
	return 1;
}
int LuaSlider::set_max_value(lua_State* L) {
	get()->set_max_value(luaL_checkint32(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can move this slider.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaSlider::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. function:: set_cursor_fixed_height(h)

      Set the slider cursor's height.

      :arg h: Height in pixels.
      :type h: :class:`int`
*/
int LuaSlider::set_cursor_fixed_height(lua_State* L) {
	get()->set_cursor_fixed_height(luaL_checkint32(L, -1));
	return 0;
}

/*
 * C Functions
 */

/* RST
MultilineTextarea
-----------------

.. class:: MultilineTextarea

   .. versionadded:: 1.2

   A static text area with multiple lines that can not be modified by the user.
*/
const char LuaMultilineTextarea::className[] = "MultilineTextarea";
const MethodType<LuaMultilineTextarea> LuaMultilineTextarea::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMultilineTextarea> LuaMultilineTextarea::Properties[] = {
   PROP_RW(LuaMultilineTextarea, text),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: text

      (RW) The text currently shown by this area.
*/
int LuaMultilineTextarea::get_text(lua_State* L) {
	lua_pushstring(L, get()->get_text().c_str());
	return 1;
}
int LuaMultilineTextarea::set_text(lua_State* L) {
	get()->set_text(luaL_checkstring(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

/* RST
Textarea
--------

.. class:: Textarea

   .. versionadded:: 1.2

   A static text area with a single line of text that can not be modified by the user.
*/
const char LuaTextarea::className[] = "Textarea";
const MethodType<LuaTextarea> LuaTextarea::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTextarea> LuaTextarea::Properties[] = {
   PROP_RW(LuaTextarea, text),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: text

      (RW) The text currently shown by this area.
*/
int LuaTextarea::get_text(lua_State* L) {
	lua_pushstring(L, get()->get_text().c_str());
	return 1;
}
int LuaTextarea::set_text(lua_State* L) {
	get()->set_text(luaL_checkstring(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

/* RST
TextInputPanel
--------------

.. class:: TextInputPanel

   .. versionadded:: 1.2

   This represents a box containing arbitrary text that can be edited by the user.
*/
const char LuaTextInputPanel::className[] = "TextInputPanel";
const MethodType<LuaTextInputPanel> LuaTextInputPanel::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTextInputPanel> LuaTextInputPanel::Properties[] = {
   PROP_RW(LuaTextInputPanel, text),      PROP_RO(LuaTextInputPanel, selected_text),
   PROP_RW(LuaTextInputPanel, password),  PROP_RW(LuaTextInputPanel, warning),
   PROP_RW(LuaTextInputPanel, caret_pos), PROP_RO(LuaTextInputPanel, multiline),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: text

      (RW) The text currently held in this box.
*/
int LuaTextInputPanel::get_text(lua_State* L) {
	lua_pushstring(L, get()->get_text().c_str());
	return 1;
}
int LuaTextInputPanel::set_text(lua_State* L) {
	get()->set_text(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: selected_text

      (RO) The text currently selected by the user (may be empty).
*/
int LuaTextInputPanel::get_selected_text(lua_State* L) {
	lua_pushstring(L, get()->get_selected_text().c_str());
	return 1;
}

/* RST
   .. attribute:: password

      (RW) Whether the actual text is displayed with asterisks for password boxes.
*/
int LuaTextInputPanel::get_password(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_password()));
	return 1;
}
int LuaTextInputPanel::set_password(lua_State* L) {
	get()->set_password(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: warning

      (RW) Whether the box is highlighted to indicate an error.
*/
int LuaTextInputPanel::get_warning(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->has_warning()));
	return 1;
}
int LuaTextInputPanel::set_warning(lua_State* L) {
	get()->set_warning(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: caret_pos

      (RW) The position of the caret in the text.
*/
int LuaTextInputPanel::get_caret_pos(lua_State* L) {
	lua_pushinteger(L, get()->get_caret_pos());
	return 1;
}
int LuaTextInputPanel::set_caret_pos(lua_State* L) {
	get()->set_caret_pos(luaL_checkuint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: multiline

      (RO) Whether this editbox has multiple lines or just one line.
*/
int LuaTextInputPanel::get_multiline(lua_State* L) {
	lua_pushboolean(L, dynamic_cast<const UI::MultilineEditbox*>(get()) != nullptr ? 1 : 0);
	return 1;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

/* RST
Dropdown
--------

.. class:: Dropdown

   This represents a dropdown menu.

   Some attributes and functions are available only for dropdowns with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaDropdown::className[] = "Dropdown";
const MethodType<LuaDropdown> LuaDropdown::Methods[] = {
   METHOD(LuaDropdown, open),           METHOD(LuaDropdown, highlight_item),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaDropdown, indicate_item),
#endif
   METHOD(LuaDropdown, select),         METHOD(LuaDropdown, add),
   METHOD(LuaDropdown, get_value_at),   METHOD(LuaDropdown, get_label_at),
   METHOD(LuaDropdown, get_tooltip_at), {nullptr, nullptr},
};
const PropertyType<LuaDropdown> LuaDropdown::Properties[] = {
   PROP_RO(LuaDropdown, datatype),    PROP_RO(LuaDropdown, expanded),
   PROP_RO(LuaDropdown, no_of_items), PROP_RO(LuaDropdown, selection),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      .. versionadded:: 1.2

      (RO) The dropdown's datatype as :class:`string` if supported,
      or ``nil`` for dropdowns with unsupported datatype.

      Currently only ``"string"`` is supported.
*/
int LuaDropdown::get_datatype(lua_State* L) {
	if (dynamic_cast<const DropdownOfString*>(get()) != nullptr) {
		lua_pushstring(L, "string");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: expanded

      (RO) True if the dropdown's list is currently expanded.
*/
int LuaDropdown::get_expanded(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_expanded()));
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The number of items this dropdown has.
*/
int LuaDropdown::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection

      .. versionadded:: 1.2

      (RO) The currently selected entry or ``nil`` if none is selected.
      For dropdowns with unsupported datatype, this is always ``nil``.
*/
int LuaDropdown::get_selection(lua_State* L) {
	if (upcast(const DropdownOfString, dd, get()); dd != nullptr && dd->has_selection()) {
		lua_pushstring(L, dd->get_selected().c_str());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: open

      Open this dropdown menu.
*/
int LuaDropdown::open(lua_State* /* L */) {
	log_info("Opening dropdown '%s'\n", get()->get_name().c_str());
	get()->set_list_visibility(true);
	return 0;
}

/* RST
   .. method:: highlight_item(index)

      :arg index: the index of the item to highlight, starting from ``1``
      :type index: :class:`integer`

      Highlights an item in this dropdown without triggering a selection.
*/
int LuaDropdown::highlight_item(lua_State* L) {
	unsigned int desired_item = luaL_checkuint32(L, -1);
	if (desired_item < 1 || desired_item > get()->size()) {
		report_error(L,
		             "Attempted to highlight item %d on dropdown '%s'. Available range for this "
		             "dropdown is 1-%d.",
		             desired_item, get()->get_name().c_str(), get()->size());
	}
	log_info("Highlighting item %d in dropdown '%s'\n", desired_item, get()->get_name().c_str());
	// Open the dropdown
	get()->set_list_visibility(true);

	SDL_Keysym code;
	// Ensure that we're at the top
	code.sym = SDLK_UP;
	code.scancode = SDL_SCANCODE_UP;
	code.mod = KMOD_NONE;
	code.unused = 0;
	for (size_t i = 1; i < get()->size(); ++i) {
		get()->handle_key(true, code);
	}
	// Press arrow down until the desired item is highlighted
	code.sym = SDLK_DOWN;
	code.scancode = SDL_SCANCODE_DOWN;
	for (size_t i = 1; i < desired_item; ++i) {
		get()->handle_key(true, code);
	}
	return 0;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: indicate_item(index)

      :arg index: the index of the item to indicate, starting from ``1``
      :type index: :class:`integer`

      Show an arrow that points to an item in this dropdown. You can only point to 1 panel at the
      same time.
*/
int LuaDropdown::indicate_item(lua_State* L) {
	assert(panel_);
	if (lua_gettop(L) != 2) {
		report_error(L, "Expected 1 int");
	}

	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}

	size_t desired_item = luaL_checkuint32(L, -1);
	if (desired_item < 1 || desired_item > get()->size()) {
		report_error(L,
		             "Attempted to indicate item %" PRIuS
		             " on dropdown '%s'. Available range for this "
		             "dropdown is 1-%d.",
		             desired_item, get()->get_name().c_str(), get()->size());
	}
	log_info(
	   "Indicating item %" PRIuS " in dropdown '%s'\n", desired_item, get()->get_name().c_str());

	int x = panel_->get_x() + panel_->get_w();
	int y = panel_->get_y();
	UI::Panel* parent = panel_->get_parent();
	while (parent != nullptr) {
		x += parent->get_x() + parent->get_lborder();
		y += parent->get_y() + parent->get_tborder();
		parent = parent->get_parent();
	}

	// Open the dropdown
	get()->set_list_visibility(true);

	for (; desired_item <= get()->size(); ++desired_item) {
		y -= get()->lineheight();
	}

	ipl->set_training_wheel_indicator_pos(Vector2i(x, y));

	return 0;
}
#endif

/* RST
   .. method:: select()

      Selects the currently highlighted item in this dropdown.
*/
int LuaDropdown::select(lua_State* /* L */) {
	log_info("Selecting current item in dropdown '%s'\n", get()->get_name().c_str());
	SDL_Keysym code;
	code.sym = SDLK_RETURN;
	code.scancode = SDL_SCANCODE_RETURN;
	code.mod = KMOD_NONE;
	code.unused = 0;
	get()->handle_key(true, code);
	return 0;
}

/* RST
   .. method:: add(label, value[, icon = nil, tooltip = "", select = false, hotkey = nil])

      .. versionadded:: 1.2

      .. versionchanged:: 1.3
         Added parameter ``hotkey``.

      Add an entry to the dropdown. Only allowed for dropdowns with supported datatypes.

      :arg label: The label to display for the entry.
      :type label: :class:`string`
      :arg value: The internal value of the entry.
      :type value: This dropdown's :attr:`datatype`
      :arg icon: The icon filepath for the entry (``nil`` for no icon).
      :type icon: :class:`string`
      :arg tooltip: The entry's tooltip text.
      :type tooltip: :class:`string`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaDropdown::add(lua_State* L) {
	int top = lua_gettop(L);
	std::string label = luaL_checkstring(L, 2);
	std::string icon = (top >= 4 && !lua_isnil(L, 4)) ? luaL_checkstring(L, 4) : "";
	std::string tooltip = top >= 5 ? luaL_checkstring(L, 5) : "";
	bool select = top >= 6 && luaL_checkboolean(L, 6);
	std::string hotkey = top >= 7 ? luaL_checkstring(L, 7) : "";

	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		std::string value = luaL_checkstring(L, 3);
		dd->add(label, value, icon.empty() ? nullptr : g_image_cache->get(icon), select, tooltip,
		        shortcut_string_if_set(hotkey, false));
	} else {
		report_error(L, "add() not allowed for dropdown with unsupported datatype");
	}
	return 0;
}

/* RST
   .. method:: get_value_at(index)

      .. versionadded:: 1.3

      Get the internal value of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's internal value.
      :rtype: This list's :attr:`datatype`
*/
int LuaDropdown::get_value_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).value);
	} else {
		report_error(L, "get_value_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_label_at(index)

      .. versionadded:: 1.3

      Get the user-facing name of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's display name.
      :rtype: :class:`string`
*/
int LuaDropdown::get_label_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).name);
	} else {
		report_error(L, "get_label_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_tooltip_at(index)

      .. versionadded:: 1.3

      Get the tooltip of the item at the specified position.
      Only allowed for dropdowns with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's tooltip (may be empty if the item has no tooltip).
      :rtype: :class:`string`
*/
int LuaDropdown::get_tooltip_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for dropdown of size %u", index, nritems);
	}
	if (upcast(DropdownOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, dd->at(index - 1).tooltip);
	} else {
		report_error(L, "get_tooltip_at() not allowed for dropdown with unsupported datatype");
	}
	return 1;
}

/*
 * C Functions
 */

/* RST
Listselect
----------

.. class:: Listselect

   .. versionadded:: 1.2

   This represents a selection list.

   Some attributes and functions are available only for listselects with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaListselect::className[] = "Listselect";
const MethodType<LuaListselect> LuaListselect::Methods[] = {
   METHOD(LuaListselect, add),
   METHOD(LuaListselect, get_value_at),
   METHOD(LuaListselect, get_label_at),
   METHOD(LuaListselect, get_tooltip_at),
   METHOD(LuaListselect, get_enable_at),
   METHOD(LuaListselect, get_indent_at),
   {nullptr, nullptr},
};
const PropertyType<LuaListselect> LuaListselect::Properties[] = {
   PROP_RO(LuaListselect, datatype),
   PROP_RO(LuaListselect, no_of_items),
   PROP_RO(LuaListselect, selection),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      (RO) The listselect's datatype as :class:`string` if supported,
      or ``nil`` for listselects with unsupported datatype.

      Currently only ``"string"`` is supported.
*/
int LuaListselect::get_datatype(lua_State* L) {
	if (dynamic_cast<const ListselectOfString*>(get()) != nullptr) {
		lua_pushstring(L, "string");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The number of items this listselect has.
*/
int LuaListselect::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection

      (RO) The currently selected entry or ``nil`` if none is selected.
      For listselects with unsupported datatype, this is always ``nil``.
*/
int LuaListselect::get_selection(lua_State* L) {
	if (upcast(const ListselectOfString, list, get()); list != nullptr && list->has_selection()) {
		lua_pushstring(L, list->get_selected().c_str());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: add(label, value
      [, icon = nil, tooltip = "", select = false, indent = 0, enable = true, hotkey = nil])

   .. versionchanged:: 1.3
      Added ``enable`` and ``hotkey`` parameters.

      Add an entry to the list. Only allowed for lists with supported datatypes.

      :arg label: The label to display for the entry.
      :type label: :class:`string`
      :arg value: The internal value of the entry.
      :type value: This list's :attr:`datatype`
      :arg icon: The icon filepath for the entry (``nil`` for no icon).
      :type icon: :class:`string`
      :arg tooltip: The entry's tooltip text.
      :type tooltip: :class:`string`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg indent: By how many levels to indent this entry.
      :type indent: :class:`int`
      :arg enable: Whether to enable this entry.
      :type enable: :class:`boolean`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaListselect::add(lua_State* L) {
	int top = lua_gettop(L);
	std::string label = luaL_checkstring(L, 2);
	std::string icon = (top >= 4 && !lua_isnil(L, 4)) ? luaL_checkstring(L, 4) : "";
	std::string tooltip = top >= 5 ? luaL_checkstring(L, 5) : "";
	bool select = top >= 6 && luaL_checkboolean(L, 6);
	uint32_t indent = top >= 7 ? luaL_checkuint32(L, 7) : 0;
	bool enable = top < 8 || luaL_checkboolean(L, 8);
	std::string hotkey = top >= 9 ? luaL_checkstring(L, 9) : "";

	if (upcast(ListselectOfString, list, get()); list != nullptr) {
		std::string value = luaL_checkstring(L, 3);
		list->add(label, value, icon.empty() ? nullptr : g_image_cache->get(icon), select, tooltip,
		          shortcut_string_if_set(hotkey, false), indent, enable);
	} else {
		report_error(L, "add() not allowed for listselect with unsupported datatype");
	}
	return 0;
}

/* RST
   .. method:: get_value_at(index)

      .. versionadded:: 1.3

      Get the internal value of the item at the specified position.
      Only allowed for lists with supported datatypes.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's internal value.
      :rtype: This list's :attr:`datatype`
*/
int LuaListselect::get_value_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	if (upcast(ListselectOfString, dd, get()); dd != nullptr) {
		lua_pushstring(L, (*dd)[index - 1]);
	} else {
		report_error(L, "get_value_at() not allowed for list with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: get_label_at(index)

      .. versionadded:: 1.3

      Get the user-facing name of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's display name.
      :rtype: :class:`string`
*/
int LuaListselect::get_label_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushstring(L, get()->at(index - 1).name);
	return 1;
}

/* RST
   .. method:: get_tooltip_at(index)

      .. versionadded:: 1.3

      Get the tooltip of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's tooltip (may be empty if the item has no tooltip).
      :rtype: :class:`string`
*/
int LuaListselect::get_tooltip_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushstring(L, get()->at(index - 1).tooltip);
	return 1;
}

/* RST
   .. method:: get_enable_at(index)

      .. versionadded:: 1.3

      Get the enable/disable display state of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: Whether the entry is marked as enabled.
      :rtype: :class:`boolean`
*/
int LuaListselect::get_enable_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushboolean(L, static_cast<int>(get()->at(index - 1).enable));
	return 1;
}

/* RST
   .. method:: get_indent_at(index)

      .. versionadded:: 1.3

      Get the indentation level of the item at the specified position.

      :arg index: The index to query, starting from ``1``.
      :type index: :class:`integer`
      :returns: The entry's indentation.
      :rtype: :class:`integer`
*/
int LuaListselect::get_indent_at(lua_State* L) {
	const uint32_t index = luaL_checkuint32(L, 2);
	const uint32_t nritems = get()->size();
	if (index < 1 || index > nritems) {
		report_error(L, "Index %u out of range for list of size %u", index, nritems);
	}
	lua_pushinteger(L, get()->at(index - 1).indent);
	return 1;
}

/*
 * C Functions
 */

/* RST
Table
-----

.. class:: Table

   .. versionadded:: 1.2

   This represents a table.

   Some attributes and functions are available only for tables with a data type
   supported by the Lua interface. See :attr:`datatype`.
*/
const char LuaTable::className[] = "Table";
const MethodType<LuaTable> LuaTable::Methods[] = {
   METHOD(LuaTable, get),          METHOD(LuaTable, add), METHOD(LuaTable, remove_row),
   METHOD(LuaTable, remove_entry), {nullptr, nullptr},
};
const PropertyType<LuaTable> LuaTable::Properties[] = {
   PROP_RO(LuaTable, datatype),        PROP_RO(LuaTable, no_of_rows),
   PROP_RW(LuaTable, selection_index), PROP_RO(LuaTable, selections),
   PROP_RW(LuaTable, sort_column),     PROP_RW(LuaTable, sort_descending),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: datatype

      (RO) The table's datatype as :class:`string` if supported,
      or ``nil`` for tables with unsupported datatype.

      Currently only ``"int"`` is supported.
*/
int LuaTable::get_datatype(lua_State* L) {
	if (dynamic_cast<const TableOfInt*>(get()) != nullptr) {
		lua_pushstring(L, "int");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: no_of_rows

      (RO) The number of rows this table has.
*/
int LuaTable::get_no_of_rows(lua_State* L) {
	lua_pushinteger(L, get()->size());
	return 1;
}

/* RST
   .. attribute:: selection_index

      (RW) The index of the currently selected row or ``nil`` if none is selected.
      Setting this to ``nil`` clears the selection.
*/
int LuaTable::get_selection_index(lua_State* L) {
	if (!get()->has_selection()) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, get()->selection_index());
	}
	return 1;
}
int LuaTable::set_selection_index(lua_State* L) {
	if (lua_isnil(L, -1)) {
		get()->clear_selections();
	} else {
		get()->select(luaL_checkuint32(L, -1));
	}
	return 0;
}

/* RST
   .. attribute:: selections

      (RO) An :class:`array` with all currently selected row indices (may be empty).
*/
int LuaTable::get_selections(lua_State* L) {
	lua_newtable(L);
	int index = 0;
	for (uint32_t value : get()->selections()) {
		lua_pushinteger(L, ++index);
		lua_pushinteger(L, value);
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: sort_column

      (RW) The index of the column by which the table is sorted.
*/
int LuaTable::get_sort_column(lua_State* L) {
	lua_pushinteger(L, get()->get_sort_column());
	return 1;
}
int LuaTable::set_sort_column(lua_State* L) {
	get()->set_sort_column(luaL_checkuint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: sort_descending

      (RW) Whether the table is sorted in reverse order.
*/
int LuaTable::get_sort_descending(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_sort_descending()));
	return 1;
}
int LuaTable::set_sort_descending(lua_State* L) {
	get()->set_sort_descending(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: get(row)

      Lookup the internal value associated with a given row index.
      Only allowed for tables with supported datatypes.

      :arg row: The row index to look up.
      :type row: :class:`int`
      :returns: The row's internal value.
      :rtype: :class:`int`
*/
int LuaTable::get(lua_State* L) {
	if (upcast(TableOfInt, table, get()); table != nullptr) {
		lua_pushinteger(L, (*table)[luaL_checkuint32(L, 2)]);
	} else {
		report_error(L, "get() not allowed for table with unsupported datatype");
	}
	return 1;
}

/* RST
   .. method:: add(value, select, disable, columns)

      Add a row to the table. Only allowed for tables with supported datatypes.

      The named arguments are followed by an array of as many entries as the table has columns.
      Each array entry is a :class:`table` describing the content of one table cell.
      Valid keys are:

         * ``"text"``: **Optional**. The text to show in the cell.
         * ``"icon"``: **Optional**. The icon filepath in the cell.

      :arg value: The internal value of the entry.
      :type value: This table's :attr:`datatype`
      :arg select: Whether to select this entry.
      :type select: :class:`boolean`
      :arg disable: Whether to disable this row.
      :type disable: :class:`boolean`
      :arg columns: The cell descriptors.
      :type columns: :class:`array` of :class:`table`
*/
int LuaTable::add(lua_State* L) {
	bool select = luaL_checkboolean(L, 3);
	bool disable = luaL_checkboolean(L, 4);

	if (upcast(TableOfInt, table, get()); table != nullptr) {
		uintptr_t value = luaL_checkuint32(L, 2);
		TableOfInt::EntryRecord& record = table->add(value, select);
		record.set_disabled(disable);

		int column = 0;
		lua_pushnil(L);
		while (lua_next(L, 5) != 0) {
			std::string text = get_table_string(L, "text", false);
			std::string icon = get_table_string(L, "icon", false);

			if (icon.empty()) {
				record.set_string(column, text);
			} else {
				record.set_picture(column, g_image_cache->get(icon), text);
			}

			lua_pop(L, 1);
			++column;
		}
	} else {
		report_error(L, "add() not allowed for table with unsupported datatype");
	}

	return 0;
}

/* RST
   .. method:: remove_row(row)

      Delete the row at the specified index from the table.

      :arg row: The row index to delete.
      :type row: :class:`int`
*/
int LuaTable::remove_row(lua_State* L) {
	get()->remove(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. method:: remove_entry(entry)

      Delete the row with the specified internal value from the table.
      Only allowed for tables with supported datatypes.

      :arg entry: The entry value to delete.
      :type entry: :class:`int`
*/
int LuaTable::remove_entry(lua_State* L) {
	if (upcast(TableOfInt, table, get()); table != nullptr) {
		table->remove_entry(luaL_checkuint32(L, 2));
	} else {
		report_error(L, "get() not allowed for table with unsupported datatype");
	}
	return 0;
}

/*
 * C Functions
 */

/* RST
TabPanel
--------

.. class:: TabPanel

   .. versionadded:: 1.2

   This represents a panel that allows switching between multiple tabs.
*/
const char LuaTabPanel::className[] = "TabPanel";
const MethodType<LuaTabPanel> LuaTabPanel::Methods[] = {
   METHOD(LuaTabPanel, remove_last_tab),
   {nullptr, nullptr},
};
const PropertyType<LuaTabPanel> LuaTabPanel::Properties[] = {
   PROP_RO(LuaTabPanel, no_of_tabs),
   PROP_RW(LuaTabPanel, active),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: no_of_tabs

      (RO) The number of tabs this tab panel has.
*/
int LuaTabPanel::get_no_of_tabs(lua_State* L) {
	lua_pushinteger(L, get()->tabs().size());
	return 1;
}

/* RST
   .. attribute:: active

      (RW) The index of the currently active tab.
      When assigning this property, it is also allowed to activate a tab by name instead of index.
*/
int LuaTabPanel::get_active(lua_State* L) {
	lua_pushinteger(L, get()->active());
	return 1;
}
int LuaTabPanel::set_active(lua_State* L) {
	if (static_cast<bool>(lua_isnumber(L, -1))) {
		get()->activate(luaL_checkuint32(L, -1));
	} else {
		get()->activate(luaL_checkstring(L, -1));
	}
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: remove_last_tab(name)

      Remove the **last** tab in the panel.

      As a precaution against accidental removal of tabs, the name of the tab
      that will be removed has to be specified.

      :arg name: The name of the last tab.
      :type name: :class:`string`
      :returns: Whether the tab was removed.
      :rtype: :class:`boolean`
*/
int LuaTabPanel::remove_last_tab(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->remove_last_tab(luaL_checkstring(L, 2))));
	return 0;
}

/*
 * C Functions
 */

/* RST
Tab
------

.. class:: Tab

   A tab button.
*/
const char LuaTab::className[] = "Tab";
const MethodType<LuaTab> LuaTab::Methods[] = {
   METHOD(LuaTab, click),
   {nullptr, nullptr},
};
const PropertyType<LuaTab> LuaTab::Properties[] = {
   PROP_RO(LuaTab, active),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: active

      (RO) Is this the currently active tab in this window?
*/
int LuaTab::get_active(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->active()));
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: click

      Click this tab making it the active one.
*/
int LuaTab::click(lua_State* /* L */) {
	log_info("Clicking tab '%s'\n", get()->get_name().c_str());
	get()->activate();
	return 0;
}

/*
 * C Functions
 */

/* RST
Window
------

.. class:: Window

   This represents a Window.
*/
const char LuaWindow::className[] = "Window";
const MethodType<LuaWindow> LuaWindow::Methods[] = {
   METHOD(LuaWindow, close),
   {nullptr, nullptr},
};
const PropertyType<LuaWindow> LuaWindow::Properties[] = {
   PROP_RW(LuaWindow, title),
   PROP_RW(LuaWindow, pinned),
   PROP_RW(LuaWindow, minimal),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: title

      .. versionadded:: 1.2

      (RW) The title shown in the window's title bar.
*/
int LuaWindow::get_title(lua_State* L) {
	lua_pushstring(L, get()->get_title().c_str());
	return 1;
}
int LuaWindow::set_title(lua_State* L) {
	get()->set_title(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: pinned

      .. versionadded:: 1.2

      (RW) Whether the window is pinned so it can't be closed accidentally.
*/
int LuaWindow::get_pinned(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_pinned()));
	return 1;
}
int LuaWindow::set_pinned(lua_State* L) {
	get()->set_pinned(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: minimal

      .. versionadded:: 1.2

      (RW) Whether the window is minimized and only the title bar visible.
*/
int LuaWindow::get_minimal(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_minimal()));
	return 1;
}
int LuaWindow::set_minimal(lua_State* L) {
	if (luaL_checkboolean(L, -1)) {
		if (!get()->is_minimal()) {
			get()->minimize();
		}
	} else {
		if (get()->is_minimal()) {
			get()->restore();
		}
	}
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: close

      Closes this window. This invalidates this Object, do
      not use it any longer.
*/
int LuaWindow::close(lua_State* /* L */) {
	log_info("Closing window '%s'\n", get()->get_name().c_str());
	panel_->die();
	panel_ = nullptr;
	return 0;
}

/*
 * C Functions
 */

/* RST
MapView
-------

.. class:: MapView

   The map view is the main widget and the root of all panels. It is the big
   view of the map that is visible at all times while playing.

   This class may only be accessed in a game or the editor.
   You can construct as many instances of it as you like,
   and they will all refer to the same map view.
*/
const char LuaMapView::className[] = "MapView";
const MethodType<LuaMapView> LuaMapView::Methods[] = {
   METHOD(LuaMapView, click),
   METHOD(LuaMapView, start_road_building),
   METHOD(LuaMapView, abort_road_building),
   METHOD(LuaMapView, close),
   METHOD(LuaMapView, scroll_to_field),
   METHOD(LuaMapView, scroll_to_map_pixel),
   METHOD(LuaMapView, is_visible),
   METHOD(LuaMapView, mouse_to_field),
   METHOD(LuaMapView, mouse_to_pixel),
   METHOD(LuaMapView, add_toolbar_plugin),
   METHOD(LuaMapView, update_toolbar),
   METHOD(LuaMapView, set_keyboard_shortcut),
   METHOD(LuaMapView, set_keyboard_shortcut_release),
   METHOD(LuaMapView, add_plugin_timer),
   {nullptr, nullptr},
};
const PropertyType<LuaMapView> LuaMapView::Properties[] = {
   PROP_RO(LuaMapView, average_fps),
   PROP_RO(LuaMapView, center_map_pixel),
   PROP_RW(LuaMapView, buildhelp),
   PROP_RW(LuaMapView, census),
   PROP_RW(LuaMapView, statistics),
   PROP_RO(LuaMapView, is_building_road),
   PROP_RO(LuaMapView, auto_roadbuilding_mode),
   PROP_RO(LuaMapView, is_animating),
   PROP_RO(LuaMapView, toolbar),
   {nullptr, nullptr, nullptr},
};

LuaMapView::LuaMapView(lua_State* L) : LuaPanel(get_egbase(L).get_ibase()) {
}

void LuaMapView::__unpersist(lua_State* L) {
	panel_ = get_egbase(L).get_ibase();
}

/*
 * Properties
 */
/* RST
   .. attribute:: toolbar

      .. versionadded:: 1.2

      (RO) The main toolbar.
*/
int LuaMapView::get_toolbar(lua_State* L) {
	to_lua<LuaPanel>(L, new LuaPanel(get()->toolbar()));
	return 1;
}

/* RST
   .. attribute:: average_fps

      (RO) The average frames per second that the user interface is being drawn at.
*/
int LuaMapView::get_average_fps(lua_State* L) {
	lua_pushdouble(L, get()->average_fps());
	return 1;
}
/* RST
   .. attribute:: center_map_pixel

      (RO) The map position (in pixels) that the center pixel of this map view
      currently sees. This is a table containing 'x', 'y'.
*/
int LuaMapView::get_center_map_pixel(lua_State* L) {
	const Vector2f center = get()->map_view()->view_area().rect().center();
	lua_newtable(L);

	lua_pushstring(L, "x");
	lua_pushdouble(L, center.x);
	lua_rawset(L, -3);

	lua_pushstring(L, "y");
	lua_pushdouble(L, center.y);
	lua_rawset(L, -3);
	return 1;
}

/* RST
   .. attribute:: buildhelp

      (RW) True if the buildhelp is show, false otherwise.
*/
int LuaMapView::get_buildhelp(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->buildhelp()));
	return 1;
}
int LuaMapView::set_buildhelp(lua_State* L) {
	get()->show_buildhelp(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: census

      (RW) True if the census strings are shown on buildings, false otherwise
*/
int LuaMapView::get_census(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_display_flag(InteractiveBase::dfShowCensus)));
	return 1;
}
int LuaMapView::set_census(lua_State* L) {
	get()->set_display_flag(InteractiveBase::dfShowCensus, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: statistics

      (RW) True if the statistics strings are shown on buildings, false
      otherwise
*/
int LuaMapView::get_statistics(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_display_flag(InteractiveBase::dfShowStatistics)));
	return 1;
}
int LuaMapView::set_statistics(lua_State* L) {
	get()->set_display_flag(InteractiveBase::dfShowStatistics, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: is_building_road

      (RO) Is the player currently in road/waterway building mode?
*/
int LuaMapView::get_is_building_road(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->in_road_building_mode()));
	return 1;
}

/* RST
   .. attribute:: auto_roadbuild_mode

      (RO) Is the player using automatic road building mode?
*/
int LuaMapView::get_auto_roadbuilding_mode(lua_State* L) {
	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, static_cast<int>(ipl->auto_roadbuild_mode()));
	}
	return 1;
}

/* RST
   .. attribute:: is_animating

      (RO) True if this MapView is currently panning or zooming.
*/
int LuaMapView::get_is_animating(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->map_view()->is_animating()));
	return 1;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: click(field)

      Jumps the mouse onto a field and clicks it just like the user would
      have.

      :arg field: the field to click on
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::click(lua_State* L) {
	auto* const field = *get_user_class<LuaMaps::LuaField>(L, 2);
	get()->map_view()->mouse_to_field(field->coords(), MapView::Transition::Jump);

	// We fake the triangle here, since we only support clicking on Nodes from
	// Lua.
	Widelands::NodeAndTriangle<> node_and_triangle{
	   field->coords(), Widelands::TCoords<>(field->coords(), Widelands::TriangleIndex::D)};
	get()->map_view()->field_clicked(node_and_triangle);
	return 0;
}

/* RST
   .. method:: start_road_building(flag[, waterway = false])

      Enters the road building mode as if the player has clicked
      the flag and chosen build road. It will also warp the mouse
      to the given starting node. Throws an error if we are already in road
      building mode.

      :arg flag: :class:`wl.map.Flag` object to start building from.
      :arg waterway: if `true`, start building a waterway rather than a road
*/
// UNTESTED
int LuaMapView::start_road_building(lua_State* L) {
	InteractiveBase* me = get();
	if (me->in_road_building_mode()) {
		report_error(L, "Already building road!");
	}

	Widelands::Coords starting_field =
	   (*get_user_class<LuaMaps::LuaFlag>(L, 2))->get(L, get_egbase(L))->get_position();

	me->map_view()->mouse_to_field(starting_field, MapView::Transition::Jump);
	me->start_build_road(starting_field, me->get_player()->player_number(),
	                     lua_gettop(L) > 2 && luaL_checkboolean(L, 3) ? RoadBuildingType::kWaterway :
	                                                                    RoadBuildingType::kRoad);

	return 0;
}

/* RST
   .. method:: abort_road_building

      If the player is currently in road building mode, this will cancel it.
      If he wasn't, this will do nothing.
*/
// UNTESTED
int LuaMapView::abort_road_building(lua_State* /* L */) {
	InteractiveBase* me = get();
	if (me->in_road_building_mode()) {
		me->abort_build_road();
	}
	return 0;
}

/* RST
   .. method:: close

      Closes the MapView. Note that this is the equivalent as clicking on
      the exit button in the game; that is the game will be exited.

      This is especially useful for automated testing of features and is for
      example used in the widelands Lua test suite.
*/
int LuaMapView::close(lua_State* /* l */) {
	get()->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	return 0;
}

/* RST
   .. method:: scroll_to_map_pixel(x, y)

      Starts an animation to center the view on top of the pixel (x, y) in map
      pixel space. Use `is_animating` to check if the animation is still going
      on.

      :arg x: x coordinate of the pixel
      :type x: number
      :arg y: y coordinate of the pixel
      :type y: number
*/
int LuaMapView::scroll_to_map_pixel(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	// don't move view in replays
	if (egbase.is_game() &&
	    dynamic_cast<Widelands::Game&>(egbase).game_controller()->get_game_type() ==
	       GameController::GameType::kReplay) {
		return 0;
	}

	const Vector2f center(luaL_checkdouble(L, 2), luaL_checkdouble(L, 3));
	get()->map_view()->scroll_to_map_pixel(center, MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: scroll_to_field(field)

      Starts an animation to center the view on top of the 'field'. Use
      `is_animating` to check if the animation is still going on.

      :arg field: the field to center on
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::scroll_to_field(lua_State* L) {
	get()->map_view()->scroll_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: is_visible(field)

      Returns `true` if `field` is currently visible in the map view.

      :arg field: the field
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::is_visible(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->map_view()->view_area().contains(
	                      (*get_user_class<LuaMaps::LuaField>(L, 2))->coords())));
	return 1;
}

/* RST
   .. method:: mouse_to_pixel(x, y)

      Starts an animation to move the mouse onto the pixel (x, y) of this panel.
      Use `is_animating` to check if the animation is still going on.

      :arg x: x coordinate of the pixel
      :type x: number
      :arg y: y coordinate of the pixel
      :type y: number
*/
int LuaMapView::mouse_to_pixel(lua_State* L) {
	int x = luaL_checkint32(L, 2);
	int y = luaL_checkint32(L, 3);
	get()->map_view()->mouse_to_pixel(Vector2i(x, y), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: mouse_to_field(field)

      Starts an animation to move the mouse onto the 'field'. If 'field' is not
      visible on the screen currently, does nothing. Use `is_animating` to
      check if the animation is still going on.

      :arg field: the field
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::mouse_to_field(lua_State* L) {
	get()->map_view()->mouse_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: update_toolbar()

      .. versionadded:: 1.2

      Recompute the size and position of the toolbar.
      Call this after you have modified the toolbar in any way.
*/
int LuaMapView::update_toolbar(lua_State* L) {
	get_egbase(L).get_ibase()->finalize_toolbar();
	return 0;
}

/* RST
   .. method:: add_toolbar_plugin(action, icon, name[, tooltip = "", hotkey = nil])

      .. versionadded:: 1.2

   .. versionchanged:: 1.3
      Added ``hotkey`` parameter.

      Add an entry to the main toolbar's Plugin dropdown.
      This makes the plugin dropdown visible if it was hidden.

      :arg action: The Lua code to run when the user selects the entry.
      :type action: :class:`string`
      :arg icon: Icon filepath for the entry.
      :type icon: :class:`string`
      :arg name: Label for the entry.
      :type name: :class:`string`
      :arg tooltip: Tooltip for the entry.
      :type tooltip: :class:`string`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaMapView::add_toolbar_plugin(lua_State* L) {
	get_egbase(L).get_ibase()->add_toolbar_plugin(
	   luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checkstring(L, 4),
	   lua_gettop(L) >= 5 ? luaL_checkstring(L, 5) : "",
	   lua_gettop(L) >= 6 ? shortcut_string_if_set(luaL_checkstring(L, 6), false) : "");
	return 0;
}

/* RST
   .. method:: set_keyboard_shortcut(internal_name, action[, failsafe=true])

      .. versionadded:: 1.3

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is pressed.
      This replaces any existing action associated with pressing the shortcut.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut_release`
*/
int LuaMapView::set_keyboard_shortcut(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, true);
	return 0;
}

/* RST
   .. method:: set_keyboard_shortcut_release(internal_name, action[, failsafe=true])

      .. versionadded:: 1.3

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is released
      after having been previously pressed.
      This replaces any existing action associated with releasing the shortcut.

      You don't need this in normal cases. When in doubt, use only meth:`set_keyboard_shortcut`.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut`
*/
int LuaMapView::set_keyboard_shortcut_release(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, false);
	return 0;
}

/* RST
   .. method:: add_plugin_timer(action, interval[, failsafe=true])

      .. versionadded:: 1.2

      Register a piece of code that will be run periodically as long as the game/editor is running.

      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg interval: The interval in milliseconds realtime in which the code will be invoked.
      :type interval: :class:`int`
      :arg failsafe: In event of an error, an error message is shown and the timer is removed.
         If this is set to :const:`false`, the game will be aborted with no error handling instead.
      :type failsafe: :class:`boolean`
*/
int LuaMapView::add_plugin_timer(lua_State* L) {
	std::string action = luaL_checkstring(L, 2);
	uint32_t interval = luaL_checkuint32(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);

	if (interval == 0) {
		report_error(L, "Timer interval must be non-zero");
	}

	get_egbase(L).get_ibase()->add_plugin_timer(action, interval, failsafe);
	return 0;
}

/*
 * C Functions
 */

/* RST
MainMenu
--------

.. class:: MainMenu

   .. versionadded:: 1.3

   The main menu screen is the main widget and the root of all panels.

   This class may not be accessed in a game or the editor.
   You can construct as many instances of it as you like,
   and they will all refer to the same main menu.
*/
const char LuaMainMenu::className[] = "MainMenu";
const MethodType<LuaMainMenu> LuaMainMenu::Methods[] = {
   METHOD(LuaMainMenu, set_keyboard_shortcut),
   METHOD(LuaMainMenu, set_keyboard_shortcut_release),
   METHOD(LuaMainMenu, add_plugin_timer),
   {nullptr, nullptr},
};
const PropertyType<LuaMainMenu> LuaMainMenu::Properties[] = {
   {nullptr, nullptr, nullptr},
};

LuaMainMenu::LuaMainMenu(lua_State* L) : LuaPanel(&get_main_menu(L)) {
}

void LuaMainMenu::__unpersist(lua_State* L) {
	panel_ = &get_main_menu(L);
}

/*
 * Lua Functions
 */

/* RST
   .. method:: set_keyboard_shortcut(internal_name, action[, failsafe=true])

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is pressed.
      This replaces any existing action associated with pressing the shortcut.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut_release`
*/
int LuaMainMenu::set_keyboard_shortcut(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, true);
	return 0;
}

/* RST
   .. method:: set_keyboard_shortcut_release(internal_name, action[, failsafe=true])

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is released
      after having been previously pressed.
      This replaces any existing action associated with releasing the shortcut.

      You don't need this in normal cases. When in doubt, use only meth:`set_keyboard_shortcut`.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut`
*/
int LuaMainMenu::set_keyboard_shortcut_release(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, false);
	return 0;
}

/* RST
   .. method:: add_plugin_timer(action, interval[, failsafe=true])

      Register a piece of code that will be run periodically as long as the main menu is running
      and its Lua context is not reset.

      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg interval: The interval in milliseconds realtime in which the code will be invoked.
      :type interval: :class:`int`
      :arg failsafe: In event of an error, an error message is shown and the timer is removed.
         If this is set to :const:`false`, the game will be aborted with no error handling instead.
      :type failsafe: :class:`boolean`
*/
int LuaMainMenu::add_plugin_timer(lua_State* L) {
	std::string action = luaL_checkstring(L, 2);
	uint32_t interval = luaL_checkuint32(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);

	if (interval == 0) {
		report_error(L, "Timer interval must be non-zero");
	}

	get()->add_plugin_timer(action, interval, failsafe);
	return 0;
}

/*
 * C Functions
 */

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

/* RST
.. function:: set_user_input_allowed(b)

   Allow or disallow user input. Be warned, setting this will make that
   mouse movements and keyboard presses are completely ignored. Only
   scripted stuff will still happen.

   :arg b: :const:`true` or :const:`false`
   :type b: :class:`boolean`
*/
static int L_set_user_input_allowed(lua_State* L) {
	UI::Panel::set_allow_user_input(luaL_checkboolean(L, -1));
	return 0;
}
/* RST
.. method:: get_user_input_allowed

   Return the current state of this flag.

   :returns: :const:`true` or :const:`false`
   :rtype: :class:`boolean`
*/
static int L_get_user_input_allowed(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(UI::Panel::allow_user_input()));
	return 1;
}

/* RST
.. method:: get_shortcut(name)

   Returns the keyboard shortcut with the given name.

   :returns: The human-readable and localized shortcut.
   :rtype: :class:`string`
*/
static int L_get_shortcut(lua_State* L) {
	const std::string name = luaL_checkstring(L, -1);
	try {
		lua_pushstring(L, shortcut_string_for(shortcut_from_string(name), true).c_str());
	} catch (const WException& e) {
		report_error(L, "Unable to query shortcut for '%s': %s", name.c_str(), e.what());
	}
	return 1;
}

/* RST
.. method:: shortcut_exists(internal_name)

   .. versionadded:: 1.3

   Check whether the given name belongs to a known keyboard shortcut.

   :arg internal_name: The internal name of the keyboard shortcut.
   :type internal_name: :class:`string`
   :returns: Whether the named shortcut exists.
   :rtype: :class:`boolean`
*/
static int L_shortcut_exists(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(shortcut_exists(luaL_checkstring(L, -1))));
	return 1;
}

/* RST
.. method:: get_all_keyboard_shortcut_names()

   .. versionadded:: 1.3

   List the internal names of all known keyboard shortcuts.

   :returns: The names.
   :rtype: :class:`array` of :class:`string`
*/
static int L_get_all_keyboard_shortcut_names(lua_State* L) {
	lua_newtable(L);
	int i = 1;
	for (const std::string& name : get_all_keyboard_shortcut_names()) {
		lua_pushint32(L, i++);
		lua_pushstring(L, name.c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
.. method:: get_ingame_shortcut_help()

   .. versionadded:: 1.2

   Returns the list of current in-game keyboard shortcuts formatted as richtext.

   :returns: The richtext formatted list of shortcuts
   :rtype: :class:`string`
*/
static int L_get_ingame_shortcut_help(lua_State* L) {
	lua_pushstring(L, get_ingame_shortcut_help().c_str());
	return 1;
}

/* RST
.. method:: get_fastplace_help()

   .. versionadded:: 1.2

   Returns the list of current fastplace shortcuts for the current player formatted as richtext.

   :returns: The richtext formatted list of shortcuts
   :rtype: :class:`string`
*/
static int L_get_fastplace_help(lua_State* L) {
	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}
	lua_pushstring(L, ipl->get_fastplace_help().c_str());
	return 1;
}

/* RST
.. method:: get_editor_shortcut_help()

   .. versionadded:: 1.2

   Returns the list of current map editor keyboard shortcuts formatted as richtext.

   :returns: The richtext formatted list of shortcuts
   :rtype: :class:`string`
*/
static int L_get_editor_shortcut_help(lua_State* L) {
	lua_pushstring(L, get_editor_shortcut_help().c_str());
	return 1;
}

/* RST
.. method:: show_messagebox(title, text[, cancel_button = true])

   .. versionadded:: 1.2

   Show the user a modal message box with an OK button and optionally a cancel button.

   You can use :ref:`richtext <wlrichtext>` to style the text.
   Don't forget to wrap it in an :ref:`rt tag <rt_tags_rt>` in this case.

   :arg title: The caption of the window
   :type title: :class:`string`
   :arg text: The message to show
   :type text: :class:`string`
   :arg cancel_button: Whether to include a Cancel button
   :type cancel_button: :class:`boolean`
   :returns: Whether the user clicked OK.
   :rtype: :class:`boolean`
*/
static int L_show_messagebox(lua_State* L) {
	const int nargs = lua_gettop(L);
	if (nargs < 2 || nargs > 3) {
		report_error(L, "Wrong number of arguments");
	}

	std::string title = luaL_checkstring(L, 1);
	std::string text = luaL_checkstring(L, 2);
	bool allow_cancel = nargs < 3 || luaL_checkboolean(L, 3);

	const bool mainmenu = is_main_menu(L);
	UI::WLMessageBox m(
	   mainmenu ? static_cast<UI::Panel*>(&get_main_menu(L)) :
	              static_cast<UI::Panel*>(get_egbase(L).get_ibase()),
	   mainmenu ? UI::WindowStyle::kFsMenu : UI::WindowStyle::kWui, title, text,
	   allow_cancel ? UI::WLMessageBox::MBoxType::kOkCancel : UI::WLMessageBox::MBoxType::kOk);
	UI::Panel::Returncodes result;
	NoteThreadSafeFunction::instantiate(
	   [&result, &m]() { result = m.run<UI::Panel::Returncodes>(); }, true);

	lua_pushboolean(L, static_cast<int>(result == UI::Panel::Returncodes::kOk));
	return 1;
}

const static struct luaL_Reg wlui[] = {
   {"set_user_input_allowed", &L_set_user_input_allowed},
   {"get_user_input_allowed", &L_get_user_input_allowed},
   {"get_shortcut", &L_get_shortcut},
   {"get_ingame_shortcut_help", &L_get_ingame_shortcut_help},
   {"get_fastplace_help", &L_get_fastplace_help},
   {"get_editor_shortcut_help", &L_get_editor_shortcut_help},
   {"show_messagebox", &L_show_messagebox},
   {"shortcut_exists", &L_shortcut_exists},
   {"get_all_keyboard_shortcut_names", &L_get_all_keyboard_shortcut_names},
   {nullptr, nullptr}};

void luaopen_wlui(lua_State* L, const bool game_or_editor) {
	lua_getglobal(L, "wl");   // S: wl_table
	lua_pushstring(L, "ui");  // S: wl_table "ui"
	luaL_newlib(L, wlui);     // S: wl_table "ui" wl.ui_table
	lua_settable(L, -3);      // S: wl_table
	lua_pop(L, 1);            // S:

	register_class<LuaPanel>(L, "ui");

	register_class<LuaButton>(L, "ui", true);
	add_parent<LuaButton, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMultilineTextarea>(L, "ui", true);
	add_parent<LuaMultilineTextarea, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTextarea>(L, "ui", true);
	add_parent<LuaTextarea, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaCheckbox>(L, "ui", true);
	add_parent<LuaCheckbox, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaRadioButton>(L, "ui", true);
	add_parent<LuaRadioButton, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaProgressBar>(L, "ui", true);
	add_parent<LuaProgressBar, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaSpinBox>(L, "ui", true);
	add_parent<LuaSpinBox, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaSlider>(L, "ui", true);
	add_parent<LuaSlider, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTextInputPanel>(L, "ui", true);
	add_parent<LuaTextInputPanel, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaDropdown>(L, "ui", true);
	add_parent<LuaDropdown, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaListselect>(L, "ui", true);
	add_parent<LuaListselect, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTable>(L, "ui", true);
	add_parent<LuaTable, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTabPanel>(L, "ui", true);
	add_parent<LuaTabPanel, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTab>(L, "ui", true);
	add_parent<LuaTab, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWindow>(L, "ui", true);
	add_parent<LuaWindow, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	if (game_or_editor) {
		// Only in game and editor
		register_class<LuaMapView>(L, "ui", true);
		add_parent<LuaMapView, LuaPanel>(L);
		lua_pop(L, 1);  // Pop the meta table
	} else {
		// Only in main menu
		register_class<LuaMainMenu>(L, "ui", true);
		add_parent<LuaMainMenu, LuaPanel>(L);
		lua_pop(L, 1);  // Pop the meta table
	}
}
}  // namespace LuaUi
