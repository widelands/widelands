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
#include "ui_basic/textarea.h"
#include "wlapplication_options.h"
#include "wui/interactive_player.h"

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

   The Panel is the most basic ui class. Each ui element is a panel.
*/
const char LuaPanel::className[] = "Panel";
const PropertyType<LuaPanel> LuaPanel::Properties[] = {
   PROP_RO(LuaPanel, buttons), PROP_RO(LuaPanel, dropdowns),  PROP_RO(LuaPanel, tabs),
   PROP_RO(LuaPanel, windows), PROP_RW(LuaPanel, position_x), PROP_RW(LuaPanel, position_y),
   PROP_RW(LuaPanel, width),   PROP_RW(LuaPanel, height),     {nullptr, nullptr, nullptr},
};
const MethodType<LuaPanel> LuaPanel::Methods[] = {
   METHOD(LuaPanel, get_descendant_position),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaPanel, indicate),
#endif
   METHOD(LuaPanel, create_child),
   {nullptr, nullptr},
};

// Look for all descendant panels of class P and add the corresponding Lua version to the currently
// active Lua table. Class P needs to be a NamedPanel.
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

      (RO) The name of this panel
*/

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
	return 1;
}
int LuaPanel::get_height(lua_State* L) {
	assert(panel_);
	lua_pushint32(L, panel_->get_h());
	return 1;
}
int LuaPanel::set_height(lua_State* L) {
	assert(panel_);
	panel_->set_size(panel_->get_w(), luaL_checkint32(L, -1));
	return 1;
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
	return 1;
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
	return 1;
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
   .. method:: create_child(table)

      .. versionadded:: 1.2

      Create a UI widget as a child of this panel as specified by the provided table.

      A UI descriptor table contains multiple keys of type :class:`string`. Common properties are:

         * ``"widget"``: **Mandatory**. The type of widget to create. See below for allowed values.
         * ``"name"``: **Mandatory** for named panels. The internal name of the panel.
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
                     children    = {
                        {
                           widget = "textarea",
                           font   = "wui_info_panel_paragraph",
                           text   = _("Click Yes or No"),
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

	} else if (widget_type == "box") {
		std::string orientation_str = get_table_string(L, "orientation", true);
		unsigned orientation;
		if (orientation_str == "v" || orientation_str == "vert" || orientation_str == "vertical") {
			orientation = UI::Box::Vertical;
		} else if (orientation_str == "h" || orientation_str == "horz" ||
		           orientation_str == "horizontal") {
			orientation = UI::Box::Horizontal;
		} else {
			report_error(L, "Unknown box orientation '%s'", orientation_str.c_str());
		}

		int32_t max_x = get_table_int(L, "max_x", false);
		int32_t max_y = get_table_int(L, "max_y", false);
		int32_t spacing = get_table_int(L, "spacing", false);

		child_as_box =
		   new UI::Box(parent, UI::PanelStyle::kWui, x, y, orientation, max_x, max_y, spacing);
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
		std::string name = get_table_string(L, "name", false);
		if (name.empty()) {
			created_panel = new UI::Panel(parent, UI::PanelStyle::kWui, x, y, w, h, tooltip);
		} else {
			created_panel =
			   new UI::NamedPanel(parent, UI::PanelStyle::kWui, name, x, y, w, h, tooltip);
		}

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

	} else if (widget_type == "textarea") {
		std::string text = get_table_string(L, "text", true);
		UI::FontStyle font = g_style_manager->safe_font_style(get_table_string(L, "font", true));
		UI::Align align = get_table_align(L, "text_align", false);
		UI::Textarea* txt =
		   new UI::Textarea(parent, UI::PanelStyle::kWui, font, x, y, w, h, text, align);
		created_panel = txt;

		txt->set_fixed_width(get_table_int(L, "fixed_width", false));

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
   PROP_RO(LuaButton, name),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaButton::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

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
   PROP_RO(LuaDropdown, name),
   PROP_RO(LuaDropdown, expanded),
   PROP_RO(LuaDropdown, no_of_items),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaDropdown::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
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
   PROP_RO(LuaTab, name),
   PROP_RO(LuaTab, active),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaTab::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

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
   PROP_RO(LuaWindow, name),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaWindow::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
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
