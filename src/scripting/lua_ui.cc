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

#include "scripting/lua_ui.h"

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

	// TODO(Nordfriese): Support more types of components

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
	else TRY_TO_LUA(Tab, LuaTab)
	else TRY_TO_LUA(BaseDropdown, LuaDropdown)
	else {
		to_lua<LuaPanel>(L, new LuaPanel(panel));
	}
	// clang-format on
#undef TRY_TO_LUA

	return 1;
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
         * ``"space"``: Only valid as the direct child of a Box. A fixed-size spacer. Property:

           * ``"value"``: **Mandatory**. The size of the space.

         * ``"panel"``: A plain panel that can act as a spacer or as a container for other widgets.

         * ``"textarea"``: A static text area with a single line of text. Properties:

           * ``"text"``: **Mandatory**. The text to display.
           * ``"font"``: **Mandatory**. The font style to use.
           * ``"text_align"``: **Optional**. The alignment of the text. Valid values are
             ``"center"`` (the default), ``"left"``, and ``"right"``.
           * ``"fixed_width"``: **Optional**. If set, the text area's width is fixed instead
             of resizing to accomodate the text or the parent. Default: not set.

         * ``"multilinetextarea"``: A static text area displaying multiple lines of text.
           Properties:

           * ``"text"``: **Mandatory**. The text to display.
           * ``"scroll_mode"``: **Mandatory**. The text area's scrolling behaviour. One of:

             * ``"none"``: The text area expands to accommodate its content instead of scrolling.
             * ``"normal"``: Explicit scrolling only.
             * ``"normal_force"``: Explicit and forced scrolling.
             * ``"log"``: Follow the bottom of the log.
             * ``"log_force"``: Follow the bottom of the log with forced scrolling.

           * ``"font"``: **Optional**. The font style to use.
           * ``"text_align"``: **Optional**. The alignment of the text. Valid values are
             ``"center"`` (the default), ``"left"``, and ``"right"``.

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

         * ``"progressbar"``: A partially filled bar that indicates the progress
           of an operation. Properties:

           * ``"orientation"``: **Mandatory**. The progress bar's direction:
             ``"vertical"`` or ``"horizontal"``.
             The shorthands ``"vert"``, ``"v"``, ``"horz"``, and ``"h"`` may be used.
           * ``"total"``: **Mandatory**. The progress bar's maximum value.
           * ``"state"``: **Mandatory**. The progress bar's initial value.
           * ``"percent"``: **Optional**. Whether to show a percentage instead of absolute values.
             Default: true.

         * ``"spinbox"``: A box with buttons to increase or decrease a numerical value. Properties:

           * ``"unit_w"``: **Mandatory**. The total width of the buttons and value display.
           * ``"value"``: **Mandatory**. The spinbox's initial value.
           * ``"min"``: **Mandatory**. The spinbox's minimum value.
           * ``"max"``: **Mandatory**. The spinbox's maximum value.
           * ``"label"``: **Optional**. Text to display next to the spinbox.
           * ``"units"``: **Optional**. The unit for the spinbox's value. One of:

             * ``"none"`` (default)
             * ``"pixels"``
             * ``"percent"``
             * ``"fields"``
             * ``"minutes"``
             * ``"weeks"``

           * ``"step_size_small"``: **Optional**.
             The amount by which the value changes on each button click.
           * ``"step_size_big"``: **Optional**. If set, the spinbox additionally shows
             buttons to change the value by this larger amount.
           * ``"values"``: **Optional**. An array of integers.
             If set, the spinbox can only switch between the values in this array.
             Can not be combined with ``"step_size_small"`` and ``"step_size_big"``.
           * ``"replacements"``: **Optional**. An array of tables with keys ``"value"`` and
             ``"replacement"``. When the spinbox's value is equal to any replaced value,
             the replacement string is displayed instead of the value.
           * ``"on_changed"``: **Optional**. Callback code to run when the spinbox's value changes.

         * ``"slider"``: A button that can be slid along a line to change a value. Properties:

           * ``"orientation"``: **Mandatory**. The slider's direction:
             ``"vertical"`` or ``"horizontal"``.
             The shorthands ``"vert"``, ``"v"``, ``"horz"``, and ``"h"`` may be used.
           * ``"value"``: **Mandatory**. The slider's initial value.
           * ``"min"``: **Mandatory**. The slider's minimum value.
           * ``"max"``: **Mandatory**. The slider's maximum value.
           * ``"cursor_size"``: **Optional**. The size of the slider button in pixels (default 20).
           * ``"dark"``: **Optional**. Draw the slider darker instead of lighter.
           * ``"on_changed"``: **Optional**. Callback code to run when the slider's value changes.

         * ``"discrete_slider"``: A button that can be slid along a horizontal line to change
           a value between several predefined points. Properties:

           * ``"labels"``: **Mandatory**. Array of strings. Each string defines one slider point.
           * ``"value"``: **Mandatory**. The initially selected value.
           * ``"cursor_size"``: **Optional**. The size of the slider button in pixels (default 20).
           * ``"dark"``: **Optional**. Draw the slider darker instead of lighter.
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

      Note that event callbacks functions must be provided as raw code in string form.
      During the lifetime of a *toolbar* widget, the Lua Interface used by the game may be reset.
      Therefore, any callbacks attached to such widgets must not use any functions or variables
      defined at an arbitrary earlier time by your script -
      they may have been deleted by the time the callback is invoked.
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

