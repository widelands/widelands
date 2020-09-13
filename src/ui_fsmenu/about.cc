/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

constexpr int16_t kPadding = 4;

FullscreenMenuAbout::FullscreenMenuAbout(FullscreenMenuMain& fsmm)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "about",
                (fsmm.get_w() - calc_desired_window_width(fsmm)) / 2,
                (fsmm.get_h() - calc_desired_window_height(fsmm)) / 2,
                calc_desired_window_width(fsmm),
                calc_desired_window_height(fsmm),
                _("About Widelands")),
     parent_(fsmm),
     box_(this, 0, 0, UI::Box::Vertical),
     tabs_(&box_, UI::PanelStyle::kFsMenu, UI::TabPanelStyle::kFsMenu),
     close_(&box_, "close", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Close")) {
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> t(lua.run_script("txts/ABOUT.lua"));
		for (const auto& entry : t->array_entries<std::unique_ptr<LuaTable>>()) {
			try {
				tabs_.add_tab(entry->get_string("name"), entry->get_string("script"));
			} catch (LuaError& err) {
				tabs_.add_tab(_("Lua Error"), "");
				log_err("%s", err.what());
			}
		}
	} catch (LuaError& err) {
		tabs_.add_tab(_("Lua Error"), "");
		log_err("%s", err.what());
	}

	graphic_resolution_changed_subscriber_ = Notifications::subscribe<GraphicResolutionChanged>(
	   [this](const GraphicResolutionChanged&) { layout(); });

	close_.sigclicked.connect([this]() {
		end_modal<MenuTarget>(MenuTarget::kBack);
	});

	box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kPadding);
	box_.add(&close_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add_space(kPadding);

	set_center_panel(&box_);

	layout();
	tabs_.load_tab_contents();
}

bool FullscreenMenuAbout::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			end_modal<MenuTarget>(MenuTarget::kBack);
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void FullscreenMenuAbout::layout() {
	if (!is_minimal()) {
		set_size(calc_desired_window_width(parent_), calc_desired_window_height(parent_));
	}
	UI::Window::layout();
}
