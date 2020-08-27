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
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

FullscreenMenuAbout::FullscreenMenuAbout()
   : FullscreenMenuBase(),
     title_(this,
            0,
            0,
            0,
            0,
            _("About Widelands"),
            UI::Align::kCenter,
            g_gr->styles().font_style(UI::FontStyle::kFsMenuTitle)),
     close_(this, "close", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("Close")),
     tabs_(this, UI::PanelStyle::kFsMenu, UI::TabPanelStyle::kFsMenu) {
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> t(lua.run_script("txts/ABOUT.lua"));
		for (const auto& entry : t->array_entries<std::unique_ptr<LuaTable>>()) {
			try {
				tabs_.add_tab(entry->get_string("name"), entry->get_string("script"));
			} catch (LuaError& err) {
				tabs_.add_tab(_("Lua Error"), "");
				log("%s\n", err.what());
			}
		}
	} catch (LuaError& err) {
		tabs_.add_tab(_("Lua Error"), "");
		log("%s\n", err.what());
	}

	close_.sigclicked.connect([this]() { clicked_back(); });
	layout();
	tabs_.load_tab_contents();
}

void FullscreenMenuAbout::layout() {
	// Values for alignment and size
	butw_ = get_w() / 5;
	buth_ = get_h() * 9 / 200;
	hmargin_ = get_w() * 19 / 200;
	tab_panel_width_ = get_inner_w() - 2 * hmargin_;
	tab_panel_y_ = get_h() * 14 / 100;

	title_.set_size(get_w(), title_.get_h());
	title_.set_pos(Vector2i(0, buth_));
	close_.set_size(butw_, buth_);
	close_.set_pos(Vector2i(get_w() * 2 / 4 - butw_ / 2, get_inner_h() - hmargin_));
	tabs_.set_pos(Vector2i(hmargin_, tab_panel_y_));
	tabs_.set_size(tab_panel_width_, get_inner_h() - tab_panel_y_ - buth_ - hmargin_);
}
