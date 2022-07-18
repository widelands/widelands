/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#include "ui_fsmenu/about.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_fsmenu/tech_info.h"

namespace FsMenu {

About::About(MainMenu& fsmm, UI::UniqueWindow::Registry& r)
   : UI::UniqueWindow(&fsmm,
                      UI::WindowStyle::kFsMenu,
                      "about",
                      &r,
                      fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuAbout),
                      fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuAbout),
                      _("About Widelands")),
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     tabs_(&box_, UI::TabPanelStyle::kFsMenu),
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

	TechInfoBox* infobox = new TechInfoBox(&tabs_, TechInfoBox::Type::kAbout);
	tabs_.add_tab_without_script("info", _("Technical Info"), infobox);

	close_.sigclicked.connect([this]() { die(); });

	box_.add(&tabs_, UI::Box::Resizing::kExpandBoth);
	box_.add_space(kPadding);
	box_.add(&close_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add_space(kPadding);

	do_not_layout_on_resolution_change();

	layout();
	tabs_.load_tab_contents();
	center_to_parent();
	initialization_complete();
}

bool About::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
		case SDLK_ESCAPE:
			die();
			return true;
		default:
			break;
		}
	}
	return UI::Window::handle_key(down, code);
}

void About::layout() {
	UI::Window::layout();
	if (!is_minimal()) {
		box_.set_size(get_inner_w(), get_inner_h());
	}
}

}  // namespace FsMenu
