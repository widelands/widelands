/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

constexpr int kTextPadding = 48;

void draw_game_tip(RenderTarget& rt,
                   const Recti& bounds,
                   const std::string& text,
                   unsigned opacity) {
	const Image& pic_background = load_safe_template_image("loadscreens/gametips.png");
	const int w = pic_background.width();
	const int h = pic_background.height();
	Vector2i pt(bounds.x + (bounds.w - w) / 2, bounds.y + (bounds.h - h) / 2);

	for (; opacity; --opacity) {
		rt.blit(pt, &pic_background);
	}

	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_game_tip(text), w - 2 * kTextPadding);
	pt = Vector2i(bounds.x + (bounds.w - rendered_text->width()) / 2,
	              bounds.y + (bounds.h - rendered_text->height()) / 2);
	rendered_text->draw(rt, pt);
}
