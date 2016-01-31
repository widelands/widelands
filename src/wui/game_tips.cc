/*
 * Copyright (C) 2007-2016 by the Widelands Development Team
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

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "io/fileread.h"
#include "profile/profile.h"

#define DEFAULT_INTERVAL 5  // seconds
#define BG_IMAGE "pics/tips_bg.png"

GameTips::GameTips
	(UI::ProgressWindow & progressWindow, const std::vector<std::string>& names)
:
lastUpdated_   (0),
updateAfter_   (0),
progressWindow_(progressWindow),
registered_    (false),
lastTip_       (0)
{
	// Loading texts-locals, for translating the tips
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

/// Loads tips out of \var filename
void GameTips::load_tips(std::string name)
{
	std::string filename = "txts/tips/" + name + ".tip";
	try {
		Profile prof(filename.c_str());
		while (Section * const s = prof.get_next_section(nullptr)) {
			char const * const text = s->get_string("text");
			if (nullptr == text)
				continue;

			Tip tip;
			tip.text = text;
			tip.interval = s->get_int("sec", DEFAULT_INTERVAL);
			tips_.push_back (tip);
		}
	} catch (std::exception &) {
		// just ignore - tips do not impact game
		return;
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

	RenderTarget & rt = *g_gr->get_render_target();
	Rect tips_area;

	uint16_t w = pic_background->width();
	uint16_t h = pic_background->height();
	Point pt((g_gr->get_xres() - w) / 2, (g_gr->get_yres() - h) / 2);
	tips_area = Rect(pt, w, h);
	rt.blit(pt, pic_background);

	Point center(tips_area.x + tips_area.w / 2, tips_area.y + tips_area.h / 2);
	const Image* rendered_text = UI::g_fh1->render(as_game_tip(tips_[index].text), tips_area.w);
	rt.blit(center - Point(rendered_text->width() / 2, rendered_text->height() / 2), rendered_text);
}
