/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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
           PanelStyle s,
           const int32_t x,
           const int32_t y,
           const int32_t w,
           const int32_t h,
           const Image* picture_id)
   : Panel(parent, s, x, y, w, h),
     pic_(picture_id),
     draw_frame_(false),
     grey_out_color_(191, 191, 191, 191),
     grey_out_(false) {
	set_handle_mouse(false);
	set_thinks(false);
}

Icon::Icon(Panel* const parent, PanelStyle s, const Image* picture_id)
   : Icon(parent, s, 0, 0, picture_id->width(), picture_id->height(), picture_id) {
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
		const int available_width = draw_frame_ ? get_w() - 2 : get_w();
		const int available_height = draw_frame_ ? get_h() - 2 : get_h();
		const float scale =
		   std::min(1.f, std::min(static_cast<float>(available_width) / pic_->width(),
		                          static_cast<float>(available_height) / pic_->height()));
		// We need to be pixel perfect, so we use ints.
		const int width = scale * pic_->width();
		const int height = scale * pic_->height();
		const int x = (available_width - width) / 2;
		const int y = (available_height - height) / 2;
		if (grey_out_) {
			dst.blitrect_scale_monochrome(
			   Rectf(draw_frame_ ? x + 1 : x, draw_frame_ ? y + 1 : y, width, height), pic_,
			   Recti(0, 0, pic_->width(), pic_->height()), grey_out_color_);
		} else {
			dst.blitrect_scale(Rectf(draw_frame_ ? x + 1 : x, draw_frame_ ? y + 1 : y, width, height),
			                   pic_, Recti(0, 0, pic_->width(), pic_->height()), 1.,
			                   BlendMode::UseAlpha);
		}
		if (draw_frame_) {
			dst.draw_rect(Recti(x, y, width + 2, height + 2), framecolor_);
		}
	}
}
}  // namespace UI
