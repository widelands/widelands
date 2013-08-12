/*
 * Copyright (C) 2010, 2013 by the Widelands Development Team
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

Icon::Icon
	(Panel * const parent,
	 const int32_t x, const int32_t y, const int32_t w, const int32_t h,
	 const Image* picture_id)
	:
	Panel(parent, x, y, w, h),
	m_pic(picture_id)
{
	set_handle_mouse(false);
	set_think(false);
}

void Icon::setIcon(const Image* picture_id) {
	m_pic = picture_id;
	update();
}

void Icon::setFrame(const RGBColor& color)
{
	m_draw_frame = true;
	m_framecolor.r = color.r;
	m_framecolor.g = color.g;
	m_framecolor.b = color.b;
}

void Icon::setNoFrame()
{
	m_draw_frame = false;
}



void Icon::draw(RenderTarget & dst) {
	if (m_pic) {
		int32_t w = (get_w() - m_pic->width()) / 2;
		int32_t h = (get_h() - m_pic->height()) / 2;
		dst.blit(Point(w, h), m_pic);
	}
	if (m_draw_frame) {
		dst.draw_rect(Rect(0, 0, get_w(), get_h()), m_framecolor);
	}
}

}
