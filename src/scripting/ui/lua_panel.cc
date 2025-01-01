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

namespace LuaUi {

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
   PROP_RW(LuaPanel, visible),    PROP_RO(LuaPanel, name),    PROP_RO(LuaPanel, parent),
   {nullptr, nullptr, nullptr},
};
const MethodType<LuaPanel> LuaPanel::Methods[] = {
   METHOD(LuaPanel, get_descendant_position),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaPanel, indicate),
#endif
   METHOD(LuaPanel, get_child),
   METHOD(LuaPanel, create_child),
   METHOD(LuaPanel, layout),
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
   .. attribute:: parent

      .. versionadded:: 1.3

      (RO) The direct parent panel of this panel, or ``nil`` for a top-level panel.
*/
int LuaPanel::get_parent(lua_State* L) {
	if (UI::Panel* parent = panel_->get_parent(); parent != nullptr) {
		upcasted_panel_to_lua(L, parent);
	} else {
		lua_pushnil(L);
	}
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
   .. method:: layout()

      .. versionadded:: 1.3

      Force this panel and all its descendants to recompute their layout now.
*/
int LuaPanel::layout(lua_State* /* L */) {
	panel_->layout();
	return 0;
}

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

}  // namespace LuaUi
