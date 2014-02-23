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

#ifndef UI_ICON_H
#define UI_ICON_H

#include "ui_basic/panel.h"
#include "rgbcolor.h"

namespace UI {

/**
 * A simple icon drawn in the center of the area
*/
struct Icon : public Panel {
	Icon
		(Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 const Image* picture_id);

	void setIcon(const Image* picture_id);
	void setFrame(const RGBColor& color);
	void setNoFrame();

	virtual void draw(RenderTarget &) override;

private:
	const Image* m_pic;
	bool         m_draw_frame;
	RGBColor    m_framecolor;
};

}

#endif
