/*
 * Copyright (C) 2010-2017 by the Widelands Development Team
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

#include "ui_basic/icon.h"

#include "graphic/image.h"
#include "graphic/rendertarget.h"

namespace UI {

Icon::Icon(Panel* const parent,
           const int32_t x,
           const int32_t y,
           const int32_t w,
           const int32_t h,
           const Image* picture_id)
   : Panel(parent, x, y, w, h), pic_(picture_id), draw_frame_(false) {
	set_handle_mouse(false);
	set_thinks(false);
}

void Icon::set_icon(const Image* picture_id) {
	pic_ = picture_id;
}

void Icon::set_frame(const RGBColor& color) {
	draw_frame_ = true;
	framecolor_.r = color.r;
	framecolor_.g = color.g;
	framecolor_.b = color.b;
}

void Icon::set_no_frame() {
	draw_frame_ = false;
}

void Icon::draw(RenderTarget& dst) {
	if (pic_) {
		const float scale = std::min(1.f, std::min(static_cast<float>(get_w()) / pic_->width(),
		                                           static_cast<float>(get_h()) / pic_->height()));
		// We need to be pixel perfect, so we use ints.
		const int width = scale * get_w();
		const int height = scale * get_h();
		const int x = (get_w() - width) / 2;
		const int y = (get_h() - height) / 2;
		dst.blitrect_scale(Rectf(x, y, width, height), pic_,
		                   Recti(0, 0, pic_->width(), pic_->height()), 1., BlendMode::UseAlpha);
		if (draw_frame_) {
			dst.draw_rect(Recti(x, y, width, height), framecolor_);
		}
	}
}
}
