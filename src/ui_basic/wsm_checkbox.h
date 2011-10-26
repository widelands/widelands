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

#ifndef WSM_CHECKBOX_H
#define WSM_CHECKBOX_H

/**
 * This class is the same as an ordinary
 * checkbox, the only difference is, it has
 * a small rectangle on it with the color
 * of the graph and it needs a picture
 */
struct WSM_Checkbox : public UI::Checkbox {
	WSM_Checkbox(UI::Panel *, Point, int32_t id, PictureID picid, RGBColor);

	virtual void draw(RenderTarget &);

private:
	PictureID  m_pic;
	RGBColor   m_color;
};


#endif
