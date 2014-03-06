/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef LUA_UI_H
#define LUA_UI_H

#include "scripting/eris/lua.hpp"
#include "scripting/luna.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/window.h"
#include "wui/interactive_base.h"

namespace LuaUi {

/*
 * Base class for all classes in wl.ui
 */
class L_UiModuleClass : public LunaClass {
	public:
		const char * get_modulename() override {return "ui";}
};


class L_Panel : public L_UiModuleClass {
protected:
	UI::Panel * m_panel;

public:
	LUNA_CLASS_HEAD(L_Panel);

	L_Panel() : m_panel(nullptr) {}
	L_Panel(UI::Panel * p) : m_panel(p) {}
	L_Panel(lua_State * L) : m_panel(nullptr) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	virtual ~L_Panel() {}

	virtual void __persist(lua_State * L) override {
		report_error
			(L, "Trying to persist a User Interface Panel which is no supported!");
	}
	virtual void __unpersist(lua_State * L) override {
		report_error
			(L, "Trying to unpersist a User Interface Panel which is "
			 "not supported!");
	}

	/*
	 * Properties
	 */
	int get_buttons(lua_State * L);
	int get_tabs(lua_State * L);
	int get_windows(lua_State * L);
	int get_mouse_position_x(lua_State * L);
	int get_mouse_position_y(lua_State * L);
	int set_mouse_position_x(lua_State * L);
	int set_mouse_position_y(lua_State * L);
	int get_width(lua_State * L);
	int set_width(lua_State * L);
	int get_height(lua_State * L);
	int set_height(lua_State * L);
	int get_position_x(lua_State * L);
	int set_position_x(lua_State * L);
	int get_position_y(lua_State * L);
	int set_position_y(lua_State * L);

	/*
	 * Lua Methods
	 */
	int get_descendant_position(lua_State * L);

	/*
	 * C Methods
	 */
};

class L_Button : public L_Panel {
public:
	LUNA_CLASS_HEAD(L_Button);

	L_Button() : L_Panel() {}
	L_Button(UI::Panel * p) : L_Panel(p) {}
	L_Button(lua_State * L) : L_Panel(L) {}
	virtual ~L_Button() {}

	/*
	 * Properties
	 */
	int get_name(lua_State * L);

	/*
	 * Lua Methods
	 */
	int press(lua_State * L);
	int click(lua_State * L);

	/*
	 * C Methods
	 */
	UI::Button * get() {return static_cast<UI::Button *>(m_panel);}
};

class L_Tab : public L_Panel {
public:
	LUNA_CLASS_HEAD(L_Tab);

	L_Tab() : L_Panel() {}
	L_Tab(UI::Panel * p) : L_Panel(p) {}
	L_Tab(lua_State * L) : L_Panel(L) {}
	virtual ~L_Tab() {}

	/*
	 * Properties
	 */
	int get_name(lua_State * L);
	int get_active(lua_State * L);

	/*
	 * Lua Methods
	 */
	int click(lua_State * L);

	/*
	 * C Methods
	 */
	UI::Tab * get() {return static_cast<UI::Tab *>(m_panel);}
};

class L_Window : public L_Panel {
public:
	LUNA_CLASS_HEAD(L_Window);

	L_Window() : L_Panel() {}
	L_Window(UI::Panel * p) : L_Panel(p) {}
	L_Window(lua_State * L) : L_Panel(L) {}
	virtual ~L_Window() {}

	/*
	 * Properties
	 */
	int get_name(lua_State * L);

	/*
	 * Lua Methods
	 */
	int close(lua_State * L);

	/*
	 * C Methods
	 */
	UI::Window * get() {return static_cast<UI::Window *>(m_panel);}
};


class L_MapView : public L_Panel {
public:
	LUNA_CLASS_HEAD(L_MapView);

	L_MapView() : L_Panel() {}
	L_MapView(Map_View * p) : L_Panel(p) {};
	L_MapView(lua_State * L);
	virtual ~L_MapView() {}

	virtual void __persist(lua_State *) override {}
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_viewpoint_x(lua_State *);
	int set_viewpoint_x(lua_State *);
	int get_viewpoint_y(lua_State *);
	int set_viewpoint_y(lua_State *);
	int get_buildhelp(lua_State * L);
	int set_buildhelp(lua_State * L);
	int get_census(lua_State * L);
	int set_census(lua_State * L);
	int get_statistics(lua_State * L);
	int set_statistics(lua_State * L);
	int get_is_building_road(lua_State * L);

	/*
	 * Lua Methods
	 */
	int close(lua_State * L);
	int click(lua_State * L);
	int abort_road_building(lua_State * L);
	int start_road_building(lua_State * L);

	/*
	 * C Methods
	 */
	Interactive_Base * get() {return static_cast<Interactive_Base *>(m_panel);}
};

void luaopen_wlui(lua_State *);

};

#endif