static UI::ButtonStyle
get_table_button_style(lua_State* L,
                       const char* key,
                       bool mandatory,
                       UI::ButtonStyle default_value = UI::ButtonStyle::kWuiSecondary) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		std::string str = luaL_checkstring(L, -1);
		if (str == "primary") {
			default_value = UI::ButtonStyle::kWuiPrimary;
		} else if (str == "secondary") {
			default_value = UI::ButtonStyle::kWuiSecondary;
		} else if (str == "menu") {
			default_value = UI::ButtonStyle::kWuiMenu;
		} else {
			report_error(L, "Unknown button style '%s'", str.c_str());
		}
	} else if (mandatory) {
		report_error(L, "Missing button style: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
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

static std::function<void()> create_plugin_action_lambda(lua_State* L, const std::string& cmd) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	return [&egbase, cmd]() {  // do not capture L directly
		try {
			egbase.lua().interpret_string(cmd);
		} catch (const LuaError& e) {
			log_err("Lua error in plugin: %s", e.what());
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

	// Read some common properties
	std::string widget_type = get_table_string(L, "widget", true);
	std::string tooltip = get_table_string(L, "tooltip", false);
	int32_t x = get_table_int(L, "x", false);
	int32_t y = get_table_int(L, "y", false);
	int32_t w = get_table_int(L, "w", false);
	int32_t h = get_table_int(L, "h", false);

	// Actually create the panel
	UI::Panel* created_panel = nullptr;
	UI::Box* child_as_box = nullptr;

	if (widget_type == "button") {
		std::string name = get_table_string(L, "name", true);
		std::string title = get_table_string(L, "title", false);
		std::string icon = get_table_string(L, "icon", false);
		if (title.empty() == icon.empty()) {
			report_error(
			   L, "Button must have either a title or an icon, but not both and not neither");
		}

		UI::ButtonStyle style = get_table_button_style(L, "style", false);
		UI::Button::VisualState visual = get_table_button_visual_state(L, "visual", false);

		UI::Button* button;
		if (title.empty()) {
			button = new UI::Button(
			   parent, name, x, y, w, h, style, g_image_cache->get(icon), tooltip, visual);
		} else {
			button = new UI::Button(parent, name, x, y, w, h, style, title, tooltip, visual);
		}
		created_panel = button;

		button->set_repeating(get_table_boolean(L, "repeating", false));

		if (std::string on_click = get_table_string(L, "on_click", false); !on_click.empty()) {
			button->sigclicked.connect(create_plugin_action_lambda(L, on_click));
		}

	} else if (widget_type == "checkbox") {
		std::string name = get_table_string(L, "name", true);
		std::string title = get_table_string(L, "title", false);
		std::string icon = get_table_string(L, "icon", false);
		bool initial_state = get_table_boolean(L, "state", false);

		if (title.empty() == icon.empty()) {
			report_error(
			   L, "Checkbox must have either a title or an icon, but not both and not neither");
		}

		UI::Checkbox* checkbox;
		if (title.empty()) {
			checkbox = new UI::Checkbox(
			   parent, UI::PanelStyle::kWui, name, Vector2i(x, y), g_image_cache->get(icon), tooltip);
		} else {
			checkbox =
			   new UI::Checkbox(parent, UI::PanelStyle::kWui, name, Vector2i(x, y), title, tooltip);
		}
		created_panel = checkbox;

		checkbox->set_state(initial_state, false);

		if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
			checkbox->changed.connect(create_plugin_action_lambda(L, on_changed));
		}

	} else if (widget_type == "radiogroup") {
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
			group->add_button(parent, UI::PanelStyle::kWui, name, Vector2i(rx, ry),
			                  g_image_cache->get(icon), rtooltip, &radiobutton);

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

	} else if (widget_type == "progressbar") {
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

		UI::ProgressBar* bar =
		   new UI::ProgressBar(parent, UI::PanelStyle::kWui, name, x, y, w, h, orientation);
		created_panel = bar;

		bar->set_total(total);
		bar->set_state(state);
		bar->set_show_percent(percent);

	} else if (widget_type == "spinbox") {
		std::string name = get_table_string(L, "name", true);
		uint32_t unit_w = get_table_int(L, "unit_w", true);
		int32_t val_min = get_table_int(L, "min", true);
		int32_t val_max = get_table_int(L, "max", true);
		int32_t val = get_table_int(L, "value", true);
		int32_t step_size_small = get_table_int(L, "step_size_small", false, 1);
		int32_t step_size_big = get_table_int(L, "step_size_big", false, 0);
		std::string label = get_table_string(L, "label", false);

		if (val_min > val_max) {
			report_error(L, "Malformed spinbox value range");
		}
		if (val < val_min || val > val_max) {
			report_error(L, "Spinbox initial value out of range");
		}

		std::string units_str = get_table_string(L, "units", false);
		UI::SpinBox::Units units;
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
			if (step_size_big != 0 || step_size_small != 1) {
				report_error(L, "Spinbox: Cannot combine value list and step sizes");
			}
			luaL_checktype(L, -1, LUA_TTABLE);
			lua_pushnil(L);
			while (lua_next(L, -2) != 0) {
				value_list.push_back(luaL_checkint32(L, -1));
				lua_pop(L, 1);
			}
		}
		lua_pop(L, 1);

		UI::SpinBox* spinbox = new UI::SpinBox(
		   parent, name, x, y, w, unit_w, val, val_min, val_max, UI::PanelStyle::kWui, label, units,
		   value_list.empty() ?
            step_size_big > 0 ? UI::SpinBox::Type::kBig : UI::SpinBox::Type::kSmall :
            UI::SpinBox::Type::kValueList,
		   step_size_small, step_size_big);
		created_panel = spinbox;

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

	} else if (widget_type == "slider") {
		std::string name = get_table_string(L, "name", true);
		unsigned orientation = get_table_button_box_orientation(L, "orientation", true);
		int32_t val_min = get_table_int(L, "min", true);
		int32_t val_max = get_table_int(L, "max", true);
		int32_t val = get_table_int(L, "value", true);
		uint32_t cursor_size = get_table_int(L, "cursor_size", false, 20);
		bool dark = get_table_boolean(L, "dark", false);

		if (val_min > val_max) {
			report_error(L, "Malformed slider value range");
		}
		if (val < val_min || val > val_max) {
			report_error(L, "Slider initial value out of range");
		}

		UI::Slider* slider;
		if (orientation == UI::Box::Vertical) {
			slider = new UI::VerticalSlider(
			   parent, name, x, y, w, h, val_min, val_max, val,
			   dark ? UI::SliderStyle::kWuiDark : UI::SliderStyle::kWuiLight, cursor_size, tooltip);
		} else {
			slider = new UI::HorizontalSlider(
			   parent, name, x, y, w, h, val_min, val_max, val,
			   dark ? UI::SliderStyle::kWuiDark : UI::SliderStyle::kWuiLight, tooltip, cursor_size);
		}
		created_panel = slider;

		if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
			slider->changed.connect(create_plugin_action_lambda(L, on_changed));
		}

	} else if (widget_type == "discrete_slider") {
		std::string name = get_table_string(L, "name", true);
		uint32_t cursor_size = get_table_int(L, "cursor_size", false, 20);
		uint32_t init_value = get_table_int(L, "value", true);
		bool dark = get_table_boolean(L, "dark", false);

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
		   parent, name, x, y, w, h, labels, init_value,
		   dark ? UI::SliderStyle::kWuiDark : UI::SliderStyle::kWuiLight, tooltip, cursor_size);
		created_panel = slider;

		if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
			slider->changed.connect(create_plugin_action_lambda(L, on_changed));
		}

	} else if (widget_type == "editbox") {
		std::string name = get_table_string(L, "name", true);
		std::string text = get_table_string(L, "text", false);
		bool password = get_table_boolean(L, "password", false);
		bool warning = get_table_boolean(L, "warning", false);

		UI::EditBox* editbox = new UI::EditBox(parent, name, x, y, w, UI::PanelStyle::kWui);
		created_panel = editbox;

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

	} else if (widget_type == "multilineeditbox") {
		std::string name = get_table_string(L, "name", true);
		std::string text = get_table_string(L, "text", false);
		bool password = get_table_boolean(L, "password", false);
		bool warning = get_table_boolean(L, "warning", false);

		UI::MultilineEditbox* editbox =
		   new UI::MultilineEditbox(parent, name, x, y, w, h, UI::PanelStyle::kWui);
		created_panel = editbox;

		editbox->set_password(password);
		editbox->set_warning(warning);

		if (std::string on_changed = get_table_string(L, "on_changed", false); !on_changed.empty()) {
			editbox->changed.connect(create_plugin_action_lambda(L, on_changed));
		}
		if (std::string on_cancel = get_table_string(L, "on_cancel", false); !on_cancel.empty()) {
			editbox->cancel.connect(create_plugin_action_lambda(L, on_cancel));
		}

	} else if (widget_type == "box") {
		std::string name = get_table_string(L, "name", true);
		unsigned orientation = get_table_button_box_orientation(L, "orientation", true);
		int32_t max_x = get_table_int(L, "max_x", false);
		int32_t max_y = get_table_int(L, "max_y", false);
		int32_t spacing = get_table_int(L, "spacing", false);

		child_as_box =
		   new UI::Box(parent, UI::PanelStyle::kWui, name, x, y, orientation, max_x, max_y, spacing);
		created_panel = child_as_box;

		child_as_box->set_scrolling(get_table_boolean(L, "scrolling", false));

	} else if (widget_type == "inf_space") {
		if (as_box == nullptr) {
			report_error(L, "'inf_space' only valid in boxes");
		}
		as_box->add_inf_space();

	} else if (widget_type == "space") {
		if (as_box == nullptr) {
			report_error(L, "'space' only valid in boxes");
		}
		as_box->add_space(get_table_int(L, "value", true));

	} else if (widget_type == "panel") {
		std::string name = get_table_string(L, "name", true);
		created_panel = new UI::Panel(parent, UI::PanelStyle::kWui, name, x, y, w, h, tooltip);

	} else if (widget_type == "window") {
		if (parent != get_egbase(L).get_ibase()) {
			report_error(L, "Windows must be toplevel components");
		}

		std::string name = get_table_string(L, "name", true);
		std::string title = get_table_string(L, "title", true);
		UI::Window* window = new UI::Window(parent, UI::WindowStyle::kWui, name, x, y, w, h, title);
		created_panel = window;

		lua_getfield(L, -1, "content");
		if (!lua_isnil(L, -1)) {
			window->set_center_panel(do_create_child(L, window, nullptr));
		}
		lua_pop(L, 1);

	} else if (widget_type == "unique_window") {
		if (parent != get_egbase(L).get_ibase()) {
			report_error(L, "Unique windows must be toplevel components");
		}

		std::string registry = get_table_string(L, "registry", true);
		UI::UniqueWindow::Registry& reg =
		   get_egbase(L).get_ibase()->unique_windows().get_registry(registry);
		if (reg.window != nullptr) {
			return reg.window;
		}

		std::string name = get_table_string(L, "name", true);
		std::string title = get_table_string(L, "title", true);

		UI::UniqueWindow* window =
		   new UI::UniqueWindow(parent, UI::WindowStyle::kWui, name, &reg, x, y, w, h, title);
		created_panel = window;

		lua_getfield(L, -1, "content");
		if (!lua_isnil(L, -1)) {
			window->set_center_panel(do_create_child(L, window, nullptr));
		}
		lua_pop(L, 1);

	} else if (widget_type == "textarea") {
		std::string name = get_table_string(L, "name", true);
		std::string text = get_table_string(L, "text", true);
		UI::FontStyle font = g_style_manager->safe_font_style(get_table_string(L, "font", true));
		UI::Align align = get_table_align(L, "text_align", false);
		UI::Textarea* txt =
		   new UI::Textarea(parent, UI::PanelStyle::kWui, name, font, x, y, w, h, text, align);
		created_panel = txt;

		txt->set_fixed_width(get_table_int(L, "fixed_width", false));

	} else if (widget_type == "multilinetextarea") {
		std::string name = get_table_string(L, "name", true);
		std::string text = get_table_string(L, "text", true);
		UI::Align align = get_table_align(L, "text_align", false);

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

		UI::MultilineTextarea* txt = new UI::MultilineTextarea(
		   parent, name, x, y, w, h, UI::PanelStyle::kWui, text, align, scroll_mode);
		created_panel = txt;

		if (std::string font = get_table_string(L, "font", false); !font.empty()) {
			txt->set_style(g_style_manager->safe_font_style(font));
		}

	} else {
		// TODO(Nordfriese): Add more widget types
		report_error(L, "Unknown widget type '%s'", widget_type.c_str());
	}

	if (created_panel != nullptr) {
		// Signal bindings
		if (std::string cmd = get_table_string(L, "on_panel_clicked", false); !cmd.empty()) {
			created_panel->clicked.connect(create_plugin_action_lambda(L, cmd));
		}
		if (std::string cmd = get_table_string(L, "on_position_changed", false); !cmd.empty()) {
			created_panel->position_changed.connect(create_plugin_action_lambda(L, cmd));
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

/* RST
Button
------

.. class:: Button

   This represents a simple push button.
*/
const char LuaButton::className[] = "Button";
const MethodType<LuaButton> LuaButton::Methods[] = {
   METHOD(LuaButton, press),
   METHOD(LuaButton, click),
   {nullptr, nullptr},
};
const PropertyType<LuaButton> LuaButton::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/*
 * Lua Functions
 */
/* RST
   .. method:: press

      Press and hold this button. This is mainly to visualize a pressing
      event in tutorials
*/
int LuaButton::press(lua_State* /* L */) {
	log_info("Pressing button '%s'\n", get()->get_name().c_str());
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
   .. method:: click

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
*/
const char LuaDropdown::className[] = "Dropdown";
const MethodType<LuaDropdown> LuaDropdown::Methods[] = {
   METHOD(LuaDropdown, open),
   METHOD(LuaDropdown, highlight_item),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaDropdown, indicate_item),
#endif
   METHOD(LuaDropdown, select),
   {nullptr, nullptr},
};
const PropertyType<LuaDropdown> LuaDropdown::Properties[] = {
   PROP_RO(LuaDropdown, expanded),
   PROP_RO(LuaDropdown, no_of_items),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

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

      (RO) The number of items his dropdown has.
*/
int LuaDropdown::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->size());
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
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

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
   METHOD(LuaMapView, update_toolbar),
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
   .. method:: update_toolbar(field)

      .. versionadded:: 1.2

      Recompute the size and position of the toolbar.
      Call this after you have modified the toolbar in any way.
*/
int LuaMapView::update_toolbar(lua_State* L) {
	get_egbase(L).get_ibase()->finalize_toolbar();
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

	UI::WLMessageBox m(
	   get_egbase(L).get_ibase(), UI::WindowStyle::kWui, title, text,
	   allow_cancel ? UI::WLMessageBox::MBoxType::kOkCancel : UI::WLMessageBox::MBoxType::kOk);
	UI::Panel::Returncodes result = m.run<UI::Panel::Returncodes>();

	lua_pushboolean(L, static_cast<int>(result == UI::Panel::Returncodes::kOk));
	return 1;
}

const static struct luaL_Reg wlui[] = {{"set_user_input_allowed", &L_set_user_input_allowed},
                                       {"get_user_input_allowed", &L_get_user_input_allowed},
                                       {"get_shortcut", &L_get_shortcut},
                                       {"get_ingame_shortcut_help", &L_get_ingame_shortcut_help},
                                       {"get_fastplace_help", &L_get_fastplace_help},
                                       {"get_editor_shortcut_help", &L_get_editor_shortcut_help},
                                       {"show_messagebox", &L_show_messagebox},
                                       {nullptr, nullptr}};

void luaopen_wlui(lua_State* L) {
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

	register_class<LuaTab>(L, "ui", true);
	add_parent<LuaTab, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWindow>(L, "ui", true);
	add_parent<LuaWindow, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMapView>(L, "ui", true);
	add_parent<LuaMapView, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table
}
}  // namespace LuaUi
