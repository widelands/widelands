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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef LUA_UI_H
#define LUA_UI_H

#include <lua.hpp>

#include "ui_basic/button.h"

#include "luna.h"

void luaopen_wlui(lua_State *);


/*
 * Base class for all classes in wl.ui
 */
class L_UiModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "ui";}
};


class L_Panel : public L_UiModuleClass {
protected:
	UI::Panel * m_panel;

public:
	LUNA_CLASS_HEAD(L_Panel);

	L_Panel() : m_panel(0) {}
	L_Panel(UI::Panel * p) : m_panel(p) {}
	L_Panel(lua_State * L) : m_panel(0) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	virtual ~L_Panel() {}

	virtual void __persist(lua_State * L) {
		report_error(L, "Trying to persist a User Interface Panel which is "
			"no supported!");
	}
	virtual void __unpersist(lua_State * L) {
		report_error(L, "Trying to unpersist a User Interface Panel which is "
			"no supported!");
	}

	/*
	 * Properties
	 */
	int get_buttons(lua_State * L);

	/*
	 * Lua Methods
	 */

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
	int release(lua_State * L);
	int click(lua_State * L);

	/*
	 * C Methods
	 */
	UI::Button * get() {return static_cast<UI::Button *>(m_panel);}
};

#endif
