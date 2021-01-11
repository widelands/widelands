/*
 * Copyright (C) 2007-2020 by the Widelands Development Team
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
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

constexpr int kTextPadding = 48;

GameTips::GameTips(UI::ProgressWindow& progressWindow, const std::vector<std::string>& names)
   : lastUpdated_(0),
     updateAfter_(0),
     progressWindow_(progressWindow),
     registered_(false),
     lastTip_(0) {
	// Loading the "texts" locale for translating the tips
	i18n::Textdomain textdomain("texts");

	for (const std::string& name : names) {
		load_tips(name);
	}

	if (!tips_.empty()) {
		// add visualization only if any tips are loaded
		progressWindow_.add_visualization(this);
		registered_ = true;
		lastTip_ = tips_.size();
	}
}

GameTips::~GameTips() {
	stop();
}

/// Loads tips out of \var name
void GameTips::load_tips(const std::string& name) {
	try {
		LuaInterface lua;
		std::unique_ptr<LuaTable> table(lua.run_script("txts/tips/" + name + ".lua"));
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

void GameTips::update(bool repaint) {
	uint32_t ticks = SDL_GetTicks();
	if (ticks >= (lastUpdated_ + updateAfter_)) {
		const uint32_t next = std::rand() % tips_.size();  // NOLINT
		if (next == lastTip_) {
			lastTip_ = (next + 1) % tips_.size();
		} else {
			lastTip_ = next;
		}
		show_tip(next);
		lastUpdated_ = SDL_GetTicks();
		updateAfter_ = tips_[next].interval * 1000;
	} else if (repaint) {
		show_tip(lastTip_);
	}
}

void GameTips::stop() {
	if (registered_) {
		progressWindow_.remove_visualization(this);
		registered_ = false;
	}
}

void GameTips::show_tip(int32_t index) {
	RenderTarget& rt = *g_gr->get_render_target();

	const Image* pic_background =
	   g_image_cache->get(std::string(kTemplateDir) + "loadscreens/gametips.png");
	const int w = pic_background->width();
	const int h = pic_background->height();
	Vector2i pt((g_gr->get_xres() - w) / 2, (g_gr->get_yres() - h) / 2);
	rt.blit(pt, pic_background);

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(tips_[index].text), w - 2 * kTextPadding);
	pt = Vector2i((g_gr->get_xres() - rendered_text->width()) / 2,
	              (g_gr->get_yres() - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}
