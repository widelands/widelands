/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#include <memory>

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

#define BG_IMAGE "images/loadscreens/tips_bg.png"

GameTips::GameTips(UI::ProgressWindow& progressWindow, const std::vector<std::string>& names)
   : lastUpdated_(0),
     updateAfter_(0),
     progressWindow_(progressWindow),
     registered_(false),
     lastTip_(0) {
	// Loading the "texts" locale for translating the tips
	i18n::Textdomain textdomain("texts");

	for (uint8_t i = 0; i < names.size(); ++i)
		load_tips(names[i]);

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
		log("Error loading tips script for %s:\n%s\n", name.c_str(), err.what());
		// No further handling necessary - tips do not impact game
	}
}

void GameTips::update(bool repaint) {
	uint8_t ticks = SDL_GetTicks();
	if (ticks >= (lastUpdated_ + updateAfter_)) {
		const uint32_t next = rand() % tips_.size();
		if (next == lastTip_)
			lastTip_ = (next + 1) % tips_.size();
		else
			lastTip_ = next;
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
	// try to load a background
	const Image* pic_background = g_gr->images().get(BG_IMAGE);
	assert(pic_background);

	RenderTarget& rt = *g_gr->get_render_target();

	uint16_t w = pic_background->width();
	uint16_t h = pic_background->height();
	Vector2i pt((g_gr->get_xres() - w) / 2, (g_gr->get_yres() - h) / 2);
	Recti tips_area(pt, w, h);
	rt.blit(pt, pic_background);
// NOCOM
	const Image* rendered_text = UI::g_fh1->render(as_game_tip(tips_[index].text), tips_area.w)->texts[0]->image();
	rt.blit(tips_area.center().cast<int>() - Vector2i(rendered_text->width() / 2, rendered_text->height() / 2),
	        rendered_text);
}
