/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_fsmenu/intro.h"

#include <SDL_timer.h>

#include "base/i18n.h"

constexpr uint32_t kAnimStart = 3000;
constexpr uint32_t kAnimLength = 5000;

FullscreenMenuIntro::FullscreenMenuIntro()
   : FullscreenMenuBase(),

     // Text area
     message_(this,
              get_w() / 2,
              get_h() * 19 / 20,
              0,
              0,
              _("Press any key or click to continue…"),
              UI::Align::kCenter,
              g_gr->styles().font_style(UI::FontStyle::kFsMenuIntro)),
     init_time_(SDL_GetTicks()) {
	message_.set_font_scale(scale_factor());
	add_overlay_image("images/loadscreens/splash.jpg",
	                  FullscreenWindow::Alignment(UI::Align::kCenter, UI::Align::kCenter));
}

bool FullscreenMenuIntro::handle_mousepress(uint8_t, int32_t, int32_t) {
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	return true;
}

bool FullscreenMenuIntro::handle_key(const bool down, const SDL_Keysym) {
	if (down) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}

	return false;
}

void FullscreenMenuIntro::think() {
	if (SDL_GetTicks() - init_time_ > kAnimStart + kAnimLength) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
	}
	FullscreenMenuBase::think();
}

void FullscreenMenuIntro::draw(RenderTarget& r) {
	FullscreenMenuBase::draw(r);

	const uint32_t time = SDL_GetTicks();
	const float opacity = time - init_time_ < kAnimStart ? 0.f :
	   std::max(0.f, std::min(1.f, static_cast<float>(time - init_time_ - kAnimStart) / kAnimLength));

	r.fill_rect(Recti(0, 0, get_w(), get_h()), RGBAColor(0, 0, 0, opacity * 255), BlendMode::Default);

	const Image* img = g_gr->images().get("images/ui_fsmenu/main_title.png");
	const int w = img->width();
	const int h = img->height();
	r.blitrect_scale(Rectf((get_w() - w) / 2.f, get_h() * 3.f / 40.f + h / 4.f, w, h),
	                 img, Recti(0, 0, w, h), opacity, BlendMode::UseAlpha);
}
