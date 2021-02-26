/*
 * Copyright (C) 2007-2021 by the Widelands Development Team
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

#include "wui/game_tips.h"

#include <cstdlib>
#include <memory>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "io/filesystem/filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

constexpr int kTextPadding = 48;

GameTips::GameTips(UI::ProgressWindow& progressWindow, const std::vector<std::string>& names)
   : last_updated_(0),
     update_after_(0),
     progressWindow_(progressWindow),
     registered_(false),
     last_tip_(0) {
	// Loading the "texts" locale for translating the tips
	i18n::Textdomain textdomain("texts");

	for (const std::string& name : names) {
		load_tips(name);
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
void GameTips::load_tips(const std::string& name) {
	try {
		LuaInterface lua;
		std::string filename = "txts/tips/";
		filename += name;
		filename += ".lua";
		for (const Widelands::TribeBasicInfo& tribe : Widelands::get_all_tribeinfos()) {
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
		const uint32_t next = std::rand() % tips_.size();  // NOLINT
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
	const Image& pic_background = load_safe_template_image("loadscreens/gametips.png");
	const int w = pic_background.width();
	const int h = pic_background.height();
	Vector2i pt(bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2);
	rt.blit(pt, &pic_background);

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(tips_[index].text), w - 2 * kTextPadding);
	pt = Vector2i(bounds.x + (bounds.w - rendered_text->width()) / 2,
	              bounds.y + (bounds.h - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}
