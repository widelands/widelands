/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#include "graphic/graphic_functions.h"

#include <memory>

#include "graphic/font_handler.h"
#include "graphic/graphic.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "wlapplication_options.h"

constexpr int kTextPadding = 48;

void draw_game_tip(RenderTarget& rt,
                   const Recti& bounds,
                   const std::string& text,
                   unsigned opacity) {
	const Image& pic_background = *g_image_cache->get("loadscreens/gametips.png");
	const int w = pic_background.width();
	const int h = pic_background.height();
	Vector2i pt(bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2);

	for (; opacity != 0u; --opacity) {
		rt.blit(pt, &pic_background);
	}

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(text), w - 2 * kTextPadding);
	pt = Vector2i(bounds.x + (bounds.w - rendered_text->width()) / 2,
	              bounds.y + (bounds.h - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}

void draw_splashscreen(RenderTarget& rt, const std::string& footer_message, const float opacity) {
	std::string image_name = get_config_string("splash_image", "");
	if (image_name.empty() || resolve_template_image_filename(image_name) == kFallbackImage) {
		image_name = kSplashImage;
	}

	const Image* image = g_image_cache->get(image_name);
	rt.fill_rect(Recti(0, 0, rt.width(), rt.height()), RGBAColor(0, 0, 0, 255), BlendMode::Default);
	rt.blit_fit(image, false, opacity);

	if (!footer_message.empty()) {
		std::string intro_font = get_config_string(
		   "intro_font", g_style_manager->font_style(UI::FontStyle::kFsMenuIntro).as_font_open());
		std::shared_ptr<const UI::RenderedText> footer = UI::g_fh->render(
		   format("<rt><p align=center>%s%s</font></p></rt>", intro_font, footer_message));
		Vector2i footer_pos(rt.width() / 2, rt.height() - 3 * footer->height());
		footer->draw(rt, footer_pos, UI::Align::kCenter);
	}
}
