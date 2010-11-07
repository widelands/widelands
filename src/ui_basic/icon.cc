/*
 * Copyright (C) 2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "icon.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"

namespace UI {

Icon::Icon
	(Panel * const parent,
	 const int32_t x, const int32_t y, const int32_t w, const int32_t h,
	 const PictureID picture_id)
	:
	Panel(parent, x, y, w, h),
	m_pic(picture_id),
	m_w(w),
	m_h(h)
{
	set_handle_mouse(false);
	set_think(false);
}

void Icon::setIcon(PictureID picture_id) {
	m_pic = picture_id;
	update();
}

void Icon::draw(RenderTarget & dst) {
	assert(m_pic != g_gr->get_no_picture());
	int32_t w = (m_w - m_pic->surface->get_w()) / 2;
	int32_t h = (m_h - m_pic->surface->get_h()) / 2;
	dst.blit(Point(w, h), m_pic);
}

}
