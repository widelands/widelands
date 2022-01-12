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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/tribal_encyclopedia.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "logic/game_controller.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_player.h"

TribalEncyclopedia::TribalEncyclopedia(InteractivePlayer& parent,
                                       UI::UniqueWindow::Registry& registry,
                                       LuaInterface* const lua)
   : EncyclopediaWindow(parent, registry, lua) {
	const Widelands::TribeDescr& tribe = parent.player().tribe();
	try {
		std::unique_ptr<LuaTable> table(lua_->run_script("tribes/scripting/help/init.lua"));
		std::unique_ptr<LuaCoroutine> cr(table->get_coroutine("func"));
		cr->push_arg(tribe.name());
		upcast(Widelands::Game, game, &parent.egbase());
		if (game->game_controller() == nullptr ||
		    game->game_controller()->get_game_type() == GameController::GameType::kSingleplayer) {
			cr->push_arg("singleplayer");
		} else {
			cr->push_arg("multiplayer");
		}
		cr->resume();
		init(parent, cr->pop_table());
	} catch (LuaError& err) {
		log_err_time(parent.egbase().get_gametime(),
		             "Error loading script for tribal encyclopedia:\n%s\n", err.what());
		UI::WLMessageBox wmb(&parent, UI::WindowStyle::kWui, _("Error!"),
		                     format("Error loading script for tribal encyclopedia:\n%s", err.what()),
		                     UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();
	}

	initialization_complete();
}
