/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include "graphic/rendertarget.h"
#include "logic/item_ware_descr.h"
#include "compile_assert.h"
#include "checkbox.h"

#include "wsm_checkbox.h"


#define COLOR_BOX_HEIGHT 7
#define WARES_DISPLAY_BG "pics/ware_list_bg.png"


WSM_Checkbox::WSM_Checkbox
	(UI::Panel * const parent,
	 Point       const p,
	 int32_t     const id,
	 PictureID   const picid,
	 RGBColor    const color)
:
UI::Checkbox(parent, p, g_gr->get_picture(PicMod_Game,  WARES_DISPLAY_BG)),
m_pic       (picid),
m_color     (color)
{
	set_id(id);
}

/**
 * draw the normal checkbox, the picture and the color rectangle
 */
void WSM_Checkbox::draw(RenderTarget & dst) {
	//  First, draw normal.
	UI::Checkbox::draw(dst);

	//  Now, draw a small box with the color.
	assert(1 <= get_inner_w());
	compile_assert(2 <= COLOR_BOX_HEIGHT);
	dst.fill_rect
		(Rect(Point(1, 1), get_inner_w() - 1, COLOR_BOX_HEIGHT - 2), m_color);

	//  and the item
	dst.blit
		(Point((get_inner_w() - WARE_MENU_PIC_WIDTH) / 2, COLOR_BOX_HEIGHT),
		 m_pic);
}
