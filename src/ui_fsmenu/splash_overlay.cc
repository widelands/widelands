/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include <algorithm>

#include <SDL_timer.h>

#include "base/i18n.h"
#include "base/log.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/graphic_functions.h"
#include "graphic/style_manager.h"
#include "scripting/lua_interface.h"
// #include "sound/sound_handler.h"
#include "ui_fsmenu/splash_overlay.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr uint32_t kIntroMusicDuration = 51000;
constexpr uint kCreditsWidth = 780;
constexpr uint kMargin = 10;
constexpr uint kBoxW = kCreditsWidth + 2 * kMargin;

// TODO(tothxa): This ratio is for the default splash screen, should use a styled size for the top
//               position inside the splash image and query the actual image height.
constexpr float kLogoBottomRatio = 0.35;

void SplashOverlay::draw_fade(RenderTarget& r, const float opacity) {
	draw_splashscreen(r, "", opacity);
}

bool SplashOverlay::draw_main(RenderTarget& r) {
	/** TRANSLATORS: Actually any key works */
	const Rectf image_pos = draw_splashscreen(r, _("Press ‘Space’ to skip credits"));

	if (credits_.get() == nullptr && !load_credits()) {
		return false;
	}

	// Calculate credits box position and size
	const int box_x = (r.width() - kCreditsWidth) / 2 - kMargin;
	const int box_y = image_pos.y + kLogoBottomRatio * image_pos.h;

	int box_h = (1.0f - kLogoBottomRatio) * image_pos.h;
	int box_from_bottom = 6 * g_style_manager->font_style(UI::FontStyle::kFsMenuIntro).size();
	if (image_pos.y > 0) {  // window is taller than fitted image
		assert(abs(r.height() - image_pos.h - 2 * image_pos.y) < 2);  // should be symmetric
		box_from_bottom = std::max<int>(box_from_bottom - image_pos.y, image_pos.h / 12);
	}
	box_h -= box_from_bottom;

	const Recti credits_box_outer(box_x, box_y, kBoxW, box_h);
	const Recti credits_box_inner(
	   box_x + kMargin, box_y + kMargin, kBoxW - 2 * kMargin, box_h - 2 * kMargin);

	// Text and time related calculations
	const int text_pos = calculate_text_position(credits_box_inner.h);

	int outside = 0;
	if (text_pos > credits_box_inner.h) {
		outside = text_pos - credits_box_inner.h;
	} else if (const int bottom = text_pos + credits_->height(); bottom < 0) {
		// text_pos is negative once the top scrolled off

		outside = -bottom;
	}
	assert(0 <= outside && outside <= credits_box_inner.h / 2 + 1);

	constexpr int kBoxDefaultAlpha = 160;
	int box_alpha = kBoxDefaultAlpha;
	if (outside > 0) {
		box_alpha -= kBoxDefaultAlpha * outside * 2 / credits_box_inner.h;
		assert(0 <= box_alpha && box_alpha <= kBoxDefaultAlpha);
	}

	// Draw box and text
	const Recti orig_rect = r.get_rect();
	const Vector2i orig_ofs = r.get_offset();

	r.enter_window(credits_box_outer, nullptr, nullptr);
	r.fill_rect(Recti(0, 0, kBoxW, box_h), RGBAColor(0, 0, 0, box_alpha), BlendMode::Default);
	r.set_window(orig_rect, orig_ofs);

	r.enter_window(credits_box_inner, nullptr, nullptr);
	credits_->draw(r, Vector2i(0, text_pos));
	r.set_window(orig_rect, orig_ofs);

	return true;
}

int SplashOverlay::calculate_text_position(const int window_h) {
	// TODO(tothxa): sdl-mixer >= 2.6.0 can tell us the actual music position, use that where
	//               available
	float progress =
	   static_cast<float>(SDL_GetTicks() - init_time_) / (kIntroMusicDuration - init_time_);
	if (progress > 1.0f) {
		// the music actually lasts a little longer, but we have to clamp the value for box alpha
		progress = 1.0f;
	}

	// The amount we want to move the text during the available time.
	// We start with the top of the actual text at 1.5 * window_h below the top of the window and
	// finish with the bottom of the text 0.5 * window_h above the top of the window.
	const float text_h = 2 * window_h + credits_->height();

	const float text_pos = 1.5f * window_h - progress * text_h;
	return static_cast<int>(text_pos);
}

inline bool check_time_left(uint32_t now) {
	if (now + 10000 > kIntroMusicDuration) {
		log_warn("Starting up took too long, not enough time left for credits");
		return false;
	}
	return true;
}

bool SplashOverlay::load_credits() {
	if (!check_time_left(SDL_GetTicks())) {
		return false;
	}

	verb_log_info("Rendering credits");

	try {
		LuaInterface lua;

		// Push language code to Lua as input for the script
		const std::string& lang = get_config_string("language", "");
		if (lang.empty()) {
			lua.interpret_string("lang = nil");
		} else {
			lua.interpret_string(format("lang = \"%s\"", lang));
		}

		std::unique_ptr<LuaTable> t(lua.run_script("txts/SPLASH.lua"));

		credits_ = UI::g_fh->render(t->get_string(1), kCreditsWidth);

	} catch (const std::exception& e) {
		log_warn("Failed to load text for splash screen: %s", e.what());
		credits_.reset();
	}

	verb_log_info("Finished rendering credits");

	if (credits_.get() == nullptr) {
		return false;
	}

	init_time_ = SDL_GetTicks();

	if (!check_time_left(init_time_)) {
		credits_.reset();
		return false;
	}

	return true;
}

}  // namespace FsMenu
