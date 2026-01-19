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

#include "scripting/lua_ui.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/luna.h"
#include "scripting/ui/lua_box.h"
#include "scripting/ui/lua_button.h"
#include "scripting/ui/lua_checkbox.h"
#include "scripting/ui/lua_dropdown.h"
#include "scripting/ui/lua_listselect.h"
#include "scripting/ui/lua_main_menu.h"
#include "scripting/ui/lua_map_view.h"
#include "scripting/ui/lua_multiline_textarea.h"
#include "scripting/ui/lua_pagination.h"
#include "scripting/ui/lua_panel.h"
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
#include "ui/basic/messagebox.h"
#include "ui/wui/interactive_player.h"
#include "wlapplication_options.h"

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
	if (upcast(UI::PanelType, temp_##PanelType, panel); temp_##PanelType != nullptr) {              \
		to_lua<LuaType>(L, new LuaType(temp_##PanelType));                                           \
	}

	// clang-format off
	TRY_TO_LUA(Window, LuaWindow)
	else TRY_TO_LUA(Pagination, LuaPagination)
	else TRY_TO_LUA(Box, LuaBox)
	else TRY_TO_LUA(Button, LuaButton)
	else TRY_TO_LUA(Checkbox, LuaCheckbox)
	else TRY_TO_LUA(Radiobutton, LuaRadioButton)
	else TRY_TO_LUA(ProgressBar, LuaProgressBar)
	else TRY_TO_LUA(SpinBox, LuaSpinBox)
	else TRY_TO_LUA(Slider, LuaSlider)
	else if (upcast(UI::DiscreteSlider, temp_DiscreteSlider, panel); temp_DiscreteSlider != nullptr) {
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
	else if (!is_main_menu(L) && panel == get_egbase(L).get_ibase()) {
		to_lua<LuaMapView>(L, new LuaMapView(L));
	} else if (upcast(MapView, temp_MapView, panel); temp_MapView != nullptr) {
		to_lua<LuaMapView>(L, new LuaMapView(temp_MapView));
	} else if (upcast(FsMenu::MainMenu, temp_MainMenu, panel); temp_MainMenu != nullptr) {
		to_lua<LuaMainMenu>(L, new LuaMainMenu(temp_MainMenu));
	} else {
		to_lua<LuaPanel>(L, new LuaPanel(panel));
	}
	// clang-format on
#undef TRY_TO_LUA

	return 1;
}

std::string shortcut_string_if_set(const std::string& name, bool rt_escape) {
	if (name.empty()) {
		return std::string();
	}
	return shortcut_string_for(shortcut_from_string(name), rt_escape);
}

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

/* RST
Module Functions
^^^^^^^^^^^^^^^^

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

/* RST
.. method:: is_rtl()

   .. versionadded:: 1.3

   Returns whether the current locale uses right-to-left text.

   :returns: Whether the text flow is right-to-left.
   :rtype: :class:`boolean`
*/
static int L_is_rtl(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(UI::g_fh->fontset()->is_rtl()));
	return 1;
}

/* RST
.. method:: get_clipboard()

   .. versionadded:: 1.3

   Returns the current content of the system-wide clipboard.

   This may be empty if the clipboard is currently empty or does not contain text data.

   :returns: The clipboard content.
   :rtype: :class:`string`
*/
static int L_get_clipboard(lua_State* L) {
	lua_pushstring(L, SDL_HasClipboardText() != 0 ? SDL_GetClipboardText() : "");
	return 1;
}

/* RST
.. method:: set_clipboard(text)

   .. versionadded:: 1.3

   Set the content of the system-wide clipboard.

   :arg text: Text to set.
   :type text: :class:`string`
*/
static int L_set_clipboard(lua_State* L) {
	SDL_SetClipboardText(luaL_checkstring(L, 1));
	return 0;
}

// TODO(Nordfriese): We do not currently expose the primary selection buffer because it is
// only available if we compile with a sufficiently recent SDL version. Once this becomes the
// standard for all deployments, add the primary buffer bindings
// (HAS_PRIMARY_SELECTION_BUFFER, SDL_SetPrimarySelectionText, SDL_GetPrimarySelectionText) here.

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/

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
   {"is_rtl", &L_is_rtl},
   {"get_clipboard", &L_get_clipboard},
   {"set_clipboard", &L_set_clipboard},
   {nullptr, nullptr}};

void luaopen_wlui(lua_State* L, const bool game_or_editor) {
	lua_getglobal(L, "wl");   // S: wl_table
	lua_pushstring(L, "ui");  // S: wl_table "ui"
	luaL_newlib(L, wlui);     // S: wl_table "ui" wl.ui_table
	lua_settable(L, -3);      // S: wl_table
	lua_pop(L, 1);            // S:

	register_class<LuaPanel>(L, "ui");

	register_class<LuaBox>(L, "ui", true);
	add_parent<LuaBox, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

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

	register_class<LuaPagination>(L, "ui", true);
	add_parent<LuaPagination, LuaPanel>(L);
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
