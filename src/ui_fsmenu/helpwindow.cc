/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_fsmenu/helpwindow.h"

#include <memory>

#include <SDL_mouse.h>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_table.h"
#include "ui_basic/button.h"

namespace FsMenu {

HelpWindow::HelpWindow(UI::Panel* const parent,
                       LuaInterface* lua,
                       const std::string& script_path,
                       const std::string& caption,
                       uint32_t width,
                       uint32_t height)
   : UI::Window(parent,
                UI::WindowStyle::kFsMenu,
                "help_window",
                0,
                0,
                width,
                height,
                format(_("Help: %s"), caption)),
     textarea_(
        new UI::MultilineTextarea(this, 5, 5, width - 10, height - 30, UI::PanelStyle::kFsMenu)) {
	int margin = 5;

	// Calculate sizes
	width = (width == 0) ? g_gr->get_xres() * 3 / 5 : width;
	height = (height == 0) ? g_gr->get_yres() * 4 / 5 : height;

	UI::Button* btn = new UI::Button(
	   this, "ok", width / 3, 0, width / 3, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK"));

	btn->sigclicked.connect([this]() { clicked_ok(); });
	btn->set_pos(Vector2i(btn->get_x(), height - margin - btn->get_h()));

	std::string helptext;
	try {
		std::unique_ptr<LuaTable> t(lua->run_script(script_path));
		std::unique_ptr<LuaCoroutine> cr(t->get_coroutine("func"));
		cr->resume();
		helptext = cr->pop_string();
	} catch (LuaError& err) {
		helptext = err.what();
	}

	textarea_->set_size(width - 2 * margin, height - btn->get_h() - 3 * margin);
	textarea_->set_text(helptext);

	set_inner_size(width, height);
	center_to_parent();
	focus();

	initialization_complete();
}

/**
 * Handle mouseclick.
 *
 * Clicking the right mouse button inside the window acts like pressing Ok.
 */
bool HelpWindow::handle_mousepress(const uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_RIGHT) {
		play_click();
		clicked_ok();
		return true;
	}
	return UI::Window::handle_mousepress(btn, x, y);
}

bool HelpWindow::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void HelpWindow::clicked_ok() {
	if (is_modal()) {
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		// Do not call die() here - could lead to broken pointers.
		// The window should get deleted with the parent anyways - best use a unique_ptr there.
		set_visible(false);
	}
}

}  // namespace FsMenu
