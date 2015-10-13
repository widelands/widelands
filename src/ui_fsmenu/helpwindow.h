/*
 * Copyright (C) 2002-2004, 2006, 2008-2010 by the Widelands Development Team
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

#ifndef WL_UI_FS_MENU_HELPWINDOW_H
#define WL_UI_FS_MENU_HELPWINDOW_H

#include <memory>

#include "scripting/lua_interface.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"


namespace UI {

/**
 * Shows a help window with an OK button.
 * See scripting/widelands/multiplayer_help.lua for an example Lua file.
 */
class FullscreenHelpWindow : public Window {
public:
	FullscreenHelpWindow
		(Panel * parent,
		 LuaInterface* lua,
		 const std::string& script_path,
		 const std::string & caption,
		 uint32_t width = 0, uint32_t height = 0);

	bool handle_mousepress  (uint8_t btn, int32_t mx, int32_t my) override;
	bool handle_mouserelease(uint8_t btn, int32_t mx, int32_t my) override;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	void clicked_ok();

private:
	std::unique_ptr<MultilineTextarea> textarea_;
};


} // namespace UI

#endif  // end of include guard: WL_UI_FS_MENU_HELPWINDOW_H
