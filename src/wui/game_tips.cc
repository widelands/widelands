/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#include "wui/game_tips.h"

#include <cstdlib>
#include <memory>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "base/random.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/graphic_functions.h"
#include "graphic/rendertarget.h"
#include "io/filesystem/filesystem.h"
#include "logic/filesystem_constants.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

GameTips::GameTips(UI::ProgressWindow& progressWindow,
                   const std::vector<std::string>& names,
                   const Widelands::AllTribes& t)
   : last_updated_(0),
     update_after_(0),
     progressWindow_(progressWindow),
     registered_(false),
     last_tip_(0) {
	// Loading the "texts" locale for translating the tips
	i18n::Textdomain textdomain("texts");

	for (const std::string& name : names) {
		load_tips(name, t);
	}

	if (!tips_.empty()) {
		// add visualization only if any tips are loaded
		progressWindow_.add_visualization(this);
		registered_ = true;
		last_tip_ = tips_.size();
	}
}

GameTips::~GameTips() {
	stop();
}

/// Loads tips out of \var name
void GameTips::load_tips(const std::string& name, const Widelands::AllTribes& t) {
	try {
		LuaInterface lua;
		std::string filename = "txts/tips/";
		filename += name;
		filename += ".lua";
		for (const Widelands::TribeBasicInfo& tribe : t) {
			if (tribe.name == name) {
				if (tribe.script.compare(0, kAddOnDir.size(), kAddOnDir) == 0) {
					filename = FileSystem::fs_dirname(tribe.script);
					filename += FileSystem::file_separator();
					filename += "tips.lua";
				}
				break;
			}
		}
		std::unique_ptr<LuaTable> table(lua.run_script(filename));
		std::unique_ptr<LuaTable> tip_table;
		for (const int key : table->keys<int>()) {
			tip_table = table->get_table(key);
			Tip tip;
			tip.text = tip_table->get_string("text");
			tip.interval = tip_table->get_int("seconds");
			tips_.push_back(tip);
		}
	} catch (LuaError& err) {
		log_err("Error loading tips script for %s:\n%s\n", name.c_str(), err.what());
		// No further handling necessary - tips do not impact game
	}
}

void GameTips::update(RenderTarget& rt, const Recti& bounds) {
	uint32_t ticks = SDL_GetTicks();
	if (ticks >= (last_updated_ + update_after_)) {
		const uint32_t next = RNG::static_rand(tips_.size());
		if (next == last_tip_) {
			last_tip_ = (next + 1) % tips_.size();
		} else {
			last_tip_ = next;
		}
		last_updated_ = SDL_GetTicks();
		update_after_ = tips_[next].interval * 1000;
	}
	show_tip(rt, bounds, last_tip_);
}

void GameTips::stop() {
	if (registered_) {
		progressWindow_.remove_visualization(this);
		registered_ = false;
	}
}

void GameTips::show_tip(RenderTarget& rt, const Recti& bounds, int32_t index) {
	draw_game_tip(rt, bounds, tips_[index].text);
}
