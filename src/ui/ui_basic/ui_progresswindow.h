/*
 * Copyright (C) 2007 by the Widelands Development Team
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

#ifndef __S__UI_PROGRESSWINDOW_H
#define __S__UI_PROGRESSWINDOW_H

#include "graphic.h"

#include <string>

namespace UI {
/// Manages a progress window on the screen.
struct ProgressWindow {
	ProgressWindow();

	/// Display a progress step description.
	void step(const std::string & description);
	void stepf(const std::string & format, ...);

private:
	uint  m_xres;
	uint  m_yres;
	Point m_label_center;
	Rect  m_label_rectangle;
	
	void draw_background(RenderTarget & rt,
						 const uint xres,
						 const uint yres);
};
};

#endif // __S__UI_PROGRESSWINDOW_H
